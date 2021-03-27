// Stolen from:
//   https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu/-/blob/master/tuple_processing_chain/emulate_HLT1_cuts.py

#ifndef _RUN2_HLT1_TWOTRACKMVA_
#define _RUN2_HLT1_TWOTRACKMVA_

#include <iostream>

bool Hlt1TwoTrackInputDec( double PT, double P, double TRCHI2DOF,
                           double BPVIPCHI2, double TRGHOSTPROB, int year ) {
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

bool Hlt1TwoTrackMVADec( double VDCHI2, double SUMPT, double VCHI2,
                         double BPVETA, double BPVCORRM, double BPVDIRA,
                         double MVA, int year ) {
  // VDCHI2: Vertex distance chi2
  // Mindlessly copied from RD+ script:
  //   Remove dummy values from the input to the MVA
  if ( VDCHI2 <= 0 || SUMPT <= 0 ) return false;
  if ( year == 2015 || year == 2017 || year == 2018 ) {
    // VFASPF: Vertex functor as particle functor
    if ( VCHI2 < 10 && ( BPVETA > 2 && BPVETA < 5 ) &&
         ( BPVCORRM > 1000 && BPVCORRM < 1000000000 ) && BPVDIRA > 0 &&
         MVA > 0.95 )
      return true;
    return false;
  } else if ( year == 2016 ) {
    if ( VCHI2 < 10 && ( BPVETA > 2 && BPVETA < 5 ) &&
         ( BPVCORRM > 100 && BPVCORRM < 1000000000 ) && BPVDIRA > 0 &&
         MVA > 0.97 )
      return true;
    return false;
  } else {
    std::cout << "Year: " << year << " not recognized." << std::endl;
  }
  return false;
}

bool Hlt1TwoTrackMVATriggerEmu() { return false; }

#endif
