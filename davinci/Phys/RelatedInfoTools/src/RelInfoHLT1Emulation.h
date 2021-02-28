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
#ifndef HLT1EMULATION_H
#define HLT1EMULATION_H 1

// Include files
#include "CaloUtils/CaloParticle.h"
#include "GaudiAlg/GaudiTool.h"
#include "Kernel/IRelatedInfoTool.h"

#include <Kernel/GetIDVAlgorithm.h>
#include "Kernel/IDVAlgorithm.h"
#include "Kernel/IParticleCombiner.h"
#include "Kernel/IVertexFit.h"

// ROOT
#include "TLorentzVector.h"
#include "TVector3.h"

struct IDVAlgorithm;
struct IDistanceCalculator;
struct IParticleCombiner;
struct IVertexFit;

/** @class RelInfoHLT1Emulation RelInfoEmulation.h
 *
 * \brief Calculate informations from two-particle combinations of candidate's
 * daughters. Describe better here what the tool does
 *
 *
 * Variables:
 * - NDAUGHTERS             : Number of stable daughters found in the decay.
 * maximum saved is MAXNUMBER
 * - NCOMBINATIONS          : Number of combinations. Maximum is
 * [MAXNUMBER*(MAXNUMBER-1)]/2
 * - PT_DAU_i               : PT of the i^th stable daughter
 * - P_DAU_i                : P of the i^th stable daughter
 * - IPCHI2_OWNPV_DAU_i     : IPChi2 of the i^th daughter wrt its best PV
 * - IPCHI2_MINIPPV_DAU_i   : IPCHi2 of the i^th daughter wrt the vertex wrt
 * which it has minimum IP
 * - TRACK_GHOSTPROB_DAU_i  : Ghost Probability of the i^th daughter
 * - TRACK_CHI2_DAU_i       : Track Chi2 of the i^th daughter
 * - TRACK_NDOF_DAU_i       : Track NDoF of the i^th daughter
 * - VERTEX_CHI2_COMB_i_j   : Vertex Chi2 of the i-j combination
 * - VERTEX_NDOF_COMB_i_j   : Vertex NDoF of the i-j combination
 * - ETA_COMB_i_j           : Pseudorapidity of the i-j combination
 * - MCORR_OWNPV_COMB_i_j   : Corrected Mass of the i-j combination using the
 * OWNPV
 * - MCORR_MINIPPV_COMB_i_j : Corrected Mass of the i-j combination using the
 * MINIPPV
 * - SUMPT_COMB_i_j         : Sum of the daughters' PT of the i-j combination
 * - DIRA_OWNPV_COMB_i_j    : DIRA of the i-j combination using the OWNPV
 * - DIRA_MINIPPV_COMB_i_j  : DIRA of the i-j combination using the MINIPPV
 * - DOCA_COMB_i_j          : DOCAChi2  of the daughters of the i-j combination
 * - VDCHI2_OWNPV_COMB_i_j  : VDCHI2 of the i-j combination wrt the OWNPV
 * - VDCHI2_MINIPPV_COMB_i_j: VDCHI2 of the i-j combination wrt the MINIPPV
 * - IPCHI2_ONWPV_COMB_i_j  : IPCHI2 of the i-j combination wrt the OWNPV
 * - IPCHI2_MINIPPV_COMB_i_j: IPCHI2 of the i-j combination wrt the MINIPPV
 * - NLT_OWNPV_COMB_i_j     : Nlt variable used in the HLT1, wrt the OWNPV
 * - NLT_MINIPPV_COMB_i_j   : Nlt variable used in the HLT1, wrt the MINIPPV
 * Options:
 * - Variables : List of variables to be stored (All if empty)
 *
 *  @author Simone Meloni, Julian Garcìa Pardinas (simone.meloni@cern.ch)
 *  @date   1/12/2019
 *
 */

