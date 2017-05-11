/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2016, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     TAppEncTop.cpp
    \brief    Encoder application class
*/

#include <list>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#if !SVC_EXTENSION
#include <iomanip>
#endif

#include "TAppEncTop.h"
#include "TLibEncoder/AnnexBwrite.h"

using namespace std;

//! \ingroup TAppEncoder
//! \{

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================

TAppEncTop::TAppEncTop()
{
  m_iFrameRcvd = 0;
  m_totalBytes = 0;
  m_essentialBytes = 0;
#if SVC_EXTENSION
  memset( m_apcTVideoIOYuvInputFile, 0, sizeof(m_apcTVideoIOYuvInputFile) );
  memset( m_apcTVideoIOYuvReconFile, 0, sizeof(m_apcTVideoIOYuvReconFile) );
  memset( m_apcTEncTop, 0, sizeof(m_apcTEncTop) );
#endif
}

TAppEncTop::~TAppEncTop()
{
}

Void TAppEncTop::xInitLibCfg()
{
#if SVC_EXTENSION
  TComVPS& vps = *m_apcTEncTop[0]->getVPS();

  vps.setMaxTLayers                                               ( m_apcLayerCfg[0]->m_maxTempLayer );
  if (m_apcLayerCfg[0]->m_maxTempLayer == 1)
#else
  TComVPS vps;

  vps.setMaxTLayers                                               ( m_maxTempLayer );
  if (m_maxTempLayer == 1)
#endif
  {
    vps.setTemporalNestingFlag(true);
  }
  vps.setMaxLayers                                                ( 1 );
  for(Int i = 0; i < MAX_TLAYER; i++)
  {
    vps.setNumReorderPics                                         ( m_numReorderPics[i], i );
    vps.setMaxDecPicBuffering                                     ( m_maxDecPicBuffering[i], i );
  }

#if !SVC_EXTENSION
  m_cTEncTop.setVPS(&vps);
#endif
  
#if SVC_EXTENSION
  vps.setVpsPocLsbAlignedFlag(false);  

  Int maxRepFormatIdx = -1;
  Int formatIdx = -1;
  for(UInt layer=0; layer < m_numLayers; layer++)
  {
    // Auto generation of the format index
    if( m_apcLayerCfg[layer]->getRepFormatIdx() == -1 )
    {      
      Bool found = false;
      for( UInt idx = 0; idx < layer; idx++ )
      {
        if( m_apcLayerCfg[layer]->m_iSourceWidth == m_apcLayerCfg[idx]->m_iSourceWidth && m_apcLayerCfg[layer]->m_iSourceHeight == m_apcLayerCfg[idx]->m_iSourceHeight
#if AUXILIARY_PICTURES
          && m_apcLayerCfg[layer]->m_chromaFormatIDC == m_apcLayerCfg[idx]->m_chromaFormatIDC
#endif
          && m_apcLayerCfg[layer]->m_internalBitDepth[CHANNEL_TYPE_LUMA] == m_apcLayerCfg[idx]->m_internalBitDepth[CHANNEL_TYPE_LUMA] && m_apcLayerCfg[layer]->m_internalBitDepth[CHANNEL_TYPE_CHROMA] == m_apcLayerCfg[idx]->m_internalBitDepth[CHANNEL_TYPE_CHROMA]
          )
        {
          found = true;
          break;
        }
      }
      if( !found )
      {
        formatIdx++;
      }

      m_apcLayerCfg[layer]->setRepFormatIdx( formatIdx );
    }

    assert( m_apcLayerCfg[layer]->getRepFormatIdx() != -1 && "RepFormatIdx not assigned for a layer" );

    vps.setVpsRepFormatIdx( layer, m_apcLayerCfg[layer]->getRepFormatIdx() );

    maxRepFormatIdx = std::max( m_apcLayerCfg[layer]->getRepFormatIdx(), maxRepFormatIdx );

    for(Int compareLayer = 0; compareLayer < layer; compareLayer++ )
    {
      if(m_apcLayerCfg[layer]->m_repFormatIdx == m_apcLayerCfg[compareLayer]->m_repFormatIdx && (
           m_apcLayerCfg[layer]->m_chromaFormatIDC != m_apcLayerCfg[compareLayer]->m_chromaFormatIDC
           // separate_colour_plane_flag not supported yet but if supported insert check here
           || m_apcLayerCfg[layer]->m_iSourceWidth != m_apcLayerCfg[compareLayer]->m_iSourceWidth
           || m_apcLayerCfg[layer]->m_iSourceHeight != m_apcLayerCfg[compareLayer]->m_iSourceHeight
           || m_apcLayerCfg[layer]->m_internalBitDepth[0] != m_apcLayerCfg[compareLayer]->m_internalBitDepth[0]
           || m_apcLayerCfg[layer]->m_internalBitDepth[1] != m_apcLayerCfg[compareLayer]->m_internalBitDepth[1]
           || m_apcLayerCfg[layer]->m_confWinLeft != m_apcLayerCfg[compareLayer]->m_confWinLeft
           || m_apcLayerCfg[layer]->m_confWinRight != m_apcLayerCfg[compareLayer]->m_confWinRight
           || m_apcLayerCfg[layer]->m_confWinTop != m_apcLayerCfg[compareLayer]->m_confWinTop
           || m_apcLayerCfg[layer]->m_confWinBottom != m_apcLayerCfg[compareLayer]->m_confWinBottom
        ))
      {
        fprintf(stderr, "Error: Two layers using the same FormatIdx value must share the same values of the related parameters\n");
        exit(EXIT_FAILURE);
      }
    }
  }

  assert( vps.getVpsRepFormatIdx( 0 ) == 0 );  // Base layer should point to the first one.

  Int* mapIdxToLayer = new Int[maxRepFormatIdx + 1];

  // Check that all the indices from 0 to maxRepFormatIdx are used in the VPS
  for(Int i = 0; i <= maxRepFormatIdx; i++)
  {
    mapIdxToLayer[i] = -1;
    UInt layer;
    for(layer=0; layer < m_numLayers; layer++)
    {
      if( vps.getVpsRepFormatIdx(layer) == i )
      {
        mapIdxToLayer[i] = layer;
        break;
      }
    }
    assert( layer != m_numLayers );   // One of the VPS Rep format indices not set
  }

  vps.setVpsNumRepFormats                                                ( maxRepFormatIdx + 1 );

  // When not present, the value of rep_format_idx_present_flag is inferred to be equal to 0
  vps.setRepFormatIdxPresentFlag                                         ( vps.getVpsNumRepFormats() > 1 ? true : false );

  for(UInt idx=0; idx < vps.getVpsNumRepFormats(); idx++)
  {
    RepFormat *repFormat = vps.getVpsRepFormat( idx );
    repFormat->setChromaAndBitDepthVpsPresentFlag                         ( true ); 
    if (idx==0)
    {
      assert(repFormat->getChromaAndBitDepthVpsPresentFlag() == true); 
    }

    repFormat->setPicWidthVpsInLumaSamples                                ( m_apcLayerCfg[mapIdxToLayer[idx]]->m_iSourceWidth   );
    repFormat->setPicHeightVpsInLumaSamples                               ( m_apcLayerCfg[mapIdxToLayer[idx]]->m_iSourceHeight  );
#if AUXILIARY_PICTURES
    repFormat->setChromaFormatVpsIdc                                      ( m_apcLayerCfg[mapIdxToLayer[idx]]->m_chromaFormatIDC );
#else
    repFormat->setChromaFormatVpsIdc                                      ( 1 );  // Need modification to change for each layer - corresponds to 420
#endif
    repFormat->setSeparateColourPlaneVpsFlag                              ( 0 );  // Need modification to change for each layer

    repFormat->setBitDepthVpsLuma                                         ( getInternalBitDepth(mapIdxToLayer[idx], CHANNEL_TYPE_LUMA)      );  // Need modification to change for each layer
    repFormat->setBitDepthVpsChroma                                       ( getInternalBitDepth(mapIdxToLayer[idx], CHANNEL_TYPE_CHROMA)    );  // Need modification to change for each layer

    repFormat->getConformanceWindowVps().setWindow                        ( m_apcLayerCfg[mapIdxToLayer[idx]]->m_confWinLeft,                                                                             
                                                                            m_apcLayerCfg[mapIdxToLayer[idx]]->m_confWinRight, 
                                                                            m_apcLayerCfg[mapIdxToLayer[idx]]->m_confWinTop,
                                                                            m_apcLayerCfg[mapIdxToLayer[idx]]->m_confWinBottom );

    m_apcTEncTop[mapIdxToLayer[idx]]->setSkipPictureAtArcSwitch           ( m_skipPictureAtArcSwitch );
  }

  delete [] mapIdxToLayer;

  //Populate PTL in VPS
  for( Int ii = 0; ii < m_numPTLInfo; ii++ )
  {
    vps.getPTL(ii)->getGeneralPTL()->setLevelIdc(m_levelList[ii]);
    vps.getPTL(ii)->getGeneralPTL()->setTierFlag(m_levelTierList[ii]);
    vps.getPTL(ii)->getGeneralPTL()->setProfileIdc(m_profileList[ii]);
    vps.getPTL(ii)->getGeneralPTL()->setProfileCompatibilityFlag(m_profileCompatibility[ii], 1);
    vps.getPTL(ii)->getGeneralPTL()->setProgressiveSourceFlag(m_progressiveSourceFlagList[ii]);
    vps.getPTL(ii)->getGeneralPTL()->setInterlacedSourceFlag(m_interlacedSourceFlagList[ii]);
    vps.getPTL(ii)->getGeneralPTL()->setNonPackedConstraintFlag(m_nonPackedConstraintFlagList[ii]);
    vps.getPTL(ii)->getGeneralPTL()->setFrameOnlyConstraintFlag(m_frameOnlyConstraintFlagList[ii]);
  }
  vps.setNumProfileTierLevel(m_numPTLInfo);

  std::vector<Int> myvector;
  vps.getProfileLevelTierIdx()->resize(m_numOutputLayerSets);
  for( Int ii = 0; ii < m_numOutputLayerSets; ii++ )
  {
    myvector =  m_listOfLayerPTLofOlss[ii];

    for( std::vector<Int>::iterator it = myvector.begin() ; it != myvector.end(); ++it )
    {
      vps.addProfileLevelTierIdx(ii, it[0]);
    }
  }

  assert( m_numLayers <= MAX_LAYERS );

  for( UInt layer=0; layer < m_numLayers; layer++ )
  {
    TEncTop& m_cTEncTop = *m_apcTEncTop[layer];
    //1
    m_cTEncTop.setInterLayerWeightedPredFlag                      ( m_useInterLayerWeightedPred );
#if AVC_BASE
    m_cTEncTop.setMFMEnabledFlag                                  ( layer == 0 ? false : ( m_nonHEVCBaseLayerFlag ? false : true ) && m_apcLayerCfg[layer]->getNumMotionPredRefLayers());
#else
    m_cTEncTop.setMFMEnabledFlag                                  ( layer == 0 ? false : m_apcLayerCfg[layer]->getNumMotionPredRefLayers());
#endif

    // set layer ID
    m_cTEncTop.setLayerId                                         ( m_apcLayerCfg[layer]->m_layerId );
    m_cTEncTop.setNumLayer                                        ( m_numLayers );
    m_cTEncTop.setLayerEnc                                        ( m_apcTEncTop );

    if( layer < m_numLayers - 1 )
    {
       m_cTEncTop.setMaxTidIlRefPicsPlus1                         ( m_apcLayerCfg[layer]->getMaxTidIlRefPicsPlus1());
    }

    if( layer )
    {
      UInt prevLayerIdx = 0;
      UInt prevLayerId  = 0;
      if (m_apcLayerCfg[layer]->getNumActiveRefLayers() > 0)
      {
        prevLayerIdx = m_apcLayerCfg[layer]->getPredLayerIdx(m_apcLayerCfg[layer]->getNumActiveRefLayers() - 1);
        prevLayerId  = m_cTEncTop.getRefLayerId(prevLayerIdx);
      }
      for(Int i = 0; i < MAX_VPS_LAYER_IDX_PLUS1; i++)
      {
        m_cTEncTop.setSamplePredEnabledFlag                       (i, false);
        m_cTEncTop.setMotionPredEnabledFlag                       (i, false);
      }
      if(m_apcLayerCfg[layer]->getNumSamplePredRefLayers() == -1)
      {
        // Not included in the configuration file; assume that each layer depends on previous layer
        m_cTEncTop.setNumSamplePredRefLayers                      (1);      // One sample pred ref. layer
        m_cTEncTop.setSamplePredRefLayerId                        (prevLayerIdx, prevLayerId);   // Previous layer
        m_cTEncTop.setSamplePredEnabledFlag                       (prevLayerIdx, true);
      }
      else
      {
        m_cTEncTop.setNumSamplePredRefLayers                      ( m_apcLayerCfg[layer]->getNumSamplePredRefLayers() );
        for( Int i = 0; i < m_cTEncTop.getNumSamplePredRefLayers(); i++ )
        {
          m_cTEncTop.setSamplePredRefLayerId                      ( i, m_apcLayerCfg[layer]->getSamplePredRefLayerId(i));
        }
      }
      if( m_apcLayerCfg[layer]->getNumMotionPredRefLayers() == -1)
      {
        // Not included in the configuration file; assume that each layer depends on previous layer
        m_cTEncTop.setNumMotionPredRefLayers                      (1);      // One motion pred ref. layer
        m_cTEncTop.setMotionPredRefLayerId                        (prevLayerIdx, prevLayerId);   // Previous layer
        m_cTEncTop.setMotionPredEnabledFlag                       (prevLayerIdx, true);
      }
      else
      {
        m_cTEncTop.setNumMotionPredRefLayers                      ( m_apcLayerCfg[layer]->getNumMotionPredRefLayers() );
        for( Int i = 0; i < m_cTEncTop.getNumMotionPredRefLayers(); i++ )
        {
          m_cTEncTop.setMotionPredRefLayerId                      ( i, m_apcLayerCfg[layer]->getMotionPredRefLayerId(i));
        }
      }
      Int numDirectRefLayers = 0;

      assert( layer < MAX_LAYERS );

      for (Int i = 0; i < m_apcLayerCfg[layer]->m_layerId; i++)
      {
        Int refLayerId = -1;

        for( Int layerIdc = 0; layerIdc < m_cTEncTop.getNumSamplePredRefLayers(); layerIdc++ )
        {
          if( m_apcLayerCfg[layer]->getSamplePredRefLayerId(layerIdc) == i )
          {
            refLayerId = i;
            m_cTEncTop.setSamplePredEnabledFlag( numDirectRefLayers, true );
            break;
          }
        }

        for( Int layerIdc = 0; layerIdc < m_cTEncTop.getNumMotionPredRefLayers(); layerIdc++ )
        {
          if( m_apcLayerCfg[layer]->getMotionPredRefLayerId(layerIdc) == i )
          {
            refLayerId = i;
            m_cTEncTop.setMotionPredEnabledFlag( numDirectRefLayers, true );
            break;
          }
        }

        if( refLayerId >= 0 )
        {
          m_cTEncTop.setRefLayerId                                ( numDirectRefLayers, refLayerId );
          numDirectRefLayers++;
        }
      }

      m_cTEncTop.setNumDirectRefLayers                            ( numDirectRefLayers );

      if(m_apcLayerCfg[layer]->getNumActiveRefLayers() == -1)
      {
        m_cTEncTop.setNumActiveRefLayers                          ( m_cTEncTop.getNumDirectRefLayers() );
        for( Int i = 0; i < m_cTEncTop.getNumActiveRefLayers(); i++ )
        {
          m_cTEncTop.setPredLayerIdx(i, i);
        }
      }
      else
      {
        m_cTEncTop.setNumActiveRefLayers                          ( m_apcLayerCfg[layer]->getNumActiveRefLayers() );
        for(Int i = 0; i < m_cTEncTop.getNumActiveRefLayers(); i++)
        {
          m_cTEncTop.setPredLayerIdx                              ( i, m_apcLayerCfg[layer]->getPredLayerIdx(i));
        }
      }
    }
    else
    {
      assert( layer == 0 );
      m_cTEncTop.setNumDirectRefLayers(0);
    }

    if( layer > 0 )
    {
#if AUXILIARY_PICTURES
      Int subWidthC  = ( m_apcLayerCfg[layer]->m_chromaFormatIDC == CHROMA_420 || m_apcLayerCfg[layer]->m_chromaFormatIDC == CHROMA_422 ) ? 2 : 1;
      Int subHeightC = ( m_apcLayerCfg[layer]->m_chromaFormatIDC == CHROMA_420 ) ? 2 : 1;
#else
      Int subWidthC  = 2;
      Int subHeightC = 2;
#endif
      m_cTEncTop.setNumRefLayerLocationOffsets          ( m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets );
      for(Int i = 0; i < m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets; i++)
      {
        m_cTEncTop.setRefLocationOffsetLayerId          ( i, m_apcLayerCfg[layer]->m_refLocationOffsetLayerId[i]);
        m_cTEncTop.setScaledRefLayerOffsetPresentFlag   ( i, m_apcLayerCfg[layer]->m_scaledRefLayerOffsetPresentFlag[i] );
        m_cTEncTop.getScaledRefLayerWindow(i).setWindow ( subWidthC  * m_apcLayerCfg[layer]->m_scaledRefLayerLeftOffset[i], subWidthC  * m_apcLayerCfg[layer]->m_scaledRefLayerRightOffset[i],
          subHeightC * m_apcLayerCfg[layer]->m_scaledRefLayerTopOffset[i],  subHeightC * m_apcLayerCfg[layer]->m_scaledRefLayerBottomOffset[i]);

        Int rlSubWidthC  = ( m_apcLayerCfg[i]->m_chromaFormatIDC == CHROMA_420 || m_apcLayerCfg[i]->m_chromaFormatIDC == CHROMA_422 ) ? 2 : 1;
        Int rlSubHeightC = ( m_apcLayerCfg[i]->m_chromaFormatIDC == CHROMA_420 ) ? 2 : 1;

        m_cTEncTop.setRefRegionOffsetPresentFlag        ( i, m_apcLayerCfg[layer]->m_refRegionOffsetPresentFlag[i] );
        m_cTEncTop.getRefLayerWindow(i).setWindow       ( rlSubWidthC  * m_apcLayerCfg[layer]->m_refRegionLeftOffset[i], rlSubWidthC  * m_apcLayerCfg[layer]->m_refRegionRightOffset[i],
          rlSubHeightC * m_apcLayerCfg[layer]->m_refRegionTopOffset[i],  rlSubHeightC * m_apcLayerCfg[layer]->m_refRegionBottomOffset[i]);
        m_cTEncTop.setResamplePhaseSetPresentFlag       ( i, m_apcLayerCfg[layer]->m_resamplePhaseSetPresentFlag[i] );
        m_cTEncTop.setPhaseHorLuma                      ( i, m_apcLayerCfg[layer]->m_phaseHorLuma[i] );
        m_cTEncTop.setPhaseVerLuma                      ( i, m_apcLayerCfg[layer]->m_phaseVerLuma[i] );
        m_cTEncTop.setPhaseHorChroma                    ( i, m_apcLayerCfg[layer]->m_phaseHorChroma[i] );
        m_cTEncTop.setPhaseVerChroma                    ( i, m_apcLayerCfg[layer]->m_phaseVerChroma[i] );
      }
    }

    m_cTEncTop.setElRapSliceTypeB                        ( m_elRapSliceBEnabled );

    m_cTEncTop.setAdaptiveResolutionChange               ( m_adaptiveResolutionChange );
    m_cTEncTop.setLayerSwitchOffBegin                    ( m_apcLayerCfg[layer]->m_layerSwitchOffBegin );
    m_cTEncTop.setLayerSwitchOffEnd                      ( m_apcLayerCfg[layer]->m_layerSwitchOffEnd );
    m_cTEncTop.setChromaFormatIDC                        ( m_apcLayerCfg[layer]->m_chromaFormatIDC );
    m_cTEncTop.setAltOuputLayerFlag                      ( m_altOutputLayerFlag );
    m_cTEncTop.setCrossLayerBLAFlag                      ( m_crossLayerBLAFlag );
#if CGS_3D_ASYMLUT
    m_cTEncTop.setCGSFlag                                ( layer == 0 ? 0 : m_nCGSFlag );
    m_cTEncTop.setCGSMaxOctantDepth                      ( m_nCGSMaxOctantDepth );
    m_cTEncTop.setCGSMaxYPartNumLog2                     ( m_nCGSMaxYPartNumLog2 );
    m_cTEncTop.setCGSLUTBit                              ( m_nCGSLUTBit );
    m_cTEncTop.setCGSAdaptChroma                         ( m_nCGSAdaptiveChroma );
#if R0179_ENC_OPT_3DLUT_SIZE
    m_cTEncTop.setCGSLutSizeRDO                          ( m_nCGSLutSizeRDO );
#endif
#endif
    m_cTEncTop.setNumAddLayerSets                        ( m_numAddLayerSets );

#if FAST_INTRA_SHVC
    m_cTEncTop.setUseFastIntraScalable                              ( m_useFastIntraScalable );
#endif

#if P0123_ALPHA_CHANNEL_SEI
    m_cTEncTop.setAlphaSEIEnabled                                   ( m_alphaSEIEnabled );
    m_cTEncTop.setAlphaCancelFlag                                   ( m_alphaCancelFlag );
    m_cTEncTop.setAlphaUseIdc                                       ( m_alphaUseIdc );
    m_cTEncTop.setAlphaBitDepthMinus8                               ( m_alphaBitDepthMinus8 );
    m_cTEncTop.setAlphaTransparentValue                             ( m_alphaTransparentValue );
    m_cTEncTop.setAlphaOpaqueValue                                  ( m_alphaOpaqueValue );
    m_cTEncTop.setAlphaIncrementFlag                                ( m_alphaIncrementFlag );
    m_cTEncTop.setAlphaClipFlag                                     ( m_alphaClipFlag );
    m_cTEncTop.setAlphaClipTypeFlag                                 ( m_alphaClipTypeFlag );
#endif
#if Q0096_OVERLAY_SEI
    m_cTEncTop.setOverlaySEIEnabled                                 ( m_overlaySEIEnabled );
    m_cTEncTop.setOverlaySEICancelFlag                              ( m_overlayInfoCancelFlag );
    m_cTEncTop.setOverlaySEIContentAuxIdMinus128                    ( m_overlayContentAuxIdMinus128 );
    m_cTEncTop.setOverlaySEILabelAuxIdMinus128                      ( m_overlayLabelAuxIdMinus128 );
    m_cTEncTop.setOverlaySEIAlphaAuxIdMinus128                      ( m_overlayAlphaAuxIdMinus128 );
    m_cTEncTop.setOverlaySEIElementLabelValueLengthMinus8           ( m_overlayElementLabelValueLengthMinus8 );
    m_cTEncTop.setOverlaySEINumOverlaysMinus1                       ( m_numOverlaysMinus1 );
    m_cTEncTop.setOverlaySEIIdx                                     ( m_overlayIdx );
    m_cTEncTop.setOverlaySEILanguagePresentFlag                     ( m_overlayLanguagePresentFlag );
    m_cTEncTop.setOverlaySEIContentLayerId                          ( m_overlayContentLayerId );
    m_cTEncTop.setOverlaySEILabelPresentFlag                        ( m_overlayLabelPresentFlag );
    m_cTEncTop.setOverlaySEILabelLayerId                            ( m_overlayLabelLayerId );
    m_cTEncTop.setOverlaySEIAlphaPresentFlag                        ( m_overlayAlphaPresentFlag );
    m_cTEncTop.setOverlaySEIAlphaLayerId                            ( m_overlayAlphaLayerId );
    m_cTEncTop.setOverlaySEINumElementsMinus1                       ( m_numOverlayElementsMinus1 );
    m_cTEncTop.setOverlaySEIElementLabelMin                         ( m_overlayElementLabelMin );
    m_cTEncTop.setOverlaySEIElementLabelMax                         ( m_overlayElementLabelMax );
    m_cTEncTop.setOverlaySEILanguage                                ( m_overlayLanguage );
    m_cTEncTop.setOverlaySEIName                                    ( m_overlayName );
    m_cTEncTop.setOverlaySEIElementName                             ( m_overlayElementName );
    m_cTEncTop.setOverlaySEIPersistenceFlag                         ( m_overlayInfoPersistenceFlag );
#endif  
#if Q0189_TMVP_CONSTRAINTS
    m_cTEncTop.setTMVPConstraintsSEIEnabled                         ( m_TMVPConstraintsSEIEnabled);           
#endif
#if N0383_IL_CONSTRAINED_TILE_SETS_SEI
    m_cTEncTop.setInterLayerConstrainedTileSetsSEIEnabled           ( m_interLayerConstrainedTileSetsSEIEnabled );
    m_cTEncTop.setIlNumSetsInMessage                                ( m_ilNumSetsInMessage );
    m_cTEncTop.setSkippedTileSetPresentFlag                         ( m_skippedTileSetPresentFlag );
    m_cTEncTop.setTopLeftTileIndex                                  ( m_topLeftTileIndex );
    m_cTEncTop.setBottomRightTileIndex                              ( m_bottomRightTileIndex );
    m_cTEncTop.setIlcIdc                                            ( m_ilcIdc );
#endif
#if VIEW_SCALABILITY 
    m_cTEncTop.setUseDisparitySearchRangeRestriction                ( m_scalabilityMask[VIEW_ORDER_INDEX] );
#endif

    Int& layerPTLIdx                                            = m_apcLayerCfg[layer]->m_layerPTLIdx;

    Profile::Name& m_profile                                    = m_profileList[layerPTLIdx];
    Level::Tier&   m_levelTier                                  = m_levelTierList[layerPTLIdx];
    Level::Name&   m_level                                      = m_levelList[layerPTLIdx];
    UInt&          m_bitDepthConstraint                         = m_apcLayerCfg[layer]->m_bitDepthConstraint;
    ChromaFormat&  m_chromaFormatConstraint                     = m_apcLayerCfg[layer]->m_chromaFormatConstraint;
    Bool&          m_intraConstraintFlag                        = m_apcLayerCfg[layer]->m_intraConstraintFlag;
    Bool&          m_onePictureOnlyConstraintFlag               = m_apcLayerCfg[layer]->m_onePictureOnlyConstraintFlag;
    Bool&          m_lowerBitRateConstraintFlag                 = m_apcLayerCfg[layer]->m_lowerBitRateConstraintFlag;
    Bool&          m_progressiveSourceFlag                      = m_progressiveSourceFlagList[layerPTLIdx];
    Bool&          m_interlacedSourceFlag                       = m_interlacedSourceFlagList[layerPTLIdx];
    Bool&          m_nonPackedConstraintFlag                    = m_nonPackedConstraintFlagList[layerPTLIdx];
    Bool&          m_frameOnlyConstraintFlag                    = m_frameOnlyConstraintFlagList[layerPTLIdx];

    Int&           m_iFrameRate                                 = m_apcLayerCfg[layer]->m_iFrameRate;
    Int&           m_iSourceWidth                               = m_apcLayerCfg[layer]->m_iSourceWidth;
    Int&           m_iSourceHeight                              = m_apcLayerCfg[layer]->m_iSourceHeight;
    Int&           m_confWinLeft                                = m_apcLayerCfg[layer]->m_confWinLeft;
    Int&           m_confWinRight                               = m_apcLayerCfg[layer]->m_confWinRight;
    Int&           m_confWinTop                                 = m_apcLayerCfg[layer]->m_confWinTop;
    Int&           m_confWinBottom                              = m_apcLayerCfg[layer]->m_confWinBottom;

    Int&           m_iIntraPeriod                               = m_apcLayerCfg[layer]->m_iIntraPeriod;
    Int&           m_iQP                                        = m_apcLayerCfg[layer]->m_iQP;
    Int           *m_aiPad                                      = m_apcLayerCfg[layer]->m_aiPad;

    Int&           m_iMaxCuDQPDepth                             = m_apcLayerCfg[layer]->m_iMaxCuDQPDepth;
    ChromaFormat&  m_chromaFormatIDC                            = m_apcLayerCfg[layer]->m_chromaFormatIDC;
    Int           *m_aidQP                                      = m_apcLayerCfg[layer]->m_aidQP;

    UInt&          m_uiMaxCUWidth                               = m_apcLayerCfg[layer]->m_uiMaxCUWidth;
    UInt&          m_uiMaxCUHeight                              = m_apcLayerCfg[layer]->m_uiMaxCUHeight;
    UInt&          m_uiMaxTotalCUDepth                          = m_apcLayerCfg[layer]->m_uiMaxTotalCUDepth;
    UInt&          m_uiLog2DiffMaxMinCodingBlockSize            = m_apcLayerCfg[layer]->m_uiLog2DiffMaxMinCodingBlockSize;
    UInt&          m_uiQuadtreeTULog2MaxSize                    = m_apcLayerCfg[layer]->m_uiQuadtreeTULog2MaxSize;
    UInt&          m_uiQuadtreeTULog2MinSize                    = m_apcLayerCfg[layer]->m_uiQuadtreeTULog2MinSize;
    UInt&          m_uiQuadtreeTUMaxDepthInter                  = m_apcLayerCfg[layer]->m_uiQuadtreeTUMaxDepthInter;
    UInt&          m_uiQuadtreeTUMaxDepthIntra                  = m_apcLayerCfg[layer]->m_uiQuadtreeTUMaxDepthIntra;

    Bool&          m_entropyCodingSyncEnabledFlag               = m_apcLayerCfg[layer]->m_entropyCodingSyncEnabledFlag;
    Bool&          m_RCEnableRateControl                        = m_apcLayerCfg[layer]->m_RCEnableRateControl;
    Int&           m_RCTargetBitrate                            = m_apcLayerCfg[layer]->m_RCTargetBitrate;
    Bool&          m_RCKeepHierarchicalBit                      = m_apcLayerCfg[layer]->m_RCKeepHierarchicalBit;
    Bool&          m_RCLCULevelRC                               = m_apcLayerCfg[layer]->m_RCLCULevelRC;
    Bool&          m_RCUseLCUSeparateModel                      = m_apcLayerCfg[layer]->m_RCUseLCUSeparateModel;
    Int&           m_RCInitialQP                                = m_apcLayerCfg[layer]->m_RCInitialQP;
    Bool&          m_RCForceIntraQP                             = m_apcLayerCfg[layer]->m_RCForceIntraQP;

#if U0132_TARGET_BITS_SATURATION
    Bool&          m_RCCpbSaturationEnabled                     = m_apcLayerCfg[layer]->m_RCCpbSaturationEnabled;
    UInt&          m_RCCpbSize                                  = m_apcLayerCfg[layer]->m_RCCpbSize;
    Double&        m_RCInitialCpbFullness                       = m_apcLayerCfg[layer]->m_RCInitialCpbFullness;
#endif

    ScalingListMode& m_useScalingListId                         = m_apcLayerCfg[layer]->m_useScalingListId;
    string&        m_scalingListFileName                        = m_apcLayerCfg[layer]->m_scalingListFileName;
    Bool&          m_bUseSAO                                    = m_apcLayerCfg[layer]->m_bUseSAO;

    GOPEntry*      m_GOPList                                    = m_apcLayerCfg[layer]->m_GOPList;
    Int&           m_extraRPSs                                  = m_apcLayerCfg[layer]->m_extraRPSs;
    Int&           m_maxTempLayer                               = m_apcLayerCfg[layer]->m_maxTempLayer;

    string&        m_colourRemapSEIFileRoot                     = m_apcLayerCfg[layer]->m_colourRemapSEIFileRoot;
#if PER_LAYER_LOSSLESS
    Bool&          m_TransquantBypassEnabledFlag                = m_apcLayerCfg[layer]->m_TransquantBypassEnabledFlag;
    Bool&          m_CUTransquantBypassFlagForce                = m_apcLayerCfg[layer]->m_CUTransquantBypassFlagForce;
    CostMode&      m_costMode                                   = m_apcLayerCfg[layer]->m_costMode;
#endif
#endif

  m_cTEncTop.setProfile                                           ( m_profile);
  m_cTEncTop.setLevel                                             ( m_levelTier, m_level);
  m_cTEncTop.setProgressiveSourceFlag                             ( m_progressiveSourceFlag);
  m_cTEncTop.setInterlacedSourceFlag                              ( m_interlacedSourceFlag);
  m_cTEncTop.setNonPackedConstraintFlag                           ( m_nonPackedConstraintFlag);
  m_cTEncTop.setFrameOnlyConstraintFlag                           ( m_frameOnlyConstraintFlag);
  m_cTEncTop.setBitDepthConstraintValue                           ( m_bitDepthConstraint );
  m_cTEncTop.setChromaFormatConstraintValue                       ( m_chromaFormatConstraint );
  m_cTEncTop.setIntraConstraintFlag                               ( m_intraConstraintFlag );
  m_cTEncTop.setOnePictureOnlyConstraintFlag                      ( m_onePictureOnlyConstraintFlag );
  m_cTEncTop.setLowerBitRateConstraintFlag                        ( m_lowerBitRateConstraintFlag );

  m_cTEncTop.setPrintMSEBasedSequencePSNR                         ( m_printMSEBasedSequencePSNR);
  m_cTEncTop.setPrintFrameMSE                                     ( m_printFrameMSE);
  m_cTEncTop.setPrintSequenceMSE                                  ( m_printSequenceMSE);
  m_cTEncTop.setCabacZeroWordPaddingEnabled                       ( m_cabacZeroWordPaddingEnabled );

  m_cTEncTop.setFrameRate                                         ( m_iFrameRate );
  m_cTEncTop.setFrameSkip                                         ( m_FrameSkip );
  m_cTEncTop.setTemporalSubsampleRatio                            ( m_temporalSubsampleRatio );
  m_cTEncTop.setSourceWidth                                       ( m_iSourceWidth );
  m_cTEncTop.setSourceHeight                                      ( m_iSourceHeight );
  m_cTEncTop.setConformanceWindow                                 ( m_confWinLeft, m_confWinRight, m_confWinTop, m_confWinBottom );
  m_cTEncTop.setFramesToBeEncoded                                 ( m_framesToBeEncoded );

  //====== Coding Structure ========
  m_cTEncTop.setIntraPeriod                                       ( m_iIntraPeriod );
  m_cTEncTop.setDecodingRefreshType                               ( m_iDecodingRefreshType );
  m_cTEncTop.setGOPSize                                           ( m_iGOPSize );
  m_cTEncTop.setGopList                                           ( m_GOPList );
  m_cTEncTop.setExtraRPSs                                         ( m_extraRPSs );
  for(Int i = 0; i < MAX_TLAYER; i++)
  {
    m_cTEncTop.setNumReorderPics                                  ( m_numReorderPics[i], i );
    m_cTEncTop.setMaxDecPicBuffering                              ( m_maxDecPicBuffering[i], i );
  }
  for( UInt uiLoop = 0; uiLoop < MAX_TLAYER; ++uiLoop )
  {
    m_cTEncTop.setLambdaModifier                                  ( uiLoop, m_adLambdaModifier[ uiLoop ] );
  }
  m_cTEncTop.setIntraLambdaModifier                               ( m_adIntraLambdaModifier );
  m_cTEncTop.setIntraQpFactor                                     ( m_dIntraQpFactor );

  m_cTEncTop.setQP                                                ( m_iQP );

  m_cTEncTop.setPad                                               ( m_aiPad );

  m_cTEncTop.setAccessUnitDelimiter                               ( m_AccessUnitDelimiter );

  m_cTEncTop.setMaxTempLayer                                      ( m_maxTempLayer );
  m_cTEncTop.setUseAMP( m_enableAMP );

  //===== Slice ========

  //====== Loop/Deblock Filter ========
  m_cTEncTop.setLoopFilterDisable                                 ( m_bLoopFilterDisable       );
  m_cTEncTop.setLoopFilterOffsetInPPS                             ( m_loopFilterOffsetInPPS );
  m_cTEncTop.setLoopFilterBetaOffset                              ( m_loopFilterBetaOffsetDiv2  );
  m_cTEncTop.setLoopFilterTcOffset                                ( m_loopFilterTcOffsetDiv2    );
#if W0038_DB_OPT
  m_cTEncTop.setDeblockingFilterMetric                            ( m_deblockingFilterMetric );
#else
  m_cTEncTop.setDeblockingFilterMetric                            ( m_DeblockingFilterMetric );
#endif

  //====== Motion search ========
  m_cTEncTop.setDisableIntraPUsInInterSlices                      ( m_bDisableIntraPUsInInterSlices );
  m_cTEncTop.setMotionEstimationSearchMethod                      ( m_motionEstimationSearchMethod  );
  m_cTEncTop.setSearchRange                                       ( m_iSearchRange );
  m_cTEncTop.setBipredSearchRange                                 ( m_bipredSearchRange );
  m_cTEncTop.setClipForBiPredMeEnabled                            ( m_bClipForBiPredMeEnabled );
  m_cTEncTop.setFastMEAssumingSmootherMVEnabled                   ( m_bFastMEAssumingSmootherMVEnabled );
  m_cTEncTop.setMinSearchWindow                                   ( m_minSearchWindow );
  m_cTEncTop.setRestrictMESampling                                ( m_bRestrictMESampling );

  //====== Quality control ========
  m_cTEncTop.setMaxDeltaQP                                        ( m_iMaxDeltaQP  );
  m_cTEncTop.setMaxCuDQPDepth                                     ( m_iMaxCuDQPDepth  );
  m_cTEncTop.setDiffCuChromaQpOffsetDepth                         ( m_diffCuChromaQpOffsetDepth );
  m_cTEncTop.setChromaCbQpOffset                                  ( m_cbQpOffset     );
  m_cTEncTop.setChromaCrQpOffset                                  ( m_crQpOffset  );
#if W0038_CQP_ADJ
  m_cTEncTop.setSliceChromaOffsetQpIntraOrPeriodic                ( m_sliceChromaQpOffsetPeriodicity, m_sliceChromaQpOffsetIntraOrPeriodic );
#endif
  m_cTEncTop.setChromaFormatIdc                                   ( m_chromaFormatIDC  );

#if ADAPTIVE_QP_SELECTION
  m_cTEncTop.setUseAdaptQpSelect                                  ( m_bUseAdaptQpSelect   );
#endif

  m_cTEncTop.setUseAdaptiveQP                                     ( m_bUseAdaptiveQP  );
  m_cTEncTop.setQPAdaptationRange                                 ( m_iQPAdaptationRange );
  m_cTEncTop.setExtendedPrecisionProcessingFlag                   ( m_extendedPrecisionProcessingFlag );
  m_cTEncTop.setHighPrecisionOffsetsEnabledFlag                   ( m_highPrecisionOffsetsEnabledFlag );

  m_cTEncTop.setWeightedPredictionMethod( m_weightedPredictionMethod );

  //====== Tool list ========
  m_cTEncTop.setDeltaQpRD                                         ( m_uiDeltaQpRD  );
  m_cTEncTop.setFastDeltaQp                                       ( m_bFastDeltaQP  );
  m_cTEncTop.setUseASR                                            ( m_bUseASR      );
  m_cTEncTop.setUseHADME                                          ( m_bUseHADME    );
  m_cTEncTop.setdQPs                                              ( m_aidQP        );
  m_cTEncTop.setUseRDOQ                                           ( m_useRDOQ     );
  m_cTEncTop.setUseRDOQTS                                         ( m_useRDOQTS   );
#if T0196_SELECTIVE_RDOQ
  m_cTEncTop.setUseSelectiveRDOQ                                  ( m_useSelectiveRDOQ );
#endif
  m_cTEncTop.setRDpenalty                                         ( m_rdPenalty );
  m_cTEncTop.setMaxCUWidth                                        ( m_uiMaxCUWidth );
  m_cTEncTop.setMaxCUHeight                                       ( m_uiMaxCUHeight );
  m_cTEncTop.setMaxTotalCUDepth                                   ( m_uiMaxTotalCUDepth );
  m_cTEncTop.setLog2DiffMaxMinCodingBlockSize                     ( m_uiLog2DiffMaxMinCodingBlockSize );
  m_cTEncTop.setQuadtreeTULog2MaxSize                             ( m_uiQuadtreeTULog2MaxSize );
  m_cTEncTop.setQuadtreeTULog2MinSize                             ( m_uiQuadtreeTULog2MinSize );
  m_cTEncTop.setQuadtreeTUMaxDepthInter                           ( m_uiQuadtreeTUMaxDepthInter );
  m_cTEncTop.setQuadtreeTUMaxDepthIntra                           ( m_uiQuadtreeTUMaxDepthIntra );
  m_cTEncTop.setFastInterSearchMode                               ( m_fastInterSearchMode );
  m_cTEncTop.setUseEarlyCU                                        ( m_bUseEarlyCU  );
  m_cTEncTop.setUseFastDecisionForMerge                           ( m_useFastDecisionForMerge  );
  m_cTEncTop.setUseCbfFastMode                                    ( m_bUseCbfFastMode  );
  m_cTEncTop.setUseEarlySkipDetection                             ( m_useEarlySkipDetection );
  m_cTEncTop.setCrossComponentPredictionEnabledFlag               ( m_crossComponentPredictionEnabledFlag );
  m_cTEncTop.setUseReconBasedCrossCPredictionEstimate             ( m_reconBasedCrossCPredictionEstimate );
  m_cTEncTop.setLog2SaoOffsetScale                                ( CHANNEL_TYPE_LUMA  , m_log2SaoOffsetScale[CHANNEL_TYPE_LUMA]   );
  m_cTEncTop.setLog2SaoOffsetScale                                ( CHANNEL_TYPE_CHROMA, m_log2SaoOffsetScale[CHANNEL_TYPE_CHROMA] );
  m_cTEncTop.setUseTransformSkip                                  ( m_useTransformSkip      );
  m_cTEncTop.setUseTransformSkipFast                              ( m_useTransformSkipFast  );
  m_cTEncTop.setTransformSkipRotationEnabledFlag                  ( m_transformSkipRotationEnabledFlag );
  m_cTEncTop.setTransformSkipContextEnabledFlag                   ( m_transformSkipContextEnabledFlag   );
  m_cTEncTop.setPersistentRiceAdaptationEnabledFlag               ( m_persistentRiceAdaptationEnabledFlag );
  m_cTEncTop.setCabacBypassAlignmentEnabledFlag                   ( m_cabacBypassAlignmentEnabledFlag );
  m_cTEncTop.setLog2MaxTransformSkipBlockSize                     ( m_log2MaxTransformSkipBlockSize  );
  for (UInt signallingModeIndex = 0; signallingModeIndex < NUMBER_OF_RDPCM_SIGNALLING_MODES; signallingModeIndex++)
  {
    m_cTEncTop.setRdpcmEnabledFlag                                ( RDPCMSignallingMode(signallingModeIndex), m_rdpcmEnabledFlag[signallingModeIndex]);
  }
  m_cTEncTop.setUseConstrainedIntraPred                           ( m_bUseConstrainedIntraPred );
  m_cTEncTop.setFastUDIUseMPMEnabled                              ( m_bFastUDIUseMPMEnabled );
  m_cTEncTop.setFastMEForGenBLowDelayEnabled                      ( m_bFastMEForGenBLowDelayEnabled );
  m_cTEncTop.setUseBLambdaForNonKeyLowDelayPictures               ( m_bUseBLambdaForNonKeyLowDelayPictures );
  m_cTEncTop.setPCMLog2MinSize                                    ( m_uiPCMLog2MinSize);
  m_cTEncTop.setUsePCM                                            ( m_usePCM );

  // set internal bit-depth and constants
  for (UInt channelType = 0; channelType < MAX_NUM_CHANNEL_TYPE; channelType++)
  {
#if SVC_EXTENSION
    m_cTEncTop.setBitDepth((ChannelType)channelType, m_apcLayerCfg[layer]->m_internalBitDepth[channelType]);
    m_cTEncTop.setPCMBitDepth((ChannelType)channelType, m_bPCMInputBitDepthFlag ? m_apcLayerCfg[layer]->m_MSBExtendedBitDepth[channelType] : m_apcLayerCfg[layer]->m_internalBitDepth[channelType]);
#else
    m_cTEncTop.setBitDepth((ChannelType)channelType, m_internalBitDepth[channelType]);
    m_cTEncTop.setPCMBitDepth((ChannelType)channelType, m_bPCMInputBitDepthFlag ? m_MSBExtendedBitDepth[channelType] : m_internalBitDepth[channelType]);
#endif
  }

  m_cTEncTop.setPCMLog2MaxSize                                    ( m_pcmLog2MaxSize);
  m_cTEncTop.setMaxNumMergeCand                                   ( m_maxNumMergeCand );


  //====== Weighted Prediction ========
  m_cTEncTop.setUseWP                                             ( m_useWeightedPred     );
  m_cTEncTop.setWPBiPred                                          ( m_useWeightedBiPred   );

  //====== Parallel Merge Estimation ========
  m_cTEncTop.setLog2ParallelMergeLevelMinus2                      ( m_log2ParallelMergeLevel - 2 );

  //====== Slice ========
  m_cTEncTop.setSliceMode                                         ( m_sliceMode );
  m_cTEncTop.setSliceArgument                                     ( m_sliceArgument );

  //====== Dependent Slice ========
  m_cTEncTop.setSliceSegmentMode                                  ( m_sliceSegmentMode );
  m_cTEncTop.setSliceSegmentArgument                              ( m_sliceSegmentArgument );

  if(m_sliceMode == NO_SLICES )
  {
    m_bLFCrossSliceBoundaryFlag = true;
  }
  m_cTEncTop.setLFCrossSliceBoundaryFlag                          ( m_bLFCrossSliceBoundaryFlag );
  m_cTEncTop.setUseSAO                                            ( m_bUseSAO );
  m_cTEncTop.setTestSAODisableAtPictureLevel                      ( m_bTestSAODisableAtPictureLevel );
  m_cTEncTop.setSaoEncodingRate                                   ( m_saoEncodingRate );
  m_cTEncTop.setSaoEncodingRateChroma                             ( m_saoEncodingRateChroma );
  m_cTEncTop.setMaxNumOffsetsPerPic                               ( m_maxNumOffsetsPerPic);

  m_cTEncTop.setSaoCtuBoundary                                    ( m_saoCtuBoundary);
#if OPTIONAL_RESET_SAO_ENCODING_AFTER_IRAP
  m_cTEncTop.setSaoResetEncoderStateAfterIRAP                     ( m_saoResetEncoderStateAfterIRAP);
#endif
  m_cTEncTop.setPCMInputBitDepthFlag                              ( m_bPCMInputBitDepthFlag);
  m_cTEncTop.setPCMFilterDisableFlag                              ( m_bPCMFilterDisableFlag);

  m_cTEncTop.setIntraSmoothingDisabledFlag                        (!m_enableIntraReferenceSmoothing );
  m_cTEncTop.setDecodedPictureHashSEIType                         ( m_decodedPictureHashSEIType );
  m_cTEncTop.setRecoveryPointSEIEnabled                           ( m_recoveryPointSEIEnabled );
  m_cTEncTop.setBufferingPeriodSEIEnabled                         ( m_bufferingPeriodSEIEnabled );
  m_cTEncTop.setPictureTimingSEIEnabled                           ( m_pictureTimingSEIEnabled );
  m_cTEncTop.setToneMappingInfoSEIEnabled                         ( m_toneMappingInfoSEIEnabled );
  m_cTEncTop.setTMISEIToneMapId                                   ( m_toneMapId );
  m_cTEncTop.setTMISEIToneMapCancelFlag                           ( m_toneMapCancelFlag );
  m_cTEncTop.setTMISEIToneMapPersistenceFlag                      ( m_toneMapPersistenceFlag );
  m_cTEncTop.setTMISEICodedDataBitDepth                           ( m_toneMapCodedDataBitDepth );
  m_cTEncTop.setTMISEITargetBitDepth                              ( m_toneMapTargetBitDepth );
  m_cTEncTop.setTMISEIModelID                                     ( m_toneMapModelId );
  m_cTEncTop.setTMISEIMinValue                                    ( m_toneMapMinValue );
  m_cTEncTop.setTMISEIMaxValue                                    ( m_toneMapMaxValue );
  m_cTEncTop.setTMISEISigmoidMidpoint                             ( m_sigmoidMidpoint );
  m_cTEncTop.setTMISEISigmoidWidth                                ( m_sigmoidWidth );
  m_cTEncTop.setTMISEIStartOfCodedInterva                         ( m_startOfCodedInterval );
  m_cTEncTop.setTMISEINumPivots                                   ( m_numPivots );
  m_cTEncTop.setTMISEICodedPivotValue                             ( m_codedPivotValue );
  m_cTEncTop.setTMISEITargetPivotValue                            ( m_targetPivotValue );
  m_cTEncTop.setTMISEICameraIsoSpeedIdc                           ( m_cameraIsoSpeedIdc );
  m_cTEncTop.setTMISEICameraIsoSpeedValue                         ( m_cameraIsoSpeedValue );
  m_cTEncTop.setTMISEIExposureIndexIdc                            ( m_exposureIndexIdc );
  m_cTEncTop.setTMISEIExposureIndexValue                          ( m_exposureIndexValue );
  m_cTEncTop.setTMISEIExposureCompensationValueSignFlag           ( m_exposureCompensationValueSignFlag );
  m_cTEncTop.setTMISEIExposureCompensationValueNumerator          ( m_exposureCompensationValueNumerator );
  m_cTEncTop.setTMISEIExposureCompensationValueDenomIdc           ( m_exposureCompensationValueDenomIdc );
  m_cTEncTop.setTMISEIRefScreenLuminanceWhite                     ( m_refScreenLuminanceWhite );
  m_cTEncTop.setTMISEIExtendedRangeWhiteLevel                     ( m_extendedRangeWhiteLevel );
  m_cTEncTop.setTMISEINominalBlackLevelLumaCodeValue              ( m_nominalBlackLevelLumaCodeValue );
  m_cTEncTop.setTMISEINominalWhiteLevelLumaCodeValue              ( m_nominalWhiteLevelLumaCodeValue );
  m_cTEncTop.setTMISEIExtendedWhiteLevelLumaCodeValue             ( m_extendedWhiteLevelLumaCodeValue );
  m_cTEncTop.setChromaResamplingFilterHintEnabled                 ( m_chromaResamplingFilterSEIenabled );
  m_cTEncTop.setChromaResamplingHorFilterIdc                      ( m_chromaResamplingHorFilterIdc );
  m_cTEncTop.setChromaResamplingVerFilterIdc                      ( m_chromaResamplingVerFilterIdc );
  m_cTEncTop.setFramePackingArrangementSEIEnabled                 ( m_framePackingSEIEnabled );
  m_cTEncTop.setFramePackingArrangementSEIType                    ( m_framePackingSEIType );
  m_cTEncTop.setFramePackingArrangementSEIId                      ( m_framePackingSEIId );
  m_cTEncTop.setFramePackingArrangementSEIQuincunx                ( m_framePackingSEIQuincunx );
  m_cTEncTop.setFramePackingArrangementSEIInterpretation          ( m_framePackingSEIInterpretation );
  m_cTEncTop.setSegmentedRectFramePackingArrangementSEIEnabled    ( m_segmentedRectFramePackingSEIEnabled );
  m_cTEncTop.setSegmentedRectFramePackingArrangementSEICancel     ( m_segmentedRectFramePackingSEICancel );
  m_cTEncTop.setSegmentedRectFramePackingArrangementSEIType       ( m_segmentedRectFramePackingSEIType );
  m_cTEncTop.setSegmentedRectFramePackingArrangementSEIPersistence( m_segmentedRectFramePackingSEIPersistence );
  m_cTEncTop.setDisplayOrientationSEIAngle                        ( m_displayOrientationSEIAngle );
  m_cTEncTop.setTemporalLevel0IndexSEIEnabled                     ( m_temporalLevel0IndexSEIEnabled );
  m_cTEncTop.setGradualDecodingRefreshInfoEnabled                 ( m_gradualDecodingRefreshInfoEnabled );
  m_cTEncTop.setNoDisplaySEITLayer                                ( m_noDisplaySEITLayer );
  m_cTEncTop.setDecodingUnitInfoSEIEnabled                        ( m_decodingUnitInfoSEIEnabled );
  m_cTEncTop.setSOPDescriptionSEIEnabled                          ( m_SOPDescriptionSEIEnabled );
  m_cTEncTop.setScalableNestingSEIEnabled                         ( m_scalableNestingSEIEnabled );
  m_cTEncTop.setTMCTSSEIEnabled                                   ( m_tmctsSEIEnabled );
  m_cTEncTop.setTimeCodeSEIEnabled                                ( m_timeCodeSEIEnabled );
  m_cTEncTop.setNumberOfTimeSets                                  ( m_timeCodeSEINumTs );
  for(Int i = 0; i < m_timeCodeSEINumTs; i++)
  {
    m_cTEncTop.setTimeSet(m_timeSetArray[i], i);
  }
  m_cTEncTop.setKneeSEIEnabled                                    ( m_kneeSEIEnabled );
  m_cTEncTop.setKneeSEIId                                         ( m_kneeSEIId );
  m_cTEncTop.setKneeSEICancelFlag                                 ( m_kneeSEICancelFlag );
  m_cTEncTop.setKneeSEIPersistenceFlag                            ( m_kneeSEIPersistenceFlag );
  m_cTEncTop.setKneeSEIInputDrange                                ( m_kneeSEIInputDrange );
  m_cTEncTop.setKneeSEIInputDispLuminance                         ( m_kneeSEIInputDispLuminance );
  m_cTEncTop.setKneeSEIOutputDrange                               ( m_kneeSEIOutputDrange );
  m_cTEncTop.setKneeSEIOutputDispLuminance                        ( m_kneeSEIOutputDispLuminance );
  m_cTEncTop.setKneeSEINumKneePointsMinus1                        ( m_kneeSEINumKneePointsMinus1 );
  m_cTEncTop.setKneeSEIInputKneePoint                             ( m_kneeSEIInputKneePoint );
  m_cTEncTop.setKneeSEIOutputKneePoint                            ( m_kneeSEIOutputKneePoint );
  m_cTEncTop.setColourRemapInfoSEIFileRoot                        ( m_colourRemapSEIFileRoot );
  m_cTEncTop.setMasteringDisplaySEI                               ( m_masteringDisplay );
#if U0033_ALTERNATIVE_TRANSFER_CHARACTERISTICS_SEI
  m_cTEncTop.setSEIAlternativeTransferCharacteristicsSEIEnable    ( m_preferredTransferCharacteristics>=0     );
  m_cTEncTop.setSEIPreferredTransferCharacteristics               ( UChar(m_preferredTransferCharacteristics) );
#endif
  m_cTEncTop.setSEIGreenMetadataInfoSEIEnable                     ( m_greenMetadataType > 0 );
  m_cTEncTop.setSEIGreenMetadataType                              ( UChar(m_greenMetadataType) );
  m_cTEncTop.setSEIXSDMetricType                                  ( UChar(m_xsdMetricType) );

  m_cTEncTop.setTileUniformSpacingFlag                            ( m_tileUniformSpacingFlag );
  m_cTEncTop.setNumColumnsMinus1                                  ( m_numTileColumnsMinus1 );
  m_cTEncTop.setNumRowsMinus1                                     ( m_numTileRowsMinus1 );
  if(!m_tileUniformSpacingFlag)
  {
    m_cTEncTop.setColumnWidth                                     ( m_tileColumnWidth );
    m_cTEncTop.setRowHeight                                       ( m_tileRowHeight );
  }
  m_cTEncTop.xCheckGSParameters();
  Int uiTilesCount = (m_numTileRowsMinus1+1) * (m_numTileColumnsMinus1+1);
  if(uiTilesCount == 1)
  {
    m_bLFCrossTileBoundaryFlag = true;
  }
  m_cTEncTop.setLFCrossTileBoundaryFlag                           ( m_bLFCrossTileBoundaryFlag );
  m_cTEncTop.setEntropyCodingSyncEnabledFlag                      ( m_entropyCodingSyncEnabledFlag );
  m_cTEncTop.setTMVPModeId                                        ( m_TMVPModeId );
  m_cTEncTop.setUseScalingListId                                  ( m_useScalingListId  );
  m_cTEncTop.setScalingListFileName                               ( m_scalingListFileName );
  m_cTEncTop.setSignDataHidingEnabledFlag                         ( m_signDataHidingEnabledFlag);
  m_cTEncTop.setUseRateCtrl                                       ( m_RCEnableRateControl );
  m_cTEncTop.setTargetBitrate                                     ( m_RCTargetBitrate );
  m_cTEncTop.setKeepHierBit                                       ( m_RCKeepHierarchicalBit );
  m_cTEncTop.setLCULevelRC                                        ( m_RCLCULevelRC );
  m_cTEncTop.setUseLCUSeparateModel                               ( m_RCUseLCUSeparateModel );
  m_cTEncTop.setInitialQP                                         ( m_RCInitialQP );
  m_cTEncTop.setForceIntraQP                                      ( m_RCForceIntraQP );
#if U0132_TARGET_BITS_SATURATION
  m_cTEncTop.setCpbSaturationEnabled                              ( m_RCCpbSaturationEnabled );
  m_cTEncTop.setCpbSize                                           ( m_RCCpbSize );
  m_cTEncTop.setInitialCpbFullness                                ( m_RCInitialCpbFullness );
#endif
  m_cTEncTop.setTransquantBypassEnabledFlag                       ( m_TransquantBypassEnabledFlag );
  m_cTEncTop.setCUTransquantBypassFlagForceValue                  ( m_CUTransquantBypassFlagForce );
  m_cTEncTop.setCostMode                                          ( m_costMode );
  m_cTEncTop.setUseRecalculateQPAccordingToLambda                 ( m_recalculateQPAccordingToLambda );
  m_cTEncTop.setUseStrongIntraSmoothing                           ( m_useStrongIntraSmoothing );
  m_cTEncTop.setActiveParameterSetsSEIEnabled                     ( m_activeParameterSetsSEIEnabled );
  m_cTEncTop.setVuiParametersPresentFlag                          ( m_vuiParametersPresentFlag );
  m_cTEncTop.setAspectRatioInfoPresentFlag                        ( m_aspectRatioInfoPresentFlag);
  m_cTEncTop.setAspectRatioIdc                                    ( m_aspectRatioIdc );
  m_cTEncTop.setSarWidth                                          ( m_sarWidth );
  m_cTEncTop.setSarHeight                                         ( m_sarHeight );
  m_cTEncTop.setOverscanInfoPresentFlag                           ( m_overscanInfoPresentFlag );
  m_cTEncTop.setOverscanAppropriateFlag                           ( m_overscanAppropriateFlag );
  m_cTEncTop.setVideoSignalTypePresentFlag                        ( m_videoSignalTypePresentFlag );
  m_cTEncTop.setVideoFormat                                       ( m_videoFormat );
  m_cTEncTop.setVideoFullRangeFlag                                ( m_videoFullRangeFlag );
  m_cTEncTop.setColourDescriptionPresentFlag                      ( m_colourDescriptionPresentFlag );
  m_cTEncTop.setColourPrimaries                                   ( m_colourPrimaries );
  m_cTEncTop.setTransferCharacteristics                           ( m_transferCharacteristics );
  m_cTEncTop.setMatrixCoefficients                                ( m_matrixCoefficients );
  m_cTEncTop.setChromaLocInfoPresentFlag                          ( m_chromaLocInfoPresentFlag );
  m_cTEncTop.setChromaSampleLocTypeTopField                       ( m_chromaSampleLocTypeTopField );
  m_cTEncTop.setChromaSampleLocTypeBottomField                    ( m_chromaSampleLocTypeBottomField );
  m_cTEncTop.setNeutralChromaIndicationFlag                       ( m_neutralChromaIndicationFlag );
  m_cTEncTop.setDefaultDisplayWindow                              ( m_defDispWinLeftOffset, m_defDispWinRightOffset, m_defDispWinTopOffset, m_defDispWinBottomOffset );
  m_cTEncTop.setFrameFieldInfoPresentFlag                         ( m_frameFieldInfoPresentFlag );
  m_cTEncTop.setPocProportionalToTimingFlag                       ( m_pocProportionalToTimingFlag );
  m_cTEncTop.setNumTicksPocDiffOneMinus1                          ( m_numTicksPocDiffOneMinus1    );
  m_cTEncTop.setBitstreamRestrictionFlag                          ( m_bitstreamRestrictionFlag );
  m_cTEncTop.setTilesFixedStructureFlag                           ( m_tilesFixedStructureFlag );
  m_cTEncTop.setMotionVectorsOverPicBoundariesFlag                ( m_motionVectorsOverPicBoundariesFlag );
  m_cTEncTop.setMinSpatialSegmentationIdc                         ( m_minSpatialSegmentationIdc );
  m_cTEncTop.setMaxBytesPerPicDenom                               ( m_maxBytesPerPicDenom );
  m_cTEncTop.setMaxBitsPerMinCuDenom                              ( m_maxBitsPerMinCuDenom );
  m_cTEncTop.setLog2MaxMvLengthHorizontal                         ( m_log2MaxMvLengthHorizontal );
  m_cTEncTop.setLog2MaxMvLengthVertical                           ( m_log2MaxMvLengthVertical );
  m_cTEncTop.setEfficientFieldIRAPEnabled                         ( m_bEfficientFieldIRAPEnabled );
  m_cTEncTop.setHarmonizeGopFirstFieldCoupleEnabled               ( m_bHarmonizeGopFirstFieldCoupleEnabled );

  m_cTEncTop.setSummaryOutFilename                                ( m_summaryOutFilename );
  m_cTEncTop.setSummaryPicFilenameBase                            ( m_summaryPicFilenameBase );
  m_cTEncTop.setSummaryVerboseness                                ( m_summaryVerboseness );

#if LAYERS_NOT_PRESENT_SEI
  m_cTEncTop.setLayersNotPresentSEIEnabled                        ( m_layersNotPresentSEIEnabled );
#endif  


#if SVC_EXTENSION
  if( layer != 0 && m_useInterLayerWeightedPred )
  {
    // Enable weighted prediction for enhancement layer
    m_cTEncTop.setUseWP                                           ( true   );
    m_cTEncTop.setWPBiPred                                        ( true   );
  }

  }
#endif
}

