////////////////////////////////////////////////////////////////////////
// Class:       StoreFlashMatchChi2
// Plugin Type: producer (art v3_01_02)
// File:        StoreFlashMatchChi2_module.cc
//
// Generated at Mon May 20 07:11:36 2019 by David Caratelli using cetskelgen
// from cetlib version v3_05_01.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "art/Framework/Services/Optional/TFileService.h"

#include "lardata/Utilities/AssociationUtil.h"

#include "larpandora/LArPandoraInterface/LArPandoraHelper.h"
#include "larpandora/LArPandoraEventBuilding/LArPandoraSliceIdHelper.h"
#include "larpandora/LArPandoraEventBuilding/SliceIdBaseTool.h"
#include "larpandora/LArPandoraEventBuilding/Slice.h"

#include "lardataobj/RecoBase/PFParticle.h"
#include "lardataobj/RecoBase/PFParticleMetadata.h"
#include "lardataobj/RecoBase/Hit.h"
#include "lardataobj/RecoBase/SpacePoint.h"
#include "lardataobj/RecoBase/OpHit.h"
#include "lardataobj/RecoBase/OpFlash.h"
#include "lardataobj/AnalysisBase/T0.h"

//#include "ubreco/LLSelectionTool/OpT0Finder/Base/OpT0FinderFMWKInterface.h"
#include "ubreco/LLSelectionTool/OpT0Finder/Base/OpT0FinderTypes.h"
#include "ubreco/LLSelectionTool/OpT0Finder/Base/FlashMatchManager.h"

#include "larcore/Geometry/Geometry.h"
#include "larevt/CalibrationDBI/Interface/PmtGainService.h"
#include "larevt/CalibrationDBI/Interface/PmtGainProvider.h"
#include "ubevt/Utilities/PMTRemapService.h"
#include "ubevt/Utilities/PMTRemapProvider.h"

#include "TTree.h"

#include <memory>

class StoreFlashMatchChi2;