class RelInfoHLT1Emulation : public GaudiTool,
                             virtual public IRelatedInfoTool {
 public:
  /// Standard constructor
  RelInfoHLT1Emulation( const std::string &type, const std::string &name,
                        const IInterface *parent );

  StatusCode initialize() override;

  virtual ~RelInfoHLT1Emulation();  ///< Destructor

 public:
  StatusCode calculateRelatedInfo( const LHCb::Particle *,
                                   const LHCb::Particle * ) override;

  LHCb::RelatedInfoMap *getInfo( void ) override;

 private:
  // Functions you are using to get all the informations
  void getAllDaughters( const LHCb::Particle *,
                        std::vector<const LHCb::Particle *> &particlesVector );
  void getAllCombinations( std::vector<const LHCb::Particle *> &daughters,
                           std::vector<const LHCb::Particle *> &mothers,
                           std::vector<const LHCb::Vertex *> &  vertices,
                           bool changePIDtoPions );

  void getBestPVs( const std::vector<const LHCb::Particle *> &particles,
                   std::vector<const LHCb::VertexBase *> &    vertexVector );

  void getAllPVs( std::vector<const LHCb::VertexBase *> &PVs );

  void getMinIPPVs( const std::vector<const LHCb::Particle *> &  particles,
                    const std::vector<const LHCb::VertexBase *> &PVs,
                    std::vector<const LHCb::VertexBase *> &vertexVector );

  void setNumber( const std::vector<const LHCb::Particle *> &particles,
                  int &                                      nParticles );

  void setTransverseMomenta(
      const std::vector<const LHCb::Particle *> &particlesVector,
      std::vector<float> &                       valuesVector );

  void setMomenta( const std::vector<const LHCb::Particle *> &particlesVector,
                   std::vector<float> &                       valuesVector );

  void setIPChi2( const std::vector<const LHCb::Particle *> &  particlesVector,
                  const std::vector<const LHCb::VertexBase *> &vertexVector,
                  std::vector<float> &                         valuesVector );
  void setTrackGhostProb(
      const std::vector<const LHCb::Particle *> &particlesVector,
      std::vector<float> &                       valuesVector );
  void setTrackChi2(
      const std::vector<const LHCb::Particle *> &particlesVector,
      std::vector<float> &                       valuesVector );
  void setTrackNDOF(
      const std::vector<const LHCb::Particle *> &particlesVector,
      std::vector<float> &                       valuesVector );

  void setVertexChi2( const std::vector<const LHCb::Vertex *> &verticesVector,
                      std::vector<float> &                     valuesVector );

  void setVertexDof( const std::vector<const LHCb::Vertex *> &verticesVector,
                     std::vector<float> &                     valuesVector );

  void setEta( const std::vector<const LHCb::Particle *> &particlesVector,
               std::vector<float> &                       valuesVector );

  double MCorr( TLorentzVector &Y, TVector3 &M_dirn ) const;

  void setMCorr( const std::vector<const LHCb::Particle *> &  particlesVector,
                 const std::vector<const LHCb::VertexBase *> &verticesVector,
                 std::vector<float> &                         valuesVector );

  void setDaughtersSumPT(
      const std::vector<const LHCb::Particle *> &particlesVector,
      std::vector<float> &                       valuesVector );

  double dira( const LHCb::Particle *P, const LHCb::VertexBase *V ) const;

  void setDira( const std::vector<const LHCb::Particle *> &  particlesVector,
                const std::vector<const LHCb::VertexBase *> &verticesVector,
                std::vector<float> &                         valuesVector );

  void setDOCACHI2( const std::vector<const LHCb::Particle *> &particlesVector,
                    std::vector<float> &                       valuesVector );

  void setVDCHI2( const std::vector<const LHCb::Particle *> &  particlesVector,
                  const std::vector<const LHCb::VertexBase *> &verticesVector,
                  std::vector<float> &                         valuesVector );

  void setnlt( const std::vector<float> &ipchi2Vector,
               std::vector<float> &      valuesVector );

  void saveRelatedInfo( const LHCb::Particle * );

  LHCb::Particle *changePIDHypothesis( const LHCb::Particle *part );

 private:
  // Check if it is a pure CALO Particle
  inline bool isPureNeutralCalo( const LHCb::Particle *P ) const {
    LHCb::CaloParticle caloP( (LHCb::Particle *)P );
    return caloP.isPureNeutralCalo();
  }

  // Check if it has a track
  inline bool isBasicParticle( const LHCb::Particle *P ) const {
    auto PID = P->particleID().pid();

    bool isbasic = P->isBasicParticle();
    bool hasproto =
        fabs( PID ) != 22 && fabs( PID ) != 111 && fabs( PID ) != 221;

    return isbasic && hasproto;
  }

  // Clear all vectors
  inline void clearVectors() {
    m_primaryVertices.clear();
    m_decayParticles.clear();
    m_decayParticles_bestPVs.clear();
    m_decayParticles_MinIPPVs.clear();
    m_combinationsParticles_bestPVs.clear();
    m_combinationsParticles_MinIPPvs.clear();
    m_DAUGHTERSMOMENTA.clear();
    m_DAUGHTERSMOMENTA_P.clear();
    m_DAUGHTERSIPCHI2BEST.clear();
    m_DAUGHTERSIPCHI2SMALLEST.clear();
    m_DAUGHTERSTRACKGHOSTPROB.clear();
    m_DAUGHTERSTRACKCHI2.clear();
    m_DAUGHTERSTRACKNDOF.clear();
    m_COMBINATIONSETA.clear();
    m_COMBINATIONSVCHI2.clear();
    m_COMBINATIONSVNDOF.clear();
    m_COMBINATIONSBVVDCHI2.clear();
    m_COMBINATIONSMINIPVDCHI2.clear();
    m_COMBINATIONSSUMPT.clear();
    m_COMBINATIONSMCORR_BPV.clear();
    m_COMBINATIONSMCORR_MINIPPV.clear();
    m_COMBINATIONSSUMPT.clear();
    m_COMBINATIONSDIRA_BPV.clear();
    m_COMBINATIONSDIRA_MINIPPV.clear();
    m_COMBINATIONSBPVIPCHI2.clear();
    m_COMBINATIONSMINIPCHI2.clear();
    m_motherParticles.clear();
    m_motherVertices.clear();
    m_COMBINATIONSDOCACHI2.clear();
    m_COMBINATIONSNLT16_BPV.clear();
    m_COMBINATIONSNLT16_MINIPPV.clear();
    m_COMBINATIONS_P.clear();
    m_COMBINATIONS_PT.clear();

    return;
  }

  // Function used to perform the loop over the vector and free its memory
  void freeVector( std::vector<const LHCb::Particle *> &vector ) {
    for ( auto it = vector.begin(); it != vector.end(); ++it ) {
      if ( ( *it ) != NULL ) delete *it;
    }
    return;
  }

  void freeVector( std::vector<const LHCb::VertexBase *> &vector ) {
    for ( auto it = vector.begin(); it != vector.end(); ++it ) {
      if ( ( *it ) != NULL ) delete *it;
    }
    return;
  }

  void freeVector( std::vector<const LHCb::Vertex *> &vector ) {
    for ( auto it = vector.begin(); it != vector.end(); ++it ) {
      if ( ( *it ) != NULL ) delete *it;
    }
    return;
  }

  // Free the memory from the objects contained by a vector
  inline void freeVectors() {
    freeVector( m_motherParticles );
    freeVector( m_motherVertices );

    return;
  }

 private:
  // Tools used to make evaluations
  IDVAlgorithm *             m_dva;
  const IDistanceCalculator *m_dist;
  const IParticleCombiner *  m_pCombiner;

  // Variables to store the Tool Properties
  std::vector<std::string> m_variables;
  float                    m_nltValue;

  // Private variables you want to use during the evaluations
  std::vector<const LHCb::VertexBase *>
      m_primaryVertices;  // All the primary vertices
  std::vector<const LHCb::Particle *>
      m_decayParticles;  // All the stable daughters
  std::vector<const LHCb::Particle *>
      m_motherParticles;  // All the two-particle combinations
  std::vector<const LHCb::Vertex *>
      m_motherVertices;  // All the vertices of the two-particle combinations
  std::vector<const LHCb::VertexBase *>
      m_decayParticles_bestPVs;  // best PV associated to each stable daughter
  std::vector<const LHCb::VertexBase *>
      m_decayParticles_MinIPPVs;  // MinIP PV associated to each stable
                                  // daughter
  std::vector<const LHCb::VertexBase *>
      m_combinationsParticles_bestPVs;  // best PV associated to each
                                        // combination
  std::vector<const LHCb::VertexBase *>
      m_combinationsParticles_MinIPPvs;  // MinIP PV associated to each
                                         // combination

 private:
  // Variables you want to save in the RelatedInfoMap
  int m_NDAUGHTERS;     // Number of stable daughters
  int m_NCOMBINATIONS;  // Number of two-particle combinations of stable
                        // daughters
  std::vector<float> m_DAUGHTERSMOMENTA;    // PT of each stable daughter
  std::vector<float> m_DAUGHTERSMOMENTA_P;  // P of each stable daughter
  std::vector<float>
                     m_DAUGHTERSIPCHI2BEST;  // IPCHI2 wrt OWNPV of each stable daughter
  std::vector<float> m_DAUGHTERSIPCHI2SMALLEST;  // IPCHI2 wrt MiNIP PV of each
                                                 // stable daughter
  std::vector<float>
      m_DAUGHTERSTRACKGHOSTPROB;  // Ghost Prob of each stable daughter
  std::vector<float>
      m_DAUGHTERSTRACKCHI2;  // Track Chi2 of each stable daughter
  std::vector<float>
                     m_DAUGHTERSTRACKNDOF;  // Track NDOF of each stable daughter
  std::vector<float> m_COMBINATIONSETA;  // Pseudorapidity of the combinations
  std::vector<float> m_COMBINATIONSVCHI2;  // Vertex Chi2 of the combinations
  std::vector<float> m_COMBINATIONSVNDOF;  // Vertex NDof of the combinations
  std::vector<float> m_COMBINATIONSMCORR_BPV;  // Corrected mass of the
                                               // combinations using the OWNPV
  std::vector<float>
      m_COMBINATIONSMCORR_MINIPPV;  // Corrected mass of the combinations using
                                    // the MinIP PV
  std::vector<float>
      m_COMBINATIONSBVVDCHI2;  // VDCHI2 of each combination wrt OWNPV
  std::vector<float>
      m_COMBINATIONSMINIPVDCHI2;  // VDCHI2 of each combination wrt MinIPPV
  std::vector<float>
      m_COMBINATIONSSUMPT;  // Sum of the PT of the combinations daughters
  std::vector<float>
                     m_COMBINATIONSDIRA_BPV;  // Dira wrt the flight direction from OWNPV
  std::vector<float> m_COMBINATIONSDIRA_MINIPPV;  // Dira wrt the flight
                                                  // direction from  MinIPPV
  std::vector<float>
      m_COMBINATIONSDOCACHI2;  // DOCA of the daughters for each combination
  std::vector<float>
      m_COMBINATIONSBPVIPCHI2;  // IPCHi2 of the combinations wrt OWNPV
  std::vector<float>
      m_COMBINATIONSMINIPCHI2;  // IPCHi2 of the combinations wrt MINIPPV
  std::vector<float>
      m_COMBINATIONSNLT16_BPV;  // NLT16 variable, i.e. number of stable
                                // daughters with IPCHI2 < 16
  std::vector<float>
      m_COMBINATIONSNLT16_MINIPPV;  // NLT16 variable, i.e. number of stable
                                    // daughters with IPCHI2 < 16
  std::vector<float> m_COMBINATIONS_PT;  // PT of the combinations
  std::vector<float> m_COMBINATIONS_P;   // P of the combinations

 private:
  std::vector<short int> m_keys;
  LHCb::RelatedInfoMap   m_map;
};

#endif  // HLT1EMULATION_H
