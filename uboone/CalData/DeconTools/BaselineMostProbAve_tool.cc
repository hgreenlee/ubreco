////////////////////////////////////////////////////////////////////////
/// \file   Baseline.cc
/// \author T. Usher
////////////////////////////////////////////////////////////////////////

#include <cmath>
#include "uboone/CalData/DeconTools/BaselineMostProbAve.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "cetlib/exception.h"

#include <fstream>
#include <algorithm> // std::minmax_element()

namespace uboone_tool
{

//----------------------------------------------------------------------
// Constructor.
BaselineMostProbAve::BaselineMostProbAve(const fhicl::ParameterSet& pset)
{
    configure(pset);
}
    
BaselineMostProbAve::~BaselineMostProbAve()
{
}
    
void BaselineMostProbAve::configure(const fhicl::ParameterSet& pset)
{
    // Get signal shaping service.
    fSignalShaping = art::ServiceHandle<util::SignalShapingServiceMicroBooNE>();
    
    return;
}

    
float BaselineMostProbAve::GetBaseline(const std::vector<float>& holder,
                                       raw::ChannelID_t          channel,
                                       size_t                    roiStart,
                                       size_t                    roiLen) const
{
    if (roiLen < 2) return 0.0; // not enough information

    float base=0;
    
    // Recover the expected electronics noise on this channel
    float deconNoise = 1.26491 * fSignalShaping->GetDeconNoise(channel);
    
    // Is it possible that we might run off the end of the buffer?
    if (roiStart + roiLen > holder.size())
    {
        std::cout << "==> Found roiLen exceeding buffer size, roiStart: " << roiStart << ", roiLen: " << roiLen << ", size: " << holder.size() << std::endl;
        roiLen = holder.size() - roiStart;
    }
    
    // Basic idea is to find the most probable value in the ROI presented to us
    // From that, get the average value within range of the expected noise and
    // return that as the ROI baseline.
    auto const minmax  = std::minmax_element(holder.begin()+roiStart,holder.begin()+roiStart+roiLen);
    
    float min = minmax.first  != holder.end() ? *(minmax.first)  : 0;
    float max = minmax.second != holder.end() ? *(minmax.second) : 0;

    if (max > min)
    {
        // we are being generous and allow for one bin more,
        // which is actually needed in the rare case where (max-min) is an integer
        size_t nbin = 2 * std::ceil(max - min) + 1;
        
        if (nbin == 0)
        {
            std::cout << "==> Found nbin == 0, min: " << min << ", max: " << max << std::endl;
        }
    
        std::vector<int> roiHistVec(nbin, 0);
        
        for(size_t binIdx = roiStart; binIdx < roiStart+roiLen; binIdx++)
        {
            // Provide overkill protection against possibility of a bad index...
            int    intIdx = std::floor(2. * (holder.at(binIdx) - min));
            size_t idx    = std::max(std::min(intIdx,int(nbin-1)),0);
            try {
                roiHistVec.at(idx)++;
            } catch (...) {
                std::cout << "***** index out of range, idx: " << idx << ", nbin: " << nbin << ", val: " << holder.at(binIdx) << ", min/max: " << min << "/" << max << ", intIdx: " << intIdx << std::endl;
                roiHistVec[idx]++;
            }
        }
        
        std::vector<int>::const_iterator mpValItr = std::max_element(roiHistVec.cbegin(),roiHistVec.cend());

        // Really can't see how this can happen...
        if (mpValItr != roiHistVec.end())
        {
            float mpVal   = min + 0.5 * std::distance(roiHistVec.cbegin(),mpValItr);
            int   baseCnt = 0;
        
            for(size_t binIdx = roiStart; binIdx < roiStart+roiLen; binIdx++)
            {
                if (std::fabs(holder.at(binIdx) - mpVal) < deconNoise)
                {
                    base += holder.at(binIdx);
                    baseCnt++;
                }
            }
        
            if (baseCnt > 0) base /= baseCnt;
        }
    }
    
    return base;
}
    
void BaselineMostProbAve::outputHistograms(art::TFileDirectory& histDir) const
{
    // It is assumed that the input TFileDirectory has been set up to group histograms into a common
    // folder at the calling routine's level. Here we create one more level of indirection to keep
    // histograms made by this tool separate.
    
    return;
}
    
}
