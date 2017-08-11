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

/** \file     TDecTop.cpp
    \brief    decoder class
*/

#include "NALread.h"
#include "TDecTop.h"

#if SVC_EXTENSION
UInt  TDecTop::m_prevPOC                       = MAX_UINT;
UInt  TDecTop::m_uiPrevLayerId                 = MAX_UINT;
Bool  TDecTop::m_bFirstSliceInSequence         = true;
Bool  TDecTop::m_checkPocRestrictionsForCurrAu = false;
Int   TDecTop::m_pocResetIdcOrCurrAu           = -1;
Bool  TDecTop::m_baseLayerIdrFlag              = false;
Bool  TDecTop::m_baseLayerPicPresentFlag       = false;
Bool  TDecTop::m_baseLayerIrapFlag             = false;
Bool  TDecTop::m_nonBaseIdrPresentFlag         = false;
Int   TDecTop::m_nonBaseIdrType                = -1;
Bool  TDecTop::m_picNonIdrWithRadlPresentFlag  = false;
Bool  TDecTop::m_picNonIdrNoLpPresentFlag      = false;
Int   TDecTop::m_crossLayerPocResetPeriodId    = -1;
Int   TDecTop::m_crossLayerPocResetIdc         = -1;
std::vector<UInt> TDecTop::m_targetDecLayerIdList; // list of layers to be decoded according to the OLS index
#endif

//! \ingroup TLibDecoder
//! \{

TDecTop::TDecTop()
  : m_iMaxRefPicNum(0)
  , m_associatedIRAPType(NAL_UNIT_INVALID)
  , m_pocCRA(0)
  , m_pocRandomAccess(MAX_INT)
  , m_cListPic()
  , m_parameterSetManager()
  , m_apcSlicePilot(NULL)
  , m_SEIs()
  , m_cPrediction()
  , m_cTrQuant()
  , m_cGopDecoder()
  , m_cSliceDecoder()
  , m_cCuDecoder()
  , m_cEntropyDecoder()
  , m_cCavlcDecoder()
  , m_cSbacDecoder()
  , m_cBinCABAC()
  , m_seiReader()
  , m_cLoopFilter()
  , m_cSAO()
  , m_pcPic(NULL)
#if !SVC_EXTENSION
  , m_prevPOC(MAX_INT)
#endif
  , m_prevTid0POC(0)
  , m_bFirstSliceInPicture(true)
#if !SVC_EXTENSION
  , m_bFirstSliceInSequence(true)
#endif
  , m_prevSliceSkipped(false)
  , m_skippedPOC(0)
  , m_bFirstSliceInBitstream(true)
  , m_lastPOCNoOutputPriorPics(-1)
  , m_isNoOutputPriorPics(false)
  , m_craNoRaslOutputFlag(false)
#if O0043_BEST_EFFORT_DECODING
  , m_forceDecodeBitDepth(8)
#endif
  , m_pDecodedSEIOutputStream(NULL)
  , m_warningMessageSkipPicture(false)
  , m_prefixSEINALUs()
{
#if ENC_DEC_TRACE
  if (g_hTrace == NULL)
  {
    g_hTrace = fopen( "TraceDec.txt", "wb" );
  }
  g_bJustDoIt = g_bEncDecTraceDisable;
  g_nSymbolCounter = 0;
#endif

#if SVC_EXTENSION 
  m_layerId = 0;
  m_smallestLayerId = 0;
#if AVC_BASE
  m_pBLReconFile = NULL;
#endif
  memset(m_cIlpPic, 0, sizeof(m_cIlpPic));
  m_isLastNALWasEos = false;
  m_lastPicHasEos = false;
#if NO_CLRAS_OUTPUT_FLAG
  m_noClrasOutputFlag          = false;
  m_layerInitializedFlag       = false;
  m_firstPicInLayerDecodedFlag = false;  
#endif
  m_parseIdc = -1;
  m_lastPocPeriodId = -1;
  m_prevPicOrderCnt = 0;
#if CGS_3D_ASYMLUT
  m_pColorMappedPic = NULL;
#endif

  resetPocRestrictionCheckParameters();
  m_pocResettingFlag        = false;
  m_pocDecrementedInDPBFlag = false;
#if CONFORMANCE_BITSTREAM_MODE
  m_confModeFlag = false;
#endif
  m_isOutputLayerFlag = false;
#endif //SVC_EXTENSION 
}

TDecTop::~TDecTop()
{
#if ENC_DEC_TRACE
  if (g_hTrace != stdout)
  {
    fclose( g_hTrace );
  }
#endif
  while (!m_prefixSEINALUs.empty())
  {
    delete m_prefixSEINALUs.front();
    m_prefixSEINALUs.pop_front();
  }
#if CGS_3D_ASYMLUT
  if(m_pColorMappedPic)
  {
    m_pColorMappedPic->destroy();
    delete m_pColorMappedPic;
    m_pColorMappedPic = NULL;
  }
#endif
}

Void TDecTop::create()
{
  m_cGopDecoder.create();
  m_apcSlicePilot = new TComSlice;
  m_uiSliceIdx = 0;
}

Void TDecTop::destroy()
{
  m_cGopDecoder.destroy();

  delete m_apcSlicePilot;
  m_apcSlicePilot = NULL;

  m_cSliceDecoder.destroy();
#if SVC_EXTENSION
  for(Int i=0; i<MAX_NUM_REF; i++)
  {
    if(m_cIlpPic[i])
    {
      m_cIlpPic[i]->destroy();
      delete m_cIlpPic[i];
      m_cIlpPic[i] = NULL;
    }
  }
#endif
}

Void TDecTop::init()
{
#if !SVC_EXTENSION
  // initialize ROM
  initROM();
#endif
#if SVC_EXTENSION
  m_cGopDecoder.init( m_ppcTDecTop, &m_cEntropyDecoder, &m_cSbacDecoder, &m_cBinCABAC, &m_cCavlcDecoder, &m_cSliceDecoder, &m_cLoopFilter, &m_cSAO);
#else
  m_cGopDecoder.init( &m_cEntropyDecoder, &m_cSbacDecoder, &m_cBinCABAC, &m_cCavlcDecoder, &m_cSliceDecoder, &m_cLoopFilter, &m_cSAO);
#endif
  m_cSliceDecoder.init( &m_cEntropyDecoder, &m_cCuDecoder );
  m_cEntropyDecoder.init(&m_cPrediction);
}

Void TDecTop::deletePicBuffer ( )
{
  TComList<TComPic*>::iterator  iterPic   = m_cListPic.begin();
  Int iSize = Int( m_cListPic.size() );

  for (Int i = 0; i < iSize; i++ )
  {
    TComPic* pcPic = *(iterPic++);
#if SVC_EXTENSION
    if( pcPic )
    {
      pcPic->destroy();

      delete pcPic;
      pcPic = NULL;
    }
#else
    pcPic->destroy();

    delete pcPic;
    pcPic = NULL;
#endif
  }

  m_cSAO.destroy();

  m_cLoopFilter.        destroy();
  
#if !SVC_EXTENSION
  // destroy ROM
  destroyROM();
#endif
}

#if SVC_EXTENSION
Void TDecTop::xGetNewPicBuffer( const TComVPS &vps, const TComSPS &sps, const TComPPS &pps, TComPic*& rpcPic, const UInt temporalLayer )
#else
Void TDecTop::xGetNewPicBuffer ( const TComSPS &sps, const TComPPS &pps, TComPic*& rpcPic, const UInt temporalLayer )
#endif
{
#if SVC_EXTENSION
  if( m_commonDecoderParams->getTargetOutputLayerSetIdx() == 0 )
  {
    assert( m_layerId == 0 );
    m_iMaxRefPicNum = sps.getMaxDecPicBuffering(temporalLayer);     // m_uiMaxDecPicBuffering has the space for the picture currently being decoded
  }
  else
  {
    m_iMaxRefPicNum = vps.getMaxVpsDecPicBufferingMinus1( m_commonDecoderParams->getTargetOutputLayerSetIdx(), vps.getLayerIdcForOls( vps.getOutputLayerSetIdx( m_commonDecoderParams->getTargetOutputLayerSetIdx()), m_layerId ), temporalLayer ) + 1; // m_uiMaxDecPicBuffering has the space for the picture currently being decoded
  }

  m_iMaxRefPicNum += 1; // SHM: it should be updated if more than 1 resampling picture is used
#else
  m_iMaxRefPicNum = sps.getMaxDecPicBuffering(temporalLayer);     // m_uiMaxDecPicBuffering has the space for the picture currently being decoded
#endif
  
  if (m_cListPic.size() < (UInt)m_iMaxRefPicNum)
  {
    rpcPic = new TComPic();

#if SVC_EXTENSION
    if(m_layerId > 0)
    {
      xSetRequireResamplingFlag( vps, sps, pps, rpcPic );
    }

#if REDUCED_ENCODER_MEMORY
    rpcPic->create ( sps, pps, false, true, m_layerId);
#else
    rpcPic->create( sps, pps, true, m_layerId);
#endif
#else //SVC_EXTENSION
#if REDUCED_ENCODER_MEMORY
    rpcPic->create ( sps, pps, false, true);
#else
    rpcPic->create ( sps, pps, true);
#endif
#endif //SVC_EXTENSION
    
    m_cListPic.pushBack( rpcPic );

    return;
  }

  Bool bBufferIsAvailable = false;
  TComList<TComPic*>::iterator  iterPic   = m_cListPic.begin();
  while (iterPic != m_cListPic.end())
  {
    rpcPic = *(iterPic++);
    if ( rpcPic->getReconMark() == false && rpcPic->getOutputMark() == false)
    {
      rpcPic->setOutputMark(false);
      bBufferIsAvailable = true;
      break;
    }

    if ( rpcPic->getSlice( 0 )->isReferenced() == false  && rpcPic->getOutputMark() == false)
    {
#if !SVC_EXTENSION
      rpcPic->setOutputMark(false);
#endif
      rpcPic->setReconMark( false );
      rpcPic->getPicYuvRec()->setBorderExtension( false );
      bBufferIsAvailable = true;
      break;
    }
  }

  if ( !bBufferIsAvailable )
  {
    //There is no room for this picture, either because of faulty encoder or dropped NAL. Extend the buffer.
    m_iMaxRefPicNum++;
    rpcPic = new TComPic();
    m_cListPic.pushBack( rpcPic );
  }
  rpcPic->destroy();

#if SVC_EXTENSION
  if( m_layerId > 0 )
  {
    xSetRequireResamplingFlag( vps, sps, pps, rpcPic );
  }
#if REDUCED_ENCODER_MEMORY
  rpcPic->create ( sps, pps, false, true, m_layerId);
#else
  rpcPic->create( sps, pps, true, m_layerId);
#endif
#else  //SVC_EXTENSION
#if REDUCED_ENCODER_MEMORY
  rpcPic->create ( sps, pps, false, true);
#else
  rpcPic->create ( sps, pps, true);
#endif
#endif //SVC_EXTENSION
}

Void TDecTop::executeLoopFilters(Int& poc, TComList<TComPic*>*& rpcListPic)
{
  if (!m_pcPic)
  {
    /* nothing to deblock */
    return;
  }

  TComPic*   pcPic         = m_pcPic;
  lmAllDecInfo* lmdec = lmAllDecInfo::getInstance();
  lmdec->output(pcPic);
  // Execute Deblock + Cleanup

  m_cGopDecoder.filterPicture(pcPic);

#if SVC_EXTENSION
  if( m_ppcTDecTop[pcPic->getLayerId()]->m_isOutputLayerFlag == false )
  {
    pcPic->setOutputMark( false );
  }
#endif

  TComSlice::sortPicList( m_cListPic ); // sorting for application output
  poc                 = pcPic->getSlice(m_uiSliceIdx-1)->getPOC();
  rpcListPic          = &m_cListPic;
  m_cCuDecoder.destroy();
  m_bFirstSliceInPicture  = true;

  return;
}

Void TDecTop::checkNoOutputPriorPics (TComList<TComPic*>* pcListPic)
{
  if (!pcListPic || !m_isNoOutputPriorPics)
  {
    return;
  }

  TComList<TComPic*>::iterator  iterPic   = pcListPic->begin();

  while (iterPic != pcListPic->end())
  {
    TComPic* pcPicTmp = *(iterPic++);
    if (m_lastPOCNoOutputPriorPics != pcPicTmp->getPOC())
    {
      pcPicTmp->setOutputMark(false);
    }
  }
}

Void TDecTop::xCreateLostPicture(Int iLostPoc)
{
  printf("\ninserting lost poc : %d\n",iLostPoc);
  TComPic *cFillPic;
#if SVC_EXTENSION
  xGetNewPicBuffer(*(m_parameterSetManager.getFirstVPS()), *(m_parameterSetManager.getFirstSPS()), *(m_parameterSetManager.getFirstPPS()), cFillPic, 0);
  cFillPic->getSlice(0)->initSlice( m_layerId );
#else
  xGetNewPicBuffer(*(m_parameterSetManager.getFirstSPS()), *(m_parameterSetManager.getFirstPPS()), cFillPic, 0);
  cFillPic->getSlice(0)->initSlice();
#endif
  
  TComList<TComPic*>::iterator iterPic = m_cListPic.begin();
  Int closestPoc = 1000000;
  while ( iterPic != m_cListPic.end())
  {
    TComPic * rpcPic = *(iterPic++);
    if(abs(rpcPic->getPicSym()->getSlice(0)->getPOC() -iLostPoc)<closestPoc&&abs(rpcPic->getPicSym()->getSlice(0)->getPOC() -iLostPoc)!=0&&rpcPic->getPicSym()->getSlice(0)->getPOC()!=m_apcSlicePilot->getPOC())
    {
      closestPoc=abs(rpcPic->getPicSym()->getSlice(0)->getPOC() -iLostPoc);
    }
  }
  iterPic = m_cListPic.begin();
  while ( iterPic != m_cListPic.end())
  {
    TComPic *rpcPic = *(iterPic++);
    if(abs(rpcPic->getPicSym()->getSlice(0)->getPOC() -iLostPoc)==closestPoc&&rpcPic->getPicSym()->getSlice(0)->getPOC()!=m_apcSlicePilot->getPOC())
    {
      printf("copying picture %d to %d (%d)\n",rpcPic->getPicSym()->getSlice(0)->getPOC() ,iLostPoc,m_apcSlicePilot->getPOC());
      rpcPic->getPicYuvRec()->copyToPic(cFillPic->getPicYuvRec());
      break;
    }
  }
  cFillPic->setCurrSliceIdx(0);
  for(Int ctuRsAddr=0; ctuRsAddr<cFillPic->getNumberOfCtusInFrame(); ctuRsAddr++)
  {
    cFillPic->getCtu(ctuRsAddr)->initCtu(cFillPic, ctuRsAddr);
  }
  cFillPic->getSlice(0)->setReferenced(true);
  cFillPic->getSlice(0)->setPOC(iLostPoc);
  xUpdatePreviousTid0POC(cFillPic->getSlice(0));
  cFillPic->setReconMark(true);
  cFillPic->setOutputMark(true);
  if(m_pocRandomAccess == MAX_INT)
  {
    m_pocRandomAccess = iLostPoc;
  }
}


