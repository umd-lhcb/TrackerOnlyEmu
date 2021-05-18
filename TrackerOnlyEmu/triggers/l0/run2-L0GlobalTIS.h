// Stolen from:
//   https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu/-/blob/master/tuple_processing_chain/emulate_L0Hadron_TOS_RLc.py
// Last Change: Wed May 19, 2021 at 12:27 AM +0200
//
#ifndef _RUN2_L0_HADRON_
#define _RUN2_L0_HADRON_

#include <map>

#include <TFile.h>
#include <TH2F.h>
#include <TMath.h>
#include <TString.h>

using std::map;

///////////////////
// Configurables //
///////////////////

const map<int, TString> YEAR_HISTO_REL = {
    { 2015, "0" },
    { 2016, "1" },
    { 2017, "1" },  // FIXME: We don't have TIS response for 2017 and 2018
    { 2018, "1" },
};

const TString RESP_HISTO_PREFIX = "Jpsi_data_eff";

//////////////////
// L0Global TIS //
//////////////////

map<int, TH2F*> readL0GlobalTisResp( TFile* ntp ) {
  map<int, TH2F*> resp;

  for ( auto const& m : YEAR_HISTO_REL ) {
    auto histo_name = RESP_HISTO_PREFIX + m.second;
    resp[m.first]   = static_cast<TH2F*>( ntp->Get( histo_name ) );
  }

  return resp;
}

// This is emulated as a weight in float
float l0GlobalTisTriggerEmu( double PZ, double PT, int year,
                             map<int, TH2F*> respHistos ) {
  auto hist = respHistos[year];

  if ( PZ > 0 ) {
    auto binPZ = hist->GetXaxis()->FindBin( TMath::Log( PZ ) );
    auto binPT = hist->GetYaxis()->FindBin( TMath::Log( PT ) );
    return hist->GetBinContent( binPZ, binPT );
  }

  return 0;
}

#endif