Void TAppEncTop::xCreateLib()
{
  // Video I/O
#if SVC_EXTENSION
  // initialize global variables
  initROM();

  for(UInt layer=0; layer<m_numLayers; layer++)
  {
    //2
    // Video I/O
    m_apcTVideoIOYuvInputFile[layer]->open( m_apcLayerCfg[layer]->getInputFileName(),  false, m_apcLayerCfg[layer]->m_inputBitDepth, m_apcLayerCfg[layer]->m_MSBExtendedBitDepth, m_apcLayerCfg[layer]->m_internalBitDepth );  // read  mode
    m_apcTVideoIOYuvInputFile[layer]->skipFrames(m_FrameSkip, m_apcLayerCfg[layer]->m_iSourceWidth - m_apcLayerCfg[layer]->m_aiPad[0], m_apcLayerCfg[layer]->m_iSourceHeight - m_apcLayerCfg[layer]->m_aiPad[1], m_apcLayerCfg[layer]->m_InputChromaFormatIDC);

    if( !m_apcLayerCfg[layer]->getReconFileName().empty() )
    {
      m_apcTVideoIOYuvReconFile[layer]->open( m_apcLayerCfg[layer]->getReconFileName(), true, m_apcLayerCfg[layer]->m_outputBitDepth, m_apcLayerCfg[layer]->m_MSBExtendedBitDepth, m_apcLayerCfg[layer]->m_internalBitDepth );  // write mode
    }

    // Neo Decoder
    m_apcTEncTop[layer]->create();
  }
#else //SVC_EXTENSION
  // Video I/O
  m_cTVideoIOYuvInputFile.open( m_inputFileName,     false, m_inputBitDepth, m_MSBExtendedBitDepth, m_internalBitDepth );  // read  mode
  m_cTVideoIOYuvInputFile.skipFrames(m_FrameSkip, m_iSourceWidth - m_aiPad[0], m_iSourceHeight - m_aiPad[1], m_InputChromaFormatIDC);

  if (!m_reconFileName.empty())
  {
    m_cTVideoIOYuvReconFile.open(m_reconFileName, true, m_outputBitDepth, m_outputBitDepth, m_internalBitDepth);  // write mode
  }

  // Neo Decoder
  m_cTEncTop.create();
#endif //SVC_EXTENSION
}

