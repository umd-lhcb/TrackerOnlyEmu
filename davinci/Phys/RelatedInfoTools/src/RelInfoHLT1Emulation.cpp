/*****************************************************************************\
* (c) Copyright 2000-2018 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
// Gaudi
#include "Event/Particle.h"
#include "LoKi/Combiner.h"
// kernel
#include "GaudiKernel/PhysicalConstants.h"
#include "Kernel/IDistanceCalculator.h"
#include "Kernel/IParticleCombiner.h"
#include "Kernel/RelatedInfoNamed.h"
// local
#include "RelInfoHLT1Emulation.h"

#define MAXNUMBER 6
//
#include <math.h>

//#include "Phys/LoKiPhys/LoKi/Particles38.h"
//#include "Phys/LoKiPhys/LoKi/Particles20.h"

#include "LoKi/LoKi.h"
#include "LoKi/Particles.h"

//-----------------------------------------------------------------------------
// Implementation file for class : RelInfoHLT1Emulation
// @authors Simone Meloni, Julian Garcìa Pardinas
// @date 2019-02-14
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_COMPONENT( RelInfoHLT1Emulation )

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
RelInfoHLT1Emulation::RelInfoHLT1Emulation( const std::string &type,
                                            const std::string &name,
                                            const IInterface * parent )
    : GaudiTool( type, name, parent ),
      m_dva( 0 ),
      m_dist( 0 ),
      m_pCombiner( 0 ) {
  declareInterface<IRelatedInfoTool>( this );
  declareProperty( "Variables", m_variables,
                   "List of variables to store (store all if empty)" );
  declareProperty( "nltValue", m_nltValue = 16.,
                   "Value of IPCHI2 cut for the nlt variable in HLT (16 if "
                   "not configured)" );
}

//=============================================================================
// Destructor
//=============================================================================
RelInfoHLT1Emulation::~RelInfoHLT1Emulation() {}

//=============================================================================
// Initialize
//=============================================================================
StatusCode RelInfoHLT1Emulation::initialize() {
  const StatusCode sc = GaudiTool::initialize();
  if ( sc.isFailure() ) return sc;

  m_dva = Gaudi::Utils::getIDVAlgorithm( contextSvc(), this );
  if ( !m_dva )
    return Error( "Couldn't get parent DVAlgorithm", StatusCode::FAILURE );

  m_dist = m_dva->distanceCalculator( "LoKi::DistanceCalculator" );
  if ( !m_dist )
    return Error( "Unable to retrieve the IDistanceCalculator tool",
                  StatusCode::FAILURE );

  m_pCombiner = m_dva->particleCombiner( "LoKi::VertexFitter" );
  if ( !m_pCombiner )
    return Error( "Unable to retrieve the IParticleCombiner tool",
                  StatusCode::FAILURE );
  m_keys.clear();

  if ( m_variables.empty() ) {
    if ( msgLevel( MSG::DEBUG ) )
      debug() << "List of variables empty, adding all" << endmsg;
    m_keys.push_back( RelatedInfoNamed::NDAUGHTERS );

    m_keys.push_back( RelatedInfoNamed::PT_DAU_1 );
    m_keys.push_back( RelatedInfoNamed::PT_DAU_2 );
    m_keys.push_back( RelatedInfoNamed::PT_DAU_3 );
    m_keys.push_back( RelatedInfoNamed::PT_DAU_4 );
    m_keys.push_back( RelatedInfoNamed::PT_DAU_5 );
    m_keys.push_back( RelatedInfoNamed::PT_DAU_6 );

    m_keys.push_back( RelatedInfoNamed::P_DAU_1 );
    m_keys.push_back( RelatedInfoNamed::P_DAU_2 );
    m_keys.push_back( RelatedInfoNamed::P_DAU_3 );
    m_keys.push_back( RelatedInfoNamed::P_DAU_4 );
    m_keys.push_back( RelatedInfoNamed::P_DAU_5 );
    m_keys.push_back( RelatedInfoNamed::P_DAU_6 );

    m_keys.push_back( RelatedInfoNamed::IPCHI2_OWNPV_DAU_1 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_OWNPV_DAU_2 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_OWNPV_DAU_3 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_OWNPV_DAU_4 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_OWNPV_DAU_5 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_OWNPV_DAU_6 );

    m_keys.push_back( RelatedInfoNamed::IPCHI2_MINIPPV_DAU_1 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_MINIPPV_DAU_2 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_MINIPPV_DAU_3 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_MINIPPV_DAU_4 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_MINIPPV_DAU_5 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_MINIPPV_DAU_6 );

    m_keys.push_back( RelatedInfoNamed::TRACK_GHOSTPROB_DAU_1 );
    m_keys.push_back( RelatedInfoNamed::TRACK_GHOSTPROB_DAU_2 );
    m_keys.push_back( RelatedInfoNamed::TRACK_GHOSTPROB_DAU_3 );
    m_keys.push_back( RelatedInfoNamed::TRACK_GHOSTPROB_DAU_4 );
    m_keys.push_back( RelatedInfoNamed::TRACK_GHOSTPROB_DAU_5 );
    m_keys.push_back( RelatedInfoNamed::TRACK_GHOSTPROB_DAU_6 );

    m_keys.push_back( RelatedInfoNamed::TRACK_CHI2_DAU_1 );
    m_keys.push_back( RelatedInfoNamed::TRACK_CHI2_DAU_2 );
    m_keys.push_back( RelatedInfoNamed::TRACK_CHI2_DAU_3 );
    m_keys.push_back( RelatedInfoNamed::TRACK_CHI2_DAU_4 );
    m_keys.push_back( RelatedInfoNamed::TRACK_CHI2_DAU_5 );
    m_keys.push_back( RelatedInfoNamed::TRACK_CHI2_DAU_6 );
    m_keys.push_back( RelatedInfoNamed::TRACK_NDOF_DAU_1 );
    m_keys.push_back( RelatedInfoNamed::TRACK_NDOF_DAU_2 );
    m_keys.push_back( RelatedInfoNamed::TRACK_NDOF_DAU_3 );
    m_keys.push_back( RelatedInfoNamed::TRACK_NDOF_DAU_4 );
    m_keys.push_back( RelatedInfoNamed::TRACK_NDOF_DAU_5 );
    m_keys.push_back( RelatedInfoNamed::TRACK_NDOF_DAU_6 );

    m_keys.push_back( RelatedInfoNamed::VERTEX_CHI2_COMB_1_2 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_CHI2_COMB_1_3 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_CHI2_COMB_1_4 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_CHI2_COMB_1_5 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_CHI2_COMB_1_6 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_CHI2_COMB_2_3 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_CHI2_COMB_2_4 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_CHI2_COMB_2_5 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_CHI2_COMB_2_6 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_CHI2_COMB_3_4 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_CHI2_COMB_3_5 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_CHI2_COMB_3_6 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_CHI2_COMB_4_5 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_CHI2_COMB_4_6 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_CHI2_COMB_5_6 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_NDOF_COMB_1_2 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_NDOF_COMB_1_3 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_NDOF_COMB_1_4 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_NDOF_COMB_1_5 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_NDOF_COMB_1_6 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_NDOF_COMB_2_3 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_NDOF_COMB_2_4 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_NDOF_COMB_2_5 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_NDOF_COMB_2_6 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_NDOF_COMB_3_4 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_NDOF_COMB_3_5 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_NDOF_COMB_3_6 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_NDOF_COMB_4_5 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_NDOF_COMB_4_6 );
    m_keys.push_back( RelatedInfoNamed::VERTEX_NDOF_COMB_5_6 );

    m_keys.push_back( RelatedInfoNamed::ETA_COMB_1_2 );
    m_keys.push_back( RelatedInfoNamed::ETA_COMB_1_3 );
    m_keys.push_back( RelatedInfoNamed::ETA_COMB_1_4 );
    m_keys.push_back( RelatedInfoNamed::ETA_COMB_1_5 );
    m_keys.push_back( RelatedInfoNamed::ETA_COMB_1_6 );
    m_keys.push_back( RelatedInfoNamed::ETA_COMB_2_3 );
    m_keys.push_back( RelatedInfoNamed::ETA_COMB_2_4 );
    m_keys.push_back( RelatedInfoNamed::ETA_COMB_2_5 );
    m_keys.push_back( RelatedInfoNamed::ETA_COMB_2_6 );
    m_keys.push_back( RelatedInfoNamed::ETA_COMB_3_4 );
    m_keys.push_back( RelatedInfoNamed::ETA_COMB_3_5 );
    m_keys.push_back( RelatedInfoNamed::ETA_COMB_3_6 );
    m_keys.push_back( RelatedInfoNamed::ETA_COMB_4_5 );
    m_keys.push_back( RelatedInfoNamed::ETA_COMB_4_6 );
    m_keys.push_back( RelatedInfoNamed::ETA_COMB_5_6 );

    m_keys.push_back( RelatedInfoNamed::MCORR_OWNPV_COMB_1_2 );
    m_keys.push_back( RelatedInfoNamed::MCORR_OWNPV_COMB_1_3 );
    m_keys.push_back( RelatedInfoNamed::MCORR_OWNPV_COMB_1_4 );
    m_keys.push_back( RelatedInfoNamed::MCORR_OWNPV_COMB_1_5 );
    m_keys.push_back( RelatedInfoNamed::MCORR_OWNPV_COMB_1_6 );
    m_keys.push_back( RelatedInfoNamed::MCORR_OWNPV_COMB_2_3 );
    m_keys.push_back( RelatedInfoNamed::MCORR_OWNPV_COMB_2_4 );
    m_keys.push_back( RelatedInfoNamed::MCORR_OWNPV_COMB_2_5 );
    m_keys.push_back( RelatedInfoNamed::MCORR_OWNPV_COMB_2_6 );
    m_keys.push_back( RelatedInfoNamed::MCORR_OWNPV_COMB_3_4 );
    m_keys.push_back( RelatedInfoNamed::MCORR_OWNPV_COMB_3_5 );
    m_keys.push_back( RelatedInfoNamed::MCORR_OWNPV_COMB_3_6 );
    m_keys.push_back( RelatedInfoNamed::MCORR_OWNPV_COMB_4_5 );
    m_keys.push_back( RelatedInfoNamed::MCORR_OWNPV_COMB_4_6 );
    m_keys.push_back( RelatedInfoNamed::MCORR_OWNPV_COMB_5_6 );

    m_keys.push_back( RelatedInfoNamed::MCORR_MINIPPV_COMB_1_2 );
    m_keys.push_back( RelatedInfoNamed::MCORR_MINIPPV_COMB_1_3 );
    m_keys.push_back( RelatedInfoNamed::MCORR_MINIPPV_COMB_1_4 );
    m_keys.push_back( RelatedInfoNamed::MCORR_MINIPPV_COMB_1_5 );
    m_keys.push_back( RelatedInfoNamed::MCORR_MINIPPV_COMB_1_6 );
    m_keys.push_back( RelatedInfoNamed::MCORR_MINIPPV_COMB_2_3 );
    m_keys.push_back( RelatedInfoNamed::MCORR_MINIPPV_COMB_2_4 );
    m_keys.push_back( RelatedInfoNamed::MCORR_MINIPPV_COMB_2_5 );
    m_keys.push_back( RelatedInfoNamed::MCORR_MINIPPV_COMB_2_6 );
    m_keys.push_back( RelatedInfoNamed::MCORR_MINIPPV_COMB_3_4 );
    m_keys.push_back( RelatedInfoNamed::MCORR_MINIPPV_COMB_3_5 );
    m_keys.push_back( RelatedInfoNamed::MCORR_MINIPPV_COMB_3_6 );
    m_keys.push_back( RelatedInfoNamed::MCORR_MINIPPV_COMB_4_5 );
    m_keys.push_back( RelatedInfoNamed::MCORR_MINIPPV_COMB_4_6 );
    m_keys.push_back( RelatedInfoNamed::MCORR_MINIPPV_COMB_5_6 );

    m_keys.push_back( RelatedInfoNamed::SUMPT_COMB_1_2 );
    m_keys.push_back( RelatedInfoNamed::SUMPT_COMB_1_3 );
    m_keys.push_back( RelatedInfoNamed::SUMPT_COMB_1_4 );
    m_keys.push_back( RelatedInfoNamed::SUMPT_COMB_1_5 );
    m_keys.push_back( RelatedInfoNamed::SUMPT_COMB_1_6 );
    m_keys.push_back( RelatedInfoNamed::SUMPT_COMB_2_3 );
    m_keys.push_back( RelatedInfoNamed::SUMPT_COMB_2_4 );
    m_keys.push_back( RelatedInfoNamed::SUMPT_COMB_2_5 );
    m_keys.push_back( RelatedInfoNamed::SUMPT_COMB_2_6 );
    m_keys.push_back( RelatedInfoNamed::SUMPT_COMB_3_4 );
    m_keys.push_back( RelatedInfoNamed::SUMPT_COMB_3_5 );
    m_keys.push_back( RelatedInfoNamed::SUMPT_COMB_3_6 );
    m_keys.push_back( RelatedInfoNamed::SUMPT_COMB_4_5 );
    m_keys.push_back( RelatedInfoNamed::SUMPT_COMB_4_6 );
    m_keys.push_back( RelatedInfoNamed::SUMPT_COMB_5_6 );

    m_keys.push_back( RelatedInfoNamed::DIRA_OWNPV_COMB_1_2 );
    m_keys.push_back( RelatedInfoNamed::DIRA_OWNPV_COMB_1_3 );
    m_keys.push_back( RelatedInfoNamed::DIRA_OWNPV_COMB_1_4 );
    m_keys.push_back( RelatedInfoNamed::DIRA_OWNPV_COMB_1_5 );
    m_keys.push_back( RelatedInfoNamed::DIRA_OWNPV_COMB_1_6 );
    m_keys.push_back( RelatedInfoNamed::DIRA_OWNPV_COMB_2_3 );
    m_keys.push_back( RelatedInfoNamed::DIRA_OWNPV_COMB_2_4 );
    m_keys.push_back( RelatedInfoNamed::DIRA_OWNPV_COMB_2_5 );
    m_keys.push_back( RelatedInfoNamed::DIRA_OWNPV_COMB_2_6 );
    m_keys.push_back( RelatedInfoNamed::DIRA_OWNPV_COMB_3_4 );
    m_keys.push_back( RelatedInfoNamed::DIRA_OWNPV_COMB_3_5 );
    m_keys.push_back( RelatedInfoNamed::DIRA_OWNPV_COMB_3_6 );
    m_keys.push_back( RelatedInfoNamed::DIRA_OWNPV_COMB_4_5 );
    m_keys.push_back( RelatedInfoNamed::DIRA_OWNPV_COMB_4_6 );
    m_keys.push_back( RelatedInfoNamed::DIRA_OWNPV_COMB_5_6 );

    m_keys.push_back( RelatedInfoNamed::DIRA_MINIPPV_COMB_1_2 );
    m_keys.push_back( RelatedInfoNamed::DIRA_MINIPPV_COMB_1_3 );
    m_keys.push_back( RelatedInfoNamed::DIRA_MINIPPV_COMB_1_4 );
    m_keys.push_back( RelatedInfoNamed::DIRA_MINIPPV_COMB_1_5 );
    m_keys.push_back( RelatedInfoNamed::DIRA_MINIPPV_COMB_1_6 );
    m_keys.push_back( RelatedInfoNamed::DIRA_MINIPPV_COMB_2_3 );
    m_keys.push_back( RelatedInfoNamed::DIRA_MINIPPV_COMB_2_4 );
    m_keys.push_back( RelatedInfoNamed::DIRA_MINIPPV_COMB_2_5 );
    m_keys.push_back( RelatedInfoNamed::DIRA_MINIPPV_COMB_2_6 );
    m_keys.push_back( RelatedInfoNamed::DIRA_MINIPPV_COMB_3_4 );
    m_keys.push_back( RelatedInfoNamed::DIRA_MINIPPV_COMB_3_5 );
    m_keys.push_back( RelatedInfoNamed::DIRA_MINIPPV_COMB_3_6 );
    m_keys.push_back( RelatedInfoNamed::DIRA_MINIPPV_COMB_4_5 );
    m_keys.push_back( RelatedInfoNamed::DIRA_MINIPPV_COMB_4_6 );
    m_keys.push_back( RelatedInfoNamed::DIRA_MINIPPV_COMB_5_6 );

    m_keys.push_back( RelatedInfoNamed::DOCA_COMB_1_2 );
    m_keys.push_back( RelatedInfoNamed::DOCA_COMB_1_3 );
    m_keys.push_back( RelatedInfoNamed::DOCA_COMB_1_4 );
    m_keys.push_back( RelatedInfoNamed::DOCA_COMB_1_5 );
    m_keys.push_back( RelatedInfoNamed::DOCA_COMB_1_6 );
    m_keys.push_back( RelatedInfoNamed::DOCA_COMB_2_3 );
    m_keys.push_back( RelatedInfoNamed::DOCA_COMB_2_4 );
    m_keys.push_back( RelatedInfoNamed::DOCA_COMB_2_5 );
    m_keys.push_back( RelatedInfoNamed::DOCA_COMB_2_6 );
    m_keys.push_back( RelatedInfoNamed::DOCA_COMB_3_4 );
    m_keys.push_back( RelatedInfoNamed::DOCA_COMB_3_5 );
    m_keys.push_back( RelatedInfoNamed::DOCA_COMB_3_6 );
    m_keys.push_back( RelatedInfoNamed::DOCA_COMB_4_5 );
    m_keys.push_back( RelatedInfoNamed::DOCA_COMB_4_6 );
    m_keys.push_back( RelatedInfoNamed::DOCA_COMB_5_6 );

    m_keys.push_back( RelatedInfoNamed::VDCHI2_OWNPV_COMB_1_2 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_OWNPV_COMB_1_3 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_OWNPV_COMB_1_4 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_OWNPV_COMB_1_5 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_OWNPV_COMB_1_6 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_OWNPV_COMB_2_3 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_OWNPV_COMB_2_4 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_OWNPV_COMB_2_5 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_OWNPV_COMB_2_6 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_OWNPV_COMB_3_4 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_OWNPV_COMB_3_5 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_OWNPV_COMB_3_6 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_OWNPV_COMB_4_5 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_OWNPV_COMB_4_6 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_OWNPV_COMB_5_6 );

    m_keys.push_back( RelatedInfoNamed::VDCHI2_MINIPPV_COMB_1_2 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_MINIPPV_COMB_1_3 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_MINIPPV_COMB_1_4 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_MINIPPV_COMB_1_5 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_MINIPPV_COMB_1_6 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_MINIPPV_COMB_2_3 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_MINIPPV_COMB_2_4 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_MINIPPV_COMB_2_5 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_MINIPPV_COMB_2_6 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_MINIPPV_COMB_3_4 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_MINIPPV_COMB_3_5 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_MINIPPV_COMB_3_6 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_MINIPPV_COMB_4_5 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_MINIPPV_COMB_4_6 );
    m_keys.push_back( RelatedInfoNamed::VDCHI2_MINIPPV_COMB_5_6 );

    m_keys.push_back( RelatedInfoNamed::IPCHI2_OWNPV_COMB_1_2 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_OWNPV_COMB_1_3 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_OWNPV_COMB_1_4 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_OWNPV_COMB_1_5 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_OWNPV_COMB_1_6 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_OWNPV_COMB_2_3 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_OWNPV_COMB_2_4 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_OWNPV_COMB_2_5 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_OWNPV_COMB_2_6 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_OWNPV_COMB_3_4 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_OWNPV_COMB_3_5 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_OWNPV_COMB_3_6 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_OWNPV_COMB_4_5 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_OWNPV_COMB_4_6 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_OWNPV_COMB_5_6 );

    m_keys.push_back( RelatedInfoNamed::IPCHI2_MINIPPV_COMB_1_2 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_MINIPPV_COMB_1_3 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_MINIPPV_COMB_1_4 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_MINIPPV_COMB_1_5 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_MINIPPV_COMB_1_6 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_MINIPPV_COMB_2_3 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_MINIPPV_COMB_2_4 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_MINIPPV_COMB_2_5 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_MINIPPV_COMB_2_6 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_MINIPPV_COMB_3_4 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_MINIPPV_COMB_3_5 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_MINIPPV_COMB_3_6 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_MINIPPV_COMB_4_5 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_MINIPPV_COMB_4_6 );
    m_keys.push_back( RelatedInfoNamed::IPCHI2_MINIPPV_COMB_5_6 );

    m_keys.push_back( RelatedInfoNamed::NLT_OWNPV_COMB_1_2 );
    m_keys.push_back( RelatedInfoNamed::NLT_OWNPV_COMB_1_3 );
    m_keys.push_back( RelatedInfoNamed::NLT_OWNPV_COMB_1_4 );
    m_keys.push_back( RelatedInfoNamed::NLT_OWNPV_COMB_1_5 );
    m_keys.push_back( RelatedInfoNamed::NLT_OWNPV_COMB_1_6 );
    m_keys.push_back( RelatedInfoNamed::NLT_OWNPV_COMB_2_3 );
    m_keys.push_back( RelatedInfoNamed::NLT_OWNPV_COMB_2_4 );
    m_keys.push_back( RelatedInfoNamed::NLT_OWNPV_COMB_2_5 );
    m_keys.push_back( RelatedInfoNamed::NLT_OWNPV_COMB_2_6 );
    m_keys.push_back( RelatedInfoNamed::NLT_OWNPV_COMB_3_4 );
    m_keys.push_back( RelatedInfoNamed::NLT_OWNPV_COMB_3_5 );
    m_keys.push_back( RelatedInfoNamed::NLT_OWNPV_COMB_3_6 );
    m_keys.push_back( RelatedInfoNamed::NLT_OWNPV_COMB_4_5 );
    m_keys.push_back( RelatedInfoNamed::NLT_OWNPV_COMB_4_6 );
    m_keys.push_back( RelatedInfoNamed::NLT_OWNPV_COMB_5_6 );

    m_keys.push_back( RelatedInfoNamed::NLT_MINIPPV_COMB_1_2 );
    m_keys.push_back( RelatedInfoNamed::NLT_MINIPPV_COMB_1_3 );
    m_keys.push_back( RelatedInfoNamed::NLT_MINIPPV_COMB_1_4 );
    m_keys.push_back( RelatedInfoNamed::NLT_MINIPPV_COMB_1_5 );
    m_keys.push_back( RelatedInfoNamed::NLT_MINIPPV_COMB_1_6 );
    m_keys.push_back( RelatedInfoNamed::NLT_MINIPPV_COMB_2_3 );
    m_keys.push_back( RelatedInfoNamed::NLT_MINIPPV_COMB_2_4 );
    m_keys.push_back( RelatedInfoNamed::NLT_MINIPPV_COMB_2_5 );
    m_keys.push_back( RelatedInfoNamed::NLT_MINIPPV_COMB_2_6 );
    m_keys.push_back( RelatedInfoNamed::NLT_MINIPPV_COMB_3_4 );
    m_keys.push_back( RelatedInfoNamed::NLT_MINIPPV_COMB_3_5 );
    m_keys.push_back( RelatedInfoNamed::NLT_MINIPPV_COMB_3_6 );
    m_keys.push_back( RelatedInfoNamed::NLT_MINIPPV_COMB_4_5 );
    m_keys.push_back( RelatedInfoNamed::NLT_MINIPPV_COMB_4_6 );
    m_keys.push_back( RelatedInfoNamed::NLT_MINIPPV_COMB_5_6 );

    m_keys.push_back( RelatedInfoNamed::PT_COMB_1_2 );
    m_keys.push_back( RelatedInfoNamed::PT_COMB_1_3 );
    m_keys.push_back( RelatedInfoNamed::PT_COMB_1_4 );
    m_keys.push_back( RelatedInfoNamed::PT_COMB_1_5 );
    m_keys.push_back( RelatedInfoNamed::PT_COMB_1_6 );
    m_keys.push_back( RelatedInfoNamed::PT_COMB_2_3 );
    m_keys.push_back( RelatedInfoNamed::PT_COMB_2_4 );
    m_keys.push_back( RelatedInfoNamed::PT_COMB_2_5 );
    m_keys.push_back( RelatedInfoNamed::PT_COMB_2_6 );
    m_keys.push_back( RelatedInfoNamed::PT_COMB_3_4 );
    m_keys.push_back( RelatedInfoNamed::PT_COMB_3_5 );
    m_keys.push_back( RelatedInfoNamed::PT_COMB_3_6 );
    m_keys.push_back( RelatedInfoNamed::PT_COMB_4_5 );
    m_keys.push_back( RelatedInfoNamed::PT_COMB_4_6 );
    m_keys.push_back( RelatedInfoNamed::PT_COMB_5_6 );

    m_keys.push_back( RelatedInfoNamed::P_COMB_1_2 );
    m_keys.push_back( RelatedInfoNamed::P_COMB_1_3 );
    m_keys.push_back( RelatedInfoNamed::P_COMB_1_4 );
    m_keys.push_back( RelatedInfoNamed::P_COMB_1_5 );
    m_keys.push_back( RelatedInfoNamed::P_COMB_1_6 );
    m_keys.push_back( RelatedInfoNamed::P_COMB_2_3 );
    m_keys.push_back( RelatedInfoNamed::P_COMB_2_4 );
    m_keys.push_back( RelatedInfoNamed::P_COMB_2_5 );
    m_keys.push_back( RelatedInfoNamed::P_COMB_2_6 );
    m_keys.push_back( RelatedInfoNamed::P_COMB_3_4 );
    m_keys.push_back( RelatedInfoNamed::P_COMB_3_5 );
    m_keys.push_back( RelatedInfoNamed::P_COMB_3_6 );
    m_keys.push_back( RelatedInfoNamed::P_COMB_4_5 );
    m_keys.push_back( RelatedInfoNamed::P_COMB_4_6 );
    m_keys.push_back( RelatedInfoNamed::P_COMB_5_6 );

    m_keys.push_back( RelatedInfoNamed::NCOMBINATIONS );
  } else {
    for ( const auto &var : m_variables ) {
      short int key = RelatedInfoNamed::indexByName( var );
      if ( key != RelatedInfoNamed::UNKNOWN ) {
        m_keys.push_back( key );
        if ( msgLevel( MSG::DEBUG ) )
          debug() << "Adding variable " << var << ", key = " << key << endmsg;
      } else {
        warning() << "Unknown variable " << var << ", skipping" << endmsg;
      }
    }
  }

  info() << "NLT threshold in RelInfoHLT1Emulation tool is : " << m_nltValue
         << endmsg;

  return sc;
}

//=============================================================================
// Save the particles in the decay chain (recursive function)
//=============================================================================
void RelInfoHLT1Emulation::getAllDaughters(
    const LHCb::Particle *               top,
    std::vector<const LHCb::Particle *> &particlesVector ) {
  // -- Get all the daughters of the top particle
  const SmartRefVector<LHCb::Particle> daughters = top->daughters();

  // -- Insert all the daughters in m_decayParticles
  for ( SmartRefVector<LHCb::Particle>::const_iterator ip = daughters.begin();
        ip != daughters.end(); ++ip ) {
    // -- If the particle is stable, save it in the vector, or...
    if ( ( *ip )->isBasicParticle() ) {
      if ( msgLevel( MSG::DEBUG ) )
        debug() << "Filling particle with ID " << ( *ip )->particleID().pid()
                << endmsg;
      particlesVector.push_back( ( *ip ) );
    }
    // -- ...if it is not stable, call the function recursively
    else {
      getAllDaughters( ( *ip ).data(), particlesVector );
    }
  }

  return;
}

//=============================================================================
// Get the best PVs associated to the particle, i.e. the one wrt which
// the particle has the minimum IPChi2
//=============================================================================
void RelInfoHLT1Emulation::getBestPVs(
    const std::vector<const LHCb::Particle *> &particles,
    std::vector<const LHCb::VertexBase *> &    vertexVector ) {
  // Loop over all the particles and get the best PVs for each of them
  for ( std::vector<const LHCb::Particle *>::const_iterator ip =
            particles.begin();
        ip != particles.end(); ++ip ) {
    if ( ( *ip ) == NULL )
      vertexVector.push_back( NULL );
    else {
      vertexVector.push_back( m_dva->bestVertex( ( *ip ) ) );
      if ( vertexVector.back() ) {
        if ( msgLevel( MSG::VERBOSE ) )
          verbose() << "Got best PV of particle: " << *( vertexVector.back() )
                    << endmsg;
      } else
        warning() << "Could not retreive Best Vertex" << endmsg;
    }
  }
  return;
}

//=============================================================================
// Save all the Primary Vertices found in the event
//=============================================================================
void RelInfoHLT1Emulation::getAllPVs(
    std::vector<const LHCb::VertexBase *> &PVs ) {
  const LHCb::RecVertex::Range PrimaryVertices = m_dva->primaryVertices();

  if ( PrimaryVertices.empty() ) {
    warning() << "Could not find any primary vertex in the event" << endmsg;
    return;
  } else {
    for ( LHCb::RecVertex::Range::const_iterator pv = PrimaryVertices.begin();
          pv != PrimaryVertices.end(); ++pv ) {
      PVs.push_back( ( *pv ) );
    }
  }

  return;
}

//=============================================================================
// Get Minimum IP PV associated to the particle, i.e. the one wrt which
// the particle has the minimum IP
//=============================================================================
void RelInfoHLT1Emulation::getMinIPPVs(
    const std::vector<const LHCb::Particle *> &  particles,
    const std::vector<const LHCb::VertexBase *> &PVs,
    std::vector<const LHCb::VertexBase *> &      vertexVector ) {
  LHCb::VertexBase *minIPVtx = NULL;

  double Ip = 0, Chi2 = 0;
  double minIp;

  for ( std::vector<const LHCb::Particle *>::const_iterator ip =
            particles.begin();
        ip != particles.end(); ++ip ) {
    if ( ( *ip ) == NULL )
      vertexVector.push_back( NULL );
    else {
      minIp = 10000000.;
      for ( std::vector<const LHCb::VertexBase *>::const_iterator iv =
                PVs.begin();
            iv != PVs.end(); ++iv ) {
        m_dist->distance( ( *ip ), ( *iv ), Ip, Chi2 );
        if ( Ip < minIp ) {
          minIPVtx = const_cast<LHCb::VertexBase *>( *iv );
          minIp    = Ip;
        }
      }
      vertexVector.push_back( minIPVtx );
    }
  }

  return;
}

//================================================================================
// Save the number of particles you have in the vector
//================================================================================
void RelInfoHLT1Emulation::setNumber(
    const std::vector<const LHCb::Particle *> &particles, int &nParticles ) {
  nParticles = 0;
  for ( std::vector<const LHCb::Particle *>::const_iterator ip =
            particles.begin();
        ip != particles.end(); ++ip ) {
    if ( ( *ip ) != NULL ) {
      ++nParticles;
    }
  }

  return;
}

//=============================================================================
// Save the transverse momentum of each particle in the vector
//=============================================================================
void RelInfoHLT1Emulation::setTransverseMomenta(
    const std::vector<const LHCb::Particle *> &particlesVector,
    std::vector<float> &                       valuesVector ) {
  // Loop over the entries of the particles vector and push their transverse
  // momentum
  for ( std::vector<const LHCb::Particle *>::const_iterator ip =
            particlesVector.begin();
        ip != particlesVector.end(); ++ip ) {
    if ( ( *ip ) == NULL )
      valuesVector.push_back( -500. );
    else
      valuesVector.push_back( ( *ip )->pt() );
  }

  return;
}

//=============================================================================
// Save the momentum of each particle in the vector
//=============================================================================
void RelInfoHLT1Emulation::setMomenta(
    const std::vector<const LHCb::Particle *> &particlesVector,
    std::vector<float> &                       valuesVector ) {
  // Loop over the entries of the particles vector and push their transverse
  // momentum
  for ( std::vector<const LHCb::Particle *>::const_iterator ip =
            particlesVector.begin();
        ip != particlesVector.end(); ++ip ) {
    if ( ( *ip ) == NULL )
      valuesVector.push_back( -500. );
    else
      valuesVector.push_back( ( *ip )->p() );
  }

  return;
}

//==============================================================================
// Save the IPChi2 of each particle wrt the associated vertex
//==============================================================================
void RelInfoHLT1Emulation::setIPChi2(
    const std::vector<const LHCb::Particle *> &  particlesVector,
    const std::vector<const LHCb::VertexBase *> &vertexVector,
    std::vector<float> &                         valuesVector ) {
  std::vector<const LHCb::Particle *>::const_iterator ip =
      particlesVector.begin();
  std::vector<const LHCb::VertexBase *>::const_iterator iv =
      vertexVector.begin();

  // double Ip=0, Chi2=0;
  for ( ; ip != particlesVector.end(); ) {
    if ( ( ( *ip ) == NULL ) || ( ( *iv ) == NULL ) )
      valuesVector.push_back( -500. );
    else {
      // m_dist->distance((*ip), (*iv), Ip, Chi2);
      // valuesVector.push_back(Chi2);
      LoKi::Particles::ImpParChi2WithTheBestPV fun =
          LoKi::Particles::ImpParChi2WithTheBestPV();
      valuesVector.push_back( fun( *ip ) );
    }
    ++ip;
    ++iv;
  }
  return;
}

//===============================================================================
// Save the Ghost Probability of each stable daughter
//===============================================================================
void RelInfoHLT1Emulation::setTrackGhostProb(
    const std::vector<const LHCb::Particle *> &particlesVector,
    std::vector<float> &                       valuesVector ) {
  for ( std::vector<const LHCb::Particle *>::const_iterator it =
            particlesVector.begin();
        it != particlesVector.end(); ++it ) {
    if ( ( *it ) == NULL )
      valuesVector.push_back( -1000. );
    else {
      if ( isBasicParticle( ( *it ) ) ) {
        valuesVector.push_back(
            ( *it )->proto()->track()->ghostProbability() );
      } else
        valuesVector.push_back( -500. );
    }
  }
  return;
}

//===============================================================================
// Save the Track Chi2 of each stable daughter
//===============================================================================
void RelInfoHLT1Emulation::setTrackChi2(
    const std::vector<const LHCb::Particle *> &particlesVector,
    std::vector<float> &                       valuesVector ) {
  for ( std::vector<const LHCb::Particle *>::const_iterator it =
            particlesVector.begin();
        it != particlesVector.end(); ++it ) {
    if ( ( *it ) == NULL )
      valuesVector.push_back( -1000. );
    else {
      if ( isBasicParticle( ( *it ) ) ) {
        valuesVector.push_back( ( *it )->proto()->track()->chi2() );
      } else
        valuesVector.push_back( -500. );
    }
  }

  return;
}

//===============================================================================
// Save the Track NDOF of each stable daughter
//===============================================================================
void RelInfoHLT1Emulation::setTrackNDOF(
    const std::vector<const LHCb::Particle *> &particlesVector,
    std::vector<float> &                       valuesVector ) {
  for ( std::vector<const LHCb::Particle *>::const_iterator it =
            particlesVector.begin();
        it != particlesVector.end(); ++it ) {
    if ( ( *it ) == NULL )
      valuesVector.push_back( -1000. );
    else {
      if ( isBasicParticle( ( *it ) ) ) {
        valuesVector.push_back( ( *it )->proto()->track()->nDoF() );
      } else
        valuesVector.push_back( -500. );
    }
  }

  return;
}

//===============================================================================
// Change the PID of a particle
//===============================================================================
LHCb::Particle *RelInfoHLT1Emulation::changePIDHypothesis(
    const LHCb::Particle *part ) {
  LHCb::Particle *newPart = part->clone();

  // if it is basic and has a track, change the PID to pion,
  // else do not change it
  if ( isBasicParticle( newPart ) ) {
    int sign =
        newPart->particleID().pid() / fabs( newPart->particleID().pid() );
    newPart->setParticleID( LHCb::ParticleID( 211 * sign ) );
  }

  return newPart;
}

//===============================================================================
// Save all the two-particles combinations in a vector, without repeating.
// if changePIDtoPions is set to true, change PID to charged pions to each
// stable particle before
//===============================================================================
void RelInfoHLT1Emulation::getAllCombinations(
    std::vector<const LHCb::Particle *> &daughters,
    std::vector<const LHCb::Particle *> &mothers,
    std::vector<const LHCb::Vertex *> &  vertices,
    bool                                 changePIDtoPions = false ) {
  //--Make a loop over the possible combinations, without repeating
  for ( std::vector<const LHCb::Particle *>::const_iterator ip1 =
            daughters.begin();
        ip1 + 1 != daughters.end(); ++ip1 ) {
    for ( std::vector<const LHCb::Particle *>::const_iterator ip2 = ip1 + 1;
          ip2 != daughters.end(); ++ip2 ) {
      if ( ( ( *ip1 ) == NULL ) || ( ( *ip2 ) == NULL ) ) {
        mothers.push_back( NULL );
        vertices.push_back( NULL );
      } else {
        LHCb::Particle *mother     = new LHCb::Particle;
        LHCb::Vertex *  mother_vtx = new LHCb::Vertex;

        LHCb::Particle::ConstVector toBeCombined;

        if ( changePIDtoPions ) {
          toBeCombined.push_back( changePIDHypothesis( *ip1 ) );
          toBeCombined.push_back( changePIDHypothesis( *ip2 ) );
        } else {
          toBeCombined.push_back( ( *ip1 ) );
          toBeCombined.push_back( ( *ip2 ) );
        }

        StatusCode sc =
            m_pCombiner->combine( toBeCombined, *mother, *mother_vtx );

        if ( sc.isFailure() ) {
          warning()
              << "Error in V0 -> P P fit in HLT1Emulation RelatedInfo tool."
              << endmsg;
          mothers.push_back( NULL );
          vertices.push_back( NULL );
        } else {
          mothers.push_back( const_cast<LHCb::Particle *>( mother ) );
          vertices.push_back( const_cast<LHCb::Vertex *>( mother_vtx ) );
        }
      }
    }
  }
  return;
}

//===============================================================================
// Save the Vertex Chi2 of each vertex
//===============================================================================
void RelInfoHLT1Emulation::setVertexChi2(
    const std::vector<const LHCb::Vertex *> &verticesVector,
    std::vector<float> &                     valuesVector ) {
  for ( std::vector<const LHCb::Vertex *>::const_iterator iv =
            verticesVector.begin();
        iv != verticesVector.end(); ++iv ) {
    if ( ( *iv ) == NULL )
      valuesVector.push_back( -500. );
    else {
      valuesVector.push_back( ( *iv )->chi2() );
    }
  }
  return;
}

//===============================================================================
// Save the Vertex Ndof of each vertex
//===============================================================================
void RelInfoHLT1Emulation::setVertexDof(
    const std::vector<const LHCb::Vertex *> &verticesVector,
    std::vector<float> &                     valuesVector ) {
  for ( std::vector<const LHCb::Vertex *>::const_iterator iv =
            verticesVector.begin();
        iv != verticesVector.end(); ++iv ) {
    if ( ( *iv ) == NULL )
      valuesVector.push_back( -20. );
    else {
      valuesVector.push_back( ( *iv )->nDoF() );
    }
  }
  return;
}

//===============================================================================
// Save the pseudorapidity of the combinations
//===============================================================================
void RelInfoHLT1Emulation::setEta(
    const std::vector<const LHCb::Particle *> &particlesVector,
    std::vector<float> &                       valuesVector ) {
  for ( std::vector<const LHCb::Particle *>::const_iterator ip =
            particlesVector.begin();
        ip != particlesVector.end(); ++ip ) {
    if ( ( *ip ) == NULL )
      valuesVector.push_back( -5. );
    else {
      const LoKi::Particles::PseudoRapidityWithTheBestPV fun =
          LoKi::Particles::PseudoRapidityWithTheBestPV();
      valuesVector.push_back( fun( *ip ) );
    }
  }
  return;
}

//===============================================================================
// Function used to get the corrected mass
//===============================================================================
double RelInfoHLT1Emulation::MCorr( TLorentzVector &Y,
                                    TVector3 &      M_dirn ) const {
  double P_T = ( Y.Vect() ).Perp( M_dirn );
  return sqrt( Y.M2() + P_T * P_T ) + P_T;
}

//===============================================================================
// Function used to get the DIRA
//===============================================================================
double RelInfoHLT1Emulation::dira( const LHCb::Particle *  P,
                                   const LHCb::VertexBase *V ) const {
  const LHCb::Vertex *    evtx = P->endVertex();
  const Gaudi::XYZVector &A    = P->momentum().Vect();
  const Gaudi::XYZVector  B    = evtx->position() - V->position();

  return A.Dot( B ) / std::sqrt( A.Mag2() * B.Mag2() );
}

//===============================================================================
// Set the corrected mass wrt to a given vertex
//===============================================================================
void RelInfoHLT1Emulation::setMCorr(
    const std::vector<const LHCb::Particle *> &  particlesVector,
    const std::vector<const LHCb::VertexBase *> &verticesVector,
    std::vector<float> &                         valuesVector ) {
  // iterators
  std::vector<const LHCb::Particle *>::const_iterator ip =
      particlesVector.begin();
  std::vector<const LHCb::VertexBase *>::const_iterator iv =
      verticesVector.begin();

  // double correctedMass = 0;

  for ( ; ip != particlesVector.end(); ) {
    if ( ( ( *ip ) == NULL ) || ( ( *iv ) == NULL ) )
      valuesVector.push_back( -100. );
    else {
      /*
      Gaudi::LorentzVector LV_P = (*ip)->momentum();
      TLorentzVector TLV_P (LV_P.px(), LV_P.py(), LV_P.pz(), LV_P.E());
      const LHCb::Vertex* SV    = (*ip)->endVertex();
      TVector3 TV3_PV((*iv)->position().X(), (*iv)->position().Y(),
      (*iv)->position().Z()); TVector3 TV3_SV(SV->position().X(),
      SV->position().Y(),    SV->position().Z());

      TVector3 M_dirn = (TV3_SV - TV3_PV).Unit();

      correctedMass = MCorr(TLV_P, M_dirn);
      valuesVector.push_back(correctedMass);
      */
      LoKi::Particles::MCorrectedWithBestVertex fun =
          LoKi::Particles::MCorrectedWithBestVertex();
      valuesVector.push_back( fun( *ip ) );
    }
    ++ip;
    ++iv;
  }

  return;
}

