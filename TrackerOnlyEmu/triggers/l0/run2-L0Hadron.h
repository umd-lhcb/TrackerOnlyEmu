// Stolen from:
//   https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu/-/blob/master/tuple_processing_chain/emulate_L0Hadron_TOS_RLc.py
// Last Change: Mon Apr 19, 2021 at 12:58 AM +0200
//
#ifndef _RUN2_L0_HADRON_
#define _RUN2_L0_HADRON_

#include <vector>

#include <TFile.h>
#include <TH1D.h>
#include <TRandom3.h>
#include <TString.h>

using std::vector;

///////////////////
// Configurables //
///////////////////

const int P_BIN  = 10;
const int PT_BIN = 6;

const double P_LOW   = 0;
const double P_HIGH  = 1e5;
const double PT_LOW  = 0;
const double PT_HIGH = 15000;

///////////////////////////////////
// Single particle HCAL response //
///////////////////////////////////

vector<vector<TH1D*> > readSinglePartResp( TFile* ntp ) {
  vector<vector<TH1D*> > hcalResp;

  for ( auto i = 0; i < P_BIN; i++ ) {
    vector<TH1D*> rowResp;
    for ( auto j = 0; j < PT_BIN; j++ ) {
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

int computeRespBin( double x, double xLow, double xHigh, int numBins ) {
  auto bin = static_cast<int>( floor( x / ( xHigh - xLow ) * numBins ) );
  if ( bin < 0 ) bin = 0;
  if ( bin >= numBins ) bin = numBins - 1;
  return bin;
}

// Reimplementation of 'random_smearing'
double singlePartEtSmear( double P, double PT, double realET,
                          vector<vector<TH1D*> > respHistos ) {
  auto binP  = computeRespBin( P, P_LOW, P_HIGH, P_BIN );
  auto binPT = computeRespBin( PT, PT_LOW, PT_HIGH, PT_BIN );
  auto hist  = respHistos[binP][binPT];

  double smearFactor = hist->GetRandom();
  double smearedET   = realET * ( 1 - smearFactor );
  if ( smearedET > 6100 ) smearedET = 6100;  // Due to limitation of HCAL

  return smearedET;
}

#endif