Void TDecTop::xActivateParameterSets()
{
  if (m_bFirstSliceInPicture)
  {
    const TComPPS *pps = m_parameterSetManager.getPPS(m_apcSlicePilot->getPPSId()); // this is a temporary PPS object. Do not store this value
    assert (pps != 0);

#if SVC_EXTENSION
    TComSPS *updateSPS = m_parameterSetManager.getSPS(pps->getSPSId());             // this is a temporary SPS object. Do not store this value
#endif
    const TComSPS *sps = m_parameterSetManager.getSPS(pps->getSPSId());             // this is a temporary SPS object. Do not store this value
    assert (sps != 0);

    m_parameterSetManager.clearSPSChangedFlag(sps->getSPSId());
    m_parameterSetManager.clearPPSChangedFlag(pps->getPPSId());

    if (false == m_parameterSetManager.activatePPS(m_apcSlicePilot->getPPSId(),m_apcSlicePilot->isIRAP()))
    {
      printf ("Parameter set activation failed!");
      assert (0);
    }

    xParsePrefixSEImessages();

#if RExt__HIGH_BIT_DEPTH_SUPPORT==0
    if (sps->getSpsRangeExtension().getExtendedPrecisionProcessingFlag() || sps->getBitDepth(CHANNEL_TYPE_LUMA)>12 || sps->getBitDepth(CHANNEL_TYPE_CHROMA)>12 )
    {
      printf("High bit depth support must be enabled at compile-time in order to decode this bitstream\n");
      assert (0);
      exit(1);
    }
#endif

#if SVC_EXTENSION
    // scaling list settings and checks
    TComVPS *vps = m_parameterSetManager.getVPS(sps->getVPSId());
    assert (vps != 0);

    m_parameterSetManager.clearVPSChangedFlag(sps->getVPSId());    

    if( sps->getInferScalingListFlag() )
    {
      UInt refLayerId = sps->getScalingListRefLayerId();
      const TComSPS *refSps = m_ppcTDecTop[refLayerId]->getParameterSetManager()->getActiveSPS(); assert( refSps != NULL );

      // When avc_base_layer_flag is equal to 1, it is a requirement of bitstream conformance that the value of sps_scaling_list_ref_layer_id shall be greater than 0
      if( vps->getNonHEVCBaseLayerFlag() )
      {
        assert( refLayerId > 0 );
      }

      // reference layer active SPS (specified by sps_scaling_list_ref_layer_id or pps_scaling_list_ref_layer_id) shall have scaling_list_enabled_flag equal to 1.
      assert( refSps->getScalingListFlag() == true );

      // It is a requirement of bitstream conformance that, when an SPS with nuh_layer_id equal to nuhLayerIdA is active for a layer with nuh_layer_id equal to nuhLayerIdB and
      // sps_infer_scaling_list_flag in the SPS is equal to 1, sps_infer_scaling_list_flag shall be equal to 0 for the SPS that is active for the layer with nuh_layer_id equal to sps_scaling_list_ref_layer_id
      assert( refSps->getInferScalingListFlag() == false );

      // It is a requirement of bitstream conformance that, when an SPS with nuh_layer_id equal to nuhLayerIdA is active for a layer with nuh_layer_id equal to nuhLayerIdB,
      // the layer with nuh_layer_id equal to sps_scaling_list_ref_layer_id shall be a direct or indirect reference layer of the layer with nuh_layer_id equal to nuhLayerIdB
      assert( vps->getRecursiveRefLayerFlag( sps->getLayerId(), refLayerId ) == true );
    }

    if( pps->getInferScalingListFlag() )
    {
      UInt refLayerId = pps->getScalingListRefLayerId();
      const TComPPS *refPps = m_ppcTDecTop[refLayerId]->getParameterSetManager()->getActivePPS(); assert( refPps != NULL );

      // When avc_base_layer_flag is equal to 1, it is a requirement of bitstream conformance that the value of sps_scaling_list_ref_layer_id shall be greater than 0
      if( vps->getNonHEVCBaseLayerFlag() )
      {
        assert( refLayerId > 0 );
      }

      // It is a requirement of bitstream conformance that, when an PPS with nuh_layer_id equal to nuhLayerIdA is active for a layer with nuh_layer_id equal to nuhLayerIdB and
      // pps_infer_scaling_list_flag in the PPS is equal to 1, pps_infer_scaling_list_flag shall be equal to 0 for the PPS that is active for the layer with nuh_layer_id equal to pps_scaling_list_ref_layer_id
      assert( refPps->getInferScalingListFlag() == false );

      // It is a requirement of bitstream conformance that, when an PPS with nuh_layer_id equal to nuhLayerIdA is active for a layer with nuh_layer_id equal to nuhLayerIdB,
      // the layer with nuh_layer_id equal to pps_scaling_list_ref_layer_id shall be a direct or indirect reference layer of the layer with nuh_layer_id equal to nuhLayerIdB
      assert( vps->getRecursiveRefLayerFlag( pps->getLayerId(), refLayerId ) == true );
    }

#if AVC_BASE
    if( vps->getNonHEVCBaseLayerFlag() )
    {
      TComPic* pBLPic = (*m_ppcTDecTop[0]->getListPic()->begin());
      if( m_layerId > 0 && pBLPic->getPicYuvRec() == NULL )
      {
        UInt refLayerId = 0;

        pBLPic->getPicSym()->inferSpsForNonHEVCBL(vps, sps->getMaxCUWidth(), sps->getMaxCUHeight());

#if REDUCED_ENCODER_MEMORY
        pBLPic->create( pBLPic->getPicSym()->getSPS(), *pps, false, true, refLayerId);
#else
        pBLPic->create( pBLPic->getPicSym()->getSPS(), *pps, true, refLayerId);
#endif

        // it is needed where the VPS is accessed through the slice
        pBLPic->getSlice(0)->setVPS( vps );
        pBLPic->getSlice(0)->setSPS( &pBLPic->getPicSym()->getSPS() );
      }
    }
#endif

    if( m_layerId > 0 )
    {
      // When not present sps_max_sub_layers_minus1 is inferred to be equal to vps_max_sub_layers_minus1.
      updateSPS->setMaxTLayers( vps->getMaxTLayers() );

      // When not present sps_temporal_id_nesting_flag is inferred to be equal to vps_temporal_id_nesting_flag
      updateSPS->setTemporalIdNestingFlag( (sps->getMaxTLayers() > 1) ? vps->getTemporalNestingFlag() : true );

      // When sps_max_dec_pic_buffering_minus1[ i ] is not present for i in the range of 0 to sps_max_sub_layers_minus1, inclusive, due to nuh_layer_id being greater than 0, 
      // it is inferred to be equal to max_vps_dec_pic_buffering_minus1[ TargetOptLayerSetIdx ][ currLayerId ][ i ] of the active VPS, where currLayerId is the nuh_layer_id of the layer that refers to the SPS.
      for(UInt i=0; i < sps->getMaxTLayers(); i++)
      {
        // to avoid compiler warning "array subscript is above array bounds"
        assert( i < MAX_TLAYER );

        // When sps_max_dec_pic_buffering_minus1[ i ] is not present for i in the range of 0 to sps_max_sub_layers_minus1, inclusive, 
        // due to MultiLayerExtSpsFlag being equal to 1, for a layer that refers to the SPS and has nuh_layer_id equal to currLayerId, 
        // the value of sps_max_dec_pic_buffering_minus1[ i ] is inferred to be equal to max_vps_dec_pic_buffering_minus1[ TargetOlsIdx ][ layerIdx ][ i ] of the active VPS, 
        // where layerIdx is equal to the value such that LayerSetLayerIdList[ TargetDecLayerSetIdx ][ layerIdx ] is equal to currLayerId.
        if( sps->getMultiLayerExtSpsFlag() )
        {
          updateSPS->setMaxDecPicBuffering( vps->getMaxVpsDecPicBufferingMinus1( m_commonDecoderParams->getTargetOutputLayerSetIdx(), vps->getLayerIdcForOls( vps->getOutputLayerSetIdx( m_commonDecoderParams->getTargetOutputLayerSetIdx() ), m_layerId ), i) + 1, i);
        }

        // The value of sps_max_dec_pic_buffering_minus1[ i ] shall be less than or equal to vps_max_dec_pic_buffering_minus1[ i ] for each value of i.
        assert( sps->getMaxDecPicBuffering(i) <= vps->getMaxDecPicBuffering(i) );      
      }

      UInt layerIdx = vps->getLayerIdxInVps( m_layerId );

      if( vps->getBaseLayerPSCompatibilityFlag(layerIdx) )
      {
        const RepFormat* repFormat = vps->getVpsRepFormat(vps->getVpsRepFormatIdx(layerIdx));

        assert( pps->getLayerId() == 0 );
        assert( sps->getLayerId() == 0 );
        assert( repFormat->getChromaFormatVpsIdc() == sps->getChromaFormatIdc() );
        assert( repFormat->getSeparateColourPlaneVpsFlag() == 0 );
        assert( repFormat->getPicHeightVpsInLumaSamples() == sps->getPicHeightInLumaSamples() );
        assert( repFormat->getPicWidthVpsInLumaSamples()  == sps->getPicWidthInLumaSamples() );
        assert( repFormat->getBitDepthVpsLuma()   == sps->getBitDepth(CHANNEL_TYPE_LUMA) );
        assert( repFormat->getBitDepthVpsChroma() == sps->getBitDepth(CHANNEL_TYPE_CHROMA) );
        assert( repFormat->getConformanceWindowVps().getWindowLeftOffset()   == sps->getConformanceWindow().getWindowLeftOffset() );
        assert( repFormat->getConformanceWindowVps().getWindowRightOffset()  == sps->getConformanceWindow().getWindowRightOffset() );
        assert( repFormat->getConformanceWindowVps().getWindowTopOffset()    == sps->getConformanceWindow().getWindowTopOffset() );
        assert( repFormat->getConformanceWindowVps().getWindowBottomOffset() == sps->getConformanceWindow().getWindowBottomOffset() );
      }    
    }

    //Conformance checking for rep format -- rep format of current picture of current layer shall never be greater rep format defined in VPS for the current layer
    UInt layerIdx = vps->getLayerIdxInVps(m_apcSlicePilot->getLayerId());

    if ( vps->getVpsExtensionFlag() == 1 && (m_apcSlicePilot->getLayerId() == 0 || sps->getV1CompatibleSPSFlag() == 1) )
    {
      assert( sps->getPicWidthInLumaSamples()        <= vps->getVpsRepFormat( vps->getVpsRepFormatIdx(layerIdx) )->getPicWidthVpsInLumaSamples() );
      assert( sps->getPicHeightInLumaSamples()       <= vps->getVpsRepFormat( vps->getVpsRepFormatIdx(layerIdx) )->getPicHeightVpsInLumaSamples() );
      assert( sps->getChromaFormatIdc()              <= vps->getVpsRepFormat( vps->getVpsRepFormatIdx(layerIdx) )->getChromaFormatVpsIdc() );
      assert( sps->getBitDepth(CHANNEL_TYPE_LUMA)    <= vps->getVpsRepFormat( vps->getVpsRepFormatIdx(layerIdx) )->getBitDepthVpsLuma() );
      assert( sps->getBitDepth(CHANNEL_TYPE_CHROMA)  <= vps->getVpsRepFormat( vps->getVpsRepFormatIdx(layerIdx) )->getBitDepthVpsChroma() );
    }
    else if ( vps->getVpsExtensionFlag() == 1 )
    {
      assert( vps->getVpsRepFormat( sps->getUpdateRepFormatFlag() ? sps->getUpdateRepFormatIndex() : vps->getVpsRepFormatIdx(layerIdx))->getPicWidthVpsInLumaSamples()  <= vps->getVpsRepFormat( vps->getVpsRepFormatIdx(layerIdx))->getPicWidthVpsInLumaSamples());
      assert( vps->getVpsRepFormat( sps->getUpdateRepFormatFlag() ? sps->getUpdateRepFormatIndex() : vps->getVpsRepFormatIdx(layerIdx))->getPicHeightVpsInLumaSamples() <= vps->getVpsRepFormat( vps->getVpsRepFormatIdx(layerIdx))->getPicHeightVpsInLumaSamples());
      assert( vps->getVpsRepFormat( sps->getUpdateRepFormatFlag() ? sps->getUpdateRepFormatIndex() : vps->getVpsRepFormatIdx(layerIdx))->getChromaFormatVpsIdc()        <= vps->getVpsRepFormat( vps->getVpsRepFormatIdx(layerIdx))->getChromaFormatVpsIdc());
      assert( vps->getVpsRepFormat( sps->getUpdateRepFormatFlag() ? sps->getUpdateRepFormatIndex() : vps->getVpsRepFormatIdx(layerIdx))->getBitDepthVpsLuma()           <= vps->getVpsRepFormat( vps->getVpsRepFormatIdx(layerIdx))->getBitDepthVpsLuma());
      assert( vps->getVpsRepFormat( sps->getUpdateRepFormatFlag() ? sps->getUpdateRepFormatIndex() : vps->getVpsRepFormatIdx(layerIdx))->getBitDepthVpsChroma()         <= vps->getVpsRepFormat( vps->getVpsRepFormatIdx(layerIdx))->getBitDepthVpsChroma());
    }

    updateSPS->inferSPS( m_layerId, vps );
#endif //SVC_EXTENSION

    // NOTE: globals were set up here originally. You can now use:
    // g_uiMaxCUDepth = sps->getMaxTotalCUDepth();
    // g_uiAddCUDepth = sps->getMaxTotalCUDepth() - sps->getLog2DiffMaxMinCodingBlockSize()

    //  Get a new picture buffer. This will also set up m_pcPic, and therefore give us a SPS and PPS pointer that we can use.
#if SVC_EXTENSION
    xGetNewPicBuffer( *(vps), *(sps), *(pps), m_pcPic, m_apcSlicePilot->getTLayer() );
#else
    xGetNewPicBuffer (*(sps), *(pps), m_pcPic, m_apcSlicePilot->getTLayer());
#endif

#if ALIGNED_BUMPING
    m_apcSlicePilot->checkLeadingPictureRestrictions(m_cListPic);
#else
    m_apcSlicePilot->applyReferencePictureSet(m_cListPic, m_apcSlicePilot->getRPS());
#endif

    // make the slice-pilot a real slice, and set up the slice-pilot for the next slice
    assert(m_pcPic->getNumAllocatedSlice() == (m_uiSliceIdx + 1));
    m_apcSlicePilot = m_pcPic->getPicSym()->swapSliceObject(m_apcSlicePilot, m_uiSliceIdx);

    // we now have a real slice:
    TComSlice *pSlice = m_pcPic->getSlice(m_uiSliceIdx);

    // Update the PPS and SPS pointers with the ones of the picture.
    pps=pSlice->getPPS();
    sps=pSlice->getSPS();

    // Initialise the various objects for the new set of settings
    m_cSAO.create( sps->getPicWidthInLumaSamples(), sps->getPicHeightInLumaSamples(), sps->getChromaFormatIdc(), sps->getMaxCUWidth(), sps->getMaxCUHeight(), sps->getMaxTotalCUDepth(), pps->getPpsRangeExtension().getLog2SaoOffsetScale(CHANNEL_TYPE_LUMA), pps->getPpsRangeExtension().getLog2SaoOffsetScale(CHANNEL_TYPE_CHROMA) );
    m_cLoopFilter.create( sps->getMaxTotalCUDepth() );
    m_cPrediction.initTempBuff(sps->getChromaFormatIdc());


    Bool isField = false;
    Bool isTopField = false;

    if(!m_SEIs.empty())
    {
      // Check if any new Picture Timing SEI has arrived
      SEIMessages pictureTimingSEIs = getSeisByType(m_SEIs, SEI::PICTURE_TIMING);
      if (pictureTimingSEIs.size()>0)
      {
        SEIPictureTiming* pictureTiming = (SEIPictureTiming*) *(pictureTimingSEIs.begin());
        isField    = (pictureTiming->m_picStruct == 1) || (pictureTiming->m_picStruct == 2) || (pictureTiming->m_picStruct == 9) || (pictureTiming->m_picStruct == 10) || (pictureTiming->m_picStruct == 11) || (pictureTiming->m_picStruct == 12);
        isTopField = (pictureTiming->m_picStruct == 1) || (pictureTiming->m_picStruct == 9) || (pictureTiming->m_picStruct == 11);
      }
      
#if Q0189_TMVP_CONSTRAINTS
      // Check if any new temporal motion vector prediction constraints SEI has arrived
      SEIMessages seiTMVPConstrainsList = extractSeisByType (m_SEIs, SEI::TMVP_CONSTRAINTS);
      if (seiTMVPConstrainsList.size() > 0)
      {
        assert ( m_pcPic->getTLayer() == 0 );  //this SEI can present only for AU with Tid equal to 0
        SEITMVPConstrains* tmvpConstraintSEI = (SEITMVPConstrains*) *(seiTMVPConstrainsList.begin());
        if ( tmvpConstraintSEI->prev_pics_not_used_flag == 1 )
        {
          //update all pics in the DPB such that they cannot be used for TMPV ref
          TComList<TComPic*>::iterator  iterRefPic = m_cListPic.begin();  
          while( iterRefPic != m_cListPic.end() )
          {
            TComPic *refPic = *iterRefPic;
            if( ( refPic->getLayerId() == m_pcPic->getLayerId() ) && refPic->getReconMark() )
            {
              for(Int i = refPic->getNumAllocatedSlice()-1; i >= 0; i--)
              {
                TComSlice *refSlice = refPic->getSlice(i);
                refSlice->setAvailableForTMVPRefFlag( false );
              }
            }
            iterRefPic++;
          }
        }
      }
#endif
    }

    //Set Field/Frame coding mode
    m_pcPic->setField(isField);
    m_pcPic->setTopField(isTopField);

    // transfer any SEI messages that have been received to the picture
    m_pcPic->setSEIs(m_SEIs);
    m_SEIs.clear();

    // Recursive structure
    m_cCuDecoder.create ( sps->getMaxTotalCUDepth(), sps->getMaxCUWidth(), sps->getMaxCUHeight(), sps->getChromaFormatIdc() );
#if SVC_EXTENSION
    m_cCuDecoder.init   ( m_ppcTDecTop, &m_cEntropyDecoder, &m_cTrQuant, &m_cPrediction, m_layerId );
#else
    m_cCuDecoder.init   ( &m_cEntropyDecoder, &m_cTrQuant, &m_cPrediction );
#endif
    m_cTrQuant.init     ( sps->getMaxTrSize() );

    m_cSliceDecoder.create();
  }
  else
  {
    // make the slice-pilot a real slice, and set up the slice-pilot for the next slice
    m_pcPic->allocateNewSlice();
    assert(m_pcPic->getNumAllocatedSlice() == (m_uiSliceIdx + 1));
    m_apcSlicePilot = m_pcPic->getPicSym()->swapSliceObject(m_apcSlicePilot, m_uiSliceIdx);

    TComSlice *pSlice = m_pcPic->getSlice(m_uiSliceIdx); // we now have a real slice.

    const TComSPS *sps = pSlice->getSPS();
    const TComPPS *pps = pSlice->getPPS();

    // check that the current active PPS has not changed...
    if (m_parameterSetManager.getSPSChangedFlag(sps->getSPSId()) )
    {
      printf("Error - a new SPS has been decoded while processing a picture\n");
      exit(1);
    }
    if (m_parameterSetManager.getPPSChangedFlag(pps->getPPSId()) )
    {
      printf("Error - a new PPS has been decoded while processing a picture\n");
      exit(1);
    }

    xParsePrefixSEImessages();

    // Check if any new SEI has arrived
     if(!m_SEIs.empty())
     {
       // Currently only decoding Unit SEI message occurring between VCL NALUs copied
       SEIMessages &picSEI = m_pcPic->getSEIs();
       SEIMessages decodingUnitInfos = extractSeisByType (m_SEIs, SEI::DECODING_UNIT_INFO);
       picSEI.insert(picSEI.end(), decodingUnitInfos.begin(), decodingUnitInfos.end());
       deleteSEIs(m_SEIs);
     }
  }
}