//===============================================================================
// Set the sum of the PT of the daughters
//===============================================================================
void RelInfoHLT1Emulation::setDaughtersSumPT(
    const std::vector<const LHCb::Particle *> &particlesVector,
    std::vector<float> &                       valuesVector ) {
  for ( std::vector<const LHCb::Particle *>::const_iterator ip =
            particlesVector.begin();
        ip != particlesVector.end(); ++ip ) {
    if ( ( *ip ) == NULL )
      valuesVector.push_back( -100. );
    else {
      /*
      const SmartRefVector< LHCb::Particle > daughters = (*ip)->daughters();

      float SumPT = 0.;

      for ( SmartRefVector< LHCb::Particle >::const_iterator id =
      daughters.begin(); id != daughters.end(); ++id)
      {
        SumPT += (*id)->pt();
      }
      */
      LoKi::Particles::IsBasic            ISBASIC = LoKi::Particles::IsBasic();
      LoKi::Particles::TransverseMomentum PT =
          LoKi::Particles::TransverseMomentum();
      LoKi::Particles::SumTree SUMTREE =
          LoKi::Particles::SumTree( PT, ISBASIC, 0.0 );

      // valuesVector.push_back(SumPT);

      valuesVector.push_back( SUMTREE( *ip ) );
    }
  }

  return;
}

