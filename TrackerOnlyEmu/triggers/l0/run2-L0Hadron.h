// Stolen from:
//   https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu/-/blob/master/tuple_processing_chain/emulate_L0Hadron_TOS_RLc.py
// Last Change: Tue May 18, 2021 at 12:44 AM +0200
//
#ifndef _RUN2_L0_HADRON_
#define _RUN2_L0_HADRON_

#include <vector>

#include <TFile.h>
#include <TH1D.h>
#include <TMath.h>
#include <TRandom3.h>
#include <TString.h>

using std::vector;

///////////////////
// Configurables //
///////////////////

const int P_BIN        = 10;
const int PT_BIN       = 6;
const int TOT_FRAC_BIN = 10;

const double P_LOW   = 0;
const double P_HIGH  = 1e5;
const double PT_LOW  = 0;
const double PT_HIGH = 15000;

TRandom3 SHARED_EFF = TRandom3( 41 );

///////////////////////////////////
// Single particle HCAL response //
///////////////////////////////////

double capHcalResp( double ET ) {
  if ( ET > 6100 ) return 6100;
  return ET;
}

double capHcalResp( double ET1, double ET2 ) {
  double ET = TMath::Max( ET1, ET2 );
  return capHcalResp( ET );
}

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
double singlePartEt( double P, double PT, double realET,
                     vector<vector<TH1D*> > respHistos ) {
  auto binP  = computeRespBin( P, P_LOW, P_HIGH, P_BIN );
  auto binPT = computeRespBin( PT, PT_LOW, PT_HIGH, PT_BIN );
  auto hist  = respHistos[binP][binPT];

  double smearFactor = hist->GetRandom();
  double smearedET   = realET * ( 1 - smearFactor );
  if ( smearedET < 0 ) smearedET = 0;
  if ( smearedET > 6100 ) smearedET = 6100;  // Due to limitation of HCAL

  return smearedET;
}

//////////////////////////////
// Two particle corrections //
//////////////////////////////

double rDiff( double x1, double y1, double x2, double y2 ) {
  return TMath::Sqrt( TMath::Power( x1 - x2, 2 ) +
                      TMath::Power( y1 - y2, 2 ) );
}

bool isShared( double rDiff, int region1, int region2, TH1D* histoSharedInner,
               TH1D* histoSharedOuter ) {
  if ( region1 != region2 ) return false;

  TH1D* histo = histoSharedOuter;
  if ( region1 == 1 ) histo = histoSharedInner;

  double fracShared = histo->GetBinContent( histo->FindBin( rDiff ) );
  if ( SHARED_EFF.Uniform() < fracShared ) return true;
  return false;
}

double missingFraction( double rDiff, int region1, int region2,
                        TH1D* histoMissingInner, TH1D* histoMissingOuter ) {
  if ( region1 != region2 ) return 0;

  TH1D* histo = histoMissingOuter;
  if ( region1 == 1 ) histo = histoMissingInner;

  auto   bin      = histo->FindBin( rDiff );
  auto   totFrac  = histo->GetBinContent( TOT_FRAC_BIN );
  double missFrac = histo->GetBinContent( bin ) / totFrac;
  return missFrac;
}

// Reimplementation of 'calcET' and 'calcDplusET'
double twoPartEt( double smearedET1, double smearedET2, bool isShared,
                  double missFrac ) {
  double ET = smearedET1;
  if ( isShared ) ET = ( ET + smearedET2 ) * missFrac;
  if ( ET < 0 ) ET = 0;
  if ( ET > 6100 ) ET = 6100;
  return ET;
}

//////////////
// L0Hadron //
//////////////

bool l0HadronTriggerEmu( double ET, int year ) {
  double triggerThresh = 3600;
  if ( year == 2016 ) triggerThresh = 3744;

  return ET > triggerThresh;
}

#endif
