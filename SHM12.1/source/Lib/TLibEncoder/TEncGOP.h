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

/** \file     TEncGOP.h
    \brief    GOP encoder class (header)
*/

#ifndef __TENCGOP__
#define __TENCGOP__

#include <list>

#include <stdlib.h>

#include "TLibCommon/TComList.h"
#include "TLibCommon/TComPic.h"
#include "TLibCommon/TComBitCounter.h"
#include "TLibCommon/TComLoopFilter.h"
#include "TLibCommon/AccessUnit.h"
#include "TEncSampleAdaptiveOffset.h"
#include "TEncSlice.h"
#include "TEncEntropy.h"
#include "TEncCavlc.h"
#include "TEncSbac.h"
#include "SEIwrite.h"
#include "SEIEncoder.h"

#include "TEncAnalyze.h"
#include "TEncRateCtrl.h"
#include <vector>

#if CGS_3D_ASYMLUT
#include "TEnc3DAsymLUT.h"
#endif

//! \ingroup TLibEncoder
//! \{

class TEncTop;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

class TEncGOP
{
  class DUData
  {
  public:
    DUData()
    :accumBitsDU(0)
    ,accumNalsDU(0) {};

    Int accumBitsDU;
    Int accumNalsDU;
  };

private:

  TEncAnalyze             m_gcAnalyzeAll;
  TEncAnalyze             m_gcAnalyzeI;
  TEncAnalyze             m_gcAnalyzeP;
  TEncAnalyze             m_gcAnalyzeB;

  TEncAnalyze             m_gcAnalyzeAll_in;
  //  Data
  Bool                    m_bLongtermTestPictureHasBeenCoded;
  Bool                    m_bLongtermTestPictureHasBeenCoded2;
  UInt                    m_numLongTermRefPicSPS;
  UInt                    m_ltRefPicPocLsbSps[MAX_NUM_LONG_TERM_REF_PICS];
  Bool                    m_ltRefPicUsedByCurrPicFlag[MAX_NUM_LONG_TERM_REF_PICS];
  Int                     m_iLastIDR;
  Int                     m_iGopSize;
  Int                     m_iNumPicCoded;
  Bool                    m_bFirst;
  Int                     m_iLastRecoveryPicPOC;

  //  Access channel
  TEncTop*                m_pcEncTop;
  TEncCfg*                m_pcCfg;
  TEncSlice*              m_pcSliceEncoder;
  TComList<TComPic*>*     m_pcListPic;

  TEncEntropy*            m_pcEntropyCoder;
  TEncCavlc*              m_pcCavlcCoder;
  TEncSbac*               m_pcSbacCoder;
  TEncBinCABAC*           m_pcBinCABAC;
  TComLoopFilter*         m_pcLoopFilter;

  SEIWriter               m_seiWriter;

  //--Adaptive Loop filter
  TEncSampleAdaptiveOffset*  m_pcSAO;
  TEncRateCtrl*           m_pcRateCtrl;
  // indicate sequence first
  Bool                    m_bSeqFirst;

  // clean decoding refresh
  Bool                    m_bRefreshPending;
  Int                     m_pocCRA;
  NalUnitType             m_associatedIRAPType;
  Int                     m_associatedIRAPPOC;

  std::vector<Int> m_vRVM_RP;
  UInt                    m_lastBPSEI;
  UInt                    m_totalCoded;
  Bool                    m_bufferingPeriodSEIPresentInAU;
  SEIEncoder              m_seiEncoder;
#if W0038_DB_OPT
  TComPicYuv*             m_pcDeblockingTempPicYuv;
  Int                     m_DBParam[MAX_ENCODER_DEBLOCKING_QUALITY_LAYERS][4];   //[layer_id][0: available; 1: bDBDisabled; 2: Beta Offset Div2; 3: Tc Offset Div2;]
#endif

#if SVC_EXTENSION
  Int                     m_pocCraWithoutReset;
  Int                     m_associatedIrapPocBeforeReset;
  UInt                    m_layerId;      
  TEncTop**               m_ppcTEncTop;
  TEncSearch*             m_pcPredSearch;                       ///< encoder search class
#if CGS_3D_ASYMLUT
  TEnc3DAsymLUT           m_Enc3DAsymLUTPicUpdate;
  TEnc3DAsymLUT           m_Enc3DAsymLUTPPS;
  TComPicYuv*             m_pColorMappedPic;