//===============================================================================
// Set the DIRA wrt a given vertex
//===============================================================================
void RelInfoHLT1Emulation::setDira(
    const std::vector<const LHCb::Particle *> &  particlesVector,
    const std::vector<const LHCb::VertexBase *> &verticesVector,
    std::vector<float> &                         valuesVector ) {
  // iterators
  std::vector<const LHCb::Particle *>::const_iterator ip =
      particlesVector.begin();
  std::vector<const LHCb::VertexBase *>::const_iterator iv =
      verticesVector.begin();

  // double Dira = 0;

  for ( ; ip != particlesVector.end(); ) {
    if ( ( ( *ip ) == NULL ) || ( ( *iv ) == NULL ) )
      valuesVector.push_back( -100. );
    else {
      /*
      Dira = dira((*ip), (*iv));
      valuesVector.push_back(Dira);
      */
      LoKi::Particles::CosineDirectionAngleWithTheBestPV fun =
          LoKi::Particles::CosineDirectionAngleWithTheBestPV();
      valuesVector.push_back( fun( *ip ) );
    }
    ++ip;
    ++iv;
  }

  return;
}

//===============================================================================
// Set the DOCA of the daughters of two particle combinations
//===============================================================================
void RelInfoHLT1Emulation::setDOCACHI2(
    const std::vector<const LHCb::Particle *> &particlesVector,
    std::vector<float> &                       valuesVector ) {
  double dist = 0, chi2 = 0;

  for ( std::vector<const LHCb::Particle *>::const_iterator ip =
            particlesVector.begin();
        ip != particlesVector.end(); ++ip ) {
    if ( ( *ip ) == NULL )
      valuesVector.push_back( -100. );
    else {
      // Get the first and the second daughters;
      const SmartRefVector<LHCb::Particle> daughters = ( *ip )->daughters();
      const LHCb::Particle *               daughter1 = daughters[0];
      const LHCb::Particle *               daughter2 = daughters[1];

      StatusCode sc = m_dist->distance( daughter1, daughter2, dist, chi2 );

      if ( sc.isFailure() ) {
        warning()
            << "Error in DOCA evaluation in HLT1Emulation RelatedInfo tool."
            << endmsg;
        valuesVector.push_back( -200. );
      } else
        valuesVector.push_back( chi2 );
    }
  }
  return;
}

