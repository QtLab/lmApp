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

/** \file     TEncGOP.cpp
    \brief    GOP encoder class
*/

#include <list>
#include <algorithm>
#include <functional>

#include "TEncTop.h"
#include "TEncGOP.h"
#include "TEncAnalyze.h"
#include "libmd5/MD5.h"
#include "TLibCommon/SEI.h"
#include "TLibCommon/NAL.h"
#include "NALwrite.h"
#include <time.h>
#include <math.h>

#include <deque>
using namespace std;

#if SVC_EXTENSION
#include <limits.h>
Bool TEncGOP::m_signalledVPS = false;
#endif


//! \ingroup TLibEncoder
//! \{

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================
Int getLSB(Int poc, Int maxLSB)
{
  if (poc >= 0)
  {
    return poc % maxLSB;
  }
  else
  {
    return (maxLSB - ((-poc) % maxLSB)) % maxLSB;
  }
}

TEncGOP::TEncGOP()
{
  m_iLastIDR            = 0;
  m_iGopSize            = 0;
  m_iNumPicCoded        = 0; //Niko
  m_bFirst              = true;
  m_iLastRecoveryPicPOC = 0;

  m_pcCfg               = NULL;
  m_pcSliceEncoder      = NULL;
  m_pcListPic           = NULL;

  m_pcEntropyCoder      = NULL;
  m_pcCavlcCoder        = NULL;
  m_pcSbacCoder         = NULL;
  m_pcBinCABAC          = NULL;

  m_bSeqFirst           = true;

  m_bRefreshPending     = 0;
  m_pocCRA            = 0;
  m_numLongTermRefPicSPS = 0;
  ::memset(m_ltRefPicPocLsbSps, 0, sizeof(m_ltRefPicPocLsbSps));
  ::memset(m_ltRefPicUsedByCurrPicFlag, 0, sizeof(m_ltRefPicUsedByCurrPicFlag));
  m_lastBPSEI         = 0;
  m_bufferingPeriodSEIPresentInAU = false;
  m_associatedIRAPType = NAL_UNIT_CODED_SLICE_IDR_N_LP;
  m_associatedIRAPPOC  = 0;
#if W0038_DB_OPT
  m_pcDeblockingTempPicYuv = NULL;
#endif
#if SVC_EXTENSION
  m_pocCraWithoutReset = 0;
  m_associatedIrapPocBeforeReset = 0;
  m_pcPredSearch        = NULL;
#if CGS_3D_ASYMLUT
  m_temp = NULL;
  m_pColorMappedPic = NULL;
#endif
  m_lastPocPeriodId = -1;
  m_noRaslOutputFlag = false;
  m_prevPicHasEos    = false;
#endif //SVC_EXTENSION
  return;
}

TEncGOP::~TEncGOP()
{
#if CGS_3D_ASYMLUT
  if(m_pColorMappedPic)
  {
    m_pColorMappedPic->destroy();
    delete m_pColorMappedPic;
    m_pColorMappedPic = NULL;                
  }
  if(m_temp)
  {
    xDestroy2DArray(m_temp, m_cgsFilterLength>>1, 0);
    m_temp = NULL;
  }
#endif
}

/** Create list to contain pointers to CTU start addresses of slice.
 */
#if SVC_EXTENSION
Void  TEncGOP::create( UInt layerId )
{
  m_bLongtermTestPictureHasBeenCoded = 0;
  m_bLongtermTestPictureHasBeenCoded2 = 0;
  m_layerId = layerId;
}
#else
Void  TEncGOP::create()
{
  m_bLongtermTestPictureHasBeenCoded = 0;
  m_bLongtermTestPictureHasBeenCoded2 = 0;
}
#endif

Void  TEncGOP::destroy()
{
#if W0038_DB_OPT
  if (m_pcDeblockingTempPicYuv)
  {
    m_pcDeblockingTempPicYuv->destroy();
    delete m_pcDeblockingTempPicYuv;
    m_pcDeblockingTempPicYuv = NULL;
  }
#endif
}

Void TEncGOP::init ( TEncTop* pcTEncTop )
{
  m_pcEncTop     = pcTEncTop;
  m_pcCfg                = pcTEncTop;
  m_seiEncoder.init(m_pcCfg, pcTEncTop, this);
  m_pcSliceEncoder       = pcTEncTop->getSliceEncoder();
  m_pcListPic            = pcTEncTop->getListPic();

  m_pcEntropyCoder       = pcTEncTop->getEntropyCoder();
  m_pcCavlcCoder         = pcTEncTop->getCavlcCoder();
  m_pcSbacCoder          = pcTEncTop->getSbacCoder();
  m_pcBinCABAC           = pcTEncTop->getBinCABAC();
  m_pcLoopFilter         = pcTEncTop->getLoopFilter();

  m_pcSAO                = pcTEncTop->getSAO();
  m_pcRateCtrl           = pcTEncTop->getRateCtrl();
  m_lastBPSEI          = 0;
  m_totalCoded         = 0;

#if SVC_EXTENSION
  m_ppcTEncTop           = pcTEncTop->getLayerEnc();
  m_pcPredSearch         = pcTEncTop->getPredSearch();                       ///< encoder search class
#if CGS_3D_ASYMLUT
  if( pcTEncTop->getLayerId() )
  {
    UInt prevLayerIdx = 0;

    if( pcTEncTop->getNumActiveRefLayers() > 0 )
    {
      prevLayerIdx = pcTEncTop->getPredLayerIdx( pcTEncTop->getNumActiveRefLayers() - 1);
    }

    const TComSPS *sps = pcTEncTop->getSPS();

    const Int bitDepthLuma = sps->getBitDepth(CHANNEL_TYPE_LUMA);
    const Int bitDepthChroma = sps->getBitDepth(CHANNEL_TYPE_CHROMA);
    const Int prevBitDepthLuma = m_ppcTEncTop[prevLayerIdx]->getSPS()->getBitDepth(CHANNEL_TYPE_LUMA);
    const Int prevBitDepthChroma = m_ppcTEncTop[prevLayerIdx]->getSPS()->getBitDepth(CHANNEL_TYPE_CHROMA);

    m_Enc3DAsymLUTPicUpdate.create( m_pcCfg->getCGSMaxOctantDepth() , prevBitDepthLuma, prevBitDepthChroma, bitDepthLuma, bitDepthChroma , m_pcCfg->getCGSMaxYPartNumLog2() );
    m_Enc3DAsymLUTPPS.create( m_pcCfg->getCGSMaxOctantDepth(), prevBitDepthLuma, prevBitDepthChroma, bitDepthLuma, bitDepthChroma, m_pcCfg->getCGSMaxYPartNumLog2() );

    if(!m_pColorMappedPic)
    {
      m_pColorMappedPic = new TComPicYuv;
      m_pColorMappedPic->create( m_ppcTEncTop[0]->getSourceWidth(), m_ppcTEncTop[0]->getSourceHeight(), m_ppcTEncTop[0]->getChromaFormatIDC(), sps->getMaxCUWidth(), sps->getMaxCUHeight(), sps->getMaxTotalCUDepth(), true, NULL );
    }
  }
#endif
#endif //SVC_EXTENSION
}

Int TEncGOP::xWriteVPS (AccessUnit &accessUnit, const TComVPS *vps)
{
#if SVC_EXTENSION
  if( m_signalledVPS )
  {
    return 0;
  }
  m_signalledVPS = true;
#endif
  OutputNALUnit nalu(NAL_UNIT_VPS);
  m_pcEntropyCoder->setBitstream(&nalu.m_Bitstream);
  m_pcEntropyCoder->encodeVPS(vps);
  accessUnit.push_back(new NALUnitEBSP(nalu));
  return (Int)(accessUnit.back()->m_nalUnitData.str().size()) * 8;
}

Int TEncGOP::xWriteSPS (AccessUnit &accessUnit, const TComSPS *sps)
{
#if SVC_EXTENSION
  OutputNALUnit nalu(NAL_UNIT_SPS, 0, m_layerId);

  if (m_pcEncTop->getVPS()->getNumDirectRefLayers(m_layerId) == 0 && m_pcEncTop->getVPS()->getNumAddLayerSets() > 0)
  {
    nalu.m_nuhLayerId = 0; // For independent base layer rewriting
  }

  // dependency constraint
  assert( sps->getLayerId() == 0 || sps->getLayerId() == m_layerId || m_pcEncTop->getVPS()->getRecursiveRefLayerFlag(m_layerId, sps->getLayerId()) );
#else
  OutputNALUnit nalu(NAL_UNIT_SPS);
#endif
  m_pcEntropyCoder->setBitstream(&nalu.m_Bitstream);
  m_pcEntropyCoder->encodeSPS(sps);
  accessUnit.push_back(new NALUnitEBSP(nalu));
  return (Int)(accessUnit.back()->m_nalUnitData.str().size()) * 8;

}

Int TEncGOP::xWritePPS (AccessUnit &accessUnit, const TComPPS *pps)
{
#if SVC_EXTENSION
  OutputNALUnit nalu(NAL_UNIT_PPS, 0, m_layerId);

  if( m_pcEncTop->getVPS()->getNumDirectRefLayers(m_layerId) == 0 && m_pcEncTop->getVPS()->getNumAddLayerSets() > 0 )
  {
    // For independent base layer rewriting
    nalu.m_nuhLayerId = 0;
  }

  // dependency constraint
  assert( pps->getLayerId() == 0 || pps->getLayerId() == m_layerId || m_pcEncTop->getVPS()->getRecursiveRefLayerFlag(m_layerId, pps->getLayerId()) );
#else
  OutputNALUnit nalu(NAL_UNIT_PPS);
#endif
  m_pcEntropyCoder->setBitstream(&nalu.m_Bitstream);
#if SVC_EXTENSION && CGS_3D_ASYMLUT
  m_pcEntropyCoder->encodePPS(pps, &m_Enc3DAsymLUTPPS);
#else
  m_pcEntropyCoder->encodePPS(pps);
#endif  
  accessUnit.push_back(new NALUnitEBSP(nalu));
  return (Int)(accessUnit.back()->m_nalUnitData.str().size()) * 8;
}


Int TEncGOP::xWriteParameterSets (AccessUnit &accessUnit, TComSlice *slice)
{
  Int actualTotalBits = 0;

  actualTotalBits += xWriteVPS(accessUnit, m_pcEncTop->getVPS());
  actualTotalBits += xWriteSPS(accessUnit, slice->getSPS());
  actualTotalBits += xWritePPS(accessUnit, slice->getPPS());

  return actualTotalBits;
}

Void TEncGOP::xWriteAccessUnitDelimiter (AccessUnit &accessUnit, TComSlice *slice)
{
  AUDWriter audWriter;
  OutputNALUnit nalu(NAL_UNIT_ACCESS_UNIT_DELIMITER);

  Int picType = slice->isIntra() ? 0 : (slice->isInterP() ? 1 : 2);

  audWriter.codeAUD(nalu.m_Bitstream, picType);
  accessUnit.push_front(new NALUnitEBSP(nalu));
}

// write SEI list into one NAL unit and add it to the Access unit at auPos
#if O0164_MULTI_LAYER_HRD
Void TEncGOP::xWriteSEI (NalUnitType naluType, SEIMessages& seiMessages, AccessUnit &accessUnit, AccessUnit::iterator &auPos, Int temporalId, const TComVPS *vps, const TComSPS *sps, const SEIScalableNesting* nestingSei, const SEIBspNesting* bspNestingSei)
#else
Void TEncGOP::xWriteSEI (NalUnitType naluType, SEIMessages& seiMessages, AccessUnit &accessUnit, AccessUnit::iterator &auPos, Int temporalId, const TComSPS *sps)
#endif
{
  // don't do anything, if we get an empty list
  if (seiMessages.empty())
  {
    return;
  }
#if O0164_MULTI_LAYER_HRD
  OutputNALUnit nalu(naluType, temporalId, sps->getLayerId());
  m_seiWriter.writeSEImessages(nalu.m_Bitstream, seiMessages, vps, sps, false, nestingSei, bspNestingSei);
#else
  OutputNALUnit nalu(naluType, temporalId);
  m_seiWriter.writeSEImessages(nalu.m_Bitstream, seiMessages, sps, false);
#endif  
  auPos = accessUnit.insert(auPos, new NALUnitEBSP(nalu));
  auPos++;
}

#if O0164_MULTI_LAYER_HRD
Void TEncGOP::xWriteSEISeparately (NalUnitType naluType, SEIMessages& seiMessages, AccessUnit &accessUnit, AccessUnit::iterator &auPos, Int temporalId, const TComVPS *vps, const TComSPS *sps, const SEIScalableNesting* nestingSei, const SEIBspNesting* bspNestingSei)
#else
Void TEncGOP::xWriteSEISeparately (NalUnitType naluType, SEIMessages& seiMessages, AccessUnit &accessUnit, AccessUnit::iterator &auPos, Int temporalId, const TComSPS *sps)
#endif
{
  // don't do anything, if we get an empty list
  if (seiMessages.empty())
  {
    return;
  }
  for (SEIMessages::const_iterator sei = seiMessages.begin(); sei!=seiMessages.end(); sei++ )
  {
    SEIMessages tmpMessages;
    tmpMessages.push_back(*sei);
#if O0164_MULTI_LAYER_HRD
    OutputNALUnit nalu(naluType, temporalId, sps->getLayerId());
    m_seiWriter.writeSEImessages(nalu.m_Bitstream, tmpMessages, vps, sps, false, nestingSei, bspNestingSei);
#else
    OutputNALUnit nalu(naluType, temporalId);
    m_seiWriter.writeSEImessages(nalu.m_Bitstream, tmpMessages, sps, false);
#endif
    auPos = accessUnit.insert(auPos, new NALUnitEBSP(nalu));
    auPos++;
  }
}

Void TEncGOP::xClearSEIs(SEIMessages& seiMessages, Bool deleteMessages)
{
  if (deleteMessages)
  {
    deleteSEIs(seiMessages);
  }
  else
  {
    seiMessages.clear();
  }
}

// write SEI messages as separate NAL units ordered
#if O0164_MULTI_LAYER_HRD
Void TEncGOP::xWriteLeadingSEIOrdered (SEIMessages& seiMessages, SEIMessages& duInfoSeiMessages, AccessUnit &accessUnit, Int temporalId,const TComVPS *vps, const TComSPS *sps, Bool testWrite, const SEIScalableNesting* nestingSei, const SEIBspNesting* bspNestingSei)
#else
Void TEncGOP::xWriteLeadingSEIOrdered (SEIMessages& seiMessages, SEIMessages& duInfoSeiMessages, AccessUnit &accessUnit, Int temporalId, const TComSPS *sps, Bool testWrite)
#endif
{
  AccessUnit::iterator itNalu = accessUnit.begin();

  while ( (itNalu!=accessUnit.end())&&
    ( (*itNalu)->m_nalUnitType==NAL_UNIT_ACCESS_UNIT_DELIMITER 
    || (*itNalu)->m_nalUnitType==NAL_UNIT_VPS
    || (*itNalu)->m_nalUnitType==NAL_UNIT_SPS
    || (*itNalu)->m_nalUnitType==NAL_UNIT_PPS
    ))
  {
    itNalu++;
  }

  SEIMessages localMessages = seiMessages;
  SEIMessages currentMessages;
  
#if ENC_DEC_TRACE
  g_HLSTraceEnable = !testWrite;
#endif
  // The case that a specific SEI is not present is handled in xWriteSEI (empty list)

  // Active parameter sets SEI must always be the first SEI
  currentMessages = extractSeisByType(localMessages, SEI::ACTIVE_PARAMETER_SETS);
  assert (currentMessages.size() <= 1);
#if O0164_MULTI_LAYER_HRD
  xWriteSEI(NAL_UNIT_PREFIX_SEI, currentMessages, accessUnit, itNalu, temporalId, vps, sps, nestingSei, bspNestingSei);
#else
  xWriteSEI(NAL_UNIT_PREFIX_SEI, currentMessages, accessUnit, itNalu, temporalId, sps);
#endif
  xClearSEIs(currentMessages, !testWrite);
  
  // Buffering period SEI must always be following active parameter sets
  currentMessages = extractSeisByType(localMessages, SEI::BUFFERING_PERIOD);
  assert (currentMessages.size() <= 1);
#if O0164_MULTI_LAYER_HRD
  xWriteSEI(NAL_UNIT_PREFIX_SEI, currentMessages, accessUnit, itNalu, temporalId, vps, sps, nestingSei, bspNestingSei);
#else
  xWriteSEI(NAL_UNIT_PREFIX_SEI, currentMessages, accessUnit, itNalu, temporalId, sps);
#endif
  xClearSEIs(currentMessages, !testWrite);

  // Picture timing SEI must always be following buffering period
  currentMessages = extractSeisByType(localMessages, SEI::PICTURE_TIMING);
  assert (currentMessages.size() <= 1);
#if O0164_MULTI_LAYER_HRD
  xWriteSEI(NAL_UNIT_PREFIX_SEI, currentMessages, accessUnit, itNalu, temporalId, vps, sps, nestingSei, bspNestingSei);
#else
  xWriteSEI(NAL_UNIT_PREFIX_SEI, currentMessages, accessUnit, itNalu, temporalId, sps);
#endif
  xClearSEIs(currentMessages, !testWrite);

  // Decoding unit info SEI must always be following picture timing
  if (!duInfoSeiMessages.empty())
  {
    currentMessages.push_back(duInfoSeiMessages.front());
    if (!testWrite)
    {
      duInfoSeiMessages.pop_front();
    }
#if O0164_MULTI_LAYER_HRD
    xWriteSEI(NAL_UNIT_PREFIX_SEI, currentMessages, accessUnit, itNalu, temporalId, vps, sps, nestingSei, bspNestingSei);
#else
    xWriteSEI(NAL_UNIT_PREFIX_SEI, currentMessages, accessUnit, itNalu, temporalId, sps);
#endif
    xClearSEIs(currentMessages, !testWrite);
  }

  // Scalable nesting SEI must always be the following DU info
  currentMessages = extractSeisByType(localMessages, SEI::SCALABLE_NESTING);
#if O0164_MULTI_LAYER_HRD
  xWriteSEISeparately(NAL_UNIT_PREFIX_SEI, currentMessages, accessUnit, itNalu, temporalId, vps, sps, nestingSei, bspNestingSei);
#else
  xWriteSEISeparately(NAL_UNIT_PREFIX_SEI, currentMessages, accessUnit, itNalu, temporalId, sps);
#endif
  xClearSEIs(currentMessages, !testWrite);

  // And finally everything else one by one
#if O0164_MULTI_LAYER_HRD
  xWriteSEISeparately(NAL_UNIT_PREFIX_SEI, localMessages, accessUnit, itNalu, temporalId, vps, sps, nestingSei, bspNestingSei);
#else
  xWriteSEISeparately(NAL_UNIT_PREFIX_SEI, localMessages, accessUnit, itNalu, temporalId, sps);
#endif
  xClearSEIs(localMessages, !testWrite);

  if (!testWrite)
  {
    seiMessages.clear();
  }
}

#if O0164_MULTI_LAYER_HRD
Void TEncGOP::xWriteLeadingSEIMessages (SEIMessages& seiMessages, SEIMessages& duInfoSeiMessages, AccessUnit &accessUnit, Int temporalId,const TComVPS *vps, const TComSPS *sps, std::deque<DUData> &duData, const SEIScalableNesting* nestingSei, const SEIBspNesting* bspNestingSei)
#else
Void TEncGOP::xWriteLeadingSEIMessages (SEIMessages& seiMessages, SEIMessages& duInfoSeiMessages, AccessUnit &accessUnit, Int temporalId, const TComSPS *sps, std::deque<DUData> &duData)
#endif
{
  AccessUnit testAU;
  SEIMessages picTimingSEIs = getSeisByType(seiMessages, SEI::PICTURE_TIMING);
  assert (picTimingSEIs.size() < 2);
  SEIPictureTiming * picTiming = picTimingSEIs.empty() ? NULL : (SEIPictureTiming*) picTimingSEIs.front();

  // test writing
#if O0164_MULTI_LAYER_HRD
  xWriteLeadingSEIOrdered(seiMessages, duInfoSeiMessages, testAU, temporalId, vps, sps, true, nestingSei, bspNestingSei);
#else
  xWriteLeadingSEIOrdered(seiMessages, duInfoSeiMessages, testAU, temporalId, sps, true);
#endif
  // update Timing and DU info SEI
  xUpdateDuData(testAU, duData);
  xUpdateTimingSEI(picTiming, duData, sps);
  xUpdateDuInfoSEI(duInfoSeiMessages, picTiming);
  // actual writing
#if O0164_MULTI_LAYER_HRD
  xWriteLeadingSEIOrdered(seiMessages, duInfoSeiMessages, accessUnit, temporalId, vps, sps, false, nestingSei, bspNestingSei);
#else
  xWriteLeadingSEIOrdered(seiMessages, duInfoSeiMessages, accessUnit, temporalId, sps, false);
#endif

  // testAU will automatically be cleaned up when losing scope
}

#if O0164_MULTI_LAYER_HRD
Void TEncGOP::xWriteTrailingSEIMessages (SEIMessages& seiMessages, AccessUnit &accessUnit, Int temporalId, const TComVPS *vps, const TComSPS *sps, const SEIScalableNesting* nestingSei, const SEIBspNesting* bspNestingSei)
#else
Void TEncGOP::xWriteTrailingSEIMessages (SEIMessages& seiMessages, AccessUnit &accessUnit, Int temporalId, const TComSPS *sps)
#endif
{
  // Note: using accessUnit.end() works only as long as this function is called after slice coding and before EOS/EOB NAL units
  AccessUnit::iterator pos = accessUnit.end();
#if O0164_MULTI_LAYER_HRD
  xWriteSEISeparately(NAL_UNIT_SUFFIX_SEI, seiMessages, accessUnit, pos, temporalId, vps, sps, nestingSei, bspNestingSei);
#else
  xWriteSEISeparately(NAL_UNIT_SUFFIX_SEI, seiMessages, accessUnit, pos, temporalId, sps);
#endif
  deleteSEIs(seiMessages);
}

#if O0164_MULTI_LAYER_HRD
Void TEncGOP::xWriteDuSEIMessages (SEIMessages& duInfoSeiMessages, AccessUnit &accessUnit, Int temporalId, const TComVPS *vps, const TComSPS *sps, std::deque<DUData> &duData, const SEIScalableNesting* nestingSei, const SEIBspNesting* bspNestingSei)
#else
Void TEncGOP::xWriteDuSEIMessages (SEIMessages& duInfoSeiMessages, AccessUnit &accessUnit, Int temporalId, const TComSPS *sps, std::deque<DUData> &duData)
#endif
{
  const TComHRD *hrd = sps->getVuiParameters()->getHrdParameters();

  if( m_pcCfg->getDecodingUnitInfoSEIEnabled() && hrd->getSubPicCpbParamsPresentFlag() )
  {
    Int naluIdx = 0;
    AccessUnit::iterator nalu = accessUnit.begin();

    // skip over first DU, we have a DU info SEI there already
    while (naluIdx < duData[0].accumNalsDU && nalu!=accessUnit.end())
    {
      naluIdx++;
      nalu++;
    }

    SEIMessages::iterator duSEI = duInfoSeiMessages.begin();
    // loop over remaining DUs
    for (Int duIdx = 1; duIdx < duData.size(); duIdx++)
    {
      if (duSEI == duInfoSeiMessages.end())
      {
        // if the number of generated SEIs matches the number of DUs, this should not happen
        assert (false);
        return;
      }
      // write the next SEI
      SEIMessages tmpSEI;
      tmpSEI.push_back(*duSEI);
#if O0164_MULTI_LAYER_HRD
      xWriteSEI(NAL_UNIT_PREFIX_SEI, tmpSEI, accessUnit, nalu, temporalId, vps, sps, nestingSei, bspNestingSei);
#else
      xWriteSEI(NAL_UNIT_PREFIX_SEI, tmpSEI, accessUnit, nalu, temporalId, sps);
#endif
      // nalu points to the position after the SEI, so we have to increase the index as well
      naluIdx++;
      while ((naluIdx < duData[duIdx].accumNalsDU) && nalu!=accessUnit.end())
      {
        naluIdx++;
        nalu++;
      }
      duSEI++;
    }
  }
  deleteSEIs(duInfoSeiMessages);
}