Void TDecTop::xParsePrefixSEIsForUnknownVCLNal()
{
  while (!m_prefixSEINALUs.empty())
  {
    // do nothing?
    printf("Discarding Prefix SEI associated with unknown VCL NAL unit.\n");
    delete m_prefixSEINALUs.front();
  }
  // TODO: discard following suffix SEIs as well?
}


Void TDecTop::xParsePrefixSEImessages()
{
#if SVC_EXTENSION
    if (m_prevSliceSkipped) // No need to decode SEI messages of a skipped access unit
    {
      return;
    }
#endif

  while (!m_prefixSEINALUs.empty())
  {
    InputNALUnit &nalu=*m_prefixSEINALUs.front();
#if LAYERS_NOT_PRESENT_SEI
    m_seiReader.parseSEImessage( &(nalu.getBitstream()), m_SEIs, nalu.m_nalUnitType, m_parameterSetManager.getActiveVPS(), m_parameterSetManager.getActiveSPS(), m_pDecodedSEIOutputStream );
#else
    m_seiReader.parseSEImessage( &(nalu.getBitstream()), m_SEIs, nalu.m_nalUnitType, m_parameterSetManager.getActiveSPS(), m_pDecodedSEIOutputStream );
#endif
    delete m_prefixSEINALUs.front();
    m_prefixSEINALUs.pop_front();
  }
}

