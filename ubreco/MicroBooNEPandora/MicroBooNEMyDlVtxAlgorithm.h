/**
 *  @file   larpandoracontent/MicroBooNEMyDlVtxAlgorithm.h
 * 
 *  @brief  Header file for the mydlvtx algorithm class.
 * 
 *  $Log: $
 */
#ifndef LAR_MYDLVTX_ALGORITHM_H
#define LAR_MYDLVTX_ALGORITHM_H 1

#include "Pandora/Algorithm.h"

#include <torch/script.h>

namespace lar_content
{

/**
 *  @brief  MicroBooNEMyDlVtxAlgorithm class
 */
class MicroBooNEMyDlVtxAlgorithm : public pandora::Algorithm
{
public:
    /**
     *  @brief  Factory class for instantiating algorithm
     */
    class Factory : public pandora::AlgorithmFactory
    {
    public:
        pandora::Algorithm *CreateAlgorithm() const;
    };

    /**
      * @brief Default constructor
      */
    MicroBooNEMyDlVtxAlgorithm();

    /**
     *  @brief  Destructor
     */
    ~MicroBooNEMyDlVtxAlgorithm();

private:
    pandora::StatusCode Run();
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);
    pandora::StatusCode Initialize();

    pandora::CartesianVector GetDlVtxForView(const pandora::ClusterList *pClusterList, std::string view, 
                             pandora::CartesianVector positionInput, double length) const;

    pandora::CartesianVector DeepLearning(double out2darr[128][128], std::string view, double length) const;

    // Member variables here
    std::string             m_outputVertexListName;           ///< The name under which to save the output vertex list
    pandora::StringVector   m_inputClusterListNames;          ///< The list of cluster list names
    std::string             m_filePathEnvironmentVariable;    ///< The environment variable providing paths to model files
    std::string             m_modelFileNamePrefix;            ///< The model file name prefix
    std::shared_ptr<torch::jit::script::Module> m_pModule[9]; ///< Array of Pointers to Torch models.
};

//------------------------------------------------------------------------------------------------------------------------------------------

} // namespace lar_content

#endif // #ifndef LAR_MYDLVTX_ALGORITHM_H