  UChar                   m_cgsFilterLength;
  UChar                   m_cgsFilterPhases;
  Int                     m_iN;
  Int                   **m_temp;
  const Pel             (*m_phaseFilter)[CGS_FILTER_LENGTH];
  const Pel             (*m_phaseFilterLuma)[CGS_FILTER_LENGTH];
  const Pel             (*m_phaseFilterChroma)[CGS_FILTER_LENGTH];
  static const Pel        m_phaseFilter0T0[CGS_FILTER_PHASES_2X][CGS_FILTER_LENGTH];
  static const Pel        m_phaseFilter0T1[CGS_FILTER_PHASES_2X][CGS_FILTER_LENGTH];
  static const Pel        m_phaseFilter0T1Chroma[CGS_FILTER_PHASES_2X][CGS_FILTER_LENGTH];
  static const Pel        m_phaseFilter1[CGS_FILTER_PHASES_1X][CGS_FILTER_LENGTH];
#endif
  Int                     m_lastPocPeriodId;
  Bool                    m_noRaslOutputFlag;
  Bool                    m_prevPicHasEos;
  static Bool             m_signalledVPS;
#endif
  
public:
  TEncGOP();
  virtual ~TEncGOP();
  
#if SVC_EXTENSION
  Void  create      ( UInt layerId );
#else
  Void  create      ();
#endif
  Void  destroy     ();

  Void  init        ( TEncTop* pcTEncTop );
#if SVC_EXTENSION  
  Void  compressGOP ( Int iPicIdInGOP, Int iPOCLast, Int iNumPicRcvd, TComList<TComPic*>& rcListPic, TComList<TComPicYuv*>& rcListPicYuvRec,
                      std::list<AccessUnit>& accessUnitsInGOP, Bool isField, Bool isTff, const InputColourSpaceConversion snr_conversion, const Bool printFrameMSE );
#else
  Void  compressGOP ( Int iPOCLast, Int iNumPicRcvd, TComList<TComPic*>& rcListPic, TComList<TComPicYuv*>& rcListPicYuvRec,
                      std::list<AccessUnit>& accessUnitsInGOP, Bool isField, Bool isTff, const InputColourSpaceConversion snr_conversion, const Bool printFrameMSE );
#endif
  Void  xAttachSliceDataToNalUnit (OutputNALUnit& rNalu, TComOutputBitstream* pcBitstreamRedirect);


  Int   getGOPSize()          { return  m_iGopSize;  }

  TComList<TComPic*>*   getListPic()      { return m_pcListPic; }

  Void  printOutSummary      ( UInt uiNumAllPicCoded, Bool isField, const Bool printMSEBasedSNR, const Bool printSequenceMSE, const BitDepths &bitDepths );
  Void  preLoopFilterPicAll  ( TComPic* pcPic, UInt64& ruiDist );

  TEncSlice*  getSliceEncoder()   { return m_pcSliceEncoder; }
  NalUnitType getNalUnitType( Int pocCurr, Int lastIdr, Bool isField );
  Void arrangeLongtermPicturesInRPS(TComSlice *, TComList<TComPic*>& );

#if SVC_EXTENSION
  Void  determinePocResetIdc( Int const pocCurr, TComSlice *const slice);
  Int   getIntraRefreshInterval()   { return m_pcCfg->getIntraPeriod(); }
  Int   getIntraRefreshType()       { return m_pcCfg->getDecodingRefreshType(); }  
  Int   getLastPocPeriodId()        { return m_lastPocPeriodId; }
  Void  setLastPocPeriodId(Int x)   { m_lastPocPeriodId = x;    }
  Void  updatePocValuesOfPics( Int const pocCurr, TComSlice *const slice);

  TEncAnalyze* getAnalyzeAll()      { return &m_gcAnalyzeAll;  }
  TEncAnalyze* getAnalyzeI()        { return &m_gcAnalyzeI;    }
  TEncAnalyze* getAnalyzeP()        { return &m_gcAnalyzeP;    }
  TEncAnalyze* getAnalyzeB()        { return &m_gcAnalyzeB;    }
  TEncAnalyze* getAnalyzeAllin()    { return &m_gcAnalyzeAll_in; }
  Double       calculateRVM()       { return xCalculateRVM();    }
#endif

protected:
  TEncRateCtrl* getRateCtrl()       { return m_pcRateCtrl;  }

protected:

  Void  xInitGOP          ( Int iPOCLast, Int iNumPicRcvd, Bool isField );
  Void  xGetBuffer        ( TComList<TComPic*>& rcListPic, TComList<TComPicYuv*>& rcListPicYuvRecOut, Int iNumPicRcvd, Int iTimeOffset, TComPic*& rpcPic, TComPicYuv*& rpcPicYuvRecOut, Int pocCurr, Bool isField );

  Void  xCalculateAddPSNRs         ( const Bool isField, const Bool isFieldTopFieldFirst, const Int iGOPid, TComPic* pcPic, const AccessUnit&accessUnit, TComList<TComPic*> &rcListPic, Double dEncTime, const InputColourSpaceConversion snr_conversion, const Bool printFrameMSE, Double* PSNR_Y );
  Void  xCalculateAddPSNR          ( TComPic* pcPic, TComPicYuv* pcPicD, const AccessUnit&, Double dEncTime, const InputColourSpaceConversion snr_conversion, const Bool printFrameMSE, Double* PSNR_Y );
  Void  xCalculateInterlacedAddPSNR( TComPic* pcPicOrgFirstField, TComPic* pcPicOrgSecondField,
                                     TComPicYuv* pcPicRecFirstField, TComPicYuv* pcPicRecSecondField,
                                     const InputColourSpaceConversion snr_conversion, const Bool printFrameMSE, Double* PSNR_Y );

  UInt64 xFindDistortionFrame (TComPicYuv* pcPic0, TComPicYuv* pcPic1, const BitDepths &bitDepths);

  Double xCalculateRVM();

  Void xWriteAccessUnitDelimiter (AccessUnit &accessUnit, TComSlice *slice);

  Void xCreateIRAPLeadingSEIMessages (SEIMessages& seiMessages, const TComSPS *sps, const TComPPS *pps);
  Void xCreatePerPictureSEIMessages (Int picInGOP, SEIMessages& seiMessages, SEIMessages& nestedSeiMessages, TComSlice *slice);
  Void xCreatePictureTimingSEI  (Int IRAPGOPid, SEIMessages& seiMessages, SEIMessages& nestedSeiMessages, SEIMessages& duInfoSeiMessages, TComSlice *slice, Bool isField, std::deque<DUData> &duData);
  Void xUpdateDuData(AccessUnit &testAU, std::deque<DUData> &duData);
  Void xUpdateTimingSEI(SEIPictureTiming *pictureTimingSEI, std::deque<DUData> &duData, const TComSPS *sps);
  Void xUpdateDuInfoSEI(SEIMessages &duInfoSeiMessages, SEIPictureTiming *pictureTimingSEI);

