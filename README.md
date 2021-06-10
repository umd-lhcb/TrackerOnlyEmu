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

It extract all stable daughter particles starting from top reconstructed
particle. It traverse the whole decay tree from the mother and if a daughter
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

    ![formula](https://render.githubusercontent.com/render/math?math=\text{efficiency%20loss}%20=%20\left(1%20-%20\frac{N_{\text{VeloTT-Forward}}}{N_{\text{Velo-Forward}}}%20\right))
4. Now compute the correction factor:

    ![formula](https://render.githubusercontent.com/render/math?math=\text{correction}%20=%20\left(1%20-%20\frac{0.8923}{0.9315}%20\right)%20=%200.042)


## Comment on the transverse energy of ![D0](https://render.githubusercontent.com/render/math?math=D^0)

First, the transverse energy is defined as (take from p. 4 of [this paper](https://arxiv.org/pdf/2008.11556.pdf)):

![E_T definition](https://render.githubusercontent.com/render/math?math=E_T%20=%20E%20\sin\theta%20=%20\sqrt{m^2%20%2B%20p^2}%20\sin\theta)

In the massless limit:

![E_T p_T relation](https://render.githubusercontent.com/render/math?math=E_T%20\approx%20\sqrt{p^2}%20\sin\theta%20=%20p_T)


## Acknowledgement
Most of the source code are shamelessly stolen from [`B02DplusTauNu`](https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu) analysis,
with minor changes from us.
