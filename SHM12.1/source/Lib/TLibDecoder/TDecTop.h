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

/** \file     TDecTop.h
    \brief    decoder class (header)
*/

#ifndef __TDECTOP__
#define __TDECTOP__

#include "TLibCommon/CommonDef.h"
#include "TLibCommon/TComList.h"
#include "TLibCommon/TComPicYuv.h"
#include "TLibCommon/TComPic.h"
#include "TLibCommon/TComTrQuant.h"
#include "TLibCommon/TComPrediction.h"
#include "TLibCommon/SEI.h"
#if CGS_3D_ASYMLUT
#include "TLibCommon/TCom3DAsymLUT.h"
#endif

#include "TDecGop.h"
#include "TDecEntropy.h"
#include "TDecSbac.h"
#include "TDecCAVLC.h"
#include "SEIread.h"

class InputNALUnit;

//! \ingroup TLibDecoder
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// decoder class
class TDecTop
{
private:
  Int                     m_iMaxRefPicNum;

  NalUnitType             m_associatedIRAPType; ///< NAL unit type of the associated IRAP picture
  Int                     m_pocCRA;            ///< POC number of the latest CRA picture
  Int                     m_pocRandomAccess;   ///< POC number of the random access point (the first IDR or CRA picture)

  TComList<TComPic*>      m_cListPic;         //  Dynamic buffer
  ParameterSetManager     m_parameterSetManager;  // storage for parameter sets
  TComSlice*              m_apcSlicePilot;

  SEIMessages             m_SEIs; ///< List of SEI messages that have been received before the first slice and between slices, excluding prefix SEIs...

  // functional classes
  TComPrediction          m_cPrediction;
#if CGS_3D_ASYMLUT
  TCom3DAsymLUT           m_c3DAsymLUTPPS;
  TComPicYuv*             m_pColorMappedPic;
#endif
  TComTrQuant             m_cTrQuant;
  TDecGop                 m_cGopDecoder;
  TDecSlice               m_cSliceDecoder;
  TDecCu                  m_cCuDecoder;
  TDecEntropy             m_cEntropyDecoder;
  TDecCavlc               m_cCavlcDecoder;
  TDecSbac                m_cSbacDecoder;
  TDecBinCABAC            m_cBinCABAC;
  SEIReader               m_seiReader;
  TComLoopFilter          m_cLoopFilter;
  TComSampleAdaptiveOffset m_cSAO;

  Bool isSkipPictureForBLA(Int& iPOCLastDisplay);
  Bool isRandomAccessSkipPicture(Int& iSkipFrame,  Int& iPOCLastDisplay);
  TComPic*                m_pcPic;
  UInt                    m_uiSliceIdx;
#if !SVC_EXTENSION
  Int                     m_prevPOC;
#endif
  Int                     m_prevTid0POC;
  Bool                    m_bFirstSliceInPicture;
#if !SVC_EXTENSION
  Bool                    m_bFirstSliceInSequence;
#endif
  Bool                    m_prevSliceSkipped;
  Int                     m_skippedPOC;
  Bool                    m_bFirstSliceInBitstream;
  Int                     m_lastPOCNoOutputPriorPics;
  Bool                    m_isNoOutputPriorPics;
  Bool                    m_craNoRaslOutputFlag;    //value of variable NoRaslOutputFlag of the last CRA pic
#if O0043_BEST_EFFORT_DECODING
  UInt                    m_forceDecodeBitDepth;
#endif
  std::ostream           *m_pDecodedSEIOutputStream;

  Bool                    m_warningMessageSkipPicture;

  std::list<InputNALUnit*> m_prefixSEINALUs; /// Buffered up prefix SEI NAL Units.

#if SVC_EXTENSION
  Bool                    m_isLastNALWasEos;
  Bool                    m_lastPicHasEos;
  static UInt             m_prevPOC;        // POC of the previous slice
  static UInt             m_uiPrevLayerId;  // LayerId of the previous slice
  static Bool             m_bFirstSliceInSequence;
  UInt                    m_layerId;      
  TDecTop**               m_ppcTDecTop;
  UInt                    m_smallestLayerId;
  Bool                    m_pocResettingFlag;
  Bool                    m_pocDecrementedInDPBFlag;
#if AVC_BASE
  fstream*                m_pBLReconFile;
  TComPic*                m_blPic;
#endif

  Int                     m_numDirectRefLayers;
  Int                     m_refLayerId[MAX_VPS_LAYER_IDX_PLUS1];
  Int                     m_numSamplePredRefLayers;
  Int                     m_numMotionPredRefLayers;