class StoreFlashMatchChi2 : public art::EDProducer {
public:
  explicit StoreFlashMatchChi2(fhicl::ParameterSet const& p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  StoreFlashMatchChi2(StoreFlashMatchChi2 const&) = delete;
  StoreFlashMatchChi2(StoreFlashMatchChi2&&) = delete;
  StoreFlashMatchChi2& operator=(StoreFlashMatchChi2 const&) = delete;
  StoreFlashMatchChi2& operator=(StoreFlashMatchChi2&&) = delete;

  // Required functions.
  void produce(art::Event& e) override;

  // Selected optional functions.
  void beginJob() override;
  void endJob() override;

private:

  // Declare member data here.

  ::flashana::FlashMatchManager m_flashMatchManager; ///< The flash match manager
  art::InputTag fFlashProducer;
  std::string fPandoraProducer, fSpacePointProducer;
  float fBeamWindowEnd, fBeamWindowStart;
  float fMaxTotalPE;
  float fChargeToNPhotonsShower, fChargeToNPhotonsTrack;
  std::vector<float> fPMTChannelCorrection;

  ::flashana::Flash_t GetFlashPESpectrum(const recob::OpFlash& opflash);

  void CollectDownstreamPFParticles(const lar_pandora::PFParticleMap &pfParticleMap, 
				    const art::Ptr<recob::PFParticle> &particle,
				    lar_pandora::PFParticleVector &downstreamPFParticles) const;

  void CollectDownstreamPFParticles(const lar_pandora::PFParticleMap &pfParticleMap, 
				    const lar_pandora::PFParticleVector &parentPFParticles,
				    lar_pandora::PFParticleVector &downstreamPFParticles) const;

  void AddDaughters(const art::Ptr<recob::PFParticle>& pfp_ptr,  
		    const art::ValidHandle<std::vector<recob::PFParticle> >& pfp_h, 
		    std::vector<art::Ptr<recob::PFParticle> > &pfp_v);

  TTree* _tree;
  int _evt, _run, _sub;
  float _flashtime;
  float _flashpe;
  std::vector<float> _chisq_v, _nuscore_v;
  std::vector<int> _slpdg_v, _nsps_v;

    // PFP map
    std::map<unsigned int, unsigned int> _pfpmap;

};


StoreFlashMatchChi2::StoreFlashMatchChi2(fhicl::ParameterSet const& p)
  : EDProducer{p}  // ,
  // More initializers here.
{

  produces< std::vector< anab::T0 > >();
  produces< art::Assns <recob::PFParticle, anab::T0> >();

  fFlashProducer      = p.get<art::InputTag>("FlashProducer"   );
  fPandoraProducer    = p.get<std::string>("PandoraProducer"   );
  fSpacePointProducer = p.get<std::string>("SpacePointProducer");
  fBeamWindowStart = p.get<float>("BeamWindowStart");
  fBeamWindowEnd   = p.get<float>("BeamWindowEnd");
  fMaxTotalPE      = p.get<float>("MaxTotalPE");
  fChargeToNPhotonsShower   = p.get<float>("ChargeToNPhotonsShower");
  fChargeToNPhotonsTrack    = p.get<float>("ChargeToNPhotonsTrack");
  fPMTChannelCorrection         = p.get<std::vector<float>>("PMTChannelCorrection");

  m_flashMatchManager.Configure(p.get<flashana::Config_t>("FlashMatchConfig"));

  art::ServiceHandle<art::TFileService> tfs;
  _tree = tfs->make<TTree>("chi2tree","chi2 tree");
  _tree->Branch("evt",&_evt,"evt/I");
  _tree->Branch("run",&_run,"run/I");
  _tree->Branch("sub",&_sub,"sub/I");
  _tree->Branch("flashtime",&_flashtime,"flashtime/F");
  _tree->Branch("flashpe"  ,&_flashpe  ,"flashpe/F"  );
  _tree->Branch("chisq_v","std::vector<float>",&_chisq_v);
  _tree->Branch("nuscore_v","std::vector<float>",&_nuscore_v);
  _tree->Branch("slpdg_v","std::vector<int>",&_slpdg_v);
  _tree->Branch("nsps_v","std::vector<int>",&_nsps_v);

  // Call appropriate produces<>() functions here.
  // Call appropriate consumes<>() for any products to be retrieved by this module.
}

void StoreFlashMatchChi2::produce(art::Event& e)
{

  std::unique_ptr< std::vector<anab::T0> > T0_v(new std::vector<anab::T0>);
  std::unique_ptr< art::Assns <recob::PFParticle, anab::T0> > pfp_t0_assn_v( new art::Assns<recob::PFParticle, anab::T0>  );

  // reset TTree variables
  _evt = e.event();
  _sub = e.subRun();
  _run = e.run();
  _flashtime = -9999.;
  _flashpe   = -9999.;
  _chisq_v.clear();
  _nuscore_v.clear();
  _slpdg_v.clear();
  _nsps_v.clear();

  //  prepare flash object
  ::flashana::Flash_t beamflash;
  beamflash.time = 0.;
  float maxEventPE = 0.;

  // Collect all flashes from the event
  const auto flashes(*e.getValidHandle< std::vector<recob::OpFlash> >(fFlashProducer));
  
  for (const auto &flash : flashes) {

    // get claibrated PE spectrum
    ::flashana::Flash_t thisflash = GetFlashPESpectrum(flash);

    if ( (thisflash.time < fBeamWindowStart) || (thisflash.time > fBeamWindowEnd) )
      continue;
    
    auto totalPE = std::accumulate(thisflash.pe_v.begin(), thisflash.pe_v.end(), 0);

    if (totalPE < fMaxTotalPE)
      continue;

    if (totalPE < maxEventPE) 
      continue;

    // made it this far, save as beam candidate    
    maxEventPE = totalPE;
    beamflash = thisflash;
  }// for all flashes


  if (beamflash.time == 0) {
    // quit here!
    e.put(std::move(T0_v));
    e.put(std::move(pfp_t0_assn_v));
    _tree->Fill();
    return;
  }

  _flashtime = beamflash.time;
  _flashpe   = std::accumulate( beamflash.pe_v.begin(), beamflash.pe_v.end(), 0);


  // grab PFParticles in event
  auto const& pfp_h = e.getValidHandle<std::vector<recob::PFParticle> >(fPandoraProducer);
  
  // grab spacepoints associated with PFParticles
  art::FindManyP<recob::SpacePoint> pfp_spacepoint_assn_v(pfp_h, e, fPandoraProducer);
  // grab tracks associated with PFParticles
  art::FindManyP<recob::Track> pfp_track_assn_v(pfp_h, e, fPandoraProducer);
  
  // grab associated metadata
  art::FindManyP< larpandoraobj::PFParticleMetadata > pfPartToMetadataAssoc(pfp_h, e, fPandoraProducer);    
  auto const& spacepoint_h = e.getValidHandle<std::vector<recob::SpacePoint> >(fSpacePointProducer);
  
  art::FindManyP<recob::Hit> spacepoint_hit_assn_v(spacepoint_h, e, fSpacePointProducer);

  
  _pfpmap.clear();

  for (unsigned int p=0; p < pfp_h->size(); p++)
    _pfpmap[pfp_h->at(p).Self()] = p;
  
  for (unsigned int p=0; p < pfp_h->size(); p++){
    
    auto const& pfp = pfp_h->at(p);

    ::flashana::Flash_t beamflashcopy = beamflash;
    m_flashMatchManager.Reset();
    flashana::QCluster_t lightCluster;
    
    // start from primary PFParticles
    if (pfp.IsPrimary() == false) continue;
    
    const art::Ptr<recob::PFParticle> pfp_ptr(pfp_h, p);
    
    // now build vectors of PFParticles, space-points, and hits for this slice
    std::vector<recob::PFParticle> pfp_v;
    std::vector<art::Ptr<recob::PFParticle> > pfp_ptr_v;
    std::vector<std::vector<art::Ptr<recob::SpacePoint>>> spacepoint_v_v;
    std::vector<std::vector<art::Ptr<recob::Hit>>> hit_v_v;
    
    AddDaughters(pfp_ptr, pfp_h, pfp_ptr_v);
    
    // go through these pfparticles and fill info needed for matching
    for (size_t i=0; i < pfp_ptr_v.size(); i++) {
      
      auto key = pfp_ptr_v.at(i).key();
      recob::PFParticle pfp = *pfp_ptr_v.at(i);
      
      pfp_v.push_back(pfp);
      
      //auto const& spacepoint_ptr_v = pfp_spacepoint_assn_v.at(key);
      const std::vector< art::Ptr<recob::SpacePoint> >& spacepoint_ptr_v = pfp_spacepoint_assn_v.at(key);
      
      std::vector< art::Ptr<recob::Hit> > hit_ptr_v;
      
      for (size_t sp=0; sp < spacepoint_ptr_v.size(); sp++) {

	auto SP = spacepoint_ptr_v[sp];

	auto const& spkey = SP.key();
	const std::vector< art::Ptr<recob::Hit> > this_hit_ptr_v = spacepoint_hit_assn_v.at( spkey );
	for (size_t h=0; h < this_hit_ptr_v.size(); h++) {

	  auto hit = this_hit_ptr_v.at( h );

	  // Only use hits from the collection plane
	  if (hit->View() != geo::kZ)
	    continue;
	  
	  // Add the charged point to the vector
	  const auto &position(SP->XYZ());
	  const auto charge(hit->Integral());
	  lightCluster.emplace_back(position[0], position[1], position[2], charge * (lar_pandora::LArPandoraHelper::IsTrack(pfp_ptr) ? fChargeToNPhotonsTrack : fChargeToNPhotonsShower));

	}// for all hits associated to this spacepoint
      }// fpr all spacepoints
      
    }// for all pfp pointers
    
    // Perform the match
    m_flashMatchManager.Emplace(std::move(beamflashcopy));
    m_flashMatchManager.Emplace(std::move(lightCluster));
    
    const auto matches(m_flashMatchManager.Match());
    
    float FMscore = 9999.;
    
    if (matches.size() != 0)
      FMscore = matches.back().score;
    
    // create T0 object with this information!
    anab::T0 t0(beamflash.time, 0, 0, 0, FMscore);
    T0_v->emplace_back(t0);
    util::CreateAssn(*this, e, *T0_v, pfp_ptr, *pfp_t0_assn_v);
    
    _chisq_v.push_back( FMscore );
    

  }//  for all PFParticles

  /* OLD

  lar_pandora::PFParticleVector pfParticles;
  lar_pandora::PFParticlesToMetadata particlesToMetadata;
  lar_pandora::SpacePointVector spacePoints;
  lar_pandora::SpacePointsToHits spacePointToHitMap;
  lar_pandora::PFParticleMap pfParticleMap;
  lar_pandora::PFParticlesToSpacePoints pfParticleToSpacePointMap;
  lar_pandora::LArPandoraHelper::CollectPFParticles(e, fPandoraProducer, pfParticles, pfParticleToSpacePointMap);
  lar_pandora::LArPandoraHelper::CollectSpacePoints(e, fPandoraProducer, spacePoints, spacePointToHitMap);
  lar_pandora::LArPandoraHelper::BuildPFParticleMap(pfParticles, pfParticleMap);
  lar_pandora::LArPandoraHelper::CollectPFParticleMetadata(e, fPandoraProducer, pfParticles, particlesToMetadata);

  m_flashMatchManager.Reset();

  std::cout << "There are a total of " << pfParticles.size() << " pfparticles" << std::endl;
  
  int nprimary = 0;


  
  for (const art::Ptr<recob::PFParticle> &pfp : pfParticles) {
    
    std::cout << "\t New PFP" << std::endl; 
    
    lar_pandora::MetadataVector pfp_metadata_vec = particlesToMetadata.at(pfp);
    const larpandoraobj::PFParticleMetadata::PropertiesMap &pfp_properties = pfp_metadata_vec.front()->GetPropertiesMap();
    
    //if (pfp_properties.count("IsClearCosmic")) {
    //if (pfp_properties.at("IsClearCosmic") && pfp->IsPrimary()) {
    
    if (pfp->IsPrimary() == false) 
      continue;
      
    if (pfp_properties.count("NuScore"))
      _nuscore_v.push_back( pfp_properties.at("NuScore") );
    else
      _nuscore_v.push_back( -1.0 );
    
    _slpdg_v.push_back( pfp->PdgCode() );
    
    int nsps = 0.;
    
    nprimary += 1;
    
    ::flashana::Flash_t beamflashcopy = beamflash;
    
    m_flashMatchManager.Reset();
    
    lar_pandora::PFParticleVector downstreamPFParticles;
    CollectDownstreamPFParticles(pfParticleMap, pfp, downstreamPFParticles);
    
    flashana::QCluster_t lightCluster;
    for (const auto &particle : downstreamPFParticles) {
      
      // Get the associated spacepoints
      const auto &partToSpacePointIter(pfParticleToSpacePointMap.find(particle));
      if (partToSpacePointIter == pfParticleToSpacePointMap.end())
	continue;
      
      for (const auto &spacePoint : partToSpacePointIter->second) {
	
	nsps += 1;
	
	// Get the associated hit
	const auto &spacePointToHitIter(spacePointToHitMap.find(spacePoint));
	if (spacePointToHitIter == spacePointToHitMap.end())
	  continue;
	
	// Only use hits from the collection plane
	const auto &hit(spacePointToHitIter->second);
	if (hit->View() != geo::kZ)
	  continue;
	
	// Add the charged point to the vector
	const auto &position(spacePoint->XYZ());
	const auto charge(hit->Integral());
	lightCluster.emplace_back(position[0], position[1], position[2], charge * (lar_pandora::LArPandoraHelper::IsTrack(particle) ? fChargeToNPhotonsTrack : fChargeToNPhotonsShower));
	
      }// for all space-points
    }// for all PFParticles in the slice
    
    
    // Perform the match
    m_flashMatchManager.Emplace(std::move(beamflashcopy));
    m_flashMatchManager.Emplace(std::move(lightCluster));

    const auto matches(m_flashMatchManager.Match());
    
    float FMscore = 9999.;
    
    if (matches.size() != 0)
      FMscore = matches.back().score;
    
    // create T0 object with this information!
    anab::T0 t0(beamflash.time, 0, 0, 0, FMscore);
    T0_v->emplace_back(t0);
    util::CreateAssn(*this, e, *T0_v, pfp, *pfp_t0_assn_v);
    
    _chisq_v.push_back( FMscore );
    _nsps_v.push_back( nsps );
    
    std::cout << "\t added association...w/ score " << FMscore << std::endl; 
    
  }// for all pfparticles

  OLD */ 

  _tree->Fill();
  
  e.put(std::move(T0_v));
  e.put(std::move(pfp_t0_assn_v));

}

::flashana::Flash_t StoreFlashMatchChi2::GetFlashPESpectrum(const recob::OpFlash& opflash) {
  
  // prepare conainer to store flash
  ::flashana::Flash_t flash;
  flash.time = opflash.Time();

  
  // geometry service
  const art::ServiceHandle<geo::Geometry> geometry;
  uint nOpDets(geometry->NOpDets());
  std::vector<float> PEspectrum;
  PEspectrum.resize(nOpDets);
  
  // Correct the PE values using the gain database and save the values in the OpDet order
  // gain service
  const ::lariov::PmtGainProvider &gain_provider = art::ServiceHandle<lariov::PmtGainService>()->GetProvider();
  // pmt remapping service
  const ::util::PMTRemapProvider &pmtremap_provider = art::ServiceHandle<util::PMTRemapService>()->GetProvider();
  
  // apply gain to OpDets
  for (uint OpChannel = 0; OpChannel < nOpDets; ++OpChannel)
    {
      auto oldch = pmtremap_provider.OriginalOpChannel(OpChannel);
      auto gain = gain_provider.Gain(oldch);
      uint opdet = geometry->OpDetFromOpChannel(OpChannel);
      
      if (gain != 0)
        {
	  PEspectrum[opdet] = opflash.PEs().at(OpChannel) * 120 / gain * fPMTChannelCorrection.at(OpChannel);
        }
      else
        {
	  PEspectrum[opdet] = 0;
        }
    }

  // Reset variables
  flash.x = flash.y = flash.z = 0;
  flash.x_err = flash.y_err = flash.z_err = 0;
  float totalPE = 0.;
  float sumy = 0., sumz = 0., sumy2 = 0., sumz2 = 0.;
  
  for (unsigned int opdet = 0; opdet < PEspectrum.size(); opdet++)
    {
      double PMTxyz[3];
      geometry->OpDetGeoFromOpDet(opdet).GetCenter(PMTxyz);
      // Add up the position, weighting with PEs
      sumy += PEspectrum[opdet] * PMTxyz[1];
      sumy2 += PEspectrum[opdet] * PMTxyz[1] * PMTxyz[1];
      sumz += PEspectrum[opdet] * PMTxyz[2];
      sumz2 += PEspectrum[opdet] * PMTxyz[2] * PMTxyz[2];
      totalPE += PEspectrum[opdet];
    }
  flash.y = sumy / totalPE;
  flash.z = sumz / totalPE;
  // This is just sqrt(<x^2> - <x>^2)
  if ((sumy2 * totalPE - sumy * sumy) > 0.)
    flash.y_err = std::sqrt(sumy2 * totalPE - sumy * sumy) / totalPE;
  
  if ((sumz2 * totalPE - sumz * sumz) > 0.)
    flash.z_err = std::sqrt(sumz2 * totalPE - sumz * sumz) / totalPE;
  
  // Set the flash properties
  flash.pe_v.resize(nOpDets);
  flash.pe_err_v.resize(nOpDets);
  
  // Fill the flash with the PE spectrum
  for (unsigned int i = 0; i < nOpDets; ++i)
    {
      const auto PE(PEspectrum.at(i));
      flash.pe_v.at(i) = PE;
      flash.pe_err_v.at(i) = std::sqrt(PE);
    }
  
  if (flash.pe_v.size() != nOpDets)
    throw cet::exception("FlashNeutrinoId") << "Number of channels in beam flash doesn't match the number of OpDets!" << std::endl;

  return flash;
}


void StoreFlashMatchChi2::CollectDownstreamPFParticles(const lar_pandora::PFParticleMap &pfParticleMap, 
						       const art::Ptr<recob::PFParticle> &particle,
						       lar_pandora::PFParticleVector &downstreamPFParticles) const
{
  if (std::find(downstreamPFParticles.begin(), downstreamPFParticles.end(), particle) == downstreamPFParticles.end())
    downstreamPFParticles.push_back(particle);
  
  for (const auto &daughterId : particle->Daughters())
    {
      const auto iter(pfParticleMap.find(daughterId));
      if (iter == pfParticleMap.end())
	throw cet::exception("FlashNeutrinoId") << "Scrambled PFParticle IDs" << std::endl;
      
      this->CollectDownstreamPFParticles(pfParticleMap, iter->second, downstreamPFParticles);
    }
}


void StoreFlashMatchChi2::CollectDownstreamPFParticles(const lar_pandora::PFParticleMap &pfParticleMap, 
						       const lar_pandora::PFParticleVector &parentPFParticles,
						       lar_pandora::PFParticleVector &downstreamPFParticles) const
{
  for (const auto &particle : parentPFParticles)
    this->CollectDownstreamPFParticles(pfParticleMap, particle, downstreamPFParticles);
}

void StoreFlashMatchChi2::AddDaughters(const art::Ptr<recob::PFParticle>& pfp_ptr,  const art::ValidHandle<std::vector<recob::PFParticle> >& pfp_h, std::vector<art::Ptr<recob::PFParticle> > &pfp_v) {
  
  auto daughters = pfp_ptr->Daughters();
  
  pfp_v.push_back(pfp_ptr);
  
  for(auto const& daughterid : daughters) {
    
    if (_pfpmap.find(daughterid) == _pfpmap.end()) {
      std::cout << "Did not find DAUGHTERID in map! error"<< std::endl;
      continue;
    }
    
    const art::Ptr<recob::PFParticle> pfp_ptr(pfp_h, _pfpmap.at(daughterid) );
    
    AddDaughters(pfp_ptr, pfp_h, pfp_v);
    
  }// for all daughters
  
  return;
}


void StoreFlashMatchChi2::beginJob()
{
  // Implementation of optional member function here.
}

void StoreFlashMatchChi2::endJob()
{
  // Implementation of optional member function here.
}

DEFINE_ART_MODULE(StoreFlashMatchChi2)