#if SVC_EXTENSION
Bool TDecTop::xDecodeSlice(InputNALUnit &nalu, Int &iSkipFrame, Int iPOCLastDisplay, UInt& curLayerId, Bool& bNewPOC )
#else
Bool TDecTop::xDecodeSlice(InputNALUnit &nalu, Int &iSkipFrame, Int iPOCLastDisplay )
#endif
{
#if SVC_EXTENSION
  m_apcSlicePilot->initSlice( nalu.m_nuhLayerId );
  m_apcSlicePilot->setTLayer( nalu.m_temporalId );
#else //SVC_EXTENSION
  m_apcSlicePilot->initSlice(); // the slice pilot is an object to prepare for a new slice
                                // it is not associated with picture, sps or pps structures.
#endif
 

  if (m_bFirstSliceInPicture)
  {
    m_uiSliceIdx = 0;
  }
  else
  {
    m_apcSlicePilot->copySliceInfo( m_pcPic->getPicSym()->getSlice(m_uiSliceIdx-1) );
  }
  m_apcSlicePilot->setSliceIdx(m_uiSliceIdx);

  m_apcSlicePilot->setNalUnitType(nalu.m_nalUnitType);

  Bool nonReferenceFlag = (m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_TRAIL_N ||
                           m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_TSA_N   ||
                           m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_STSA_N  ||
                           m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_RADL_N  ||
                           m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_RASL_N);
  m_apcSlicePilot->setTemporalLayerNonReferenceFlag(nonReferenceFlag);
  m_apcSlicePilot->setReferenced(true); // Putting this as true ensures that picture is referenced the first time it is in an RPS
  m_apcSlicePilot->setTLayerInfo(nalu.m_temporalId);

#if ENC_DEC_TRACE
  const UInt64 originalSymbolCount = g_nSymbolCounter;
#endif

  m_cEntropyDecoder.decodeSliceHeader (m_apcSlicePilot, &m_parameterSetManager, m_prevTid0POC);

#if SVC_EXTENSION
  const TComPPS *pps = m_parameterSetManager.getPPS(m_apcSlicePilot->getPPSId());
  assert (pps != 0);
  const TComSPS *sps = m_parameterSetManager.getSPS(pps->getSPSId());
  assert (sps != 0);
  const TComVPS *vps = m_parameterSetManager.getVPS(sps->getVPSId());
  assert (vps != 0);

  // Following check should go wherever the VPS is activated
  if( !vps->getBaseLayerAvailableFlag() )
  {
    assert( nalu.m_nuhLayerId != 0 );
    assert( vps->getNumAddLayerSets() > 0 );

    if( m_commonDecoderParams->getTargetOutputLayerSetIdx() >= 0 )
    {
      UInt layerIdx = vps->getOutputLayerSetIdx( getCommonDecoderParams()->getTargetOutputLayerSetIdx() );
      assert( layerIdx > vps->getVpsNumLayerSetsMinus1() );
    }
  }  

  setRefLayerParams(vps);
  m_apcSlicePilot->setNumMotionPredRefLayers(m_numMotionPredRefLayers);

  // set motion mapping flag
  m_apcSlicePilot->setMFMEnabledFlag( ( m_apcSlicePilot->getNumMotionPredRefLayers() > 0 && m_apcSlicePilot->getActiveNumILRRefIdx() && !m_apcSlicePilot->isIntra() ) ? true : false );

#if VIEW_SCALABILITY
  if( vps->getViewIndex(nalu.m_nuhLayerId) == 1 && sps->getPTL()->getGeneralPTL()->getProfileIdc() == Profile::MULTIVIEWMAIN )
  {
    assert( sps->getInterViewMvVertConstraintFlag() == 1 );
  }
#endif
#endif

  // set POC for dependent slices in skipped pictures
  if(m_apcSlicePilot->getDependentSliceSegmentFlag() && m_prevSliceSkipped)
  {
    m_apcSlicePilot->setPOC(m_skippedPOC);
  }

#if !CONFORMANCE_BITSTREAM_FIX  //This update previous Tid0 POC is moved down and invoke only after the POC derivation has been finalized
  xUpdatePreviousTid0POC(m_apcSlicePilot);
#endif

  m_apcSlicePilot->setAssociatedIRAPPOC(m_pocCRA);
  m_apcSlicePilot->setAssociatedIRAPType(m_associatedIRAPType);

  //For inference of NoOutputOfPriorPicsFlag
  if (m_apcSlicePilot->getRapPicFlag())
  {
    if ((m_apcSlicePilot->getNalUnitType() >= NAL_UNIT_CODED_SLICE_BLA_W_LP && m_apcSlicePilot->getNalUnitType() <= NAL_UNIT_CODED_SLICE_IDR_N_LP) || 
        (m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_CRA && m_bFirstSliceInSequence) ||
        (m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_CRA && m_apcSlicePilot->getHandleCraAsBlaFlag()))
    {
      m_apcSlicePilot->setNoRaslOutputFlag(true);
    }
    //the inference for NoOutputPriorPicsFlag
    if (!m_bFirstSliceInBitstream && m_apcSlicePilot->getRapPicFlag() && m_apcSlicePilot->getNoRaslOutputFlag())
    {
      if (m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_CRA)
      {
        m_apcSlicePilot->setNoOutputPriorPicsFlag(true);
      }
    }
    else
    {
      m_apcSlicePilot->setNoOutputPriorPicsFlag(false);
    }

    if(m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_CRA)
    {
      m_craNoRaslOutputFlag = m_apcSlicePilot->getNoRaslOutputFlag();
    }
  }
  if (m_apcSlicePilot->getRapPicFlag() && m_apcSlicePilot->getNoOutputPriorPicsFlag())
  {
    m_lastPOCNoOutputPriorPics = m_apcSlicePilot->getPOC();
    m_isNoOutputPriorPics = true;
  }
  else
  {
    m_isNoOutputPriorPics = false;
  }

  //For inference of PicOutputFlag
  if (m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_RASL_N || m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_RASL_R)
  {
    if ( m_craNoRaslOutputFlag )
    {
      m_apcSlicePilot->setPicOutputFlag(false);
    }
  }

  if (m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_CRA && m_craNoRaslOutputFlag) //Reset POC MSB when CRA has NoRaslOutputFlag equal to 1
  {
#if !SVC_EXTENSION
    TComPPS *pps = m_parameterSetManager.getPPS(m_apcSlicePilot->getPPSId());
    assert (pps != 0);
    TComSPS *sps = m_parameterSetManager.getSPS(pps->getSPSId());
    assert (sps != 0);
#endif
    Int iMaxPOClsb = 1 << sps->getBitsForPOC();
    m_apcSlicePilot->setPOC( m_apcSlicePilot->getPOC() & (iMaxPOClsb - 1) );
#if !CONFORMANCE_BITSTREAM_FIX  //This update previous Tid0 POC is moved down and invoke only after the POC derivation has been finalized
    xUpdatePreviousTid0POC(m_apcSlicePilot);
#endif
  }

  // Skip pictures due to random access

  if (isRandomAccessSkipPicture(iSkipFrame, iPOCLastDisplay))
  {
    m_prevSliceSkipped = true;
    m_skippedPOC = m_apcSlicePilot->getPOC();
    return false;
  }
  // Skip TFD pictures associated with BLA/BLANT pictures
  if (isSkipPictureForBLA(iPOCLastDisplay))
  {
    m_prevSliceSkipped = true;
    m_skippedPOC = m_apcSlicePilot->getPOC();
    return false;
  }

  // clear previous slice skipped flag
  m_prevSliceSkipped = false;

  // exit when a new picture is found
#if SVC_EXTENSION
  bNewPOC = m_apcSlicePilot->getPOC() != m_prevPOC || ( m_apcSlicePilot->getFirstSliceInPic() && m_parseIdc == -1 );

#if NO_CLRAS_OUTPUT_FLAG
  if( m_layerId == m_smallestLayerId && m_apcSlicePilot->getRapPicFlag() )
  {
    if( m_bFirstSliceInSequence )
    {
      setNoClrasOutputFlag(true);
    }
    else if( m_lastPicHasEos )
    {
      setNoClrasOutputFlag(true);
    }
    else if( m_apcSlicePilot->getBlaPicFlag() )
    {
      setNoClrasOutputFlag(true);
    }
    else if( m_apcSlicePilot->getIdrPicFlag() && m_apcSlicePilot->getCrossLayerBLAFlag() )
    {
      setNoClrasOutputFlag(true);
    }
    else
    {
      setNoClrasOutputFlag(false);
    }      
  }
  else
  {
    setNoClrasOutputFlag(false);
  }

  m_apcSlicePilot->decodingRefreshMarking( m_cListPic, m_noClrasOutputFlag, m_smallestLayerId );
#endif

  // Derive the value of NoOutputOfPriorPicsFlag
  if( bNewPOC || m_layerId!=m_uiPrevLayerId )   // i.e. new coded picture
  {
    if( m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_CRA && m_apcSlicePilot->getNoRaslOutputFlag() )
    {
      this->setNoOutputPriorPicsFlag( true );
    }
    else if( m_apcSlicePilot->getRapPicFlag() && m_apcSlicePilot->getNoRaslOutputFlag() )
    {
      this->setNoOutputPriorPicsFlag( m_apcSlicePilot->getNoOutputPriorPicsFlag() );
    }
    else
    {
      if( this->m_ppcTDecTop[0]->getNoClrasOutputFlag() )
      {
        this->setNoOutputPriorPicsFlag( true );
      }
    }
  }

#if SVC_POC
  if( m_parseIdc != -1 ) // Second pass for a POC resetting picture
  {
    m_parseIdc++; // Proceed to POC decoding and RPS derivation
  }
  
  if( m_parseIdc == 2 )
  {
    bNewPOC = false;
  }

  if( (bNewPOC || m_layerId!=m_uiPrevLayerId) && (m_parseIdc == -1) ) // Will be true at the first pass
  {
  //if (bNewPOC || m_layerId!=m_uiPrevLayerId)
  // Check if new reset period has started - this is needed just so that the SHM decoder which calls slice header decoding twice 
  // does not invoke the output twice
  //if( m_lastPocPeriodId[m_apcSlicePilot->getLayerId()] == m_apcSlicePilot->getPocResetPeriodId() )
    // Update CurrAU marking
    if(( m_layerId < m_uiPrevLayerId) ||( ( m_layerId == m_uiPrevLayerId) && bNewPOC)) // Decoding a lower layer than or same layer as previous - mark all earlier pictures as not in current AU
    {
      // New access unit; reset all variables related to POC reset restrictions
      resetPocRestrictionCheckParameters();

      markAllPicsAsNoCurrAu(vps);

      for( UInt i = 0; i < MAX_LAYERS; i++ )
      {
        m_ppcTDecTop[vps->getLayerIdInNuh(i)]->m_pocDecrementedInDPBFlag = false;
      }
    }

    m_pocResettingFlag = false;

    if( m_apcSlicePilot->getPocResetIdc() != 0 )
    {
      if( vps->getVpsPocLsbAlignedFlag() )
      {
        m_pocResettingFlag = true;
      }
      else if (m_pocDecrementedInDPBFlag)
      {
        m_pocResettingFlag = false;
      }
      else
      {
        m_pocResettingFlag = true;
      }
    }

#if CONFORMANCE_BITSTREAM_FIX
    if( m_apcSlicePilot->getPocResetIdc() && m_apcSlicePilot->getFirstSliceInPic() )
#else
    if( m_apcSlicePilot->getPocResetIdc() && m_apcSlicePilot->getSliceIdx() == 0 )
#endif
    {
      Int pocResetPeriodId = m_apcSlicePilot->getPocResetPeriodId();
      if ( m_apcSlicePilot->getPocResetIdc() == 1 || m_apcSlicePilot->getPocResetIdc() == 2 ||
        ( m_apcSlicePilot->getPocResetIdc() == 3 && pocResetPeriodId != getLastPocPeriodId() ) )
      {
        setLastPocPeriodId(pocResetPeriodId);
        m_parseIdc = 0;
      }

      // Check if the POC Reset period ID matches with the Reset Period ID 
      if( pocResetPeriodId == m_crossLayerPocResetPeriodId )
      {
        // If matching, and current poc_reset_idc = 3, then the values should match
        if( m_apcSlicePilot->getPocResetIdc() == 3 )
        {
          assert( ( m_apcSlicePilot->getFullPocResetFlag() == false && m_crossLayerPocResetIdc == 1 ) ||
                  ( m_apcSlicePilot->getFullPocResetFlag() == true  && m_crossLayerPocResetIdc == 2 ) );
        }
      }
      else
      {
        // This is the first picture of a POC resetting access unit
        m_crossLayerPocResetPeriodId = pocResetPeriodId;
        if( m_apcSlicePilot->getPocResetIdc() == 1 || m_apcSlicePilot->getPocResetIdc() == 2 )
        {
          m_crossLayerPocResetIdc = m_apcSlicePilot->getPocResetIdc();
        }
        else
        { // poc_reset_idc = 3
          // In this case, the entire POC resetting access unit has been lost. 
          // Need more checking to ensure poc_reset_idc = 3 works.
          assert ( 0 );
        }
      }
    }
    else
    {
      m_parseIdc = 3; // Proceed to decoding POC and RPS
    }
  }
#endif

#if ALIGNED_BUMPING
  UInt affectedLayerList[MAX_LAYERS];
  Int  numAffectedLayers;

  affectedLayerList[0] = m_apcSlicePilot->getLayerId();
  numAffectedLayers = 1;

  if( vps->getVpsPocLsbAlignedFlag() )
  {
    for( UInt j = 0; j < vps->getNumPredictedLayers(m_apcSlicePilot->getLayerId()); j++ )
    {
      affectedLayerList[j + 1] = vps->getPredictedLayerId(m_apcSlicePilot->getLayerId(), j);
    }
    numAffectedLayers = vps->getNumPredictedLayers(m_apcSlicePilot->getLayerId()) + 1;
  }

  if( m_parseIdc == 1 && m_pocResettingFlag )
  {
    // Invoke output of pictures if the current picture is a POC reset picture
    bNewPOC = true;
    /* Include reset of all POCs in the layer */

    // This operation would do the following:
    // 1. Update the other picture in the DPB. This should be done only for the first slice of the picture.
    // 2. Update the value of m_pocCRA.
    // 3. Reset the POC values at the decoder for the current picture to be zero - will be done later
    // 4. update value of POCLastDisplay

    //Do the reset stuff here
    Int maxPocLsb = 1 << sps->getBitsForPOC();
    Int pocLsbVal;
    if( m_apcSlicePilot->getPocResetIdc() == 3 )
    {
      pocLsbVal = m_apcSlicePilot->getPocLsbVal() ;
    }
    else
    {
      pocLsbVal = (m_apcSlicePilot->getPOC() % maxPocLsb);
    }

    Int pocMsbDelta = 0;
    if( m_apcSlicePilot->getPocMsbValPresentFlag() ) 
    {
      pocMsbDelta = m_apcSlicePilot->getPocMsbVal() * maxPocLsb;
    }
    else
    {
      //This MSB derivation can be made into one function. Item to do next.
      Int prevPoc     = this->getPrevPicOrderCnt();
      Int prevPocLsb  = prevPoc & (maxPocLsb - 1);
      Int prevPocMsb  = prevPoc - prevPocLsb;

      pocMsbDelta = m_apcSlicePilot->getCurrMsb( pocLsbVal, prevPocLsb, prevPocMsb, maxPocLsb );
    }

    Int pocLsbDelta;
    if( m_apcSlicePilot->getPocResetIdc() == 2 ||  ( m_apcSlicePilot->getPocResetIdc() == 3 && m_apcSlicePilot->getFullPocResetFlag() ))
    {
      pocLsbDelta = pocLsbVal;
    }
    else
    {
      pocLsbDelta = 0; 
    }

    Int deltaPocVal  =  pocMsbDelta + pocLsbDelta;

    for( UInt layerIdx = 0; layerIdx < numAffectedLayers; layerIdx++ )
    {
      if (!m_ppcTDecTop[affectedLayerList[layerIdx]]->m_pocDecrementedInDPBFlag)
      {
        m_ppcTDecTop[affectedLayerList[layerIdx]]->m_pocDecrementedInDPBFlag = true;
        TComList<TComPic*>::iterator  iterPic = m_ppcTDecTop[affectedLayerList[layerIdx]]->getListPic()->begin();

        while (iterPic != m_ppcTDecTop[affectedLayerList[layerIdx]]->getListPic()->end())
        {
          TComPic *dpbPic = *iterPic;
          // Check if the picture pointed to by iterPic is either used for reference or
          // needed for output, are in the same layer, and not the current picture.
          assert(dpbPic->getLayerId() == affectedLayerList[layerIdx]);

          if( (dpbPic->getReconMark()) && (dpbPic->getPicSym()->getSlice(0)->getPicOutputFlag()) )
          {
            for(Int i = dpbPic->getNumAllocatedSlice()-1; i >= 0; i--)
            {

              TComSlice *slice = dpbPic->getSlice(i);
              TComReferencePictureSet *rps = slice->getLocalRPS();
              slice->setPOC( slice->getPOC() - deltaPocVal );

              // Also adjust the POC value stored in the RPS of each such slice
              for(Int j = rps->getNumberOfPictures(); j >= 0; j--)
              {
                rps->setPOC( j, rps->getPOC(j) - deltaPocVal );
              }

              slice->setRPS(rps);

              // Also adjust the value of refPOC
              for(Int k = 0; k < 2; k++)  // For List 0 and List 1
              {
                RefPicList list = (k == 1) ? REF_PIC_LIST_1 : REF_PIC_LIST_0;
                for(Int j = 0; j < slice->getNumRefIdx(list); j++)
                {
                  slice->setRefPOC( slice->getRefPOC(list, j) - deltaPocVal, list, j);
                }
              }
            }
          }
          iterPic++;
        }
        // Update the value of pocCRA
        m_ppcTDecTop[affectedLayerList[layerIdx]]->m_pocCRA -= deltaPocVal;
      }
    }

    // Update value of POCLastDisplay
    iPOCLastDisplay -= deltaPocVal;
  }
  Int maxPocLsb = 1 << sps->getBitsForPOC();
  Int slicePicOrderCntLsb = m_apcSlicePilot->getPicOrderCntLsb();

  if( m_pocResettingFlag && (m_parseIdc == 1 || m_parseIdc == 2) )
  {
    // Set poc for current slice
    if( m_apcSlicePilot->getPocResetIdc() == 1 )
    {        
      m_apcSlicePilot->setPOC( slicePicOrderCntLsb );
    }
    else if( m_apcSlicePilot->getPocResetIdc() == 2 )
    {
      m_apcSlicePilot->setPOC( 0 );
    }
    else 
    {
      Int picOrderCntMsb = m_apcSlicePilot->getCurrMsb( slicePicOrderCntLsb, m_apcSlicePilot->getFullPocResetFlag() ? 0 : m_apcSlicePilot->getPocLsbVal(), 0 , maxPocLsb );
      m_apcSlicePilot->setPOC( picOrderCntMsb + slicePicOrderCntLsb );
    }
  }
  else if (m_parseIdc == 3)
  {
    Int picOrderCntMsb = 0;
    if( m_apcSlicePilot->getPocMsbValPresentFlag() )
    {
      picOrderCntMsb = m_apcSlicePilot->getPocMsbVal() * maxPocLsb;
    }
    else if( m_apcSlicePilot->getIdrPicFlag() )
    {
      picOrderCntMsb = 0;
    }
    else
    {
      Int prevPicOrderCntLsb = this->getPrevPicOrderCnt() & ( maxPocLsb - 1);
      Int prevPicOrderCntMsb  = this->getPrevPicOrderCnt() - prevPicOrderCntLsb;
      picOrderCntMsb = m_apcSlicePilot->getCurrMsb(slicePicOrderCntLsb, prevPicOrderCntLsb, prevPicOrderCntMsb, maxPocLsb );
    }
    m_apcSlicePilot->setPOC( picOrderCntMsb + slicePicOrderCntLsb );
  }

#if CONFORMANCE_BITSTREAM_FIX
  xUpdatePreviousTid0POC(m_apcSlicePilot);
#endif

  //detect lost reference picture and insert copy of earlier frame.
  {
    Int lostPoc;
    while((lostPoc=m_apcSlicePilot->checkThatAllRefPicsAreAvailable(m_cListPic, m_apcSlicePilot->getRPS(), true, m_pocRandomAccess)) > 0)
    {
      xCreateLostPicture(lostPoc-1);
    }
  }

  if( m_parseIdc == 1 || m_parseIdc == 3)
  {
    // Adjust prevPicOrderCnt
    if(    !m_apcSlicePilot->getRaslPicFlag() 
      && !m_apcSlicePilot->getRadlPicFlag()
      && (m_apcSlicePilot->getNalUnitType() % 2 == 1
#if CONFORMANCE_BITSTREAM_FIX
        || (m_apcSlicePilot->getNalUnitType() >= NAL_UNIT_CODED_SLICE_BLA_W_LP && m_apcSlicePilot->getNalUnitType() <= NAL_UNIT_RESERVED_IRAP_VCL23)) // should it be NAL_UNIT_CODED_SLICE_CRA instead?
#else
        )
#endif
      && ( nalu.m_temporalId == 0 )
      && !m_apcSlicePilot->getDiscardableFlag() )
    {
      for( UInt i = 0; i < numAffectedLayers; i++ )
      {
        m_ppcTDecTop[affectedLayerList[i]]->setPrevPicOrderCnt(m_apcSlicePilot->getPOC());
      }
    }
    else if( m_apcSlicePilot->getPocResetIdc() == 3 )
    {
      if( !m_firstPicInLayerDecodedFlag || (m_firstPicInLayerDecodedFlag && m_pocResettingFlag) )
      {
        for( UInt i = 0; i < numAffectedLayers; i++ )
        {
          m_ppcTDecTop[affectedLayerList[i]]->setPrevPicOrderCnt( m_apcSlicePilot->getFullPocResetFlag() ? 0 : m_apcSlicePilot->getPocLsbVal() );
        }
      }
    }
    m_apcSlicePilot->applyReferencePictureSet(m_cListPic, m_apcSlicePilot->getRPS());
  }

  if( !m_apcSlicePilot->getDependentSliceSegmentFlag() && (bNewPOC || m_layerId!=m_uiPrevLayerId || m_parseIdc == 1) && !m_bFirstSliceInSequence )
#else
  if (!m_apcSlicePilot->getDependentSliceSegmentFlag() && (bNewPOC || m_layerId!=m_uiPrevLayerId) && !m_bFirstSliceInSequence )
#endif
  {
    m_prevPOC = m_apcSlicePilot->getPOC();
#if ENC_DEC_TRACE
    //rewind the trace counter since we didn't actually decode the slice
    g_nSymbolCounter = originalSymbolCount;
#endif
    curLayerId = m_uiPrevLayerId; 
    m_uiPrevLayerId = m_layerId;
    return true;
  }

  m_parseIdc = -1;

  if( m_apcSlicePilot->getTLayer() == 0 && m_apcSlicePilot->getEnableTMVPFlag() == 0 )
  {
    //update all pics in the DPB such that they cannot be used for TMPV ref
    TComList<TComPic*>::iterator  iterRefPic = m_cListPic.begin();

    while( iterRefPic != m_cListPic.end() )
    {
      TComPic *refPic = *iterRefPic;
      if( ( refPic->getLayerId() == m_apcSlicePilot->getLayerId() ) && refPic->getReconMark() )
      {
        for( Int i = refPic->getNumAllocatedSlice()-1; i >= 0; i-- )
        {

          TComSlice *refSlice = refPic->getSlice(i);
          refSlice->setAvailableForTMVPRefFlag( false );
        }
      }
      iterRefPic++;
    }
  }
  m_apcSlicePilot->setAvailableForTMVPRefFlag( true );