  TComPic*                m_cIlpPic[MAX_NUM_REF];                    ///<  Inter layer Prediction picture =  upsampled picture
  CommonDecoderParams*    m_commonDecoderParams;
#if NO_CLRAS_OUTPUT_FLAG  
  Bool                    m_noClrasOutputFlag;
  Bool                    m_layerInitializedFlag;
  Bool                    m_firstPicInLayerDecodedFlag;
#endif
  Int                     m_parseIdc;
  Int                     m_lastPocPeriodId;
  Int                     m_prevPicOrderCnt;
#if CONFORMANCE_BITSTREAM_MODE
  Bool                    m_confModeFlag;
  std::vector<TComPic>    m_confListPic;         //  Dynamic buffer for storing pictures for conformance purposes
#endif
  Bool                    m_isOutputLayerFlag;
  static std::vector<UInt> m_targetDecLayerIdList; // list of layers to be decoded according to the OLS index
#endif //SVC_EXTENSION

public:
#if SVC_EXTENSION
  static Bool             m_checkPocRestrictionsForCurrAu;
  static Int              m_pocResetIdcOrCurrAu;
  static Bool             m_baseLayerIdrFlag;
  static Bool             m_baseLayerPicPresentFlag;
  static Bool             m_baseLayerIrapFlag;
  static Bool             m_nonBaseIdrPresentFlag;
  static Int              m_nonBaseIdrType;
  static Bool             m_picNonIdrWithRadlPresentFlag;
  static Bool             m_picNonIdrNoLpPresentFlag;
  static Int              m_crossLayerPocResetPeriodId;
  static Int              m_crossLayerPocResetIdc;
#endif //SVC_EXTENSION

  TDecTop();
  virtual ~TDecTop();

  Void  create  ();
  Void  destroy ();

  Void setDecodedPictureHashSEIEnabled(Int enabled) { m_cGopDecoder.setDecodedPictureHashSEIEnabled(enabled); }

  Void  init();
#if SVC_EXTENSION
  Bool  decode(InputNALUnit& nalu, Int& iSkipFrame, Int& iPOCLastDisplay, UInt& curLayerId, Bool& bNewPOC);
#else
  Bool  decode(InputNALUnit& nalu, Int& iSkipFrame, Int& iPOCLastDisplay);
#endif
  
  Void  deletePicBuffer();

  
  Void  executeLoopFilters(Int& poc, TComList<TComPic*>*& rpcListPic);
  Void  checkNoOutputPriorPics (TComList<TComPic*>* rpcListPic);

  Bool  getNoOutputPriorPicsFlag () { return m_isNoOutputPriorPics; }
  Void  setNoOutputPriorPicsFlag (Bool val) { m_isNoOutputPriorPics = val; }
  Void  setFirstSliceInPicture (bool val)  { m_bFirstSliceInPicture = val; }
  Bool  getFirstSliceInSequence ()         { return m_bFirstSliceInSequence; }
  Void  setFirstSliceInSequence (bool val) { m_bFirstSliceInSequence = val; }
#if O0043_BEST_EFFORT_DECODING
  Void  setForceDecodeBitDepth(UInt bitDepth) { m_forceDecodeBitDepth = bitDepth; }
#endif
  Void  setDecodedSEIMessageOutputStream(std::ostream *pOpStream) { m_pDecodedSEIOutputStream = pOpStream; }
  UInt  getNumberOfChecksumErrorsDetected() const { return m_cGopDecoder.getNumberOfChecksumErrorsDetected(); }

#if SVC_EXTENSION
  Int       getParseIdc                     ()                              { return m_parseIdc;               }
  Void      setParseIdc                     (Int x)                         { m_parseIdc = x;                  }
  Void      markAllPicsAsNoCurrAu           ( const TComVPS *vps );

  Int       getLastPocPeriodId              ()                              { return m_lastPocPeriodId;        }
  Void      setLastPocPeriodId              (Int x)                         { m_lastPocPeriodId = x;           }

  Int       getPrevPicOrderCnt              ()                              { return m_prevPicOrderCnt;        }
  Void      setPrevPicOrderCnt              (Int const x)                   { m_prevPicOrderCnt = x;           }

  UInt      getLayerId                      ()                              { return m_layerId;                }
  Void      setLayerId                      (UInt layer)                    { m_layerId = layer;               }
  TComList<TComPic*>*  getListPic           ()                              { return &m_cListPic;              }
  Void      setLayerDec                     (TDecTop **p)                   { m_ppcTDecTop = p;                }
  TDecTop*  getLayerDec                     (UInt layerId)                  { return m_ppcTDecTop[layerId];    }
  Void      xDeriveSmallestLayerId(TComVPS* vps);

  TDecTop*  getRefLayerDec                  (UInt refLayerIdx);
  Int       getNumDirectRefLayers           ()                              { return m_numDirectRefLayers;      }
  Void      setNumDirectRefLayers           (Int num)                       { m_numDirectRefLayers = num;       }

  Int       getRefLayerId                   (Int i)                         { return m_refLayerId[i];           }
  Void      setRefLayerId                   (Int i, Int refLayerId)         { m_refLayerId[i] = refLayerId;     }

