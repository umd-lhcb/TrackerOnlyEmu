###############################################################################
# (c) Copyright 2000-2018 CERN for the benefit of the LHCb Collaboration      #
#                                                                             #
# This software is distributed under the terms of the GNU General Public      #
# Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   #
#                                                                             #
# In applying this licence, CERN does not waive the privileges and immunities #
# granted to it by virtue of its status as an Intergovernmental Organization  #
# or submit itself to any jurisdiction.                                       #
###############################################################################
################################################################################
# Package: RelatedInfoTools
################################################################################
gaudi_subdir(RelatedInfoTools)

gaudi_depends_on_subdirs(Phys/DaVinciKernel
                         Phys/LoKiPhys)

find_package(ROOT)
find_package(ROOT COMPONENTS Physics)
find_package(Boost)
include_directories(SYSTEM ${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

gaudi_add_module(RelatedInfoTools
                 src/*.cpp
                 LINK_LIBRARIES DaVinciKernelLib LoKiPhysLib ROOT)
