// Stolen from:
//   https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu/-/blob/master/tuple_processing_chain/emulate_L0Hadron_TOS_RLc.py
// Last Change: Sun Apr 18, 2021 at 11:41 PM +0200
//
#ifndef _RUN2_L0_HADRON_
#define _RUN2_L0_HADRON_

#include <vector>

#include <TFile.h>
#include <TH1D.h>
#include <TString.h>

using std::vector;

vector<vector<TH1D*> > readSinglePartResp( TFile* ntp, int momBinsOut = 10,
                                           int momBinsIn = 6 ) {
  vector<vector<TH1D*> > hcalResp;

  for ( auto i = 0; i < momBinsOut; i++ ) {
    vector<TH1D*> rowResp;
    for ( auto j = 0; j < momBinsIn; j++ ) {
      TString histName = "hdiff_";
      histName += i;
      histName += "_";
      histName += j;
      rowResp.push_back( static_cast<TH1D*>( ntp->Get( histName ) ) );
    }
    hcalResp.push_back( rowResp );
  }

  return hcalResp;
}

#endif
