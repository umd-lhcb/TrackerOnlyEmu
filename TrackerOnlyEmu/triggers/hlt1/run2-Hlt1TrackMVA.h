// Stolen from:
//   https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu/-/blob/master/tuple_processing_chain/emulate_HLT1_cuts.py
// Last Change: Fri Apr 02, 2021 at 12:51 AM +0200
// Description: Hlt1TrackMVA offline emulation

#ifndef _RUN2_HLT1_TRACKMVA_
#define _RUN2_HLT1_TRACKMVA_

#include <iostream>

#include <TMath.h>

using std::cout;
using std::endl;

//////////////////
// Hlt1TrackMVA //
//////////////////

bool trackMVAVal( double BPVIPCHI2, double PT, double param1, double param2,
                  double param3, double MAXPT = 25000 ) {
  return TMath::Log( BPVIPCHI2 ) >
         param1 / TMath::Power( ( PT / 1000 - param2 ), 2 ) +
             ( param3 / MAXPT ) * ( MAXPT - PT ) + TMath::Log( 7.4 );
}

bool hlt1TrackInputDec( double PT, double P, double TRCHI2DOF,
                        double TRGHOSTPROB, int year ) {
  if ( year == 2015 ) {
    if ( PT > 500 && P > 3000 && TRCHI2DOF < 4 ) return true;
    return false;
  } else if ( year == 2016 || year == 2017 || year == 2018 ) {
    if ( PT > 600 && P > 5000 && TRCHI2DOF < 4 && TRGHOSTPROB < 0.999 )
      return true;
    return false;
  } else {
    cout << "Year: " << year << " not recognized." << endl;
  }

  return false;
}

bool hlt1TrackMVADec( double PT, double P, double TRCHI2DOF, double BPVIPCHI2,
                      double TRGHOSTPROB, int year ) {
  if ( TRCHI2DOF <= 0 || BPVIPCHI2 <= 0 ) return false;

  if ( year == 2015 ) {
    if ( TRCHI2DOF >= 2.5 ) return false;
    if ( ( PT > 25000 && BPVIPCHI2 > 7.4 ) ||
         ( ( PT > 1000 && PT < 25000 ) &&
           trackMVAVal( BPVIPCHI2, PT, 1.0, 1.0, 1.1 ) ) )
      return true;
    return false;
  } else if ( year == 2016 ) {
    if ( TRCHI2DOF >= 2.5 || TRGHOSTPROB >= 0.2 ) return false;
    if ( ( PT > 25000 && BPVIPCHI2 > 7.4 ) ||
         ( ( PT > 1000 && PT < 25000 ) &&
           trackMVAVal( BPVIPCHI2, PT, 1.0, 1.0, 2.3 ) ) )
      return true;
    return false;
  } else if ( year == 2017 || 2018 ) {
    if ( TRCHI2DOF >= 2.5 || TRGHOSTPROB >= 0.2 ) return false;
    if ( ( PT > 25000 && BPVIPCHI2 > 7.4 ) ||
         ( ( PT > 1000 && PT < 25000 ) &&
           trackMVAVal( BPVIPCHI2, PT, 1.0, 1.0, 1.1 ) ) )
      return true;
    return false;
  } else {
    cout << "Year: " << year << " not recognized." << endl;
  }

  return false;
}

bool hlt1TrackMVATriggerEmu( double PT, double P, double TRCHI2DOF,
                             double BPVIPCHI2, double TRGHOSTPROB, int year ) {
  if ( hlt1TrackInputDec( PT, P, TRCHI2DOF, TRGHOSTPROB, year ) ) {
    return hlt1TrackMVADec( PT, P, TRCHI2DOF, BPVIPCHI2, TRGHOSTPROB, year );
  }
  return false;
}

#endif
