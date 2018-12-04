#ifndef SHOWERRECOTYPE_H
#define SHOWERRECOTYPE_H

#include <TVector3.h>
#include <vector>

#include "ubreco/ShowerReco/ShowerReco3D/ProtoShower/ProtoShower.h"

#include "larcore/Geometry/Geometry.h"
#include "larcorealg/Geometry/GeometryCore.h"

namespace showerreco {


/// Utility: maximum value for double
const double kDOUBLE_MAX = std::numeric_limits<double>::max();

/// Utility: minimum value for double
const double kDOUBLE_MIN = std::numeric_limits<double>::min();

/// Utility: maximum value for int
const int    kINT_MAX    = std::numeric_limits<int>::max();

/// Utility: maximum value for unsigned int
const unsigned int kUINT_MAX    = std::numeric_limits<unsigned int>::max();

/// Utility: maximum value for size_t
const size_t kSIZE_MAX   = std::numeric_limits<size_t>::max();


/// Output shower representation from shower reco algorithms
struct Shower_t {

  bool fPassedReconstruction;

  TVector3 fDCosStart;                        ///< direction cosines at start of shower
  TVector3 fSigmaDCosStart;                   ///< uncertainty on initial direction cosines
  TVector3 fXYZStart;                         ///< 3D start point of shower
  TVector3 fSigmaXYZStart;                    ///< uncertainty on 3D start point
  TVector3 fCentroid;                         ///< 3D centroid of shower
  TVector3 fSigmaCentroid;                    ///< uncertainty on 3D centroid
  double   fLength;                           ///< 3D length of a shower
  double   fWidth[2];                         ///< 3D width of a shower (2 directions)
  double   fOpeningAngle;                     ///< 3D opening angle of a shower

  double fTotalEnergy;                        ///< Calculated Energy
  double fSigmaTotalEnergy;                   ///< Calculated Energy uncertainty
  std::vector< double > fTotalEnergy_v;       ///< Calculated Energy per each plane
  std::vector< double > fSigmaTotalEnergy_v;  ///< Calculated Energy per each plane
  std::vector< double > fTotalMIPEnergy_v;    ///< Calculated Energy per each plane
  std::vector< double > fSigmaTotalMIPEnergy_v;///< Calculated Energy per each plane

  double fdEdx;                               ///< Calculated dEdx
  double fSigmadEdx;                          ///< Calculated dEdx uncertainty
  std::vector< std::vector<double> > fdEdx_v_v;///< Calculated dEdx entries, segment-by-segment, per each plane
  std::vector< double > fdEdx_v;              ///< Calculated dEdx per each plane
  double fdEdx_0;              ///< Calculated dEdx plane0
  double fdEdx_1;              ///< Calculated dEdx plane1
  double fdEdx_2;              ///< Calculated dEdx plane2
  std::vector< double > fSigmadEdx_v;         ///< Calculated dEdx per each plane

  double fdEdxBox_0;              ///< Calculated dEdx plane0
  double fdEdxBox_1;              ///< Calculated dEdx plane1
  double fdEdxBox_2;              ///< Calculated dEdx plane2

  double fdQdx;                               ///< Calculated dQdx [fC/cm]
  double fSigmadQdx;                          ///< Calculated dWdx uncertainty [fC/cm]
  std::vector< double > fdQdx_v;              ///< Calculated dQdx per each plane [fC/cm]
  std::vector< double > fSigmadQdx_v;         ///< Calculated dQdx per each plane
  size_t fBestdEdxPlane;                      ///< Best plane for dQdx calculation
  std::vector< std::vector < double > > fHitdQdx_v;      ///< Hit-by-hit dQdx per each plane [ADC/cm]
  double fBestdEdx;                           ///< Selects dQdx with the longest ShoweringLength [ADC/cm]

  double fPitch_0, fPitch_1, fPitch_2;
  double fClStart_t_0, fClStart_t_1, fClStart_t_2;
  double fClStart_w_0, fClStart_w_1, fClStart_w_2;
  double fClEnd_t_0, fClEnd_t_1, fClEnd_t_2;
  double fClEnd_w_0, fClEnd_w_1, fClEnd_w_2;