Void TAppEncTop::xDestroyLib()
{
  // Video I/O
#if SVC_EXTENSION
  // destroy ROM
  destroyROM();

  for( UInt layer=0; layer < m_numLayers; layer++ )
  {
    if( m_apcTVideoIOYuvInputFile[layer] )
    {
      m_apcTVideoIOYuvInputFile[layer]->close();
      delete m_apcTVideoIOYuvInputFile[layer];
      m_apcTVideoIOYuvInputFile[layer] = NULL;
    }

    if( m_apcTVideoIOYuvReconFile[layer] )
    {
      m_apcTVideoIOYuvReconFile[layer]->close();
      delete m_apcTVideoIOYuvReconFile[layer];
      m_apcTVideoIOYuvReconFile[layer] = NULL;
    }

    if( m_apcTEncTop[layer] )
    {
      m_apcTEncTop[layer]->destroy();
      delete m_apcTEncTop[layer];
      m_apcTEncTop[layer] = NULL;
    }
  }
#else //SVC_EXTENSION
  m_cTVideoIOYuvInputFile.close();
  m_cTVideoIOYuvReconFile.close();

  // Neo Decoder
  m_cTEncTop.destroy();
#endif //SVC_EXTENSION
}

Void TAppEncTop::xInitLib(Bool isFieldCoding)
{
#if SVC_EXTENSION
  TComVPS* vps = m_apcTEncTop[0]->getVPS();
  m_apcTEncTop[0]->getVPS()->setMaxLayers( m_numLayers );

  UInt i = 0, dimIdLen = 0;
#if VIEW_SCALABILITY
  Int curDimId=0;

  if( m_scalabilityMask[VIEW_ORDER_INDEX] )
  {
    UInt maxViewOrderIndex = 0,maxViewId=0;
    UInt voiLen = 1,vidLen=1;

    for( i = 0; i < vps->getMaxLayers(); i++ )
    {
      if( m_apcLayerCfg[i]->getViewOrderIndex() > maxViewOrderIndex )
      {
        maxViewOrderIndex = m_apcLayerCfg[i]->getViewOrderIndex();
      }

      if( m_apcLayerCfg[i]->getViewId() > maxViewId )
      {
        maxViewId = m_apcLayerCfg[i]->getViewId();
      }

    }
    while((1 << voiLen) < (maxViewOrderIndex + 1))
    {
      voiLen++;
    }

    while((1 << vidLen) < (maxViewId + 1))
    {
      vidLen++;
    }

    vps->setDimensionIdLen(0, voiLen);
    vps->setViewIdLen(vidLen);

    for( i = 0; i < vps->getMaxLayers(); i++ )
    {
      vps->setDimensionId(i, 0, m_apcLayerCfg[i]->getViewOrderIndex());          
    }

    for( i = 0; i < m_iNumberOfViews; i++ )
    {
      vps->setViewIdVal(i, m_ViewIdVal[i]); 
    }

    curDimId++;
  }
#endif

  while((1 << dimIdLen) < m_numLayers)
  {
    dimIdLen++;
  }
#if VIEW_SCALABILITY
  vps->setDimensionIdLen(curDimId, dimIdLen); 
#else
  vps->setDimensionIdLen(0, dimIdLen);
#endif
  vps->setNuhLayerIdPresentFlag(false);
  vps->setLayerIdInNuh(0, 0);
  vps->setLayerIdxInVps(0, 0);
  for(i = 1; i < vps->getMaxLayers(); i++)
  {
    vps->setLayerIdInNuh(i, m_apcLayerCfg[i]->m_layerId);    
    vps->setLayerIdxInVps(vps->getLayerIdInNuh(i), i);
#if VIEW_SCALABILITY
    vps->setDimensionId(i, curDimId, i);
#else
    vps->setDimensionId(i, 0, i);
#endif
    if( m_apcLayerCfg[i]->m_layerId != i )
    {
      vps->setNuhLayerIdPresentFlag(true);
    }
  }

  for(UInt layer=0; layer<m_numLayers; layer++)
  {
    //3
#if LAYER_CTB
    memcpy( g_auiZscanToRaster, g_auiLayerZscanToRaster[layer], sizeof( g_auiZscanToRaster ) );
    memcpy( g_auiRasterToZscan, g_auiLayerRasterToZscan[layer], sizeof( g_auiRasterToZscan ) );
    memcpy( g_auiRasterToPelX,  g_auiLayerRasterToPelX[layer],  sizeof( g_auiRasterToPelX ) );
    memcpy( g_auiRasterToPelY,  g_auiLayerRasterToPelY[layer],  sizeof( g_auiRasterToPelY ) );
#endif
    m_apcTEncTop[layer]->init(isFieldCoding);
  }

  // Set max-layer ID
  vps->setMaxLayerId( m_apcLayerCfg[m_numLayers - 1]->m_layerId );
  vps->setVpsExtensionFlag( m_numLayers > 1 ? true : false );

  if( m_numLayerSets > 1 )
  {
    vps->setNumLayerSets(m_numLayerSets);

    for (Int setId = 1; setId < vps->getNumLayerSets(); setId++)
    {
      for (Int layerId = 0; layerId <= vps->getMaxLayerId(); layerId++)
      {
        vps->setLayerIdIncludedFlag(false, setId, layerId);
      }
    }
    for (Int setId = 1; setId < vps->getNumLayerSets(); setId++)
    {
      for( i = 0; i < m_numLayerInIdList[setId]; i++ )
      {
        //4
        vps->setLayerIdIncludedFlag(true, setId, m_layerSetLayerIdList[setId][i]);
      }
    }
  }
  else
  {
    // Default layer sets
    vps->setNumLayerSets(m_numLayers);
    for (Int setId = 1; setId < vps->getNumLayerSets(); setId++)
    {
      for (Int layerIdx = 0; layerIdx <= vps->getMaxLayers(); layerIdx++)
      {
        //4
        UInt layerId = vps->getLayerIdInNuh(layerIdx);

        if (layerId <= setId)
        {
          vps->setLayerIdIncludedFlag(true, setId, layerId);
        }
        else
        {
          vps->setLayerIdIncludedFlag(false, setId, layerId);
        }
      }
    }
  }

  vps->setVpsNumLayerSetsMinus1(vps->getNumLayerSets() - 1);
  vps->setNumAddLayerSets(m_numAddLayerSets);
  vps->setNumLayerSets(vps->getNumLayerSets() + vps->getNumAddLayerSets());

  if( m_numAddLayerSets > 0 )
  {
    for (Int setId = 0; setId < m_numAddLayerSets; setId++)
    {
      for (Int j = 0; j < m_numHighestLayerIdx[setId]; j++)
      {
        vps->setHighestLayerIdxPlus1(setId, j + 1, m_highestLayerIdx[setId][j] + 1);
      }
    }
  }

#if AVC_BASE
  vps->setNonHEVCBaseLayerFlag( m_nonHEVCBaseLayerFlag );
  if( m_nonHEVCBaseLayerFlag )
  {
    vps->setBaseLayerInternalFlag(false);
  }
#endif
  
  for( Int idx = vps->getBaseLayerInternalFlag() ? 2 : 1; idx < vps->getNumProfileTierLevel(); idx++ )
  {
    vps->setProfilePresentFlag(idx, true);
  }

  vps->setSplittingFlag(false);

  for(i = 0; i < MAX_VPS_NUM_SCALABILITY_TYPES; i++)
  {
    vps->setScalabilityMask(i, false);
  }
  if(m_numLayers > 1)
  {
    Int scalabilityTypes = 0;
    for(i = 0; i < MAX_VPS_NUM_SCALABILITY_TYPES; i++)
    {
      vps->setScalabilityMask(i, m_scalabilityMask[i]);
      scalabilityTypes += m_scalabilityMask[i];
    }
#if VIEW_SCALABILITY
    assert( scalabilityTypes <= 3 );
#else
#if AUXILIARY_PICTURES
    assert( scalabilityTypes <= 2 );
#else
    assert( scalabilityTypes == 1 );
#endif
#endif
    vps->setNumScalabilityTypes(scalabilityTypes);
  }
  else
  {
    vps->setNumScalabilityTypes(0);
  }
  
#if AUXILIARY_PICTURES
  if (m_scalabilityMask[AUX_ID])
  {
    UInt maxAuxId = 0;
    UInt auxDimIdLen = 1;
    Int  auxId = vps->getNumScalabilityTypes() - 1;
    for(i = 1; i < vps->getMaxLayers(); i++)
    {
      if (m_apcLayerCfg[i]->m_auxId > maxAuxId)
      {
        maxAuxId = m_apcLayerCfg[i]->m_auxId;
      }
    }
    while((1 << auxDimIdLen) < (maxAuxId + 1))
    {
      auxDimIdLen++;
    }
    vps->setDimensionIdLen(auxId, auxDimIdLen);
    for(i = 1; i < vps->getMaxLayers(); i++)
    {
      vps->setDimensionId(i, auxId, m_apcLayerCfg[i]->m_auxId);
    }
  }
#endif

  vps->setMaxTSLayersPresentFlag(true);

  for( i = 0; i < vps->getMaxLayers(); i++ )
  {
    vps->setMaxTSLayersMinus1(i, vps->getMaxTLayers()-1);
  }

  vps->setMaxTidRefPresentFlag(m_maxTidRefPresentFlag);
  if (vps->getMaxTidRefPresentFlag())
  {
    for( i = 0; i < vps->getMaxLayers() - 1; i++ )
    {
      for( Int j = i+1; j < vps->getMaxLayers(); j++)
      {
        vps->setMaxTidIlRefPicsPlus1(i, j, m_apcTEncTop[i]->getMaxTidIlRefPicsPlus1());
      }
    }
  }
  else
  {
    for( i = 0; i < vps->getMaxLayers() - 1; i++ )
    {
      for( Int j = i+1; j < vps->getMaxLayers(); j++)
      {
        vps->setMaxTidIlRefPicsPlus1(i, j, 7);
      }
    }
  }

  vps->setDefaultRefLayersActiveFlag(false);

  // Direct reference layers
  UInt maxDirectRefLayers = 0;
  Bool isDefaultDirectDependencyTypeSet = false;
  for (UInt layerCtr = 1; layerCtr < vps->getMaxLayers(); layerCtr++)
  {
    UInt layerId = vps->getLayerIdInNuh(layerCtr);
    Int numDirectRefLayers = 0;

    vps->setNumDirectRefLayers(layerId, m_apcTEncTop[layerCtr]->getNumDirectRefLayers());
    maxDirectRefLayers = max<UInt>(maxDirectRefLayers, vps->getNumDirectRefLayers(layerId));

    for (i = 0; i < vps->getNumDirectRefLayers(layerId); i++)
    {
      vps->setRefLayerId(layerId, i, m_apcTEncTop[layerCtr]->getRefLayerId(i));
    }
    // Set direct dependency flag
    // Initialize flag to 0
    for (Int refLayerCtr = 0; refLayerCtr < layerCtr; refLayerCtr++)
    {
      vps->setDirectDependencyFlag(layerCtr, refLayerCtr, false);
    }
    for (i = 0; i < vps->getNumDirectRefLayers(layerId); i++)
    {
      vps->setDirectDependencyFlag(layerCtr, vps->getLayerIdxInVps(m_apcTEncTop[layerCtr]->getRefLayerId(i)), true);
    }
    // prediction indications
    vps->setDirectDepTypeLen(2); // sample and motion types are encoded
    for (Int refLayerCtr = 0; refLayerCtr < layerCtr; refLayerCtr++)
    {
      if (vps->getDirectDependencyFlag(layerCtr, refLayerCtr))
      {
        assert(m_apcTEncTop[layerCtr]->getSamplePredEnabledFlag(numDirectRefLayers) || m_apcTEncTop[layerCtr]->getMotionPredEnabledFlag(numDirectRefLayers));
        vps->setDirectDependencyType(layerCtr, refLayerCtr, ((m_apcTEncTop[layerCtr]->getSamplePredEnabledFlag(numDirectRefLayers) ? 1 : 0) |
          (m_apcTEncTop[layerCtr]->getMotionPredEnabledFlag(numDirectRefLayers) ? 2 : 0)) - 1);

        if (!isDefaultDirectDependencyTypeSet)
        {
          vps->setDefaultDirectDependecyTypeFlag(1);
          vps->setDefaultDirectDependecyType(vps->getDirectDependencyType(layerCtr, refLayerCtr));
          isDefaultDirectDependencyTypeSet = true;
        }
        else if (vps->getDirectDependencyType(layerCtr, refLayerCtr) != vps->getDefaultDirectDependencyType())
        {
          vps->setDefaultDirectDependecyTypeFlag(0);
        }

        numDirectRefLayers ++;
      }
      else
      {
        vps->setDirectDependencyType(layerCtr, refLayerCtr, 0);
      }
    }
  }

  // dependency constraint
  vps->setNumRefLayers();

  if( vps->getMaxLayers() > MAX_REF_LAYERS )
  {
    for( UInt layerCtr = 1; layerCtr <= vps->getMaxLayers() - 1; layerCtr++ )
    {
      assert(vps->getNumRefLayers(vps->getLayerIdInNuh(layerCtr)) <= MAX_REF_LAYERS);
    }
  }

  // The Layer ID List variables should be derived here.
  vps->deriveLayerIdListVariables();
  vps->setPredictedLayerIds();
  vps->setTreePartitionLayerIdList();
  vps->deriveLayerIdListVariablesForAddLayerSets();

  vps->setDefaultTargetOutputLayerIdc( m_defaultTargetOutputLayerIdc ); // As per configuration file

  if( m_numOutputLayerSets == -1 )  // # of output layer sets not specified in the configuration file
  {
    vps->setNumOutputLayerSets(vps->getNumLayerSets());

    for(i = 1; i < vps->getNumLayerSets(); i++)
    {
        vps->setOutputLayerSetIdx(i, i);
    }
  }
  else
  {
    vps->setNumOutputLayerSets( m_numOutputLayerSets );
    for( Int olsCtr = 0; olsCtr < vps->getNumLayerSets(); olsCtr ++ ) // Default output layer sets
    {
      vps->setOutputLayerSetIdx(olsCtr, olsCtr);
    }
    for( Int olsCtr = vps->getNumLayerSets(); olsCtr < vps->getNumOutputLayerSets(); olsCtr ++ )  // Non-default output layer sets
    {
      vps->setOutputLayerSetIdx(olsCtr, m_outputLayerSetIdx[olsCtr - vps->getNumLayerSets()]);
    }
  }

  // Target output layer
  vps->deriveNumberOfSubDpbs();
  vps->setOutputLayerFlag( 0, 0, 1 );

  // derive OutputLayerFlag[i][j] 
  // default_output_layer_idc equal to 1 specifies that only the layer with the highest value of nuh_layer_id such that nuh_layer_id equal to nuhLayerIdA and 
  // AuxId[ nuhLayerIdA ] equal to 0 in each of the output layer sets with index in the range of 1 to vps_num_layer_sets_minus1, inclusive, is an output layer of its output layer set.

  // Include the highest layer as output layer for each layer set
  for(Int lsIdx = 1; lsIdx <= vps->getVpsNumLayerSetsMinus1(); lsIdx++)
  {
    for( UInt layer = 0; layer < vps->getNumLayersInIdList(lsIdx); layer++ )
    {
      switch( vps->getDefaultTargetOutputLayerIdc() )
      {
      case 0: vps->setOutputLayerFlag( lsIdx, layer, 1 );
        break;
      case 1: vps->setOutputLayerFlag( lsIdx, layer, layer == vps->getNumLayersInIdList(lsIdx) - 1 );
        break;
      case 2:
      case 3: vps->setOutputLayerFlag( lsIdx, layer, (layer != vps->getNumLayersInIdList(lsIdx) - 1) ? std::find( m_listOfOutputLayers[lsIdx].begin(), m_listOfOutputLayers[lsIdx].end(), m_layerSetLayerIdList[lsIdx][layer]) != m_listOfOutputLayers[lsIdx].end() 
                : m_listOfOutputLayers[lsIdx][m_listOfOutputLayers[lsIdx].size()-1] == m_layerSetLayerIdList[lsIdx][layer] );
        break;
      }
    }
  }

  for( Int olsIdx = vps->getVpsNumLayerSetsMinus1() + 1; olsIdx < vps->getNumOutputLayerSets(); olsIdx++ )
  {
    for( UInt layer = 0; layer < vps->getNumLayersInIdList(vps->getOutputLayerSetIdx(olsIdx)); layer++ )
    {
      vps->setOutputLayerFlag( olsIdx, layer, (layer != vps->getNumLayersInIdList(vps->getOutputLayerSetIdx(olsIdx)) - 1) ? std::find( m_listOfOutputLayers[olsIdx].begin(), m_listOfOutputLayers[olsIdx].end(), vps->getLayerSetLayerIdList(vps->getOutputLayerSetIdx(olsIdx), layer)) != m_listOfOutputLayers[olsIdx].end()
        : m_listOfOutputLayers[olsIdx][m_listOfOutputLayers[olsIdx].size()-1] == vps->getLayerSetLayerIdList(vps->getOutputLayerSetIdx(olsIdx), layer) );
    }
  }

  vps->deriveNecessaryLayerFlag();
  vps->checkNecessaryLayerFlagCondition();
  vps->calculateMaxSLInLayerSets();

  // Initialize dpb_size_table() for all ouput layer sets in the VPS extension
  for( i = 1; i < vps->getNumOutputLayerSets(); i++ )
  {
    Int layerSetIdxForOutputLayerSet = vps->getOutputLayerSetIdx( i );
    Int layerSetId = vps->getOutputLayerSetIdx(i);

    for(Int j = 0; j < vps->getMaxTLayers(); j++)
    {

      Int maxNumReorderPics = -1;
      for(Int k = 0; k < vps->getNumSubDpbs(layerSetIdxForOutputLayerSet); k++)
      {
        Int layerId = vps->getLayerSetLayerIdList(layerSetId, k); // k-th layer in the output layer set
        vps->setMaxVpsDecPicBufferingMinus1( i, k, j,  m_apcTEncTop[vps->getLayerIdxInVps(layerId)]->getMaxDecPicBuffering(j) - 1 );
        maxNumReorderPics       = std::max( maxNumReorderPics, m_apcTEncTop[vps->getLayerIdxInVps(layerId)]->getNumReorderPics(j));
      }
      vps->setMaxVpsNumReorderPics(i, j, maxNumReorderPics);
      vps->determineSubDpbInfoFlags();
    }
  }

  vps->setMaxOneActiveRefLayerFlag(maxDirectRefLayers > 1 ? false : true);

  // POC LSB not present flag
  for( i = 1; i< vps->getMaxLayers(); i++ )
  {
    if( vps->getNumDirectRefLayers( vps->getLayerIdInNuh(i) ) == 0  )
    {
      // make independedent layers base-layer compliant
      vps->setPocLsbNotPresentFlag(i, true); 
    }
  }

  vps->setCrossLayerPictureTypeAlignFlag( m_crossLayerPictureTypeAlignFlag );  
  vps->setCrossLayerAlignedIdrOnlyFlag( m_crossLayerAlignedIdrOnlyFlag );
  vps->setCrossLayerIrapAlignFlag( m_crossLayerIrapAlignFlag );

  if( vps->getCrossLayerPictureTypeAlignFlag() )
  {
    // When not present, the value of cross_layer_irap_aligned_flag is inferred to be equal to vps_vui_present_flag,   
    assert( m_crossLayerIrapAlignFlag == true );
    vps->setCrossLayerIrapAlignFlag( true ); 
  }

  for(UInt layerCtr = 1;layerCtr < vps->getMaxLayers(); layerCtr++)
  {
    for(Int refLayerCtr = 0; refLayerCtr < layerCtr; refLayerCtr++)
    {
      if (vps->getDirectDependencyFlag( layerCtr, refLayerCtr))
      {
        assert( layerCtr < MAX_LAYERS );

        if(m_apcTEncTop[layerCtr]->getIntraPeriod() !=  m_apcTEncTop[refLayerCtr]->getIntraPeriod())
        {
          vps->setCrossLayerIrapAlignFlag(false);
          break;
        }
      }
    }
  }
  vps->setSingleLayerForNonIrapFlag(m_adaptiveResolutionChange > 0 ? true : false);
  vps->setHigherLayerIrapSkipFlag(m_skipPictureAtArcSwitch);

  for( Int k = 0; k < MAX_VPS_LAYER_SETS_PLUS1; k++ )
  {
    vps->setAltOuputLayerFlag( k, m_altOutputLayerFlag );
  }

  // VPS VUI BSP HRD parameters
  vps->setVpsVuiBspHrdPresentFlag(false);
  TEncTop *pcCfg = m_apcTEncTop[0];
  if( pcCfg->getBufferingPeriodSEIEnabled() )
  {
    Int j;
    vps->setVpsVuiBspHrdPresentFlag(true);
    vps->setVpsNumAddHrdParams( vps->getMaxLayers() );
    vps->createBspHrdParamBuffer(vps->getVpsNumAddHrdParams() + 1);
    for( i = vps->getNumHrdParameters(), j = 0; i < vps->getNumHrdParameters() + vps->getVpsNumAddHrdParams(); i++, j++ )
    {
      vps->setCprmsAddPresentFlag( j, true );
      vps->setNumSubLayerHrdMinus1( j, vps->getMaxTLayers() - 1 );

      UInt layerIdx = j;
      TEncTop *pcCfgLayer = m_apcTEncTop[layerIdx];

      Int iPicWidth         = pcCfgLayer->getSourceWidth();
      Int iPicHeight        = pcCfgLayer->getSourceHeight();

      UInt uiWidthInCU      = ( iPicWidth  % m_apcLayerCfg[layerIdx]->m_uiMaxCUWidth  ) ? iPicWidth  / m_apcLayerCfg[layerIdx]->m_uiMaxCUWidth  + 1 : iPicWidth  / m_apcLayerCfg[layerIdx]->m_uiMaxCUWidth;
      UInt uiHeightInCU     = ( iPicHeight % m_apcLayerCfg[layerIdx]->m_uiMaxCUHeight ) ? iPicHeight / m_apcLayerCfg[layerIdx]->m_uiMaxCUHeight + 1 : iPicHeight / m_apcLayerCfg[layerIdx]->m_uiMaxCUHeight;
      UInt maxCU = pcCfgLayer->getSliceArgument() >> ( m_apcLayerCfg[layerIdx]->m_uiMaxCUDepth << 1);

      UInt uiNumCUsInFrame   = uiWidthInCU * uiHeightInCU;

      UInt numDU = ( pcCfgLayer->getSliceMode() == 1 ) ? ( uiNumCUsInFrame / maxCU ) : ( 0 );
      if( uiNumCUsInFrame % maxCU != 0 || numDU == 0 )
      {
        numDU ++;
      }
      //vps->getBspHrd(i)->setNumDU( numDU );
      vps->setBspHrdParameters( i, pcCfgLayer->getFrameRate(), numDU, pcCfgLayer->getTargetBitrate(), ( pcCfgLayer->getIntraPeriod() > 0 ) );
    }

    // Signalling of additional partitioning schemes
    for( Int h = 1; h < vps->getNumOutputLayerSets(); h++ )
    {
      Int lsIdx = vps->getOutputLayerSetIdx( h );
      vps->setNumSignalledPartitioningSchemes(h, 1);  // Only the default per-layer partitioning scheme
      for( j = 1; j < vps->getNumSignalledPartitioningSchemes(h); j++ )
      {
        // ToDo: Add code for additional partitioning schemes here
        // ToDo: Initialize num_partitions_in_scheme_minus1 and layer_included_in_partition_flag
      }

      for( i = 0; i < vps->getNumSignalledPartitioningSchemes(h); i++ )
      {
        if( i == 0 )
        {
          for(Int t = 0; t <= vps->getMaxSLayersInLayerSetMinus1( lsIdx ); t++)
          {
            vps->setNumBspSchedulesMinus1( h, i, t, 0 );
            for( j = 0; j <= vps->getNumBspSchedulesMinus1(h, i, t); j++ )
            {
              for( Int k = 0; k <= vps->getNumPartitionsInSchemeMinus1(h, i); k++ )
              {
                // Only for the default partition
                Int nuhlayerId = vps->getLayerSetLayerIdList( lsIdx, k);
                Int layerIdxInVps = vps->getLayerIdxInVps( nuhlayerId );
                vps->setBspHrdIdx(h, i, t, j, k, layerIdxInVps + vps->getNumHrdParameters());

                vps->setBspSchedIdx(h, i, t, j, k, 0);
              }
            }
          }
        }
        else
        {
          assert(0);    // Need to add support for additional partitioning schemes.
        }
      }
    }
  }
#else //SVC_EXTENSION
  m_cTEncTop.init(isFieldCoding);
#endif //SVC_EXTENSION
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/**
 - create internal class
 - initialize internal variable
 - until the end of input YUV file, call encoding function in TEncTop class
 - delete allocated buffers
 - destroy internal class
 .
 */
#if SVC_EXTENSION
Void TAppEncTop::encode()
{
  fstream bitstreamFile(m_bitstreamFileName.c_str(), fstream::binary | fstream::out);
  if (!bitstreamFile)
  {
    fprintf(stderr, "\nfailed to open bitstream file `%s' for writing\n", m_bitstreamFileName.c_str());
    exit(EXIT_FAILURE);
  }

  // allocated memory
  for( Int layer = 0; layer < m_numLayers; layer++ )
  {
    m_apcTVideoIOYuvInputFile[layer] = new TVideoIOYuv;    
    m_apcTVideoIOYuvReconFile[layer] = new TVideoIOYuv;
    m_apcTEncTop[layer] = new TEncTop;
  }

  TComPicYuv*       pcPicYuvOrg [MAX_LAYERS];
  TComPicYuv*       pcPicYuvRec = NULL;

  // initialize internal class & member variables
  xInitLibCfg();
  xCreateLib();
  xInitLib(m_isField);

  // main encoder loop
  Int   iNumEncoded = 0, iTotalNumEncoded = 0;
  Bool  bEos = false;

  const InputColourSpaceConversion ipCSC  =  m_inputColourSpaceConvert;
  const InputColourSpaceConversion snrCSC = (!m_snrInternalColourSpace) ? m_inputColourSpaceConvert : IPCOLOURSPACE_UNCHANGED;

  list<AccessUnit> outputAccessUnits; ///< list of access units to write out.  is populated by the encoding process

  TComPicYuv acPicYuvTrueOrg[MAX_LAYERS];

  for(UInt layer=0; layer<m_numLayers; layer++)
  {
    //5
    // allocate original YUV buffer
    pcPicYuvOrg[layer] = new TComPicYuv;
    if( m_isField )
    {
      pcPicYuvOrg[layer]->create( m_apcLayerCfg[layer]->m_iSourceWidth, m_apcLayerCfg[layer]->m_iSourceHeightOrg, m_apcLayerCfg[layer]->m_chromaFormatIDC, m_apcLayerCfg[layer]->m_uiMaxCUWidth, m_apcLayerCfg[layer]->m_uiMaxCUHeight, m_apcLayerCfg[layer]->m_uiMaxTotalCUDepth, true, NULL );
      acPicYuvTrueOrg[layer].create( m_apcLayerCfg[layer]->m_iSourceWidth, m_apcLayerCfg[layer]->m_iSourceHeightOrg, m_apcLayerCfg[layer]->m_chromaFormatIDC, m_apcLayerCfg[layer]->m_uiMaxCUWidth, m_apcLayerCfg[layer]->m_uiMaxCUHeight, m_apcLayerCfg[layer]->m_uiMaxTotalCUDepth, true, NULL );
    }
    else
    {
      pcPicYuvOrg[layer]->create( m_apcLayerCfg[layer]->m_iSourceWidth, m_apcLayerCfg[layer]->m_iSourceHeight, m_apcLayerCfg[layer]->m_chromaFormatIDC, m_apcLayerCfg[layer]->m_uiMaxCUWidth, m_apcLayerCfg[layer]->m_uiMaxCUHeight, m_apcLayerCfg[layer]->m_uiMaxTotalCUDepth, true, NULL );
      acPicYuvTrueOrg[layer].create( m_apcLayerCfg[layer]->m_iSourceWidth, m_apcLayerCfg[layer]->m_iSourceHeight, m_apcLayerCfg[layer]->m_chromaFormatIDC, m_apcLayerCfg[layer]->m_uiMaxCUWidth, m_apcLayerCfg[layer]->m_uiMaxCUHeight, m_apcLayerCfg[layer]->m_uiMaxTotalCUDepth, true, NULL );
    }
  }

  Bool bFirstFrame = true;
  while ( !bEos )
  {
    // Read enough frames
    Bool bFramesReadyToCode = false;
    while(!bFramesReadyToCode)
    {
      for(UInt layer=0; layer<m_numLayers; layer++)
      {
        //6
        // get buffers
        xGetBuffer(pcPicYuvRec, layer);

        // read input YUV file
        m_apcTVideoIOYuvInputFile[layer]->read( pcPicYuvOrg[layer], &acPicYuvTrueOrg[layer], ipCSC, m_apcLayerCfg[layer]->m_aiPad, m_apcLayerCfg[layer]->m_InputChromaFormatIDC, m_bClipInputVideoToRec709Range );

#if AUXILIARY_PICTURES
        if( m_apcLayerCfg[layer]->m_chromaFormatIDC == CHROMA_400 || (m_apcTEncTop[0]->getVPS()->getScalabilityMask(AUX_ID) && (m_apcLayerCfg[layer]->m_auxId == AUX_ALPHA || m_apcLayerCfg[layer]->m_auxId == AUX_DEPTH)) )
        {
          pcPicYuvOrg[layer]->convertToMonochrome(m_apcLayerCfg[layer]->m_internalBitDepth[CHANNEL_TYPE_CHROMA]);
        }
#endif

        if(layer == m_numLayers-1)
        {
          // increase number of received frames
          m_iFrameRcvd++;
          // check end of file
          bEos = (m_isField && (m_iFrameRcvd == (m_framesToBeEncoded >> 1) )) || ( !m_isField && (m_iFrameRcvd == m_framesToBeEncoded) );
        }

        if ( m_isField )
        {
          m_apcTEncTop[layer]->encodePrep( pcPicYuvOrg[layer], &acPicYuvTrueOrg[layer], m_isTopFieldFirst );
        }
        else
        {
          m_apcTEncTop[layer]->encodePrep( pcPicYuvOrg[layer], &acPicYuvTrueOrg[layer] );
        }
      }

      bFramesReadyToCode = !(!bFirstFrame && ( m_apcTEncTop[m_numLayers-1]->getNumPicRcvd() != m_iGOPSize && m_iGOPSize ) && !bEos );
    }
    Bool flush = 0;
    // if end of file (which is only detected on a read failure) flush the encoder of any queued pictures
    if (m_apcTVideoIOYuvInputFile[m_numLayers-1]->isEof())
    {
      flush = true;
      bEos = true;
      m_iFrameRcvd--;
      m_apcTEncTop[m_numLayers-1]->setFramesToBeEncoded(m_iFrameRcvd);
    }

#if RC_SHVC_HARMONIZATION
    for( UInt layer=0; layer<m_numLayers; layer++ )
    {
      if( m_apcTEncTop[layer]->getUseRateCtrl() )
      {
        m_apcTEncTop[layer]->getRateCtrl()->initRCGOP(m_apcTEncTop[layer]->getNumPicRcvd());
      }
    }
#endif

    if( m_adaptiveResolutionChange )
    {
      for(UInt layer = 0; layer < m_numLayers; layer++)
      {
        TComList<TComPicYuv*>::iterator iterPicYuvRec;
        for (iterPicYuvRec = m_acListPicYuvRec[layer].begin(); iterPicYuvRec != m_acListPicYuvRec[layer].end(); iterPicYuvRec++)
        {
          TComPicYuv* recPic = *(iterPicYuvRec);
          recPic->setReconstructed(false);
        }
      }
    }

    // loop through frames in one GOP
    for( UInt iPicIdInGOP=0; iPicIdInGOP < (bFirstFrame? 1:m_iGOPSize); iPicIdInGOP++ )
    {
      // layer by layer for each frame
      for( UInt layer=0; layer<m_numLayers; layer++ )
      {
        //7
#if LAYER_CTB
        memcpy( g_auiZscanToRaster, g_auiLayerZscanToRaster[layer], sizeof( g_auiZscanToRaster ) );
        memcpy( g_auiRasterToZscan, g_auiLayerRasterToZscan[layer], sizeof( g_auiRasterToZscan ) );
        memcpy( g_auiRasterToPelX,  g_auiLayerRasterToPelX[layer],  sizeof( g_auiRasterToPelX ) );
        memcpy( g_auiRasterToPelY,  g_auiLayerRasterToPelY[layer],  sizeof( g_auiRasterToPelY ) );
#endif
        // call encoding function for one frame
        if ( m_isField )
        {
          m_apcTEncTop[layer]->encode( flush ? 0 : pcPicYuvOrg[layer], snrCSC, m_acListPicYuvRec[layer], outputAccessUnits, iPicIdInGOP, m_isTopFieldFirst );
        }
        else
        {
          m_apcTEncTop[layer]->encode( flush ? 0 : pcPicYuvOrg[layer], snrCSC, m_acListPicYuvRec[layer], outputAccessUnits, iPicIdInGOP );
        }
      }
    }
#if R0247_SEI_ACTIVE
    if(bFirstFrame)
    {
      list<AccessUnit>::iterator first_au = outputAccessUnits.begin();
#if AVC_BASE
      if( m_nonHEVCBaseLayerFlag )
      {
        first_au++;
      }
#endif
      AccessUnit::iterator it_sps;
      for (it_sps = first_au->begin(); it_sps != first_au->end(); it_sps++)
      {
        if( (*it_sps)->m_nalUnitType == NAL_UNIT_SPS )
        {
          break;
        }
      }

      for (list<AccessUnit>::iterator it_au = ++outputAccessUnits.begin(); it_au != outputAccessUnits.end(); it_au++)
      {
        for (AccessUnit::iterator it_nalu = it_au->begin(); it_nalu != it_au->end(); it_nalu++)
        {
          if( (*it_nalu)->m_nalUnitType == NAL_UNIT_SPS )
          {
            it_sps = first_au->insert(++it_sps, *it_nalu);
            it_nalu = it_au->erase(it_nalu);
          }
        }
      }
    }
#endif

#if RC_SHVC_HARMONIZATION
    for( UInt layer = 0; layer < m_numLayers; layer++ )
    {
      if( m_apcTEncTop[layer]->getUseRateCtrl() )
      {
        m_apcTEncTop[layer]->getRateCtrl()->destroyRCGOP();
      }
    }
#endif

    iTotalNumEncoded = 0;
    for( UInt layer=0; layer<m_numLayers; layer++ )
    {
      //8
      // write bistream to file if necessary
      iNumEncoded = m_apcTEncTop[layer]->getNumPicRcvd();
      if ( iNumEncoded > 0 )
      {
        xWriteRecon(layer, iNumEncoded);
        iTotalNumEncoded += iNumEncoded;
      }
      m_apcTEncTop[layer]->setNumPicRcvd( 0 );

      // temporally skip frames
      if( m_temporalSubsampleRatio > 1 )
      {
        m_apcTVideoIOYuvInputFile[layer]->skipFrames(m_temporalSubsampleRatio-1, m_apcTEncTop[layer]->getSourceWidth() - m_apcTEncTop[layer]->getPad(0), m_apcTEncTop[layer]->getSourceHeight() - m_apcTEncTop[layer]->getPad(1), m_apcLayerCfg[layer]->m_InputChromaFormatIDC);
      }
    }

    // write bitstream out
    if( iTotalNumEncoded )
    {
      if( bEos )
      {
        OutputNALUnit nalu(NAL_UNIT_EOB);
        nalu.m_nuhLayerId = 0;

        AccessUnit& accessUnit = outputAccessUnits.back();
        nalu.m_temporalId = 0;
        accessUnit.push_back(new NALUnitEBSP(nalu));
      }

      xWriteStream(bitstreamFile, iTotalNumEncoded, outputAccessUnits);
      outputAccessUnits.clear();
    }

    // print out summary
    if (bEos)
    {
      printOutSummary(m_isTopFieldFirst, m_printMSEBasedSequencePSNR, m_printSequenceMSE);
    }

    bFirstFrame = false;
  }
  // delete original YUV buffer
  for(UInt layer=0; layer<m_numLayers; layer++)
  {
    pcPicYuvOrg[layer]->destroy();
    delete pcPicYuvOrg[layer];
    pcPicYuvOrg[layer] = NULL;

    // delete used buffers in encoder class
    m_apcTEncTop[layer]->deletePicBuffer();
    acPicYuvTrueOrg[layer].destroy();
  }

  // delete buffers & classes
  xDeleteBuffer();
  xDestroyLib();

  printRateSummary();

  return;
}

Void TAppEncTop::printOutSummary(Bool isField, const Bool printMSEBasedSNR, const Bool printSequenceMSE)
{
  UInt layer;
  const Int rateMultiplier = isField ? 2 : 1;

  // set frame rate
  for(layer = 0; layer < m_numLayers; layer++)
  {
    m_apcTEncTop[layer]->getAnalyzeAll()->setFrmRate( m_apcLayerCfg[layer]->m_iFrameRate * rateMultiplier );
    m_apcTEncTop[layer]->getAnalyzeI()->setFrmRate( m_apcLayerCfg[layer]->m_iFrameRate * rateMultiplier );
    m_apcTEncTop[layer]->getAnalyzeP()->setFrmRate( m_apcLayerCfg[layer]->m_iFrameRate * rateMultiplier );
    m_apcTEncTop[layer]->getAnalyzeB()->setFrmRate( m_apcLayerCfg[layer]->m_iFrameRate * rateMultiplier );
  }

  //-- all
  printf( "\n\nSUMMARY --------------------------------------------------------\n" );
  for(layer = 0; layer < m_numLayers; layer++)
  {
    const UInt layerId = m_apcTEncTop[layer]->getVPS()->getLayerIdInNuh(layer);
    const BitDepths bitDepths(m_apcLayerCfg[layer]->m_internalBitDepth[CHANNEL_TYPE_LUMA], m_apcLayerCfg[layer]->m_internalBitDepth[CHANNEL_TYPE_CHROMA]);    
    m_apcTEncTop[layer]->getAnalyzeAll()->printOut('a', m_apcLayerCfg[layer]->m_chromaFormatIDC, printMSEBasedSNR, printSequenceMSE, bitDepths, layerId);
  }

  printf( "\n\nI Slices--------------------------------------------------------\n" );
  for(layer = 0; layer < m_numLayers; layer++)
  {
    const UInt layerId = m_apcTEncTop[layer]->getVPS()->getLayerIdInNuh(layer);
    const BitDepths bitDepths(m_apcLayerCfg[layer]->m_internalBitDepth[CHANNEL_TYPE_LUMA], m_apcLayerCfg[layer]->m_internalBitDepth[CHANNEL_TYPE_CHROMA]);
    m_apcTEncTop[layer]->getAnalyzeI()->printOut('i', m_apcLayerCfg[layer]->m_chromaFormatIDC, printMSEBasedSNR, printSequenceMSE, bitDepths, layerId);
  }

  printf( "\n\nP Slices--------------------------------------------------------\n" );
  for(layer = 0; layer < m_numLayers; layer++)
  {
    const UInt layerId = m_apcTEncTop[layer]->getVPS()->getLayerIdInNuh(layer);
    const BitDepths bitDepths(m_apcLayerCfg[layer]->m_internalBitDepth[CHANNEL_TYPE_LUMA], m_apcLayerCfg[layer]->m_internalBitDepth[CHANNEL_TYPE_CHROMA]);
    m_apcTEncTop[layer]->getAnalyzeP()->printOut('p', m_apcLayerCfg[layer]->m_chromaFormatIDC, printMSEBasedSNR, printSequenceMSE, bitDepths, layerId);
  }

  printf( "\n\nB Slices--------------------------------------------------------\n" );
  for(layer = 0; layer < m_numLayers; layer++)
  {
    const UInt layerId = m_apcTEncTop[layer]->getVPS()->getLayerIdInNuh(layer);
    const BitDepths bitDepths(m_apcLayerCfg[layer]->m_internalBitDepth[CHANNEL_TYPE_LUMA], m_apcLayerCfg[layer]->m_internalBitDepth[CHANNEL_TYPE_CHROMA]);
    m_apcTEncTop[layer]->getAnalyzeB()->printOut('b', m_apcLayerCfg[layer]->m_chromaFormatIDC, printMSEBasedSNR, printSequenceMSE, bitDepths, layerId);
  }

  for( layer = 0; layer < m_numLayers; layer++ )
  {
    if (!m_apcTEncTop[layer]->getSummaryOutFilename().empty())
    {
      const BitDepths bitDepths(m_apcLayerCfg[layer]->m_internalBitDepth[CHANNEL_TYPE_LUMA], m_apcLayerCfg[layer]->m_internalBitDepth[CHANNEL_TYPE_CHROMA]);

      m_apcTEncTop[layer]->getAnalyzeAll()->printSummary(m_apcLayerCfg[layer]->m_chromaFormatIDC, printSequenceMSE, bitDepths, m_apcTEncTop[layer]->getSummaryOutFilename());
    }
  }

  for( layer = 0; layer < m_numLayers; layer++ )
  {
    if (!m_apcTEncTop[layer]->getSummaryPicFilenameBase().empty())
    {
      const BitDepths bitDepths(m_apcLayerCfg[layer]->m_internalBitDepth[CHANNEL_TYPE_LUMA], m_apcLayerCfg[layer]->m_internalBitDepth[CHANNEL_TYPE_CHROMA]);

      m_apcTEncTop[layer]->getAnalyzeI()->printSummary(m_apcLayerCfg[layer]->m_chromaFormatIDC, printSequenceMSE, bitDepths, m_apcTEncTop[layer]->getSummaryPicFilenameBase()+"I.txt");
      m_apcTEncTop[layer]->getAnalyzeP()->printSummary(m_apcLayerCfg[layer]->m_chromaFormatIDC, printSequenceMSE, bitDepths, m_apcTEncTop[layer]->getSummaryPicFilenameBase()+"P.txt");
      m_apcTEncTop[layer]->getAnalyzeB()->printSummary(m_apcLayerCfg[layer]->m_chromaFormatIDC, printSequenceMSE, bitDepths, m_apcTEncTop[layer]->getSummaryPicFilenameBase()+"B.txt");
    }
  }

  if(isField)
  {
    for(layer = 0; layer < m_numLayers; layer++)
    {
      const UInt layerId = m_apcTEncTop[layer]->getVPS()->getLayerIdInNuh(layer);
      const BitDepths bitDepths(m_apcLayerCfg[layer]->m_internalBitDepth[CHANNEL_TYPE_LUMA], m_apcLayerCfg[layer]->m_internalBitDepth[CHANNEL_TYPE_CHROMA]);
      TEncAnalyze *analyze = m_apcTEncTop[layer]->getAnalyzeAllin();

      //-- interlaced summary
      analyze->setFrmRate( m_apcLayerCfg[layer]->m_iFrameRate);
      analyze->setBits(m_apcTEncTop[layer]->getAnalyzeB()->getBits());
      // prior to the above statement, the interlace analyser does not contain the correct total number of bits.

      printf( "\n\nSUMMARY INTERLACED ---------------------------------------------\n" );
      analyze->printOut('a', m_apcLayerCfg[layer]->m_chromaFormatIDC, printMSEBasedSNR, printSequenceMSE, bitDepths, layerId);

      if (!m_apcTEncTop[layer]->getSummaryOutFilename().empty())
      {
        analyze->printSummary(m_apcLayerCfg[layer]->m_chromaFormatIDC, printSequenceMSE, bitDepths, m_apcTEncTop[layer]->getSummaryOutFilename());
      }
    }
  }   

  printf("\n");
  for( layer = 0; layer < m_numLayers; layer++ )
  {
    const UInt layerId = m_apcTEncTop[layer]->getVPS()->getLayerIdInNuh(layer);
    printf("RVM[L%d]: %.3lf\n", layerId, m_apcTEncTop[layer]->calculateRVM());
  }
  printf("\n");
}
#else
// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/**
 - create internal class
 - initialize internal variable
 - until the end of input YUV file, call encoding function in TEncTop class
 - delete allocated buffers
 - destroy internal class
 .
 */
Void TAppEncTop::encode()
{
  fstream bitstreamFile(m_bitstreamFileName.c_str(), fstream::binary | fstream::out);
  if (!bitstreamFile)
  {
    fprintf(stderr, "\nfailed to open bitstream file `%s' for writing\n", m_bitstreamFileName.c_str());
    exit(EXIT_FAILURE);
  }

  TComPicYuv*       pcPicYuvOrg = new TComPicYuv;
  TComPicYuv*       pcPicYuvRec = NULL;

  // initialize internal class & member variables
  xInitLibCfg();
  xCreateLib();
  xInitLib(m_isField);

  printChromaFormat();

  // main encoder loop
  Int   iNumEncoded = 0;
  Bool  bEos = false;

  const InputColourSpaceConversion ipCSC  =  m_inputColourSpaceConvert;
  const InputColourSpaceConversion snrCSC = (!m_snrInternalColourSpace) ? m_inputColourSpaceConvert : IPCOLOURSPACE_UNCHANGED;

  list<AccessUnit> outputAccessUnits; ///< list of access units to write out.  is populated by the encoding process

  TComPicYuv cPicYuvTrueOrg;

  // allocate original YUV buffer
  if( m_isField )
  {
    pcPicYuvOrg->create  ( m_iSourceWidth, m_iSourceHeightOrg, m_chromaFormatIDC, m_uiMaxCUWidth, m_uiMaxCUHeight, m_uiMaxTotalCUDepth, true );
    cPicYuvTrueOrg.create(m_iSourceWidth, m_iSourceHeightOrg, m_chromaFormatIDC, m_uiMaxCUWidth, m_uiMaxCUHeight, m_uiMaxTotalCUDepth, true);
  }
  else
  {
    pcPicYuvOrg->create  ( m_iSourceWidth, m_iSourceHeight, m_chromaFormatIDC, m_uiMaxCUWidth, m_uiMaxCUHeight, m_uiMaxTotalCUDepth, true );
    cPicYuvTrueOrg.create(m_iSourceWidth, m_iSourceHeight, m_chromaFormatIDC, m_uiMaxCUWidth, m_uiMaxCUHeight, m_uiMaxTotalCUDepth, true );
  }

  while ( !bEos )
  {
    // get buffers
    xGetBuffer(pcPicYuvRec);

    // read input YUV file
    m_cTVideoIOYuvInputFile.read( pcPicYuvOrg, &cPicYuvTrueOrg, ipCSC, m_aiPad, m_InputChromaFormatIDC, m_bClipInputVideoToRec709Range );

    // increase number of received frames
    m_iFrameRcvd++;

    bEos = (m_isField && (m_iFrameRcvd == (m_framesToBeEncoded >> 1) )) || ( !m_isField && (m_iFrameRcvd == m_framesToBeEncoded) );

    Bool flush = 0;
    // if end of file (which is only detected on a read failure) flush the encoder of any queued pictures
    if (m_cTVideoIOYuvInputFile.isEof())
    {
      flush = true;
      bEos = true;
      m_iFrameRcvd--;
      m_cTEncTop.setFramesToBeEncoded(m_iFrameRcvd);
    }

    // call encoding function for one frame
    if ( m_isField )
    {
      m_cTEncTop.encode( bEos, flush ? 0 : pcPicYuvOrg, flush ? 0 : &cPicYuvTrueOrg, snrCSC, m_cListPicYuvRec, outputAccessUnits, iNumEncoded, m_isTopFieldFirst );
    }
    else
    {
      m_cTEncTop.encode( bEos, flush ? 0 : pcPicYuvOrg, flush ? 0 : &cPicYuvTrueOrg, snrCSC, m_cListPicYuvRec, outputAccessUnits, iNumEncoded );
    }

    // write bistream to file if necessary
    if ( iNumEncoded > 0 )
    {
      xWriteOutput(bitstreamFile, iNumEncoded, outputAccessUnits);
      outputAccessUnits.clear();
    }
    // temporally skip frames
    if( m_temporalSubsampleRatio > 1 )
    {
      m_cTVideoIOYuvInputFile.skipFrames(m_temporalSubsampleRatio-1, m_iSourceWidth - m_aiPad[0], m_iSourceHeight - m_aiPad[1], m_InputChromaFormatIDC);
    }
  }

  m_cTEncTop.printSummary(m_isField);

  // delete original YUV buffer
  pcPicYuvOrg->destroy();
  delete pcPicYuvOrg;
  pcPicYuvOrg = NULL;

  // delete used buffers in encoder class
  m_cTEncTop.deletePicBuffer();
  cPicYuvTrueOrg.destroy();

  // delete buffers & classes
  xDeleteBuffer();
  xDestroyLib();

  printRateSummary();

  return;
}
#endif //SVC_EXTENSION

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

/**
 - application has picture buffer list with size of GOP
 - picture buffer list acts as ring buffer
 - end of the list has the latest picture
 .
 */
#if SVC_EXTENSION
Void TAppEncTop::xGetBuffer( TComPicYuv*& rpcPicYuvRec, UInt layer)
{
  assert( m_iGOPSize > 0 );

  // org. buffer
  if ( m_acListPicYuvRec[layer].size() >= (UInt)m_iGOPSize ) // buffer will be 1 element longer when using field coding, to maintain first field whilst processing second.
  {
    rpcPicYuvRec = m_acListPicYuvRec[layer].popFront();
  }
  else
  {
    rpcPicYuvRec = new TComPicYuv;

    rpcPicYuvRec->create( m_apcLayerCfg[layer]->m_iSourceWidth, m_apcLayerCfg[layer]->m_iSourceHeight, m_apcLayerCfg[layer]->m_chromaFormatIDC, m_apcLayerCfg[layer]->m_uiMaxCUWidth, m_apcLayerCfg[layer]->m_uiMaxCUHeight, m_apcLayerCfg[layer]->m_uiMaxTotalCUDepth, true, NULL );
  }
  m_acListPicYuvRec[layer].pushBack( rpcPicYuvRec );
}

Void TAppEncTop::xDeleteBuffer( )
{
  for(UInt layer=0; layer<m_numLayers; layer++)
  {
    TComList<TComPicYuv*>::iterator iterPicYuvRec  = m_acListPicYuvRec[layer].begin();

    Int iSize = Int( m_acListPicYuvRec[layer].size() );

    for ( Int i = 0; i < iSize; i++ )
    {
      TComPicYuv*  pcPicYuvRec  = *(iterPicYuvRec++);
      pcPicYuvRec->destroy();
      delete pcPicYuvRec; pcPicYuvRec = NULL;
    }
  }
}

Void TAppEncTop::xWriteRecon(UInt layer, Int iNumEncoded)
{
  ChromaFormat& chromaFormatIdc = m_apcLayerCfg[layer]->m_chromaFormatIDC;
  Int xScal = TComSPS::getWinUnitX( chromaFormatIdc );
  Int yScal = TComSPS::getWinUnitY( chromaFormatIdc );

  const InputColourSpaceConversion ipCSC = (!m_outputInternalColourSpace) ? m_inputColourSpaceConvert : IPCOLOURSPACE_UNCHANGED;

  if (m_isField)
  {
    //Reinterlace fields
    Int i;
    TComList<TComPicYuv*>::iterator iterPicYuvRec = m_acListPicYuvRec[layer].end();

    for ( i = 0; i < iNumEncoded; i++ )
    {
      --iterPicYuvRec;
    }

    for ( i = 0; i < iNumEncoded/2; i++ )
    {
      TComPicYuv*  pcPicYuvRecTop  = *(iterPicYuvRec++);
      TComPicYuv*  pcPicYuvRecBottom  = *(iterPicYuvRec++);

      if( !m_apcLayerCfg[layer]->getReconFileName().empty() && pcPicYuvRecTop->isReconstructed() && pcPicYuvRecBottom->isReconstructed() )
      {
        m_apcTVideoIOYuvReconFile[layer]->write( pcPicYuvRecTop, pcPicYuvRecBottom, ipCSC, m_apcLayerCfg[layer]->getConfWinLeft() * xScal, m_apcLayerCfg[layer]->getConfWinRight() * xScal, 
        m_apcLayerCfg[layer]->getConfWinTop() * yScal, m_apcLayerCfg[layer]->getConfWinBottom() * yScal, NUM_CHROMA_FORMAT, m_isTopFieldFirst );
      }
    }
  }
  else
  {
    Int i;

    TComList<TComPicYuv*>::iterator iterPicYuvRec = m_acListPicYuvRec[layer].end();

    for ( i = 0; i < iNumEncoded; i++ )
    {
      --iterPicYuvRec;
    }

    for ( i = 0; i < iNumEncoded; i++ )
    {
      TComPicYuv*  pcPicYuvRec  = *(iterPicYuvRec++);
      if( !m_apcLayerCfg[layer]->getReconFileName().empty() && pcPicYuvRec->isReconstructed() )
      {
        m_apcTVideoIOYuvReconFile[layer]->write( pcPicYuvRec, ipCSC, m_apcLayerCfg[layer]->getConfWinLeft() * xScal, m_apcLayerCfg[layer]->getConfWinRight() * xScal,
          m_apcLayerCfg[layer]->getConfWinTop() * yScal, m_apcLayerCfg[layer]->getConfWinBottom() * yScal,
          NUM_CHROMA_FORMAT, m_bClipOutputVideoToRec709Range  );
      }
    }
  }
}

Void TAppEncTop::xWriteStream(std::ostream& bitstreamFile, Int iNumEncoded, const std::list<AccessUnit>& accessUnits)
{
  if (m_isField)
  {
    //Reinterlace fields
    Int i;
    list<AccessUnit>::const_iterator iterBitstream = accessUnits.begin();

    for( i = 0; i < iNumEncoded/2 && iterBitstream != accessUnits.end(); i++ )
    {
      const AccessUnit& auTop = *(iterBitstream++);
      const vector<UInt>& statsTop = writeAnnexB(bitstreamFile, auTop);
      rateStatsAccum(auTop, statsTop);

      const AccessUnit& auBottom = *(iterBitstream++);
      const vector<UInt>& statsBottom = writeAnnexB(bitstreamFile, auBottom);
      rateStatsAccum(auBottom, statsBottom);
    }
  }
  else
  {
    Int i;

    list<AccessUnit>::const_iterator iterBitstream = accessUnits.begin();

    for( i = 0; i < iNumEncoded && iterBitstream != accessUnits.end(); i++ )
    {
      const AccessUnit& au = *(iterBitstream++);
      const vector<UInt>& stats = writeAnnexB(bitstreamFile, au);
      rateStatsAccum(au, stats);
    }
  }
}

#else // SVC_EXTENSION
Void TAppEncTop::xGetBuffer( TComPicYuv*& rpcPicYuvRec)
{
  assert( m_iGOPSize > 0 );

  // org. buffer
  if ( m_cListPicYuvRec.size() >= (UInt)m_iGOPSize ) // buffer will be 1 element longer when using field coding, to maintain first field whilst processing second.
  {
    rpcPicYuvRec = m_cListPicYuvRec.popFront();

  }
  else
  {
    rpcPicYuvRec = new TComPicYuv;

    rpcPicYuvRec->create( m_iSourceWidth, m_iSourceHeight, m_chromaFormatIDC, m_uiMaxCUWidth, m_uiMaxCUHeight, m_uiMaxTotalCUDepth, true );

  }
  m_cListPicYuvRec.pushBack( rpcPicYuvRec );
}

Void TAppEncTop::xDeleteBuffer( )
{
  TComList<TComPicYuv*>::iterator iterPicYuvRec  = m_cListPicYuvRec.begin();

  Int iSize = Int( m_cListPicYuvRec.size() );

  for ( Int i = 0; i < iSize; i++ )
  {
    TComPicYuv*  pcPicYuvRec  = *(iterPicYuvRec++);
    pcPicYuvRec->destroy();
    delete pcPicYuvRec; pcPicYuvRec = NULL;
  }

}

/** 
  Write access units to output file.
  \param bitstreamFile  target bitstream file
  \param iNumEncoded    number of encoded frames
  \param accessUnits    list of access units to be written
 */
Void TAppEncTop::xWriteOutput(std::ostream& bitstreamFile, Int iNumEncoded, const std::list<AccessUnit>& accessUnits)
{
  const InputColourSpaceConversion ipCSC = (!m_outputInternalColourSpace) ? m_inputColourSpaceConvert : IPCOLOURSPACE_UNCHANGED;

  if (m_isField)
  {
    //Reinterlace fields
    Int i;
    TComList<TComPicYuv*>::iterator iterPicYuvRec = m_cListPicYuvRec.end();
    list<AccessUnit>::const_iterator iterBitstream = accessUnits.begin();

    for ( i = 0; i < iNumEncoded; i++ )
    {
      --iterPicYuvRec;
    }

    for ( i = 0; i < iNumEncoded/2; i++ )
    {
      TComPicYuv*  pcPicYuvRecTop  = *(iterPicYuvRec++);
      TComPicYuv*  pcPicYuvRecBottom  = *(iterPicYuvRec++);

      if (!m_reconFileName.empty())
      {
        m_cTVideoIOYuvReconFile.write( pcPicYuvRecTop, pcPicYuvRecBottom, ipCSC, m_confWinLeft, m_confWinRight, m_confWinTop, m_confWinBottom, NUM_CHROMA_FORMAT, m_isTopFieldFirst );
      }

      const AccessUnit& auTop = *(iterBitstream++);
      const vector<UInt>& statsTop = writeAnnexB(bitstreamFile, auTop);
      rateStatsAccum(auTop, statsTop);

      const AccessUnit& auBottom = *(iterBitstream++);
      const vector<UInt>& statsBottom = writeAnnexB(bitstreamFile, auBottom);
      rateStatsAccum(auBottom, statsBottom);
    }
  }
  else
  {
    Int i;

    TComList<TComPicYuv*>::iterator iterPicYuvRec = m_cListPicYuvRec.end();
    list<AccessUnit>::const_iterator iterBitstream = accessUnits.begin();

    for ( i = 0; i < iNumEncoded; i++ )
    {
      --iterPicYuvRec;
    }

    for ( i = 0; i < iNumEncoded; i++ )
    {
      TComPicYuv*  pcPicYuvRec  = *(iterPicYuvRec++);
      if (!m_reconFileName.empty())
      {
        m_cTVideoIOYuvReconFile.write( pcPicYuvRec, ipCSC, m_confWinLeft, m_confWinRight, m_confWinTop, m_confWinBottom,
            NUM_CHROMA_FORMAT, m_bClipOutputVideoToRec709Range  );
      }

      const AccessUnit& au = *(iterBitstream++);
      const vector<UInt>& stats = writeAnnexB(bitstreamFile, au);
      rateStatsAccum(au, stats);
    }
  }
}
#endif

/**
 *
 */
Void TAppEncTop::rateStatsAccum(const AccessUnit& au, const std::vector<UInt>& annexBsizes)
{
  AccessUnit::const_iterator it_au = au.begin();
  vector<UInt>::const_iterator it_stats = annexBsizes.begin();

  for (; it_au != au.end(); it_au++, it_stats++)
  {
    switch ((*it_au)->m_nalUnitType)
    {
    case NAL_UNIT_CODED_SLICE_TRAIL_R:
    case NAL_UNIT_CODED_SLICE_TRAIL_N:
    case NAL_UNIT_CODED_SLICE_TSA_R:
    case NAL_UNIT_CODED_SLICE_TSA_N:
    case NAL_UNIT_CODED_SLICE_STSA_R:
    case NAL_UNIT_CODED_SLICE_STSA_N:
    case NAL_UNIT_CODED_SLICE_BLA_W_LP:
    case NAL_UNIT_CODED_SLICE_BLA_W_RADL:
    case NAL_UNIT_CODED_SLICE_BLA_N_LP:
    case NAL_UNIT_CODED_SLICE_IDR_W_RADL:
    case NAL_UNIT_CODED_SLICE_IDR_N_LP:
    case NAL_UNIT_CODED_SLICE_CRA:
    case NAL_UNIT_CODED_SLICE_RADL_N:
    case NAL_UNIT_CODED_SLICE_RADL_R:
    case NAL_UNIT_CODED_SLICE_RASL_N:
    case NAL_UNIT_CODED_SLICE_RASL_R:
    case NAL_UNIT_VPS:
    case NAL_UNIT_SPS:
    case NAL_UNIT_PPS:
      m_essentialBytes += *it_stats;
      break;
    default:
      break;
    }

    m_totalBytes += *it_stats;
  }
}

Void TAppEncTop::printRateSummary()
{
#if SVC_EXTENSION
  Double time = (Double) m_iFrameRcvd / m_apcLayerCfg[m_numLayers-1]->m_iFrameRate * m_temporalSubsampleRatio;
#else
  Double time = (Double) m_iFrameRcvd / m_iFrameRate * m_temporalSubsampleRatio;
#endif
  printf("Bytes written to file: %u (%.3f kbps)\n", m_totalBytes, 0.008 * m_totalBytes / time);
  if (m_summaryVerboseness > 0)
  {
    printf("Bytes for SPS/PPS/Slice (Incl. Annex B): %u (%.3f kbps)\n", m_essentialBytes, 0.008 * m_essentialBytes / time);
  }
}

#if !SVC_EXTENSION
Void TAppEncTop::printChromaFormat()
{
  std::cout << std::setw(43) << "Input ChromaFormatIDC = ";
  switch (m_InputChromaFormatIDC)
  {
  case CHROMA_400:  std::cout << "  4:0:0"; break;
  case CHROMA_420:  std::cout << "  4:2:0"; break;
  case CHROMA_422:  std::cout << "  4:2:2"; break;
  case CHROMA_444:  std::cout << "  4:4:4"; break;
  default:
    std::cerr << "Invalid";
    exit(1);
  }
  std::cout << std::endl;

  std::cout << std::setw(43) << "Output (internal) ChromaFormatIDC = ";
  switch (m_cTEncTop.getChromaFormatIdc())
  {
  case CHROMA_400:  std::cout << "  4:0:0"; break;
  case CHROMA_420:  std::cout << "  4:2:0"; break;
  case CHROMA_422:  std::cout << "  4:2:2"; break;
  case CHROMA_444:  std::cout << "  4:4:4"; break;
  default:
    std::cerr << "Invalid";
    exit(1);
  }
  std::cout << "\n" << std::endl;
}
#endif

//! \}
