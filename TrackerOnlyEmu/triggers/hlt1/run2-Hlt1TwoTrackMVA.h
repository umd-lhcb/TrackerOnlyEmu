// Stolen from:
//   https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu/-/blob/master/tuple_processing_chain/emulate_HLT1_cuts.py
// Last Change: Thu Apr 01, 2021 at 01:27 AM +0200

#ifndef _RUN2_HLT1_TWOTRACKMVA_
#define _RUN2_HLT1_TWOTRACKMVA_

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <TMath.h>

using std::cout;
using std::endl;
using std::map;
using std::string;
using std::vector;

vector<vector<int> > combination( int totSize, int combSize,
                                  int headIdx = 0 ) {
  vector<vector<int> > result;

  for ( int head = headIdx; head <= totSize - combSize; head++ ) {
    if ( combSize > 1 ) {
      for ( auto subComb : combination( totSize, combSize - 1, head + 1 ) ) {
        subComb.emplace( subComb.begin(), head );
        result.emplace_back( subComb );
      }
    } else
      result.emplace_back( vector<int>{ head } );
  }

  return result;
}

bool hlt1TwoTrackInputDec( double PT, double P, double TRCHI2DOF,
                           double BPVIPCHI2, double TRGHOSTPROB, int year ) {
  if ( TRCHI2DOF <= 0 ) return false;

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
    cout << "Year: " << year << " not recognized." << endl;
  }
  return false;
}

bool hlt1TwoTrackMVADec( double VDCHI2, double APT, double DOCA, double VCHI2,
                         double BPVETA, double BPVCORRM, double BPVDIRA,
                         double MVA, int year ) {
  // VDCHI2: Vertex distance chi2
  // APT: This is the PT of the vector sum of the particles, unlike SUMPT,
  // which is just the scalar sum of PTs. Remove dummy values from the input to
  // the MVA
  if ( VDCHI2 <= 0 || APT <= 0 ) return false;

  if ( year == 2015 || year == 2017 || year == 2018 ) {
    // VFASPF: Vertex functor as particle functor
    bool selPreVertexing = ( DOCA > 0 && DOCA < 10 ) && APT > 2000;
    bool selCombo        = VCHI2 < 10 && ( BPVETA > 2 && BPVETA < 5 ) &&
                    ( BPVCORRM > 1000 && BPVCORRM < 1000000000 ) &&
                    BPVDIRA > 0 && MVA > 0.95;
    return selPreVertexing && selCombo;
  } else if ( year == 2016 ) {
    bool selPreVertexing = ( DOCA > 0 && DOCA < 10 ) && APT > 2000;
    bool selCombo        = VCHI2 < 10 && ( BPVETA > 2 && BPVETA < 5 ) &&
                    ( BPVCORRM > 100 && BPVCORRM < 1000000000 ) &&
                    BPVDIRA > 0 && MVA > 0.97;
    return selPreVertexing && selCombo;
  } else {
    cout << "Year: " << year << " not recognized." << endl;
  }
  return false;
}

bool hlt1TwoTrackMVATriggerEmu( vector<map<string, double> >& trackSpec,
                                vector<map<string, double> >& combSpec,
                                vector<bool>& trackPassSel, int year ) {
  // This is used to compare reference SUMPT extracted from BDT and sum of PT
  // from two tracks. If the difference is below threshold, we consider them as
  // the same combo.
  double sumPtThresh = 1;  // in MeV

  for ( auto idxSet : combination( trackSpec.size(), 2 ) ) {
    // First check if any 2 tracks pass the per-track selection
    bool   passPerSel = true;
    double trackSumPt, trackSumPx, trackSumPy;

    for ( auto idx : idxSet ) {
      auto track = trackSpec[idx];
      passPerSel = ( passPerSel &&
                     hlt1TwoTrackInputDec(
                         track["PT"], track["P"], track["TRCHI2DOF"],
                         track["BPVIPCHI2"], track["TRGHOSTPROB"], year ) );
      trackSumPt += track["PT"];
      trackSumPx += track["PX"];
      trackSumPy += track["PY"];
    }

    auto trackAPt =
        TMath::Sqrt( trackSumPx * trackSumPx + trackSumPy * trackSumPy );

    if ( passPerSel ) {
      // Now find if these 2 tracks correspond to any set of two-track BDT
      // variables

      for ( auto comb : combSpec ) {
        if ( TMath::Abs( comb["SUMPT"] - trackSumPt ) <= sumPtThresh ) {
          auto passCombSel = hlt1TwoTrackMVADec(
              comb["VDCHI2"], trackAPt, comb["DOCA"], comb["VCHI2"],
              comb["BPVETA"], comb["BPVCORRM"], comb["BPVDIRA"], comb["MVA"],
              year );
          if ( passCombSel ) return true;
        }
      }
    }
  }

  return false;
}

#endif