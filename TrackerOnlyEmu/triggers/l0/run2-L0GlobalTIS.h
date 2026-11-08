// Stolen from:
//   https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu/-/blob/master/tuple_processing_chain/emulate_L0Hadron_TOS_RLc.py
// Last Change: Wed May 19, 2021 at 12:27 AM +0200
//
#ifndef _RUN2_L0_HADRON_
#define _RUN2_L0_HADRON_

#include <map>
#include <cmath>
#include <stdexcept>
#include <TH1.h>

#include <TFile.h>
#include <TH2F.h>
#include <TMath.h>
#include <TString.h>

using std::map;
using std::vector;

///////////////////
// Configurables //
///////////////////

const map<int, TString> YEAR_HISTO_REL = {
    // { 2015, "0" },
    { 2016, "0" },
    { 2017, "1" },
    { 2018, "2" },
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

TH1* readL0NTracksCorrection(const char* path) {
  TFile file(path, "READ");
  if (file.IsZombie()) throw std::runtime_error("Cannot open nTracks correction file");
  auto input = dynamic_cast<TH1*>(file.Get("correction"));
  if (!input || input->GetDimension() != 1)
    throw std::runtime_error("Expected a one-dimensional correction histogram");
  for (int bin = 0; bin <= input->GetNbinsX() + 1; ++bin) {
    double value = input->GetBinContent(bin);
    if (!std::isfinite(value) || value < 0)
      throw std::runtime_error("Invalid nTracks correction bin content");
  }
  auto hist = static_cast<TH1*>(input->Clone());
  hist->SetDirectory(nullptr);
  return hist;
}

// Used for both correction derived from FS Dstmunu (TIS boolean over measured eff) and nspdhits<450 eff correction
// Interpolate between bin centers; hold endpoint values outside that range
double l0NTracksCorrection(double nTracks, const TH1* hist) {
  if (!std::isfinite(nTracks)) throw std::runtime_error("Nonfinite nTracks");

  const auto axis = hist->GetXaxis();
  const double first = axis->GetBinCenter(1);
  const double last = axis->GetBinCenter(hist->GetNbinsX());

  if (nTracks <= first) return hist->GetBinContent(1);
  if (nTracks >= last) return hist->GetBinContent(hist->GetNbinsX());

  return hist->Interpolate(nTracks);
}

// returns a float- an eff/prob for passing B L0Global TIS, as measured using TISTOS method in JpsiK data
float l0GlobalTisTriggerEmu( double PZ, double PT, int year,
                             map<int, TH2F*> respHistos, bool adhoc_correction ) {
  auto hist = respHistos[year];

  // ad-hoc correction to mimic high log(pT) behavior seen in rdx fullsim MC, found in lhcb-ntuples-gen/scripts/l0_global_tis_highpT_adhoc_correction.py using D*+munu fullsim MC
  // assumes measurement was made in 4 log(pz) bins (with bin boundaries as in the response histos)
  map<int, map<int, vector<float>>> ADHOC_DSTMUNU_HIGHPT_CORRECTION;
  ADHOC_DSTMUNU_HIGHPT_CORRECTION[2016][1] = {9.555505392236217, 0.36532145738601685, 0.052622689128916246}; // D*munu high log(pT) bin mean, eff from JpsiK data, slope correction
  ADHOC_DSTMUNU_HIGHPT_CORRECTION[2016][2] = {9.62712222725513, 0.3874987065792084, 0.16081922872501483};
  ADHOC_DSTMUNU_HIGHPT_CORRECTION[2016][3] = {9.707212447766441, 0.41867148876190186, 0.2094722718545437};
  ADHOC_DSTMUNU_HIGHPT_CORRECTION[2016][4] = {9.789813900824859, 0.45282208919525146, 0.2511064432038998};
  ADHOC_DSTMUNU_HIGHPT_CORRECTION[2017][1] = {9.55302345373079, 0.3617384433746338, 0.04477694232568823};
  ADHOC_DSTMUNU_HIGHPT_CORRECTION[2017][2] = {9.629396296912823, 0.3950282335281372, 0.16859804456854055};
  ADHOC_DSTMUNU_HIGHPT_CORRECTION[2017][3] = {9.706393915824071, 0.4250040352344513, 0.2012668775442716};
  ADHOC_DSTMUNU_HIGHPT_CORRECTION[2017][4] = {9.789393427319851, 0.4539465606212616, 0.23066077407631463};
  ADHOC_DSTMUNU_HIGHPT_CORRECTION[2018][1] = {9.554155399668709, 0.335138738155365, -0.05308479738335352};
  ADHOC_DSTMUNU_HIGHPT_CORRECTION[2018][2] = {9.628126691503851, 0.3630722463130951, 0.15098674006113016};
  ADHOC_DSTMUNU_HIGHPT_CORRECTION[2018][3] = {9.708051195320044, 0.38340288400650024, 0.2196162394569915};
  ADHOC_DSTMUNU_HIGHPT_CORRECTION[2018][4] = {9.789697427320792, 0.42213207483291626, 0.2591529450676936};

  if ( PZ > 0 ) {
    // clamp log(pz), log(pt) using response histo first
    auto binPZ = hist->GetXaxis()->FindBin( TMath::Log( PZ ) );
    if (binPZ==0) binPZ=1;
    if (binPZ==5) binPZ=4;
    auto binPT = hist->GetYaxis()->FindBin( TMath::Log( PT ) );
    if (binPT==0) binPT=1;
    if (binPT==9) binPT=8; // assumes measurement made in 8 log(pt) bins
    if (adhoc_correction && binPT == hist->GetNbinsY()) { // only apply correction to high log(pT) bin (w = w_uncor(a_i(log(pT)-m_i)+e_i)/e_i, but w_uncor = e_i, see l0_global_tis_highpT_adhoc_correction.py for notation)
      // std::cout << "...correcting L0 Global TIS measurement for high B log(pT)..." << std::endl;
      vector<float> adhoc = ADHOC_DSTMUNU_HIGHPT_CORRECTION[year][binPZ];
      return adhoc[2]*(TMath::Log(PT)-adhoc[0])+adhoc[1];
    }
    return hist->GetBinContent( binPZ, binPT );
  }

  return 0;
}

// alternative weight using 3D info for eff- [log(pz), log(pt), ntracks (new)]
float l0GlobalTisTrigger3DEmu( double PZ, double PT, int nTracks, int year,
                             map<int, TH2F*> respHistos, bool adhoc_correction ) {
  auto hist = respHistos[year];

  // ad-hoc correction to mimic high log(pT) behavior seen in rdx fullsim MC, found in lhcb-ntuples-gen/scripts/l0_global_tis_highpT_adhoc_correction.py using D*+munu fullsim MC
  // map<int, map<int, vector<float>>> ADHOC_DSTMUNU_HIGHPT_CORRECTION;
  // ADHOC_DSTMUNU_HIGHPT_CORRECTION[2016][1] = {9.555505392236217, 0.36532145738601685, 0.052622689128916246}; // D*munu high log(pT) bin mean, eff from JpsiK data, slope correction
  // ADHOC_DSTMUNU_HIGHPT_CORRECTION[2016][2] = {9.62712222725513, 0.3874987065792084, 0.16081922872501483};
  // ADHOC_DSTMUNU_HIGHPT_CORRECTION[2016][3] = {9.707212447766441, 0.41867148876190186, 0.2094722718545437};
  // ADHOC_DSTMUNU_HIGHPT_CORRECTION[2016][4] = {9.789813900824859, 0.45282208919525146, 0.2511064432038998};
  // ADHOC_DSTMUNU_HIGHPT_CORRECTION[2017][1] = {9.55302345373079, 0.3617384433746338, 0.04477694232568823};
  // ADHOC_DSTMUNU_HIGHPT_CORRECTION[2017][2] = {9.629396296912823, 0.3950282335281372, 0.16859804456854055};
  // ADHOC_DSTMUNU_HIGHPT_CORRECTION[2017][3] = {9.706393915824071, 0.4250040352344513, 0.2012668775442716};
  // ADHOC_DSTMUNU_HIGHPT_CORRECTION[2017][4] = {9.789393427319851, 0.4539465606212616, 0.23066077407631463};
  // ADHOC_DSTMUNU_HIGHPT_CORRECTION[2018][1] = {9.554155399668709, 0.335138738155365, -0.05308479738335352};
  // ADHOC_DSTMUNU_HIGHPT_CORRECTION[2018][2] = {9.628126691503851, 0.3630722463130951, 0.15098674006113016};
  // ADHOC_DSTMUNU_HIGHPT_CORRECTION[2018][3] = {9.708051195320044, 0.38340288400650024, 0.2196162394569915};
  // ADHOC_DSTMUNU_HIGHPT_CORRECTION[2018][4] = {9.789697427320792, 0.42213207483291626, 0.2591529450676936};

  if ( PZ > 0 ) {
    auto binPZ = hist->GetXaxis()->FindBin( TMath::Log( PZ ) );
    auto binPT = hist->GetYaxis()->FindBin( TMath::Log( PT ) );
    auto binNTracks = hist->GetZaxis()->FindBin( nTracks );
    // if (adhoc_correction && binPT == hist->GetNbinsY()) { // only apply correction to high log(pT) bin (w = w_uncor(a_i(log(pT)-m_i)+e_i)/e_i, but w_uncor = e_i, see l0_global_tis_highpT_adhoc_correction.py for notation)
    //   // std::cout << "...correcting L0 Global TIS measurement for high B log(pT)..." << std::endl;
    //   vector<float> adhoc = ADHOC_DSTMUNU_HIGHPT_CORRECTION[year][binPZ];
    //   return adhoc[2]*(TMath::Log(PT)-adhoc[0])+adhoc[1];
    // }
    return hist->GetBinContent( binPZ, binPT, binNTracks );
  }

  return 0;
}

#endif
