////////////////////////////////////////////////////////////////////////
// Class:       DecayFinder
// Plugin Type: analyzer module
// File:        DecayFinder.h
//
// Generated by Wouter Van De Pontseele
// June 25, 2020
////////////////////////////////////////////////////////////////////////

#ifndef DECAYFINDER_H
#define DECAYFINDER_H

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "canvas/Persistency/Common/FindMany.h"
#include "canvas/Persistency/Common/FindManyP.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "lardataobj/RecoBase/Hit.h"
#include "lardataobj/RecoBase/Cluster.h"
#include "lardataobj/RawData/RawDigit.h"
#include "lardataobj/RecoBase/SpacePoint.h"
#include "lardataobj/AnalysisBase/BackTrackerMatchingData.h"

#include "lardata/Utilities/GeometryUtilities.h"
#include "lardata/DetectorInfoServices/DetectorPropertiesService.h"


#include "larcore/Geometry/Geometry.h"
#include "nusimdata/SimulationBase/MCParticle.h"
#include "nusimdata/SimulationBase/MCTruth.h"

#include "larpandora/LArPandoraInterface/LArPandoraHelper.h"
#include "TTree.h"

class DecayFinder;

class DecayFinder : public art::EDAnalyzer
{
public:
    explicit DecayFinder(fhicl::ParameterSet const &p);
    // The compiler-generated destructor is fine for non-base
    // classes without bare pointers or other resource use.

    // Plugins should not be copied or assigned.
    DecayFinder(DecayFinder const &) = delete;
    DecayFinder(DecayFinder &&) = delete;
    DecayFinder &operator=(DecayFinder const &) = delete;
    DecayFinder &operator=(DecayFinder &&) = delete;

    // Required functions.
    void analyze(art::Event const &e) override;
    void reconfigure(fhicl::ParameterSet const &p);
    void clearEvent();

    void endSubRun(const art::SubRun &subrun);

private:
    typedef art::Handle<std::vector<recob::Hit>> HitHandle;
    //typedef art::Handle<std::vector<raw::RawDigit>> RawDigitHandle;
    typedef art::Handle<std::vector<simb::MCParticle>> MCParticleHandle;
    typedef art::Handle<std::vector<recob::SpacePoint> > SpacePointHandle;
    typedef art::Handle<std::vector<recob::Cluster> > ClusterHandle;


    // Fields needed for the analyser
    std::string m_hit_producer;
    std::string m_spacepoint_producer;
    std::string m_cluster_producer;
    std::string m_particle_producer;
    //std::string m_rawdigits_producer;
    std::string m_mcparticle_producer;
    bool m_isData;

    lar_pandora::PFParticleVector particles;

    //// Tree for every event
    TTree *fEventTree;
    uint fRun, fSubrun, fEvent;

    // MC info
    UInt_t fNumMCParticles = 0;
    std::vector<Int_t> fTrackId;
    std::vector<Int_t> fMother;
    std::vector<Int_t> fNumberDaughters;
    std::vector<Int_t> fpdg;
    std::vector<double> fEng;
    std::vector<double> fStartPointx;
    std::vector<double> fStartPointy;
    std::vector<double> fStartPointz;
    std::vector<double> fEndPointx;
    std::vector<double> fEndPointy;
    std::vector<double> fEndPointz;
    std::vector<double> fTrueWire;
    std::vector<double> fTrueTime;
    std::vector<double> fTime;
    std::vector<double> fPx;
    std::vector<double> fPy;
    std::vector<double> fPz;
    std::vector<std::string> fprocess;
    std::vector<double> falpha_wire_truth;
    std::vector<double> falpha_time_truth;
    std::vector<double> falpha_x_truth;
    std::vector<double> falpha_y_truth;
    std::vector<double> falpha_z_truth;
    std::vector<double> falpha_integral_truth;
    std::vector<double> falpha_RMS_truth;
    std::vector<double> fbeta_wire_truth;
    std::vector<double> fbeta_time_truth;
    std::vector<double> fbeta_x_truth;
    std::vector<double> fbeta_y_truth;
    std::vector<double> fbeta_z_truth;
    std::vector<double> fbeta_integral_truth;
    std::vector<double> fbeta_RMS_truth;
    std::vector<double> fbeta_E_truth;
    std::vector<double> fbeta_EndE_truth;


    //SpacePoint and Cluster info
    std::vector<double> fcluster_integral;
  //  std::vector<Int_t> fnumber_cluster_in_spacepoint;
    std::vector<UInt_t> fnumber_hits;
    std::vector<Int_t> fcluster_plane;
    std::vector<float> fstart_wire;
    std::vector<float> fend_wire;
    std::vector<float> fstart_time;
    std::vector<double> fcluster_x;
    std::vector<double> fcluster_z;
//    std::vector<Int_t> fcandidate_index_one;
//    std::vector<Int_t> fcandidate_index_two;
//    std::vector<double> fcandidate_time_diff;
//    UInt_t fnumber_candidates = 0;
//    Int_t fcorrectly_found = 0;

