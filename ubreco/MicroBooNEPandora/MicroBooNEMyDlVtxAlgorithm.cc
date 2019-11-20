/**
 *  @file   larpandoracontent/MicroBooNEMyDlVtxAlgorithm.cc
 * 
 *  @brief  Implementation of the mydlvtx algorithm class.
 * 
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h" 

#include "larpandoracontent/LArHelpers/LArGeometryHelper.h"
#include "larpandoracontent/LArHelpers/LArMCParticleHelper.h"
#include "larpandoracontent/LArHelpers/LArFileHelper.h"

#include "MicroBooNEMyDlVtxAlgorithm.h"

#include <algorithm>

#include <iostream>
#include <fstream>
#include <torch/script.h>
#include <memory>
  
using namespace pandora;

namespace lar_content
{

MicroBooNEMyDlVtxAlgorithm::MicroBooNEMyDlVtxAlgorithm() :
    m_outputVertexListName(),
    m_inputClusterListNames(),
    m_filePathEnvironmentVariable("FW_SEARCH_PATH"),
    m_pModule()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
MicroBooNEMyDlVtxAlgorithm::~MicroBooNEMyDlVtxAlgorithm()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
StatusCode MicroBooNEMyDlVtxAlgorithm::Initialize()
{
    // Load the model file. 
    std::string view[3];
    view[0]="W";view[1]="V";view[2]="U";

    for(int j=0; j<3;j++)
    {
        for(int i=0; i<3;i++)
        {
            std::string fileNameString(m_modelFileNamePrefix+std::to_string(j)+view[i]+".pt");
            std::string fullFileNameString(LArFileHelper::FindFileInPath(fileNameString, m_filePathEnvironmentVariable));

            try
            {
                m_pModule[3*j+i] = torch::jit::load(fullFileNameString);
            }
            catch (const c10::Error &e)
            {
                std::cout << "Error loading the PyTorch module" << std::endl;
                return STATUS_CODE_FAILURE;
            }
        }
    }

    return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------
StatusCode MicroBooNEMyDlVtxAlgorithm::Run()
{
    torch::manual_seed(0);

    const ClusterList *clustersU, *clustersV, *clustersW;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, m_inputClusterListNames[2], clustersW));
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, m_inputClusterListNames[1], clustersV));
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, m_inputClusterListNames[0], clustersU));

    std::string viewW="W", viewV="V", viewU="U";
    CartesianVector positionInput(0.f, 0.f, 0.f); 
    CartesianVector positionW(this->GetDlVtxForView(clustersW, viewW, positionInput, 0.f));
    CartesianVector positionV(this->GetDlVtxForView(clustersV, viewV, positionInput, 0.f));
    CartesianVector positionU(this->GetDlVtxForView(clustersU, viewU, positionInput, 0.f));

    CartesianVector positionW1(this->GetDlVtxForView(clustersW, viewW, positionW, 50.0f));
    CartesianVector positionV1(this->GetDlVtxForView(clustersV, viewV, positionV, 50.0f));
    CartesianVector positionU1(this->GetDlVtxForView(clustersU, viewU, positionU, 50.0f));

    CartesianVector positionW2(this->GetDlVtxForView(clustersW, viewW, positionW1, 40.0f));
    CartesianVector positionV2(this->GetDlVtxForView(clustersV, viewV, positionV1, 40.0f));
    CartesianVector positionU2(this->GetDlVtxForView(clustersU, viewU, positionU1, 40.0f));

    CartesianVector position3D(0.f, 0.f, 0.f); float chiSquared(0);
    if(positionW2.GetX()!=0 && positionV2.GetX()!=0 && positionU2.GetX()!=0)
    {
        LArGeometryHelper::MergeThreePositions3D(this->GetPandora(), TPC_VIEW_W, TPC_VIEW_V, TPC_VIEW_U,
                    positionW2, positionV2, positionU2, position3D, chiSquared);
    }

    const VertexList *pVertexList(NULL); std::string temporaryListName;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::CreateTemporaryListAndSetCurrent(*this, pVertexList, temporaryListName));

    PandoraContentApi::Vertex::Parameters parameters;
    parameters.m_position = position3D;
    parameters.m_vertexLabel = VERTEX_INTERACTION;
    parameters.m_vertexType = VERTEX_3D;
    const Vertex *pVertex(NULL);

    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::Vertex::Create(*this, parameters, pVertex));
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::SaveList<Vertex>(*this, m_outputVertexListName));
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::ReplaceCurrentList<Vertex>(*this, m_outputVertexListName));

    return STATUS_CODE_SUCCESS;
}

//---------------------------------------------------------------------------------------------------------------------------------
CartesianVector MicroBooNEMyDlVtxAlgorithm::GetDlVtxForView(const pandora::ClusterList *pClusterList, std::string view,
                CartesianVector positionInput, double length) const
{
    double *xarr, *zarr, *sigma, *height;
    int i(0), j(0), l(0), count1(0), count2(0);

    for (ClusterList::const_iterator iter = pClusterList->begin(), iterEnd = pClusterList->end(); iter != iterEnd; ++iter)
    {
        if( ((*iter)->GetNCaloHits()<5) && (length==0)) continue;
        if( ((*iter)->GetNCaloHits()>=5) ) count2+=(*iter)->GetNCaloHits();
        count1+=(*iter)->GetNCaloHits();
    }

    if(count1==0||count2==0){CartesianVector position(0.f, 0.f, 0.f); return(position);}

    xarr  = new double[count1]; zarr   = new double[count1];
    sigma = new double[count1]; height = new double[count1];

    for(i=0; i<count1; i++) {xarr[i]=0; zarr[i]=0; sigma[i]=0; height[i]=0;}

    for (ClusterList::const_iterator iter = pClusterList->begin(), iterEnd = pClusterList->end(); iter != iterEnd; ++iter)
    {
        if( ((*iter)->GetNCaloHits()<5) && (length==0)) continue;
        const OrderedCaloHitList &orderedCaloHitList1((*iter)->GetOrderedCaloHitList());
        for (OrderedCaloHitList::const_iterator iter1 = orderedCaloHitList1.begin(), iter1End = orderedCaloHitList1.end(); iter1 != iter1End; ++iter1)
        {
            for (CaloHitList::const_iterator hitIter1 = iter1->second->begin(), hitIter1End = iter1->second->end(); hitIter1 != hitIter1End; ++hitIter1)
            {
                const CartesianVector &positionVector1((*hitIter1)->GetPositionVector());
                xarr[l]=positionVector1.GetX(); zarr[l]=positionVector1.GetZ();
                sigma[l]=(*hitIter1)->GetCellSize1(); height[l]=(*hitIter1)->GetInputEnergy(); l++;
            }
        }
    }

    double minx=0, minz=0, npixels=128, nstepx=0, nstepz=0;
    double out2darr[128][128], zC=0, xC=0;
    double lengthX(0), lengthZ(0), length1(0);

    if(length==0)
    {
        minx=(*std::min_element(xarr,xarr+l))-10.0;
        minz=(*std::min_element(zarr,zarr+l))-10.0;
        lengthX=(*std::max_element(xarr,xarr+l)) - (*std::min_element(xarr,xarr+l))+20.0;
        lengthZ=(*std::max_element(zarr,zarr+l)) - (*std::min_element(zarr,zarr+l))+20.0;
        length1=std::max(lengthX, lengthZ);
        nstepx=length1/npixels;
        nstepz=length1/npixels;
    }
    else
    {
        minx=positionInput.GetX()-length/2.0;
        minz=positionInput.GetZ()-length/2.0;
        nstepx=length/npixels;
        nstepz=length/npixels;
    }

    if(length==0)
    {
        delete[] xarr; delete[] zarr; delete[] sigma; delete[] height; count1=0; l=0;
        for (ClusterList::const_iterator iter = pClusterList->begin(), iterEnd = pClusterList->end(); iter != iterEnd; ++iter)
        {
            count1+=(*iter)->GetNCaloHits();
        }
        if(count1==0){CartesianVector position(0.f, 0.f, 0.f); return(position);}

        xarr  = new double[count1]; zarr   = new double[count1];
        sigma = new double[count1]; height = new double[count1];
        for(i=0; i<count1; i++) {xarr[i]=0; zarr[i]=0; sigma[i]=0; height[i]=0;}

        for (ClusterList::const_iterator iter = pClusterList->begin(), iterEnd = pClusterList->end(); iter != iterEnd; ++iter)
        {
            const OrderedCaloHitList &orderedCaloHitList1((*iter)->GetOrderedCaloHitList());
            for (OrderedCaloHitList::const_iterator iter1 = orderedCaloHitList1.begin(), iter1End = orderedCaloHitList1.end(); iter1 != iter1End; ++iter1)
            {
                for (CaloHitList::const_iterator hitIter1 = iter1->second->begin(), hitIter1End = iter1->second->end(); hitIter1 != hitIter1End; ++hitIter1)
                {
                    const CartesianVector &positionVector1((*hitIter1)->GetPositionVector());
                    xarr[l]=positionVector1.GetX(); zarr[l]=positionVector1.GetZ();
                    sigma[l]=(*hitIter1)->GetCellSize1(); height[l]=(*hitIter1)->GetInputEnergy(); l++;
                }
            }
        }
    }

    for(i=0;i<npixels;i++)
        for(j=0;j<npixels;j++)
            out2darr[i][j]=0;

    for(i=0;i<l;i++)
    {
         zC=(int)((zarr[i]-minz)/nstepz); xC=(int)((xarr[i]-minx)/nstepx);
         if(xC>npixels||xC<0||zC>npixels||zC<0) continue;
         if(zC==npixels) zC--;

         for(j=0;j<npixels;j++)
         {
             double tempval(0),sigmaZ(0.3),inputvalue1(0),dist(0);
             tempval=height[i]*(0.5*erfc(-((minx+(j+1)*nstepx)-xarr[i])/sqrt(2*sigma[i]*sigma[i]))
                                   - 0.5*erfc(-((minx+j*nstepx)-xarr[i])/sqrt(2*sigma[i]*sigma[i])));

             for(int a=npixels-1;a>-1;a--)
             {
                 dist=(minz+(a+1)*nstepz)-(zarr[i]+sigmaZ/2.0);

                 if(dist>nstepz)
                     inputvalue1=0;
                 else if(dist<nstepz&&dist>0)
                     inputvalue1=std::min((nstepz-dist),sigmaZ)*tempval;
                 else if(dist<0 && fabs(dist)<nstepz)
                     {if(fabs(dist)>sigmaZ) inputvalue1=0; else inputvalue1=std::min((sigmaZ-fabs(dist)),nstepz)*tempval;}
                 else if(dist<0 && fabs(dist)>nstepz)
                     {if(fabs(dist)>sigmaZ) inputvalue1=0; else inputvalue1=std::min((sigmaZ-fabs(dist)),nstepz)*tempval;}

                 out2darr[j][a]+=inputvalue1;
             }
         }
    }

    /*******************************************************************************************/
    /* Use Deep Learning here on the created image to get the 2D vertex coordinate for 1 view. */ 
    CartesianVector pixelPosition(this->DeepLearning(out2darr,view,length));
    /*******************************************************************************************/

    double recoX(minx+(pixelPosition.GetX())*nstepx);
    double recoZ(minz+(pixelPosition.GetZ())*nstepz);
    CartesianVector position(recoX, 0.f, recoZ);

    delete[] xarr; delete[] zarr; delete[] sigma; delete[] height;
    return(position);
}

