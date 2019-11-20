/**
 *  @file   larpandoracontent/MicroBooNEMyDlVtxFeatureTool.h
 *
 *  @brief  Header file for the myDlVtx feature tool class.
 *
 *  $Log: $
 */
#ifndef LAR_MYDLVTX_FEATURE_TOOL_H
#define LAR_MYDLVTX_FEATURE_TOOL_H 1

#include "larpandoracontent/LArVertex/VertexSelectionBaseAlgorithm.h"

namespace lar_content
{

/**
 *  @brief  MicroBooNEMyDlVtxFeatureTool class
 */
class MicroBooNEMyDlVtxFeatureTool : public VertexSelectionBaseAlgorithm::VertexFeatureTool
{
public:
    /**
     *  @brief  Default constructor
     */
    MicroBooNEMyDlVtxFeatureTool();

    /**
     *  @brief  Run the tool
     *
     *  @param  featureVector containing the MyDlVtx feature
     *  @param  pAlgorithm address of the calling algorithm
     *  @param  pVertex address of the vertex
     *
     */
    void Run(LArMvaHelper::MvaFeatureVector &featureVector, const VertexSelectionBaseAlgorithm * const pAlgorithm, const pandora::Vertex * const pVertex,
        const VertexSelectionBaseAlgorithm::SlidingFitDataListMap &/*slidingFitDataListMap*/,const VertexSelectionBaseAlgorithm::ClusterListMap &,
        const VertexSelectionBaseAlgorithm::KDTreeMap &, const VertexSelectionBaseAlgorithm::ShowerClusterListMap &, const float, float &);

private:
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    float           m_myDlVtxConstant ;          ///< The DlVtx constant
};

} // namespace lar_content

#endif // #ifndef LAR_MYDLVTX_FEATURE_TOOL_H
