// Stolen from:
//   https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu/-/blob/master/tuple_processing_chain/emulate_HLT1_cuts.py

#ifndef _RUN2_HLT1_TWOTRACKMVA_
#define _RUN2_HLT1_TWOTRACKMVA_

#include <iostream>

bool TwoTrackInputDec( double PT, double P, double TRCHI2DOF, double BPVIPCHI2,
                       double TRGHOSTPROB, int year ) {
  if ( year == 2015 ) {
    if ( PT > 500 && P > 5000 && TRCHI2DOF < 2.5 && BPVIPCHI2 > 4.0 )
      return true;
    return false;
  } else if ( year == 2016 || year == 2017 || year == 2018 ) {
    if ( PT > 600 && P > 5000 && TRCHI2DOF < 2.5 && TRGHOSTPROB < 0.2 &&
         BPVIPCHI2 > 4.0 )
      return true;
    return false;
  } else {
    std::cout << "Year: " << year << " not recognized." << std::endl;
  }
  return false;
}

#endif
