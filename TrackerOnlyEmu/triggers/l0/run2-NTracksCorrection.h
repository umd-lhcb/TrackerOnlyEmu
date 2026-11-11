#ifndef TRACKERONLYEMU_RUN2_NTRACKS_CORRECTION_H
#define TRACKERONLYEMU_RUN2_NTRACKS_CORRECTION_H
#include <cmath>
#include <stdexcept>
#include <string>
#include <TH1.h>
#include <TFile.h>

// Shared lookup for TIS corrections and the independent SPD acceptance.
TH1* readL0NTracksCorrection(const char* path, const char* histogramName = "correction") {
  TFile file(path, "READ");
  if (file.IsZombie()) throw std::runtime_error(std::string("Cannot open nTracks map: ") + path);
  auto input = dynamic_cast<TH1*>(file.Get(histogramName));
  if (!input || input->GetDimension() != 1)
    throw std::runtime_error(std::string("Expected a one-dimensional histogram: ") + histogramName);
  for (int bin = 0; bin <= input->GetNbinsX() + 1; ++bin) {
    double value = input->GetBinContent(bin);
    if (!std::isfinite(value) || value < 0)
      throw std::runtime_error("Invalid nTracks correction bin content");
  }
  auto hist = static_cast<TH1*>(input->Clone());
  hist->SetDirectory(nullptr);
  return hist;
}

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


#endif
