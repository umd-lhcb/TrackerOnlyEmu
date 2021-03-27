// Stolen from:
//   https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu/-/blob/master/tuple_processing_chain/emulate_HLT1_cuts.py

#ifndef _PRESEL_RUN2_EFF_TRACK_
#define _PRESEL_RUN2_EFF_TRACK_

#include <TRandomGen.h>

// Blindly copied from RD+:
//   This is a number I get from LHCb-PUB-2015-024
const double EFF_CORRECTION = 0.042;

// Set the RNG, bug seed it for reproducibility
TRandomMixMax17 EffTrackReco( 7 );

bool EffTrackRecoDec( double nTTHits ) {
  auto RandNum = EffTrackReco.Uniform( 0, 1 );
  if ( RandNum < EFF_CORRECTION || nTTHits < 3 ) return false;
  return true;
}

#endif
