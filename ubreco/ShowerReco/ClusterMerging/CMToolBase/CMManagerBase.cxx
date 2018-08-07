#ifndef RECOTOOL_CMMANAGERBASE_CXX
#define RECOTOOL_CMMANAGERBASE_CXX

#include "CMManagerBase.h"

namespace clusmtool {

  CMManagerBase::CMManagerBase()
  {
    _fout = 0;
    _debug_mode = kNone;
    //_priority_algo = nullptr;
    _min_nhits = 1;
    _merge_till_converge = false;
    Reset();
    _time_report=false;
  }

  void CMManagerBase::Reset()
  {
    _planes.clear();
    _in_clusters.clear();
    if(_priority_algo) _priority_algo->Reset();
  }

  void CMManagerBase::SetClusters(const std::vector<std::vector<cluster::pt> > &clusters)
  {

    TStopwatch localWatch;

    // Reset
    this->Reset();

    // Clear & fill cluster info

    _in_clusters.clear();

    _in_clusters.reserve(clusters.size());

    for(auto const &c : clusters) {

      ::cluster::Cluster thisclus;
      thisclus.SetHits(c);
      
      _in_clusters.push_back(thisclus);//::cluster::Cluster());
      
      //// Set the hits for this cluster params
      //if((*_in_clusters.rbegin()).SetHits(c) < _min_nhits) continue;

      // Use the params alg to fill the Cluster
      //_params_alg.FillParams(_in_clusters.back());

    }

    if(_time_report) {
      std::cout << Form("  CMManagerBase Time Report: SetClusters (CPAN computation) = %g [s]",
                        localWatch.RealTime())
                << " ... details below." << std::endl;

      // for(auto const& c : _in_clusters)
      //   c.ReportTimes();

    }
    
  }

  void CMManagerBase::SetClusters(const std::vector<::cluster::Cluster> &clusters)
  {
    TStopwatch localWatch;

    localWatch.Start();

    _in_clusters = clusters;

    if(_time_report) std::cout << Form("  CMManagerBase Time Report: SetClusters (copy) = %g [s]",
                                       localWatch.RealTime())
                               << std::endl;
  }

  void CMManagerBase::SetClusters(std::vector<::cluster::Cluster>&& clusters)
  {
    std::swap(_in_clusters,clusters);
  }

  void CMManagerBase::Process()
  {
    
    if(!(_in_clusters.size()))

      return;

    TStopwatch localWatch;

    localWatch.Start();

    EventBegin();

    if(_time_report) std::cout << Form("  CMManagerBase Time Report: EventBegin = %g [s]",
				       localWatch.RealTime())
                               << std::endl;
    
    bool keep_going = true;
    
    while(keep_going) {
      
      localWatch.Start();

      IterationBegin();

      if(_time_report) std::cout << Form("  CMManagerBase Time Report: IterationBegin = %g [s]",
                                       localWatch.RealTime())
                                 << std::endl;

      localWatch.Start();
      
      keep_going = IterationProcess();

      if(_time_report) std::cout << Form("  CMManagerBase Time Report: IterationProcess = %g [s]",
                                         localWatch.RealTime())
                                 << std::endl;
      localWatch.Start();

      IterationEnd();

      if(_time_report) std::cout << Form("  CMManagerBase Time Report: IterationEnd = %g [s]",
                                         localWatch.RealTime())
                                 << std::endl;

      if(!_merge_till_converge) {

        if(_debug_mode <= kPerIteration)

          std::cout << "\033[93m  Iterative approach = OFF ... exiting from iteration loop. \033[00m" << std::endl;
        
        break;
      }
    }

    localWatch.Start();

    EventEnd();

    if(_time_report) std::cout << Form("  CMManagerBase Time Report: EventEnd = %g [s]",
                                       localWatch.RealTime())
                               << std::endl;
    
  }

  void CMManagerBase::ComputePriority(const std::vector<::cluster::Cluster> &clusters) {

    TStopwatch localWatch;
    localWatch.Start();

    _priority.clear();
    _planes.clear();

    // verbosity to debug mysterious call of priority algo
    /*
      std::cout << "\t\t DD Calling Priority ALGO : " << std::endl;
      std::cout << "\t\t DD " << typeid(*(_priority_algo.get())).name() << std::endl;    
      std::cout << "\t\t DD " << _priority_algo->Name() 
		<< " for " << clusters.size() << " clusters" 
		<< std::endl;
    */

    if(!clusters.size()) return;

    // Priority is computed cluster-by-cluster. In case of two clusters having the same priority 
    // value the one with lower cluster index gets the priority. Also, clusters with priority < 0
    // are not logged (assumed not to be used)
    
    for(size_t i=0; i<clusters.size(); ++i) {

      size_t c_index = clusters.size() - i - 1;
      float priority = clusters.at(c_index).size();

      if(_priority_algo) {
	priority = _priority_algo->Priority(clusters.at(c_index));
      }

      if(priority>0) {
        _priority.insert(std::make_pair(priority,c_index));
	
        if( _planes.find(clusters.at(c_index)._plane) == _planes.end() )
	  
          _planes.insert(clusters.at(c_index)._plane);
	
      }
      
    }
    
    if(_time_report) std::cout << Form("  CMManagerBase Time Report: ComputePriority = %g [s]",
                                       localWatch.RealTime())
                               << std::endl;

  }


}

#endif