//===============================================================================
// Set the VDCHI2 wrt a given vertex
//===============================================================================
void RelInfoHLT1Emulation::setVDCHI2(
    const std::vector<const LHCb::Particle *> &  particlesVector,
    const std::vector<const LHCb::VertexBase *> &verticesVector,
    std::vector<float> &                         valuesVector ) {
  // iterators
  std::vector<const LHCb::Particle *>::const_iterator ip =
      particlesVector.begin();
  std::vector<const LHCb::VertexBase *>::const_iterator iv =
      verticesVector.begin();

  // double VDistance=0, VDistanceChi2=0;

  for ( ; ip != particlesVector.end(); ) {
    if ( ( ( *ip ) == NULL ) || ( ( *iv ) == NULL ) )
      valuesVector.push_back( -100. );
    else {
      /*StatusCode sc = m_dist->distance((*iv), (*ip)->endVertex(), VDistance,
      VDistanceChi2); if (sc.isFailure())
      {
        warning() << "Error in VDCHI2 evaluation in HLT1Emulation RelatedInfo
      tool." << endmsg; valuesVector.push_back(-200.);
      }
      else
      {
        valuesVector.push_back(VDistanceChi2);
      }*/
      LoKi::Particles::VertexChi2DistanceDV fun =
          LoKi::Particles::VertexChi2DistanceDV();
      valuesVector.push_back( fun( *ip ) );
    }
    ++ip;
    ++iv;
  }

  return;
}

//===============================================================================
// Set the NLT variable used by HLT1TrackMVA
//===============================================================================
void RelInfoHLT1Emulation::setnlt( const std::vector<float> &ipchi2Vector,
                                   std::vector<float> &      valuesVector ) {
  float nlt = 0.;
  //--Make a loop over the possible combinations, without repeating
  for ( std::vector<float>::const_iterator ip1 = ipchi2Vector.begin();
        ip1 + 1 != ipchi2Vector.end(); ++ip1 ) {
    for ( std::vector<float>::const_iterator ip2 = ip1 + 1;
          ip2 != ipchi2Vector.end(); ++ip2 ) {
      if ( ( ( *ip1 ) < 0 ) || ( ( *ip2 ) < 0 ) )
        valuesVector.push_back( -1. );
      else {
        // For each combination I count the number of tracks with IPChi2 < 16
        nlt = 0.;
        if ( ( ( *ip1 ) < m_nltValue ) ) nlt += 1;  // Make it configurable
        if ( ( ( *ip2 ) < m_nltValue ) ) nlt += 1;  // Make it configurable
        valuesVector.push_back( nlt );
      }
    }
  }
  return;
}

