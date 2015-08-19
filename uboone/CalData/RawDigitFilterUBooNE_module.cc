////////////////////////////////////////////////////////////////////////
//
// Class:       RawDigitFilterUBooNE
// Module Type: producer
// File:        RawDigitFilterUBooNE_module.cc
//
//              The intent of this module is to filter out "bad" channels
//              in an input RawDigit data stream. In the current implementation,
//              "bad" is defined as the truncated rms for a channel crossing
//              a user controlled threshold
//
// Configuration parameters:
//
// DigitModuleLabel      - the source of the RawDigit collection
// TruncMeanFraction     - the fraction of waveform bins to discard when
//                         computing the means and rms
// RMSRejectionCut       - vector of maximum allowed rms values to keep channel
//
// Created by Tracy Usher (usher@slac.stanford.edu) on August 17, 2015
//
////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <algorithm>
#include <vector>

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Services/Registry/ServiceHandle.h" 
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "Geometry/Geometry.h"
#include "Utilities/TimeService.h"
#include "Utilities/SimpleTimeService.h"
#include "CalibrationDBI/Interface/IDetPedestalService.h"
#include "CalibrationDBI/Interface/IDetPedestalProvider.h"

#include "RawData/RawDigit.h"
#include "RawData/raw.h"

#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"

class Propagator;

class RawDigitFilterUBooNE : public art::EDProducer
{
public:

    // Copnstructors, destructor.
    explicit RawDigitFilterUBooNE(fhicl::ParameterSet const & pset);
    virtual ~RawDigitFilterUBooNE();

    // Overrides.
    virtual void reconfigure(fhicl::ParameterSet const & pset);
    virtual void produce(art::Event & e);
    virtual void beginJob();
    virtual void endJob();

private:

    // Fcl parameters.
    std::string          fDigitModuleLabel;     ///< The full collection of hits
    float                fTruncMeanFraction;    ///< Fraction for truncated mean
    std::vector<double>  fRmsRejectionCut;      ///< channel rms cut
    unsigned int         fTheChosenWire;        ///< For example hist
    double               fMaxPedestalDiff;      ///< Max pedestal diff to db to warn

    // Statistics.
    int fNumEvent;        ///< Number of events seen.
    
    // Pointers to the histograms we'll create for monitoring what is happening
    TH1D*     fAdcCntHist[3];
    TH1D*     fAveValHist[3];
    TH1D*     fRmsValHist[3];
    TH1D*     fPedValHist[3];
    TH1D*     fAverageHist[3];
    TProfile* fRmsValProf[3];
    TProfile* fPedValProf[3];
    
    
    bool      fFirstEvent;
    
    // Useful services, keep copies for now (we can update during begin run periods)
    art::ServiceHandle<geo::Geometry>   fGeometry;             ///< pointer to Geometry service
    const lariov::IDetPedestalProvider& fPedestalRetrievalAlg; ///< Keep track of an instance to the pedestal retrieval alg
};

DEFINE_ART_MODULE(RawDigitFilterUBooNE)

//----------------------------------------------------------------------------
/// Constructor.
///
/// Arguments:
///
/// pset - Fcl parameters.
///
RawDigitFilterUBooNE::RawDigitFilterUBooNE(fhicl::ParameterSet const & pset) :
                      fNumEvent(0),
                      fFirstEvent(true),
                      fPedestalRetrievalAlg(art::ServiceHandle<lariov::IDetPedestalService>()->GetPedestalProvider())

{
    reconfigure(pset);
    produces<std::vector<raw::RawDigit> >();

    // Report.
    mf::LogInfo("RawDigitFilterUBooNE") << "RawDigitFilterUBooNE configured\n";
}

//----------------------------------------------------------------------------
/// Destructor.
RawDigitFilterUBooNE::~RawDigitFilterUBooNE()
{}

//----------------------------------------------------------------------------
/// Reconfigure method.
///
/// Arguments:
///
/// pset - Fcl parameter set.
///
void RawDigitFilterUBooNE::reconfigure(fhicl::ParameterSet const & pset)
{
    fDigitModuleLabel  = pset.get<std::string>        ("DigitModuleLabel",   "daq");
    fTruncMeanFraction = pset.get<float>              ("TruncMeanFraction",   0.2);
    fRmsRejectionCut   = pset.get<std::vector<double>>("RMSRejectonCut",      std::vector<double>() = {10.,10.,5.});
    fTheChosenWire     = pset.get<unsigned int>       ("TheChosenWire",       1200);
    fMaxPedestalDiff   = pset.get<double>             ("MaxPedestalDiff",      10.);
}

