/**
 * \file ClassicHitOrderer.h
 *
 * \ingroup Algorithms
 * 
 * \brief Class def header for a class ClassicHitOrderer
 *
 * @author Marco Del Tutto
 */

/** \addtogroup Algorithms

    @{*/

#ifndef ClassicHitOrderer_H
#define ClassicHitOrderer_H

#include <iostream>
#include "../Base/BaseOrdererAlgo.h"
#include "../Base/HitOrdererFactory.h"

namespace cosmictag {
  /**
     \class ClassicHitOrderer
     User custom analysis class made by SHELL_USER_NAME
   */
  class ClassicHitOrderer : public BaseOrdererAlgo {
  
  public:

    /// Default constructor
    ClassicHitOrderer(const std::string name="ClassicHitOrderer");

    /// Default destructor
    virtual ~ClassicHitOrderer(){}

    bool OrderHits(const SimpleCluster&) const;

  protected:

    void _Configure_(const Config_t &pset);
    double _global_qe;         ///< Global QE
  };
  
  /**
     \class flashana::ClassicHitOrdererFactory
  */
  class ClassicHitOrdererFactory : public HitOrdererFactoryBase {
  public:
    /// ctor
    ClassicHitOrdererFactory() { HitOrdererFactory::get().add_factory("ClassicHitOrderer",this); }
    /// dtor
    ~ClassicHitOrdererFactory() {}
    /// creation method
    BaseOrdererAlgo* create(const std::string instance_name) { return new ClassicHitOrderer(instance_name); }
  };
}
#endif

/** @} */ // end of doxygen group 