//=============================================================================
// Get all the informations
//=============================================================================
void RelInfoHLT1Emulation::saveRelatedInfo( const LHCb::Particle *part ) {
  //--Get all the objects in the event

  std::vector<const LHCb::Particle *> daughters_temp;

  // std::cout << "Getting all the daughters" << std::endl;
  getAllDaughters( part, daughters_temp );
  // getAllDaughters(part, m_decayParticles);
  // std::cout << "Getting all the PVs" << std::endl;
  getAllPVs( m_primaryVertices );

  // --- If the number of stable daughters is greater than MAXNUMBER, then
  // prune away some of the entries
  if ( daughters_temp.size() > MAXNUMBER ) {
    warning() << "Number of daughters is " << m_decayParticles.size()
              << ". Tool is written for";
    warning() << " a maximum of MAXNUMBER. SOME INFO WON'T BE SAVED!"
              << endmsg;
    for ( unsigned int i = 0; i < MAXNUMBER; ++i ) {
      m_decayParticles.push_back( daughters_temp[i] );
    }
  }

  // --- If the number of stable daughters is less than MAXNUMBER, then insert
  // some NULL pointers
  if ( daughters_temp.size() < MAXNUMBER ) {
    for ( unsigned int i = 0; i < daughters_temp.size(); ++i ) {
      m_decayParticles.push_back( daughters_temp[i] );
    }
    for ( unsigned int i = daughters_temp.size(); i < MAXNUMBER; ++i ) {
      m_decayParticles.push_back( NULL );
    }
  }

  //---------------------------------------------------------------------------
  // STABLE DAUGHTERS
  //---------------------------------------------------------------------------
  //--Get the PVs associated to each of the tracks
  getBestPVs( m_decayParticles, m_decayParticles_bestPVs );
  getMinIPPVs( m_decayParticles, m_primaryVertices,
               m_decayParticles_MinIPPVs );

  //--Evaluate the associated quantities
  setNumber( m_decayParticles, m_NDAUGHTERS );
  setTransverseMomenta( m_decayParticles, m_DAUGHTERSMOMENTA );
  setMomenta( m_decayParticles, m_DAUGHTERSMOMENTA_P );

  setIPChi2( m_decayParticles, m_decayParticles_bestPVs,
             m_DAUGHTERSIPCHI2BEST );
  setIPChi2( m_decayParticles, m_decayParticles_MinIPPVs,
             m_DAUGHTERSIPCHI2SMALLEST );
  setTrackGhostProb( m_decayParticles, m_DAUGHTERSTRACKGHOSTPROB );
  setTrackChi2( m_decayParticles, m_DAUGHTERSTRACKCHI2 );
  setTrackNDOF( m_decayParticles, m_DAUGHTERSTRACKNDOF );

  //--------------------------------------------------------------------------
  // TWO PARTICLES COMBINATIONS
  //--------------------------------------------------------------------------
  //--Get all the combinations, changing the PID hypothesis of the daughters
  getAllCombinations( m_decayParticles, m_motherParticles, m_motherVertices,
                      true );

  //--Get the PVs associated to each of the combinations
  getBestPVs( m_motherParticles, m_combinationsParticles_bestPVs );
  getMinIPPVs( m_motherParticles, m_primaryVertices,
               m_combinationsParticles_MinIPPvs );

  //--Evaluate the associated quantities
  setTransverseMomenta( m_motherParticles, m_COMBINATIONS_PT );
  setMomenta( m_motherParticles, m_COMBINATIONS_P );
  setNumber( m_motherParticles, m_NCOMBINATIONS );
  setVertexChi2( m_motherVertices, m_COMBINATIONSVCHI2 );
  setVertexDof( m_motherVertices, m_COMBINATIONSVNDOF );
  setEta( m_motherParticles, m_COMBINATIONSETA );
  setMCorr( m_motherParticles, m_combinationsParticles_bestPVs,
            m_COMBINATIONSMCORR_BPV );
  setMCorr( m_motherParticles, m_combinationsParticles_MinIPPvs,
            m_COMBINATIONSMCORR_MINIPPV );
  setDaughtersSumPT( m_motherParticles, m_COMBINATIONSSUMPT );
  setDira( m_motherParticles, m_combinationsParticles_bestPVs,
           m_COMBINATIONSDIRA_BPV );
  setDira( m_motherParticles, m_combinationsParticles_MinIPPvs,
           m_COMBINATIONSDIRA_MINIPPV );
  setDOCACHI2( m_motherParticles, m_COMBINATIONSDOCACHI2 );
  setVDCHI2( m_motherParticles, m_combinationsParticles_bestPVs,
             m_COMBINATIONSBVVDCHI2 );
  setVDCHI2( m_motherParticles, m_combinationsParticles_MinIPPvs,
             m_COMBINATIONSMINIPVDCHI2 );
  setIPChi2( m_motherParticles, m_combinationsParticles_bestPVs,
             m_COMBINATIONSBPVIPCHI2 );
  setIPChi2( m_motherParticles, m_combinationsParticles_MinIPPvs,
             m_COMBINATIONSMINIPCHI2 );
  setnlt( m_DAUGHTERSIPCHI2BEST, m_COMBINATIONSNLT16_BPV );
  setnlt( m_DAUGHTERSIPCHI2SMALLEST, m_COMBINATIONSNLT16_MINIPPV );

  return;
}