#else //SVC_EXTENSION
  //we should only get a different poc for a new picture (with CTU address==0)
  if (!m_apcSlicePilot->getDependentSliceSegmentFlag() && m_apcSlicePilot->getPOC()!=m_prevPOC && !m_bFirstSliceInSequence && (m_apcSlicePilot->getSliceCurStartCtuTsAddr() != 0))  
  {
    printf ("Warning, the first slice of a picture might have been lost!\n");
  }

  // exit when a new picture is found
  if (!m_apcSlicePilot->getDependentSliceSegmentFlag() && (m_apcSlicePilot->getSliceCurStartCtuTsAddr() == 0 && !m_bFirstSliceInPicture) )
  {
    if (m_prevPOC >= m_pocRandomAccess)
    {
      m_prevPOC = m_apcSlicePilot->getPOC();
#if ENC_DEC_TRACE
      //rewind the trace counter since we didn't actually decode the slice
      g_nSymbolCounter = originalSymbolCount;
#endif
      return true;
    }
    m_prevPOC = m_apcSlicePilot->getPOC();
  }

  //detect lost reference picture and insert copy of earlier frame.
  {
    Int lostPoc;
    while((lostPoc=m_apcSlicePilot->checkThatAllRefPicsAreAvailable(m_cListPic, m_apcSlicePilot->getRPS(), true, m_pocRandomAccess)) > 0)
    {
      xCreateLostPicture(lostPoc-1);
    }
  }
#endif //SVC_EXTENSION

  if (!m_apcSlicePilot->getDependentSliceSegmentFlag()) 
  {
    m_prevPOC = m_apcSlicePilot->getPOC();
#if SVC_EXTENSION
    curLayerId = m_layerId;
    m_uiPrevLayerId = m_layerId;
#endif
  }

  // actual decoding starts here
  //下面这个函数功能有：1.构造TComPic
  xActivateParameterSets();
  //运行到这里，应该所有参数集合都正式解码完了;
#if 1//&& m_layerId ==MaxLayerId;
  lmAllDecInfo *lminfo = lmAllDecInfo::getInstance();
  if (m_layerId==vps->getMaxLayerId()&& (lminfo->hasOutputDec()==0))
	{
	  ParameterSetManager *allps = getParameterSetManager();
	  int i = 0;
	  TComVPS*      fvps = allps->getVPS(i);
	  while (fvps != nullptr)
	  {
		  lmPSData tVPS(lmPSData::getPSTypeInString(paraTYPE::vps));
		  tVPS << sParam(tVPS.getParamName(0), int(fvps->getMaxLayers()))
			  << sParam(tVPS.getParamName(1), int(fvps->getVPSId()));
		  (*lminfo) << tVPS;
		  i++;
		  fvps = allps->getVPS(i);
	  }

	  i = 0;
	  TComSPS*      fsps = allps->getSPS(i);
	  while (fsps != nullptr)
	  {
		  lmPSData tSPS(lmPSData::getPSTypeInString(paraTYPE::sps));
		  int x = 0;
		  tSPS << sParam(tSPS.getParamName(0), int(fsps->getSPSId()))
				<< sParam(tSPS.getParamName(1), int(fsps->getLayerId()))
				<< sParam(tSPS.getParamName(2), int(fsps->getChromaFormatIdc()))
				<< sParam(tSPS.getParamName(3), int(fsps->getPicWidthInLumaSamples()))
				<< sParam(tSPS.getParamName(4), int(fsps->getPicHeightInLumaSamples()));
		  (*lminfo) << tSPS;
		  i++;
		  fsps = allps->getSPS(i);
	  }

	  i = 0;
	  TComPPS*      fpps = allps->getPPS(i);
	  while (fpps != nullptr)
	  {
		  lmPSData tPPS(lmPSData::getPSTypeInString(paraTYPE::pps));
		  tPPS << sParam(tPPS.getParamName(0), int(fpps->getSPSId()))
			  << sParam(tPPS.getParamName(1), int(fpps->getLayerId()));
		  (*lminfo) << tPPS;
		  i++;
		  fpps = allps->getPPS(i);
	  }
	  lminfo->outputDec();
	}
#endif
  m_bFirstSliceInSequence = false;
  m_bFirstSliceInBitstream  = false;

  TComSlice* pcSlice = m_pcPic->getPicSym()->getSlice(m_uiSliceIdx);

#if SVC_EXTENSION
  if (m_bFirstSliceInPicture)
  {
    // Initialize ILRP if needed, only for the current layer  
    // ILRP intialization should go along with activation of parameters sets, 
    // although activation of parameter sets itself need not be done for each and every slice!!!
    xInitILRP(pcSlice->getSPS(), pcSlice->getPPS());

#if AVC_BASE
    if( m_layerId > 0 && vps->getNonHEVCBaseLayerFlag() )
    {
      TComPic* pBLPic = (*m_ppcTDecTop[0]->getListPic()->begin());
      pBLPic->getSlice(0)->setReferenced(true);
      fstream* pFile  = m_ppcTDecTop[0]->getBLReconFile();

      if( pFile->good() )
      {
        Bool is16bit  = pBLPic->getSlice(0)->getSPS()->getBitDepth(CHANNEL_TYPE_LUMA) > 8 || pBLPic->getSlice(0)->getSPS()->getBitDepth(CHANNEL_TYPE_CHROMA) > 8;
        UInt uiWidth  = pBLPic->getPicYuvRec()->getWidth(COMPONENT_Y);
        UInt uiHeight = pBLPic->getPicYuvRec()->getHeight(COMPONENT_Y);

        Int len = uiWidth * (is16bit ? 2 : 1);
        UChar *buf = new UChar[len];

        UInt64 uiPos = (UInt64) pcSlice->getPOC() * uiWidth * uiHeight * 3 / 2;
        if( is16bit )
        {
          uiPos <<= 1;
        }

        pFile->seekg((UInt)uiPos, ios::beg );

        // read Y component
        Pel* pPel = pBLPic->getPicYuvRec()->getAddr(COMPONENT_Y);
        UInt uiStride = pBLPic->getPicYuvRec()->getStride(COMPONENT_Y);
        for( Int i = 0; i < uiHeight; i++ )
        {
          pFile->read(reinterpret_cast<TChar*>(buf), len);

          if( !is16bit )
          {
            for (Int x = 0; x < uiWidth; x++)
            {
              pPel[x] = buf[x];
            }
          }
          else
          {
            for (Int x = 0; x < uiWidth; x++)
            {
              pPel[x] = (buf[2*x+1] << 8) | buf[2*x];
            }
          }
     
          pPel += uiStride;
        }

        len >>= 1;
        uiWidth >>= 1;
        uiHeight >>= 1;

        // read Cb component
        pPel = pBLPic->getPicYuvRec()->getAddr(COMPONENT_Cb);
        uiStride = pBLPic->getPicYuvRec()->getStride(COMPONENT_Cb);
        for( Int i = 0; i < uiHeight; i++ )
        {
          pFile->read(reinterpret_cast<TChar*>(buf), len);

          if( !is16bit )
          {
            for( Int x = 0; x < uiWidth; x++ )
            {
              pPel[x] = buf[x];
            }
          }
          else
          {
            for( Int x = 0; x < uiWidth; x++ )
            {
              pPel[x] = (buf[2*x+1] << 8) | buf[2*x];
            }
          }
     
          pPel += uiStride;
        }

        // read Cr component
        pPel = pBLPic->getPicYuvRec()->getAddr(COMPONENT_Cr);
        uiStride = pBLPic->getPicYuvRec()->getStride(COMPONENT_Cr);
        for( Int i = 0; i < uiHeight; i++ )
        {
          pFile->read(reinterpret_cast<TChar*>(buf), len);

          if( !is16bit )
          {
            for( Int x = 0; x < uiWidth; x++ )
            {
              pPel[x] = buf[x];
            }
          }
          else
          {
            for( Int x = 0; x < uiWidth; x++ )
            {
              pPel[x] = (buf[2*x+1] << 8) | buf[2*x];
            }
          }
     
          pPel += uiStride;
        }

        delete[] buf;
      }
    }
#endif

    if ( m_layerId == 0 && pcSlice->getRapPicFlag() && getNoClrasOutputFlag() )
    {
      for (UInt i = 0; i < vps->getMaxLayers(); i++)
      {
        m_ppcTDecTop[i]->setLayerInitializedFlag(false);
        m_ppcTDecTop[i]->setFirstPicInLayerDecodedFlag(false);
      }
    }

    xCheckLayerReset();
    xSetLayerInitializedFlag();
    
#if SVC_POC
    m_pcPic->setCurrAuFlag( true );

    if( m_pcPic->getLayerId() > 0 && pcSlice->isIDR() && !m_nonBaseIdrPresentFlag )
    {
      // IDR picture with nuh_layer_id > 0 present
      m_nonBaseIdrPresentFlag = true;
      m_nonBaseIdrType = (pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_W_RADL);
    }
    else
    {
      if( pcSlice->getNalUnitType() != NAL_UNIT_CODED_SLICE_IDR_W_RADL )
      {
        // Picture with nal_unit_type not equal IDR_W_RADL present
        m_picNonIdrWithRadlPresentFlag = true;
      }
      if( pcSlice->getNalUnitType() != NAL_UNIT_CODED_SLICE_IDR_N_LP )
      {
        // Picture with nal_unit_type not equal IDR_N_LP present
        m_picNonIdrNoLpPresentFlag = true;
      }
    }
    if( !m_checkPocRestrictionsForCurrAu )  // Will be true for the first slice/picture of the AU
    {
      m_checkPocRestrictionsForCurrAu = true;
      m_pocResetIdcOrCurrAu = pcSlice->getPocResetIdc();
      if( m_pcPic->getLayerId() == 0 )
      {
        // Base layer picture is present
        m_baseLayerPicPresentFlag = true;
        if( pcSlice->isIRAP() )
        {
          // Base layer picture is IRAP
          m_baseLayerIrapFlag = true;
        }
        if( pcSlice->isIDR() )
        {
          // Base layer picture is IDR
          m_baseLayerIdrFlag = true;
        }
        else
        {
          if( vps->getBaseLayerInternalFlag())
          {
            /* When the picture with nuh_layer_id equal to 0 in an access unit is not an IDR picture 
            and vps_base_layer_internal_flag is equal to 1, the value of poc_reset_idc shall not be equal to 2 
            for any picture in the access unit. */
            assert( pcSlice->getPocResetIdc() != 2 );
          }
        }
      }
    }
    else
    {
      // The value of poc_reset_idc of all coded pictures that are present in the bitstream in an access unit shall be the same.
      assert( m_pocResetIdcOrCurrAu == pcSlice->getPocResetIdc() );

      /* When the picture in an access unit with nuh_layer_id equal to 0 is an IRAP picture and vps_base_layer_internal_flag is equal to 1 
      and there is at least one other picture in the same access unit that is not an IRAP picture, 
      the value of poc_reset_idc shall be equal to 1 or 2 for all pictures in the access unit. */
      if( m_baseLayerPicPresentFlag && m_baseLayerIrapFlag && !pcSlice->isIRAP() && vps->getBaseLayerInternalFlag() )
      {
        assert( pcSlice->getPocResetIdc() == 1 || pcSlice->getPocResetIdc() == 2 );
      }

      /* When the picture with nuh_layer_id equal to 0 in an access unit is an IDR picture and 
      vps_base_layer_internal_flag is equal to 1 and there is at least one non-IDR picture in the same access unit, 
      the value of poc_reset_idc shall be equal to 2 for all pictures in the access unit. */
      if( m_baseLayerPicPresentFlag && m_baseLayerIdrFlag && !pcSlice->isIDR() && vps->getBaseLayerInternalFlag() )
      {
        assert( pcSlice->getPocResetIdc() == 2 );
      }

      /* When there is at least one picture that has nuh_layer_id greater than 0 and that is an IDR picture 
      with a particular value of nal_unit_type in an access unit and there is at least one other coded picture 
      that is present in the bitstream in the same access unit with a different value of nal_unit_type, 
      the value of poc_reset_idc shall be equal to 1 or 2 for all pictures in the access unit. */
      if( m_nonBaseIdrPresentFlag && (
            ( m_nonBaseIdrType == 1 && m_picNonIdrWithRadlPresentFlag ) ||
            ( m_nonBaseIdrType == 0 && m_picNonIdrNoLpPresentFlag )
        ))
      {
        assert( pcSlice->getPocResetIdc() == 1 || pcSlice->getPocResetIdc() == 2 );
      }
    }
#endif
  }
#endif //SVC_EXTENSION
   
  // When decoding the slice header, the stored start and end addresses were actually RS addresses, not TS addresses.
  // Now, having set up the maps, convert them to the correct form.
  pcSlice->setSliceSegmentCurStartCtuTsAddr( m_pcPic->getPicSym()->getCtuRsToTsAddrMap(pcSlice->getSliceSegmentCurStartCtuTsAddr()) );
  pcSlice->setSliceSegmentCurEndCtuTsAddr( m_pcPic->getPicSym()->getCtuRsToTsAddrMap(pcSlice->getSliceSegmentCurEndCtuTsAddr()) );
  if(!pcSlice->getDependentSliceSegmentFlag())
  {
    pcSlice->setSliceCurStartCtuTsAddr(m_pcPic->getPicSym()->getCtuRsToTsAddrMap(pcSlice->getSliceCurStartCtuTsAddr()));
    pcSlice->setSliceCurEndCtuTsAddr(m_pcPic->getPicSym()->getCtuRsToTsAddrMap(pcSlice->getSliceCurEndCtuTsAddr()));
  }
    
  m_pcPic->setTLayer(nalu.m_temporalId);

#if SVC_EXTENSION
  m_pcPic->setLayerId(nalu.m_nuhLayerId);
  pcSlice->setLayerId(nalu.m_nuhLayerId);
  pcSlice->setPic(m_pcPic);