Void TEncGOP::xCreateIRAPLeadingSEIMessages (SEIMessages& seiMessages, const TComSPS *sps, const TComPPS *pps)
{
  OutputNALUnit nalu(NAL_UNIT_PREFIX_SEI);

#if R0247_SEI_ACTIVE
  if(m_pcCfg->getActiveParameterSetsSEIEnabled() && m_layerId == 0 )
#else
  if(m_pcCfg->getActiveParameterSetsSEIEnabled())
#endif
  {
    SEIActiveParameterSets *sei = new SEIActiveParameterSets;
    m_seiEncoder.initSEIActiveParameterSets (sei, m_pcCfg->getVPS(), sps);
    seiMessages.push_back(sei);
  }

  if(m_pcCfg->getFramePackingArrangementSEIEnabled())
  {
    SEIFramePacking *sei = new SEIFramePacking;
    m_seiEncoder.initSEIFramePacking (sei, m_iNumPicCoded);
    seiMessages.push_back(sei);
  }

  if(m_pcCfg->getSegmentedRectFramePackingArrangementSEIEnabled())
  {
    SEISegmentedRectFramePacking *sei = new SEISegmentedRectFramePacking;
    m_seiEncoder.initSEISegmentedRectFramePacking(sei);
    seiMessages.push_back(sei);
  }

  if (m_pcCfg->getDisplayOrientationSEIAngle())
  {
    SEIDisplayOrientation *sei = new SEIDisplayOrientation;
    m_seiEncoder.initSEIDisplayOrientation(sei);
    seiMessages.push_back(sei);
  }

  if(m_pcCfg->getToneMappingInfoSEIEnabled())
  {
    SEIToneMappingInfo *sei = new SEIToneMappingInfo;
    m_seiEncoder.initSEIToneMappingInfo (sei);
    seiMessages.push_back(sei);
  }

  if(m_pcCfg->getTMCTSSEIEnabled())
  {
    SEITempMotionConstrainedTileSets *sei = new SEITempMotionConstrainedTileSets;
    m_seiEncoder.initSEITempMotionConstrainedTileSets(sei, pps);
    seiMessages.push_back(sei);
  }

  if(m_pcCfg->getTimeCodeSEIEnabled())
  {
    SEITimeCode *seiTimeCode = new SEITimeCode;
    m_seiEncoder.initSEITimeCode(seiTimeCode);
    seiMessages.push_back(seiTimeCode);
  }

  if(m_pcCfg->getKneeSEIEnabled())
  {
    SEIKneeFunctionInfo *sei = new SEIKneeFunctionInfo;
    m_seiEncoder.initSEIKneeFunctionInfo(sei);
    seiMessages.push_back(sei);
  }
    
  if(m_pcCfg->getMasteringDisplaySEI().colourVolumeSEIEnabled)
  {
    const TComSEIMasteringDisplay &seiCfg=m_pcCfg->getMasteringDisplaySEI();
    SEIMasteringDisplayColourVolume *sei = new SEIMasteringDisplayColourVolume;
    sei->values = seiCfg;
    seiMessages.push_back(sei);
  }
  if(m_pcCfg->getChromaResamplingFilterHintEnabled())
  {
    SEIChromaResamplingFilterHint *seiChromaResamplingFilterHint = new SEIChromaResamplingFilterHint;
    m_seiEncoder.initSEIChromaResamplingFilterHint(seiChromaResamplingFilterHint, m_pcCfg->getChromaResamplingHorFilterIdc(), m_pcCfg->getChromaResamplingVerFilterIdc());
    seiMessages.push_back(seiChromaResamplingFilterHint);
  }
#if U0033_ALTERNATIVE_TRANSFER_CHARACTERISTICS_SEI
  if(m_pcCfg->getSEIAlternativeTransferCharacteristicsSEIEnable())
  {
    SEIAlternativeTransferCharacteristics *seiAlternativeTransferCharacteristics = new SEIAlternativeTransferCharacteristics;
    m_seiEncoder.initSEIAlternativeTransferCharacteristics(seiAlternativeTransferCharacteristics);
    seiMessages.push_back(seiAlternativeTransferCharacteristics);
  }
#endif

#if SVC_EXTENSION
#if LAYERS_NOT_PRESENT_SEI
  if(m_pcCfg->getLayersNotPresentSEIEnabled())
  {
    SEILayersNotPresent *sei = new SEILayersNotPresent;
    m_seiEncoder.initSEILayersNotPresent(sei);
    seiMessages.push_back(sei);
  }
#endif

#if N0383_IL_CONSTRAINED_TILE_SETS_SEI
  if(m_pcCfg->getInterLayerConstrainedTileSetsSEIEnabled())
  {
    SEIInterLayerConstrainedTileSets *sei = new SEIInterLayerConstrainedTileSets;
    m_seiEncoder.initSEIInterLayerConstrainedTileSets(sei);

    // nalu = NALUnit(NAL_UNIT_PREFIX_SEI, 0, m_pcCfg->getNumLayer()-1); // For highest layer //ToDo(VS)
    seiMessages.push_back(sei);
  }
#endif

#if P0123_ALPHA_CHANNEL_SEI
  if( m_pcCfg->getAlphaSEIEnabled() && m_pcEncTop->getVPS()->getScalabilityId(m_layerId, AUX_ID) && m_pcEncTop->getVPS()->getDimensionId(m_pcEncTop->getVPS()->getLayerIdxInVps(m_layerId), m_pcEncTop->getVPS()->getNumScalabilityTypes() - 1) == AUX_ALPHA )
  {
    SEIAlphaChannelInfo *sei = new SEIAlphaChannelInfo;
    m_seiEncoder.initSEIAlphaChannelInfo(sei);
    seiMessages.push_back(sei);
  }
#endif

#if Q0096_OVERLAY_SEI
  if(m_pcCfg->getOverlaySEIEnabled())
  {
    SEIOverlayInfo *sei = new SEIOverlayInfo;
    m_seiEncoder.initSEIOverlayInfo(sei);
    m_pcEntropyCoder->setBitstream(&nalu.m_Bitstream);
    seiMessages.push_back(sei);
  }
#endif
#if O0164_MULTI_LAYER_HRD
  if( m_layerId == 0 && m_pcEncTop->getVPS()->getVpsVuiBspHrdPresentFlag() )
  {
    TComVPS *vps = m_pcEncTop->getVPS();
    for(Int i = 0; i < vps->getNumOutputLayerSets(); i++)
    {
      for(Int k = 0; k < vps->getNumSignalledPartitioningSchemes(i); k++)
      {
        for(Int l = 0; l < vps->getNumPartitionsInSchemeMinus1(i, k)+1; l++)
        {
          SEIScalableNesting *scalableBspNestingSei = new SEIScalableNesting;
          m_seiEncoder.initBspNestingSEI(scalableBspNestingSei, vps, sps, i, k, l);
          seiMessages.push_back(scalableBspNestingSei);
        }
      }
    }
  }
#endif
#endif //SVC_EXTENSION
}

Void TEncGOP::xCreatePerPictureSEIMessages (Int picInGOP, SEIMessages& seiMessages, SEIMessages& nestedSeiMessages, TComSlice *slice)
{
  if( ( m_pcCfg->getBufferingPeriodSEIEnabled() ) && ( slice->getSliceType() == I_SLICE ) &&
    ( slice->getSPS()->getVuiParametersPresentFlag() ) &&
    ( ( slice->getSPS()->getVuiParameters()->getHrdParameters()->getNalHrdParametersPresentFlag() )
    || ( slice->getSPS()->getVuiParameters()->getHrdParameters()->getVclHrdParametersPresentFlag() ) ) )
  {
    SEIBufferingPeriod *bufferingPeriodSEI = new SEIBufferingPeriod();
    m_seiEncoder.initSEIBufferingPeriod(bufferingPeriodSEI, slice);
    seiMessages.push_back(bufferingPeriodSEI);
    m_bufferingPeriodSEIPresentInAU = true;

    if (m_pcCfg->getScalableNestingSEIEnabled())
    {
      SEIBufferingPeriod *bufferingPeriodSEIcopy = new SEIBufferingPeriod();
      bufferingPeriodSEI->copyTo(*bufferingPeriodSEIcopy);
      nestedSeiMessages.push_back(bufferingPeriodSEIcopy);
    }
  }

  if (picInGOP ==0 && m_pcCfg->getSOPDescriptionSEIEnabled() ) // write SOP description SEI (if enabled) at the beginning of GOP
  {
    SEISOPDescription* sopDescriptionSEI = new SEISOPDescription();
    m_seiEncoder.initSEISOPDescription(sopDescriptionSEI, slice, picInGOP, m_iLastIDR, m_iGopSize);
    seiMessages.push_back(sopDescriptionSEI);
  }

  if( ( m_pcEncTop->getRecoveryPointSEIEnabled() ) && ( slice->getSliceType() == I_SLICE ) )
  {
    if( m_pcEncTop->getGradualDecodingRefreshInfoEnabled() && !slice->getRapPicFlag() )
    {
      // Gradual decoding refresh SEI
      SEIGradualDecodingRefreshInfo *gradualDecodingRefreshInfoSEI = new SEIGradualDecodingRefreshInfo();
      gradualDecodingRefreshInfoSEI->m_gdrForegroundFlag = true; // Indicating all "foreground"
      seiMessages.push_back(gradualDecodingRefreshInfoSEI);
    }
    // Recovery point SEI
    SEIRecoveryPoint *recoveryPointSEI = new SEIRecoveryPoint();
    m_seiEncoder.initSEIRecoveryPoint(recoveryPointSEI, slice);
    seiMessages.push_back(recoveryPointSEI);
  }
  if (m_pcCfg->getTemporalLevel0IndexSEIEnabled())
  {
    SEITemporalLevel0Index *temporalLevel0IndexSEI = new SEITemporalLevel0Index();
    m_seiEncoder.initTemporalLevel0IndexSEI(temporalLevel0IndexSEI, slice);
    seiMessages.push_back(temporalLevel0IndexSEI);
  }

  if( m_pcEncTop->getNoDisplaySEITLayer() && ( slice->getTLayer() >= m_pcEncTop->getNoDisplaySEITLayer() ) )
  {
    SEINoDisplay *seiNoDisplay = new SEINoDisplay;
    seiNoDisplay->m_noDisplay = true;
    seiMessages.push_back(seiNoDisplay);
  }

  // insert one Colour Remapping Info SEI for the picture (if the file exists)
  if (!m_pcCfg->getColourRemapInfoSEIFileRoot().empty())
  {
    SEIColourRemappingInfo *seiColourRemappingInfo = new SEIColourRemappingInfo();
    const Bool success = m_seiEncoder.initSEIColourRemappingInfo(seiColourRemappingInfo, slice->getPOC() );

    if(success)
    {
      seiMessages.push_back(seiColourRemappingInfo);
    }
    else
    {
      delete seiColourRemappingInfo;
    }
  }

#if Q0189_TMVP_CONSTRAINTS
  if( m_pcEncTop->getTMVPConstraintsSEIEnabled() == 1 && (m_pcEncTop->getTMVPModeId() == 1 || m_pcEncTop->getTMVPModeId() == 2) &&
    slice->getLayerId() > 0 && (slice->getNalUnitType() ==  NAL_UNIT_CODED_SLICE_IDR_W_RADL || slice->getNalUnitType() ==  NAL_UNIT_CODED_SLICE_IDR_N_LP))
  {
    SEITMVPConstrains *seiTMVPConstrains = new SEITMVPConstrains;
    seiTMVPConstrains->no_intra_layer_col_pic_flag = 1;
    seiTMVPConstrains->prev_pics_not_used_flag = 1;
    seiMessages.push_back(seiTMVPConstrains);
  }
#endif
#if Q0247_FRAME_FIELD_INFO
  if( slice->getLayerId()> 0 && ( (m_pcCfg->getProgressiveSourceFlag() && m_pcCfg->getInterlacedSourceFlag()) || m_pcCfg->getFrameFieldInfoPresentFlag()))
  {
    Bool isField = slice->getPic()->isField();
    SEIFrameFieldInfo *seiFFInfo = new SEIFrameFieldInfo;
    seiFFInfo->m_ffinfo_picStruct = (isField && slice->getPic()->isTopField())? 1 : isField? 2 : 0;
    seiMessages.push_back(seiFFInfo);
  }
#endif
}

Void TEncGOP::xCreateScalableNestingSEI (SEIMessages& seiMessages, SEIMessages& nestedSeiMessages)
{
  SEIMessages tmpMessages;
  while (!nestedSeiMessages.empty())
  {
    SEI* sei=nestedSeiMessages.front();
    nestedSeiMessages.pop_front();
    tmpMessages.push_back(sei);
    SEIScalableNesting *nestingSEI = new SEIScalableNesting();
    m_seiEncoder.initSEIScalableNesting(nestingSEI, tmpMessages);
    seiMessages.push_back(nestingSEI);
    tmpMessages.clear();
  }
}

Void TEncGOP::xCreatePictureTimingSEI  (Int IRAPGOPid, SEIMessages& seiMessages, SEIMessages& nestedSeiMessages, SEIMessages& duInfoSeiMessages, TComSlice *slice, Bool isField, std::deque<DUData> &duData)
{
  const TComVUI *vui = slice->getSPS()->getVuiParameters();
  const TComHRD *hrd = vui->getHrdParameters();

  // update decoding unit parameters
  if( ( m_pcCfg->getPictureTimingSEIEnabled() || m_pcCfg->getDecodingUnitInfoSEIEnabled() ) &&
    ( slice->getSPS()->getVuiParametersPresentFlag() ) &&
    (  hrd->getNalHrdParametersPresentFlag() || hrd->getVclHrdParametersPresentFlag() ) )
  {
    Int picSptDpbOutputDuDelay = 0;
    SEIPictureTiming *pictureTimingSEI = new SEIPictureTiming();

    // DU parameters
    if( hrd->getSubPicCpbParamsPresentFlag() )
    {
      UInt numDU = (UInt) duData.size();
      pictureTimingSEI->m_numDecodingUnitsMinus1     = ( numDU - 1 );
      pictureTimingSEI->m_duCommonCpbRemovalDelayFlag = false;
      pictureTimingSEI->m_numNalusInDuMinus1.resize( numDU );
      pictureTimingSEI->m_duCpbRemovalDelayMinus1.resize( numDU );
    }
    pictureTimingSEI->m_auCpbRemovalDelay = std::min<Int>(std::max<Int>(1, m_totalCoded - m_lastBPSEI), static_cast<Int>(pow(2, static_cast<Double>(hrd->getCpbRemovalDelayLengthMinus1()+1)))); // Syntax element signalled as minus, hence the .
    pictureTimingSEI->m_picDpbOutputDelay = slice->getSPS()->getNumReorderPics(slice->getSPS()->getMaxTLayers()-1) + slice->getPOC() - m_totalCoded;
    if(m_pcCfg->getEfficientFieldIRAPEnabled() && IRAPGOPid > 0 && IRAPGOPid < m_iGopSize)
    {
      // if pictures have been swapped there is likely one more picture delay on their tid. Very rough approximation
      pictureTimingSEI->m_picDpbOutputDelay ++;
    }
    Int factor = hrd->getTickDivisorMinus2() + 2;
    pictureTimingSEI->m_picDpbOutputDuDelay = factor * pictureTimingSEI->m_picDpbOutputDelay;
    if( m_pcCfg->getDecodingUnitInfoSEIEnabled() )
    {
      picSptDpbOutputDuDelay = factor * pictureTimingSEI->m_picDpbOutputDelay;
    }
    if (m_bufferingPeriodSEIPresentInAU)
    {
      m_lastBPSEI = m_totalCoded;
    }

    if( hrd->getSubPicCpbParamsPresentFlag() )
    {
      Int i;
      UInt64 ui64Tmp;
      UInt uiPrev = 0;
      UInt numDU = ( pictureTimingSEI->m_numDecodingUnitsMinus1 + 1 );
      std::vector<UInt> &rDuCpbRemovalDelayMinus1 = pictureTimingSEI->m_duCpbRemovalDelayMinus1;
      UInt maxDiff = ( hrd->getTickDivisorMinus2() + 2 ) - 1;

      for( i = 0; i < numDU; i ++ )
      {
        pictureTimingSEI->m_numNalusInDuMinus1[ i ]       = ( i == 0 ) ? ( duData[i].accumNalsDU - 1 ) : ( duData[i].accumNalsDU- duData[i-1].accumNalsDU - 1 );
      }

      if( numDU == 1 )
      {
        rDuCpbRemovalDelayMinus1[ 0 ] = 0; /* don't care */
      }
      else
      {
        rDuCpbRemovalDelayMinus1[ numDU - 1 ] = 0;/* by definition */
        UInt tmp = 0;
        UInt accum = 0;

        for( i = ( numDU - 2 ); i >= 0; i -- )
        {
          ui64Tmp = ( ( ( duData[numDU - 1].accumBitsDU  - duData[i].accumBitsDU ) * ( vui->getTimingInfo()->getTimeScale() / vui->getTimingInfo()->getNumUnitsInTick() ) * ( hrd->getTickDivisorMinus2() + 2 ) ) / ( m_pcCfg->getTargetBitrate() ) );
          if( (UInt)ui64Tmp > maxDiff )
          {
            tmp ++;
          }
        }
        uiPrev = 0;

        UInt flag = 0;
        for( i = ( numDU - 2 ); i >= 0; i -- )
        {
          flag = 0;
          ui64Tmp = ( ( ( duData[numDU - 1].accumBitsDU  - duData[i].accumBitsDU ) * ( vui->getTimingInfo()->getTimeScale() / vui->getTimingInfo()->getNumUnitsInTick() ) * ( hrd->getTickDivisorMinus2() + 2 ) ) / ( m_pcCfg->getTargetBitrate() ) );

          if( (UInt)ui64Tmp > maxDiff )
          {
            if(uiPrev >= maxDiff - tmp)
            {
              ui64Tmp = uiPrev + 1;
              flag = 1;
            }
            else                            ui64Tmp = maxDiff - tmp + 1;
          }
          rDuCpbRemovalDelayMinus1[ i ] = (UInt)ui64Tmp - uiPrev - 1;
          if( (Int)rDuCpbRemovalDelayMinus1[ i ] < 0 )
          {
            rDuCpbRemovalDelayMinus1[ i ] = 0;
          }
          else if (tmp > 0 && flag == 1)
          {
            tmp --;
          }
          accum += rDuCpbRemovalDelayMinus1[ i ] + 1;
          uiPrev = accum;
        }
      }
    }
    
    if( m_pcCfg->getPictureTimingSEIEnabled() )
    {
      pictureTimingSEI->m_picStruct = (isField && slice->getPic()->isTopField())? 1 : isField? 2 : 0;
      seiMessages.push_back(pictureTimingSEI);

      if ( m_pcCfg->getScalableNestingSEIEnabled() ) // put picture timing SEI into scalable nesting SEI
      {
        SEIPictureTiming *pictureTimingSEIcopy = new SEIPictureTiming();
        pictureTimingSEI->copyTo(*pictureTimingSEIcopy);
        nestedSeiMessages.push_back(pictureTimingSEIcopy);
      }
    }

    if( m_pcCfg->getDecodingUnitInfoSEIEnabled() && hrd->getSubPicCpbParamsPresentFlag() )
    {
      for( Int i = 0; i < ( pictureTimingSEI->m_numDecodingUnitsMinus1 + 1 ); i ++ )
      {
        SEIDecodingUnitInfo *duInfoSEI = new SEIDecodingUnitInfo();
        duInfoSEI->m_decodingUnitIdx = i;
        duInfoSEI->m_duSptCpbRemovalDelay = pictureTimingSEI->m_duCpbRemovalDelayMinus1[i] + 1;
        duInfoSEI->m_dpbOutputDuDelayPresentFlag = false;
        duInfoSEI->m_picSptDpbOutputDuDelay = picSptDpbOutputDuDelay;

        duInfoSeiMessages.push_back(duInfoSEI);
      }
    }

    if( !m_pcCfg->getPictureTimingSEIEnabled() && pictureTimingSEI )
    {
      delete pictureTimingSEI;
    }
  }
}

Void TEncGOP::xUpdateDuData(AccessUnit &testAU, std::deque<DUData> &duData)
{
  if (duData.empty())
  {
    return;
  }
  // fix first 
  UInt numNalUnits = (UInt)testAU.size();
  UInt numRBSPBytes = 0;
  for (AccessUnit::const_iterator it = testAU.begin(); it != testAU.end(); it++)
  {
    numRBSPBytes += UInt((*it)->m_nalUnitData.str().size());
  }
  duData[0].accumBitsDU += ( numRBSPBytes << 3 );
  duData[0].accumNalsDU += numNalUnits;

  // adapt cumulative sums for all following DUs
  // and add one DU info SEI, if enabled
  for (Int i=1; i<duData.size(); i++)
  {
    if (m_pcCfg->getDecodingUnitInfoSEIEnabled())
    {
      numNalUnits  += 1;
      numRBSPBytes += ( 5 << 3 );
    }
    duData[i].accumBitsDU += numRBSPBytes; // probably around 5 bytes
    duData[i].accumNalsDU += numNalUnits;
  }

  // The last DU may have a trailing SEI
  if (m_pcCfg->getDecodedPictureHashSEIType()!=HASHTYPE_NONE)
  {
    duData.back().accumBitsDU += ( 20 << 3 ); // probably around 20 bytes - should be further adjusted, e.g. by type
    duData.back().accumNalsDU += 1;
  }

}
Void TEncGOP::xUpdateTimingSEI(SEIPictureTiming *pictureTimingSEI, std::deque<DUData> &duData, const TComSPS *sps)
{
  if (!pictureTimingSEI)
  {
    return;
  }
  const TComVUI *vui = sps->getVuiParameters();
  const TComHRD *hrd = vui->getHrdParameters();
  if( hrd->getSubPicCpbParamsPresentFlag() )
  {
    Int i;
    UInt64 ui64Tmp;
    UInt uiPrev = 0;
    UInt numDU = ( pictureTimingSEI->m_numDecodingUnitsMinus1 + 1 );
    std::vector<UInt> &rDuCpbRemovalDelayMinus1 = pictureTimingSEI->m_duCpbRemovalDelayMinus1;
    UInt maxDiff = ( hrd->getTickDivisorMinus2() + 2 ) - 1;

    for( i = 0; i < numDU; i ++ )
    {
      pictureTimingSEI->m_numNalusInDuMinus1[ i ]       = ( i == 0 ) ? ( duData[i].accumNalsDU - 1 ) : ( duData[i].accumNalsDU- duData[i-1].accumNalsDU - 1 );
    }

    if( numDU == 1 )
    {
      rDuCpbRemovalDelayMinus1[ 0 ] = 0; /* don't care */
    }
    else
    {
      rDuCpbRemovalDelayMinus1[ numDU - 1 ] = 0;/* by definition */
      UInt tmp = 0;
      UInt accum = 0;

      for( i = ( numDU - 2 ); i >= 0; i -- )
      {
        ui64Tmp = ( ( ( duData[numDU - 1].accumBitsDU  - duData[i].accumBitsDU ) * ( vui->getTimingInfo()->getTimeScale() / vui->getTimingInfo()->getNumUnitsInTick() ) * ( hrd->getTickDivisorMinus2() + 2 ) ) / ( m_pcCfg->getTargetBitrate() ) );
        if( (UInt)ui64Tmp > maxDiff )
        {
          tmp ++;
        }
      }
      uiPrev = 0;

      UInt flag = 0;
      for( i = ( numDU - 2 ); i >= 0; i -- )
      {
        flag = 0;
        ui64Tmp = ( ( ( duData[numDU - 1].accumBitsDU  - duData[i].accumBitsDU ) * ( vui->getTimingInfo()->getTimeScale() / vui->getTimingInfo()->getNumUnitsInTick() ) * ( hrd->getTickDivisorMinus2() + 2 ) ) / ( m_pcCfg->getTargetBitrate() ) );

        if( (UInt)ui64Tmp > maxDiff )
        {
          if(uiPrev >= maxDiff - tmp)
          {
            ui64Tmp = uiPrev + 1;
            flag = 1;
          }
          else                            ui64Tmp = maxDiff - tmp + 1;
        }
        rDuCpbRemovalDelayMinus1[ i ] = (UInt)ui64Tmp - uiPrev - 1;
        if( (Int)rDuCpbRemovalDelayMinus1[ i ] < 0 )
        {
          rDuCpbRemovalDelayMinus1[ i ] = 0;
        }
        else if (tmp > 0 && flag == 1)
        {
          tmp --;
        }
        accum += rDuCpbRemovalDelayMinus1[ i ] + 1;
        uiPrev = accum;
      }
    }
  }
}
Void TEncGOP::xUpdateDuInfoSEI(SEIMessages &duInfoSeiMessages, SEIPictureTiming *pictureTimingSEI)
{
  if (duInfoSeiMessages.empty() || (pictureTimingSEI == NULL))
  {
    return;
  }

  Int i=0;

  for (SEIMessages::iterator du = duInfoSeiMessages.begin(); du!= duInfoSeiMessages.end(); du++)
  {
    SEIDecodingUnitInfo *duInfoSEI = (SEIDecodingUnitInfo*) (*du);
    duInfoSEI->m_decodingUnitIdx = i;
    duInfoSEI->m_duSptCpbRemovalDelay = pictureTimingSEI->m_duCpbRemovalDelayMinus1[i] + 1;
    duInfoSEI->m_dpbOutputDuDelayPresentFlag = false;
    i++;
  }
}

static Void
cabac_zero_word_padding(TComSlice *const pcSlice, TComPic *const pcPic, const std::size_t binCountsInNalUnits, const std::size_t numBytesInVclNalUnits, std::ostringstream &nalUnitData, const Bool cabacZeroWordPaddingEnabled)
{
  const TComSPS &sps=*(pcSlice->getSPS());
  const Int log2subWidthCxsubHeightC = (pcPic->getComponentScaleX(COMPONENT_Cb)+pcPic->getComponentScaleY(COMPONENT_Cb));
  const Int minCuWidth  = pcPic->getMinCUWidth();
  const Int minCuHeight = pcPic->getMinCUHeight();
  const Int paddedWidth = ((sps.getPicWidthInLumaSamples()  + minCuWidth  - 1) / minCuWidth) * minCuWidth;
  const Int paddedHeight= ((sps.getPicHeightInLumaSamples() + minCuHeight - 1) / minCuHeight) * minCuHeight;
  const Int rawBits = paddedWidth * paddedHeight *
                         (sps.getBitDepth(CHANNEL_TYPE_LUMA) + 2*(sps.getBitDepth(CHANNEL_TYPE_CHROMA)>>log2subWidthCxsubHeightC));
  const std::size_t threshold = (32/3)*numBytesInVclNalUnits + (rawBits/32);
  if (binCountsInNalUnits >= threshold)
  {
    // need to add additional cabac zero words (each one accounts for 3 bytes (=00 00 03)) to increase numBytesInVclNalUnits
    const std::size_t targetNumBytesInVclNalUnits = ((binCountsInNalUnits - (rawBits/32))*3+31)/32;

    if (targetNumBytesInVclNalUnits>numBytesInVclNalUnits) // It should be!
    {
      const std::size_t numberOfAdditionalBytesNeeded=targetNumBytesInVclNalUnits - numBytesInVclNalUnits;
      const std::size_t numberOfAdditionalCabacZeroWords=(numberOfAdditionalBytesNeeded+2)/3;
      const std::size_t numberOfAdditionalCabacZeroBytes=numberOfAdditionalCabacZeroWords*3;
      if (cabacZeroWordPaddingEnabled)
      {
        std::vector<UChar> zeroBytesPadding(numberOfAdditionalCabacZeroBytes, UChar(0));
        for(std::size_t i=0; i<numberOfAdditionalCabacZeroWords; i++)
        {
          zeroBytesPadding[i*3+2]=3;  // 00 00 03
        }
        nalUnitData.write(reinterpret_cast<const TChar*>(&(zeroBytesPadding[0])), numberOfAdditionalCabacZeroBytes);
        printf("Adding %d bytes of padding\n", UInt(numberOfAdditionalCabacZeroWords*3));
      }
      else
      {
        printf("Standard would normally require adding %d bytes of padding\n", UInt(numberOfAdditionalCabacZeroWords*3));
      }
    }
  }
}

class EfficientFieldIRAPMapping
{
  private:
    Int  IRAPGOPid;
    Bool IRAPtoReorder;
    Bool swapIRAPForward;

  public:
    EfficientFieldIRAPMapping() :
      IRAPGOPid(-1),
      IRAPtoReorder(false),
      swapIRAPForward(false)
    { }

#if SVC_EXTENSION
    Void initialize(const Bool isField, const Int picIdInGOP, const Int gopSize, const Int POCLast, const Int numPicRcvd, const Int lastIDR, TEncGOP *pEncGop, TEncCfg *pCfg);
#else
    Void initialize(const Bool isField, const Int gopSize, const Int POCLast, const Int numPicRcvd, const Int lastIDR, TEncGOP *pEncGop, TEncCfg *pCfg);
#endif

    Int adjustGOPid(const Int gopID);
    Int restoreGOPid(const Int gopID);
    Int GetIRAPGOPid() const { return IRAPGOPid; }
};