    // RawDigit Info. Not used.
    /*
    UInt_t fNumRawDigits = 0;
    std::vector<Int_t> fChannel;
    std::vector<float> fPedestal;
    std::vector<float> fSigma;
    */

    // Reco info
    UInt_t fNumHits = 0;
    std::vector<float> fHitCharge;
    std::vector<float> fHitAmplitude;
    std::vector<float> fHitTime;
    std::vector<UInt_t> fHitPlane;
    std::vector<UInt_t> fHitWire;
};

void DecayFinder::reconfigure(fhicl::ParameterSet const &p)
{
    m_isData = p.get<bool>("is_data", false);
    m_hit_producer = p.get<std::string>("hit_producer", "gaushit");
    //m_spacepoint_producer = p.get<std::string>("spacepoint_producer", "pandora");
  //  m_particle_producer = p.get<std::string>("particle_producer", "pandora");
    //m_rawdigits_producer = p.get<std::string>("rawdigits_producer", "butcher");
    m_mcparticle_producer = p.get<std::string>("mcparticle_producer", "largeant");
    m_cluster_producer = p.get<std::string>("cluster_producer", "gaushitproximity");
}

DecayFinder::DecayFinder(fhicl::ParameterSet const &p)
    : EDAnalyzer(p)
{
    art::ServiceHandle<art::TFileService> tfs;
    this->reconfigure(p);

    //// Check if things are set up properly:
    std::cout << std::endl;
    std::cout << "[DecayFinder constructor] Checking set-up" << std::endl;
    std::cout << "[DecayFinder constructor] hit_producer: " << m_hit_producer << std::endl;
  //  std::cout << "[DecayFinder constructor] spacepoint_producer: " << m_spacepoint_producer << std::endl;
    std::cout << "[DecayFinder constructor] cluster_producer: " << m_cluster_producer << std::endl;
  //  std::cout << "[DecayFinder constructor] particle_producer: " << m_particle_producer << std::endl;
    //std::cout << "[DecayFinder constructor] rawdigits_producer: " << m_rawdigits_producer << std::endl;
    std::cout << "[DecayFinder constructor] is_data: " << m_isData << std::endl;

    //// Tree for every event
    fEventTree = tfs->make<TTree>("Event", "Event Tree");
    fEventTree->Branch("event", &fEvent, "event/i");
    fEventTree->Branch("run", &fRun, "run/i");
    fEventTree->Branch("subrun", &fSubrun, "subrun/i");

        fEventTree->Branch("TrackId", "std::vector<Int_t>", &fTrackId);
        fEventTree->Branch("Mother", "std::vector<Int_t>", &fMother);
        fEventTree->Branch("NumberDaughters", "std::vector<Int_t>", &fNumberDaughters);
        fEventTree->Branch("pdg", "std::vector<Int_t>", &fpdg);
        fEventTree->Branch("Eng", "std::vector<double>", &fEng);
        fEventTree->Branch("StartPointx", "std::vector<double>", &fStartPointx);
        fEventTree->Branch("StartPointy", "std::vector<double>", &fStartPointy);
        fEventTree->Branch("StartPointz", "std::vector<double>", &fStartPointz);
        fEventTree->Branch("EndPointx", "std::vector<double>", &fEndPointx);
        fEventTree->Branch("EndPointy", "std::vector<double>", &fEndPointy);
        fEventTree->Branch("EndPointz", "std::vector<double>", &fEndPointz);
        fEventTree->Branch("TrueWire", "std::vector<double>", &fTrueWire);
        fEventTree->Branch("TrueTime", "std::vector<double>", &fTrueTime);
        fEventTree->Branch("Time", "std::vector<double>", &fTime);
        fEventTree->Branch("Px", "std::vector<double>", &fPx);
        fEventTree->Branch("Py", "std::vector<double>", &fPy);
        fEventTree->Branch("Pz", "std::vector<double>", &fPz);
        fEventTree->Branch("AlphaTruthWire", "std::vector<double>", &falpha_wire_truth);
        fEventTree->Branch("AlphaTruthTime", "std::vector<double>", &falpha_time_truth);
        fEventTree->Branch("AlphaTruthX", "std::vector<double>", &falpha_x_truth);
        fEventTree->Branch("AlphaTruthY", "std::vector<double>", &falpha_y_truth);
        fEventTree->Branch("AlphaTruthZ", "std::vector<double>", &falpha_z_truth);
        fEventTree->Branch("AlphaTruthIntegral", "std::vector<double>", &falpha_integral_truth);
        fEventTree->Branch("AlphaTruthRMS", "std::vector<double>", &falpha_RMS_truth);
        fEventTree->Branch("BetaTruthWire", "std::vector<double>", &fbeta_wire_truth);
        fEventTree->Branch("BetaTruthTime", "std::vector<double>", &fbeta_time_truth);
        fEventTree->Branch("BetaTruthX", "std::vector<double>", &fbeta_x_truth);
        fEventTree->Branch("BetaTruthY", "std::vector<double>", &fbeta_y_truth);
        fEventTree->Branch("BetaTruthZ", "std::vector<double>", &fbeta_z_truth);
        fEventTree->Branch("BetaTruthIntegral", "std::vector<double>", &fbeta_integral_truth);
        fEventTree->Branch("BetaTruthRMS", "std::vector<double>", &fbeta_RMS_truth);
        fEventTree->Branch("BetaTruthE", "std::vector<double>", &fbeta_E_truth);
        fEventTree->Branch("BetaTruthEndE", "std::vector<double>", &fbeta_EndE_truth);
        fEventTree->Branch("process", "std::vector<std::string>", &fprocess);


    fEventTree->Branch("reco_num_hits", &fNumHits, "reco_num_hits/i");
    fEventTree->Branch("reco_hit_charge", "std::vector< float >", &fHitCharge);
    fEventTree->Branch("reco_hit_amplitude", "std::vector< float >", &fHitAmplitude);
    fEventTree->Branch("reco_hit_time", "std::vector< float >", &fHitTime);
    fEventTree->Branch("reco_hit_plane", "std::vector< UInt_t >", &fHitPlane);
    fEventTree->Branch("reco_hit_wire", "std::vector< UInt_t >", &fHitWire);

    /*  No longer needed.
    fEventTree->Branch("raw_Channel", "std::vector<Int_t>", &fChannel);
    fEventTree->Branch("raw_Pedestal", "std::vector<float>", &fPedestal);
    fEventTree->Branch("raw_Sigma", "std::vector<float>", &fSigma);
    */

    fEventTree->Branch("cluster_start_wire", "std::vector<float>", &fstart_wire);
    fEventTree->Branch("cluster_end_wire", "std::vector<float>", &fend_wire);
    fEventTree->Branch("cluster_x", "std::vector<double>", &fcluster_x);
    fEventTree->Branch("cluster_z", "std::vector<double>", &fcluster_z);
    fEventTree->Branch("cluster_time", "std::vector<float>", &fstart_time);
    fEventTree->Branch("cluster_integral", "std::vector<double>", &fcluster_integral);
    fEventTree->Branch("cluster_nhits", "std::vector<UInt_t>", &fnumber_hits);
    fEventTree->Branch("cluster_plane", "std::vector<Int_t>", &fcluster_plane);


}

