// Stolen from:
//   https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu/-/blob/master/tuple_processing_chain/emulate_HLT1_cuts.py
// Last Change: Wed Mar 31, 2021 at 03:17 AM +0200

#ifndef _RUN2_HLT1_GEC_
#define _RUN2_HLT1_GEC_

#include <TRandomGen.h>

// Blindly copied from RD+:
//   This is a number I get from LHCb-PUB-2015-024
const double EFF_CORRECTION = 0.042;

// Set the RNG, but seed it for reproducibility
TRandomMixMax17 ONLINE_TRACK_RECO_EFF( 7 );

bool onlineTrackRecoEffCorr( double nTTHits ) {
  auto randNum = ONLINE_TRACK_RECO_EFF.Uniform( 0, 1 );
  if ( randNum < EFF_CORRECTION || nTTHits < 3 ) return false;
  return true;
}

bool hlt1GEC( double nVeloClusters, double nITClusters, double nOTClusters ) {
  if ( ( nVeloClusters > 50 && nVeloClusters < 6000 ) &&
       ( nITClusters > 50 && nITClusters < 3000 ) &&
       ( nOTClusters > 50 && nOTClusters < 15000 ) )
    return true;
  return false;
}

bool hlt1GlobalPass( double nTTHits, double nVeloClusters, double nITClusters,
                     double nOTClusters ) {
  if ( onlineTrackRecoEffCorr( nTTHits ) &&
       hlt1GEC( nVeloClusters, nITClusters, nOTClusters ) )
    return true;
  return false;
}

#endif