#if SVC_EXTENSION
Void EfficientFieldIRAPMapping::initialize(const Bool isField, const Int picIdInGOP, const Int gopSize, const Int POCLast, const Int numPicRcvd, const Int lastIDR, TEncGOP *pEncGop, TEncCfg *pCfg )
#else
Void EfficientFieldIRAPMapping::initialize(const Bool isField, const Int gopSize, const Int POCLast, const Int numPicRcvd, const Int lastIDR, TEncGOP *pEncGop, TEncCfg *pCfg )
#endif
{
  if(isField)
  {
    Int pocCurr;
#if SVC_EXTENSION
    for ( Int iGOPid=picIdInGOP; iGOPid < picIdInGOP+1; iGOPid++ )
#else
    for ( Int iGOPid=0; iGOPid < gopSize; iGOPid++ )
#endif    
    {
      // determine actual POC
      if(POCLast == 0) //case first frame or first top field
      {
        pocCurr=0;
      }
      else if(POCLast == 1 && isField) //case first bottom field, just like the first frame, the poc computation is not right anymore, we set the right value
      {
        pocCurr = 1;
      }
      else
      {
        pocCurr = POCLast - numPicRcvd + pCfg->getGOPEntry(iGOPid).m_POC - isField;
      }

      // check if POC corresponds to IRAP
      NalUnitType tmpUnitType = pEncGop->getNalUnitType(pocCurr, lastIDR, isField);
      if(tmpUnitType >= NAL_UNIT_CODED_SLICE_BLA_W_LP && tmpUnitType <= NAL_UNIT_CODED_SLICE_CRA) // if picture is an IRAP
      {
        if(pocCurr%2 == 0 && iGOPid < gopSize-1 && pCfg->getGOPEntry(iGOPid).m_POC == pCfg->getGOPEntry(iGOPid+1).m_POC-1)
        { // if top field and following picture in enc order is associated bottom field
          IRAPGOPid = iGOPid;
          IRAPtoReorder = true;
          swapIRAPForward = true; 
          break;
        }
        if(pocCurr%2 != 0 && iGOPid > 0 && pCfg->getGOPEntry(iGOPid).m_POC == pCfg->getGOPEntry(iGOPid-1).m_POC+1)
        {
          // if picture is an IRAP remember to process it first
          IRAPGOPid = iGOPid;
          IRAPtoReorder = true;
          swapIRAPForward = false; 
          break;
        }
      }
    }
  }
}

Int EfficientFieldIRAPMapping::adjustGOPid(const Int GOPid)
{
  if(IRAPtoReorder)
  {
    if(swapIRAPForward)
    {
      if(GOPid == IRAPGOPid)
      {
        return IRAPGOPid +1;
      }
      else if(GOPid == IRAPGOPid +1)
      {
        return IRAPGOPid;
      }
    }
    else
    {
      if(GOPid == IRAPGOPid -1)
      {
        return IRAPGOPid;
      }
      else if(GOPid == IRAPGOPid)
      {
        return IRAPGOPid -1;
      }
    }
  }
  return GOPid;
}

Int EfficientFieldIRAPMapping::restoreGOPid(const Int GOPid)
{
  if(IRAPtoReorder)
  {
    if(swapIRAPForward)
    {
      if(GOPid == IRAPGOPid)
      {
        IRAPtoReorder = false;
        return IRAPGOPid +1;
      }
      else if(GOPid == IRAPGOPid +1)
      {
        return GOPid -1;
      }
    }
    else
    {
      if(GOPid == IRAPGOPid)
      {
        return IRAPGOPid -1;
      }
      else if(GOPid == IRAPGOPid -1)
      {
        IRAPtoReorder = false;
        return IRAPGOPid;
      }
    }
  }
  return GOPid;
}


static UInt calculateCollocatedFromL1Flag(TEncCfg *pCfg, const Int GOPid, const Int gopSize)
{
  Int iCloseLeft=1, iCloseRight=-1;
  for(Int i = 0; i<pCfg->getGOPEntry(GOPid).m_numRefPics; i++)
  {
    Int iRef = pCfg->getGOPEntry(GOPid).m_referencePics[i];
    if(iRef>0&&(iRef<iCloseRight||iCloseRight==-1))
    {
      iCloseRight=iRef;
    }
    else if(iRef<0&&(iRef>iCloseLeft||iCloseLeft==1))
    {
      iCloseLeft=iRef;
    }
  }
  if(iCloseRight>-1)
  {
    iCloseRight=iCloseRight+pCfg->getGOPEntry(GOPid).m_POC-1;
  }
  if(iCloseLeft<1)
  {
    iCloseLeft=iCloseLeft+pCfg->getGOPEntry(GOPid).m_POC-1;
    while(iCloseLeft<0)
    {
      iCloseLeft+=gopSize;
    }
  }
  Int iLeftQP=0, iRightQP=0;
  for(Int i=0; i<gopSize; i++)
  {
    if(pCfg->getGOPEntry(i).m_POC==(iCloseLeft%gopSize)+1)
    {
      iLeftQP= pCfg->getGOPEntry(i).m_QPOffset;
    }
    if (pCfg->getGOPEntry(i).m_POC==(iCloseRight%gopSize)+1)
    {
      iRightQP=pCfg->getGOPEntry(i).m_QPOffset;
    }
  }
  if(iCloseRight>-1&&iRightQP<iLeftQP)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}


