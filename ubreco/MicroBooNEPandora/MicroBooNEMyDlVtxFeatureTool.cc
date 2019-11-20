/**
 *  @file   larpandoracontent/MicroBooNEMyDlVtxFeatureTool.cc
 *
 *  @brief  Implementation of the myDlVtx feature tool class.
 *
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"

#include "larpandoracontent/LArHelpers/LArGeometryHelper.h"
#include "larpandoracontent/LArHelpers/LArClusterHelper.h"

#include "MicroBooNEMyDlVtxFeatureTool.h"

using namespace pandora;

namespace lar_content
{

MicroBooNEMyDlVtxFeatureTool::MicroBooNEMyDlVtxFeatureTool() :
    m_myDlVtxConstant(17)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------

void MicroBooNEMyDlVtxFeatureTool::Run(LArMvaHelper::MvaFeatureVector &featureVector, const VertexSelectionBaseAlgorithm *const pAlgorithm, const Vertex * const pVertex,
    const VertexSelectionBaseAlgorithm::SlidingFitDataListMap &/*slidingFitDataListMap*/, const VertexSelectionBaseAlgorithm::ClusterListMap &clusterListMap,
    const VertexSelectionBaseAlgorithm::KDTreeMap &, const VertexSelectionBaseAlgorithm::ShowerClusterListMap &, const float, float &)
{
    if (PandoraContentApi::GetSettings(*pAlgorithm)->ShouldDisplayAlgorithmInfo())
       std::cout << "----> Running Algorithm Tool: " << this->GetInstanceName() << ", " << this->GetType() << std::endl;

    const VertexList *pInputVertexList(NULL);
    std::string vertexListName("DlVertices3D");
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*pAlgorithm, vertexListName.c_str(), pInputVertexList));
    const Vertex * const pDlVertex(pInputVertexList->front());

    CartesianVector DlVertexPosition(pDlVertex->GetPosition());

    if(DlVertexPosition.GetX()==0 && DlVertexPosition.GetY()==0 && DlVertexPosition.GetZ()==0)
        featureVector.push_back(0.f);
    else
    {
        float score((pVertex->GetPosition()-pDlVertex->GetPosition()).GetMagnitude());

        ClusterList clustersW(clusterListMap.at(TPC_VIEW_W));
        ClusterList sortedLongClusters;
        for (const Cluster *const pCluster : clustersW)
            if (pCluster->GetNCaloHits() > 4) 
                sortedLongClusters.push_back(pCluster);

        if(sortedLongClusters.empty())
            featureVector.push_back(0.f);
        else
        {
            CartesianVector inner(0.f, 0.f, 0.f), outer(0.f, 0.f, 0.f);
            LArClusterHelper::GetExtremalCoordinates(sortedLongClusters, inner, outer);
            double rdist = std::max(fabs(outer.GetZ()-inner.GetZ()), fabs(outer.GetX()-inner.GetX()));
            rdist = std::max(rdist,(1e-9));

            float MyDlVtxFeature(-m_myDlVtxConstant*score/rdist);

            featureVector.push_back(MyDlVtxFeature);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode MicroBooNEMyDlVtxFeatureTool::ReadSettings(const TiXmlHandle xmlHandle)
{
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle,
        "MyDlVtxConstant", m_myDlVtxConstant));

    return STATUS_CODE_SUCCESS;
}

} // namespace lar_content
