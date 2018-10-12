#include "CombineHarvester/CombineTools/interface/ValidationTools.h"
#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <fstream>
#include <map>
#include "boost/format.hpp"
#include "RooFitResult.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooAbsReal.h"
#include "RooAbsData.h"
#include "CombineHarvester/CombineTools/interface/CombineHarvester.h"

namespace ch {
using json = nlohmann::json;

void PrintSystematic(ch::Systematic *syst){
  std::cout<<"Systematic "<<syst->name()<<" for process "<<syst->process()<<" in region "<<syst->bin()<<" :";
}

void PrintProc(ch::Process *proc){
  std::cout<<"Process "<<proc->process()<<" in region "<<proc->bin()<<" :";
}



void ValidateShapeUncertaintyDirection(CombineHarvester& cb, json& jsobj){
  cb.ForEachSyst([&](ch::Systematic *sys){
    if(sys->type()=="shape" && ( (sys->value_u() > 1. && sys->value_d() > 1.) || (sys->value_u() < 1. && sys->value_d() < 1.))){
      jsobj["uncertVarySameDirect"][sys->name()][sys->bin()][sys->process()]={{"value_u",sys->value_u()},{"value_d",sys->value_d()}};
    }
  });
}

void ValidateShapeUncertaintyDirection(CombineHarvester& cb){
  cb.ForEachSyst([&](ch::Systematic *sys){
    if(sys->type()=="shape" && ( (sys->value_u() > 1. && sys->value_d() > 1.) || (sys->value_u() < 1. && sys->value_d() < 1.))){
      PrintSystematic(sys);
      std::cout<<" Up/Down normalisations go in the same direction: up variation: "<<sys->value_u()<<", down variation: "<<sys->value_d()<<std::endl;
    }
  });
}

void CheckEmptyShapes(CombineHarvester& cb, json& jsobj){
  std::vector<ch::Process*> empty_procs;
  auto bins = cb.bin_set();
  cb.ForEachProc([&](ch::Process *proc){
    if(proc->rate()==0.){
      empty_procs.push_back(proc); 
      if (jsobj["emptyProcessShape"][proc->bin()] !=NULL){
        jsobj["emptyProcessShape"][proc->bin()].push_back(proc->process());
      } else {
        jsobj["emptyProcessShape"][proc->bin()] = {proc->process()};
      }
   }
  });
  cb.ForEachSyst([&](ch::Systematic *sys){
    bool no_check=0;
    for( unsigned int i=0; i< empty_procs.size(); i++){
      if ( MatchingProcess(*sys,*empty_procs.at(i)) ) no_check=1;
    }
    if(!no_check){
      if(sys->type()=="shape" &&  (sys->value_u()==0. or sys->value_d()==0.)){
        jsobj["emptySystematicShape"][sys->name()][sys->bin()][sys->process()]={{"value_u",sys->value_u()},{"value_d",sys->value_d()}};
      }
    }
  });
}

void CheckEmptyShapes(CombineHarvester& cb){
  std::vector<ch::Process*> empty_procs;
  cb.ForEachProc([&](ch::Process *proc){
    if(proc->rate()==0.){
      empty_procs.push_back(proc); 
      PrintProc(proc);
      std::cout<<" has 0 yield"<<std::endl;
   }
  });
  cb.ForEachSyst([&](ch::Systematic *sys){
    bool no_check=0;
    for( unsigned int i=0; i< empty_procs.size(); i++){
      if ( MatchingProcess(*sys,*empty_procs.at(i)) ) no_check=1;
    }
    if(!no_check){
      if(sys->type()=="shape" &&  (sys->value_u()==0. or sys->value_d()==0.)){
        PrintSystematic(sys);
        std::cout<<" At least one empty histogram: up variation: "<<sys->value_u()<<" Down variation: "<<sys->value_d()<<std::endl;
      }
    }
  });
}

void ValidateCards(CombineHarvester& cb, std::string const& filename){
 json output_js; 
 ValidateShapeUncertaintyDirection(cb, output_js);      
 CheckEmptyShapes(cb, output_js);      
 std::ofstream outfile(filename);
 outfile <<std::setw(4)<<output_js<<std::endl;
}

}
