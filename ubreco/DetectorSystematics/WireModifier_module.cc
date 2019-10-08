////////////////////////////////////////////////////////////////////////
// Class:       WireModifier
// Plugin Type: producer (art v3_01_02)
// File:        WireModifier_module.cc
//
// Generated at Thu Aug 22 12:19:37 2019 by Wesley Ketchum using cetskelgen
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

#include <memory>

#include "lardataobj/RawData/RawDigit.h"
#include "lardataobj/RecoBase/Wire.h"
#include "lardataobj/RecoBase/Hit.h"
#include "lardataobj/Simulation/SimEnergyDeposit.h"
#include "larcoreobj/SimpleTypesAndConstants/PhysicalConstants.h"

#include "lardata/Utilities/AssociationUtil.h"

#include "TFile.h"
#include "TSpline.h"

namespace sys {
  class WireModifier;
}


class sys::WireModifier : public art::EDProducer {
public:
  explicit WireModifier(fhicl::ParameterSet const& p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  WireModifier(WireModifier const&) = delete;
  WireModifier(WireModifier&&) = delete;
  WireModifier& operator=(WireModifier const&) = delete;
  WireModifier& operator=(WireModifier&&) = delete;

  // Required functions.
  void produce(art::Event& e) override;

private:

  art::InputTag fWireInputTag;
  art::InputTag fSimEdepShiftedInputTag;
  art::InputTag fSimEdepOrigInputTag;
  bool          fMakeRawDigitAssn;
  bool          fCopyRawDigitAssn;

  std::string   fSplinesFileName;
  std::vector<std::string> fSplineNames_Charge_X;
  std::vector<std::string> fSplineNames_Sigma_X;

  std::vector<TSpline3*> fTSplines_Charge_X;
  std::vector<TSpline3*> fTSplines_Sigma_X;

  //useful math things
  static constexpr double ONE_OVER_SQRT_2PI = 1./std::sqrt(2*util::pi());
  double GAUSSIAN(double t, double mean,double sigma,double a=1.0){
    return ( (a/sigma * ONE_OVER_SQRT_2PI * std::exp( -1.*(t-mean)*(t-mean)*0.5/sigma/sigma) ));
  }

  static constexpr double A_w = 3.33328;
  static constexpr double C_U = 338.140;
  static constexpr double C_V = 2732.53;
  static constexpr double C_Y = 4799.19;
  static constexpr double A_t = 18.2148;
  static constexpr double C_t = 818.351;
  static constexpr double SIN_SIXTY = std::sqrt(3)/2;
  static constexpr double COS_SIXTY = 0.5;


  typedef std::pair<unsigned int,unsigned int> ROI_Key_t;
  std::map< ROI_Key_t,std::vector<size_t> > fROIMatchedEdepMap;

  typedef struct ROIProperties{
    unsigned int plane;
    float begin;
    float end;

    float total_q;

    float center;   //charge weighted center of ROI
    float sigma;    //charge weighted RMS of ROI

  } ROIProperties_t;

  typedef struct TruthProperties{
    double x;
    double x_rms;
    double tick;
    double tick_rms;
    double charge;
  } TruthProperties_t;

  typedef struct ScaleValues{
    double r_Q;
    double r_sigma;
  } ScaleValues_t;

  ROIProperties_t CalcROIProperties(recob::Wire::RegionsOfInterest_t::datarange_t const&);

  std::vector< std::pair<unsigned int, unsigned int> > 
  GetTargetROIs(sim::SimEnergyDeposit const&);
  
  void FillROIMatchedEdepMap(std::vector<sim::SimEnergyDeposit> const&,
			     std::vector<recob::Wire> const&);

  std::vector<TruthProperties_t> CalcPropertiesFromEdeps(std::vector< std::pair<const sim::SimEnergyDeposit*,const sim::SimEnergyDeposit* > > const&);

  std::vector<ScaleValues_t> GetScaleValues(std::vector<TruthProperties_t> const&,ROIProperties_t const&);

  void ModifyROI(//recob::Wire::RegionsOfInterest_t::datarange_t::vector_t &,
		 std::vector<float> &,
		 ROIProperties_t const&,
		 std::vector<TruthProperties_t> const&,
		 std::vector<ScaleValues_t> const&);

};


sys::WireModifier::ROIProperties_t 
sys::WireModifier::CalcROIProperties(recob::Wire::RegionsOfInterest_t::datarange_t const& roi)
{
  ROIProperties_t roi_vals;
  roi_vals.begin = roi.begin_index();
  roi_vals.end = roi.end_index();

  roi_vals.center=0;
  roi_vals.total_q=0;
  roi_vals.sigma=0;

  auto const& roi_data = roi.data();
  for(size_t i_t = 0; i_t<roi_data.size(); ++i_t){
    roi_vals.center += roi_data[i_t]*(i_t+roi_vals.begin);
    roi_vals.total_q += roi_data[i_t];
  }
  roi_vals.center = roi_vals.center/roi_vals.total_q;

  for(size_t i_t = 0; i_t<roi_data.size(); ++i_t)
    roi_vals.sigma += roi_data[i_t]*(i_t+roi_vals.begin-roi_vals.center)*(i_t-+roi_vals.begin-roi_vals.center);
  roi_vals.sigma = std::sqrt(roi_vals.sigma/roi_vals.total_q);

  return roi_vals;
}

std::vector< std::pair<unsigned int, unsigned int> > 
sys::WireModifier::GetTargetROIs(sim::SimEnergyDeposit const& shifted_edep)
{
  //channel number, time tick
  std::vector< std::pair<unsigned int,unsigned int> > target_roi_vec;

  int edep_U_wire = std::round( A_w*(-SIN_SIXTY*shifted_edep.Y() + COS_SIXTY*shifted_edep.Z()) + C_U );
  int edep_V_wire = std::round( A_w*( SIN_SIXTY*shifted_edep.Y() + COS_SIXTY*shifted_edep.Z()) + C_V );
  int edep_Y_wire = std::round( A_w*shifted_edep.Z() + C_Y );
  int edep_tick   = std::round( A_t*shifted_edep.X() + C_t );

  if (edep_tick<0 || edep_tick>=6400)
    return target_roi_vec;

  if(edep_U_wire>=0 && edep_U_wire<2400)
    target_roi_vec.emplace_back((unsigned int)edep_U_wire,(unsigned int)edep_tick);

  if(edep_V_wire>=2400 && edep_V_wire<4800)
    target_roi_vec.emplace_back((unsigned int)edep_V_wire,(unsigned int)edep_tick);

  if(edep_Y_wire>=4800 && edep_Y_wire<8256)
    target_roi_vec.emplace_back((unsigned int)edep_Y_wire,(unsigned int)edep_tick);

  return target_roi_vec;
}


void sys::WireModifier::FillROIMatchedEdepMap(std::vector<sim::SimEnergyDeposit> const& edepVec,
					      std::vector<recob::Wire> const& wireVec)
{
  fROIMatchedEdepMap.clear();

  std::unordered_map<unsigned int,unsigned int> wireChannelMap;
  for(size_t i_w=0; i_w<wireVec.size(); ++i_w)
    wireChannelMap[wireVec[i_w].Channel()] = i_w;

  for(size_t i_e=0; i_e<edepVec.size(); ++i_e){

    std::vector< std::pair<unsigned int, unsigned int> > target_rois = GetTargetROIs(edepVec[i_e]); 

    for( auto const& target_roi : target_rois){

      auto const& target_wire = wireVec.at(wireChannelMap[target_roi.first]);

      if(target_wire.SignalROI().n_ranges()==0) continue;
      if(target_wire.SignalROI().is_void(target_roi.second)) continue;

      auto range_number = target_wire.SignalROI().find_range_iterator(target_roi.second) - target_wire.SignalROI().begin_range();

      fROIMatchedEdepMap[std::make_pair(target_wire.Channel(),range_number)].push_back(i_e);

    }//end loop over target rois

  }//end loop over all edeps

}

std::vector< sys::WireModifier::TruthProperties_t >
sys::WireModifier::CalcPropertiesFromEdeps(std::vector< std::pair<const sim::SimEnergyDeposit*,const sim::SimEnergyDeposit*> > const& edepPtrVec){
  
  std::vector<TruthProperties_t> edep_col_prop_vec;

  TruthProperties_t edep_col_properties;
  
  //do calculations here
  edep_col_properties.x = 0.;
  edep_col_properties.x_rms = 0.;

  double total_charge = 0.0;
  for(auto const& edep_ptrs : edepPtrVec){
    edep_col_properties.x += edep_ptrs.first->X()*edep_ptrs.second->NumElectrons();
    total_charge += edep_ptrs.second->NumElectrons();
  }

  if(total_charge>0.0)
    edep_col_properties.x = edep_col_properties.x/total_charge;

  for(auto const& edep_ptrs : edepPtrVec)
    edep_col_properties.x_rms += (edep_ptrs.first->X()-edep_col_properties.x)*(edep_ptrs.first->X()-edep_col_properties.x)*edep_ptrs.second->NumElectrons();

  if(total_charge>0.0)
    edep_col_properties.x_rms = std::sqrt(edep_col_properties.x_rms/total_charge);


  edep_col_properties.tick = A_t*edep_col_properties.x + C_t;
  edep_col_properties.tick_rms = A_t*edep_col_properties.x_rms;

  edep_col_properties.charge = total_charge;
  
  edep_col_prop_vec.emplace_back(edep_col_properties);
  
  return edep_col_prop_vec;
}

std::vector<sys::WireModifier::ScaleValues_t> 
sys::WireModifier::GetScaleValues(std::vector<TruthProperties_t> const& truth_props_vec, sys::WireModifier::ROIProperties_t const& roi_vals)
{
  std::vector<ScaleValues_t> scales_vec;

  for(auto const& truth_props : truth_props_vec){

    ScaleValues_t scales;

    //get scales here
    if(roi_vals.plane==0){
      scales.r_Q = fTSplines_Charge_X[0]->Eval(truth_props.x);
      scales.r_sigma = fTSplines_Sigma_X[0]->Eval(truth_props.x);
    }
    else if(roi_vals.plane==1){
      scales.r_Q = fTSplines_Charge_X[1]->Eval(truth_props.x);
      scales.r_sigma = fTSplines_Sigma_X[1]->Eval(truth_props.x);
    }
    else if(roi_vals.plane==2){
      scales.r_Q = fTSplines_Charge_X[2]->Eval(truth_props.x);
      scales.r_sigma = fTSplines_Sigma_X[2]->Eval(truth_props.x);
    }
    
    scales_vec.emplace_back(scales);
  }

  return scales_vec;
}

void sys::WireModifier::ModifyROI(std::vector<float> & roi_data,
				  ROIProperties_t const& roi_vals,
				  std::vector<TruthProperties_t> const& truth_props_vec,
				  std::vector<ScaleValues_t> const& scales_vec)
{

  std::vector<double> tick_rms,center_sim,total_q_sim,sigma_sim;
  std::vector<int> tick_window_begin,tick_window_end;
  
  //double total_
  for(size_t i_tp=0; i_tp<truth_props_vec.size(); ++i_tp){
    tick_rms.push_back(std::sqrt(2*2+truth_props_vec[i_tp].tick_rms*truth_props_vec[i_tp].tick_rms));
    tick_window_begin.push_back(std::round(truth_props_vec[i_tp].tick-tick_rms.back())-roi_vals.begin);
    tick_window_end.push_back(std::round(truth_props_vec[i_tp].tick+tick_rms.back()+1)-roi_vals.begin);

    if(tick_window_begin.back()<0) tick_window_begin.back()=0;
    if(tick_window_end.back()>(int)(roi_data.size())) tick_window_end.back()=roi_data.size();
  }



  double center_else=0;
  double total_q_else=0;
  double sigma_else=0;

  for(size_t i_t = 0; i_t<roi_data.size(); ++i_t){

    bool tick_in_sim=false;
    for(size_t i_tp=0; i_tp<truth_props_vec.size(); ++i_tp){

      if((int)i_t >=tick_window_begin[i_tp] && (int)i_t<tick_window_end[i_tp])
	{
	  center_sim[i_tp] += roi_data[i_t]*(i_t+roi_vals.begin);
	  total_q_sim[i_tp] += roi_data[i_t];
	  tick_in_sim=true;
	}
    }
    if(!tick_in_sim){
      center_else += roi_data[i_t]*(i_t+roi_vals.begin);
      total_q_else += roi_data[i_t];
    }
  }

  for(size_t i_tp=0; i_tp<truth_props_vec.size(); ++i_tp)
    center_sim[i_tp] = center_sim[i_tp]/total_q_sim[i_tp];
  center_else = center_else/total_q_else;


  for(size_t i_t = 0; i_t<roi_data.size(); ++i_t){

    bool tick_in_sim=false;
    for(size_t i_tp=0; i_tp<truth_props_vec.size(); ++i_tp){
      if((int)i_t >=tick_window_begin[i_tp] && (int)i_t<tick_window_end[i_tp]){
	sigma_sim[i_tp] += roi_data[i_t]*(i_t+roi_vals.begin-center_sim[i_tp])*(i_t+roi_vals.begin-center_sim[i_tp]);
	tick_in_sim=true;
      }
    }
    if(!tick_in_sim)
      sigma_else += roi_data[i_t]*(i_t+roi_vals.begin-center_else)*(i_t+roi_vals.begin-center_else);
  }
  
  for(size_t i_tp=0; i_tp<truth_props_vec.size(); ++i_tp)
    sigma_sim[i_tp] = std::sqrt(sigma_sim[i_tp]/total_q_sim[i_tp]);
  sigma_else = std::sqrt(sigma_else/total_q_else);

  double scale_ratio=1;
  double q_else=0.0;
  double q_sim_orig=0.0;
  double q_sim_mod=0.0;

  //bool verbose=false;
  //if(roi_data.size()>100) verbose=true;

  for(size_t i_t = 0; i_t<roi_data.size(); ++i_t){

    q_else = total_q_else*GAUSSIAN(i_t+roi_vals.begin,
				   center_else,
				   sigma_else);
    q_sim_orig=0.;
    q_sim_mod=0.;
    for(size_t i_tp=0; i_tp<truth_props_vec.size(); ++i_tp){
      q_sim_orig += total_q_sim[i_tp]*GAUSSIAN(i_t+roi_vals.begin,
					       center_sim[i_tp],
					       sigma_sim[i_tp]);
      
      q_sim_mod += scales_vec[i_tp].r_Q*total_q_sim[i_tp]*GAUSSIAN(i_t+roi_vals.begin,
								   center_sim[i_tp],
								   sigma_sim[i_tp]*scales_vec[i_tp].r_sigma);
    }

    if(isnan(q_else)) q_else=0.0;
    if(isnan(q_sim_orig)) q_sim_orig=0.0;
    if(isnan(q_sim_mod)) q_sim_mod=0.0;
    
    scale_ratio = (q_sim_mod+q_else)/(q_sim_orig+q_else);
    
    if(isnan(scale_ratio) || isinf(scale_ratio))
      scale_ratio = 1.0;
    
    roi_data[i_t] = roi_data[i_t] * scale_ratio;
    /*    
    if(verbose)
      std::cout << "\t\t\t tick " << i_t << ":"
		<< " data=" << roi_data[i_t]
		<< ", den. = "
		<< "( " << total_q_sim*GAUSSIAN(i_t+roi_vals.begin,
						center_sim,
						sigma_sim)
		<< " + " << total_q_else*GAUSSIAN(i_t+roi_vals.begin,
						  center_else,
						  sigma_else) << ")"
		<< ", num. = "
		<< "( " << scales.r_Q*total_q_sim*GAUSSIAN(i_t+roi_vals.begin,
							   center_sim,
							   sigma_sim*scales.r_sigma)
		<< " + " << total_q_else*GAUSSIAN(i_t+roi_vals.begin,
						  center_else,
						  sigma_else) << ")"
		<< ", ratio=" << scale_ratio
		<< std::endl;
    */
  }

  return;
}

sys::WireModifier::WireModifier(fhicl::ParameterSet const& p)
  : EDProducer{p},
  fWireInputTag(p.get<art::InputTag>("WireInputTag")),
  fSimEdepShiftedInputTag(p.get<art::InputTag>("SimEdepShiftedInputTag")),
  fSimEdepOrigInputTag(p.get<art::InputTag>("SimEdepOrigInputTag")),
  fMakeRawDigitAssn(p.get<bool>("MakeRawDigitAssn",true)),
  fCopyRawDigitAssn(p.get<bool>("CopyRawDigitAssn",false)),
  fSplinesFileName(p.get<std::string>("SplinesFileName")),
  fSplineNames_Charge_X(p.get< std::vector<std::string> >("SplineNames_Charge_X")),
  fSplineNames_Sigma_X(p.get< std::vector<std::string> >("SplineNames_Sigma_X"))
  //  ONE_OVER_SQRT_2PI(1./std::sqrt(2*util::pi()))
{
  produces< std::vector< recob::Wire > >();

  if(fMakeRawDigitAssn)
    produces< art::Assns<raw::RawDigit,recob::Wire> >();


  fSplinesFileName = std::string(std::getenv("UBOONEDATA_DIR"))+"/systematics/det_sys/"+fSplinesFileName;
  std::cout << "Spline file is " << fSplinesFileName;

  TFile f_splines(fSplinesFileName.c_str(),"r");

  fTSplines_Charge_X.resize(fSplineNames_Charge_X.size());
  for(size_t i_s=0; i_s<fSplineNames_Charge_X.size(); ++i_s)
    f_splines.GetObject(fSplineNames_Charge_X[i_s].c_str(),fTSplines_Charge_X[i_s]);
  
  fTSplines_Sigma_X.resize(fSplineNames_Sigma_X.size());
  for(size_t i_s=0; i_s<fSplineNames_Sigma_X.size(); ++i_s)
    f_splines.GetObject(fSplineNames_Sigma_X[i_s].c_str(),fTSplines_Sigma_X[i_s]);
}

void sys::WireModifier::produce(art::Event& e)
{

  //get wires
  art::Handle< std::vector<recob::Wire> > wireHandle;
  e.getByLabel(fWireInputTag, wireHandle);
  auto const& wireVec(*wireHandle);

  //get association to rawdigit
  art::FindManyP<raw::RawDigit> digit_assn(wireHandle,e,fWireInputTag);

  //get sim edeps
  art::Handle< std::vector<sim::SimEnergyDeposit> > edepShiftedHandle;
  e.getByLabel(fSimEdepShiftedInputTag,edepShiftedHandle);
  auto const& edepShiftedVec(*edepShiftedHandle);
  
  art::Handle< std::vector<sim::SimEnergyDeposit> > edepOrigHandle;
  e.getByLabel(fSimEdepOrigInputTag,edepOrigHandle);
  auto const& edepOrigVec(*edepOrigHandle);

  //output new wires and new associations
  std::unique_ptr< std::vector<recob::Wire> > new_wires(new std::vector<recob::Wire>());
  std::unique_ptr< art::Assns<raw::RawDigit,recob::Wire> > new_digit_assn(new art::Assns<raw::RawDigit,recob::Wire>());

  //first fill our roi to edep map
  FillROIMatchedEdepMap(edepShiftedVec,wireVec);

  //loop through the wires and rois per wire...
  for(size_t i_w=0; i_w<wireVec.size(); ++i_w){

    auto const& wire = wireVec[i_w];

    //make a new roi list
    recob::Wire::RegionsOfInterest_t new_rois;
    new_rois.resize(wire.SignalROI().size());

    unsigned int my_plane=wire.View();

    //for(auto const& range: wire.SignalROI().get_ranges()){
    for(size_t i_r=0; i_r<wire.SignalROI().get_ranges().size(); ++i_r){


      auto const& range = wire.SignalROI().get_ranges()[i_r];
      ROI_Key_t roi_key(wire.Channel(),i_r);

      //get the matching edeps
      auto it_map = fROIMatchedEdepMap.find(roi_key);
      if(it_map==fROIMatchedEdepMap.end()){
	new_rois.add_range(range.begin_index(),range.data());
	continue;
      }
      std::vector<size_t> matchedEdepIdxVec = it_map->second;      
      if(matchedEdepIdxVec.size()==0){
	new_rois.add_range(range.begin_index(),range.data());
	continue;
      }
      
      //calc roi properties
      auto roi_properties = CalcROIProperties(range);
      roi_properties.plane = my_plane;

      //calc the inputs to properties
      //pair is orig,shifted
      std::vector< std::pair<const sim::SimEnergyDeposit*,const sim::SimEnergyDeposit*> > matchedEdepPtrVec;
      for(auto i_e : matchedEdepIdxVec)
	matchedEdepPtrVec.emplace_back(&edepOrigVec[i_e],&edepShiftedVec[i_e]);
      auto edep_col_properties = CalcPropertiesFromEdeps(matchedEdepPtrVec);

      //get the scaling values
      auto scales = GetScaleValues(edep_col_properties,roi_properties);

      //get modified ROI given scales

      //recob::Wire::RegionsOfInterest_t::datarange_t::vector_t modified_data(range.data());
      std::vector<float> modified_data(range.data());
      ModifyROI(modified_data,roi_properties,edep_col_properties,scales);
      
      new_rois.add_range(roi_properties.begin,modified_data);
      
    }//end loop over rois
    
    //make our new wire object
    new_wires->emplace_back(new_rois,wire.Channel(),wire.View());

    
    //get the associated rawdigit
    if(fCopyRawDigitAssn){
      auto const& rd_ptrs = digit_assn.at(i_w);
      for(auto const& rd_ptr : rd_ptrs)
	util::CreateAssn(*this,e,*new_wires,rd_ptr,*new_digit_assn,new_wires->size()-1);
    }

  }//end loop over wires

  e.put(std::move(new_wires));

  if(fMakeRawDigitAssn)
    e.put(std::move(new_digit_assn));
  
  //get a list of ptrs to the matched edeps for a wire
}
DEFINE_ART_MODULE(sys::WireModifier)