  Void xCreateScalableNestingSEI (SEIMessages& seiMessages, SEIMessages& nestedSeiMessages);
#if O0164_MULTI_LAYER_HRD
  Void xWriteSEI (NalUnitType naluType, SEIMessages& seiMessages, AccessUnit &accessUnit, AccessUnit::iterator &auPos, Int temporalId, const TComVPS *vps, const TComSPS *sps, const SEIScalableNesting* nestingSei=NULL, const SEIBspNesting* bspNestingSei=NULL);
  Void xWriteSEISeparately (NalUnitType naluType, SEIMessages& seiMessages, AccessUnit &accessUnit, AccessUnit::iterator &auPos, Int temporalId, const TComVPS *vps, const TComSPS *sps, const SEIScalableNesting* nestingSei=NULL, const SEIBspNesting* bspNestingSei=NULL);
#else
  Void xWriteSEI (NalUnitType naluType, SEIMessages& seiMessages, AccessUnit &accessUnit, AccessUnit::iterator &auPos, Int temporalId, const TComSPS *sps);
  Void xWriteSEISeparately (NalUnitType naluType, SEIMessages& seiMessages, AccessUnit &accessUnit, AccessUnit::iterator &auPos, Int temporalId, const TComSPS *sps);
#endif
  Void xClearSEIs(SEIMessages& seiMessages, Bool deleteMessages);
#if O0164_MULTI_LAYER_HRD
  Void xWriteLeadingSEIOrdered (SEIMessages& seiMessages, SEIMessages& duInfoSeiMessages, AccessUnit &accessUnit, Int temporalId, const TComVPS *vps, const TComSPS *sps, Bool testWrite, const SEIScalableNesting* nestingSei=NULL, const SEIBspNesting* bspNestingSei=NULL);
  Void xWriteLeadingSEIMessages  (SEIMessages& seiMessages, SEIMessages& duInfoSeiMessages, AccessUnit &accessUnit, Int temporalId, const TComVPS *vps, const TComSPS *sps, std::deque<DUData> &duData, const SEIScalableNesting* nestingSei=NULL, const SEIBspNesting* bspNestingSei=NULL);
  Void xWriteTrailingSEIMessages (SEIMessages& seiMessages, AccessUnit &accessUnit, Int temporalId, const TComVPS *vps, const TComSPS *sps, const SEIScalableNesting* nestingSei=NULL, const SEIBspNesting* bspNestingSei=NULL);
  Void xWriteDuSEIMessages       (SEIMessages& duInfoSeiMessages, AccessUnit &accessUnit, Int temporalId, const TComVPS *sps, const TComSPS *vps, std::deque<DUData> &duData, const SEIScalableNesting* nestingSei=NULL, const SEIBspNesting* bspNestingSei=NULL);
#else
  Void xWriteLeadingSEIOrdered (SEIMessages& seiMessages, SEIMessages& duInfoSeiMessages, AccessUnit &accessUnit, Int temporalId, const TComSPS *sps, Bool testWrite);
  Void xWriteLeadingSEIMessages  (SEIMessages& seiMessages, SEIMessages& duInfoSeiMessages, AccessUnit &accessUnit, Int temporalId, const TComSPS *sps, std::deque<DUData> &duData);
  Void xWriteTrailingSEIMessages (SEIMessages& seiMessages, AccessUnit &accessUnit, Int temporalId, const TComSPS *sps);
  Void xWriteDuSEIMessages       (SEIMessages& duInfoSeiMessages, AccessUnit &accessUnit, Int temporalId, const TComSPS *sps, std::deque<DUData> &duData);
#endif

  Int xWriteVPS (AccessUnit &accessUnit, const TComVPS *vps);
  Int xWriteSPS (AccessUnit &accessUnit, const TComSPS *sps);
  Int xWritePPS (AccessUnit &accessUnit, const TComPPS *pps);
  Int xWriteParameterSets (AccessUnit &accessUnit, TComSlice *slice);

  Void applyDeblockingFilterMetric( TComPic* pcPic, UInt uiNumSlices );
#if W0038_DB_OPT
  Void applyDeblockingFilterParameterSelection( TComPic* pcPic, const UInt numSlices, const Int gopID );
#endif

#if SVC_EXTENSION
#if N0383_IL_CONSTRAINED_TILE_SETS_SEI
  Void xBuildTileSetsMap(TComPicSym* picSym);
#endif
#if CGS_3D_ASYMLUT
  Void xDetermine3DAsymLUT( TComSlice * pSlice , TComPic * pCurPic , UInt refLayerIdc , TEncCfg * pCfg , Bool bSignalPPS );
  Void xDownScalePic( TComPicYuv* pcYuvSrc, TComPicYuv* pcYuvDest, const BitDepths& bitDepth, const Int posScalingFactorX);
  Void xDownScaleComponent2x2( const Pel* pSrc, Pel* pDest, const Int iSrcStride, const Int iDestStride, const Int iSrcWidth, const Int iSrcHeight, const Int inputBitDepth, const Int outputBitDepth );
  inline Short xClip( Short x, Int bitdepth );
  Void xInitDs( const Int iWidth, const Int iHeight, const Bool allIntra, const Int posScalingFactorX);
  Void xFilterImg( Pel *src, Int iSrcStride, Pel *dst, Int iDstStride, Int height, Int width, const BitDepths& bitDepth, ComponentID comp );

  Int  xCreate2DArray(Int ***array2D, Int dim0, Int dim1, Int iPadY, Int iPadX);
  Void xDestroy2DArray(Int **array2D, Int iPadY, Int iPadX);
#endif
  Void xCheckLayerReset(TComSlice *slice);
  Void xSetNoRaslOutputFlag(TComSlice *slice);
  Void xSetLayerInitializedFlag(TComSlice *slice);
#endif //SVC_EXTENSION
};// END CLASS DEFINITION TEncGOP

//! \}

#endif // __TENCGOP__