//----------------------------------------------------------------------------
/// Begin job method.
void RawDigitFilterUBooNE::beginJob()
{
    // Access ART's TFileService, which will handle creating and writing
    // histograms and n-tuples for us.
    art::ServiceHandle<art::TFileService> tfs;
    
    // Define the histograms. Putting semi-colons around the title
    // causes it to be displayed as the x-axis label if the histogram
    // is drawn.
    fAdcCntHist[0]  = tfs->make<TH1D>("CntUPlane", ";#adc",  200, 9000., 10000.);
    fAdcCntHist[1]  = tfs->make<TH1D>("CntVPlane", ";#adc",  200, 9000., 10000.);
    fAdcCntHist[2]  = tfs->make<TH1D>("CntWPlane", ";#adc",  200, 9000., 10000.);
    fAveValHist[0]  = tfs->make<TH1D>("AveUPlane", ";Ave",   120,  -30.,    30.);
    fAveValHist[1]  = tfs->make<TH1D>("AveVPlane", ";Ave",   120,  -30.,    30.);
    fAveValHist[2]  = tfs->make<TH1D>("AveWPlane", ";Ave",   120,  -30.,    30.);
    fRmsValHist[0]  = tfs->make<TH1D>("RmsUPlane", ";RMS",   200,    0.,    50.);
    fRmsValHist[1]  = tfs->make<TH1D>("RmsVPlane", ";RMS",   200,    0.,    50.);
    fRmsValHist[2]  = tfs->make<TH1D>("RmsWPlane", ";RMS",   200,    0.,    50.);
    fPedValHist[0]  = tfs->make<TH1D>("PedUPlane", ";Ped",   200,  1950,  2150.);
    fPedValHist[1]  = tfs->make<TH1D>("PedVPlane", ";Ped",   200,  1950,  2150.);
    fPedValHist[2]  = tfs->make<TH1D>("PedWPlane", ";Ped",   200,   350,   550.);
    
    fRmsValProf[0]  = tfs->make<TProfile>("RmsUPlaneProf",  ";Wire #",  2400, 0., 2400., 0., 100.);
    fRmsValProf[1]  = tfs->make<TProfile>("RmsVPlaneProf",  ";Wire #",  2400, 0., 2400., 0., 100.);
    fRmsValProf[2]  = tfs->make<TProfile>("RmsWPlaneProf",  ";Wire #",  3456, 0., 3456., 0., 100.);

    fPedValProf[0]  = tfs->make<TProfile>("PedUPlaneProf",  ";Wire #",  2400, 0., 2400., 1500., 2500.);
    fPedValProf[1]  = tfs->make<TProfile>("PedVPlaneProf",  ";Wire #",  2400, 0., 2400., 1500., 2500.);
    fPedValProf[2]  = tfs->make<TProfile>("PedWPlaneProf",  ";Wire #",  3456, 0., 3456.,    0., 1000.);
    
    fAverageHist[0] = tfs->make<TH1D>("AverageU", ";Bin", 1000, 1500., 2500.);
    fAverageHist[1] = tfs->make<TH1D>("AverageV", ";Bin", 1000, 1500., 2500.);
    fAverageHist[2] = tfs->make<TH1D>("AverageW", ";Bin", 1000,    0., 1000.);
}