#endif

  if (!pcSlice->getDependentSliceSegmentFlag())
  {
    pcSlice->checkCRA(pcSlice->getRPS(), m_pocCRA, m_associatedIRAPType, m_cListPic );
    // Set reference list
#if SVC_EXTENSION
    if (m_layerId == 0)
#endif
    pcSlice->setRefPicList( m_cListPic, true );

#if SVC_EXTENSION
    // Create upsampling reference layer pictures for all possible dependent layers and do it only once for the first slice. 
    // Other slices might choose which reference pictures to be used for inter-layer prediction
    if( m_layerId > 0 && m_uiSliceIdx == 0 && ( !pcSlice->getVPS()->getSingleLayerForNonIrapFlag() || pcSlice->isIRAP() ) )
    {
      // create buffers for scaling factors
      if( pcSlice->getNumILRRefIdx() )
      {
        m_pcPic->createMvScalingFactor(pcSlice->getNumILRRefIdx());
        m_pcPic->createPosScalingFactor(pcSlice->getNumILRRefIdx());
      }

      for( Int i = 0; i < pcSlice->getNumILRRefIdx(); i++ )
      {
        UInt refLayerIdc = i;
        UInt refLayerId = pcSlice->getVPS()->getRefLayerId(m_layerId, refLayerIdc);
#if AVC_BASE
        if( refLayerId == 0 && m_parameterSetManager.getActiveVPS()->getNonHEVCBaseLayerFlag() )
        {          
          TComPic* pic = *m_ppcTDecTop[0]->getListPic()->begin();

          if( pic )
          {
            pcSlice->setBaseColPic( refLayerIdc, pic );
          }
          else
          {
            continue;
          }
        }
        else
        {
          TDecTop *pcTDecTop = (TDecTop *)getRefLayerDec( refLayerIdc );
          TComList<TComPic*> *cListPic = pcTDecTop->getListPic();
          if( !pcSlice->setBaseColPic( *cListPic, refLayerIdc ) )
          {
            continue;
          }
        }
#else
        TDecTop *pcTDecTop = (TDecTop *)getRefLayerDec( refLayerIdc );
        TComList<TComPic*> *cListPic = pcTDecTop->getListPic();
        pcSlice->setBaseColPic ( *cListPic, refLayerIdc );
#endif

        const Window &scalEL = pcSlice->getPPS()->getScaledRefLayerWindowForLayer(refLayerId);
        const Window &windowRL = pcSlice->getPPS()->getRefLayerWindowForLayer(pcSlice->getVPS()->getRefLayerId(m_layerId, refLayerIdc));
        Int widthBL   = pcSlice->getBaseColPic(refLayerIdc)->getPicYuvRec()->getWidth(COMPONENT_Y) - windowRL.getWindowLeftOffset() - windowRL.getWindowRightOffset();
        Int heightBL  = pcSlice->getBaseColPic(refLayerIdc)->getPicYuvRec()->getHeight(COMPONENT_Y) - windowRL.getWindowTopOffset() - windowRL.getWindowBottomOffset();
        Int widthEL   = m_pcPic->getPicYuvRec()->getWidth(COMPONENT_Y)  - scalEL.getWindowLeftOffset() - scalEL.getWindowRightOffset();
        Int heightEL  = m_pcPic->getPicYuvRec()->getHeight(COMPONENT_Y) - scalEL.getWindowTopOffset()  - scalEL.getWindowBottomOffset();

        // conformance check: the values of RefLayerRegionWidthInSamplesY, RefLayerRegionHeightInSamplesY, ScaledRefRegionWidthInSamplesY and ScaledRefRegionHeightInSamplesY shall be greater than 0
        assert(widthEL > 0 && heightEL > 0 && widthBL > 0 && heightBL > 0);

        // conformance check: ScaledRefRegionWidthInSamplesY shall be greater or equal to RefLayerRegionWidthInSamplesY and ScaledRefRegionHeightInSamplesY shall be greater or equal to RefLayerRegionHeightInSamplesY
        assert(widthEL >= widthBL && heightEL >= heightBL);

        // conformance check: when ScaledRefRegionWidthInSamplesY is equal to RefLayerRegionWidthInSamplesY, PhaseHorY shall be equal to 0, when ScaledRefRegionWidthInSamplesC is equal to RefLayerRegionWidthInSamplesC, PhaseHorC shall be equal to 0, when ScaledRefRegionHeightInSamplesY is equal to RefLayerRegionHeightInSamplesY, PhaseVerY shall be equal to 0, and when ScaledRefRegionHeightInSamplesC is equal to RefLayerRegionHeightInSamplesC, PhaseVerC shall be equal to 0.
        const ResamplingPhase &resamplingPhase = pcSlice->getPPS()->getResamplingPhase( refLayerId );

        assert( ( (widthEL  != widthBL)  || (resamplingPhase.phaseHorLuma == 0 && resamplingPhase.phaseHorChroma == 0) )
             && ( (heightEL != heightBL) || (resamplingPhase.phaseVerLuma == 0 && resamplingPhase.phaseVerChroma == 0) ) );

        m_pcPic->setMvScalingFactor( refLayerIdc, 
                                     widthEL  == widthBL  ? MV_SCALING_FACTOR_1X : Clip3(-4096, 4095, ((widthEL  << 8) + (widthBL  >> 1)) / widthBL), 
                                     heightEL == heightBL ? MV_SCALING_FACTOR_1X : Clip3(-4096, 4095, ((heightEL << 8) + (heightBL >> 1)) / heightBL) );

        m_pcPic->setPosScalingFactor( refLayerIdc, 
                                     ((widthBL  << 16) + (widthEL  >> 1)) / widthEL, 
                                     ((heightBL << 16) + (heightEL >> 1)) / heightEL );

        TComPicYuv* pBaseColRec = pcSlice->getBaseColPic(refLayerIdc)->getPicYuvRec();
#if CGS_3D_ASYMLUT 
        if( pcSlice->getPPS()->getCGSFlag() && m_c3DAsymLUTPPS.isRefLayer( pcSlice->getVPS()->getRefLayerId(m_layerId, refLayerIdc) ) )
        {
          assert( pcSlice->getBaseColPic( refLayerIdc )->getSlice( 0 )->getSPS()->getBitDepth(CHANNEL_TYPE_LUMA) == m_c3DAsymLUTPPS.getInputBitDepthY() );
          assert( pcSlice->getBaseColPic( refLayerIdc )->getSlice( 0 )->getSPS()->getBitDepth(CHANNEL_TYPE_CHROMA) == m_c3DAsymLUTPPS.getInputBitDepthC() );
          assert( pcSlice->getSPS()->getBitDepth(CHANNEL_TYPE_LUMA) >= m_c3DAsymLUTPPS.getOutputBitDepthY() );
          assert( pcSlice->getSPS()->getBitDepth(CHANNEL_TYPE_LUMA) >= m_c3DAsymLUTPPS.getOutputBitDepthC() );

          if( !m_pColorMappedPic )
          {
            initAsymLut(pcSlice->getBaseColPic(refLayerIdc)->getSlice(0));
          }

          m_c3DAsymLUTPPS.colorMapping( pcSlice->getBaseColPic(refLayerIdc)->getPicYuvRec(), m_pColorMappedPic );
          pBaseColRec = m_pColorMappedPic;
        }
#endif

        if( m_pcPic->requireResampling(refLayerIdc) )
        {
          // check for the sample prediction picture type
          if( pcSlice->getVPS()->isSamplePredictionType( pcSlice->getVPS()->getLayerIdxInVps(m_layerId), pcSlice->getVPS()->getLayerIdxInVps(refLayerId) ) )
          {
            m_cPrediction.upsampleBasePic( pcSlice, refLayerIdc, m_pcPic->getFullPelBaseRec(refLayerIdc), pBaseColRec, m_pcPic->getPicYuvRec(), pcSlice->getBaseColPic( refLayerIdc )->getSlice( 0 )->getSPS()->getBitDepth(CHANNEL_TYPE_LUMA), pcSlice->getBaseColPic( refLayerIdc )->getSlice( 0 )->getSPS()->getBitDepth(CHANNEL_TYPE_CHROMA));
          }
        }
        else
        {
          m_pcPic->setFullPelBaseRec( refLayerIdc, pcSlice->getBaseColPic(refLayerIdc)->getPicYuvRec() );
        }
        pcSlice->setFullPelBaseRec ( refLayerIdc, m_pcPic->getFullPelBaseRec(refLayerIdc) );
      }
    }

    if( m_layerId > 0 && pcSlice->getActiveNumILRRefIdx() )
    {
      for( Int i = 0; i < pcSlice->getActiveNumILRRefIdx(); i++ )
      {
        UInt refLayerIdc = pcSlice->getInterLayerPredLayerIdc(i);
#if AVC_BASE
        if( pcSlice->getVPS()->getRefLayerId( m_layerId, refLayerIdc ) == 0 && pcSlice->getVPS()->getNonHEVCBaseLayerFlag() )
        {
          pcSlice->setBaseColPic ( refLayerIdc, *m_ppcTDecTop[0]->getListPic()->begin() );
        }
        else
        {
          TDecTop *pcTDecTop = (TDecTop *)getRefLayerDec( refLayerIdc );
          TComList<TComPic*> *cListPic = pcTDecTop->getListPic();
          pcSlice->setBaseColPic ( *cListPic, refLayerIdc );
        }
#else
        TDecTop *pcTDecTop = (TDecTop *)getRefLayerDec( refLayerIdc );
        TComList<TComPic*> *cListPic = pcTDecTop->getListPic();
        pcSlice->setBaseColPic ( *cListPic, refLayerIdc );
#endif

        pcSlice->setFullPelBaseRec ( refLayerIdc, m_pcPic->getFullPelBaseRec(refLayerIdc) );
      }

      pcSlice->setILRPic( m_cIlpPic );

      pcSlice->setRefPicList( m_cListPic, false, m_cIlpPic);
    }
    else if ( m_layerId > 0 )
    {
      pcSlice->setRefPicList( m_cListPic, false, NULL);
    }

    // motion field mapping constraint
    if( pcSlice->getMFMEnabledFlag() )
    {
      TComPic* refPic = pcSlice->getRefPic( pcSlice->getSliceType() == B_SLICE ? ( RefPicList )( 1 - pcSlice->getColFromL0Flag() ) : REF_PIC_LIST_0 , pcSlice->getColRefIdx() );

      assert( refPic );

      Int refLayerId = refPic->getLayerId();

      if( refLayerId != pcSlice->getLayerId() )
      {
        TComPic* pColBasePic = pcSlice->getBaseColPic( *m_ppcTDecTop[refLayerId]->getListPic() );
        assert( pColBasePic->checkSameRefInfo() == true );
      }
    }
    
    if( m_layerId > 0 && pcSlice->getVPS()->getCrossLayerIrapAlignFlag() && ( !pcSlice->getVPS()->getSingleLayerForNonIrapFlag() || pcSlice->isIRAP() ) )
    {
      for(Int dependentLayerIdx = 0; dependentLayerIdx < pcSlice->getVPS()->getNumDirectRefLayers(m_layerId); dependentLayerIdx++)
      {
        TComList<TComPic*> *cListPic = getRefLayerDec( dependentLayerIdx )->getListPic();
        TComPic* refpicLayer = pcSlice->getRefPic(*cListPic, pcSlice->getPOC() );
        if(refpicLayer && pcSlice->isIRAP())
        {                 
          assert(pcSlice->getNalUnitType() == refpicLayer->getSlice(0)->getNalUnitType());
        }
      }
    }
    
    if( m_layerId > 0 && !pcSlice->isIntra() && pcSlice->getEnableTMVPFlag() )
    {
      TComPic* refPic = pcSlice->getRefPic(RefPicList(1 - pcSlice->getColFromL0Flag()), pcSlice->getColRefIdx());

      assert( refPic );
      assert ( refPic->getPicSym()->getSlice(0)->getAvailableForTMVPRefFlag() == true );

      // It is a requirement of bitstream conformance when the collocated picture, used for temporal motion vector prediction, is an inter-layer reference picture, 
      // VpsInterLayerMotionPredictionEnabled[ LayerIdxInVps[ currLayerId ] ][ LayerIdxInVps[ rLId ] ] shall be equal to 1, where rLId is set equal to nuh_layer_id of the inter-layer picture.
      if( refPic->isILR(pcSlice->getLayerId()) )
      {
        assert( pcSlice->getVPS()->isMotionPredictionType( pcSlice->getVPS()->getLayerIdxInVps(m_layerId), refPic->getLayerIdx() ) );
      }
    }
#endif //SVC_EXTENSION
    
    // For generalized B
    // note: maybe not existed case (always L0 is copied to L1 if L1 is empty)
    if (pcSlice->isInterB() && pcSlice->getNumRefIdx(REF_PIC_LIST_1) == 0)
    {
      Int iNumRefIdx = pcSlice->getNumRefIdx(REF_PIC_LIST_0);
      pcSlice->setNumRefIdx        ( REF_PIC_LIST_1, iNumRefIdx );

      for (Int iRefIdx = 0; iRefIdx < iNumRefIdx; iRefIdx++)
      {
        pcSlice->setRefPic(pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx), REF_PIC_LIST_1, iRefIdx);
      }
    }
    if (!pcSlice->isIntra())
    {
      Bool bLowDelay = true;
      Int  iCurrPOC  = pcSlice->getPOC();
      Int iRefIdx = 0;

      for (iRefIdx = 0; iRefIdx < pcSlice->getNumRefIdx(REF_PIC_LIST_0) && bLowDelay; iRefIdx++)
      {
        if ( pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx)->getPOC() > iCurrPOC )
        {
          bLowDelay = false;
        }
      }
      if (pcSlice->isInterB())
      {
        for (iRefIdx = 0; iRefIdx < pcSlice->getNumRefIdx(REF_PIC_LIST_1) && bLowDelay; iRefIdx++)
        {
          if ( pcSlice->getRefPic(REF_PIC_LIST_1, iRefIdx)->getPOC() > iCurrPOC )
          {
            bLowDelay = false;
          }
        }
      }

      pcSlice->setCheckLDC(bLowDelay);
    }

    //---------------
    pcSlice->setRefPOCList();
  }

  m_pcPic->setCurrSliceIdx(m_uiSliceIdx);
  if(pcSlice->getSPS()->getScalingListFlag())
  {
    TComScalingList scalingList;

#if SVC_EXTENSION
    if( pcSlice->getPPS()->getInferScalingListFlag() )
    {
      const TComPPS *refPps = m_ppcTDecTop[pcSlice->getPPS()->getScalingListRefLayerId()]->getParameterSetManager()->getActivePPS(); assert( refPps != NULL );
      scalingList = refPps->getScalingList();
    }
    else
#endif
    if(pcSlice->getPPS()->getScalingListPresentFlag())
    {
      scalingList = pcSlice->getPPS()->getScalingList();
    }
#if SVC_EXTENSION
    else if( pcSlice->getSPS()->getInferScalingListFlag() )
    {
      const TComSPS *refSps = m_ppcTDecTop[pcSlice->getSPS()->getScalingListRefLayerId()]->getParameterSetManager()->getActiveSPS(); assert( refSps != NULL );
      scalingList = refSps->getScalingList();
    }
#endif
    else if (pcSlice->getSPS()->getScalingListPresentFlag())
    {
      scalingList = pcSlice->getSPS()->getScalingList();
    }
    else
    {
      scalingList.setDefaultScalingList();
    }
    m_cTrQuant.setScalingListDec(scalingList);
    m_cTrQuant.setUseScalingList(true);
  }
  else
  {
    const Int maxLog2TrDynamicRange[MAX_NUM_CHANNEL_TYPE] =
    {
        pcSlice->getSPS()->getMaxLog2TrDynamicRange(CHANNEL_TYPE_LUMA),
        pcSlice->getSPS()->getMaxLog2TrDynamicRange(CHANNEL_TYPE_CHROMA)
    };
    m_cTrQuant.setFlatScalingList(maxLog2TrDynamicRange, pcSlice->getSPS()->getBitDepths());
    m_cTrQuant.setUseScalingList(false);
  }

  //  Decode a picture
  m_cGopDecoder.decompressSlice(&(nalu.getBitstream()), m_pcPic);