//---------------------------------------------------------------------------------------------------------------------------------
CartesianVector MicroBooNEMyDlVtxAlgorithm::DeepLearning(double out2darr[128][128], std::string view, double length) const
{
    int k(0);
    if(length==0) k=0;
    else if(fabs(length-50)<0.1) k=1;
    else if(fabs(length-40)<0.1) k=2;

    /* Get the index for model */
    int index(0);
    if(view=="W") index=3*k+0;
    if(view=="V") index=3*k+1;
    if(view=="U") index=3*k+2;

    /* Convert image to Torch "Tensor" */
    int i=0,j=0;
    const int npixels=128;

    torch::Tensor input = torch::zeros({1, 1, npixels, npixels}, torch::TensorOptions().dtype(torch::kFloat32));
    auto accessor = input.accessor<float, 4>();

    for(i=npixels-1;i>-1;i--)
        for(j=0;j<npixels;j++)
        {
            accessor[0][0][i][j]=out2darr[j][i]/100.0;
        }

    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(input);

    /* Use the Model on the "Tensor" */
    at::Tensor output = m_pModule[index]->forward(inputs).toTensor();
    auto outputAccessor = output.accessor<float, 2>();

    /* Get predicted vertex. */
    CartesianVector position(outputAccessor[0][0], 0.f, outputAccessor[0][1]);

    return(position);
}

//------------------------------------------------------------------------------------------------------------------------------------------
StatusCode MicroBooNEMyDlVtxAlgorithm::ReadSettings(const TiXmlHandle xmlHandle)
{
    // Read settings from xml file here
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle,
        "OutputVertexListName", m_outputVertexListName));

    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadVectorOfValues(xmlHandle,
        "InputClusterListNames", m_inputClusterListNames));

    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle,
        "ModelFileNamePrefix", m_modelFileNamePrefix));

    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle,
        "FilePathEnvironmentVariable", m_filePathEnvironmentVariable));

    return STATUS_CODE_SUCCESS;
}

} // namespace lar_content