static Void
printHash(const HashType hashType, const std::string &digestStr)
{
  const TChar *decodedPictureHashModeName;
  switch (hashType)
  {
    case HASHTYPE_MD5:
      decodedPictureHashModeName = "MD5";
      break;
    case HASHTYPE_CRC:
      decodedPictureHashModeName = "CRC";
      break;
    case HASHTYPE_CHECKSUM:
      decodedPictureHashModeName = "Checksum";
      break;
    default:
      decodedPictureHashModeName = NULL;
      break;
  }
  if (decodedPictureHashModeName != NULL)
  {
    if (digestStr.empty())
    {
      printf(" [%s:%s]", decodedPictureHashModeName, "?");
    }
    else
    {
      printf(" [%s:%s]", decodedPictureHashModeName, digestStr.c_str());
    }
  }
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================
#if SVC_EXTENSION
Void TEncGOP::compressGOP( Int iPicIdInGOP, Int iPOCLast, Int iNumPicRcvd, TComList<TComPic*>& rcListPic,
#else
Void TEncGOP::compressGOP( Int iPOCLast, Int iNumPicRcvd, TComList<TComPic*>& rcListPic,
#endif
                           TComList<TComPicYuv*>& rcListPicYuvRecOut, std::list<AccessUnit>& accessUnitsInGOP,
                           Bool isField, Bool isTff, const InputColourSpaceConversion snr_conversion, const Bool printFrameMSE )

{
  // TODO: Split this function up.

  TComPic*        pcPic = NULL;
  TComPicYuv*     pcPicYuvRecOut;
  TComSlice*      pcSlice;
  TComOutputBitstream  *pcBitstreamRedirect;
  pcBitstreamRedirect = new TComOutputBitstream;
  AccessUnit::iterator  itLocationToPushSliceHeaderNALU; // used to store location where NALU containing slice header is to be inserted

  xInitGOP( iPOCLast, iNumPicRcvd, isField );

  m_iNumPicCoded = 0;
  SEIMessages leadingSeiMessages;
  SEIMessages nestedSeiMessages;
  SEIMessages duInfoSeiMessages;
  SEIMessages trailingSeiMessages;
  std::deque<DUData> duData;
  SEIDecodingUnitInfo decodingUnitInfoSEI;

  EfficientFieldIRAPMapping effFieldIRAPMap;
  if (m_pcCfg->getEfficientFieldIRAPEnabled())
  {
#if SVC_EXTENSION
    effFieldIRAPMap.initialize(isField, iPicIdInGOP, m_iGopSize, iPOCLast, iNumPicRcvd, m_iLastIDR, this, m_pcCfg);
#else
    effFieldIRAPMap.initialize(isField, m_iGopSize, iPOCLast, iNumPicRcvd, m_iLastIDR, this, m_pcCfg);
#endif
  }

  // reset flag indicating whether pictures have been encoded
  for ( Int iGOPid=0; iGOPid < m_iGopSize; iGOPid++ )
  {
    m_pcCfg->setEncodedFlag(iGOPid, false);
  }
#if SVC_EXTENSION
  for ( Int iGOPid=iPicIdInGOP; iGOPid < iPicIdInGOP+1; iGOPid++ )
#else
  for ( Int iGOPid=0; iGOPid < m_iGopSize; iGOPid++ )
#endif
  {
    if (m_pcCfg->getEfficientFieldIRAPEnabled())
    {
      iGOPid=effFieldIRAPMap.adjustGOPid(iGOPid);
    }

    //-- For time output for each slice
    clock_t iBeforeTime = clock();

    UInt uiColDir = calculateCollocatedFromL1Flag(m_pcCfg, iGOPid, m_iGopSize);

    /////////////////////////////////////////////////////////////////////////////////////////////////// Initial to start encoding
    Int iTimeOffset;
    Int pocCurr;

    if(iPOCLast == 0) //case first frame or first top field
    {
      pocCurr=0;
      iTimeOffset = 1;
    }
    else if(iPOCLast == 1 && isField) //case first bottom field, just like the first frame, the poc computation is not right anymore, we set the right value
    {
      pocCurr = 1;
      iTimeOffset = 1;
    }
    else
    {
      pocCurr = iPOCLast - iNumPicRcvd + m_pcCfg->getGOPEntry(iGOPid).m_POC - ((isField && m_iGopSize>1) ? 1:0);
      iTimeOffset = m_pcCfg->getGOPEntry(iGOPid).m_POC;
    }

    if(pocCurr>=m_pcCfg->getFramesToBeEncoded())
    {
      if (m_pcCfg->getEfficientFieldIRAPEnabled())
      {
        iGOPid=effFieldIRAPMap.restoreGOPid(iGOPid);
      }
      continue;
    }

#if SVC_EXTENSION
    if (m_pcEncTop->getAdaptiveResolutionChange() > 0 && ((m_layerId > 0 && pocCurr < m_pcEncTop->getAdaptiveResolutionChange()) ||
                                                          (m_layerId == 0 && pocCurr > m_pcEncTop->getAdaptiveResolutionChange())) )
    {
      continue;
    }

    if (pocCurr > m_pcEncTop->getLayerSwitchOffBegin() && pocCurr < m_pcEncTop->getLayerSwitchOffEnd())
    {
      continue;
    }
#endif

    if( getNalUnitType(pocCurr, m_iLastIDR, isField) == NAL_UNIT_CODED_SLICE_IDR_W_RADL || getNalUnitType(pocCurr, m_iLastIDR, isField) == NAL_UNIT_CODED_SLICE_IDR_N_LP )
    {
      m_iLastIDR = pocCurr;
    }
    // start a new access unit: create an entry in the list of output access units
    accessUnitsInGOP.push_back(AccessUnit());
    AccessUnit& accessUnit = accessUnitsInGOP.back();
    xGetBuffer( rcListPic, rcListPicYuvRecOut, iNumPicRcvd, iTimeOffset, pcPic, pcPicYuvRecOut, pocCurr, isField );

#if REDUCED_ENCODER_MEMORY
    pcPic->prepareForReconstruction();

#endif
    //  Slice data initialization
    pcPic->clearSliceBuffer();
    pcPic->allocateNewSlice();
    m_pcSliceEncoder->setSliceIdx(0);
    pcPic->setCurrSliceIdx(0);

#if SVC_EXTENSION
    pcPic->setLayerId( m_layerId );
#endif

    m_pcSliceEncoder->initEncSlice ( pcPic, iPOCLast, pocCurr, iGOPid, pcSlice, isField );

    //Set Frame/Field coding
    pcSlice->getPic()->setField(isField);

#if SVC_EXTENSION
#if SVC_POC
    pcSlice->setPocValueBeforeReset( pocCurr );
    // Check if the current picture is to be assigned as a reset picture
    determinePocResetIdc(pocCurr, pcSlice);

    Bool pocResettingFlag = false;

    if( pcSlice->getPocResetIdc() != 0 )
    {
      if( pcSlice->getVPS()->getVpsPocLsbAlignedFlag() )
      {
        pocResettingFlag = true;
      }
      else if( m_pcEncTop->getPocDecrementedInDPBFlag() )
      {
        pocResettingFlag = false;
      }
      else
      {
        pocResettingFlag = true;
      }
    }

    // If reset, do the following steps:
    if( pocResettingFlag )
    {
      updatePocValuesOfPics(pocCurr, pcSlice);
    }
    else
    {
      // Check the base layer picture is IDR. If so, just set current POC equal to 0 (alignment of POC)
      if( ( m_ppcTEncTop[0]->getGOPEncoder()->getIntraRefreshType() == 2) && ( pocCurr % m_ppcTEncTop[0]->getGOPEncoder()->getIntraRefreshInterval() == 0 ) )        
      {
        m_pcEncTop->setPocAdjustmentValue( pocCurr );
      }

      // Just subtract POC by the current cumulative POC delta
      pcSlice->setPOC( pocCurr - m_pcEncTop->getPocAdjustmentValue() );

      Int maxPocLsb = 1 << pcSlice->getSPS()->getBitsForPOC();
      pcSlice->setPocMsbVal( pcSlice->getPOC() - ( pcSlice->getPOC() & (maxPocLsb-1) ) );
    }
    // Update the POC of current picture, pictures in the DPB, including references inside the reference pictures
#endif

    if( m_layerId == 0 && (getNalUnitType(pocCurr, m_iLastIDR, isField) == NAL_UNIT_CODED_SLICE_IDR_W_RADL || getNalUnitType(pocCurr, m_iLastIDR, isField) == NAL_UNIT_CODED_SLICE_IDR_N_LP) )
    {
      pcSlice->setCrossLayerBLAFlag(m_pcEncTop->getCrossLayerBLAFlag());
    }
    else
    {
      pcSlice->setCrossLayerBLAFlag(false);
    }

    // Set the nal unit type
    pcSlice->setNalUnitType(getNalUnitType(pocCurr, m_iLastIDR, isField));

#if NO_CLRAS_OUTPUT_FLAG
    if (m_layerId == 0 &&
        (pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_BLA_W_LP
      || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_BLA_W_RADL
      || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_BLA_N_LP
      || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_W_RADL
      || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_N_LP
      || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_CRA))
    {
      if (m_bFirst)
      {
        m_pcEncTop->setNoClrasOutputFlag(true);
      }
      else if (m_prevPicHasEos)
      {
        m_pcEncTop->setNoClrasOutputFlag(true);
      }
      else if (pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_BLA_W_LP
            || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_BLA_W_RADL
            || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_BLA_N_LP)
      {
        m_pcEncTop->setNoClrasOutputFlag(true);
      }
      else if( pcSlice->getCrossLayerBLAFlag() && ( pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_W_RADL || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_N_LP ) )
      {
        m_pcEncTop->setNoClrasOutputFlag(true);
      }
      else
      {
        m_pcEncTop->setNoClrasOutputFlag(false);
      }

      if( m_pcEncTop->getNoClrasOutputFlag() )
      {
        for (UInt i = 0; i < m_pcCfg->getNumLayer(); i++)
        {
          m_ppcTEncTop[i]->setLayerInitializedFlag(false);
          m_ppcTEncTop[i]->setFirstPicInLayerDecodedFlag(false);
        }
      }
    }
#endif
    xCheckLayerReset(pcSlice);
    xSetNoRaslOutputFlag(pcSlice);
    xSetLayerInitializedFlag(pcSlice);

    if (m_pcEncTop->getAdaptiveResolutionChange() > 0 && m_layerId > 0 && pocCurr > m_pcEncTop->getAdaptiveResolutionChange())
    {
      pcSlice->setActiveNumILRRefIdx(0);
      pcSlice->setInterLayerPredEnabledFlag(false);
      pcSlice->setMFMEnabledFlag(false);
    }
#endif //SVC_EXTENSION

    pcSlice->setLastIDR(m_iLastIDR);
    pcSlice->setSliceIdx(0);
    //set default slice level flag to the same as SPS level flag
    pcSlice->setLFCrossSliceBoundaryFlag(  pcSlice->getPPS()->getLoopFilterAcrossSlicesEnabledFlag()  );

    if(pcSlice->getSliceType()==B_SLICE&&m_pcCfg->getGOPEntry(iGOPid).m_sliceType=='P')
    {
      pcSlice->setSliceType(P_SLICE);
    }
    if(pcSlice->getSliceType()==B_SLICE&&m_pcCfg->getGOPEntry(iGOPid).m_sliceType=='I')
    {
      pcSlice->setSliceType(I_SLICE);
    }
    
#if SVC_EXTENSION
    if (m_layerId > 0)
    {
      if( pcSlice->getSliceIdx() == 0 )
      {
        // create buffers for scaling factors
        if( pcSlice->getNumILRRefIdx() )
        {
          pcSlice->getPic()->createMvScalingFactor(pcSlice->getNumILRRefIdx());
          pcSlice->getPic()->createPosScalingFactor(pcSlice->getNumILRRefIdx());
        }
      }

      Int interLayerPredLayerIdcTmp[MAX_VPS_LAYER_IDX_PLUS1];
      Int activeNumILRRefIdxTmp = 0;

      for( Int i = 0; i < pcSlice->getActiveNumILRRefIdx(); i++ )
      {
        UInt refLayerIdc = pcSlice->getInterLayerPredLayerIdc(i);
        UInt refLayerId = pcSlice->getVPS()->getRefLayerId(m_layerId, refLayerIdc);
        TComList<TComPic*> *cListPic = m_ppcTEncTop[pcSlice->getVPS()->getLayerIdxInVps(m_layerId)]->getRefLayerEnc(refLayerIdc)->getListPic();

        pcSlice->setBaseColPic( *cListPic, refLayerIdc );

        // Apply temporal layer restriction to inter-layer prediction
        Int maxTidIlRefPicsPlus1 = m_pcEncTop->getVPS()->getMaxTidIlRefPicsPlus1(pcSlice->getBaseColPic(refLayerIdc)->getSlice(0)->getLayerIdx(), pcSlice->getLayerIdx());
        if( ((Int)(pcSlice->getBaseColPic(refLayerIdc)->getSlice(0)->getTLayer()) < maxTidIlRefPicsPlus1) || (maxTidIlRefPicsPlus1==0 && pcSlice->getBaseColPic(refLayerIdc)->getSlice(0)->getRapPicFlag()) )
        {
          interLayerPredLayerIdcTmp[activeNumILRRefIdxTmp++] = refLayerIdc; // add picture to the list of valid inter-layer pictures
        }
        else
        {
          continue; // SHM: ILP is not valid due to temporal layer restriction
        }

        const Window &scalEL = pcSlice->getPPS()->getScaledRefLayerWindowForLayer(refLayerId);
        const Window &windowRL = pcSlice->getPPS()->getRefLayerWindowForLayer(pcSlice->getVPS()->getRefLayerId(m_layerId, refLayerIdc));
        Int widthBL   = pcSlice->getBaseColPic(refLayerIdc)->getPicYuvRec()->getWidth(COMPONENT_Y) - windowRL.getWindowLeftOffset() - windowRL.getWindowRightOffset();
        Int heightBL  = pcSlice->getBaseColPic(refLayerIdc)->getPicYuvRec()->getHeight(COMPONENT_Y) - windowRL.getWindowTopOffset() - windowRL.getWindowBottomOffset();
        Int widthEL   = pcPic->getPicYuvRec()->getWidth(COMPONENT_Y)  - scalEL.getWindowLeftOffset() - scalEL.getWindowRightOffset();
        Int heightEL  = pcPic->getPicYuvRec()->getHeight(COMPONENT_Y) - scalEL.getWindowTopOffset()  - scalEL.getWindowBottomOffset();

        // conformance check: the values of RefLayerRegionWidthInSamplesY, RefLayerRegionHeightInSamplesY, ScaledRefRegionWidthInSamplesY and ScaledRefRegionHeightInSamplesY shall be greater than 0
        assert(widthEL > 0 && heightEL > 0 && widthBL > 0 && heightBL > 0);

        // conformance check: ScaledRefRegionWidthInSamplesY shall be greater or equal to RefLayerRegionWidthInSamplesY and ScaledRefRegionHeightInSamplesY shall be greater or equal to RefLayerRegionHeightInSamplesY
        assert(widthEL >= widthBL && heightEL >= heightBL);

        // conformance check: when ScaledRefRegionWidthInSamplesY is equal to RefLayerRegionWidthInSamplesY, PhaseHorY shall be equal to 0, when ScaledRefRegionWidthInSamplesC is equal to RefLayerRegionWidthInSamplesC, PhaseHorC shall be equal to 0, when ScaledRefRegionHeightInSamplesY is equal to RefLayerRegionHeightInSamplesY, PhaseVerY shall be equal to 0, and when ScaledRefRegionHeightInSamplesC is equal to RefLayerRegionHeightInSamplesC, PhaseVerC shall be equal to 0.
        const ResamplingPhase &resamplingPhase = pcSlice->getPPS()->getResamplingPhase( refLayerId );

        assert( ( (widthEL  != widthBL)  || (resamplingPhase.phaseHorLuma == 0 && resamplingPhase.phaseHorChroma == 0) )
             && ( (heightEL != heightBL) || (resamplingPhase.phaseVerLuma == 0 && resamplingPhase.phaseVerChroma == 0) ) );

        pcSlice->getPic()->setMvScalingFactor( refLayerIdc,
                                               widthEL  == widthBL  ? MV_SCALING_FACTOR_1X : Clip3(-4096, 4095, ((widthEL  << 8) + (widthBL  >> 1)) / widthBL),
                                               heightEL == heightBL ? MV_SCALING_FACTOR_1X : Clip3(-4096, 4095, ((heightEL << 8) + (heightBL >> 1)) / heightBL) );

        pcSlice->getPic()->setPosScalingFactor( refLayerIdc, 
                                                ((widthBL  << 16) + (widthEL  >> 1)) / widthEL,
                                                ((heightBL << 16) + (heightEL >> 1)) / heightEL );

        TComPicYuv* pBaseColRec = pcSlice->getBaseColPic(refLayerIdc)->getPicYuvRec();

#if CGS_3D_ASYMLUT
        if( pcSlice->getPPS()->getCGSFlag() )
        {
          // all reference layers are currently taken as CGS reference layers
          m_Enc3DAsymLUTPPS.addRefLayerId( pcSlice->getVPS()->getRefLayerId(m_layerId, refLayerIdc) );
          m_Enc3DAsymLUTPicUpdate.addRefLayerId( pcSlice->getVPS()->getRefLayerId(m_layerId, refLayerIdc) );

          if( pcSlice->getPic()->getPosScalingFactor(refLayerIdc, 0) < POS_SCALING_FACTOR_1X || pcSlice->getPic()->getPosScalingFactor(refLayerIdc, 1) < POS_SCALING_FACTOR_1X ) //if(pcPic->requireResampling(refLayerIdc))
          {
            //downsampling
            xDownScalePic(pcPic->getPicYuvOrg(), pcSlice->getBaseColPic(refLayerIdc)->getPicYuvOrg(), pcSlice->getSPS()->getBitDepths(), pcPic->getPosScalingFactor(refLayerIdc, 0));
            
            m_Enc3DAsymLUTPPS.setDsOrigPic(pcSlice->getBaseColPic(refLayerIdc)->getPicYuvOrg());
            m_Enc3DAsymLUTPicUpdate.setDsOrigPic(pcSlice->getBaseColPic(refLayerIdc)->getPicYuvOrg());
          }
          else
          {
            m_Enc3DAsymLUTPPS.setDsOrigPic(pcPic->getPicYuvOrg());
            m_Enc3DAsymLUTPicUpdate.setDsOrigPic(pcPic->getPicYuvOrg());
          }

          Bool bSignalPPS = m_bSeqFirst;
          bSignalPPS |= m_pcCfg->getGOPSize() > 1 ? pocCurr % m_pcCfg->getIntraPeriod() == 0 : pocCurr % m_pcCfg->getFrameRate() == 0;
          xDetermine3DAsymLUT( pcSlice, pcPic, refLayerIdc, m_pcCfg, bSignalPPS );

          // update PPS in TEncTop and TComPicSym classes
          m_pcEncTop->getPPS()->setCGSOutputBitDepthY( m_Enc3DAsymLUTPPS.getOutputBitDepthY() );
          m_pcEncTop->getPPS()->setCGSOutputBitDepthC( m_Enc3DAsymLUTPPS.getOutputBitDepthC() );
          pcPic->getPicSym()->getPPSToUpdate()->setCGSOutputBitDepthY( m_Enc3DAsymLUTPPS.getOutputBitDepthY() );
          pcPic->getPicSym()->getPPSToUpdate()->setCGSOutputBitDepthC( m_Enc3DAsymLUTPPS.getOutputBitDepthC() );

          m_Enc3DAsymLUTPPS.colorMapping( pcSlice->getBaseColPic(refLayerIdc)->getPicYuvRec(),  m_pColorMappedPic );
          pBaseColRec = m_pColorMappedPic;
        }
#endif

        if( pcPic->requireResampling(refLayerIdc) )
        {
          // check for the sample prediction picture type
          if( pcSlice->getVPS()->isSamplePredictionType( pcSlice->getVPS()->getLayerIdxInVps(m_layerId), pcSlice->getVPS()->getLayerIdxInVps(refLayerId) ) )
          {
            m_pcPredSearch->upsampleBasePic( pcSlice, refLayerIdc, pcPic->getFullPelBaseRec(refLayerIdc), pBaseColRec, pcPic->getPicYuvRec(), pcSlice->getBaseColPic(refLayerIdc)->getSlice(0)->getSPS()->getBitDepth(CHANNEL_TYPE_LUMA), pcSlice->getBaseColPic(refLayerIdc)->getSlice(0)->getSPS()->getBitDepth(CHANNEL_TYPE_CHROMA) );
          }
        }
        else
        {
#if CGS_3D_ASYMLUT 
          pcPic->setFullPelBaseRec( refLayerIdc, pBaseColRec );
#else
          pcPic->setFullPelBaseRec( refLayerIdc, pcSlice->getBaseColPic(refLayerIdc)->getPicYuvRec() );
#endif
        }
        pcSlice->setFullPelBaseRec ( refLayerIdc, pcPic->getFullPelBaseRec(refLayerIdc) );
      }

      // Update the list of active inter-layer pictures
      for ( Int i = 0; i < activeNumILRRefIdxTmp; i++)
      {
        pcSlice->setInterLayerPredLayerIdc( interLayerPredLayerIdcTmp[i], i );
      }

      pcSlice->setActiveNumILRRefIdx( activeNumILRRefIdxTmp );

      if ( pcSlice->getActiveNumILRRefIdx() == 0 )
      {
        // No valid inter-layer pictures -> disable inter-layer prediction
        pcSlice->setInterLayerPredEnabledFlag(false);
      }

      if( pocCurr % m_pcCfg->getIntraPeriod() == 0 )
      {
        if(pcSlice->getVPS()->getCrossLayerIrapAlignFlag())
        {
          TComList<TComPic*> *cListPic = m_ppcTEncTop[pcSlice->getVPS()->getLayerIdxInVps(m_layerId)]->getRefLayerEnc(0)->getListPic();
          TComPic* picLayer0 = pcSlice->getRefPic(*cListPic, pcSlice->getPOC() );
          if(picLayer0)
          {
            pcSlice->setNalUnitType(picLayer0->getSlice(0)->getNalUnitType());
          }
          else
          {
            pcSlice->setNalUnitType(NAL_UNIT_CODED_SLICE_CRA);
          }
        }        
      }

      if( pcSlice->getNalUnitType() >= NAL_UNIT_CODED_SLICE_BLA_W_LP && pcSlice->getNalUnitType() <= NAL_UNIT_CODED_SLICE_CRA )
      {
        if( pcSlice->getActiveNumILRRefIdx() == 0 && m_pcEncTop->getNumDirectRefLayers() == 0 )
        {
          pcSlice->setSliceType(I_SLICE);
        }
        else if( !m_pcEncTop->getElRapSliceTypeB() && pcSlice->getSliceType() == B_SLICE )
        {
          pcSlice->setSliceType(P_SLICE);
        }
      }
    }
#else
    // Set the nal unit type
    pcSlice->setNalUnitType(getNalUnitType(pocCurr, m_iLastIDR, isField));
#endif //#if SVC_EXTENSION

    if(pcSlice->getTemporalLayerNonReferenceFlag())
    {
      if (pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_TRAIL_R &&
#if SVC_EXTENSION
        ( m_iGopSize != 1 || m_ppcTEncTop[pcSlice->getVPS()->getLayerIdxInVps(m_layerId)]->getIntraPeriod() > 1 ) )
#else
          !(m_iGopSize == 1 && pcSlice->getSliceType() == I_SLICE))
#endif
        // Add this condition to avoid POC issues with encoder_intra_main.cfg configuration (see #1127 in bug tracker)
      {
        pcSlice->setNalUnitType(NAL_UNIT_CODED_SLICE_TRAIL_N);
      }
      if(pcSlice->getNalUnitType()==NAL_UNIT_CODED_SLICE_RADL_R)
      {
        pcSlice->setNalUnitType(NAL_UNIT_CODED_SLICE_RADL_N);
      }
      if(pcSlice->getNalUnitType()==NAL_UNIT_CODED_SLICE_RASL_R)
      {
        pcSlice->setNalUnitType(NAL_UNIT_CODED_SLICE_RASL_N);
      }
    }

    if (m_pcCfg->getEfficientFieldIRAPEnabled())
    {
      if ( pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_BLA_W_LP
        || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_BLA_W_RADL
        || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_BLA_N_LP
        || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_W_RADL
        || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_N_LP
        || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_CRA )  // IRAP picture
      {
        m_associatedIRAPType = pcSlice->getNalUnitType();
#if SVC_POC
        m_associatedIRAPPOC = pcSlice->getPOC();
        m_associatedIrapPocBeforeReset = pocCurr;
#else
        m_associatedIRAPPOC = pocCurr;
#endif
      }
      pcSlice->setAssociatedIRAPType(m_associatedIRAPType);
      pcSlice->setAssociatedIRAPPOC(m_associatedIRAPPOC);
#if SVC_POC
      pcSlice->setAssociatedIrapPocBeforeReset(m_associatedIrapPocBeforeReset);
#endif
    }
    // Do decoding refresh marking if any
#if NO_CLRAS_OUTPUT_FLAG
    pcSlice->decodingRefreshMarking(m_pocCRA, m_bRefreshPending, rcListPic, m_pcCfg->getEfficientFieldIRAPEnabled(), m_pcEncTop->getNoClrasOutputFlag());
#else
    pcSlice->decodingRefreshMarking(m_pocCRA, m_bRefreshPending, rcListPic, m_pcCfg->getEfficientFieldIRAPEnabled());
#endif
#if SVC_POC
    // m_pocCRA may have been update here; update m_pocCraWithoutReset
    m_pocCraWithoutReset = m_pocCRA + m_pcEncTop->getPocAdjustmentValue();
#endif
    m_pcEncTop->selectReferencePictureSet(pcSlice, pocCurr, iGOPid);
    if (!m_pcCfg->getEfficientFieldIRAPEnabled())
    {
      if ( pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_BLA_W_LP
        || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_BLA_W_RADL
        || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_BLA_N_LP
        || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_W_RADL
        || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_N_LP
        || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_CRA )  // IRAP picture
      {
        m_associatedIRAPType = pcSlice->getNalUnitType();
        m_associatedIRAPPOC = pocCurr;
      }
      pcSlice->setAssociatedIRAPType(m_associatedIRAPType);
      pcSlice->setAssociatedIRAPPOC(m_associatedIRAPPOC);
    }

    if ((pcSlice->checkThatAllRefPicsAreAvailable(rcListPic, pcSlice->getRPS(), false, m_iLastRecoveryPicPOC, m_pcCfg->getDecodingRefreshType() == 3) != 0) || (pcSlice->isIRAP()) 
      || (m_pcCfg->getEfficientFieldIRAPEnabled() && isField && pcSlice->getAssociatedIRAPType() >= NAL_UNIT_CODED_SLICE_BLA_W_LP && pcSlice->getAssociatedIRAPType() <= NAL_UNIT_CODED_SLICE_CRA && pcSlice->getAssociatedIRAPPOC() == pcSlice->getPOC()+1)
      )
    {
      pcSlice->createExplicitReferencePictureSetFromReference(rcListPic, pcSlice->getRPS(), pcSlice->isIRAP(), m_iLastRecoveryPicPOC, m_pcCfg->getDecodingRefreshType() == 3, m_pcCfg->getEfficientFieldIRAPEnabled());
    }

#if ALIGNED_BUMPING
    pcSlice->checkLeadingPictureRestrictions(rcListPic, true);
#endif
    pcSlice->applyReferencePictureSet(rcListPic, pcSlice->getRPS());

    if(pcSlice->getTLayer() > 0 
      &&  !( pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_RADL_N     // Check if not a leading picture
          || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_RADL_R
          || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_RASL_N
          || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_RASL_R )
        )
    {
      if(pcSlice->isTemporalLayerSwitchingPoint(rcListPic) || pcSlice->getSPS()->getTemporalIdNestingFlag())
      {
        if(pcSlice->getTemporalLayerNonReferenceFlag())
        {
          pcSlice->setNalUnitType(NAL_UNIT_CODED_SLICE_TSA_N);
        }
        else
        {
          pcSlice->setNalUnitType(NAL_UNIT_CODED_SLICE_TSA_R);
        }
      }
      else if(pcSlice->isStepwiseTemporalLayerSwitchingPointCandidate(rcListPic))
      {
        Bool isSTSA=true;
        for(Int ii=iGOPid+1;(ii<m_pcCfg->getGOPSize() && isSTSA==true);ii++)
        {
          Int lTid= m_pcCfg->getGOPEntry(ii).m_temporalId;
          if(lTid==pcSlice->getTLayer())
          {
            const TComReferencePictureSet* nRPS = pcSlice->getSPS()->getRPSList()->getReferencePictureSet(ii);
            for(Int jj=0;jj<nRPS->getNumberOfPictures();jj++)
            {
              if(nRPS->getUsed(jj))
              {
                Int tPoc=m_pcCfg->getGOPEntry(ii).m_POC+nRPS->getDeltaPOC(jj);
                Int kk=0;
                for(kk=0;kk<m_pcCfg->getGOPSize();kk++)
                {
                  if(m_pcCfg->getGOPEntry(kk).m_POC==tPoc)
                  {
                    break;
                  }
                }
                Int tTid=m_pcCfg->getGOPEntry(kk).m_temporalId;
                if(tTid >= pcSlice->getTLayer())
                {
                  isSTSA=false;
                  break;
                }
              }
            }
          }
        }
        if(isSTSA==true)
        {
          if(pcSlice->getTemporalLayerNonReferenceFlag())
          {
            pcSlice->setNalUnitType(NAL_UNIT_CODED_SLICE_STSA_N);
          }
          else
          {
            pcSlice->setNalUnitType(NAL_UNIT_CODED_SLICE_STSA_R);
          }
        }
      }
    }
    arrangeLongtermPicturesInRPS(pcSlice, rcListPic);
    TComRefPicListModification* refPicListModification = pcSlice->getRefPicListModification();
    refPicListModification->setRefPicListModificationFlagL0(0);
    refPicListModification->setRefPicListModificationFlagL1(0);
    pcSlice->setNumRefIdx(REF_PIC_LIST_0,min(m_pcCfg->getGOPEntry(iGOPid).m_numRefPicsActive,pcSlice->getRPS()->getNumberOfPictures()));
    pcSlice->setNumRefIdx(REF_PIC_LIST_1,min(m_pcCfg->getGOPEntry(iGOPid).m_numRefPicsActive,pcSlice->getRPS()->getNumberOfPictures()));

#if SVC_EXTENSION
    if( m_layerId > 0 && pcSlice->getActiveNumILRRefIdx() )
    {
      if( pocCurr > 0 && pcSlice->isRADL() && pcPic->getSlice(0)->getBaseColPic(pcPic->getSlice(0)->getInterLayerPredLayerIdc(0))->getSlice(0)->isRASL() )
      {
        pcSlice->setActiveNumILRRefIdx(0);
        pcSlice->setInterLayerPredEnabledFlag(0);
      }

      if( pcSlice->getNalUnitType() >= NAL_UNIT_CODED_SLICE_BLA_W_LP && pcSlice->getNalUnitType() <= NAL_UNIT_CODED_SLICE_CRA )
      {
        pcSlice->setNumRefIdx(REF_PIC_LIST_0, pcSlice->getActiveNumILRRefIdx());
        pcSlice->setNumRefIdx(REF_PIC_LIST_1, pcSlice->getActiveNumILRRefIdx());
      }
      else
      {
        pcSlice->setNumRefIdx(REF_PIC_LIST_0, pcSlice->getNumRefIdx(REF_PIC_LIST_0)+pcSlice->getActiveNumILRRefIdx());
        pcSlice->setNumRefIdx(REF_PIC_LIST_1, pcSlice->getNumRefIdx(REF_PIC_LIST_1)+pcSlice->getActiveNumILRRefIdx());
      }

      // check for the reference pictures whether there is at least one either temporal picture or ILRP with sample prediction type
      if( pcSlice->getNumRefIdx( REF_PIC_LIST_0 ) - pcSlice->getActiveNumILRRefIdx() == 0 && pcSlice->getNumRefIdx( REF_PIC_LIST_1 ) - pcSlice->getActiveNumILRRefIdx() == 0 )
      {
        Bool foundSamplePredPicture = false;                

        for( Int i = 0; i < pcSlice->getActiveNumILRRefIdx(); i++ )
        {
          if( pcSlice->getVPS()->isSamplePredictionType( pcSlice->getVPS()->getLayerIdxInVps(m_layerId), pcSlice->getInterLayerPredLayerIdc(i) ) )
          {
            foundSamplePredPicture = true;
            break;
          }
        }

        if( !foundSamplePredPicture )
        {
          pcSlice->setSliceType(I_SLICE);
          pcSlice->setInterLayerPredEnabledFlag(0);
          pcSlice->setActiveNumILRRefIdx(0);
        }
      }
    }

   if( ( pcSlice->getTLayer() == 0 && pcSlice->getLayerId() > 0  )    // only for enhancement layer and with temporal layer 0
     && !( pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_RADL_N     
          || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_RADL_R
          || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_RASL_N
          || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_RASL_R 
          || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_W_RADL
          || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR_N_LP
          || pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_CRA
          )
        )
    {
        Bool isSTSA=true;
        Bool isIntra=false;

        for( Int i = 0; i < pcSlice->getLayerId(); i++)
        {
          TComList<TComPic *> *cListPic = m_ppcTEncTop[pcSlice->getVPS()->getLayerIdxInVps(i)]->getListPic();
          TComPic *lowerLayerPic = pcSlice->getRefPic(*cListPic, pcSlice->getPOC());
          if( lowerLayerPic && pcSlice->getVPS()->getDirectDependencyFlag(pcSlice->getLayerIdx(), i) )
          {
            if( lowerLayerPic->getSlice(0)->getSliceType() == I_SLICE)
            { 
              isIntra = true;
            }
          }
        }

        for(Int ii=iGOPid+1; ii < m_pcCfg->getGOPSize() && isSTSA; ii++)
        {
          Int lTid= m_pcCfg->getGOPEntry(ii).m_temporalId;
          if(lTid==pcSlice->getTLayer()) 
          {
            const TComReferencePictureSet* nRPS = pcSlice->getSPS()->getRPSList()->getReferencePictureSet(ii);
            for(Int jj=0; jj<nRPS->getNumberOfPictures(); jj++)
            {
              if(nRPS->getUsed(jj)) 
              {
                Int tPoc=m_pcCfg->getGOPEntry(ii).m_POC+nRPS->getDeltaPOC(jj);
                Int kk=0;
                for(kk=0; kk<m_pcCfg->getGOPSize(); kk++)
                {
                  if(m_pcCfg->getGOPEntry(kk).m_POC==tPoc)
                  {
                    break;
                  }
                }
                Int tTid=m_pcCfg->getGOPEntry(kk).m_temporalId;
                if(tTid >= pcSlice->getTLayer())
                {
                  isSTSA = false;
                  break;
                }
              }
            }
          }
        }
        if(isSTSA==true && isIntra == false)
        {    
          if(pcSlice->getTemporalLayerNonReferenceFlag())
          {
            pcSlice->setNalUnitType(NAL_UNIT_CODED_SLICE_STSA_N);
          }
          else
          {
            pcSlice->setNalUnitType(NAL_UNIT_CODED_SLICE_STSA_R);
          }
        }
    }

    if( pcSlice->getSliceType() == B_SLICE )
    {
      pcSlice->setColFromL0Flag(1-uiColDir);
    }

    //  Set reference list
    if(m_layerId ==  0 || ( m_layerId > 0 && pcSlice->getActiveNumILRRefIdx() == 0 ) )
    {
      pcSlice->setRefPicList( rcListPic );
    }

    if( m_layerId > 0 && pcSlice->getActiveNumILRRefIdx() )
    {
      pcSlice->setILRPic( m_pcEncTop->getIlpList() );
#if VIEW_SCALABILITY 
      pcSlice->setRefPicListModificationSvc(m_pcEncTop->getIlpList());
#else
      pcSlice->setRefPicListModificationSvc();
#endif
      pcSlice->setRefPicList( rcListPic, false, m_pcEncTop->getIlpList());

      if( pcSlice->getMFMEnabledFlag() )
      {
        Bool found         = false;
        UInt ColFromL0Flag = pcSlice->getColFromL0Flag();
        UInt ColRefIdx     = pcSlice->getColRefIdx();

        for(Int colIdx = 0; colIdx < pcSlice->getNumRefIdx( RefPicList(1 - ColFromL0Flag) ); colIdx++) 
        {
          RefPicList refList = RefPicList(1 - ColFromL0Flag);
          TComPic* refPic = pcSlice->getRefPic(refList, colIdx);

          // It is a requirement of bitstream conformance when the collocated picture, used for temporal motion vector prediction, is an inter-layer reference picture, 
          // VpsInterLayerMotionPredictionEnabled[ LayerIdxInVps[ currLayerId ] ][ LayerIdxInVps[ rLId ] ] shall be equal to 1, where rLId is set equal to nuh_layer_id of the inter-layer picture.
          if( refPic->isILR(m_layerId) && pcSlice->getVPS()->isMotionPredictionType( pcSlice->getVPS()->getLayerIdxInVps(m_layerId), refPic->getLayerIdx() )            
            && pcSlice->getBaseColPic( *m_ppcTEncTop[refPic->getLayerIdx()]->getListPic() )->checkSameRefInfo() == true ) 
          { 
            ColRefIdx = colIdx; 
            found = true;
            break; 
          }
        }

        if( found == false )
        {
          ColFromL0Flag = 1 - ColFromL0Flag;
          for(Int colIdx = 0; colIdx < pcSlice->getNumRefIdx( RefPicList(1 - ColFromL0Flag) ); colIdx++) 
          {
            RefPicList refList = RefPicList(1 - ColFromL0Flag);
            TComPic* refPic = pcSlice->getRefPic(refList, colIdx);

            // It is a requirement of bitstream conformance when the collocated picture, used for temporal motion vector prediction, is an inter-layer reference picture, 
            // VpsInterLayerMotionPredictionEnabled[ LayerIdxInVps[ currLayerId ] ][ LayerIdxInVps[ rLId ] ] shall be equal to 1, where rLId is set equal to nuh_layer_id of the inter-layer picture.
            if( refPic->isILR(m_layerId) && pcSlice->getVPS()->isMotionPredictionType( pcSlice->getVPS()->getLayerIdxInVps(m_layerId), refPic->getLayerIdx() )
              && pcSlice->getBaseColPic( *m_ppcTEncTop[refPic->getLayerIdx()]->getListPic() )->checkSameRefInfo() == true ) 
            { 
              ColRefIdx = colIdx; 
              found = true; 
              break; 
            } 
          }
        }

        if(found == true)
        {
          pcSlice->setColFromL0Flag(ColFromL0Flag);
          pcSlice->setColRefIdx(ColRefIdx);
        }
      }
    }
#else //SVC_EXTENSION
    //  Set reference list
    pcSlice->setRefPicList ( rcListPic );
#endif //#if SVC_EXTENSION

    //  Slice info. refinement
    if ( (pcSlice->getSliceType() == B_SLICE) && (pcSlice->getNumRefIdx(REF_PIC_LIST_1) == 0) )
    {
      pcSlice->setSliceType ( P_SLICE );
    }
    pcSlice->setEncCABACTableIdx(m_pcSliceEncoder->getEncCABACTableIdx());

    if (pcSlice->getSliceType() == B_SLICE)
    {
#if !SVC_EXTENSION
      pcSlice->setColFromL0Flag(1-uiColDir);
#endif
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
      for (iRefIdx = 0; iRefIdx < pcSlice->getNumRefIdx(REF_PIC_LIST_1) && bLowDelay; iRefIdx++)
      {
        if ( pcSlice->getRefPic(REF_PIC_LIST_1, iRefIdx)->getPOC() > iCurrPOC )
        {
          bLowDelay = false;
        }
      }

      pcSlice->setCheckLDC(bLowDelay);
    }
    else
    {
      pcSlice->setCheckLDC(true);
    }

    uiColDir = 1-uiColDir;

    //-------------------------------------------------------------
    pcSlice->setRefPOCList();

    pcSlice->setList1IdxToList0Idx();

    if (m_pcEncTop->getTMVPModeId() == 2)
    {
      if (iGOPid == 0) // first picture in SOP (i.e. forward B)
      {
        pcSlice->setEnableTMVPFlag(0);
      }
      else
      {
        // Note: pcSlice->getColFromL0Flag() is assumed to be always 0 and getcolRefIdx() is always 0.
        pcSlice->setEnableTMVPFlag(1);
      }
    }
    else if (m_pcEncTop->getTMVPModeId() == 1)
    {
#if SVC_EXTENSION
      if( pcSlice->getIdrPicFlag() )
      {
        pcSlice->setEnableTMVPFlag(0);
      }
      else
#endif
      pcSlice->setEnableTMVPFlag(1);
    }
    else
    {
      pcSlice->setEnableTMVPFlag(0);
    }

#if SVC_EXTENSION
    if( m_layerId > 0 && !pcSlice->isIntra() )
    {
      Int colFromL0Flag = 1;
      Int colRefIdx = 0;

      // check whether collocated picture is valid
      if( pcSlice->getEnableTMVPFlag() )
      {
        colFromL0Flag = pcSlice->getColFromL0Flag();
        colRefIdx = pcSlice->getColRefIdx();

        TComPic* refPic = pcSlice->getRefPic(RefPicList(1-colFromL0Flag), colRefIdx);

        assert( refPic );

        // It is a requirement of bitstream conformance when the collocated picture, used for temporal motion vector prediction, is an inter-layer reference picture, 
        // VpsInterLayerMotionPredictionEnabled[ LayerIdxInVps[ currLayerId ] ][ LayerIdxInVps[ rLId ] ] shall be equal to 1, where rLId is set equal to nuh_layer_id of the inter-layer picture.
        if( refPic->isILR(m_layerId) && !pcSlice->getVPS()->isMotionPredictionType( pcSlice->getVPS()->getLayerIdxInVps(m_layerId), refPic->getLayerIdx() ) )
        {
          pcSlice->setEnableTMVPFlag(false);
          pcSlice->setMFMEnabledFlag(false);
          colRefIdx = 0;
        }
      }

      // remove motion only ILRP from the end of the colFromL0Flag reference picture list
      RefPicList refList = RefPicList(colFromL0Flag);
      Int numRefIdx = pcSlice->getNumRefIdx(refList);

      if( numRefIdx > 0 )
      {
        for( Int refIdx = pcSlice->getNumRefIdx(refList) - 1; refIdx > 0; refIdx-- )
        {
          TComPic* refPic = pcSlice->getRefPic(refList, refIdx);

          if( !refPic->isILR(m_layerId) || ( refPic->isILR(m_layerId) && pcSlice->getVPS()->isSamplePredictionType( pcSlice->getVPS()->getLayerIdxInVps(m_layerId), refPic->getLayerIdx() ) ) )
          {
            break;
          }
          else
          {
            assert( numRefIdx > 1 );
            numRefIdx--;              
          }
        }

        pcSlice->setNumRefIdx( refList, numRefIdx );
      }

      // remove motion only ILRP from the end of the (1-colFromL0Flag) reference picture list up to colRefIdx
      refList = RefPicList(1 - colFromL0Flag);
      numRefIdx = pcSlice->getNumRefIdx(refList);

      if( numRefIdx > 0 )
      {
        for( Int refIdx = pcSlice->getNumRefIdx(refList) - 1; refIdx > colRefIdx; refIdx-- )
        {
          TComPic* refPic = pcSlice->getRefPic(refList, refIdx);

          if( !refPic->isILR(m_layerId) || ( refPic->isILR(m_layerId) && pcSlice->getVPS()->isSamplePredictionType( pcSlice->getVPS()->getLayerIdxInVps(m_layerId), refPic->getLayerIdx() ) ) )
          {
            break;
          }
          else
          {
            assert( numRefIdx > 1 );
            numRefIdx--;              
          }
        }

        pcSlice->setNumRefIdx( refList, numRefIdx );
      }

      assert( pcSlice->getNumRefIdx(REF_PIC_LIST_0) > 0 && ( pcSlice->isInterP() || (pcSlice->isInterB() && pcSlice->getNumRefIdx(REF_PIC_LIST_1) > 0) ) );
    }
#endif

    /////////////////////////////////////////////////////////////////////////////////////////////////// Compress a slice
    // set adaptive search range for non-intra-slices
    if (m_pcCfg->getUseASR() && pcSlice->getSliceType()!=I_SLICE)
    {
      m_pcSliceEncoder->setSearchRange(pcSlice);
    }

    Bool bGPBcheck=false;
    if ( pcSlice->getSliceType() == B_SLICE)
    {
      if ( pcSlice->getNumRefIdx(RefPicList( 0 ) ) == pcSlice->getNumRefIdx(RefPicList( 1 ) ) )
      {
        bGPBcheck=true;
        Int i;
        for ( i=0; i < pcSlice->getNumRefIdx(RefPicList( 1 ) ); i++ )
        {
          if ( pcSlice->getRefPOC(RefPicList(1), i) != pcSlice->getRefPOC(RefPicList(0), i) )
          {
            bGPBcheck=false;
            break;
          }
        }
      }
    }
    if(bGPBcheck)
    {
      pcSlice->setMvdL1ZeroFlag(true);
    }
    else
    {
      pcSlice->setMvdL1ZeroFlag(false);
    }
    pcPic->getSlice(pcSlice->getSliceIdx())->setMvdL1ZeroFlag(pcSlice->getMvdL1ZeroFlag());


    Double lambda            = 0.0;
    Int actualHeadBits       = 0;
    Int actualTotalBits      = 0;
    Int estimatedBits        = 0;
    Int tmpBitsBeforeWriting = 0;
    if ( m_pcCfg->getUseRateCtrl() ) // TODO: does this work with multiple slices and slice-segments?
    {
      Int frameLevel = m_pcRateCtrl->getRCSeq()->getGOPID2Level( iGOPid );
      if ( pcPic->getSlice(0)->getSliceType() == I_SLICE )
      {
        frameLevel = 0;
      }
      m_pcRateCtrl->initRCPic( frameLevel );
      estimatedBits = m_pcRateCtrl->getRCPic()->getTargetBits();

#if U0132_TARGET_BITS_SATURATION
      if (m_pcRateCtrl->getCpbSaturationEnabled() && frameLevel != 0)
      {
        Int estimatedCpbFullness = m_pcRateCtrl->getCpbState() + m_pcRateCtrl->getBufferingRate();

        // prevent overflow
        if (estimatedCpbFullness - estimatedBits > (Int)(m_pcRateCtrl->getCpbSize()*0.9f))
        {
          estimatedBits = estimatedCpbFullness - (Int)(m_pcRateCtrl->getCpbSize()*0.9f);
        }

        estimatedCpbFullness -= m_pcRateCtrl->getBufferingRate();
        // prevent underflow
#if V0078_ADAPTIVE_LOWER_BOUND
        if (estimatedCpbFullness - estimatedBits < m_pcRateCtrl->getRCPic()->getLowerBound())
        {
          estimatedBits = max(200, estimatedCpbFullness - m_pcRateCtrl->getRCPic()->getLowerBound());
        }
#else
        if (estimatedCpbFullness - estimatedBits < (Int)(m_pcRateCtrl->getCpbSize()*0.1f))
        {
          estimatedBits = max(200, estimatedCpbFullness - (Int)(m_pcRateCtrl->getCpbSize()*0.1f));
        }
#endif

        m_pcRateCtrl->getRCPic()->setTargetBits(estimatedBits);
      }
#endif

      Int sliceQP = m_pcCfg->getInitialQP();
#if SVC_EXTENSION
      if ( ( pocCurr == 0 && m_pcCfg->getInitialQP() > 0 ) || ( frameLevel == 0 && m_pcCfg->getForceIntraQP() ) ) // QP is specified
#else
      if ( ( pcSlice->getPOC() == 0 && m_pcCfg->getInitialQP() > 0 ) || ( frameLevel == 0 && m_pcCfg->getForceIntraQP() ) ) // QP is specified
#endif
      {
        Int    NumberBFrames = ( m_pcCfg->getGOPSize() - 1 );
        Double dLambda_scale = 1.0 - Clip3( 0.0, 0.5, 0.05*(Double)NumberBFrames );
        Double dQPFactor     = 0.57*dLambda_scale;
        Int    SHIFT_QP      = 12;
        Int    bitdepth_luma_qp_scale = 0;
        Double qp_temp = (Double) sliceQP + bitdepth_luma_qp_scale - SHIFT_QP;
        lambda = dQPFactor*pow( 2.0, qp_temp/3.0 );
      }
      else if ( frameLevel == 0 )   // intra case, but use the model
      {
        m_pcSliceEncoder->calCostSliceI(pcPic); // TODO: This only analyses the first slice segment - what about the others?

        if ( m_pcCfg->getIntraPeriod() != 1 )   // do not refine allocated bits for all intra case
        {
          Int bits = m_pcRateCtrl->getRCSeq()->getLeftAverageBits();
          bits = m_pcRateCtrl->getRCPic()->getRefineBitsForIntra( bits );

#if U0132_TARGET_BITS_SATURATION
          if (m_pcRateCtrl->getCpbSaturationEnabled() )
          {
            Int estimatedCpbFullness = m_pcRateCtrl->getCpbState() + m_pcRateCtrl->getBufferingRate();

            // prevent overflow
            if (estimatedCpbFullness - bits > (Int)(m_pcRateCtrl->getCpbSize()*0.9f))
            {
              bits = estimatedCpbFullness - (Int)(m_pcRateCtrl->getCpbSize()*0.9f);
            }

            estimatedCpbFullness -= m_pcRateCtrl->getBufferingRate();
            // prevent underflow
#if V0078_ADAPTIVE_LOWER_BOUND
            if (estimatedCpbFullness - bits < m_pcRateCtrl->getRCPic()->getLowerBound())
            {
              bits = estimatedCpbFullness - m_pcRateCtrl->getRCPic()->getLowerBound();
            }
#else
            if (estimatedCpbFullness - bits < (Int)(m_pcRateCtrl->getCpbSize()*0.1f))
            {
              bits = estimatedCpbFullness - (Int)(m_pcRateCtrl->getCpbSize()*0.1f);
            }
#endif
          }
#endif

          if ( bits < 200 )
          {
            bits = 200;
          }
          m_pcRateCtrl->getRCPic()->setTargetBits( bits );
        }

        list<TEncRCPic*> listPreviousPicture = m_pcRateCtrl->getPicList();
        m_pcRateCtrl->getRCPic()->getLCUInitTargetBits();
        lambda  = m_pcRateCtrl->getRCPic()->estimatePicLambda( listPreviousPicture, pcSlice->getSliceType());
        sliceQP = m_pcRateCtrl->getRCPic()->estimatePicQP( lambda, listPreviousPicture );
      }
      else    // normal case
      {
        list<TEncRCPic*> listPreviousPicture = m_pcRateCtrl->getPicList();
        lambda  = m_pcRateCtrl->getRCPic()->estimatePicLambda( listPreviousPicture, pcSlice->getSliceType());
        sliceQP = m_pcRateCtrl->getRCPic()->estimatePicQP( lambda, listPreviousPicture );
      }

      sliceQP = Clip3( -pcSlice->getSPS()->getQpBDOffset(CHANNEL_TYPE_LUMA), MAX_QP, sliceQP );
      m_pcRateCtrl->getRCPic()->setPicEstQP( sliceQP );

      m_pcSliceEncoder->resetQP( pcPic, sliceQP, lambda );
    }

    UInt uiNumSliceSegments = 1;

#if AVC_BASE
    if( m_layerId == 0 && m_pcEncTop->getVPS()->getNonHEVCBaseLayerFlag() )
    {
      pcPic->getPicYuvOrg()->copyToPic( pcPic->getPicYuvRec() );

      // Calculate for the base layer to be used in EL as Inter layer reference
      if( m_pcEncTop->getInterLayerWeightedPredFlag() )
      {
        m_pcSliceEncoder->estimateILWpParam( pcSlice );
      }

      return;
    }
#endif

    // Allocate some coders, now the number of tiles are known.
    const Int numSubstreamsColumns = (pcSlice->getPPS()->getNumTileColumnsMinus1() + 1);
    const Int numSubstreamRows     = pcSlice->getPPS()->getEntropyCodingSyncEnabledFlag() ? pcPic->getFrameHeightInCtus() : (pcSlice->getPPS()->getNumTileRowsMinus1() + 1);
    const Int numSubstreams        = numSubstreamRows * numSubstreamsColumns;
    std::vector<TComOutputBitstream> substreamsOut(numSubstreams);

    // now compress (trial encode) the various slice segments (slices, and dependent slices)
    {
      const UInt numberOfCtusInFrame=pcPic->getPicSym()->getNumberOfCtusInFrame();
      pcSlice->setSliceCurStartCtuTsAddr( 0 );
      pcSlice->setSliceSegmentCurStartCtuTsAddr( 0 );

      for(UInt nextCtuTsAddr = 0; nextCtuTsAddr < numberOfCtusInFrame; )
      {
        m_pcSliceEncoder->precompressSlice( pcPic );
        m_pcSliceEncoder->compressSlice   ( pcPic, false, false );

        const UInt curSliceSegmentEnd = pcSlice->getSliceSegmentCurEndCtuTsAddr();
        if (curSliceSegmentEnd < numberOfCtusInFrame)
        {
          const Bool bNextSegmentIsDependentSlice=curSliceSegmentEnd<pcSlice->getSliceCurEndCtuTsAddr();
          const UInt sliceBits=pcSlice->getSliceBits();
          pcPic->allocateNewSlice();
          // prepare for next slice
          pcPic->setCurrSliceIdx                    ( uiNumSliceSegments );
          m_pcSliceEncoder->setSliceIdx             ( uiNumSliceSegments   );
          pcSlice = pcPic->getSlice                 ( uiNumSliceSegments   );
          assert(pcSlice->getPPS()!=0);
          pcSlice->copySliceInfo                    ( pcPic->getSlice(uiNumSliceSegments-1)  );
          pcSlice->setSliceIdx                      ( uiNumSliceSegments   );
          if (bNextSegmentIsDependentSlice)
          {
            pcSlice->setSliceBits(sliceBits);
          }
          else
          {
            pcSlice->setSliceCurStartCtuTsAddr      ( curSliceSegmentEnd );
            pcSlice->setSliceBits(0);
          }
          pcSlice->setDependentSliceSegmentFlag(bNextSegmentIsDependentSlice);
          pcSlice->setSliceSegmentCurStartCtuTsAddr ( curSliceSegmentEnd );
          // TODO: optimise cabac_init during compress slice to improve multi-slice operation
          // pcSlice->setEncCABACTableIdx(m_pcSliceEncoder->getEncCABACTableIdx());
          uiNumSliceSegments ++;
        }
        nextCtuTsAddr = curSliceSegmentEnd;
      }
    }

#if N0383_IL_CONSTRAINED_TILE_SETS_SEI
    if (m_pcCfg->getInterLayerConstrainedTileSetsSEIEnabled())
    {
      xBuildTileSetsMap(pcPic->getPicSym());
    }
#endif

    duData.clear();
    pcSlice = pcPic->getSlice(0);

    // SAO parameter estimation using non-deblocked pixels for CTU bottom and right boundary areas
    if( pcSlice->getSPS()->getUseSAO() && m_pcCfg->getSaoCtuBoundary() )
    {
      m_pcSAO->getPreDBFStatistics(pcPic);
    }

    //-- Loop filter
    Bool bLFCrossTileBoundary = pcSlice->getPPS()->getLoopFilterAcrossTilesEnabledFlag();
    m_pcLoopFilter->setCfg(bLFCrossTileBoundary);
    if ( m_pcCfg->getDeblockingFilterMetric() )
    {
#if W0038_DB_OPT
      if ( m_pcCfg->getDeblockingFilterMetric()==2 )
      {
        applyDeblockingFilterParameterSelection(pcPic, uiNumSliceSegments, iGOPid);
      }
      else
      {
#endif
        applyDeblockingFilterMetric(pcPic, uiNumSliceSegments);
#if W0038_DB_OPT
      }
#endif
    }
    m_pcLoopFilter->loopFilterPic( pcPic );

    /////////////////////////////////////////////////////////////////////////////////////////////////// File writing
    // Set entropy coder
    m_pcEntropyCoder->setEntropyCoder   ( m_pcCavlcCoder );

    if ( m_bSeqFirst )
    {
      // write various parameter sets
      actualTotalBits += xWriteParameterSets(accessUnit, pcSlice);

      // create prefix SEI messages at the beginning of the sequence
      assert(leadingSeiMessages.empty());
      xCreateIRAPLeadingSEIMessages(leadingSeiMessages, pcSlice->getSPS(), pcSlice->getPPS());

      m_bSeqFirst = false;
    }
#if SVC_EXTENSION && CGS_3D_ASYMLUT
    else if( m_pcCfg->getCGSFlag() && pcSlice->getLayerId() && pcSlice->getCGSOverWritePPS() )
    {
      OutputNALUnit nalu(NAL_UNIT_PPS, 0, m_layerId);
      m_pcEntropyCoder->setBitstream(&nalu.m_Bitstream);
      m_pcEntropyCoder->encodePPS(pcSlice->getPPS() , &m_Enc3DAsymLUTPPS );
      accessUnit.push_back(new NALUnitEBSP(nalu));
    }
#endif

    if (m_pcCfg->getAccessUnitDelimiter())
    {
      xWriteAccessUnitDelimiter(accessUnit, pcSlice);
    }

    // reset presence of BP SEI indication
    m_bufferingPeriodSEIPresentInAU = false;
    // create prefix SEI associated with a picture
    xCreatePerPictureSEIMessages(iGOPid, leadingSeiMessages, nestedSeiMessages, pcSlice);

    /* use the main bitstream buffer for storing the marshalled picture */
    m_pcEntropyCoder->setBitstream(NULL);

    pcSlice = pcPic->getSlice(0);


#if HIGHER_LAYER_IRAP_SKIP_FLAG
    if ( pcSlice->getSPS()->getUseSAO() && !( m_pcEncTop->getSkipPictureAtArcSwitch() && m_pcEncTop->getAdaptiveResolutionChange() > 0 && pcSlice->getLayerId() == 1 && pcSlice->getPOC() == m_pcEncTop->getAdaptiveResolutionChange()) )
#else
    if (pcSlice->getSPS()->getUseSAO())
#endif
    {
      Bool sliceEnabled[MAX_NUM_COMPONENT];
      TComBitCounter tempBitCounter;
      tempBitCounter.resetBits();
      m_pcEncTop->getRDGoOnSbacCoder()->setBitstream(&tempBitCounter);
      m_pcSAO->initRDOCabacCoder(m_pcEncTop->getRDGoOnSbacCoder(), pcSlice);
#if OPTIONAL_RESET_SAO_ENCODING_AFTER_IRAP
      m_pcSAO->SAOProcess(pcPic, sliceEnabled, pcPic->getSlice(0)->getLambdas(),
                          m_pcCfg->getTestSAODisableAtPictureLevel(),
                          m_pcCfg->getSaoEncodingRate(),
                          m_pcCfg->getSaoEncodingRateChroma(),
                          m_pcCfg->getSaoCtuBoundary(),
                          m_pcCfg->getSaoResetEncoderStateAfterIRAP());
#else
      m_pcSAO->SAOProcess(pcPic, sliceEnabled, pcPic->getSlice(0)->getLambdas(), m_pcCfg->getTestSAODisableAtPictureLevel(), m_pcCfg->getSaoEncodingRate(), m_pcCfg->getSaoEncodingRateChroma(), m_pcCfg->getSaoCtuBoundary());
#endif
      m_pcSAO->PCMLFDisableProcess(pcPic);
      m_pcEncTop->getRDGoOnSbacCoder()->setBitstream(NULL);

      //assign SAO slice header
      for(Int s=0; s< uiNumSliceSegments; s++)
      {
        pcPic->getSlice(s)->setSaoEnabledFlag(CHANNEL_TYPE_LUMA, sliceEnabled[COMPONENT_Y]);
        assert(sliceEnabled[COMPONENT_Cb] == sliceEnabled[COMPONENT_Cr]);
        pcPic->getSlice(s)->setSaoEnabledFlag(CHANNEL_TYPE_CHROMA, sliceEnabled[COMPONENT_Cb]);
      }
    }

    // pcSlice is currently slice 0.
    std::size_t binCountsInNalUnits   = 0; // For implementation of cabac_zero_word stuffing (section 7.4.3.10)
    std::size_t numBytesInVclNalUnits = 0; // For implementation of cabac_zero_word stuffing (section 7.4.3.10)

    for( UInt sliceSegmentStartCtuTsAddr = 0, sliceIdxCount=0; sliceSegmentStartCtuTsAddr < pcPic->getPicSym()->getNumberOfCtusInFrame(); sliceIdxCount++, sliceSegmentStartCtuTsAddr=pcSlice->getSliceSegmentCurEndCtuTsAddr() )
    {
      pcSlice = pcPic->getSlice(sliceIdxCount);
      if(sliceIdxCount > 0 && pcSlice->getSliceType()!= I_SLICE)
      {
        pcSlice->checkColRefIdx(sliceIdxCount, pcPic);
      }
      pcPic->setCurrSliceIdx(sliceIdxCount);
      m_pcSliceEncoder->setSliceIdx(sliceIdxCount);

      pcSlice->setRPS(pcPic->getSlice(0)->getRPS());
      pcSlice->setRPSidx(pcPic->getSlice(0)->getRPSidx());

      for ( UInt ui = 0 ; ui < numSubstreams; ui++ )
      {
        substreamsOut[ui].clear();
      }

      m_pcEntropyCoder->setEntropyCoder   ( m_pcCavlcCoder );
      m_pcEntropyCoder->resetEntropy      ( pcSlice );
      /* start slice NALunit */
#if SVC_EXTENSION
      OutputNALUnit nalu( pcSlice->getNalUnitType(), pcSlice->getTLayer(), m_layerId );
#else
      OutputNALUnit nalu( pcSlice->getNalUnitType(), pcSlice->getTLayer() );
#endif
      m_pcEntropyCoder->setBitstream(&nalu.m_Bitstream);

#if SVC_EXTENSION
      if( pcSlice->isIRAP() )
      {
        //the inference for NoOutputPriorPicsFlag
        // KJS: This cannot happen at the encoder
        if (!m_bFirst && pcSlice->isIRAP() && m_noRaslOutputFlag)
        {
          if (pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_CRA)
          {
            pcSlice->setNoOutputPriorPicsFlag(true);
          }
        }
      }
#else
      pcSlice->setNoRaslOutputFlag(false);
      if (pcSlice->isIRAP())
      {
        if (pcSlice->getNalUnitType() >= NAL_UNIT_CODED_SLICE_BLA_W_LP && pcSlice->getNalUnitType() <= NAL_UNIT_CODED_SLICE_IDR_N_LP)
        {
          pcSlice->setNoRaslOutputFlag(true);
        }
        //the inference for NoOutputPriorPicsFlag
        // KJS: This cannot happen at the encoder
        if (!m_bFirst && pcSlice->isIRAP() && pcSlice->getNoRaslOutputFlag())
        {
          if (pcSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_CRA)
          {
            pcSlice->setNoOutputPriorPicsFlag(true);
          }
        }
      }
#endif

      pcSlice->setEncCABACTableIdx(m_pcSliceEncoder->getEncCABACTableIdx());

      tmpBitsBeforeWriting = m_pcEntropyCoder->getNumberOfWrittenBits();
      m_pcEntropyCoder->encodeSliceHeader(pcSlice);
      actualHeadBits += ( m_pcEntropyCoder->getNumberOfWrittenBits() - tmpBitsBeforeWriting );

      pcSlice->setFinalized(true);

      pcSlice->clearSubstreamSizes(  );
      {
        UInt numBinsCoded = 0;
        m_pcSliceEncoder->encodeSlice(pcPic, &(substreamsOut[0]), numBinsCoded);
        binCountsInNalUnits+=numBinsCoded;
      }

      {
        // Construct the final bitstream by concatenating substreams.
        // The final bitstream is either nalu.m_Bitstream or pcBitstreamRedirect;
        // Complete the slice header info.
        m_pcEntropyCoder->setEntropyCoder   ( m_pcCavlcCoder );
        m_pcEntropyCoder->setBitstream(&nalu.m_Bitstream);
#if SVC_EXTENSION
        tmpBitsBeforeWriting = m_pcEntropyCoder->getNumberOfWrittenBits();
        m_pcEntropyCoder->encodeTilesWPPEntryPoint( pcSlice );
        actualHeadBits += ( m_pcEntropyCoder->getNumberOfWrittenBits() - tmpBitsBeforeWriting );
        m_pcEntropyCoder->encodeSliceHeaderExtn( pcSlice, actualHeadBits );
#else
        m_pcEntropyCoder->encodeTilesWPPEntryPoint( pcSlice );
#endif

        // Append substreams...
        TComOutputBitstream *pcOut = pcBitstreamRedirect;
        const Int numZeroSubstreamsAtStartOfSlice  = pcPic->getSubstreamForCtuAddr(pcSlice->getSliceSegmentCurStartCtuTsAddr(), false, pcSlice);
        const Int numSubstreamsToCode  = pcSlice->getNumberOfSubstreamSizes()+1;
        for ( UInt ui = 0 ; ui < numSubstreamsToCode; ui++ )
        {
          pcOut->addSubstream(&(substreamsOut[ui+numZeroSubstreamsAtStartOfSlice]));
        }
      }

      // If current NALU is the first NALU of slice (containing slice header) and more NALUs exist (due to multiple dependent slices) then buffer it.
      // If current NALU is the last NALU of slice and a NALU was buffered, then (a) Write current NALU (b) Update an write buffered NALU at approproate location in NALU list.
      Bool bNALUAlignedWrittenToList    = false; // used to ensure current NALU is not written more than once to the NALU list.
      xAttachSliceDataToNalUnit(nalu, pcBitstreamRedirect);
      accessUnit.push_back(new NALUnitEBSP(nalu));
      actualTotalBits += UInt(accessUnit.back()->m_nalUnitData.str().size()) * 8;
      numBytesInVclNalUnits += (std::size_t)(accessUnit.back()->m_nalUnitData.str().size());
      bNALUAlignedWrittenToList = true;

      if (!bNALUAlignedWrittenToList)
      {
        nalu.m_Bitstream.writeAlignZero();
        accessUnit.push_back(new NALUnitEBSP(nalu));
      }

      if( ( m_pcCfg->getPictureTimingSEIEnabled() || m_pcCfg->getDecodingUnitInfoSEIEnabled() ) &&
          ( pcSlice->getSPS()->getVuiParametersPresentFlag() ) &&
          ( ( pcSlice->getSPS()->getVuiParameters()->getHrdParameters()->getNalHrdParametersPresentFlag() )
         || ( pcSlice->getSPS()->getVuiParameters()->getHrdParameters()->getVclHrdParametersPresentFlag() ) ) &&
          ( pcSlice->getSPS()->getVuiParameters()->getHrdParameters()->getSubPicCpbParamsPresentFlag() ) )
      {
          UInt numNalus = 0;
        UInt numRBSPBytes = 0;
        for (AccessUnit::const_iterator it = accessUnit.begin(); it != accessUnit.end(); it++)
        {
          numRBSPBytes += UInt((*it)->m_nalUnitData.str().size());
          numNalus ++;
        }
        duData.push_back(DUData());
        duData.back().accumBitsDU = ( numRBSPBytes << 3 );
        duData.back().accumNalsDU = numNalus;
      }
    } // end iteration over slices

    // cabac_zero_words processing
    cabac_zero_word_padding(pcSlice, pcPic, binCountsInNalUnits, numBytesInVclNalUnits, accessUnit.back()->m_nalUnitData, m_pcCfg->getCabacZeroWordPaddingEnabled());

    pcPic->compressMotion();

    //-- For time output for each slice
    Double dEncTime = (Double)(clock()-iBeforeTime) / CLOCKS_PER_SEC;

    std::string digestStr;
    if (m_pcCfg->getDecodedPictureHashSEIType()!=HASHTYPE_NONE)
    {
      SEIDecodedPictureHash *decodedPictureHashSei = new SEIDecodedPictureHash();
      m_seiEncoder.initDecodedPictureHashSEI(decodedPictureHashSei, pcPic, digestStr, pcSlice->getSPS()->getBitDepths());
      trailingSeiMessages.push_back(decodedPictureHashSei);
    }

    m_pcCfg->setEncodedFlag(iGOPid, true);

    Double PSNR_Y;
    xCalculateAddPSNRs( isField, isTff, iGOPid, pcPic, accessUnit, rcListPic, dEncTime, snr_conversion, printFrameMSE, &PSNR_Y );
    
    // Only produce the Green Metadata SEI message with the last picture.
    if( m_pcCfg->getSEIGreenMetadataInfoSEIEnable() && pcSlice->getPOC() == ( m_pcCfg->getFramesToBeEncoded() - 1 )  )
    {
      SEIGreenMetadataInfo *seiGreenMetadataInfo = new SEIGreenMetadataInfo;
      m_seiEncoder.initSEIGreenMetadataInfo(seiGreenMetadataInfo, (UInt)(PSNR_Y * 100 + 0.5));
      trailingSeiMessages.push_back(seiGreenMetadataInfo);
    }
    
#if O0164_MULTI_LAYER_HRD
    xWriteTrailingSEIMessages(trailingSeiMessages, accessUnit, pcSlice->getTLayer(), pcSlice->getVPS(), pcSlice->getSPS());
#else
    xWriteTrailingSEIMessages(trailingSeiMessages, accessUnit, pcSlice->getTLayer(), pcSlice->getSPS());
#endif
        
    printHash(m_pcCfg->getDecodedPictureHashSEIType(), digestStr);

    if ( m_pcCfg->getUseRateCtrl() )
    {
      Double avgQP     = m_pcRateCtrl->getRCPic()->calAverageQP();
      Double avgLambda = m_pcRateCtrl->getRCPic()->calAverageLambda();
      if ( avgLambda < 0.0 )
      {
        avgLambda = lambda;
      }

      m_pcRateCtrl->getRCPic()->updateAfterPicture( actualHeadBits, actualTotalBits, avgQP, avgLambda, pcSlice->getSliceType());
      m_pcRateCtrl->getRCPic()->addToPictureLsit( m_pcRateCtrl->getPicList() );

      m_pcRateCtrl->getRCSeq()->updateAfterPic( actualTotalBits );
      if ( pcSlice->getSliceType() != I_SLICE )
      {
        m_pcRateCtrl->getRCGOP()->updateAfterPicture( actualTotalBits );
      }
      else    // for intra picture, the estimated bits are used to update the current status in the GOP
      {
        m_pcRateCtrl->getRCGOP()->updateAfterPicture( estimatedBits );
      }
#if U0132_TARGET_BITS_SATURATION
      if (m_pcRateCtrl->getCpbSaturationEnabled())
      {
        m_pcRateCtrl->updateCpbState(actualTotalBits);
        printf(" [CPB %6d bits]", m_pcRateCtrl->getCpbState());
      }
#endif
    }

    xCreatePictureTimingSEI(m_pcCfg->getEfficientFieldIRAPEnabled()?effFieldIRAPMap.GetIRAPGOPid():0, leadingSeiMessages, nestedSeiMessages, duInfoSeiMessages, pcSlice, isField, duData);
    if (m_pcCfg->getScalableNestingSEIEnabled())
    {
      xCreateScalableNestingSEI (leadingSeiMessages, nestedSeiMessages);
    }
#if O0164_MULTI_LAYER_HRD
    xWriteLeadingSEIMessages(leadingSeiMessages, duInfoSeiMessages, accessUnit, pcSlice->getTLayer(), pcSlice->getVPS(), pcSlice->getSPS(), duData);
    xWriteDuSEIMessages(duInfoSeiMessages, accessUnit, pcSlice->getTLayer(), pcSlice->getVPS(), pcSlice->getSPS(), duData);
#else
    xWriteLeadingSEIMessages(leadingSeiMessages, duInfoSeiMessages, accessUnit, pcSlice->getTLayer(), pcSlice->getSPS(), duData);
    xWriteDuSEIMessages(duInfoSeiMessages, accessUnit, pcSlice->getTLayer(), pcSlice->getSPS(), duData);
#endif

#if SVC_EXTENSION
    m_prevPicHasEos = false;
    if (m_pcCfg->getLayerSwitchOffBegin() < m_pcCfg->getLayerSwitchOffEnd())
    {
      Int pocNext;
      if (iGOPid == m_iGopSize - 1)
      {
        pocNext = iPOCLast - iNumPicRcvd + m_iGopSize + m_pcCfg->getGOPEntry(0).m_POC;
      }
      else
      {
        pocNext = iPOCLast - iNumPicRcvd + m_pcCfg->getGOPEntry(iGOPid + 1).m_POC;
      }

      if (pocNext > m_pcCfg->getLayerSwitchOffBegin() && pocCurr < m_pcCfg->getLayerSwitchOffEnd())
      {
        OutputNALUnit nalu(NAL_UNIT_EOS, 0, pcSlice->getLayerId());
        m_pcEntropyCoder->setEntropyCoder(m_pcCavlcCoder);
        accessUnit.push_back(new NALUnitEBSP(nalu));
        m_prevPicHasEos = true;
      }
    }
#endif

    pcPic->getPicYuvRec()->copyToPic(pcPicYuvRecOut);

#if SVC_EXTENSION
    pcPicYuvRecOut->setReconstructed(true);
    m_pcEncTop->setFirstPicInLayerDecodedFlag(true);
#endif

    pcPic->setReconMark   ( true );
    m_bFirst = false;
    m_iNumPicCoded++;
    m_totalCoded ++;
    /* logging: insert a newline at end of picture period */
    printf("\n");
    fflush(stdout);

    if (m_pcCfg->getEfficientFieldIRAPEnabled())
    {
      iGOPid=effFieldIRAPMap.restoreGOPid(iGOPid);
    }
#if REDUCED_ENCODER_MEMORY
#if !SVC_EXTENSION // syntax data is needed when picture is used as a base layer
    pcPic->releaseReconstructionIntermediateData();
    if (!isField) // don't release the source data for field-coding because the fields are dealt with in pairs. // TODO: release source data for interlace simulations.
    {
      pcPic->releaseEncoderSourceImageData();
    }
#endif
#endif
  } // iGOPid-loop

  delete pcBitstreamRedirect;

#if SVC_EXTENSION
  assert ( m_iNumPicCoded <= 1 );
#else
  assert ( (m_iNumPicCoded == iNumPicRcvd) );
#endif
}

Void TEncGOP::printOutSummary(UInt uiNumAllPicCoded, Bool isField, const Bool printMSEBasedSNR, const Bool printSequenceMSE, const BitDepths &bitDepths)
{
  assert (uiNumAllPicCoded == m_gcAnalyzeAll.getNumPic());


  //--CFG_KDY
  const Int rateMultiplier=(isField?2:1);
  m_gcAnalyzeAll.setFrmRate( m_pcCfg->getFrameRate()*rateMultiplier / (Double)m_pcCfg->getTemporalSubsampleRatio());
  m_gcAnalyzeI.setFrmRate( m_pcCfg->getFrameRate()*rateMultiplier / (Double)m_pcCfg->getTemporalSubsampleRatio());
  m_gcAnalyzeP.setFrmRate( m_pcCfg->getFrameRate()*rateMultiplier / (Double)m_pcCfg->getTemporalSubsampleRatio());
  m_gcAnalyzeB.setFrmRate( m_pcCfg->getFrameRate()*rateMultiplier / (Double)m_pcCfg->getTemporalSubsampleRatio());
  const ChromaFormat chFmt = m_pcCfg->getChromaFormatIdc();

  //-- all
  printf( "\n\nSUMMARY --------------------------------------------------------\n" );
  m_gcAnalyzeAll.printOut('a', chFmt, printMSEBasedSNR, printSequenceMSE, bitDepths);

  printf( "\n\nI Slices--------------------------------------------------------\n" );
  m_gcAnalyzeI.printOut('i', chFmt, printMSEBasedSNR, printSequenceMSE, bitDepths);

  printf( "\n\nP Slices--------------------------------------------------------\n" );
  m_gcAnalyzeP.printOut('p', chFmt, printMSEBasedSNR, printSequenceMSE, bitDepths);

  printf( "\n\nB Slices--------------------------------------------------------\n" );
  m_gcAnalyzeB.printOut('b', chFmt, printMSEBasedSNR, printSequenceMSE, bitDepths);

  if (!m_pcCfg->getSummaryOutFilename().empty())
  {
    m_gcAnalyzeAll.printSummary(chFmt, printSequenceMSE, bitDepths, m_pcCfg->getSummaryOutFilename());
  }

  if (!m_pcCfg->getSummaryPicFilenameBase().empty())
  {
    m_gcAnalyzeI.printSummary(chFmt, printSequenceMSE, bitDepths, m_pcCfg->getSummaryPicFilenameBase()+"I.txt");
    m_gcAnalyzeP.printSummary(chFmt, printSequenceMSE, bitDepths, m_pcCfg->getSummaryPicFilenameBase()+"P.txt");
    m_gcAnalyzeB.printSummary(chFmt, printSequenceMSE, bitDepths, m_pcCfg->getSummaryPicFilenameBase()+"B.txt");
  }

  if(isField)
  {
    //-- interlaced summary
    m_gcAnalyzeAll_in.setFrmRate( m_pcCfg->getFrameRate() / (Double)m_pcCfg->getTemporalSubsampleRatio());
    m_gcAnalyzeAll_in.setBits(m_gcAnalyzeAll.getBits());
    // prior to the above statement, the interlace analyser does not contain the correct total number of bits.

    printf( "\n\nSUMMARY INTERLACED ---------------------------------------------\n" );
    m_gcAnalyzeAll_in.printOut('a', chFmt, printMSEBasedSNR, printSequenceMSE, bitDepths);

    if (!m_pcCfg->getSummaryOutFilename().empty())
    {
      m_gcAnalyzeAll_in.printSummary(chFmt, printSequenceMSE, bitDepths, m_pcCfg->getSummaryOutFilename());
    }
  }

  printf("\nRVM: %.3lf\n" , xCalculateRVM());
}

Void TEncGOP::preLoopFilterPicAll( TComPic* pcPic, UInt64& ruiDist )
{
  Bool bCalcDist = false;
  m_pcLoopFilter->setCfg(m_pcCfg->getLFCrossTileBoundaryFlag());
  m_pcLoopFilter->loopFilterPic( pcPic );

  if (!bCalcDist)
  {
    ruiDist = xFindDistortionFrame(pcPic->getPicYuvOrg(), pcPic->getPicYuvRec(), pcPic->getPicSym()->getSPS().getBitDepths());
  }
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================


Void TEncGOP::xInitGOP( Int iPOCLast, Int iNumPicRcvd, Bool isField )
{
  assert( iNumPicRcvd > 0 );
  //  Exception for the first frames
  if ( ( isField && (iPOCLast == 0 || iPOCLast == 1) ) || (!isField  && (iPOCLast == 0))  )
  {
    m_iGopSize    = 1;
  }
  else
  {
    m_iGopSize    = m_pcCfg->getGOPSize();
  }
  assert (m_iGopSize > 0);

  return;
}


Void TEncGOP::xGetBuffer( TComList<TComPic*>&      rcListPic,
                         TComList<TComPicYuv*>&    rcListPicYuvRecOut,
                         Int                       iNumPicRcvd,
                         Int                       iTimeOffset,
                         TComPic*&                 rpcPic,
                         TComPicYuv*&              rpcPicYuvRecOut,
                         Int                       pocCurr,
                         Bool                      isField)
{
  Int i;
  //  Rec. output
  TComList<TComPicYuv*>::iterator     iterPicYuvRec = rcListPicYuvRecOut.end();

  if (isField && pocCurr > 1 && m_iGopSize!=1)
  {
    iTimeOffset--;
  }

  for ( i = 0; i < (iNumPicRcvd - iTimeOffset + 1); i++ )
  {
    iterPicYuvRec--;
  }

  rpcPicYuvRecOut = *(iterPicYuvRec);

  //  Current pic.
  TComList<TComPic*>::iterator        iterPic       = rcListPic.begin();
  while (iterPic != rcListPic.end())
  {
    rpcPic = *(iterPic);
    rpcPic->setCurrSliceIdx(0);
    if (rpcPic->getPOC() == pocCurr)
    {
      break;
    }
    iterPic++;
  }

  assert (rpcPic != NULL);
  assert (rpcPic->getPOC() == pocCurr);

  return;
}

UInt64 TEncGOP::xFindDistortionFrame (TComPicYuv* pcPic0, TComPicYuv* pcPic1, const BitDepths &bitDepths)
{
  UInt64  uiTotalDiff = 0;

  for(Int chan=0; chan<pcPic0 ->getNumberValidComponents(); chan++)
  {
    const ComponentID ch=ComponentID(chan);
    Pel*  pSrc0   = pcPic0 ->getAddr(ch);
    Pel*  pSrc1   = pcPic1 ->getAddr(ch);
    UInt  uiShift     = 2 * DISTORTION_PRECISION_ADJUSTMENT(bitDepths.recon[toChannelType(ch)]-8);

    const Int   iStride = pcPic0->getStride(ch);
    const Int   iWidth  = pcPic0->getWidth(ch);
    const Int   iHeight = pcPic0->getHeight(ch);

    for(Int y = 0; y < iHeight; y++ )
    {
      for(Int x = 0; x < iWidth; x++ )
      {
        Intermediate_Int iTemp = pSrc0[x] - pSrc1[x];
        uiTotalDiff += UInt64((iTemp*iTemp) >> uiShift);
      }
      pSrc0 += iStride;
      pSrc1 += iStride;
    }
  }

  return uiTotalDiff;
}

Void TEncGOP::xCalculateAddPSNRs( const Bool isField, const Bool isFieldTopFieldFirst, const Int iGOPid, TComPic* pcPic, const AccessUnit&accessUnit, TComList<TComPic*> &rcListPic, const Double dEncTime, const InputColourSpaceConversion snr_conversion, const Bool printFrameMSE, Double* PSNR_Y )
{
  xCalculateAddPSNR( pcPic, pcPic->getPicYuvRec(), accessUnit, dEncTime, snr_conversion, printFrameMSE, PSNR_Y );

  //In case of field coding, compute the interlaced PSNR for both fields
  if(isField)
  {
    Bool bothFieldsAreEncoded = false;
    Int correspondingFieldPOC = pcPic->getPOC();
    Int currentPicGOPPoc = m_pcCfg->getGOPEntry(iGOPid).m_POC;
    if(pcPic->getPOC() == 0)
    {
      // particular case for POC 0 and 1.
      // If they are not encoded first and separately from other pictures, we need to change this
      // POC 0 is always encoded first then POC 1 is encoded
      bothFieldsAreEncoded = false;
    }
    else if(pcPic->getPOC() == 1)
    {
      // if we are at POC 1, POC 0 has been encoded for sure
      correspondingFieldPOC = 0;
      bothFieldsAreEncoded = true;
    }
    else
    {
      if(pcPic->getPOC()%2 == 1)
      {
        correspondingFieldPOC -= 1; // all odd POC are associated with the preceding even POC (e.g poc 1 is associated to poc 0)
        currentPicGOPPoc      -= 1;
      }
      else
      {
        correspondingFieldPOC += 1; // all even POC are associated with the following odd POC (e.g poc 0 is associated to poc 1)
        currentPicGOPPoc      += 1;
      }
      for(Int i = 0; i < m_iGopSize; i ++)
      {
        if(m_pcCfg->getGOPEntry(i).m_POC == currentPicGOPPoc)
        {
          bothFieldsAreEncoded = m_pcCfg->getGOPEntry(i).m_isEncoded;
          break;
        }
      }
    }

    if(bothFieldsAreEncoded)
    {
      //get complementary top field
      TComList<TComPic*>::iterator   iterPic = rcListPic.begin();
      while ((*iterPic)->getPOC() != correspondingFieldPOC)
      {
        iterPic ++;
      }
      TComPic* correspondingFieldPic = *(iterPic);

      if( (pcPic->isTopField() && isFieldTopFieldFirst) || (!pcPic->isTopField() && !isFieldTopFieldFirst))
      {
        xCalculateInterlacedAddPSNR(pcPic, correspondingFieldPic, pcPic->getPicYuvRec(), correspondingFieldPic->getPicYuvRec(), snr_conversion, printFrameMSE, PSNR_Y );
      }
      else
      {
        xCalculateInterlacedAddPSNR(correspondingFieldPic, pcPic, correspondingFieldPic->getPicYuvRec(), pcPic->getPicYuvRec(), snr_conversion, printFrameMSE, PSNR_Y );
      }
    }
  }
}

Void TEncGOP::xCalculateAddPSNR( TComPic* pcPic, TComPicYuv* pcPicD, const AccessUnit& accessUnit, Double dEncTime, const InputColourSpaceConversion conversion, const Bool printFrameMSE, Double* PSNR_Y )
{
  Double  dPSNR[MAX_NUM_COMPONENT];

  for(Int i=0; i<MAX_NUM_COMPONENT; i++)
  {
    dPSNR[i]=0.0;
  }

  TComPicYuv cscd;
  if (conversion!=IPCOLOURSPACE_UNCHANGED)
  {
    cscd.createWithoutCUInfo(pcPicD->getWidth(COMPONENT_Y), pcPicD->getHeight(COMPONENT_Y), pcPicD->getChromaFormat() );
    TVideoIOYuv::ColourSpaceConvert(*pcPicD, cscd, conversion, false);
  }
  TComPicYuv &picd=(conversion==IPCOLOURSPACE_UNCHANGED)?*pcPicD : cscd;

  //===== calculate PSNR =====
  Double MSEyuvframe[MAX_NUM_COMPONENT] = {0, 0, 0};

  for(Int chan=0; chan<pcPicD->getNumberValidComponents(); chan++)
  {
    const ComponentID ch=ComponentID(chan);
    const TComPicYuv *pOrgPicYuv =(conversion!=IPCOLOURSPACE_UNCHANGED) ? pcPic ->getPicYuvTrueOrg() : pcPic ->getPicYuvOrg();
    const Pel*  pOrg       = pOrgPicYuv->getAddr(ch);
    const Int   iOrgStride = pOrgPicYuv->getStride(ch);
    Pel*  pRec             = picd.getAddr(ch);
    const Int   iRecStride = picd.getStride(ch);
    const Int   iWidth  = pcPicD->getWidth (ch) - (m_pcEncTop->getPad(0) >> pcPic->getComponentScaleX(ch));
    const Int   iHeight = pcPicD->getHeight(ch) - ((m_pcEncTop->getPad(1) >> (pcPic->isField()?1:0)) >> pcPic->getComponentScaleY(ch));

    Int   iSize   = iWidth*iHeight;

    UInt64 uiSSDtemp=0;
    for(Int y = 0; y < iHeight; y++ )
    {
      for(Int x = 0; x < iWidth; x++ )
      {
        Intermediate_Int iDiff = (Intermediate_Int)( pOrg[x] - pRec[x] );
        uiSSDtemp   += iDiff * iDiff;
      }
      pOrg += iOrgStride;
      pRec += iRecStride;
    }
    const Int maxval = 255 << (pcPic->getPicSym()->getSPS().getBitDepth(toChannelType(ch)) - 8);
    const Double fRefValue = (Double) maxval * maxval * iSize;
    dPSNR[ch]         = ( uiSSDtemp ? 10.0 * log10( fRefValue / (Double)uiSSDtemp ) : 999.99 );
    MSEyuvframe[ch]   = (Double)uiSSDtemp/(iSize);
  }


  /* calculate the size of the access unit, excluding:
   *  - any AnnexB contributions (start_code_prefix, zero_byte, etc.,)
   *  - SEI NAL units
   */
  UInt numRBSPBytes = 0;
  for (AccessUnit::const_iterator it = accessUnit.begin(); it != accessUnit.end(); it++)
  {
    UInt numRBSPBytes_nal = UInt((*it)->m_nalUnitData.str().size());
    if (m_pcCfg->getSummaryVerboseness() > 0)
    {
      printf("*** %6s numBytesInNALunit: %u\n", nalUnitTypeToString((*it)->m_nalUnitType), numRBSPBytes_nal);
    }
    if ((*it)->m_nalUnitType != NAL_UNIT_PREFIX_SEI && (*it)->m_nalUnitType != NAL_UNIT_SUFFIX_SEI)
    {
      numRBSPBytes += numRBSPBytes_nal;
    }
  }

  UInt uibits = numRBSPBytes * 8;
  m_vRVM_RP.push_back( uibits );

  //===== add PSNR =====
  m_gcAnalyzeAll.addResult (dPSNR, (Double)uibits, MSEyuvframe);
  TComSlice*  pcSlice = pcPic->getSlice(0);
  if (pcSlice->isIntra())
  {
    m_gcAnalyzeI.addResult (dPSNR, (Double)uibits, MSEyuvframe);
    *PSNR_Y = dPSNR[COMPONENT_Y];
  }
  if (pcSlice->isInterP())
  {
    m_gcAnalyzeP.addResult (dPSNR, (Double)uibits, MSEyuvframe);
    *PSNR_Y = dPSNR[COMPONENT_Y];
  }
  if (pcSlice->isInterB())
  {
    m_gcAnalyzeB.addResult (dPSNR, (Double)uibits, MSEyuvframe);
    *PSNR_Y = dPSNR[COMPONENT_Y];
  }

  TChar c = (pcSlice->isIntra() ? 'I' : pcSlice->isInterP() ? 'P' : 'B');
  if (!pcSlice->isReferenced())
  {
    c += 32;
  }

#if SVC_EXTENSION
#if ADAPTIVE_QP_SELECTION  
  printf("POC %4d LId: %1d TId: %1d ( %c-SLICE %s, nQP %d QP %d ) %10d bits",
         pcSlice->getPOC(),
         pcSlice->getLayerId(),
         pcSlice->getTLayer(),
         c,
         nalUnitTypeToString( pcSlice->getNalUnitType() ),
         pcSlice->getSliceQpBase(),
         pcSlice->getSliceQp(),
         uibits );
#else
  printf("POC %4d LId: %1d TId: %1d ( %c-SLICE %s, QP %d ) %10d bits",
         pcSlice->getPOC()-pcSlice->getLastIDR(),
         pcSlice->getLayerId(),
         pcSlice->getTLayer(),
         c,
         nalUnitTypeToString( pcSlice->getNalUnitType() ),
         pcSlice->getSliceQp(),
         uibits );
#endif
#else
#if ADAPTIVE_QP_SELECTION
  printf("POC %4d TId: %1d ( %c-SLICE, nQP %d QP %d ) %10d bits",
         pcSlice->getPOC(),
         pcSlice->getTLayer(),
         c,
         pcSlice->getSliceQpBase(),
         pcSlice->getSliceQp(),
         uibits );
#else
  printf("POC %4d TId: %1d ( %c-SLICE, QP %d ) %10d bits",
         pcSlice->getPOC()-pcSlice->getLastIDR(),
         pcSlice->getTLayer(),
         c,
         pcSlice->getSliceQp(),
         uibits );
#endif
#endif

  printf(" [Y %6.4lf dB    U %6.4lf dB    V %6.4lf dB]", dPSNR[COMPONENT_Y], dPSNR[COMPONENT_Cb], dPSNR[COMPONENT_Cr] );
  if (printFrameMSE)
  {
    printf(" [Y MSE %6.4lf  U MSE %6.4lf  V MSE %6.4lf]", MSEyuvframe[COMPONENT_Y], MSEyuvframe[COMPONENT_Cb], MSEyuvframe[COMPONENT_Cr] );
  }
  printf(" [ET %5.0f ]", dEncTime );

  // printf(" [WP %d]", pcSlice->getUseWeightedPrediction());

  for (Int iRefList = 0; iRefList < 2; iRefList++)
  {
    printf(" [L%d ", iRefList);
    for (Int iRefIndex = 0; iRefIndex < pcSlice->getNumRefIdx(RefPicList(iRefList)); iRefIndex++)
    {
#if SVC_EXTENSION
      if( pcSlice->getRefPic(RefPicList(iRefList), iRefIndex)->isILR(m_layerId) )
      {
        UInt refLayerId = pcSlice->getRefPic(RefPicList(iRefList), iRefIndex)->getLayerId();
        UInt refLayerIdc = pcSlice->getReferenceLayerIdc(refLayerId);
        assert( pcSlice->getPic()->getPosScalingFactor(refLayerIdc, 0) );
        assert( pcSlice->getPic()->getPosScalingFactor(refLayerIdc, 1) );

        printf( "%d(%d, {%1.2f, %1.2f}x)", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex), refLayerId, (Double)POS_SCALING_FACTOR_1X/pcSlice->getPic()->getPosScalingFactor(refLayerIdc, 0), (Double)POS_SCALING_FACTOR_1X/pcSlice->getPic()->getPosScalingFactor(refLayerIdc, 1) );
      }
      else
      {
        printf ("%d", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex));
      }

      if( pcSlice->getEnableTMVPFlag() && iRefList == 1 - pcSlice->getColFromL0Flag() && iRefIndex == pcSlice->getColRefIdx() )
      {
        printf( "c" );
      }

      printf( " " );
#else
      printf ("%d ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex)-pcSlice->getLastIDR());
#endif
    }
    printf("]");
  }
#if CGS_3D_ASYMLUT
  pcPic->setFrameBit( (Int)uibits );
  if( m_layerId && pcSlice->getPPS()->getCGSFlag() )
  {
#if R0179_ENC_OPT_3DLUT_SIZE
      m_Enc3DAsymLUTPicUpdate.update3DAsymLUTParam( &m_Enc3DAsymLUTPPS );
#else
    if( m_Enc3DAsymLUTPPS.getPPSBit() > 0 )
      m_Enc3DAsymLUTPicUpdate.copy3DAsymLUT( &m_Enc3DAsymLUTPPS );
#endif
    m_Enc3DAsymLUTPicUpdate.updatePicCGSBits( pcSlice , m_Enc3DAsymLUTPPS.getPPSBit() );
  }
#endif
#if !_DEBUG
  fstream outg("FrameInfo.txt", ios::app | ios::out);
  outg << pcSlice->getPOC() << " " << pcSlice->getLayerId() << " " << uibits << " " << 
	  dPSNR[COMPONENT_Y] << " " << m_pcRateCtrl->getRCPic()->getPicActualLambda()<<" "<< pcSlice->getSliceQp() << endl;
#endif
  cscd.destroy();
}

Void TEncGOP::xCalculateInterlacedAddPSNR( TComPic* pcPicOrgFirstField, TComPic* pcPicOrgSecondField,
                                           TComPicYuv* pcPicRecFirstField, TComPicYuv* pcPicRecSecondField,
                                           const InputColourSpaceConversion conversion, const Bool printFrameMSE, Double* PSNR_Y )
{
  const TComSPS &sps=pcPicOrgFirstField->getPicSym()->getSPS();
  Double  dPSNR[MAX_NUM_COMPONENT];
  TComPic    *apcPicOrgFields[2]={pcPicOrgFirstField, pcPicOrgSecondField};
  TComPicYuv *apcPicRecFields[2]={pcPicRecFirstField, pcPicRecSecondField};

  for(Int i=0; i<MAX_NUM_COMPONENT; i++)
  {
    dPSNR[i]=0.0;
  }

  TComPicYuv cscd[2 /* first/second field */];
  if (conversion!=IPCOLOURSPACE_UNCHANGED)
  {
    for(UInt fieldNum=0; fieldNum<2; fieldNum++)
    {
      TComPicYuv &reconField=*(apcPicRecFields[fieldNum]);
      cscd[fieldNum].createWithoutCUInfo(reconField.getWidth(COMPONENT_Y), reconField.getHeight(COMPONENT_Y), reconField.getChromaFormat() );
      TVideoIOYuv::ColourSpaceConvert(reconField, cscd[fieldNum], conversion, false);
      apcPicRecFields[fieldNum]=cscd+fieldNum;
    }
  }

  //===== calculate PSNR =====
  Double MSEyuvframe[MAX_NUM_COMPONENT] = {0, 0, 0};

  assert(apcPicRecFields[0]->getChromaFormat()==apcPicRecFields[1]->getChromaFormat());
  const UInt numValidComponents=apcPicRecFields[0]->getNumberValidComponents();

  for(Int chan=0; chan<numValidComponents; chan++)
  {
    const ComponentID ch=ComponentID(chan);
    assert(apcPicRecFields[0]->getWidth(ch)==apcPicRecFields[1]->getWidth(ch));
    assert(apcPicRecFields[0]->getHeight(ch)==apcPicRecFields[1]->getHeight(ch));

    UInt64 uiSSDtemp=0;
    const Int   iWidth  = apcPicRecFields[0]->getWidth (ch) - (m_pcEncTop->getPad(0) >> apcPicRecFields[0]->getComponentScaleX(ch));
    const Int   iHeight = apcPicRecFields[0]->getHeight(ch) - ((m_pcEncTop->getPad(1) >> 1) >> apcPicRecFields[0]->getComponentScaleY(ch));

    Int   iSize   = iWidth*iHeight;

    for(UInt fieldNum=0; fieldNum<2; fieldNum++)
    {
      TComPic *pcPic=apcPicOrgFields[fieldNum];
      TComPicYuv *pcPicD=apcPicRecFields[fieldNum];

      const Pel*  pOrg    = (conversion!=IPCOLOURSPACE_UNCHANGED) ? pcPic ->getPicYuvTrueOrg()->getAddr(ch) : pcPic ->getPicYuvOrg()->getAddr(ch);
      Pel*  pRec    = pcPicD->getAddr(ch);
      const Int   iStride = pcPicD->getStride(ch);


      for(Int y = 0; y < iHeight; y++ )
      {
        for(Int x = 0; x < iWidth; x++ )
        {
          Intermediate_Int iDiff = (Intermediate_Int)( pOrg[x] - pRec[x] );
          uiSSDtemp   += iDiff * iDiff;
        }
        pOrg += iStride;
        pRec += iStride;
      }
    }
    const Int maxval = 255 << (sps.getBitDepth(toChannelType(ch)) - 8);
    const Double fRefValue = (Double) maxval * maxval * iSize*2;
    dPSNR[ch]         = ( uiSSDtemp ? 10.0 * log10( fRefValue / (Double)uiSSDtemp ) : 999.99 );
    MSEyuvframe[ch]   = (Double)uiSSDtemp/(iSize*2);
  }

  UInt uibits = 0; // the number of bits for the pair is not calculated here - instead the overall total is used elsewhere.

  //===== add PSNR =====
  m_gcAnalyzeAll_in.addResult (dPSNR, (Double)uibits, MSEyuvframe);

  *PSNR_Y = dPSNR[COMPONENT_Y];

  printf("\n                                      Interlaced frame %d: [Y %6.4lf dB    U %6.4lf dB    V %6.4lf dB]", pcPicOrgSecondField->getPOC()/2 , dPSNR[COMPONENT_Y], dPSNR[COMPONENT_Cb], dPSNR[COMPONENT_Cr] );
  if (printFrameMSE)
  {
    printf(" [Y MSE %6.4lf  U MSE %6.4lf  V MSE %6.4lf]", MSEyuvframe[COMPONENT_Y], MSEyuvframe[COMPONENT_Cb], MSEyuvframe[COMPONENT_Cr] );
  }

  for(UInt fieldNum=0; fieldNum<2; fieldNum++)
  {
    cscd[fieldNum].destroy();
  }
}

/** Function for deciding the nal_unit_type.
 * \param pocCurr POC of the current picture
 * \param lastIDR  POC of the last IDR picture
 * \param isField  true to indicate field coding
 * \returns the NAL unit type of the picture
 * This function checks the configuration and returns the appropriate nal_unit_type for the picture.
 */
NalUnitType TEncGOP::getNalUnitType(Int pocCurr, Int lastIDR, Bool isField)
{
  if (pocCurr == 0)
  {
    return NAL_UNIT_CODED_SLICE_IDR_W_RADL;
  }

  if(m_pcCfg->getEfficientFieldIRAPEnabled() && isField && pocCurr == 1)
  {
    // to avoid the picture becoming an IRAP
    return NAL_UNIT_CODED_SLICE_TRAIL_R;
  }

  if(m_pcCfg->getDecodingRefreshType() != 3 && (pocCurr - isField) % m_pcCfg->getIntraPeriod() == 0)
  {
    if (m_pcCfg->getDecodingRefreshType() == 1)
    {
      return NAL_UNIT_CODED_SLICE_CRA;
    }
    else if (m_pcCfg->getDecodingRefreshType() == 2)
    {
      return NAL_UNIT_CODED_SLICE_IDR_W_RADL;
    }
  }

#if SVC_POC
  if( m_pocCraWithoutReset > 0 && m_associatedIRAPType == NAL_UNIT_CODED_SLICE_CRA )
  {
    if(pocCurr < m_pocCraWithoutReset)
#else
  if(m_pocCRA>0)
  {
    if(pocCurr<m_pocCRA)
#endif
    {
      // All leading pictures are being marked as TFD pictures here since current encoder uses all
      // reference pictures while encoding leading pictures. An encoder can ensure that a leading
      // picture can be still decodable when random accessing to a CRA/CRANT/BLA/BLANT picture by
      // controlling the reference pictures used for encoding that leading picture. Such a leading
      // picture need not be marked as a TFD picture.
      return NAL_UNIT_CODED_SLICE_RASL_R;
    }
  }
  if (lastIDR>0)
  {
    if (pocCurr < lastIDR)
    {
      return NAL_UNIT_CODED_SLICE_RADL_R;
    }
  }
  return NAL_UNIT_CODED_SLICE_TRAIL_R;
}

Double TEncGOP::xCalculateRVM()
{
  Double dRVM = 0;

  if( m_pcCfg->getGOPSize() == 1 && m_pcCfg->getIntraPeriod() != 1 && m_pcCfg->getFramesToBeEncoded() > RVM_VCEGAM10_M * 2 )
  {
    // calculate RVM only for lowdelay configurations
    std::vector<Double> vRL , vB;
    size_t N = m_vRVM_RP.size();
    vRL.resize( N );
    vB.resize( N );

    Int i;
    Double dRavg = 0 , dBavg = 0;
    vB[RVM_VCEGAM10_M] = 0;
    for( i = RVM_VCEGAM10_M + 1 ; i < N - RVM_VCEGAM10_M + 1 ; i++ )
    {
      vRL[i] = 0;
      for( Int j = i - RVM_VCEGAM10_M ; j <= i + RVM_VCEGAM10_M - 1 ; j++ )
      {
        vRL[i] += m_vRVM_RP[j];
      }
      vRL[i] /= ( 2 * RVM_VCEGAM10_M );
      vB[i] = vB[i-1] + m_vRVM_RP[i] - vRL[i];
      dRavg += m_vRVM_RP[i];
      dBavg += vB[i];
    }

    dRavg /= ( N - 2 * RVM_VCEGAM10_M );
    dBavg /= ( N - 2 * RVM_VCEGAM10_M );

    Double dSigamB = 0;
    for( i = RVM_VCEGAM10_M + 1 ; i < N - RVM_VCEGAM10_M + 1 ; i++ )
    {
      Double tmp = vB[i] - dBavg;
      dSigamB += tmp * tmp;
    }
    dSigamB = sqrt( dSigamB / ( N - 2 * RVM_VCEGAM10_M ) );

    Double f = sqrt( 12.0 * ( RVM_VCEGAM10_M - 1 ) / ( RVM_VCEGAM10_M + 1 ) );

    dRVM = dSigamB / dRavg * f;
  }

  return( dRVM );
}

/** Attaches the input bitstream to the stream in the output NAL unit
    Updates rNalu to contain concatenated bitstream. rpcBitstreamRedirect is cleared at the end of this function call.
 *  \param codedSliceData contains the coded slice data (bitstream) to be concatenated to rNalu
 *  \param rNalu          target NAL unit
 */
Void TEncGOP::xAttachSliceDataToNalUnit (OutputNALUnit& rNalu, TComOutputBitstream* codedSliceData)
{
  // Byte-align
  rNalu.m_Bitstream.writeByteAlignment();   // Slice header byte-alignment

  // Perform bitstream concatenation
  if (codedSliceData->getNumberOfWrittenBits() > 0)
  {
    rNalu.m_Bitstream.addSubstream(codedSliceData);
  }

  m_pcEntropyCoder->setBitstream(&rNalu.m_Bitstream);

  codedSliceData->clear();
}

// Function will arrange the long-term pictures in the decreasing order of poc_lsb_lt,
// and among the pictures with the same lsb, it arranges them in increasing delta_poc_msb_cycle_lt value
Void TEncGOP::arrangeLongtermPicturesInRPS(TComSlice *pcSlice, TComList<TComPic*>& rcListPic)
{
  if(pcSlice->getRPS()->getNumberOfLongtermPictures() == 0)
  {
    return;
  }
  // we can only modify the local RPS!
  assert (pcSlice->getRPSidx()==-1);
  TComReferencePictureSet *rps = pcSlice->getLocalRPS();

  // Arrange long-term reference pictures in the correct order of LSB and MSB,
  // and assign values for pocLSBLT and MSB present flag
  Int longtermPicsPoc[MAX_NUM_REF_PICS], longtermPicsLSB[MAX_NUM_REF_PICS], indices[MAX_NUM_REF_PICS];
  Int longtermPicsMSB[MAX_NUM_REF_PICS];
  Bool mSBPresentFlag[MAX_NUM_REF_PICS];
  ::memset(longtermPicsPoc, 0, sizeof(longtermPicsPoc));    // Store POC values of LTRP
  ::memset(longtermPicsLSB, 0, sizeof(longtermPicsLSB));    // Store POC LSB values of LTRP
  ::memset(longtermPicsMSB, 0, sizeof(longtermPicsMSB));    // Store POC LSB values of LTRP
  ::memset(indices        , 0, sizeof(indices));            // Indices to aid in tracking sorted LTRPs
  ::memset(mSBPresentFlag , 0, sizeof(mSBPresentFlag));     // Indicate if MSB needs to be present

  // Get the long-term reference pictures
  Int offset = rps->getNumberOfNegativePictures() + rps->getNumberOfPositivePictures();
  Int i, ctr = 0;
  Int maxPicOrderCntLSB = 1 << pcSlice->getSPS()->getBitsForPOC();
  for(i = rps->getNumberOfPictures() - 1; i >= offset; i--, ctr++)
  {
    longtermPicsPoc[ctr] = rps->getPOC(i);                                  // LTRP POC
    longtermPicsLSB[ctr] = getLSB(longtermPicsPoc[ctr], maxPicOrderCntLSB); // LTRP POC LSB
    indices[ctr]      = i;
    longtermPicsMSB[ctr] = longtermPicsPoc[ctr] - longtermPicsLSB[ctr];
  }
  Int numLongPics = rps->getNumberOfLongtermPictures();
  assert(ctr == numLongPics);

  // Arrange pictures in decreasing order of MSB;
  for(i = 0; i < numLongPics; i++)
  {
    for(Int j = 0; j < numLongPics - 1; j++)
    {
      if(longtermPicsMSB[j] < longtermPicsMSB[j+1])
      {
        std::swap(longtermPicsPoc[j], longtermPicsPoc[j+1]);
        std::swap(longtermPicsLSB[j], longtermPicsLSB[j+1]);
        std::swap(longtermPicsMSB[j], longtermPicsMSB[j+1]);
        std::swap(indices[j]        , indices[j+1]        );
      }
    }
  }

  for(i = 0; i < numLongPics; i++)
  {
    // Check if MSB present flag should be enabled.
    // Check if the buffer contains any pictures that have the same LSB.
    TComList<TComPic*>::iterator  iterPic = rcListPic.begin();
    TComPic*                      pcPic;
    while ( iterPic != rcListPic.end() )
    {
      pcPic = *iterPic;
      if( (getLSB(pcPic->getPOC(), maxPicOrderCntLSB) == longtermPicsLSB[i])   &&     // Same LSB
                                      (pcPic->getSlice(0)->isReferenced())     &&    // Reference picture
                                        (pcPic->getPOC() != longtermPicsPoc[i])    )  // Not the LTRP itself
      {
        mSBPresentFlag[i] = true;
        break;
      }
      iterPic++;
    }
  }

  // tempArray for usedByCurr flag
  Bool tempArray[MAX_NUM_REF_PICS]; ::memset(tempArray, 0, sizeof(tempArray));
  for(i = 0; i < numLongPics; i++)
  {
    tempArray[i] = rps->getUsed(indices[i]);
  }
  // Now write the final values;
  ctr = 0;
  Int currMSB = 0, currLSB = 0;
  // currPicPoc = currMSB + currLSB
  currLSB = getLSB(pcSlice->getPOC(), maxPicOrderCntLSB);
  currMSB = pcSlice->getPOC() - currLSB;

  for(i = rps->getNumberOfPictures() - 1; i >= offset; i--, ctr++)
  {
    rps->setPOC                   (i, longtermPicsPoc[ctr]);
    rps->setDeltaPOC              (i, - pcSlice->getPOC() + longtermPicsPoc[ctr]);
    rps->setUsed                  (i, tempArray[ctr]);
    rps->setPocLSBLT              (i, longtermPicsLSB[ctr]);
    rps->setDeltaPocMSBCycleLT    (i, (currMSB - (longtermPicsPoc[ctr] - longtermPicsLSB[ctr])) / maxPicOrderCntLSB);
    rps->setDeltaPocMSBPresentFlag(i, mSBPresentFlag[ctr]);

    assert(rps->getDeltaPocMSBCycleLT(i) >= 0);   // Non-negative value
  }
  for(i = rps->getNumberOfPictures() - 1, ctr = 1; i >= offset; i--, ctr++)
  {
    for(Int j = rps->getNumberOfPictures() - 1 - ctr; j >= offset; j--)
    {
      // Here at the encoder we know that we have set the full POC value for the LTRPs, hence we
      // don't have to check the MSB present flag values for this constraint.
      assert( rps->getPOC(i) != rps->getPOC(j) ); // If assert fails, LTRP entry repeated in RPS!!!
    }
  }
}

Void TEncGOP::applyDeblockingFilterMetric( TComPic* pcPic, UInt uiNumSlices )
{
  TComPicYuv* pcPicYuvRec = pcPic->getPicYuvRec();
  Pel* Rec    = pcPicYuvRec->getAddr(COMPONENT_Y);
  Pel* tempRec = Rec;
  Int  stride = pcPicYuvRec->getStride(COMPONENT_Y);
  UInt log2maxTB = pcPic->getSlice(0)->getSPS()->getQuadtreeTULog2MaxSize();
  UInt maxTBsize = (1<<log2maxTB);
  const UInt minBlockArtSize = 8;
  const UInt picWidth = pcPicYuvRec->getWidth(COMPONENT_Y);
  const UInt picHeight = pcPicYuvRec->getHeight(COMPONENT_Y);
  const UInt noCol = (picWidth>>log2maxTB);
  const UInt noRows = (picHeight>>log2maxTB);
  assert(noCol > 1);
  assert(noRows > 1);
  std::vector<UInt64> colSAD(noCol,  UInt64(0));
  std::vector<UInt64> rowSAD(noRows, UInt64(0));
  UInt colIdx = 0;
  UInt rowIdx = 0;
  Pel p0, p1, p2, q0, q1, q2;

  Int qp = pcPic->getSlice(0)->getSliceQp();
  const Int bitDepthLuma=pcPic->getSlice(0)->getSPS()->getBitDepth(CHANNEL_TYPE_LUMA);
  Int bitdepthScale = 1 << (bitDepthLuma-8);
  Int beta = TComLoopFilter::getBeta( qp ) * bitdepthScale;
  const Int thr2 = (beta>>2);
  const Int thr1 = 2*bitdepthScale;
  UInt a = 0;

  if (maxTBsize > minBlockArtSize)
  {
    // Analyze vertical artifact edges
    for(Int c = maxTBsize; c < picWidth; c += maxTBsize)
    {
      for(Int r = 0; r < picHeight; r++)
      {
        p2 = Rec[c-3];
        p1 = Rec[c-2];
        p0 = Rec[c-1];
        q0 = Rec[c];
        q1 = Rec[c+1];
        q2 = Rec[c+2];
        a = ((abs(p2-(p1<<1)+p0)+abs(q0-(q1<<1)+q2))<<1);
        if ( thr1 < a && a < thr2)
        {
          colSAD[colIdx] += abs(p0 - q0);
        }
        Rec += stride;
      }
      colIdx++;
      Rec = tempRec;
    }

    // Analyze horizontal artifact edges
    for(Int r = maxTBsize; r < picHeight; r += maxTBsize)
    {
      for(Int c = 0; c < picWidth; c++)
      {
        p2 = Rec[c + (r-3)*stride];
        p1 = Rec[c + (r-2)*stride];
        p0 = Rec[c + (r-1)*stride];
        q0 = Rec[c + r*stride];
        q1 = Rec[c + (r+1)*stride];
        q2 = Rec[c + (r+2)*stride];
        a = ((abs(p2-(p1<<1)+p0)+abs(q0-(q1<<1)+q2))<<1);
        if (thr1 < a && a < thr2)
        {
          rowSAD[rowIdx] += abs(p0 - q0);
        }
      }
      rowIdx++;
    }
  }

  UInt64 colSADsum = 0;
  UInt64 rowSADsum = 0;
  for(Int c = 0; c < noCol-1; c++)
  {
    colSADsum += colSAD[c];
  }
  for(Int r = 0; r < noRows-1; r++)
  {
    rowSADsum += rowSAD[r];
  }

  colSADsum <<= 10;
  rowSADsum <<= 10;
  colSADsum /= (noCol-1);
  colSADsum /= picHeight;
  rowSADsum /= (noRows-1);
  rowSADsum /= picWidth;

  UInt64 avgSAD = ((colSADsum + rowSADsum)>>1);
  avgSAD >>= (bitDepthLuma-8);

  if ( avgSAD > 2048 )
  {
    avgSAD >>= 9;
    Int offset = Clip3(2,6,(Int)avgSAD);
    for (Int i=0; i<uiNumSlices; i++)
    {
      pcPic->getSlice(i)->setDeblockingFilterOverrideFlag(true);
      pcPic->getSlice(i)->setDeblockingFilterDisable(false);
      pcPic->getSlice(i)->setDeblockingFilterBetaOffsetDiv2( offset );
      pcPic->getSlice(i)->setDeblockingFilterTcOffsetDiv2( offset );
    }
  }
  else
  {
    for (Int i=0; i<uiNumSlices; i++)
    {
      pcPic->getSlice(i)->setDeblockingFilterOverrideFlag(false);
      pcPic->getSlice(i)->setDeblockingFilterDisable(        pcPic->getSlice(i)->getPPS()->getPPSDeblockingFilterDisabledFlag() );
      pcPic->getSlice(i)->setDeblockingFilterBetaOffsetDiv2( pcPic->getSlice(i)->getPPS()->getDeblockingFilterBetaOffsetDiv2() );
      pcPic->getSlice(i)->setDeblockingFilterTcOffsetDiv2(   pcPic->getSlice(i)->getPPS()->getDeblockingFilterTcOffsetDiv2()   );
    }
  }
}

#if W0038_DB_OPT
Void TEncGOP::applyDeblockingFilterParameterSelection( TComPic* pcPic, const UInt numSlices, const Int gopID )
{
  enum DBFltParam
  {
    DBFLT_PARAM_AVAILABLE = 0,
    DBFLT_DISABLE_FLAG,
    DBFLT_BETA_OFFSETD2,
    DBFLT_TC_OFFSETD2,
    //NUM_DBFLT_PARAMS
  };
  const Int MAX_BETA_OFFSET = 3;
  const Int MIN_BETA_OFFSET = -3;
  const Int MAX_TC_OFFSET = 3;
  const Int MIN_TC_OFFSET = -3;

  TComPicYuv* pcPicYuvRec = pcPic->getPicYuvRec();
  TComPicYuv* pcPicYuvOrg = pcPic ->getPicYuvOrg();

  const Int currQualityLayer = (pcPic->getSlice(0)->getSliceType() != I_SLICE) ? m_pcCfg->getGOPEntry(gopID).m_temporalId+1 : 0;
  assert(currQualityLayer <MAX_ENCODER_DEBLOCKING_QUALITY_LAYERS);

  if(!m_pcDeblockingTempPicYuv)
  {
    m_pcDeblockingTempPicYuv         = new TComPicYuv;
    m_pcDeblockingTempPicYuv->create( m_pcEncTop->getSourceWidth(), m_pcEncTop->getSourceHeight(), m_pcEncTop->getChromaFormatIdc(),  pcPic->getSlice(0)->getSPS()->getMaxCUWidth(), pcPic->getSlice(0)->getSPS()->getMaxCUHeight(), pcPic->getSlice(0)->getSPS()->getMaxTotalCUDepth(),true );
    memset(m_DBParam, 0, sizeof(m_DBParam));
  }

  //preserve current reconstruction
  pcPicYuvRec->copyToPic(m_pcDeblockingTempPicYuv);

  const Bool bNoFiltering      = m_DBParam[currQualityLayer][DBFLT_PARAM_AVAILABLE] && m_DBParam[currQualityLayer][DBFLT_DISABLE_FLAG]==false /*&& pcPic->getTLayer()==0*/;
  const Int  maxBetaOffsetDiv2 = bNoFiltering? Clip3(MIN_BETA_OFFSET, MAX_BETA_OFFSET, m_DBParam[currQualityLayer][DBFLT_BETA_OFFSETD2]+1) : MAX_BETA_OFFSET;
  const Int  minBetaOffsetDiv2 = bNoFiltering? Clip3(MIN_BETA_OFFSET, MAX_BETA_OFFSET, m_DBParam[currQualityLayer][DBFLT_BETA_OFFSETD2]-1) : MIN_BETA_OFFSET;
  const Int  maxTcOffsetDiv2   = bNoFiltering? Clip3(MIN_TC_OFFSET, MAX_TC_OFFSET, m_DBParam[currQualityLayer][DBFLT_TC_OFFSETD2]+2)       : MAX_TC_OFFSET;
  const Int  minTcOffsetDiv2   = bNoFiltering? Clip3(MIN_TC_OFFSET, MAX_TC_OFFSET, m_DBParam[currQualityLayer][DBFLT_TC_OFFSETD2]-2)       : MIN_TC_OFFSET;

  UInt64 distBetaPrevious      = std::numeric_limits<UInt64>::max();
  UInt64 distMin               = std::numeric_limits<UInt64>::max();
  Bool   bDBFilterDisabledBest = true;
  Int    betaOffsetDiv2Best    = 0;
  Int    tcOffsetDiv2Best      = 0;

  for(Int betaOffsetDiv2=maxBetaOffsetDiv2; betaOffsetDiv2>=minBetaOffsetDiv2; betaOffsetDiv2--)
  {
    UInt64 distTcMin = std::numeric_limits<UInt64>::max();
    for(Int tcOffsetDiv2=maxTcOffsetDiv2; tcOffsetDiv2 >= minTcOffsetDiv2; tcOffsetDiv2--)
    {
      for (Int i=0; i<numSlices; i++)
      {
        pcPic->getSlice(i)->setDeblockingFilterOverrideFlag(true);
        pcPic->getSlice(i)->setDeblockingFilterDisable(false);
        pcPic->getSlice(i)->setDeblockingFilterBetaOffsetDiv2( betaOffsetDiv2 );
        pcPic->getSlice(i)->setDeblockingFilterTcOffsetDiv2( tcOffsetDiv2 );
      }
      m_pcDeblockingTempPicYuv->copyToPic(pcPicYuvRec); // restore reconstruction
      m_pcLoopFilter->loopFilterPic( pcPic );
      const UInt64 dist = xFindDistortionFrame(pcPicYuvOrg, pcPicYuvRec, pcPic->getPicSym()->getSPS().getBitDepths());
      if(dist < distMin)
      {
        distMin = dist;
        bDBFilterDisabledBest = false;
        betaOffsetDiv2Best  = betaOffsetDiv2;
        tcOffsetDiv2Best = tcOffsetDiv2;
      }
      if(dist < distTcMin)
      {
        distTcMin = dist;
      }
      else if(tcOffsetDiv2 <-2)
      {
        break;
      }
    }
    if(betaOffsetDiv2<-1 && distTcMin >= distBetaPrevious)
    {
      break;
    }
    distBetaPrevious = distTcMin;
  }

  //update:
  m_DBParam[currQualityLayer][DBFLT_PARAM_AVAILABLE] = 1;
  m_DBParam[currQualityLayer][DBFLT_DISABLE_FLAG]    = bDBFilterDisabledBest;
  m_DBParam[currQualityLayer][DBFLT_BETA_OFFSETD2]   = betaOffsetDiv2Best;
  m_DBParam[currQualityLayer][DBFLT_TC_OFFSETD2]     = tcOffsetDiv2Best;

  m_pcDeblockingTempPicYuv->copyToPic(pcPicYuvRec); //restore reconstruction

  if(bDBFilterDisabledBest)
  {
    for (Int i=0; i<numSlices; i++)
    {
      pcPic->getSlice(i)->setDeblockingFilterOverrideFlag(true);
      pcPic->getSlice(i)->setDeblockingFilterDisable(true);
    }
  }
  else if(betaOffsetDiv2Best ==pcPic->getSlice(0)->getPPS()->getDeblockingFilterBetaOffsetDiv2() &&  tcOffsetDiv2Best==pcPic->getSlice(0)->getPPS()->getDeblockingFilterTcOffsetDiv2())
  {
    for (Int i=0; i<numSlices; i++)
    {
      pcPic->getSlice(i)->setDeblockingFilterOverrideFlag(false);
      pcPic->getSlice(i)->setDeblockingFilterDisable(        pcPic->getSlice(i)->getPPS()->getPPSDeblockingFilterDisabledFlag() );
      pcPic->getSlice(i)->setDeblockingFilterBetaOffsetDiv2( pcPic->getSlice(i)->getPPS()->getDeblockingFilterBetaOffsetDiv2() );
      pcPic->getSlice(i)->setDeblockingFilterTcOffsetDiv2(   pcPic->getSlice(i)->getPPS()->getDeblockingFilterTcOffsetDiv2()   );
    }
  }
  else
  {
    for (Int i=0; i<numSlices; i++)
    {
      pcPic->getSlice(i)->setDeblockingFilterOverrideFlag(true);
      pcPic->getSlice(i)->setDeblockingFilterDisable( false );
      pcPic->getSlice(i)->setDeblockingFilterBetaOffsetDiv2(betaOffsetDiv2Best);
      pcPic->getSlice(i)->setDeblockingFilterTcOffsetDiv2(tcOffsetDiv2Best);
    }
  }
}
#endif

#if SVC_EXTENSION
#if N0383_IL_CONSTRAINED_TILE_SETS_SEI
Void TEncGOP::xBuildTileSetsMap(TComPicSym* picSym)
{
  Int numCUs = picSym->getFrameWidthInCtus() * picSym->getFrameHeightInCtus();

  for (Int i = 0; i < numCUs; i++)
  {
    picSym->setTileSetIdxMap(i, -1, 0, false);
  }

  for (Int i = 0; i < m_pcCfg->getIlNumSetsInMessage(); i++)
  {
    const TComTile* topLeftTile     = picSym->getTComTile(m_pcCfg->getTopLeftTileIndex(i));
    TComTile* bottomRightTile = picSym->getTComTile(m_pcCfg->getBottomRightTileIndex(i));
    Int tileSetLeftEdgePosInCU = topLeftTile->getRightEdgePosInCtus() - topLeftTile->getTileWidthInCtus() + 1;
    Int tileSetRightEdgePosInCU = bottomRightTile->getRightEdgePosInCtus();
    Int tileSetTopEdgePosInCU = topLeftTile->getBottomEdgePosInCtus() - topLeftTile->getTileHeightInCtus() + 1;
    Int tileSetBottomEdgePosInCU = bottomRightTile->getBottomEdgePosInCtus();
    assert(tileSetLeftEdgePosInCU < tileSetRightEdgePosInCU && tileSetTopEdgePosInCU < tileSetBottomEdgePosInCU);
    for (Int j = tileSetTopEdgePosInCU; j <= tileSetBottomEdgePosInCU; j++)
    {
      for (Int k = tileSetLeftEdgePosInCU; k <= tileSetRightEdgePosInCU; k++)
      {
        picSym->setTileSetIdxMap(j * picSym->getFrameWidthInCtus() + k, i, m_pcCfg->getIlcIdc(i), false);
      }
    }
  }
  
  if (m_pcCfg->getSkippedTileSetPresentFlag())
  {
    Int skippedTileSetIdx = m_pcCfg->getIlNumSetsInMessage();
    for (Int i = 0; i < numCUs; i++)
    {
      if (picSym->getTileSetIdxMap(i) < 0)
      {
        picSym->setTileSetIdxMap(i, skippedTileSetIdx, 0, true);
      }
    }
  }
}
#endif

Void TEncGOP::determinePocResetIdc(Int const pocCurr, TComSlice *const slice)
{
  // If one picture in the AU is IDR, and another picture is not IDR, set the poc_reset_idc to 1 or 2
  // If BL picture in the AU is IDR, and another picture is not IDR, set the poc_reset_idc to 2
  // If BL picture is IRAP, and another picture is non-IRAP, then the poc_reset_idc is equal to 1 or 2.
  slice->setPocMsbNeeded(false);

  if( slice->getSliceIdx() == 0 ) // First slice - compute, copy for other slices
  {
    Int needReset = false;
    Int resetDueToBL = false;
    if( slice->getVPS()->getMaxLayers() > 1 )
    {
      // If IRAP is refreshed in this access unit for base layer
      if( (m_ppcTEncTop[0]->getGOPEncoder()->getIntraRefreshType() == 1 || m_ppcTEncTop[0]->getGOPEncoder()->getIntraRefreshType() == 2)
        && ( pocCurr % m_ppcTEncTop[0]->getGOPEncoder()->getIntraRefreshInterval() == 0 )
        )
      {
        // Check if the IRAP refresh interval of any layer does not match that of the base layer
        for(Int i = 1; i < slice->getVPS()->getMaxLayers(); i++)
        {
          Bool refreshIntervalFlag = ( pocCurr % m_ppcTEncTop[i]->getGOPEncoder()->getIntraRefreshInterval() == 0 );
          Bool refreshTypeFlag     = ( m_ppcTEncTop[0]->getGOPEncoder()->getIntraRefreshType() == m_ppcTEncTop[i]->getGOPEncoder()->getIntraRefreshType() );
          if( !(refreshIntervalFlag && refreshTypeFlag) )
          {
            needReset = true;
            resetDueToBL = true;
            break;
          }
        }
      }
    }

    if( !needReset )// No need reset due to base layer IRAP
    {
      // Check if EL IDRs results in POC Reset
      for(Int i = 1; i < slice->getVPS()->getMaxLayers() && !needReset; i++)
      {
        Bool idrFlag = ( (m_ppcTEncTop[i]->getGOPEncoder()->getIntraRefreshType() == 2) 
          && ( pocCurr % m_ppcTEncTop[i]->getGOPEncoder()->getIntraRefreshInterval() == 0 )
          );
        for(Int j = 0; j < slice->getVPS()->getMaxLayers(); j++)
        {
          if( j == i )
          {
            continue;
          }

          Bool idrOtherPicFlag = ( (m_ppcTEncTop[j]->getGOPEncoder()->getIntraRefreshType() == 2) 
            && ( pocCurr % m_ppcTEncTop[j]->getGOPEncoder()->getIntraRefreshInterval() == 0 )
            );

          if( idrFlag != idrOtherPicFlag )
          {
            needReset = true;
            break;
          }
        }
      }
    }
    if( needReset )
    {
      if( m_ppcTEncTop[0]->getGOPEncoder()->getIntraRefreshType() == 2 )  // BL IDR refresh, assuming BL picture present
      {
        if( resetDueToBL )
        {
          slice->setPocResetIdc( 2 ); // Full reset needed

          if( slice->getVPS()->getVpsPocLsbAlignedFlag() && slice->getVPS()->getNumDirectRefLayers(slice->getLayerId()) == 0 )
          {
            slice->setPocMsbNeeded(true);  // Force msb writing
          }
        }
        else
        {
          slice->setPocResetIdc( 1 ); // Due to IDR in EL
        }
      }
      else
      {
        slice->setPocResetIdc( 1 ); // Only MSB reset
      }

      // Start a new POC reset period
      if (m_layerId == 0)   // Assuming BL picture is always present at encoder; for other AU structures, need to change this
      {
        Int periodId = rand() % 64;
        m_lastPocPeriodId = (periodId == m_lastPocPeriodId) ? (periodId + 1) % 64 : periodId ;

        for( UInt i = 0; i < m_pcCfg->getNumLayer(); i++ )
        {
          m_ppcTEncTop[i]->setPocDecrementedInDPBFlag(false);
        }
      }
      else
      {
        m_lastPocPeriodId = m_ppcTEncTop[0]->getGOPEncoder()->getLastPocPeriodId();
      }
      slice->setPocResetPeriodId(m_lastPocPeriodId);
    }
    else
    {
      slice->setPocResetIdc( 0 );
    }
  }
}

Void TEncGOP::updatePocValuesOfPics(Int const pocCurr, TComSlice *const slice)
{
  UInt affectedLayerList[MAX_NUM_LAYER_IDS];
  Int  numAffectedLayers;

  affectedLayerList[0] = m_layerId;
  numAffectedLayers = 1;

  if (m_pcEncTop->getVPS()->getVpsPocLsbAlignedFlag())
  {
    for (UInt j = 0; j < m_pcEncTop->getVPS()->getNumPredictedLayers(m_layerId); j++)
    {
      affectedLayerList[j + 1] = m_pcEncTop->getVPS()->getPredictedLayerId(m_layerId, j);
    }
    numAffectedLayers = m_pcEncTop->getVPS()->getNumPredictedLayers(m_layerId) + 1;
  }

  Int pocAdjustValue = pocCurr - m_pcEncTop->getPocAdjustmentValue();

  // New POC reset period
  Int maxPocLsb, pocLsbVal, pocMsbDelta, pocLsbDelta, deltaPocVal;

  maxPocLsb   = 1 << slice->getSPS()->getBitsForPOC();

  Int adjustedPocValue = pocCurr;

  if (m_pcEncTop->getFirstPicInLayerDecodedFlag())
  {
    pocLsbVal   = (slice->getPocResetIdc() == 3)
      ? slice->getPocLsbVal()
      : pocAdjustValue % maxPocLsb; 
    pocMsbDelta = pocAdjustValue - pocLsbVal;
    pocLsbDelta = (slice->getPocResetIdc() == 2 || ( slice->getPocResetIdc() == 3 && slice->getFullPocResetFlag() )) 
      ? pocLsbVal 
      : 0; 
    deltaPocVal = pocMsbDelta  + pocLsbDelta;

    Int origDeltaPocVal = deltaPocVal;  // original value needed when updating POC adjustment value

    // IDR picture in base layer, non-IDR picture in other layers, poc_lsb_aligned_flag = 1
    if( slice->getPocMsbNeeded() )
    {
      if (slice->getLayerId() == 0)
      {
        Int highestPoc = INT_MIN;

        // Find greatest POC in DPB for layer 0
        for (TComList<TComPic*>::iterator iterPic = m_pcEncTop->getListPic()->begin(); iterPic != m_pcEncTop->getListPic()->end(); ++iterPic)
        {
          TComPic *dpbPic = *iterPic;
          if (dpbPic->getReconMark() && dpbPic->getLayerId() == 0 && dpbPic->getPOC() > highestPoc)
          {
            highestPoc = dpbPic->getPOC();
          }
        }
        deltaPocVal = (highestPoc - (highestPoc & (maxPocLsb - 1))) + 1*maxPocLsb;
        m_pcEncTop->setCurrPocMsb(deltaPocVal);
      }
      else
      {
        deltaPocVal = m_ppcTEncTop[0]->getCurrPocMsb();  // copy from base layer
      }
      slice->setPocMsbVal(deltaPocVal);
    }

    for( UInt layerIdx = 0; layerIdx < numAffectedLayers; layerIdx++ )
    {
      UInt lIdx = slice->getVPS()->getLayerIdxInVps(affectedLayerList[layerIdx]);

      if( !m_ppcTEncTop[lIdx]->getPocDecrementedInDPBFlag() )
      {
        m_ppcTEncTop[lIdx]->setPocDecrementedInDPBFlag(true);

        // Decrement value of associatedIrapPoc of the TEncGop object
        m_ppcTEncTop[lIdx]->getGOPEncoder()->m_associatedIRAPPOC -= deltaPocVal;

        // Decrememnt the value of m_pocCRA
        m_ppcTEncTop[lIdx]->getGOPEncoder()->m_pocCRA -= deltaPocVal;

        TComList<TComPic*>::iterator  iterPic = m_ppcTEncTop[lIdx]->getListPic()->begin();
        while (iterPic != m_ppcTEncTop[lIdx]->getListPic()->end())
        {
          TComPic *dpbPic = *iterPic;

          if( dpbPic->getReconMark() )
          {
            for( Int i = dpbPic->getNumAllocatedSlice() - 1; i >= 0; i-- )
            {
              TComSlice *dpbPicSlice = dpbPic->getSlice( i );
              TComReferencePictureSet *dpbPicRps = dpbPicSlice->getLocalRPS();

              // Decrement POC of slice
              dpbPicSlice->setPOC( dpbPicSlice->getPOC() - deltaPocVal );

              // Decrement POC value stored in the RPS of each such slice
              for( Int j = dpbPicRps->getNumberOfPictures() - 1; j >= 0; j-- )
              {
                dpbPicRps->setPOC( j, dpbPicRps->getPOC(j) - deltaPocVal );
              }
              
              dpbPicSlice->setRPS(dpbPicRps);

              // Decrement value of refPOC
              dpbPicSlice->decrementRefPocValues( deltaPocVal );

              // Update value of associatedIrapPoc of each slice
              dpbPicSlice->setAssociatedIRAPPOC( dpbPicSlice->getAssociatedIRAPPOC() - deltaPocVal );

              if( slice->getPocMsbNeeded() )
              {
                // this delta value is needed when computing delta POCs in reference picture set initialization
                dpbPicSlice->setPocResetDeltaPoc(dpbPicSlice->getPocResetDeltaPoc() + (deltaPocVal - pocLsbVal));
              }
            }
          }
          iterPic++;
        }
      }
    }

    // Actual POC value before reset
    adjustedPocValue = pocCurr - m_pcEncTop->getPocAdjustmentValue();

    // Set MSB value before reset
    Int tempLsbVal = adjustedPocValue & (maxPocLsb - 1);
    if (!slice->getPocMsbNeeded())  // set poc msb normally if special msb handling is not needed
    {
      slice->setPocMsbVal(adjustedPocValue - tempLsbVal);
    }

    // Set LSB value before reset - this is needed in the case of resetIdc = 2
    slice->setPicOrderCntLsb( tempLsbVal );

    // Cumulative delta
    deltaPocVal = origDeltaPocVal;  // restore deltaPoc for correct adjustment value update

    m_pcEncTop->setPocAdjustmentValue( m_pcEncTop->getPocAdjustmentValue() + deltaPocVal );
  }

  // New LSB value, after reset
  adjustedPocValue = pocCurr - m_pcEncTop->getPocAdjustmentValue();
  Int newLsbVal = adjustedPocValue & (maxPocLsb - 1);

  // Set value of POC current picture after RESET
  if( slice->getPocResetIdc() == 1 )
  {
    slice->setPOC( newLsbVal );
  }
  else if( slice->getPocResetIdc() == 2 )
  {
    slice->setPOC( 0 );
  }
  else if( slice->getPocResetIdc() == 3 )
  {
    Int picOrderCntMsb = slice->getCurrMsb( newLsbVal, 
      slice->getFullPocResetFlag() ? 0 : slice->getPocLsbVal(), 
      0,
      maxPocLsb );
    slice->setPOC( picOrderCntMsb + newLsbVal );
  }
  else
  {
    assert(0);
  }
}

#if CGS_3D_ASYMLUT
Void TEncGOP::xDetermine3DAsymLUT( TComSlice * pSlice, TComPic * pCurPic, UInt refLayerIdc, TEncCfg * pCfg, Bool bSignalPPS )
{
  Int nCGSFlag = pSlice->getPPS()->getCGSFlag();
  m_Enc3DAsymLUTPPS.setPPSBit( 0 );
  Double dErrorUpdatedPPS = 0 , dErrorPPS = 0;

#if R0179_ENC_OPT_3DLUT_SIZE
  Int nTLthres = m_pcCfg->getCGSLutSizeRDO() ? 2:7;
  Double dFrameLambda; 
#if FULL_NBIT
  Int    SHIFT_QP = 12 + 6 * (pSlice->getBitDepthY() - 8);
#else
  Int    SHIFT_QP = 12; 
#endif 
  Int QP = pSlice->getSliceQp();

  // set frame lambda
  dFrameLambda = 0.68 * pow (2, (QP  - SHIFT_QP) / 3.0) * (m_pcCfg->getGOPSize() > 1 && pSlice->isInterB()? 2 : 1);

  if(m_pcCfg->getCGSLutSizeRDO() == 1 && (!bSignalPPS && (pSlice->getDepth() < nTLthres)))
  {
    dErrorUpdatedPPS = m_Enc3DAsymLUTPicUpdate.derive3DAsymLUT( pSlice , pCurPic , refLayerIdc , pCfg , bSignalPPS , m_pcEncTop->getElRapSliceTypeB(), dFrameLambda );
  }
  else if (pSlice->getDepth() >= nTLthres)
  {
    dErrorUpdatedPPS = MAX_DOUBLE;
  }
  else // if (m_pcCfg->getCGSLutSizeRDO() = 0 || bSignalPPS)
#endif   
    dErrorUpdatedPPS = m_Enc3DAsymLUTPicUpdate.derive3DAsymLUT( pSlice , pCurPic , refLayerIdc , pCfg , bSignalPPS , m_pcEncTop->getElRapSliceTypeB() );


  if( bSignalPPS )
  {
    m_Enc3DAsymLUTPPS.copy3DAsymLUT( &m_Enc3DAsymLUTPicUpdate );
    pSlice->setCGSOverWritePPS( 1 ); // regular PPS update
  }
  else if( nCGSFlag )
  {
#if R0179_ENC_OPT_3DLUT_SIZE
    if(pSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_RASL_R || pSlice->getNalUnitType() == NAL_UNIT_CODED_SLICE_RASL_N) 
    {
      pSlice->setCGSOverWritePPS( 0 ); 
    }
    else if (pSlice->getDepth() >= nTLthres) 
    {
      pSlice->setCGSOverWritePPS( 0 ); 
    }
    else
    {
#endif    
      dErrorPPS = m_Enc3DAsymLUTPPS.estimateDistWithCur3DAsymLUT( pCurPic , refLayerIdc );
      Double dFactor = pCfg->getIntraPeriod() == 1 ? 0.99 : 0.9;

#if R0179_ENC_OPT_3DLUT_SIZE 
      if( m_pcCfg->getCGSLutSizeRDO() )
      {
        dErrorPPS = dErrorPPS/m_Enc3DAsymLUTPicUpdate.getDistFactor(pSlice->getSliceType(), pSlice->getDepth()); 
      }
#endif
      pSlice->setCGSOverWritePPS( dErrorUpdatedPPS < dFactor * dErrorPPS );
#if R0179_ENC_OPT_3DLUT_SIZE
    }
#endif
    if( pSlice->getCGSOverWritePPS() )
    {
      m_Enc3DAsymLUTPPS.copy3DAsymLUT( &m_Enc3DAsymLUTPicUpdate );
    }
  }
}

Void TEncGOP::xDownScalePic( TComPicYuv* pcYuvSrc, TComPicYuv* pcYuvDest, const BitDepths& bitDepth, const Int posScalingFactorX)
{
  pcYuvSrc->setBorderExtension(false);
  pcYuvSrc->extendPicBorder(); // extend the border.
  pcYuvSrc->setBorderExtension(false);

  Int iWidth  = pcYuvSrc->getWidth(COMPONENT_Y);
  Int iHeight = pcYuvSrc->getHeight(COMPONENT_Y); 

  if(!m_temp)
  {
    xInitDs(iWidth, iHeight, m_pcCfg->getIntraPeriod() > 1, posScalingFactorX);
  }

  xFilterImg(pcYuvSrc->getAddr(COMPONENT_Y),  pcYuvSrc->getStride(COMPONENT_Y),  pcYuvDest->getAddr(COMPONENT_Y),  pcYuvDest->getStride(COMPONENT_Y),  iHeight,    iWidth,    bitDepth, COMPONENT_Y);
  xFilterImg(pcYuvSrc->getAddr(COMPONENT_Cb), pcYuvSrc->getStride(COMPONENT_Cb), pcYuvDest->getAddr(COMPONENT_Cb), pcYuvDest->getStride(COMPONENT_Cb), iHeight>>1, iWidth>>1, bitDepth, COMPONENT_Cb);
  xFilterImg(pcYuvSrc->getAddr(COMPONENT_Cr), pcYuvSrc->getStride(COMPONENT_Cr), pcYuvDest->getAddr(COMPONENT_Cr), pcYuvDest->getStride(COMPONENT_Cr), iHeight>>1, iWidth>>1, bitDepth, COMPONENT_Cr);  
}
const Pel TEncGOP::m_phaseFilter0T0[CGS_FILTER_PHASES_2X][CGS_FILTER_LENGTH] =
{
  {0,  2,  -3,  -9,   6,  39,  58,  39,   6,  -9,  -3,  2,  0},  
  {0,  0,   0,  -2,  8,  -20, 116,  34, -10,   2,   0,  0,  0},            //{0,  1,  -1,  -8,  -1,  31,  57,  47,  13,  -7,  -5,  1,  0},  //
  {0,  1,   0,  -7,  -5,  22,  53,  53,  22,  -5,  -7,  0,  1},  
  {0,  0,   1,  -5,  -7,  13,  47,  57,  31,  -1,  -8, -1,  1}  
};

const Pel TEncGOP::m_phaseFilter0T1[CGS_FILTER_PHASES_2X][CGS_FILTER_LENGTH] =
{
  {0,  4,  0,  -12, 0,  40,  64,  40,   0, -12,  0,  4,  0},
  {0,  0,  0,  -2,  8, -20, 116,  34, -10,   2,  0,  0,  0},               //{0,  1,  -1,  -8,  -1,  31,  57,  47,  13,  -7,  -5,  1,  0},  //
  {0,  1,  0,  -7, -5,  22,  53,  53,  22,  -5, -7,  0,  1},  
  {0,  0,  1,  -5, -7,  13,  47,  57,  31,  -1, -8, -1,  1}  
};

const Pel TEncGOP::m_phaseFilter0T1Chroma[CGS_FILTER_PHASES_2X][CGS_FILTER_LENGTH] =
{
  {0,  0,  0,  0,  0,   0, 128,  0,   0,  0,  0,  0, 0},
  {0,  0,  0, -2,  8, -20, 116, 34, -10,  2,  0,  0, 0},                   //{0,  1,  -1,  -8,  -1,  31,  57,  47,  13,  -7,  -5,  1,  0},  //
  {0,  1,  0, -7, -5,  22,  53, 53,  22, -5, -7,  0, 1},  
  {0,  0,  1, -5, -7,  13,  47, 57,  31, -1, -8, -1, 1}  
};

const Pel TEncGOP::m_phaseFilter1[CGS_FILTER_PHASES_1X][CGS_FILTER_LENGTH] =
{
  {0,  0, 5, -6, -10, 37, 76, 37, -10,  -6, 5, 0,  0},    
  {0, -1, 5, -3, -12, 29, 75, 45,  -7,  -8, 5, 0,  0},    
  {0, -1, 4, -1, -13, 22, 73, 52,  -3, -10, 4, 1,  0},    
  {0, -1, 4,  1, -13, 14, 70, 59,   2, -12, 3, 2, -1},  
  {0, -1, 3,  2, -13,  8, 65, 65,   8, -13, 2, 3, -1},    
  {0, -1, 2,  3, -12,  2, 59, 70,  14, -13, 1, 4, -1},    
  {0,  0, 1,  4, -10, -3, 52, 73,  22, -13,-1, 4, -1},    
  {0,  0, 0,  5,  -8, -7, 45, 75,  29, -12,-3, 5, -1}    
};

#if CGS_GCC_NO_VECTORIZATION  
#ifdef __GNUC__
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if GCC_VERSION > 40600
__attribute__((optimize("no-tree-vectorize")))
#endif
#endif
#endif
Void TEncGOP::xFilterImg( Pel *src, Int iSrcStride, Pel *dst, Int iDstStride, Int height, Int width, const BitDepths& bitDepth, ComponentID comp )
{
  Int height2, width2;

  Pel *srcLine = src;
  Pel *dstLine = dst;
  Int length = m_cgsFilterLength;
  Int shift  = bitDepth.recon[CHANNEL_TYPE_LUMA] - bitDepth.recon[CHANNEL_TYPE_CHROMA];
  Int shift2 = 2*7 + shift;
  Int roundingOffset = (1 << (shift2 - 1));
  Int maxVal = (1<<(bitDepth.recon[toChannelType(comp)]-shift))-1;
  height2 = (height * m_cgsFilterPhases) / m_iN;
  width2  = (width  * m_cgsFilterPhases) / m_iN;

  m_phaseFilter = comp == COMPONENT_Y ? m_phaseFilterLuma : m_phaseFilterChroma;

  // horizontal filtering
  for( Int j1 = 0; j1 < height; j1++ )
  {
    Int i0 = -m_iN;
    Int *tmp = m_temp[j1];
    
    for( Int i1 = 0; i1 < width2; i1++ )
    {
      i0 += m_iN;
      Int div_i0 = i0 / m_cgsFilterPhases;

      Pel *srcTmp =  srcLine + ( div_i0 - (length >> 1));
      const Pel *filter = m_phaseFilter[i0 - div_i0 * m_cgsFilterPhases]; // phase_filter[i0 % M]

      Int sum = 0;
      for(Int k = 0; k < length; k++)
      {
        sum += (*srcTmp++) * (*filter++);
      }
      *tmp++ = sum;
    }
    srcLine += iSrcStride;
  }

  // pad temp (vertical)
  for( Int k = -(length>>1); k<0; k++ )
  {
    memcpy(m_temp[k], m_temp[0], width2*sizeof(Int));
  }

  for( Int k = height; k < (height + (length>>1)); k++)
  {
    memcpy(m_temp[k], m_temp[k-1], (width2) * sizeof(Int));
  }

  // vertical filtering
  Int j0 = comp == COMPONENT_Y ? -m_iN : (1 - m_iN);
  
  for( Int j1 = 0; j1 < height2; j1++ )
  {
    j0 += m_iN;
    Int div_j0 = j0 / m_cgsFilterPhases;

    Pel* dstTmp = dstLine;

    Int **temp = &m_temp[div_j0 - (length>>1)];
    const Pel *filter = m_phaseFilter[j0 - div_j0 * m_cgsFilterPhases]; // phase_filter[j0 % M]

    for( Int i1 = 0; i1 < width2;i1++ )
    {
      Int sum=0;
      for( Int k = 0; k < length; k++ )
      {
        sum += temp[k][i1] * filter[k];
      }
      sum = ((sum + roundingOffset) >> shift2);

      *dstTmp++ = Clip3<Short>(sum < 0 ? 0 : sum, maxVal, sum);
    }
    dstLine += iDstStride;
  }
}

Void TEncGOP::xInitDs( const Int iWidth, const Int iHeight, const Bool allIntra, const Int posScalingFactorX )
{
  m_cgsFilterLength = CGS_FILTER_LENGTH;

  if( posScalingFactorX == POS_SCALING_FACTOR_2X )
  {
    m_cgsFilterPhases = CGS_FILTER_PHASES_2X;
    m_iN = 8;
    m_phaseFilterLuma = allIntra ? m_phaseFilter0T1 : m_phaseFilter0T0;
    m_phaseFilterChroma = m_phaseFilter0T1Chroma;    
  }
  else
  {
    m_cgsFilterPhases = CGS_FILTER_PHASES_1X;
    m_iN = 12;
    m_phaseFilterLuma = m_phaseFilterChroma = m_phaseFilter1;
    m_phaseFilter = m_phaseFilter1;
  }

  xCreate2DArray( &m_temp, iHeight, iWidth * m_cgsFilterPhases/m_iN, m_cgsFilterLength >> 1, 0 );
}

Int TEncGOP::xCreate2DArray(Int ***array2D, Int dim0, Int dim1, Int iPadY, Int iPadX)
{
  Int i;
  Int *curr = NULL;
  Int iHeight, iWidth;

  iHeight = dim0+2*iPadY;
  iWidth = dim1+2*iPadX;
  (*array2D) = (Int**)malloc(iHeight*sizeof(Int*));
  *(*array2D) = (Int* )xMalloc(Int, iHeight*iWidth);

  (*array2D)[0] += iPadX;
  curr = (*array2D)[0];
  for(i = 1 ; i < iHeight; i++)
  {
    curr += iWidth;
    (*array2D)[i] = curr;
  }
  (*array2D) = &((*array2D)[iPadY]);

  return 0;
}

Void TEncGOP::xDestroy2DArray(Int **array2D, Int iPadY, Int iPadX)
{
  if (array2D)
  {
    if (*array2D)
    {
      xFree(array2D[-iPadY]-iPadX);
    }
    else 
    {
      printf("xDestroy2DArray: trying to free unused memory\r\nPress Any Key\r\n");
    }

    free (&array2D[-iPadY]);
  } 
  else
  {
    printf("xDestroy2DArray: trying to free unused memory\r\nPress Any Key\r\n");
  }
}
#endif

Void TEncGOP::xCheckLayerReset(TComSlice *slice)
{
  Bool layerResetFlag;
  Int dolLayerId;

  if (slice->isIRAP() && slice->getLayerId() > 0)
  {
    if (m_prevPicHasEos)
    {
      layerResetFlag = true;
      dolLayerId = slice->getLayerId();
    }
    else if ((slice->isCRA() && slice->getHandleCraAsBlaFlag()) || (slice->isIDR() && slice->getCrossLayerBLAFlag()) || slice->isBLA())
    {
      layerResetFlag = true;
      dolLayerId = slice->getLayerId();
    }
    else
    {
      layerResetFlag = false;
    }

    if (layerResetFlag)
    {
      for (Int i = 0; i < slice->getVPS()->getNumPredictedLayers(dolLayerId); i++)
      {
        Int iLayerId = slice->getVPS()->getPredictedLayerId(dolLayerId, i);
        m_ppcTEncTop[slice->getVPS()->getLayerIdxInVps(iLayerId)]->setLayerInitializedFlag(false);
        m_ppcTEncTop[slice->getVPS()->getLayerIdxInVps(iLayerId)]->setFirstPicInLayerDecodedFlag(false);
      }

      // Each picture that is in the DPB and has nuh_layer_id equal to dolLayerId is marked as "unused for reference".
      for (TComList<TComPic*>::iterator pic = m_ppcTEncTop[slice->getVPS()->getLayerIdxInVps(dolLayerId)]->getListPic()->begin(); pic != m_ppcTEncTop[slice->getVPS()->getLayerIdxInVps(dolLayerId)]->getListPic()->end(); pic++)
      {
        if ((*pic)->getSlice(0)->getPOC() != slice->getPOC())
        {
          (*pic)->getSlice(0)->setReferenced(false);
        }
      }

      // Each picture that is in DPB and has nuh_layer_id equal to any value of IdPredictedLayer[dolLayerId][i]
      // for the values of i in range of 0 to NumPredictedLayers[dolLayerId] - 1, inclusive, is marked as "unused for reference"
      for (UInt i = 0; i < slice->getVPS()->getNumPredictedLayers(dolLayerId); i++)
      {
        UInt predLId = slice->getVPS()->getPredictedLayerId(dolLayerId, i);
        for (TComList<TComPic*>::iterator pic = m_ppcTEncTop[slice->getVPS()->getLayerIdxInVps(predLId)]->getListPic()->begin(); pic != m_ppcTEncTop[slice->getVPS()->getLayerIdxInVps(predLId)]->getListPic()->end(); pic++)
        {
          if ((*pic)->getSlice(0)->getPOC() != slice->getPOC())
          {
            (*pic)->getSlice(0)->setReferenced(false);
          }
        }
      }
    }
  }
}

Void TEncGOP::xSetNoRaslOutputFlag(TComSlice *slice)
{
  if (slice->isIRAP())
  {
    m_noRaslOutputFlag = slice->getHandleCraAsBlaFlag();  // default value
    if (slice->isIDR() || slice->isBLA() || m_bFirst || m_prevPicHasEos)
    {
      m_noRaslOutputFlag = true;
    }
    else if (!m_ppcTEncTop[slice->getVPS()->getLayerIdxInVps(m_layerId)]->getLayerInitializedFlag())
    {
      Bool refLayersInitialized = true;
      for (UInt j = 0; j < slice->getVPS()->getNumDirectRefLayers(m_layerId); j++)
      {
        UInt refLayerId = slice->getVPS()->getRefLayerId(m_layerId, j);
        if (!m_ppcTEncTop[slice->getVPS()->getLayerIdxInVps(refLayerId)]->getLayerInitializedFlag())
        {
          refLayersInitialized = false;
        }
      }
      if (refLayersInitialized)
      {
        m_noRaslOutputFlag = true;
      }
    }
  }
}

Void TEncGOP::xSetLayerInitializedFlag(TComSlice *slice)
{
  if (slice->isIRAP() && m_noRaslOutputFlag)
  {
    if (m_layerId == 0)
    {
      m_ppcTEncTop[slice->getVPS()->getLayerIdxInVps(m_layerId)]->setLayerInitializedFlag(true);
    }
    else if (!m_ppcTEncTop[slice->getVPS()->getLayerIdxInVps(m_layerId)]->getLayerInitializedFlag() && slice->getVPS()->getNumDirectRefLayers(m_layerId) == 0)
    {
      m_ppcTEncTop[slice->getVPS()->getLayerIdxInVps(m_layerId)]->setLayerInitializedFlag(true);
    }
    else if (!m_ppcTEncTop[slice->getVPS()->getLayerIdxInVps(m_layerId)]->getLayerInitializedFlag())
    {
      Bool refLayersInitialized = true;
      for (UInt j = 0; j < slice->getVPS()->getNumDirectRefLayers(m_layerId); j++)
      {
        UInt refLayerId = slice->getVPS()->getRefLayerId(m_layerId, j);
        if (!m_ppcTEncTop[slice->getVPS()->getLayerIdxInVps(refLayerId)]->getLayerInitializedFlag())
        {
          refLayersInitialized = false;
        }
      }
      if (refLayersInitialized)
      {
        m_ppcTEncTop[slice->getVPS()->getLayerIdxInVps(m_layerId)]->setLayerInitializedFlag(true);
      }
    }
  }
}
#endif //SVC_EXTENSION

//! \}
