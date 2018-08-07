/**
 * \file ShowerRecoModuleBase.h
 *
 * \ingroup ModularAlgo
 *
 * \brief Class def header for a class ShowerRecoModuleBase
 *
 * @author cadams
 */

/** \addtogroup ModularAlgo

    @{*/
#ifndef SHOWERRECOMODULEBASE_H
#define SHOWERRECOMODULEBASE_H

#include <iostream>

#include "ubreco/ShowerReco/ShowerReco3D/Base/ShowerRecoTypes.h"
#include "ubreco/ShowerReco/ShowerReco3D/Base/ShowerRecoException.h"

#include "larcore/Geometry/Geometry.h"
#include "lardata/Utilities/GeometryUtilities.h"


// art TOOLS
#include "art/Utilities/ToolMacros.h"
#include "art/Utilities/make_tool.h"
#include "fhiclcpp/ParameterSet.h"

/**
   \class ShowerRecoModuleBase
   User defined class ShowerRecoModuleBase ... these comments are used to generate
   doxygen documentation!
 */
namespace showerreco {

class ShowerRecoModuleBase {

public:

  /// Default destructor
  virtual ~ShowerRecoModuleBase() noexcept = default;

    /**
     * @brief get the name of this module, used in helping organize order of modules and insertion/removal
     * @return name
     */
    std::string name() {return _name;}

    void configure(const fhicl::ParameterSet& pset){};

    /**
     * @brief Virtual function that is overridden in child class, does the reconstruction work
     * @details Override this function in a base class and use it to do the actual reconstruction.  Organize the
     * class however you like, with as many private or non private functions as needed, but this particular
     * function is the one that is called by ShowerRecoAlgModular.
     *
     * @param t proto_shower The set of shower inputs
     * @param t shower The shower that is passed by reference.  Make edits to this object
     */
    virtual void do_reconstruction(const ::protoshower::ProtoShower & proto_shower, Shower_t & shower) = 0;

    /**
     * @brief Verbosity setter function for each Modular Algo
     */
    void setVerbosity(bool on) { _verbose = on; }

    /**
     * @brief Function to initialize the algorithm (such as setting up tree)
     */
    virtual void initialize() {};

    /**
    * @brief allow access to the larlite storage manager
    * @details Pass a pointer to the current storage manager to the reco alg.  Get's called once per event but
    *          it's just passing pointers
    *
    * @param sm pointer to the storage manager
    */


protected:

    std::string _name;

    bool _verbose;

};

} // showerreco

#endif
/** @} */ // end of doxygen group