#if SVC_EXTENSION
  setFirstPicInLayerDecodedFlag(true);
  m_lastPicHasEos = false;
#endif

  m_bFirstSliceInPicture = false;
  m_uiSliceIdx++;

  return false;
}

Void TDecTop::xDecodeVPS(const std::vector<UChar> &naluData)
{
  TComVPS* vps = new TComVPS();
  //解码VPS
#if lmDecodeInfoOut
  cout << "decode vps" << "\n";
#endif
  m_cEntropyDecoder.decodeVPS( vps );
  m_parameterSetManager.storeVPS(vps, naluData);
#if SVC_EXTENSION
  checkValueOfTargetOutputLayerSetIdx(vps);

#if AVC_BASE
  if( vps->getNonHEVCBaseLayerFlag() )
  {
    m_ppcTDecTop[0]->getListPic()->pushBack( m_ppcTDecTop[0]->getBlPic() );

    if( !m_ppcTDecTop[0]->getBLReconFile()->good() )
    {
      printf( "Base layer YUV input reading error\n" );
      exit(EXIT_FAILURE);
    }        
  }  
#endif

  xDeriveSmallestLayerId(vps);
#endif

}

Void TDecTop::xDecodeSPS(const std::vector<UChar> &naluData)
{
  TComSPS* sps = new TComSPS();
#if lmDecodeInfoOut
  cout << "decode sps" <<"_layer_"<< m_layerId<< "\n";
#endif
#if O0043_BEST_EFFORT_DECODING
  sps->setForceDecodeBitDepth(m_forceDecodeBitDepth);
#endif
#if SVC_EXTENSION
  sps->setLayerId(m_layerId);
#endif
  m_cEntropyDecoder.decodeSPS( sps );
  m_parameterSetManager.storeSPS(sps, naluData);
}

#if CGS_3D_ASYMLUT
Void TDecTop::xDecodePPS( const std::vector<UChar> &naluData, TCom3DAsymLUT * pc3DAsymLUT )
#else
Void TDecTop::xDecodePPS(const std::vector<UChar> &naluData)
#endif
{
  TComPPS* pps = new TComPPS();
#if lmDecodeInfoOut
  cout << "decode pps" << "\n";
#endif
#if SVC_EXTENSION
  pps->setLayerId( m_layerId );
#endif
#if CGS_3D_ASYMLUT
  m_cEntropyDecoder.decodePPS( pps, pc3DAsymLUT, m_layerId );
#else
  m_cEntropyDecoder.decodePPS( pps );
#endif
  m_parameterSetManager.storePPS( pps, naluData);
}

#if SVC_EXTENSION
Bool TDecTop::decode(InputNALUnit& nalu, Int& iSkipFrame, Int& iPOCLastDisplay, UInt& curLayerId, Bool& bNewPOC)
#else
Bool TDecTop::decode(InputNALUnit& nalu, Int& iSkipFrame, Int& iPOCLastDisplay)
#endif
{
#if !SVC_EXTENSION
  // ignore all NAL units of layers > 0
  if (nalu.m_nuhLayerId > 0)
  {
    fprintf (stderr, "Warning: found NAL unit with nuh_layer_id equal to %d. Ignoring.\n", nalu.m_nuhLayerId);
    return false;
  }
#endif
  // Initialize entropy decoder
  m_cEntropyDecoder.setEntropyDecoder (&m_cCavlcDecoder);
  m_cEntropyDecoder.setBitstream      (&(nalu.getBitstream()));

#if SVC_EXTENSION
  // ignore any NAL units with nuh_layer_id == 63
  if( nalu.m_nuhLayerId == 63 )
  {
    return false;
  }

  // skip NAL units of the layers not needed to be decoded specified by OLS
  if( nalu.m_nalUnitType != NAL_UNIT_VPS && nalu.m_nalUnitType != NAL_UNIT_SPS && nalu.m_nalUnitType != NAL_UNIT_PPS &&
      m_targetDecLayerIdList.size() && std::find( m_targetDecLayerIdList.begin(), m_targetDecLayerIdList.end(), nalu.m_nuhLayerId ) == m_targetDecLayerIdList.end() )
  {
    return false;
  }
#endif
  switch (nalu.m_nalUnitType)
  {
    case NAL_UNIT_VPS:
#if SVC_EXTENSION
      assert( nalu.m_nuhLayerId == 0 ); // Non-conforming bitstream. The value of nuh_layer_id of VPS NAL unit shall be equal to 0.
#endif
      xDecodeVPS(nalu.getBitstream().getFifo());
#if SVC_EXTENSION
      m_isLastNALWasEos = false;    
#endif
      return false;

    case NAL_UNIT_SPS:
      xDecodeSPS(nalu.getBitstream().getFifo());
      return false;

    case NAL_UNIT_PPS:
#if CGS_3D_ASYMLUT
      xDecodePPS( nalu.getBitstream().getFifo(), &m_c3DAsymLUTPPS );
#else
      xDecodePPS(nalu.getBitstream().getFifo());
#endif
      return false;

    case NAL_UNIT_PREFIX_SEI:
      // Buffer up prefix SEI messages until SPS of associated VCL is known.
      m_prefixSEINALUs.push_back(new InputNALUnit(nalu));
      return false;

    case NAL_UNIT_SUFFIX_SEI:
#if SVC_EXTENSION
      if( nalu.m_nalUnitType == NAL_UNIT_SUFFIX_SEI )
      {
        assert( m_isLastNALWasEos == false );
      }
#endif
      if (m_pcPic)
      {
#if SVC_EXTENSION
        m_seiReader.parseSEImessage( &(nalu.getBitstream()), m_pcPic->getSEIs(), nalu.m_nalUnitType, m_parameterSetManager.getActiveVPS(), m_parameterSetManager.getActiveSPS(), m_pDecodedSEIOutputStream );
#else
        m_seiReader.parseSEImessage( &(nalu.getBitstream()), m_pcPic->getSEIs(), nalu.m_nalUnitType, m_parameterSetManager.getActiveSPS(), m_pDecodedSEIOutputStream );
#endif
      }
      else
      {
        printf ("Note: received suffix SEI but no picture currently active.\n");
      }
      return false;

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
#if SVC_EXTENSION
      if( nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_TRAIL_R || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_TRAIL_N ||
          nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_TSA_R || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_TSA_N ||
          nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_STSA_R || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_STSA_N ||
          nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_RADL_R || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_RADL_N ||
          nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_RASL_R || nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_RASL_N )
      {
        assert( m_isLastNALWasEos == false );
      }
      else
      {
        m_isLastNALWasEos = false;
      }

      return xDecodeSlice(nalu, iSkipFrame, iPOCLastDisplay, curLayerId, bNewPOC);
#else
      return xDecodeSlice(nalu, iSkipFrame, iPOCLastDisplay);
#endif
      break;

    case NAL_UNIT_EOS:
#if SVC_EXTENSION
      assert( m_isLastNALWasEos == false );

      m_isLastNALWasEos = true;
      m_lastPicHasEos = true;
#endif
      m_associatedIRAPType = NAL_UNIT_INVALID;
      m_pocCRA = 0;
      m_pocRandomAccess = MAX_INT;
      m_prevPOC = MAX_INT;
      m_prevSliceSkipped = false;
      m_skippedPOC = 0;
      return false;

    case NAL_UNIT_ACCESS_UNIT_DELIMITER:
      {
        AUDReader audReader;
        UInt picType;
        audReader.parseAccessUnitDelimiter(&(nalu.getBitstream()),picType);
        printf ("Note: found NAL_UNIT_ACCESS_UNIT_DELIMITER\n");
        return false;
      }

    case NAL_UNIT_EOB:
#if SVC_EXTENSION
      //Check layer id of the nalu. if it is not 0, give a warning message.
      if (nalu.m_nuhLayerId > 0)
      {
        printf( "\n\nThis bitstream is ended with EOB NALU that has layer id greater than 0\n" );
      }
#endif
      return false;

    case NAL_UNIT_FILLER_DATA:
      {
        FDReader fdReader;
        UInt size;
        fdReader.parseFillerData(&(nalu.getBitstream()),size);
        printf ("Note: found NAL_UNIT_FILLER_DATA with %u bytes payload.\n", size);
#if SVC_EXTENSION
        assert( m_isLastNALWasEos == false );
#endif
        return false;
      }

    case NAL_UNIT_RESERVED_VCL_N10:
    case NAL_UNIT_RESERVED_VCL_R11:
    case NAL_UNIT_RESERVED_VCL_N12:
    case NAL_UNIT_RESERVED_VCL_R13:
    case NAL_UNIT_RESERVED_VCL_N14:
    case NAL_UNIT_RESERVED_VCL_R15:

    case NAL_UNIT_RESERVED_IRAP_VCL22:
    case NAL_UNIT_RESERVED_IRAP_VCL23:

    case NAL_UNIT_RESERVED_VCL24:
    case NAL_UNIT_RESERVED_VCL25:
    case NAL_UNIT_RESERVED_VCL26:
    case NAL_UNIT_RESERVED_VCL27:
    case NAL_UNIT_RESERVED_VCL28:
    case NAL_UNIT_RESERVED_VCL29:
    case NAL_UNIT_RESERVED_VCL30:
    case NAL_UNIT_RESERVED_VCL31:
      printf ("Note: found reserved VCL NAL unit.\n");
      xParsePrefixSEIsForUnknownVCLNal();
      return false;

    case NAL_UNIT_RESERVED_NVCL41:
    case NAL_UNIT_RESERVED_NVCL42:
    case NAL_UNIT_RESERVED_NVCL43:
    case NAL_UNIT_RESERVED_NVCL44:
    case NAL_UNIT_RESERVED_NVCL45:
    case NAL_UNIT_RESERVED_NVCL46:
    case NAL_UNIT_RESERVED_NVCL47:
      printf ("Note: found reserved NAL unit.\n");
      return false;
    case NAL_UNIT_UNSPECIFIED_48:
    case NAL_UNIT_UNSPECIFIED_49:
    case NAL_UNIT_UNSPECIFIED_50:
    case NAL_UNIT_UNSPECIFIED_51:
    case NAL_UNIT_UNSPECIFIED_52:
    case NAL_UNIT_UNSPECIFIED_53:
    case NAL_UNIT_UNSPECIFIED_54:
    case NAL_UNIT_UNSPECIFIED_55:
    case NAL_UNIT_UNSPECIFIED_56:
    case NAL_UNIT_UNSPECIFIED_57:
    case NAL_UNIT_UNSPECIFIED_58:
    case NAL_UNIT_UNSPECIFIED_59:
    case NAL_UNIT_UNSPECIFIED_60:
    case NAL_UNIT_UNSPECIFIED_61:
    case NAL_UNIT_UNSPECIFIED_62:
    case NAL_UNIT_UNSPECIFIED_63:
      printf ("Note: found unspecified NAL unit.\n");
      return false;
    default:
      assert (0);
      break;
  }

  return false;
}

/** Function for checking if picture should be skipped because of association with a previous BLA picture
 * \param iPOCLastDisplay POC of last picture displayed
 * \returns true if the picture should be skipped
 * This function skips all TFD pictures that follow a BLA picture
 * in decoding order and precede it in output order.
 */
Bool TDecTop::isSkipPictureForBLA(Int& iPOCLastDisplay)
{
  if ((m_associatedIRAPType == NAL_UNIT_CODED_SLICE_BLA_N_LP || m_associatedIRAPType == NAL_UNIT_CODED_SLICE_BLA_W_LP || m_associatedIRAPType == NAL_UNIT_CODED_SLICE_BLA_W_RADL) &&
       m_apcSlicePilot->getPOC() < m_pocCRA && (m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_RASL_R || m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_RASL_N))
  {
    iPOCLastDisplay++;
    return true;
  }
  return false;
}

/** Function for checking if picture should be skipped because of random access
 * \param iSkipFrame skip frame counter
 * \param iPOCLastDisplay POC of last picture displayed
 * \returns true if the picture shold be skipped in the random access.
 * This function checks the skipping of pictures in the case of -s option random access.
 * All pictures prior to the random access point indicated by the counter iSkipFrame are skipped.
 * It also checks the type of Nal unit type at the random access point.
 * If the random access point is CRA/CRANT/BLA/BLANT, TFD pictures with POC less than the POC of the random access point are skipped.
 * If the random access point is IDR all pictures after the random access point are decoded.
 * If the random access point is none of the above, a warning is issues, and decoding of pictures with POC
 * equal to or greater than the random access point POC is attempted. For non IDR/CRA/BLA random
 * access point there is no guarantee that the decoder will not crash.
 */
Bool TDecTop::isRandomAccessSkipPicture(Int& iSkipFrame,  Int& iPOCLastDisplay)
{
  if (iSkipFrame)
  {
    iSkipFrame--;   // decrement the counter
    return true;
  }
  else if (m_pocRandomAccess == MAX_INT) // start of random access point, m_pocRandomAccess has not been set yet.
  {
    if (   m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_CRA
        || m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_BLA_W_LP
        || m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_BLA_N_LP
        || m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_BLA_W_RADL )
    {
      // set the POC random access since we need to skip the reordered pictures in the case of CRA/CRANT/BLA/BLANT.
      m_pocRandomAccess = m_apcSlicePilot->getPOC();
    }
    else if ( m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_W_RADL || m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_N_LP )
    {
      m_pocRandomAccess = -MAX_INT; // no need to skip the reordered pictures in IDR, they are decodable.
    }
    else
    {
      if(!m_warningMessageSkipPicture)
      {
        printf("\nWarning: this is not a valid random access point and the data is discarded until the first CRA picture");
        m_warningMessageSkipPicture = true;
      }
      return true;
    }
  }
  // skip the reordered pictures, if necessary
  else if (m_apcSlicePilot->getPOC() < m_pocRandomAccess && (m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_RASL_R || m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_RASL_N))
  {
    iPOCLastDisplay++;
    return true;
  }
  // if we reach here, then the picture is not skipped.
  return false;
}

#if SVC_EXTENSION
Void TDecTop::xInitILRP(const TComSPS *sps, const TComPPS *pps)
{
  if( m_layerId > 0 )
  {
    if (m_cIlpPic[0] == NULL)
    {
      for (Int j=0; j < m_numDirectRefLayers; j++)
      {
        m_cIlpPic[j] = new  TComPic;

#if REDUCED_ENCODER_MEMORY
        m_cIlpPic[j]->create(*sps, *pps, false, true, m_layerId);
#else
        m_cIlpPic[j]->create(*sps, *pps, true, m_layerId);
#endif

        for (Int i=0; i<m_cIlpPic[j]->getPicSym()->getNumberOfCtusInFrame(); i++)
        {
          m_cIlpPic[j]->getPicSym()->getCtu(i)->initCtu(m_cIlpPic[j], i);
        }
      }
    }
  }
}

TDecTop* TDecTop::getRefLayerDec( UInt refLayerIdx )
{
  const TComVPS* vps = m_parameterSetManager.getActiveVPS();
  if( vps->getNumDirectRefLayers( m_layerId ) <= 0 )
  {
    return (TDecTop *)getLayerDec( 0 );
  }
  
  return (TDecTop *)getLayerDec( vps->getRefLayerId( m_layerId, refLayerIdx ) );
}