void DecayFinder::clearEvent()
{

    // Reco info
    fNumHits = 0;
    fHitCharge.clear();
    fHitAmplitude.clear();
    fHitTime.clear();
    fHitPlane.clear();
    fHitWire.clear();

    fNumMCParticles = 0;
    fTrackId.clear();
    fMother.clear();
    fNumberDaughters.clear();
    fpdg.clear();
    fEng.clear();
    fStartPointx.clear();
    fStartPointy.clear();
    fStartPointz.clear();
    fEndPointx.clear();
    fEndPointy.clear();
    fEndPointz.clear();
    fTrueWire.clear();
    fTrueTime.clear();
    fPx.clear();
    fPy.clear();
    fPz.clear();
    falpha_time_truth.clear();
    falpha_wire_truth.clear();
    falpha_x_truth.clear();
    falpha_y_truth.clear();
    falpha_z_truth.clear();
    falpha_integral_truth.clear();
    falpha_RMS_truth.clear();
    fbeta_time_truth.clear();
    fbeta_wire_truth.clear();
    fbeta_x_truth.clear();
    fbeta_y_truth.clear();
    fbeta_z_truth.clear();
    fbeta_integral_truth.clear();
    fbeta_RMS_truth.clear();
    fbeta_E_truth.clear();
    fbeta_EndE_truth.clear();
    fTime.clear();
    fprocess.clear();


    fstart_wire.clear();
    fend_wire.clear();
    fcluster_x.clear();
    fcluster_z.clear();
    fstart_time.clear();
    fcluster_integral.clear();
    fcluster_plane.clear();
    fnumber_hits.clear();


}

DEFINE_ART_MODULE(DecayFinder)
#endif // DECAYFINDER_H
