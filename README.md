# TrackerOnlyEmu

A collection of tools for emulating tracker responses. This is a partial
reimplementation of the trigger emulation methods described in
[_LHCb-INT-2019-025_](https://cds.cern.ch/record/2703802?ln=en).

The original implementation are available at [`B02DplusTauNu`](https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu).

This repository aims at fully self-containing. Both the BDT training ntuple and
a sample ntuple for trigger emulation are provided.

If you want to try out the trigger emulation:
```
make install
make test-all
```

Note that you should have Python 3.8+ and ROOT 6.22+ installed.


## Emulation scripts

Currently we have 4 scripts in the `scripts` folder:

- `run2-rdx-hlt1.py`: Cut-based HLT1Track/TwoTrackMVA emulation. Requires extra branches from DaVinci
- `run2-rdx-l0_global_tis.py`: Weight-based L0Global TIS emulation, data-driven
- `run2-rdx-l0_hadron_tos.py`: Weight-based L0Hadron TOS emulation, with a XGB regressor
- `run2-rdx-trg_emu.py`: Emulate all RDX run 2 triggers in a single script


## Sample ntuples

We supply the following sample ntuples in the `samples` folder:

- `run2-rdx-sample.root`: FullSim sample for RDX run 2 for testing trigger application
- `run2-rdx-train_bdt.root`: Input to train a BDT for L0Hadron TOS (obsolete method)
- `run2-rdx-train_xgb.root`: Input to train a XGB for L0Hadron TOS (default method)


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


## Add HLT1 info extraction tool to DaVinci

We've tested the code work with `DaVinci/v45r6`. The instructions are adapted from the
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


## Comment on the `RelatedInfoTools`

It extracts all stable daughter particles starting from top reconstructed
particle. It traverses the whole decay tree from the mother and if a daughter
is not stable, it will skip that daughter and keep recursing.


## Comment on the tracking efficiency correction factor

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


## Acknowledgement
Most of the source code are shamelessly stolen from [`B02DplusTauNu`](https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu) analysis,
with minor changes from us.