Void TDecTop::setRefLayerParams( const TComVPS* vps )
{
#if CONFORMANCE_BITSTREAM_MODE
  for(UInt layerIdx = 0; layerIdx < MAX_VPS_LAYER_IDX_PLUS1; layerIdx++)
#else
  for(UInt layerIdx = 0; layerIdx <= m_commonDecoderParams->getTargetLayerId(); layerIdx++)
#endif
  {
    UInt layerId = vps->getLayerIdInNuh(layerIdx);
    TDecTop *decTop = m_ppcTDecTop[layerId];
    decTop->setNumSamplePredRefLayers(0);
    decTop->setNumMotionPredRefLayers(0);
    decTop->setNumDirectRefLayers(0);

    for(Int j = 0; j < layerIdx; j++)
    {
      if (vps->getDirectDependencyFlag(layerIdx, j))
      {
        decTop->setRefLayerId(decTop->getNumDirectRefLayers(), layerId);
        decTop->setNumDirectRefLayers(decTop->getNumDirectRefLayers() + 1);

        Int samplePredEnabledFlag = (vps->getDirectDependencyType(layerIdx, j) + 1) & 1;
        decTop->setNumSamplePredRefLayers(decTop->getNumSamplePredRefLayers() + samplePredEnabledFlag);

        Int motionPredEnabledFlag = ((vps->getDirectDependencyType(layerIdx, j) + 1) & 2) >> 1;
        decTop->setNumMotionPredRefLayers(decTop->getNumMotionPredRefLayers() + motionPredEnabledFlag);
      }
    }
  }
}

Void TDecTop::checkValueOfTargetOutputLayerSetIdx(TComVPS *vps)
{
  CommonDecoderParams* params = m_commonDecoderParams;

  if( params->getValueCheckedFlag() )
  {
    return; // Already checked
  }

  if( params->getTargetOutputLayerSetIdx() == -1 )  // Output layer set index not specified
  {
    if( params->getTargetLayerId() > vps->getMaxLayerId() )
    {
      printf( "Warning: specified target layerId %d is greater than max layerId %d. Target layerId is set equal to max layerId %d.\n", params->getTargetLayerId(), vps->getMaxLayerId(), vps->getMaxLayerId() );
      params->setTargetLayerId( vps->getMaxLayerId() );
    }

    Bool layerSetMatchFound = false;
    // Output layer set index not assigned.
    // Based on the value of targetLayerId, check if any of the output layer matches
    // Currently, the target layer ID in the encoder assumes that all the layers are decoded    
    // Check if any of the output layer sets match this description
    for( Int i = 0; i < vps->getNumOutputLayerSets(); i++ )
    {
      Bool layerSetMatchFlag = false;
      Int layerSetIdx = vps->getOutputLayerSetIdx( i );

      for( Int j = 0; j < vps->getNumLayersInIdList( layerSetIdx ); j++ )
      {
        if( vps->getLayerSetLayerIdList( layerSetIdx, j ) == params->getTargetLayerId() )
        {
          layerSetMatchFlag = true;
          break;
        }
      }
      
      if( layerSetMatchFlag ) // Potential output layer set candidate found
      {
        // If target dec layer ID list is also included - check if they match
        if( params->getTargetDecLayerIdSet() )
        {
          if( params->getTargetDecLayerIdSet()->size() )  
          {
            for( Int j = 0; j < vps->getNumLayersInIdList( layerSetIdx ); j++ )
            {
              if( *(params->getTargetDecLayerIdSet()->begin() + j) != vps->getLayerIdInNuh(vps->getLayerSetLayerIdList( layerSetIdx, j )))
              {
                layerSetMatchFlag = false;
              }
            }
          }
        }
        if( layerSetMatchFlag ) // The target dec layer ID list also matches, if present
        {
          // Match found
          layerSetMatchFound = true;
          params->setTargetOutputLayerSetIdx( i );
          params->setValueCheckedFlag( true );
          break;
        }
      }
    }
    assert( layerSetMatchFound ); // No output layer set matched the value of either targetLayerId or targetdeclayerIdlist
  }   
  else // Output layer set index is assigned - check if the values match
  {
    // Check if the target decoded layer is the highest layer in the list
    assert( params->getTargetOutputLayerSetIdx() < vps->getNumOutputLayerSets() );
#if !CONFORMANCE_BITSTREAM_MODE
    assert( params->getTargetOutputLayerSetIdx() < vps->getNumLayerSets() );
#endif
    Int layerSetIdx = vps->getOutputLayerSetIdx( params->getTargetOutputLayerSetIdx() );  // Index to the layer set
    
    // Check if the targetdeclayerIdlist matches the output layer set
    if( params->getTargetDecLayerIdSet() )
    {
      if( params->getTargetDecLayerIdSet()->size() )  
      {
        for(Int i = 0; i < vps->getNumLayersInIdList( layerSetIdx ); i++)
        {
          assert( *(params->getTargetDecLayerIdSet()->begin() + i) == vps->getLayerIdInNuh(vps->getLayerSetLayerIdList( layerSetIdx, i )));
        }
      }
    }
    params->setValueCheckedFlag( true );

  }

  // Set correct value of targetLayerId
  Int targetOlsIdx = params->getTargetOutputLayerSetIdx();
  Int targetLsIdx = vps->getOutputLayerSetIdx( targetOlsIdx );
  params->setTargetLayerId( vps->getLayerSetLayerIdList( targetLsIdx, vps->getNumLayersInIdList(targetLsIdx)-1 ) );

  // Check if the current layer is an output layer
  for( Int i = 0; i < vps->getNumLayersInIdList( targetLsIdx ); i++ )
  {
    if( vps->getOutputLayerFlag( targetOlsIdx, i ) )
    {
      m_ppcTDecTop[vps->getLayerSetLayerIdList( targetLsIdx, i )]->m_isOutputLayerFlag = true;
    }
  }
}

Void TDecTop::markAllPicsAsNoCurrAu( const TComVPS *vps )
{
  for(Int i = 0; i < MAX_LAYERS; i++)
  {
    TComList<TComPic*>* listPic = m_ppcTDecTop[vps->getLayerIdInNuh(i)]->getListPic();
    TComList<TComPic*>::iterator  iterPic = listPic->begin();
    while ( iterPic != listPic->end() )
    {
      TComPic *pcPic = *(iterPic);
      pcPic->setCurrAuFlag( false );
      iterPic++;
    }
  }
}

#if CGS_3D_ASYMLUT
Void TDecTop::initAsymLut(TComSlice *pcSlice)
{
  if( m_layerId > 0 )
  {
    if( !m_pColorMappedPic )
    {
      m_pColorMappedPic = new TComPicYuv;
      m_pColorMappedPic->create( pcSlice->getSPS()->getPicWidthInLumaSamples(), pcSlice->getSPS()->getPicHeightInLumaSamples(), pcSlice->getSPS()->getChromaFormatIdc(), pcSlice->getSPS()->getMaxCUWidth(), pcSlice->getSPS()->getMaxCUHeight(), pcSlice->getSPS()->getMaxTotalCUDepth(), true, NULL );
    }
  }
}
#endif

Void TDecTop::resetPocRestrictionCheckParameters()
{
  TDecTop::m_checkPocRestrictionsForCurrAu       = false;
  TDecTop::m_pocResetIdcOrCurrAu                 = -1;
  TDecTop::m_baseLayerIdrFlag                    = false;
  TDecTop::m_baseLayerPicPresentFlag             = false;
  TDecTop::m_baseLayerIrapFlag                   = false;
  TDecTop::m_nonBaseIdrPresentFlag               = false;
  TDecTop::m_nonBaseIdrType                      = -1;
  TDecTop::m_picNonIdrWithRadlPresentFlag        = false;
  TDecTop::m_picNonIdrNoLpPresentFlag            = false;
}

Void TDecTop::xCheckLayerReset()
{
  if (m_apcSlicePilot->isIRAP() && m_layerId > m_smallestLayerId)
  {
    Bool layerResetFlag;
    UInt dolLayerId;
    if (m_lastPicHasEos)
    {
      layerResetFlag = true;
      dolLayerId = m_layerId;
    }
    else if ((m_apcSlicePilot->isCRA() && m_apcSlicePilot->getHandleCraAsBlaFlag()) ||
      (m_apcSlicePilot->isIDR() && m_apcSlicePilot->getCrossLayerBLAFlag()) || m_apcSlicePilot->isBLA())
    {
      layerResetFlag = true;
      dolLayerId = m_layerId;
    }
    else
    {
      layerResetFlag = false;
    }

    if (layerResetFlag)
    {
      for (Int i = 0; i < m_apcSlicePilot->getVPS()->getNumPredictedLayers(dolLayerId); i++)
      {
        UInt iLayerId = m_apcSlicePilot->getVPS()->getPredictedLayerId(dolLayerId, i);
        m_ppcTDecTop[iLayerId]->m_layerInitializedFlag = false;
        m_ppcTDecTop[iLayerId]->m_firstPicInLayerDecodedFlag = false;
      }

      for (TComList<TComPic*>::iterator i = m_cListPic.begin(); i != m_cListPic.end(); i++)
      {
        if ((*i)->getPOC() != m_apcSlicePilot->getPOC())
        {
          (*i)->getSlice(0)->setReferenced(false);
        }
      }

      for (UInt i = 0; i < m_apcSlicePilot->getVPS()->getNumPredictedLayers(dolLayerId); i++)
      {
        UInt predLId = m_apcSlicePilot->getVPS()->getPredictedLayerId(dolLayerId, i);
        for (TComList<TComPic*>::iterator pic = m_ppcTDecTop[predLId]->getListPic()->begin(); pic != m_ppcTDecTop[predLId]->getListPic()->end(); pic++)
        {
          if ((*pic)->getSlice(0)->getPOC() != m_apcSlicePilot->getPOC())
          {
            (*pic)->getSlice(0)->setReferenced(false);
          }
        }
      }
    }
  }
}

Void TDecTop::xSetLayerInitializedFlag()
{
  if (m_apcSlicePilot->isIRAP() && m_apcSlicePilot->getNoRaslOutputFlag())
  {
    if (m_layerId == 0)
    {
      m_ppcTDecTop[m_layerId]->setLayerInitializedFlag(true);
    }
    else if (!m_ppcTDecTop[m_layerId]->getLayerInitializedFlag() && m_apcSlicePilot->getVPS()->getNumDirectRefLayers(m_layerId) == 0)
    {
      m_ppcTDecTop[m_layerId]->setLayerInitializedFlag(true);
    }
    else if (!m_ppcTDecTop[m_layerId]->getLayerInitializedFlag())
    {
      Bool refLayersInitialized = true;
      for (UInt j = 0; j < m_apcSlicePilot->getVPS()->getNumDirectRefLayers(m_layerId); j++)
      {
        UInt refLayerId = m_apcSlicePilot->getVPS()->getRefLayerId(m_layerId, j);
        if (!m_ppcTDecTop[refLayerId]->getLayerInitializedFlag())
        {
          refLayersInitialized = false;
        }
      }
      if (refLayersInitialized)
      {
        m_ppcTDecTop[m_layerId]->setLayerInitializedFlag(true);
      }
    }
  }
}

Void TDecTop::xDeriveSmallestLayerId(TComVPS* vps)
{
  UInt smallestLayerId;
  Int  targetOlsIdx = m_commonDecoderParams->getTargetOutputLayerSetIdx();
  assert( targetOlsIdx >= 0 );

  // list of layers to be decoded is not built yet
  m_targetDecLayerIdList.resize(0);

  UInt targetDecLayerSetIdx = vps->getOutputLayerSetIdx(targetOlsIdx);
  UInt lsIdx = targetDecLayerSetIdx;
  
  for (UInt i = 0; i < vps->getNumLayersInIdList(lsIdx); i++)
  {
    if (vps->getNecessaryLayerFlag(targetOlsIdx, i))
    {
      m_targetDecLayerIdList.push_back( vps->getLayerSetLayerIdList(lsIdx, i) );
    }
  }

  if (targetDecLayerSetIdx <= vps->getVpsNumLayerSetsMinus1())
  {
    smallestLayerId = 0;
  }
  else if (vps->getNumLayersInIdList(targetDecLayerSetIdx) == 1)
  {
    smallestLayerId = 0;
  }
  else
  {
    smallestLayerId = m_targetDecLayerIdList[0];
  }

  for( UInt layerId = 0; layerId < MAX_VPS_LAYER_IDX_PLUS1; layerId++ )
  {
    m_ppcTDecTop[layerId]->m_smallestLayerId = smallestLayerId;
  }
}

Void TDecTop::xSetRequireResamplingFlag( const TComVPS &vps, const TComSPS &sps, const TComPPS &pps, TComPic* pic )
{
  for(UInt i = 0; i < vps.getNumDirectRefLayers( m_layerId ); i++ )
  {
    const Window scalEL = pps.getScaledRefLayerWindowForLayer(vps.getRefLayerId(m_layerId, i));
    const Window refEL = pps.getRefLayerWindowForLayer(vps.getRefLayerId(m_layerId, i));
    Bool equalOffsets = scalEL.hasEqualOffset(refEL);
    Bool zeroPhase = pps.hasZeroResamplingPhase(vps.getRefLayerId(m_layerId, i));

    TDecTop *pcTDecTopBase = (TDecTop *)getRefLayerDec( i );
    TComPicYuv* pcPicYuvRecBase = (*(pcTDecTopBase->getListPic()->begin()))->getPicYuvRec();

    const Int bitDepthLuma = sps.getBitDepth(CHANNEL_TYPE_LUMA);
    const Int bitDepthChroma = sps.getBitDepth(CHANNEL_TYPE_CHROMA);
    const Int refBitDepthLuma = (*(pcTDecTopBase->getListPic()->begin()))->getSlice(0)->getSPS()->getBitDepth(CHANNEL_TYPE_LUMA);
    const Int refBitDepthChroma = (*(pcTDecTopBase->getListPic()->begin()))->getSlice(0)->getSPS()->getBitDepth(CHANNEL_TYPE_CHROMA);
    
    Bool sameBitDepths = ( bitDepthLuma == refBitDepthLuma ) && ( bitDepthChroma == refBitDepthChroma );

    if( pcPicYuvRecBase->getWidth(COMPONENT_Y) == sps.getPicWidthInLumaSamples() && pcPicYuvRecBase->getHeight(COMPONENT_Y) == sps.getPicHeightInLumaSamples() && equalOffsets && zeroPhase )
    {
      pic->setEqualPictureSizeAndOffsetFlag( i, true );
    }

    if( !pic->equalPictureSizeAndOffsetFlag(i) || !sameBitDepths 
#if CGS_3D_ASYMLUT
      || pps.getCGSFlag() > 0
#endif
      )
    {
      pic->setRequireResamplingFlag( i, true );

      //only for scalable extension
      assert( vps.getScalabilityMask( SCALABILITY_ID ) == true );
    }
  }
}

#endif //SVC_EXTENSION


//! \}