//=============================================================================
// Fill the RelatedInfo Map
//=============================================================================
StatusCode RelInfoHLT1Emulation::calculateRelatedInfo(
    const LHCb::Particle *, const LHCb::Particle *part ) {
  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Fill" << endmsg;

  // Let's not allow the tools to be run on basic particles or photons
  if ( part->isBasicParticle() || isPureNeutralCalo( part ) ) {
    if ( msgLevel( MSG::DEBUG ) )
      debug()
          << "Trying to fill HLT1Emulataion map for basic particle. Exiting..."
          << endmsg;
    m_map.clear();
    return StatusCode::SUCCESS;
  }

  // For each candidate you are looking at, clear the vector
  clearVectors();

  saveRelatedInfo( part );

  m_map.clear();

  for ( const auto key : m_keys ) {
    float value = 0;
    switch ( key ) {
      case RelatedInfoNamed::NDAUGHTERS:
        value = m_NDAUGHTERS;
        break;
      case RelatedInfoNamed::NCOMBINATIONS:
        value = m_NCOMBINATIONS;
        break;

      case RelatedInfoNamed::PT_DAU_1:
        value = m_DAUGHTERSMOMENTA[0];
        break;
      case RelatedInfoNamed::PT_DAU_2:
        value = m_DAUGHTERSMOMENTA[1];
        break;
      case RelatedInfoNamed::PT_DAU_3:
        value = m_DAUGHTERSMOMENTA[2];
        break;
      case RelatedInfoNamed::PT_DAU_4:
        value = m_DAUGHTERSMOMENTA[3];
        break;
      case RelatedInfoNamed::PT_DAU_5:
        value = m_DAUGHTERSMOMENTA[4];
        break;
      case RelatedInfoNamed::PT_DAU_6:
        value = m_DAUGHTERSMOMENTA[5];
        break;

      case RelatedInfoNamed::P_DAU_1:
        value = m_DAUGHTERSMOMENTA_P[0];
        break;
      case RelatedInfoNamed::P_DAU_2:
        value = m_DAUGHTERSMOMENTA_P[1];
        break;
      case RelatedInfoNamed::P_DAU_3:
        value = m_DAUGHTERSMOMENTA_P[2];
        break;
      case RelatedInfoNamed::P_DAU_4:
        value = m_DAUGHTERSMOMENTA_P[3];
        break;
      case RelatedInfoNamed::P_DAU_5:
        value = m_DAUGHTERSMOMENTA_P[4];
        break;
      case RelatedInfoNamed::P_DAU_6:
        value = m_DAUGHTERSMOMENTA_P[5];
        break;

      case RelatedInfoNamed::IPCHI2_OWNPV_DAU_1:
        value = m_DAUGHTERSIPCHI2BEST[0];
        break;
      case RelatedInfoNamed::IPCHI2_OWNPV_DAU_2:
        value = m_DAUGHTERSIPCHI2BEST[1];
        break;
      case RelatedInfoNamed::IPCHI2_OWNPV_DAU_3:
        value = m_DAUGHTERSIPCHI2BEST[2];
        break;
      case RelatedInfoNamed::IPCHI2_OWNPV_DAU_4:
        value = m_DAUGHTERSIPCHI2BEST[3];
        break;
      case RelatedInfoNamed::IPCHI2_OWNPV_DAU_5:
        value = m_DAUGHTERSIPCHI2BEST[4];
        break;
      case RelatedInfoNamed::IPCHI2_OWNPV_DAU_6:
        value = m_DAUGHTERSIPCHI2BEST[5];
        break;

      case RelatedInfoNamed::IPCHI2_MINIPPV_DAU_1:
        value = m_DAUGHTERSIPCHI2SMALLEST[0];
        break;
      case RelatedInfoNamed::IPCHI2_MINIPPV_DAU_2:
        value = m_DAUGHTERSIPCHI2SMALLEST[1];
        break;
      case RelatedInfoNamed::IPCHI2_MINIPPV_DAU_3:
        value = m_DAUGHTERSIPCHI2SMALLEST[2];
        break;
      case RelatedInfoNamed::IPCHI2_MINIPPV_DAU_4:
        value = m_DAUGHTERSIPCHI2SMALLEST[3];
        break;
      case RelatedInfoNamed::IPCHI2_MINIPPV_DAU_5:
        value = m_DAUGHTERSIPCHI2SMALLEST[4];
        break;
      case RelatedInfoNamed::IPCHI2_MINIPPV_DAU_6:
        value = m_DAUGHTERSIPCHI2SMALLEST[5];
        break;

      case RelatedInfoNamed::TRACK_GHOSTPROB_DAU_1:
        value = m_DAUGHTERSTRACKGHOSTPROB[0];
        break;
      case RelatedInfoNamed::TRACK_GHOSTPROB_DAU_2:
        value = m_DAUGHTERSTRACKGHOSTPROB[1];
        break;
      case RelatedInfoNamed::TRACK_GHOSTPROB_DAU_3:
        value = m_DAUGHTERSTRACKGHOSTPROB[2];
        break;
      case RelatedInfoNamed::TRACK_GHOSTPROB_DAU_4:
        value = m_DAUGHTERSTRACKGHOSTPROB[3];
        break;
      case RelatedInfoNamed::TRACK_GHOSTPROB_DAU_5:
        value = m_DAUGHTERSTRACKGHOSTPROB[4];
        break;
      case RelatedInfoNamed::TRACK_GHOSTPROB_DAU_6:
        value = m_DAUGHTERSTRACKGHOSTPROB[5];
        break;

      case RelatedInfoNamed::TRACK_CHI2_DAU_1:
        value = m_DAUGHTERSTRACKCHI2[0];
        break;
      case RelatedInfoNamed::TRACK_CHI2_DAU_2:
        value = m_DAUGHTERSTRACKCHI2[1];
        break;
      case RelatedInfoNamed::TRACK_CHI2_DAU_3:
        value = m_DAUGHTERSTRACKCHI2[2];
        break;
      case RelatedInfoNamed::TRACK_CHI2_DAU_4:
        value = m_DAUGHTERSTRACKCHI2[3];
        break;
      case RelatedInfoNamed::TRACK_CHI2_DAU_5:
        value = m_DAUGHTERSTRACKCHI2[4];
        break;
      case RelatedInfoNamed::TRACK_CHI2_DAU_6:
        value = m_DAUGHTERSTRACKCHI2[5];
        break;

      case RelatedInfoNamed::TRACK_NDOF_DAU_1:
        value = m_DAUGHTERSTRACKNDOF[0];
        break;
      case RelatedInfoNamed::TRACK_NDOF_DAU_2:
        value = m_DAUGHTERSTRACKNDOF[1];
        break;
      case RelatedInfoNamed::TRACK_NDOF_DAU_3:
        value = m_DAUGHTERSTRACKNDOF[2];
        break;
      case RelatedInfoNamed::TRACK_NDOF_DAU_4:
        value = m_DAUGHTERSTRACKNDOF[3];
        break;
      case RelatedInfoNamed::TRACK_NDOF_DAU_5:
        value = m_DAUGHTERSTRACKNDOF[4];
        break;
      case RelatedInfoNamed::TRACK_NDOF_DAU_6:
        value = m_DAUGHTERSTRACKNDOF[5];
        break;

      case RelatedInfoNamed::VERTEX_CHI2_COMB_1_2:
        value = m_COMBINATIONSVCHI2[0];
        break;
      case RelatedInfoNamed::VERTEX_CHI2_COMB_1_3:
        value = m_COMBINATIONSVCHI2[1];
        break;
      case RelatedInfoNamed::VERTEX_CHI2_COMB_1_4:
        value = m_COMBINATIONSVCHI2[2];
        break;
      case RelatedInfoNamed::VERTEX_CHI2_COMB_1_5:
        value = m_COMBINATIONSVCHI2[3];
        break;
      case RelatedInfoNamed::VERTEX_CHI2_COMB_1_6:
        value = m_COMBINATIONSVCHI2[4];
        break;
      case RelatedInfoNamed::VERTEX_CHI2_COMB_2_3:
        value = m_COMBINATIONSVCHI2[5];
        break;
      case RelatedInfoNamed::VERTEX_CHI2_COMB_2_4:
        value = m_COMBINATIONSVCHI2[6];
        break;
      case RelatedInfoNamed::VERTEX_CHI2_COMB_2_5:
        value = m_COMBINATIONSVCHI2[7];
        break;
      case RelatedInfoNamed::VERTEX_CHI2_COMB_2_6:
        value = m_COMBINATIONSVCHI2[8];
        break;
      case RelatedInfoNamed::VERTEX_CHI2_COMB_3_4:
        value = m_COMBINATIONSVCHI2[9];
        break;
      case RelatedInfoNamed::VERTEX_CHI2_COMB_3_5:
        value = m_COMBINATIONSVCHI2[10];
        break;
      case RelatedInfoNamed::VERTEX_CHI2_COMB_3_6:
        value = m_COMBINATIONSVCHI2[11];
        break;
      case RelatedInfoNamed::VERTEX_CHI2_COMB_4_5:
        value = m_COMBINATIONSVCHI2[12];
        break;
      case RelatedInfoNamed::VERTEX_CHI2_COMB_4_6:
        value = m_COMBINATIONSVCHI2[13];
        break;
      case RelatedInfoNamed::VERTEX_CHI2_COMB_5_6:
        value = m_COMBINATIONSVCHI2[14];
        break;

      case RelatedInfoNamed::VERTEX_NDOF_COMB_1_2:
        value = m_COMBINATIONSVNDOF[0];
        break;
      case RelatedInfoNamed::VERTEX_NDOF_COMB_1_3:
        value = m_COMBINATIONSVNDOF[1];
        break;
      case RelatedInfoNamed::VERTEX_NDOF_COMB_1_4:
        value = m_COMBINATIONSVNDOF[2];
        break;
      case RelatedInfoNamed::VERTEX_NDOF_COMB_1_5:
        value = m_COMBINATIONSVNDOF[3];
        break;
      case RelatedInfoNamed::VERTEX_NDOF_COMB_1_6:
        value = m_COMBINATIONSVNDOF[4];
        break;
      case RelatedInfoNamed::VERTEX_NDOF_COMB_2_3:
        value = m_COMBINATIONSVNDOF[5];
        break;
      case RelatedInfoNamed::VERTEX_NDOF_COMB_2_4:
        value = m_COMBINATIONSVNDOF[6];
        break;
      case RelatedInfoNamed::VERTEX_NDOF_COMB_2_5:
        value = m_COMBINATIONSVNDOF[7];
        break;
      case RelatedInfoNamed::VERTEX_NDOF_COMB_2_6:
        value = m_COMBINATIONSVNDOF[8];
        break;
      case RelatedInfoNamed::VERTEX_NDOF_COMB_3_4:
        value = m_COMBINATIONSVNDOF[9];
        break;
      case RelatedInfoNamed::VERTEX_NDOF_COMB_3_5:
        value = m_COMBINATIONSVNDOF[10];
        break;
      case RelatedInfoNamed::VERTEX_NDOF_COMB_3_6:
        value = m_COMBINATIONSVNDOF[11];
        break;
      case RelatedInfoNamed::VERTEX_NDOF_COMB_4_5:
        value = m_COMBINATIONSVNDOF[12];
        break;
      case RelatedInfoNamed::VERTEX_NDOF_COMB_4_6:
        value = m_COMBINATIONSVNDOF[13];
        break;
      case RelatedInfoNamed::VERTEX_NDOF_COMB_5_6:
        value = m_COMBINATIONSVNDOF[14];
        break;

      case RelatedInfoNamed::ETA_COMB_1_2:
        value = m_COMBINATIONSETA[0];
        break;
      case RelatedInfoNamed::ETA_COMB_1_3:
        value = m_COMBINATIONSETA[1];
        break;
      case RelatedInfoNamed::ETA_COMB_1_4:
        value = m_COMBINATIONSETA[2];
        break;
      case RelatedInfoNamed::ETA_COMB_1_5:
        value = m_COMBINATIONSETA[3];
        break;
      case RelatedInfoNamed::ETA_COMB_1_6:
        value = m_COMBINATIONSETA[4];
        break;
      case RelatedInfoNamed::ETA_COMB_2_3:
        value = m_COMBINATIONSETA[5];
        break;
      case RelatedInfoNamed::ETA_COMB_2_4:
        value = m_COMBINATIONSETA[6];
        break;
      case RelatedInfoNamed::ETA_COMB_2_5:
        value = m_COMBINATIONSETA[7];
        break;
      case RelatedInfoNamed::ETA_COMB_2_6:
        value = m_COMBINATIONSETA[8];
        break;
      case RelatedInfoNamed::ETA_COMB_3_4:
        value = m_COMBINATIONSETA[9];
        break;
      case RelatedInfoNamed::ETA_COMB_3_5:
        value = m_COMBINATIONSETA[10];
        break;
      case RelatedInfoNamed::ETA_COMB_3_6:
        value = m_COMBINATIONSETA[11];
        break;
      case RelatedInfoNamed::ETA_COMB_4_5:
        value = m_COMBINATIONSETA[12];
        break;
      case RelatedInfoNamed::ETA_COMB_4_6:
        value = m_COMBINATIONSETA[13];
        break;
      case RelatedInfoNamed::ETA_COMB_5_6:
        value = m_COMBINATIONSETA[14];
        break;

      case RelatedInfoNamed::MCORR_OWNPV_COMB_1_2:
        value = m_COMBINATIONSMCORR_BPV[0];
        break;
      case RelatedInfoNamed::MCORR_OWNPV_COMB_1_3:
        value = m_COMBINATIONSMCORR_BPV[1];
        break;
      case RelatedInfoNamed::MCORR_OWNPV_COMB_1_4:
        value = m_COMBINATIONSMCORR_BPV[2];
        break;
      case RelatedInfoNamed::MCORR_OWNPV_COMB_1_5:
        value = m_COMBINATIONSMCORR_BPV[3];
        break;
      case RelatedInfoNamed::MCORR_OWNPV_COMB_1_6:
        value = m_COMBINATIONSMCORR_BPV[4];
        break;
      case RelatedInfoNamed::MCORR_OWNPV_COMB_2_3:
        value = m_COMBINATIONSMCORR_BPV[5];
        break;
      case RelatedInfoNamed::MCORR_OWNPV_COMB_2_4:
        value = m_COMBINATIONSMCORR_BPV[6];
        break;
      case RelatedInfoNamed::MCORR_OWNPV_COMB_2_5:
        value = m_COMBINATIONSMCORR_BPV[7];
        break;
      case RelatedInfoNamed::MCORR_OWNPV_COMB_2_6:
        value = m_COMBINATIONSMCORR_BPV[8];
        break;
      case RelatedInfoNamed::MCORR_OWNPV_COMB_3_4:
        value = m_COMBINATIONSMCORR_BPV[9];
        break;
      case RelatedInfoNamed::MCORR_OWNPV_COMB_3_5:
        value = m_COMBINATIONSMCORR_BPV[10];
        break;
      case RelatedInfoNamed::MCORR_OWNPV_COMB_3_6:
        value = m_COMBINATIONSMCORR_BPV[11];
        break;
      case RelatedInfoNamed::MCORR_OWNPV_COMB_4_5:
        value = m_COMBINATIONSMCORR_BPV[12];
        break;
      case RelatedInfoNamed::MCORR_OWNPV_COMB_4_6:
        value = m_COMBINATIONSMCORR_BPV[13];
        break;
      case RelatedInfoNamed::MCORR_OWNPV_COMB_5_6:
        value = m_COMBINATIONSMCORR_BPV[14];
        break;

      case RelatedInfoNamed::MCORR_MINIPPV_COMB_1_2:
        value = m_COMBINATIONSMCORR_MINIPPV[0];
        break;
      case RelatedInfoNamed::MCORR_MINIPPV_COMB_1_3:
        value = m_COMBINATIONSMCORR_MINIPPV[1];
        break;
      case RelatedInfoNamed::MCORR_MINIPPV_COMB_1_4:
        value = m_COMBINATIONSMCORR_MINIPPV[2];
        break;
      case RelatedInfoNamed::MCORR_MINIPPV_COMB_1_5:
        value = m_COMBINATIONSMCORR_MINIPPV[3];
        break;
      case RelatedInfoNamed::MCORR_MINIPPV_COMB_1_6:
        value = m_COMBINATIONSMCORR_MINIPPV[4];
        break;
      case RelatedInfoNamed::MCORR_MINIPPV_COMB_2_3:
        value = m_COMBINATIONSMCORR_MINIPPV[5];
        break;
      case RelatedInfoNamed::MCORR_MINIPPV_COMB_2_4:
        value = m_COMBINATIONSMCORR_MINIPPV[6];
        break;
      case RelatedInfoNamed::MCORR_MINIPPV_COMB_2_5:
        value = m_COMBINATIONSMCORR_MINIPPV[7];
        break;
      case RelatedInfoNamed::MCORR_MINIPPV_COMB_2_6:
        value = m_COMBINATIONSMCORR_MINIPPV[8];
        break;
      case RelatedInfoNamed::MCORR_MINIPPV_COMB_3_4:
        value = m_COMBINATIONSMCORR_MINIPPV[9];
        break;
      case RelatedInfoNamed::MCORR_MINIPPV_COMB_3_5:
        value = m_COMBINATIONSMCORR_MINIPPV[10];
        break;
      case RelatedInfoNamed::MCORR_MINIPPV_COMB_3_6:
        value = m_COMBINATIONSMCORR_MINIPPV[11];
        break;
      case RelatedInfoNamed::MCORR_MINIPPV_COMB_4_5:
        value = m_COMBINATIONSMCORR_MINIPPV[12];
        break;
      case RelatedInfoNamed::MCORR_MINIPPV_COMB_4_6:
        value = m_COMBINATIONSMCORR_MINIPPV[13];
        break;
      case RelatedInfoNamed::MCORR_MINIPPV_COMB_5_6:
        value = m_COMBINATIONSMCORR_MINIPPV[14];
        break;

      case RelatedInfoNamed::SUMPT_COMB_1_2:
        value = m_COMBINATIONSSUMPT[0];
        break;
      case RelatedInfoNamed::SUMPT_COMB_1_3:
        value = m_COMBINATIONSSUMPT[1];
        break;
      case RelatedInfoNamed::SUMPT_COMB_1_4:
        value = m_COMBINATIONSSUMPT[2];
        break;
      case RelatedInfoNamed::SUMPT_COMB_1_5:
        value = m_COMBINATIONSSUMPT[3];
        break;
      case RelatedInfoNamed::SUMPT_COMB_1_6:
        value = m_COMBINATIONSSUMPT[4];
        break;
      case RelatedInfoNamed::SUMPT_COMB_2_3:
        value = m_COMBINATIONSSUMPT[5];
        break;
      case RelatedInfoNamed::SUMPT_COMB_2_4:
        value = m_COMBINATIONSSUMPT[6];
        break;
      case RelatedInfoNamed::SUMPT_COMB_2_5:
        value = m_COMBINATIONSSUMPT[7];
        break;
      case RelatedInfoNamed::SUMPT_COMB_2_6:
        value = m_COMBINATIONSSUMPT[8];
        break;
      case RelatedInfoNamed::SUMPT_COMB_3_4:
        value = m_COMBINATIONSSUMPT[9];
        break;
      case RelatedInfoNamed::SUMPT_COMB_3_5:
        value = m_COMBINATIONSSUMPT[10];
        break;
      case RelatedInfoNamed::SUMPT_COMB_3_6:
        value = m_COMBINATIONSSUMPT[11];
        break;
      case RelatedInfoNamed::SUMPT_COMB_4_5:
        value = m_COMBINATIONSSUMPT[12];
        break;
      case RelatedInfoNamed::SUMPT_COMB_4_6:
        value = m_COMBINATIONSSUMPT[13];
        break;
      case RelatedInfoNamed::SUMPT_COMB_5_6:
        value = m_COMBINATIONSSUMPT[14];
        break;

      case RelatedInfoNamed::DIRA_OWNPV_COMB_1_2:
        value = m_COMBINATIONSDIRA_BPV[0];
        break;
      case RelatedInfoNamed::DIRA_OWNPV_COMB_1_3:
        value = m_COMBINATIONSDIRA_BPV[1];
        break;
      case RelatedInfoNamed::DIRA_OWNPV_COMB_1_4:
        value = m_COMBINATIONSDIRA_BPV[2];
        break;
      case RelatedInfoNamed::DIRA_OWNPV_COMB_1_5:
        value = m_COMBINATIONSDIRA_BPV[3];
        break;
      case RelatedInfoNamed::DIRA_OWNPV_COMB_1_6:
        value = m_COMBINATIONSDIRA_BPV[4];
        break;
      case RelatedInfoNamed::DIRA_OWNPV_COMB_2_3:
        value = m_COMBINATIONSDIRA_BPV[5];
        break;
      case RelatedInfoNamed::DIRA_OWNPV_COMB_2_4:
        value = m_COMBINATIONSDIRA_BPV[6];
        break;
      case RelatedInfoNamed::DIRA_OWNPV_COMB_2_5:
        value = m_COMBINATIONSDIRA_BPV[7];
        break;
      case RelatedInfoNamed::DIRA_OWNPV_COMB_2_6:
        value = m_COMBINATIONSDIRA_BPV[8];
        break;
      case RelatedInfoNamed::DIRA_OWNPV_COMB_3_4:
        value = m_COMBINATIONSDIRA_BPV[9];
        break;
      case RelatedInfoNamed::DIRA_OWNPV_COMB_3_5:
        value = m_COMBINATIONSDIRA_BPV[10];
        break;
      case RelatedInfoNamed::DIRA_OWNPV_COMB_3_6:
        value = m_COMBINATIONSDIRA_BPV[11];
        break;
      case RelatedInfoNamed::DIRA_OWNPV_COMB_4_5:
        value = m_COMBINATIONSDIRA_BPV[12];
        break;
      case RelatedInfoNamed::DIRA_OWNPV_COMB_4_6:
        value = m_COMBINATIONSDIRA_BPV[13];
        break;
      case RelatedInfoNamed::DIRA_OWNPV_COMB_5_6:
        value = m_COMBINATIONSDIRA_BPV[14];
        break;

      case RelatedInfoNamed::DIRA_MINIPPV_COMB_1_2:
        value = m_COMBINATIONSDIRA_MINIPPV[0];
        break;
      case RelatedInfoNamed::DIRA_MINIPPV_COMB_1_3:
        value = m_COMBINATIONSDIRA_MINIPPV[1];
        break;
      case RelatedInfoNamed::DIRA_MINIPPV_COMB_1_4:
        value = m_COMBINATIONSDIRA_MINIPPV[2];
        break;
      case RelatedInfoNamed::DIRA_MINIPPV_COMB_1_5:
        value = m_COMBINATIONSDIRA_MINIPPV[3];
        break;
      case RelatedInfoNamed::DIRA_MINIPPV_COMB_1_6:
        value = m_COMBINATIONSDIRA_MINIPPV[4];
        break;
      case RelatedInfoNamed::DIRA_MINIPPV_COMB_2_3:
        value = m_COMBINATIONSDIRA_MINIPPV[5];
        break;
      case RelatedInfoNamed::DIRA_MINIPPV_COMB_2_4:
        value = m_COMBINATIONSDIRA_MINIPPV[6];
        break;
      case RelatedInfoNamed::DIRA_MINIPPV_COMB_2_5:
        value = m_COMBINATIONSDIRA_MINIPPV[7];
        break;
      case RelatedInfoNamed::DIRA_MINIPPV_COMB_2_6:
        value = m_COMBINATIONSDIRA_MINIPPV[8];
        break;
      case RelatedInfoNamed::DIRA_MINIPPV_COMB_3_4:
        value = m_COMBINATIONSDIRA_MINIPPV[9];
        break;
      case RelatedInfoNamed::DIRA_MINIPPV_COMB_3_5:
        value = m_COMBINATIONSDIRA_MINIPPV[10];
        break;
      case RelatedInfoNamed::DIRA_MINIPPV_COMB_3_6:
        value = m_COMBINATIONSDIRA_MINIPPV[11];
        break;
      case RelatedInfoNamed::DIRA_MINIPPV_COMB_4_5:
        value = m_COMBINATIONSDIRA_MINIPPV[12];
        break;
      case RelatedInfoNamed::DIRA_MINIPPV_COMB_4_6:
        value = m_COMBINATIONSDIRA_MINIPPV[13];
        break;
      case RelatedInfoNamed::DIRA_MINIPPV_COMB_5_6:
        value = m_COMBINATIONSDIRA_MINIPPV[14];
        break;

      case RelatedInfoNamed::DOCA_COMB_1_2:
        value = m_COMBINATIONSDOCACHI2[0];
        break;
      case RelatedInfoNamed::DOCA_COMB_1_3:
        value = m_COMBINATIONSDOCACHI2[1];
        break;
      case RelatedInfoNamed::DOCA_COMB_1_4:
        value = m_COMBINATIONSDOCACHI2[2];
        break;
      case RelatedInfoNamed::DOCA_COMB_1_5:
        value = m_COMBINATIONSDOCACHI2[3];
        break;
      case RelatedInfoNamed::DOCA_COMB_1_6:
        value = m_COMBINATIONSDOCACHI2[4];
        break;
      case RelatedInfoNamed::DOCA_COMB_2_3:
        value = m_COMBINATIONSDOCACHI2[5];
        break;
      case RelatedInfoNamed::DOCA_COMB_2_4:
        value = m_COMBINATIONSDOCACHI2[6];
        break;
      case RelatedInfoNamed::DOCA_COMB_2_5:
        value = m_COMBINATIONSDOCACHI2[7];
        break;
      case RelatedInfoNamed::DOCA_COMB_2_6:
        value = m_COMBINATIONSDOCACHI2[8];
        break;
      case RelatedInfoNamed::DOCA_COMB_3_4:
        value = m_COMBINATIONSDOCACHI2[9];
        break;
      case RelatedInfoNamed::DOCA_COMB_3_5:
        value = m_COMBINATIONSDOCACHI2[10];
        break;
      case RelatedInfoNamed::DOCA_COMB_3_6:
        value = m_COMBINATIONSDOCACHI2[11];
        break;
      case RelatedInfoNamed::DOCA_COMB_4_5:
        value = m_COMBINATIONSDOCACHI2[12];
        break;
      case RelatedInfoNamed::DOCA_COMB_4_6:
        value = m_COMBINATIONSDOCACHI2[13];
        break;
      case RelatedInfoNamed::DOCA_COMB_5_6:
        value = m_COMBINATIONSDOCACHI2[14];
        break;

      case RelatedInfoNamed::VDCHI2_OWNPV_COMB_1_2:
        value = m_COMBINATIONSBVVDCHI2[0];
        break;
      case RelatedInfoNamed::VDCHI2_OWNPV_COMB_1_3:
        value = m_COMBINATIONSBVVDCHI2[1];
        break;
      case RelatedInfoNamed::VDCHI2_OWNPV_COMB_1_4:
        value = m_COMBINATIONSBVVDCHI2[2];
        break;
      case RelatedInfoNamed::VDCHI2_OWNPV_COMB_1_5:
        value = m_COMBINATIONSBVVDCHI2[3];
        break;
      case RelatedInfoNamed::VDCHI2_OWNPV_COMB_1_6:
        value = m_COMBINATIONSBVVDCHI2[4];
        break;
      case RelatedInfoNamed::VDCHI2_OWNPV_COMB_2_3:
        value = m_COMBINATIONSBVVDCHI2[5];
        break;
      case RelatedInfoNamed::VDCHI2_OWNPV_COMB_2_4:
        value = m_COMBINATIONSBVVDCHI2[6];
        break;
      case RelatedInfoNamed::VDCHI2_OWNPV_COMB_2_5:
        value = m_COMBINATIONSBVVDCHI2[7];
        break;
      case RelatedInfoNamed::VDCHI2_OWNPV_COMB_2_6:
        value = m_COMBINATIONSBVVDCHI2[8];
        break;
      case RelatedInfoNamed::VDCHI2_OWNPV_COMB_3_4:
        value = m_COMBINATIONSBVVDCHI2[9];
        break;
      case RelatedInfoNamed::VDCHI2_OWNPV_COMB_3_5:
        value = m_COMBINATIONSBVVDCHI2[10];
        break;
      case RelatedInfoNamed::VDCHI2_OWNPV_COMB_3_6:
        value = m_COMBINATIONSBVVDCHI2[11];
        break;
      case RelatedInfoNamed::VDCHI2_OWNPV_COMB_4_5:
        value = m_COMBINATIONSBVVDCHI2[12];
        break;
      case RelatedInfoNamed::VDCHI2_OWNPV_COMB_4_6:
        value = m_COMBINATIONSBVVDCHI2[13];
        break;
      case RelatedInfoNamed::VDCHI2_OWNPV_COMB_5_6:
        value = m_COMBINATIONSBVVDCHI2[14];
        break;

      case RelatedInfoNamed::VDCHI2_MINIPPV_COMB_1_2:
        value = m_COMBINATIONSMINIPVDCHI2[0];
        break;
      case RelatedInfoNamed::VDCHI2_MINIPPV_COMB_1_3:
        value = m_COMBINATIONSMINIPVDCHI2[1];
        break;
      case RelatedInfoNamed::VDCHI2_MINIPPV_COMB_1_4:
        value = m_COMBINATIONSMINIPVDCHI2[2];
        break;
      case RelatedInfoNamed::VDCHI2_MINIPPV_COMB_1_5:
        value = m_COMBINATIONSMINIPVDCHI2[3];
        break;
      case RelatedInfoNamed::VDCHI2_MINIPPV_COMB_1_6:
        value = m_COMBINATIONSMINIPVDCHI2[4];
        break;
      case RelatedInfoNamed::VDCHI2_MINIPPV_COMB_2_3:
        value = m_COMBINATIONSMINIPVDCHI2[5];
        break;
      case RelatedInfoNamed::VDCHI2_MINIPPV_COMB_2_4:
        value = m_COMBINATIONSMINIPVDCHI2[6];
        break;
      case RelatedInfoNamed::VDCHI2_MINIPPV_COMB_2_5:
        value = m_COMBINATIONSMINIPVDCHI2[7];
        break;
      case RelatedInfoNamed::VDCHI2_MINIPPV_COMB_2_6:
        value = m_COMBINATIONSMINIPVDCHI2[8];
        break;
      case RelatedInfoNamed::VDCHI2_MINIPPV_COMB_3_4:
        value = m_COMBINATIONSMINIPVDCHI2[9];
        break;
      case RelatedInfoNamed::VDCHI2_MINIPPV_COMB_3_5:
        value = m_COMBINATIONSMINIPVDCHI2[10];
        break;
      case RelatedInfoNamed::VDCHI2_MINIPPV_COMB_3_6:
        value = m_COMBINATIONSMINIPVDCHI2[11];
        break;
      case RelatedInfoNamed::VDCHI2_MINIPPV_COMB_4_5:
        value = m_COMBINATIONSMINIPVDCHI2[12];
        break;
      case RelatedInfoNamed::VDCHI2_MINIPPV_COMB_4_6:
        value = m_COMBINATIONSMINIPVDCHI2[13];
        break;
      case RelatedInfoNamed::VDCHI2_MINIPPV_COMB_5_6:
        value = m_COMBINATIONSMINIPVDCHI2[14];
        break;

      case RelatedInfoNamed::IPCHI2_OWNPV_COMB_1_2:
        value = m_COMBINATIONSBPVIPCHI2[0];
        break;
      case RelatedInfoNamed::IPCHI2_OWNPV_COMB_1_3:
        value = m_COMBINATIONSBPVIPCHI2[1];
        break;
      case RelatedInfoNamed::IPCHI2_OWNPV_COMB_1_4:
        value = m_COMBINATIONSBPVIPCHI2[2];
        break;
      case RelatedInfoNamed::IPCHI2_OWNPV_COMB_1_5:
        value = m_COMBINATIONSBPVIPCHI2[3];
        break;
      case RelatedInfoNamed::IPCHI2_OWNPV_COMB_1_6:
        value = m_COMBINATIONSBPVIPCHI2[4];
        break;
      case RelatedInfoNamed::IPCHI2_OWNPV_COMB_2_3:
        value = m_COMBINATIONSBPVIPCHI2[5];
        break;
      case RelatedInfoNamed::IPCHI2_OWNPV_COMB_2_4:
        value = m_COMBINATIONSBPVIPCHI2[6];
        break;
      case RelatedInfoNamed::IPCHI2_OWNPV_COMB_2_5:
        value = m_COMBINATIONSBPVIPCHI2[7];
        break;
      case RelatedInfoNamed::IPCHI2_OWNPV_COMB_2_6:
        value = m_COMBINATIONSBPVIPCHI2[8];
        break;
      case RelatedInfoNamed::IPCHI2_OWNPV_COMB_3_4:
        value = m_COMBINATIONSBPVIPCHI2[9];
        break;
      case RelatedInfoNamed::IPCHI2_OWNPV_COMB_3_5:
        value = m_COMBINATIONSBPVIPCHI2[10];
        break;
      case RelatedInfoNamed::IPCHI2_OWNPV_COMB_3_6:
        value = m_COMBINATIONSBPVIPCHI2[11];
        break;
      case RelatedInfoNamed::IPCHI2_OWNPV_COMB_4_5:
        value = m_COMBINATIONSBPVIPCHI2[12];
        break;
      case RelatedInfoNamed::IPCHI2_OWNPV_COMB_4_6:
        value = m_COMBINATIONSBPVIPCHI2[13];
        break;
      case RelatedInfoNamed::IPCHI2_OWNPV_COMB_5_6:
        value = m_COMBINATIONSBPVIPCHI2[14];
        break;

      case RelatedInfoNamed::IPCHI2_MINIPPV_COMB_1_2:
        value = m_COMBINATIONSMINIPCHI2[0];
        break;
      case RelatedInfoNamed::IPCHI2_MINIPPV_COMB_1_3:
        value = m_COMBINATIONSMINIPCHI2[1];
        break;
      case RelatedInfoNamed::IPCHI2_MINIPPV_COMB_1_4:
        value = m_COMBINATIONSMINIPCHI2[2];
        break;
      case RelatedInfoNamed::IPCHI2_MINIPPV_COMB_1_5:
        value = m_COMBINATIONSMINIPCHI2[3];
        break;
      case RelatedInfoNamed::IPCHI2_MINIPPV_COMB_1_6:
        value = m_COMBINATIONSMINIPCHI2[4];
        break;
      case RelatedInfoNamed::IPCHI2_MINIPPV_COMB_2_3:
        value = m_COMBINATIONSMINIPCHI2[5];
        break;
      case RelatedInfoNamed::IPCHI2_MINIPPV_COMB_2_4:
        value = m_COMBINATIONSMINIPCHI2[6];
        break;
      case RelatedInfoNamed::IPCHI2_MINIPPV_COMB_2_5:
        value = m_COMBINATIONSMINIPCHI2[7];
        break;
      case RelatedInfoNamed::IPCHI2_MINIPPV_COMB_2_6:
        value = m_COMBINATIONSMINIPCHI2[8];
        break;
      case RelatedInfoNamed::IPCHI2_MINIPPV_COMB_3_4:
        value = m_COMBINATIONSMINIPCHI2[9];
        break;
      case RelatedInfoNamed::IPCHI2_MINIPPV_COMB_3_5:
        value = m_COMBINATIONSMINIPCHI2[10];
        break;
      case RelatedInfoNamed::IPCHI2_MINIPPV_COMB_3_6:
        value = m_COMBINATIONSMINIPCHI2[11];
        break;
      case RelatedInfoNamed::IPCHI2_MINIPPV_COMB_4_5:
        value = m_COMBINATIONSMINIPCHI2[12];
        break;
      case RelatedInfoNamed::IPCHI2_MINIPPV_COMB_4_6:
        value = m_COMBINATIONSMINIPCHI2[13];
        break;
      case RelatedInfoNamed::IPCHI2_MINIPPV_COMB_5_6:
        value = m_COMBINATIONSMINIPCHI2[14];
        break;

      case RelatedInfoNamed::NLT_OWNPV_COMB_1_2:
        value = m_COMBINATIONSNLT16_BPV[0];
        break;
      case RelatedInfoNamed::NLT_OWNPV_COMB_1_3:
        value = m_COMBINATIONSNLT16_BPV[1];
        break;
      case RelatedInfoNamed::NLT_OWNPV_COMB_1_4:
        value = m_COMBINATIONSNLT16_BPV[2];
        break;
      case RelatedInfoNamed::NLT_OWNPV_COMB_1_5:
        value = m_COMBINATIONSNLT16_BPV[3];
        break;
      case RelatedInfoNamed::NLT_OWNPV_COMB_1_6:
        value = m_COMBINATIONSNLT16_BPV[4];
        break;
      case RelatedInfoNamed::NLT_OWNPV_COMB_2_3:
        value = m_COMBINATIONSNLT16_BPV[5];
        break;
      case RelatedInfoNamed::NLT_OWNPV_COMB_2_4:
        value = m_COMBINATIONSNLT16_BPV[6];
        break;
      case RelatedInfoNamed::NLT_OWNPV_COMB_2_5:
        value = m_COMBINATIONSNLT16_BPV[7];
        break;
      case RelatedInfoNamed::NLT_OWNPV_COMB_2_6:
        value = m_COMBINATIONSNLT16_BPV[8];
        break;
      case RelatedInfoNamed::NLT_OWNPV_COMB_3_4:
        value = m_COMBINATIONSNLT16_BPV[9];
        break;
      case RelatedInfoNamed::NLT_OWNPV_COMB_3_5:
        value = m_COMBINATIONSNLT16_BPV[10];
        break;
      case RelatedInfoNamed::NLT_OWNPV_COMB_3_6:
        value = m_COMBINATIONSNLT16_BPV[11];
        break;
      case RelatedInfoNamed::NLT_OWNPV_COMB_4_5:
        value = m_COMBINATIONSNLT16_BPV[12];
        break;
      case RelatedInfoNamed::NLT_OWNPV_COMB_4_6:
        value = m_COMBINATIONSNLT16_BPV[13];
        break;
      case RelatedInfoNamed::NLT_OWNPV_COMB_5_6:
        value = m_COMBINATIONSNLT16_BPV[14];
        break;

      case RelatedInfoNamed::NLT_MINIPPV_COMB_1_2:
        value = m_COMBINATIONSNLT16_MINIPPV[0];
        break;
      case RelatedInfoNamed::NLT_MINIPPV_COMB_1_3:
        value = m_COMBINATIONSNLT16_MINIPPV[1];
        break;
      case RelatedInfoNamed::NLT_MINIPPV_COMB_1_4:
        value = m_COMBINATIONSNLT16_MINIPPV[2];
        break;
      case RelatedInfoNamed::NLT_MINIPPV_COMB_1_5:
        value = m_COMBINATIONSNLT16_MINIPPV[3];
        break;
      case RelatedInfoNamed::NLT_MINIPPV_COMB_1_6:
        value = m_COMBINATIONSNLT16_MINIPPV[4];
        break;
      case RelatedInfoNamed::NLT_MINIPPV_COMB_2_3:
        value = m_COMBINATIONSNLT16_MINIPPV[5];
        break;
      case RelatedInfoNamed::NLT_MINIPPV_COMB_2_4:
        value = m_COMBINATIONSNLT16_MINIPPV[6];
        break;
      case RelatedInfoNamed::NLT_MINIPPV_COMB_2_5:
        value = m_COMBINATIONSNLT16_MINIPPV[7];
        break;
      case RelatedInfoNamed::NLT_MINIPPV_COMB_2_6:
        value = m_COMBINATIONSNLT16_MINIPPV[8];
        break;
      case RelatedInfoNamed::NLT_MINIPPV_COMB_3_4:
        value = m_COMBINATIONSNLT16_MINIPPV[9];
        break;
      case RelatedInfoNamed::NLT_MINIPPV_COMB_3_5:
        value = m_COMBINATIONSNLT16_MINIPPV[10];
        break;
      case RelatedInfoNamed::NLT_MINIPPV_COMB_3_6:
        value = m_COMBINATIONSNLT16_MINIPPV[11];
        break;
      case RelatedInfoNamed::NLT_MINIPPV_COMB_4_5:
        value = m_COMBINATIONSNLT16_MINIPPV[12];
        break;
      case RelatedInfoNamed::NLT_MINIPPV_COMB_4_6:
        value = m_COMBINATIONSNLT16_MINIPPV[13];
        break;
      case RelatedInfoNamed::NLT_MINIPPV_COMB_5_6:
        value = m_COMBINATIONSNLT16_MINIPPV[14];
        break;

      case RelatedInfoNamed::PT_COMB_1_2:
        value = m_COMBINATIONS_PT[0];
        break;
      case RelatedInfoNamed::PT_COMB_1_3:
        value = m_COMBINATIONS_PT[1];
        break;
      case RelatedInfoNamed::PT_COMB_1_4:
        value = m_COMBINATIONS_PT[2];
        break;
      case RelatedInfoNamed::PT_COMB_1_5:
        value = m_COMBINATIONS_PT[3];
        break;
      case RelatedInfoNamed::PT_COMB_1_6:
        value = m_COMBINATIONS_PT[4];
        break;
      case RelatedInfoNamed::PT_COMB_2_3:
        value = m_COMBINATIONS_PT[5];
        break;
      case RelatedInfoNamed::PT_COMB_2_4:
        value = m_COMBINATIONS_PT[6];
        break;
      case RelatedInfoNamed::PT_COMB_2_5:
        value = m_COMBINATIONS_PT[7];
        break;
      case RelatedInfoNamed::PT_COMB_2_6:
        value = m_COMBINATIONS_PT[8];
        break;
      case RelatedInfoNamed::PT_COMB_3_4:
        value = m_COMBINATIONS_PT[9];
        break;
      case RelatedInfoNamed::PT_COMB_3_5:
        value = m_COMBINATIONS_PT[10];
        break;
      case RelatedInfoNamed::PT_COMB_3_6:
        value = m_COMBINATIONS_PT[11];
        break;
      case RelatedInfoNamed::PT_COMB_4_5:
        value = m_COMBINATIONS_PT[12];
        break;
      case RelatedInfoNamed::PT_COMB_4_6:
        value = m_COMBINATIONS_PT[13];
        break;
      case RelatedInfoNamed::PT_COMB_5_6:
        value = m_COMBINATIONS_PT[14];
        break;

      case RelatedInfoNamed::P_COMB_1_2:
        value = m_COMBINATIONS_P[0];
        break;
      case RelatedInfoNamed::P_COMB_1_3:
        value = m_COMBINATIONS_P[1];
        break;
      case RelatedInfoNamed::P_COMB_1_4:
        value = m_COMBINATIONS_P[2];
        break;
      case RelatedInfoNamed::P_COMB_1_5:
        value = m_COMBINATIONS_P[3];
        break;
      case RelatedInfoNamed::P_COMB_1_6:
        value = m_COMBINATIONS_P[4];
        break;
      case RelatedInfoNamed::P_COMB_2_3:
        value = m_COMBINATIONS_P[5];
        break;
      case RelatedInfoNamed::P_COMB_2_4:
        value = m_COMBINATIONS_P[6];
        break;
      case RelatedInfoNamed::P_COMB_2_5:
        value = m_COMBINATIONS_P[7];
        break;
      case RelatedInfoNamed::P_COMB_2_6:
        value = m_COMBINATIONS_P[8];
        break;
      case RelatedInfoNamed::P_COMB_3_4:
        value = m_COMBINATIONS_P[9];
        break;
      case RelatedInfoNamed::P_COMB_3_5:
        value = m_COMBINATIONS_P[10];
        break;
      case RelatedInfoNamed::P_COMB_3_6:
        value = m_COMBINATIONS_P[11];
        break;
      case RelatedInfoNamed::P_COMB_4_5:
        value = m_COMBINATIONS_P[12];
        break;
      case RelatedInfoNamed::P_COMB_4_6:
        value = m_COMBINATIONS_P[13];
        break;
      case RelatedInfoNamed::P_COMB_5_6:
        value = m_COMBINATIONS_P[14];
        break;

      default:
        value = 0.;
        break;
    }
    debug() << "  Inserting key = " << value << " into map" << endmsg;
    m_map.insert( std::make_pair( key, value ) );
  }

  freeVectors();

  // We're done!
  return StatusCode::SUCCESS;
}

LHCb::RelatedInfoMap *RelInfoHLT1Emulation::getInfo( void ) { return &m_map; }
