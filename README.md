# TrackerOnlyEmu
A collection of tools for emulating tracker responses.


## Add HLT1-info extraction tool to DaVinci
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


## Acknowledgement
Most of the source code are shamelessly stolen from [`B02DplusTauNu`](https://gitlab.cern.ch/lhcb-slb/B02DplusTauNu) analysis,
with minor changes from us.