  std::vector< double > fShoweringLength;     ///< Calculates the distance from start to shower points [in cm]

  ::geo::PlaneID fBestPlane;         ///< "Best" plane used for geometrical interpretation

  std::vector< ::geo::PlaneID > fPlaneIDs;    ///< List of PlaneIDs in the order aligned w.r.t. other vectors

  std::vector< bool > fPlaneIsBad;            ///< Matches number of planes, gets flagged if a plane is bad


  Shower_t()
  { Reset(); }

  void Reset() {

    fPassedReconstruction = false;

    fDCosStart[0] = fDCosStart[1] = fDCosStart[2] = kDOUBLE_MIN;
    fSigmaDCosStart[0] = fSigmaDCosStart[1] = fSigmaDCosStart[2] = kDOUBLE_MIN;

    fXYZStart[0] = fXYZStart[1] = fXYZStart[2] = kDOUBLE_MIN;
    fSigmaXYZStart[0] = fSigmaXYZStart[1] = fSigmaXYZStart[2] = kDOUBLE_MIN;

    fCentroid[0] = fCentroid[1] = fCentroid[2] = kDOUBLE_MIN;
    fSigmaCentroid[0] = fSigmaCentroid[1] = fSigmaCentroid[2] = kDOUBLE_MIN;

    fLength = kDOUBLE_MIN;
    fWidth[0] = fWidth[1] = kDOUBLE_MIN;
    fOpeningAngle = 0;

    fTotalEnergy = kDOUBLE_MIN;
    fSigmaTotalEnergy = kDOUBLE_MIN;
    fTotalEnergy_v.clear();
    fTotalEnergy_v = {kDOUBLE_MIN, kDOUBLE_MIN, kDOUBLE_MIN};
    fSigmaTotalEnergy_v.clear();
    fdEdx = kDOUBLE_MIN;
    fSigmadEdx = kDOUBLE_MIN;
    fdEdx_v_v.clear();
    fdEdx_0 = kDOUBLE_MIN;
    fdEdx_1 = kDOUBLE_MIN;
    fdEdx_2 = kDOUBLE_MIN;
    fdEdx_v_v.resize(3);
    fdEdx_v.clear();
    fdEdx_v = {kDOUBLE_MIN, kDOUBLE_MIN, kDOUBLE_MIN};
    fdQdx = kDOUBLE_MIN;
    fSigmadQdx = kDOUBLE_MIN;
    fdQdx_v.clear();
    fdQdx_v = {kDOUBLE_MIN, kDOUBLE_MIN, kDOUBLE_MIN};

    fdEdxBox_0 = kDOUBLE_MIN;
    fdEdxBox_1 = kDOUBLE_MIN;
    fdEdxBox_2 = kDOUBLE_MIN;

    fPitch_0 = kDOUBLE_MIN;
    fPitch_1 = kDOUBLE_MIN;
    fPitch_2 = kDOUBLE_MIN;
    fClStart_t_0 = kDOUBLE_MIN;
    fClStart_t_1 = kDOUBLE_MIN;
    fClStart_t_2 = kDOUBLE_MIN;
    fClStart_w_0 = kDOUBLE_MIN;
    fClStart_w_1 = kDOUBLE_MIN;
    fClStart_w_2 = kDOUBLE_MIN;
    fClEnd_t_0 = kDOUBLE_MIN;
    fClEnd_t_1 = kDOUBLE_MIN;
    fClEnd_t_2 = kDOUBLE_MIN;
    fClEnd_w_0 = kDOUBLE_MIN;
    fClEnd_w_1 = kDOUBLE_MIN;
    fClEnd_w_2 = kDOUBLE_MIN;

    fShoweringLength.clear();
    fSigmadEdx_v.clear();
    fTotalMIPEnergy_v.clear();
    fSigmaTotalMIPEnergy_v.clear();

    fBestPlane = ::geo::PlaneID();
    fPlaneIDs.clear();
  }
}; // end of shower struct

}
#endif
