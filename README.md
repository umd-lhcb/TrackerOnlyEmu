# TrackerOnlyEmu

A collection of tools for emulating tracker responses. This is a partial reimplementation of the trigger emulation methods described in [_LHCb-INT-2019-025_](https://cds.cern.ch/record/2703802?ln=en). The original implementation is available at [`B02DplusTauNu`](https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu).

This repository aims to be fully transportable. Not everything is derived in this repo (just the L0Hadron emulation training is done here), but it contains all the files necessary to apply the run2 muonic $R(D^{*+,0})$ tracker-only (TO) emulation.

If you want to try out the trigger emulation:
```
make install
make test-all
```
The test in particular runs 
```
python scripts/run2-rdx-trg_emu.py ./samples/run2-rdx-sample.root ./gen/emu_all.root
```
which runs all the emulations at once, producing a friend tuple gen/emu_all.root with the emulation branches to be used. For this analysis' trigger path

| Trigger level | Requirements |
|---|---|
| L0 | $D^0$ L0Hadron TOS \|\| $B$ L0Global TIS |
| HLT1 | ($K$ Hlt1TrackMVA TOS \|\| $\pi$ Hlt1TrackMVA TOS) \|\|<br>$D^0$ Hlt1TwoTrackMVA TOS |
| HLT2 | $B$ Hlt2XcMuXForTauB2XcMu |

the L0 and HLT1 lines require emulating (HLT1 is cut-based with variables available in TO, HLT2 can be readily reproduced by offline DaVinci selections). For MC with HLT2 already applied, the trigger weight for a given event, using an efficiency for an nSPDHits cut at the lower L0 threshold of 450, is
```
(nspdcor_lt450_eff*(d0_l0_hadron_tos_emu + b0_l0_global_tis_emu_pt_ntr_cor - d0_l0_hadron_tos_emu*b0_l0_global_tis_emu_pt_ntr_cor)) * (k_hlt1_trackmva_tos_emu || pi_hlt1_trackmva_tos_emu || d0_hlt1_twotrackmva_tos_emu)
```
where the subtracted multiplied L0 weights remove the double-counted overlap of the two L0 triggers.

Note that you should have Python 3.8+ and ROOT 6.22+ installed.


## Emulation scripts

The main emulation scripts in the `scripts` folder are:

- `run2-rdx-hlt1.py`: Cut-based HLT1Track/TwoTrackMVA emulation. Requires extra branches from DaVinci
- `run2-rdx-nspdcor_lt450.py`: Generate the corrected nSPDHits < 450 efficiency branch as a function of `nTracks`
- `run2-rdx-l0_global_tis.py`: Weight-based L0Global TIS emulation, data-driven
- `run2-rdx-l0_hadron_tos.py`: Weight-based L0Hadron TOS emulation, with a XGB regressor
- `run2-rdx-trg_emu.py`: Emulate all RDX run 2 triggers in a single script


## HLT1: add info extraction tool to DaVinci

We've tested the code works with `DaVinci/v45r6`. The instructions are adapted from the
[original `README.md`](https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu/-/blob/master/tuple_production/tuple_tools_src/RelatedInfoTools/README.md)

```shell
# Create a new DaVinci dev project
lb-dev DaVinci/<dv_version>  # <dv_version> -> v45r6
cd DaVinciDev_<dv_version>

# Checkout the correct version of Phys so we can override our changes later
git lb-use Phys
git lb-checkout Phys/<phys_version> Phys/LoKiPhys  # <phys_version> -> v26r6 for DaVinci/v45r6
git lb-checkout Phys/<phys_version> Phys/DaVinciTypes
git lb-checkout Phys/<phys_version> Phys/RelatedInfoTools

# Override our changes
cp -r ./davinci/Phys DaVinciDev_<dv_version>

# Compile
make  # Tested to at least work with DaVinci/v45r6
```

After that, add `TupleToolL0Calo` to your `DaVinci` script.


### Comment on the `RelatedInfoTools`

It extracts all stable daughter particles starting from top reconstructed
particle. It traverses the whole decay tree from the mother and if a daughter
is not stable, it will skip that daughter and keep recursing.


### Comment on the tracking efficiency correction factor

In [`run2-Hlt1GEC.h`](./TrackerOnlyEmu/triggers/hlt1/run2-Hlt1GEC.h),
we have the following efficiency correction factor:
```cpp
const double EFF_CORRECTION = 0.042;
```

This factor is obtained in the following way:

1. Go to [LHCb-PUB-2015-024](https://cds.cern.ch/record/2105078/files/LHCb-PUB-2015-024.pdf)
2. Locate _Table 2_. Note the _Velo-Forward_ and _VeloTT-Forward_ efficiencies are:

    93.15% and 89.23%

3. Go to _Appendix B_, locate _Eq. 14, efficiency loss_:

    $$
    \text{efficiency loss} = \left( 1 - \frac{N_\text{VeloTT-Forward}}{N_\text{Velo-Forward}} \right)
    $$

4. Now compute the correction factor:

    $$
    \text{correction} = \left( 1 - \frac{0.8923}{0.9315} \right) = 0.042
    $$


## nSPDHits<450 efficiency

[`run2-rdx-nspdcor_lt450.py`](./scripts/run2-rdx-nspdcor_lt450.py) assigns the efficiency of the corrected nSPDHits < 450 requirement using a year-specific `nTracks` map derived from `Dstmunu` FS MC without a trigger selection. The map is derived by [`l0_nspdhits_cut_eff_ntracks.py`](../lhcb-tmp/scripts/l0_nspdhits_cut_eff_ntracks.py), after [`apply_nspdhits_JpsiK_correction.py`](../lhcb-tmp/scripts/apply_nspdhits_JpsiK_correction.py) adds a conditional-Poisson hit-count shift calibrated in `J/psi K` FS MC.

**Required input branches:** `nTracks`

To run standalone:
```
scripts/run2-rdx-nspdcor_lt450.py samples/run2-rdx-sample.root gen/spd_eff.root
```

| Option | Meaning / default |
|---|---|
| `-t`, `--tree` | Input/output tree name; default `TupleB0/DecayTree`. |
| `-y`, `--year` | `2016`, `2017`, or `2018`; default `2016`. |
| `--map FILE` | Override the packaged efficiency map `nspdhits_corrected_lt450_Dstmunu_FS_eff_notrigger.root`; the file must contain `nspdhits_corrected_lt450_<year>`. |


## L0Global B TIS

[`run2-rdx-l0_global_tis.py`](./scripts/run2-rdx-l0_global_tis.py) looks up a year-specific L0Global $B$ TIS efficiency measured with the TISTOS method in `J/psi K` data by [`measure_and_validate_L0GlobalTIS.py`](https://github.com/umd-lhcb/lhcb-ntuples-gen/blob/master/run2-JpsiK/measure_tis/measure_and_validate_L0GlobalTIS.py), binned 8 bins of `B log(pT)` and 4 bins of `B log(pz)`. It uses true B momenta for the semileptonic signal because of the missing neutrinos, and applies a high-pT correction derived from `Dstmunu` FS (with corrected nSPDHits < 450) using [`l0_global_tis_highpT_adhoc_correction.py`](../lhcb-tmp/scripts/l0_global_tis_highpT_adhoc_correction.py) inside [`run2-L0GlobalTIS.h`](./TrackerOnlyEmu/triggers/l0/run2-L0GlobalTIS.h). A further multiplicative `nTracks` correction, derived by [`l0_global_tis_ntracks_correction.py`](../lhcb-tmp/scripts/l0_global_tis_ntracks_correction.py) as the ratio of the Boolean TIS efficiency to the mean high-pT-corrected TIS weight in `Dstmunu` FS (with corrected nSPDHits < 450), comes from the year-specific `tisbool_ratio_wtis-ntracks-nspdcorlt450-<year>.root` map.

**Required input branches:** `b0_TRUEP_Z`, `b0_TRUEPT`, `nTracks` (replacing the B prefix as appropriate)
With `--debug`, also supply `b0_L0Global_TIS`, `FitVar_q2`, `FitVar_Mmiss2`, `FitVar_El`, and `NumSPDHits`

To run standalone:
```
scripts/run2-rdx-l0_global_tis.py samples/run2-rdx-sample.root gen/l0_global_tis.root
```

| Option | Meaning / default |
|---|---|
| `-t`, `--tree` | Input/output tree name; default `TupleB0/DecayTree`. |
| `-y`, `--year` | Select the `2016`, `2017`, or `2018` efficiency/correction maps; default `2016`. |
| `-B`, `--Bmeson` | B branch prefix; default `b0`. Also determines the output branch prefix. |
| `--debug` | Save reference trigger decisions and diagnostic variables; requires the extra inputs listed above. |

Both the standalone and combined runners write:

- `b0_l0_global_tis_emu_no_cor`: the data efficiency lookup without the additional corrections.
- `b0_l0_global_tis_emu_pt_cor`: includes the high-pT correction.
- `b0_l0_global_tis_emu_pt_ntr_cor`: includes both corrections; this is the weight used in the trigger expression above.

## L0Hadron D0 TOS

[`run2-rdx-l0_hadron_tos.py`](./scripts/run2-rdx-l0_hadron_tos.py) in `-m xgb` mode trains or loads an XGBoost classifier and uses its predicted L0Hadron TOS probability as an event weight. The year-specific nominal model is trained on the original L0Hadron decision of `Dstmunu` FS (with corrected nSPDHits < 450), using samples prepared by [`gen_l0hadron_xgb_train_tuples_nspd_corrected.py`](../lhcb-tmp/scripts/gen_l0hadron_xgb_train_tuples_nspd_corrected.py).

**Required XGB input branches:**

- `nTracks`, `d0_P`, `d0_PT`.
- For each of `k` and `pi`: `<particle>_P`, `<particle>_PT`, `<particle>_L0Calo_HCAL_realET`, `<particle>_L0Calo_HCAL_xProjection`, `<particle>_L0Calo_HCAL_yProjection`, and `<particle>_L0Calo_HCAL_region`.

The standalone script also requires `d0_L0HadronDecision_TOS`, even with `--load`, because it reads the target before choosing whether to train. Its `--debug` mode also requires `NumSPDHits`, `FitVar_q2`, `FitVar_Mmiss2`, and `FitVar_El`. The combined runner only needs the XGB features for this component, plus the inputs for its other emulations if those are specified (the legacy BDT modes use a different input set).

To run standalone, applying the nominal XGB model without retraining (match the model filename to `--year`):

```
scripts/run2-rdx-l0_hadron_tos.py samples/run2-rdx-sample.root gen/l0_hadron_tos.root \
    -m xgb -y 2016 \
    --load ./TrackerOnlyEmu/triggers/l0/xgb4_nspdcorlt450_precut-2016.pickle
```

To train instead, omit `--load` and optionally save the fitted model with `--dump`:

```
scripts/run2-rdx-l0_hadron_tos.py samples/run2-rdx-sample.root gen/l0_hadron_training.root -m xgb -y 2016 --dump gen/xgb4.pickle
```

| Option | Meaning / default |
|---|---|
| `-t`, `--tree` | Input/output tree name; default `TupleB0/DecayTree`. |
| `-y`, `--year` | Year; default `2016`. In XGB mode, selects the `2016`, `2017`, or `2018` comparison models; match any loaded/trained nominal model to this year. |
| `-m`, `--mode` | `xgb` (default), `bdt`, or `bdt_old`. The latter two are legacy regression modes with different input requirements; `bdt_old` does not support `--load`. |
| `--load FILE` | Load a pickled model instead of fitting. In XGB mode this sets the nominal model; both year-specific comparison models are still loaded automatically. |
| `--dump FILE` | Save the newly fitted model; used only when training. |
| `--ntrees N` | Number of estimators when training; default `300`. |
| `--max-depth N` | Maximum tree depth when training; default `4`. |
| `--debug` | Save reference/diagnostic variables; requires the extra inputs listed above for XGB mode. |

The combined trigger runner always writes three year-matched XGB predictions:

- `d0_l0_hadron_tos_emu`: `xgb4_nspdcorlt450_precut-<year>.pickle`, trained
  after the corrected SPD<450 pre-cut on the original L0Hadron decision.
- `d0_l0_hadron_tos_emu_nspd_uncorrected`: `xgb4-<year>.pickle`, trained
  without the pre-cut on the original L0Hadron decision.
- `d0_l0_hadron_tos_emu_nspd_corrected`: `xgb4_nspd_corrected-<year>.pickle`,
  trained without the pre-cut on the joint L0Hadron-and-SPD decision.

The standalone L0Hadron runner also adds these three branches in XGB mode,
using its trained or explicitly loaded (`--load`) model as nominal. It does not
apply the corrected-SPD pre-cut automatically when training: the input sample
must already have that selection to reproduce the nominal training prescription.
A `d0_l0_hadron_tos_spd_corrected` input branch must be provided if the target is the
joint `L0Hadron && nspdhits_corrected<450`, and the corresponding `reg_br` should be
modified for the XGB config.
The combined runner currently selects all models by `--year` and does not expose
`--load`.


## Develop this project

This project uses a wrapper to `RDataFrame` to make definition of process
sequence easier. It also tries to separate C++ code from Python code by
defining C++ functions in separate headers then load them in Python.

The directory structures of this project is:

```shell
.
├── davinci         # add-on to DaVinci to produce required branches
├── gen             # non-saved outputs of scripts
├── nix             # nix overlay, currently unused
├── samples         # sample input ntuples
├── scripts         # actual emulation scripts
└── TrackerOnlyEmu  # some Python files
    ├── emulation   # emulation code for analyses, currently just for RDX run 2
    └── triggers
        ├── hlt1    # HLT1 emulation C++ code
        └── l0      # L0 emulation C++ code, input ntuples (e.g. HCAL response), and exported BDT/XGB
```

For the Python module:

- The `RDataFrame` wrapper (which is very thin) is defined in [`TrackerOnlyEmu/executor.py`](./TrackerOnlyEmu/executor.py).

- A generic file loader is defined in [`TrackerOnlyEmu/loader.py`](./TrackerOnlyEmu/loader.py)
    - This is needed because we also INSTALL C++ code, input ntuples and
      exported BDT together with the Python package, so we need to be able to
      load them without knowing their relative location to the emulation scripts.
    - The C++ files are typically loaded with:

        ```python
        load_cpp('<triggers/l0/run2-L0Hadron.h>')
        ```

    - If you want to load a file relative to your current script, just drop the `<>`:

        ```python
        load_cpp('../TrackerOnlyEmu/triggers/l0/run2-L0Hadron.h')
        ```



## Acknowledgement
Much of the source code is shamelessly stolen from [`B02DplusTauNu`](https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu) analysis.