  Int       getNumSamplePredRefLayers       ()                              { return m_numSamplePredRefLayers;  }
  Void      setNumSamplePredRefLayers       (Int num)                       { m_numSamplePredRefLayers = num;   }

  Int       getNumMotionPredRefLayers       ()                              { return m_numMotionPredRefLayers;  }
  Void      setNumMotionPredRefLayers       (Int num)                       { m_numMotionPredRefLayers = num;   }

  Void      setRefLayerParams( const TComVPS* vps );

#if AVC_BASE
  Void      setBLReconFile( fstream* pFile )                                { m_pBLReconFile = pFile; }
  fstream*  getBLReconFile()                                                { return m_pBLReconFile;  }
  Void      setBlPic( TComPic* pic )                                        { m_blPic = pic;          }
  TComPic*  getBlPic()                                                      { return m_blPic;         }
#endif
  Void      xInitILRP(const TComSPS *sps, const TComPPS *pps);
  CommonDecoderParams*    getCommonDecoderParams()                          { return m_commonDecoderParams; }
  Void                    setCommonDecoderParams(CommonDecoderParams* x)    { m_commonDecoderParams = x;    }
  Void      checkValueOfTargetOutputLayerSetIdx(TComVPS *vps);

  ParameterSetManager* getParameterSetManager()                             { return &m_parameterSetManager; }

#if CONFORMANCE_BITSTREAM_MODE
  std::vector<TComPic>* getConfListPic()                                    { return &m_confListPic; }
  Bool      getConfModeFlag() const                                         { return m_confModeFlag; }
  Void      setConfModeFlag(Bool x)                                         { m_confModeFlag = x;    }
#endif
#endif //SVC_EXTENSION

protected:
#if SVC_EXTENSION
  Void  xGetNewPicBuffer  ( const TComVPS &vps, const TComSPS &sps, const TComPPS &pps, TComPic*& rpcPic, const UInt temporalLayer);
#else
  Void  xGetNewPicBuffer  (const TComSPS &sps, const TComPPS &pps, TComPic*& rpcPic, const UInt temporalLayer);
#endif
  Void  xCreateLostPicture (Int iLostPOC);

  Void      xActivateParameterSets();
#if SVC_EXTENSION
  Bool      xDecodeSlice(InputNALUnit &nalu, Int &iSkipFrame, Int iPOCLastDisplay, UInt& curLayerId, Bool& bNewPOC);
  Void      xSetRequireResamplingFlag( const TComVPS &vps, const TComSPS &sps, const TComPPS &pps, TComPic* pic );
#else
  Bool      xDecodeSlice(InputNALUnit &nalu, Int &iSkipFrame, Int iPOCLastDisplay);
#endif
  Void      xDecodeVPS(const std::vector<UChar> &naluData);
  Void      xDecodeSPS(const std::vector<UChar> &naluData);  
#if CGS_3D_ASYMLUT
  Void      xDecodePPS(const std::vector<UChar> &naluData, TCom3DAsymLUT *pc3DAsymLUT);
#else
  Void      xDecodePPS(const std::vector<UChar> &naluData);
#endif
  Void      xDecodeSEI( TComInputBitstream* bs, const NalUnitType nalUnitType );
  Void      xUpdatePreviousTid0POC( TComSlice *pSlice ) { if ((pSlice->getTLayer()==0) && (pSlice->isReferenceNalu() && (pSlice->getNalUnitType()!=NAL_UNIT_CODED_SLICE_RASL_R)&& (pSlice->getNalUnitType()!=NAL_UNIT_CODED_SLICE_RADL_R))) { m_prevTid0POC=pSlice->getPOC(); } }
  Void      xParsePrefixSEImessages();
  Void      xParsePrefixSEIsForUnknownVCLNal();


#if SVC_EXTENSION
#if NO_CLRAS_OUTPUT_FLAG
  Int       getNoClrasOutputFlag()                { return m_noClrasOutputFlag;}
  Void      setNoClrasOutputFlag(Bool x)          { m_noClrasOutputFlag = x;   }
  Int       getLayerInitializedFlag()             { return m_layerInitializedFlag;}
  Void      setLayerInitializedFlag(Bool x)       { m_layerInitializedFlag = x;   }
  Int       getFirstPicInLayerDecodedFlag()       { return m_firstPicInLayerDecodedFlag;}
  Void      setFirstPicInLayerDecodedFlag(Bool x) { m_firstPicInLayerDecodedFlag = x;   }
#endif
#if CGS_3D_ASYMLUT
  Void     initAsymLut(TComSlice *pcSlice);
#endif
  Void     resetPocRestrictionCheckParameters();
  Void     xCheckLayerReset();
  Void     xSetNoRaslOutputFlag();
  Void     xSetLayerInitializedFlag();
#endif
};// END CLASS DEFINITION TDecTop


//! \}

#endif // __TDECTOP__