//----------------------------------------------------------------------------
/// Produce method.
///
/// Arguments:
///
/// evt - Art event.
///
/// This is the primary method.
///
void RawDigitFilterUBooNE::produce(art::Event & event)
{
    ++fNumEvent;
    
    // Agreed convention is to ALWAYS output to the event store so get a pointer to our collection
    std::unique_ptr<std::vector<raw::RawDigit> > filteredRawDigit(new std::vector<raw::RawDigit>);
    
    // Read in the digit List object(s).
    art::Handle< std::vector<raw::RawDigit> > digitVecHandle;
    event.getByLabel(fDigitModuleLabel, digitVecHandle);
    
    unsigned int maxChannels = fGeometry->Nchannels();
    
    // Commence looping over raw digits
    for(size_t rdIter = 0; rdIter < digitVecHandle->size(); ++rdIter)
    {
        // get the reference to the current raw::RawDigit
        art::Ptr<raw::RawDigit> digitVec(digitVecHandle, rdIter);
        
        raw::ChannelID_t channel = digitVec->Channel();
        
        bool goodChan(true);
        
        // The below try-catch block may no longer be necessary
        // Decode the channel and make sure we have a valid one
        std::vector<geo::WireID> wids;
        try {
            wids = fGeometry->ChannelToWire(channel);
        }
        catch(...)
        {
            //std::cout << "===>> Found illegal channel with id: " << channel << std::endl;
            goodChan = false;
        }
        
        if (channel >= maxChannels || !goodChan) continue;
        
        // Recover plane and wire in the plane
        unsigned int view = wids[0].Plane;
        unsigned int wire = wids[0].Wire;
        
        unsigned int dataSize = digitVec->Samples();
        
        // vector holding uncompressed adc values
        std::vector<short> rawadc(dataSize);
        
        // And now uncompress
        raw::Uncompress(digitVec->ADCs(), rawadc, digitVec->Compression());

        // The strategy for finding the average for a given wire will be to
        // find the most populated bin and the average using the neighboring bins
        // To do this we'll use a map with key the bin number and data the count in that bin
        // Define the map first
        std::map<short,short> binAdcMap;
        
        // Populate the mape
        for(const auto& adcVal : rawadc)
        {
            binAdcMap[adcVal]++;
        }
        
        // Find the max bin
        short binMax(-1);
        short binMaxCnt(0);
        
        for(const auto& binAdcItr : binAdcMap)
        {
            if (binAdcItr.second > binMaxCnt)
            {
                binMax    = binAdcItr.first;
                binMaxCnt = binAdcItr.second;
            }
        }
        
        // fill example hists - throw away code
        if (fFirstEvent && wire == fTheChosenWire)
        {
            for(const auto& binAdcItr : binAdcMap)
            {
                fAverageHist[view]->Fill(binAdcItr.first, binAdcItr.second);
            }
        }
        
        // Armed with the max bin and its count, now set up to get an average
        // about this bin. We'll want to cut off at some user defined fraction
        // of the total bins on the wire
        int minNumBins = (1. - fTruncMeanFraction) * dataSize - 1;
        int curBinCnt(binMaxCnt);
        
        double peakValue(curBinCnt * binMax);
        double truncMean(peakValue);
        
        short binOffset(1);
        
        // This loop to develop the average
        // In theory, we could also keep the sum of the squares for the rms but I had problems doing
        // it that way so will loop twice... (potential time savings goes here!)
        while(curBinCnt < minNumBins)
        {
            if (binAdcMap[binMax-binOffset])
            {
                curBinCnt += binAdcMap[binMax-binOffset];
                truncMean += double(binAdcMap[binMax-binOffset] * (binMax - binOffset));
            }
            
            if (binAdcMap[binMax+binOffset])
            {
                curBinCnt += binAdcMap[binMax+binOffset];
                truncMean += double(binAdcMap[binMax+binOffset] * (binMax + binOffset));
            }
            
            binOffset++;
        }
        
        truncMean /= double(curBinCnt);
        
        binOffset  = 1;
        
        int    rmsBinCnt(binMaxCnt);
        double rmsVal(double(binMax)-truncMean);
        
        rmsVal *= double(rmsBinCnt) * rmsVal;
        
        // Second loop to get the rms
        while(rmsBinCnt < minNumBins)
        {
            if (binAdcMap[binMax-binOffset] > 0)
            {
                int    binIdx  = binMax - binOffset;
                int    binCnt  = binAdcMap[binIdx];
                double binVals = double(binIdx) - truncMean;
                
                rmsBinCnt += binCnt;
                rmsVal    += double(binCnt) * binVals * binVals;
            }
            
            if (binAdcMap[binMax+binOffset] > 0)
            {
                int    binIdx  = binMax + binOffset;
                int    binCnt  = binAdcMap[binIdx];
                double binVals = double(binIdx) - truncMean;
                
                rmsBinCnt += binCnt;
                rmsVal    += double(binCnt) * binVals * binVals;
            }
            
            binOffset++;
        }
        
        rmsVal = std::sqrt(std::max(0.,rmsVal / double(rmsBinCnt)));

        // Recover the database version of the pedestal
        float pedestal = fPedestalRetrievalAlg.PedMean(channel);
        
        // Fill some histograms here
        fAdcCntHist[view]->Fill(curBinCnt, 1.);
        fAveValHist[view]->Fill(std::max(-29.9, std::min(29.9,truncMean - pedestal)), 1.);
        fRmsValHist[view]->Fill(std::min(49.9, rmsVal), 1.);
        fRmsValProf[view]->Fill(wire, rmsVal, 1.);
        fPedValProf[view]->Fill(wire, truncMean, 1.);
        fPedValHist[view]->Fill(truncMean, 1.);

        // Output a message is there is significant different to the pedestal
        if (abs(truncMean - pedestal) > fMaxPedestalDiff)
        {
            mf::LogInfo("RawDigitFilterUBooNE") << ">>> Pedestal mismatch, channel: " << channel << ", new value: " << truncMean << ", original: " << pedestal << ", rms: " << rmsVal << std::endl;
        }
       
        // Keep the RawDigit if below our rejection cut
        if (rmsVal < fRmsRejectionCut[view])
        {
            filteredRawDigit->emplace_back(*digitVec);
        }
        else
        {
            mf::LogInfo("RawDigitFilterUBooNE") <<  "--> Rejecting channel for large rms, channel: " << channel << ", rmsVal: " << rmsVal << ", truncMean: " << truncMean << ", pedestal: " << pedestal << std::endl;
        }
    }
    
    // Reset this silly flag so we only fill our example hists once...
    fFirstEvent = false;
    
    // Add tracks and associations to event.
    event.put(std::move(filteredRawDigit));
}

//----------------------------------------------------------------------------
/// End job method.
void RawDigitFilterUBooNE::endJob()
{
    mf::LogInfo("RawDigitFilterUBooNE") << "Looked at " << fNumEvent << " events" << std::endl;
}
