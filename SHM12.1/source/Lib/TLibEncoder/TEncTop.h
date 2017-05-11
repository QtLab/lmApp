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

/** \file     TEncTop.h
    \brief    encoder class (header)
*/

#ifndef __TENCTOP__
#define __TENCTOP__

// Include files
#include "TLibCommon/TComList.h"
#include "TLibCommon/TComPrediction.h"
#include "TLibCommon/TComTrQuant.h"
#include "TLibCommon/TComLoopFilter.h"
#include "TLibCommon/AccessUnit.h"

#include "TLibVideoIO/TVideoIOYuv.h"

#include "TEncCfg.h"
#include "TEncGOP.h"
#include "TEncSlice.h"
#include "TEncEntropy.h"
#include "TEncCavlc.h"
#include "TEncSbac.h"
#include "TEncSearch.h"
#include "TEncSampleAdaptiveOffset.h"
#include "TEncPreanalyzer.h"
#include "TEncRateCtrl.h"
//! \ingroup TLibEncoder
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// encoder class
class TEncTop : public TEncCfg
{
private:
  // picture
  Int                     m_iPOCLast;                     ///< time index (POC)
  Int                     m_iNumPicRcvd;                  ///< number of received pictures
  UInt                    m_uiNumAllPicCoded;             ///< number of coded pictures
  TComList<TComPic*>      m_cListPic;                     ///< dynamic list of pictures

  // encoder search
  TEncSearch              m_cSearch;                      ///< encoder search class
  //TEncEntropy*            m_pcEntropyCoder;                     ///< entropy encoder
  TEncCavlc*              m_pcCavlcCoder;                       ///< CAVLC encoder
  // coding tool
  TComTrQuant             m_cTrQuant;                     ///< transform & quantization class
  TComLoopFilter          m_cLoopFilter;                  ///< deblocking filter class
  TEncSampleAdaptiveOffset m_cEncSAO;                     ///< sample adaptive offset class
  TEncEntropy             m_cEntropyCoder;                ///< entropy encoder
  TEncCavlc               m_cCavlcCoder;                  ///< CAVLC encoder
  TEncSbac                m_cSbacCoder;                   ///< SBAC encoder
  TEncBinCABAC            m_cBinCoderCABAC;               ///< bin coder CABAC

  // processing unit
  TEncGOP                 m_cGOPEncoder;                  ///< GOP encoder
  TEncSlice               m_cSliceEncoder;                ///< slice encoder
  TEncCu                  m_cCuEncoder;                   ///< CU encoder
  // SPS
  TComSPS                 m_cSPS;                         ///< SPS. This is the base value. This is copied to TComPicSym
  TComPPS                 m_cPPS;                         ///< PPS. This is the base value. This is copied to TComPicSym
  // RD cost computation
  TComRdCost              m_cRdCost;                      ///< RD cost computation class
  TEncSbac***             m_pppcRDSbacCoder;              ///< temporal storage for RD computation
  TEncSbac                m_cRDGoOnSbacCoder;             ///< going on SBAC model for RD stage
#if FAST_BIT_EST
  TEncBinCABACCounter***  m_pppcBinCoderCABAC;            ///< temporal CABAC state storage for RD computation
  TEncBinCABACCounter     m_cRDGoOnBinCoderCABAC;         ///< going on bin coder CABAC for RD stage
#else
  TEncBinCABAC***         m_pppcBinCoderCABAC;            ///< temporal CABAC state storage for RD computation
  TEncBinCABAC            m_cRDGoOnBinCoderCABAC;         ///< going on bin coder CABAC for RD stage
#endif

  // quality control
  TEncPreanalyzer         m_cPreanalyzer;                 ///< image characteristics analyzer for TM5-step3-like adaptive QP

  TEncRateCtrl            m_cRateCtrl;                    ///< Rate control class
  
#if SVC_EXTENSION
  static Int              m_iSPSIdCnt;                    ///< next Id number for SPS    
  static Int              m_iPPSIdCnt;                    ///< next Id number for PPS    
  TEncTop**               m_ppcTEncTop;
  TEncTop*                getLayerEnc(UInt layerIdx)      { return m_ppcTEncTop[layerIdx]; }
  TComPic*                m_cIlpPic[MAX_NUM_REF];                    ///<  Inter layer Prediction picture =  upsampled picture 
  Bool                    m_bMFMEnabledFlag;
  UInt                    m_numRefLayerLocationOffsets;
  UInt                    m_refLocationOffsetLayerId[MAX_LAYERS];
  Window                  m_scaledRefLayerWindow[MAX_LAYERS];
  UInt                    m_numRefLayerOffsets;
  UInt                    m_refLayerId[MAX_LAYERS];
  Window                  m_refLayerWindow[MAX_LAYERS];
  Bool                    m_scaledRefLayerOffsetPresentFlag[MAX_LAYERS];
  Bool                    m_refRegionOffsetPresentFlag[MAX_LAYERS];
  Int                     m_phaseHorLuma  [MAX_LAYERS];
  Int                     m_phaseVerLuma  [MAX_LAYERS];
  Int                     m_phaseHorChroma[MAX_LAYERS];
  Int                     m_phaseVerChroma[MAX_LAYERS];
  Int                     m_resamplePhaseSetPresentFlag[MAX_LAYERS];
  Int                     m_pocAdjustmentValue;
#if NO_CLRAS_OUTPUT_FLAG
  Bool                    m_noClrasOutputFlag;
  Bool                    m_layerInitializedFlag;
  Bool                    m_firstPicInLayerDecodedFlag;
  Bool                    m_noOutputOfPriorPicsFlags;
#endif
  Bool                    m_interLayerWeightedPredFlag;
  Int                     m_numAddLayerSets;
  Bool                    m_pocDecrementedInDPBFlag;
  Int                     m_currPocMsb;
#endif //SVC_EXTENSION
protected:
  Void  xGetNewPicBuffer  ( TComPic*& rpcPic );           ///< get picture buffer which will be processed
  Void  xInitVPS          ();                             ///< initialize VPS from encoder options
  Void  xInitSPS          ();                             ///< initialize SPS from encoder options
  Void  xInitPPS          ();                             ///< initialize PPS from encoder options
  Void  xInitScalingLists ();                             ///< initialize scaling lists
  Void  xInitHrdParameters();                             ///< initialize HRD parameters

  Void  xInitPPSforTiles  ();
  Void  xInitRPS          (Bool isFieldCoding);           ///< initialize PPS from encoder options
#if SVC_EXTENSION
  Void  xInitILRP();
#endif
public:
  TEncTop();
  virtual ~TEncTop();

  Void      create          ();
  Void      destroy         ();
  Void      init            (Bool isFieldCoding);
  Void      deletePicBuffer ();

  // -------------------------------------------------------------------------------------------------------------------
  // member access functions
  // -------------------------------------------------------------------------------------------------------------------

  TComList<TComPic*>*     getListPic            () { return  &m_cListPic;             }
  TEncSearch*             getPredSearch         () { return  &m_cSearch;              }

  TComTrQuant*            getTrQuant            () { return  &m_cTrQuant;             }
  TComLoopFilter*         getLoopFilter         () { return  &m_cLoopFilter;          }
  TEncSampleAdaptiveOffset* getSAO              () { return  &m_cEncSAO;              }
  TEncGOP*                getGOPEncoder         () { return  &m_cGOPEncoder;          }
  TEncSlice*              getSliceEncoder       () { return  &m_cSliceEncoder;        }
  TEncCu*                 getCuEncoder          () { return  &m_cCuEncoder;           }
  TEncEntropy*            getEntropyCoder       () { return  &m_cEntropyCoder;        }
  TEncCavlc*              getCavlcCoder         () { return  &m_cCavlcCoder;          }
  TEncSbac*               getSbacCoder          () { return  &m_cSbacCoder;           }
  TEncBinCABAC*           getBinCABAC           () { return  &m_cBinCoderCABAC;       }

  TComRdCost*             getRdCost             () { return  &m_cRdCost;              }
  TEncSbac***             getRDSbacCoder        () { return  m_pppcRDSbacCoder;       }
  TEncSbac*               getRDGoOnSbacCoder    () { return  &m_cRDGoOnSbacCoder;     }
  TEncRateCtrl*           getRateCtrl           () { return &m_cRateCtrl;             }
  Void selectReferencePictureSet(TComSlice* slice, Int POCCurr, Int GOPid );
  Int getReferencePictureSetIdxForSOP(Int POCCurr, Int GOPid );
  // -------------------------------------------------------------------------------------------------------------------
  // encoder function
  // -------------------------------------------------------------------------------------------------------------------

  /// encode several number of pictures until end-of-sequence
#if SVC_EXTENSION
  TComSPS*  getSPS()                                          { return &m_cSPS;                    }
#if CGS_3D_ASYMLUT
  TComPPS*  getPPS()                                          { return &m_cPPS;                    }
#endif
  Void      setLayerEnc(TEncTop** p)                          { m_ppcTEncTop = p;                  }
  TEncTop** getLayerEnc()                                     { return m_ppcTEncTop;               }
  Int       getPOCLast()                                      { return m_iPOCLast;                 }
  Int       getNumPicRcvd()                                   { return m_iNumPicRcvd;              }
  Void      setNumPicRcvd( Int num )                          { m_iNumPicRcvd = num;               }
  Void      setNumRefLayerLocationOffsets(Int x)              { m_numRefLayerLocationOffsets = x;     }
  UInt      getNumRefLayerLocationOffsets()                   { return m_numRefLayerLocationOffsets;  }
  Void      setRefLocationOffsetLayerId(Int x, UInt id)       { m_refLocationOffsetLayerId[x] = id;   }
  UInt      getRefLocationOffsetLayerId(Int x)                { return m_refLocationOffsetLayerId[x]; }
  Window    getScaledRefLayerWindowForLayer(Int layerId);
  Window&   getScaledRefLayerWindow(Int x)                    { return m_scaledRefLayerWindow[x];  }
  Void      setNumRefLayerOffsets(Int x)                      { m_numRefLayerOffsets = x;          }
  UInt      getNumRefLayerOffsets()                           { return m_numRefLayerOffsets;       }
  Void      setRefLayerId(Int layerIdx, UInt layerId)         { m_refLayerId[layerIdx] = layerId;  }
  UInt      getRefLayerId(Int layerIdx)                       { return m_refLayerId[layerIdx];     }
  Window    getRefLayerWindowForLayer(Int layerId);
  Window&   getRefLayerWindow(Int x)                          { return m_refLayerWindow[x];                  }
  Bool      getScaledRefLayerOffsetPresentFlag(Int x)         { return m_scaledRefLayerOffsetPresentFlag[x]; }
  Void      setScaledRefLayerOffsetPresentFlag(Int x, Bool b) { m_scaledRefLayerOffsetPresentFlag[x] = b;    }
  Bool      getRefRegionOffsetPresentFlag(Int x)              { return m_refRegionOffsetPresentFlag[x];      }
  Void      setRefRegionOffsetPresentFlag(Int x, Bool b)      { m_refRegionOffsetPresentFlag[x] = b;         }
  Int       getPhaseHorLuma(Int x)                            { return m_phaseHorLuma[x];           }
  Int       getPhaseVerLuma(Int x)                            { return m_phaseVerLuma[x];           }
  Int       getPhaseHorChroma(Int x)                          { return m_phaseHorChroma[x];         }
  Int       getPhaseVerChroma(Int x)                          { return m_phaseVerChroma[x];         }
  Void      setPhaseHorLuma(Int x, Int val)                   { m_phaseHorLuma[x] = val;            }
  Void      setPhaseVerLuma(Int x, Int val)                   { m_phaseVerLuma[x] = val;            }
  Void      setPhaseHorChroma(Int x, Int val)                 { m_phaseHorChroma[x] = val;          }
  Void      setPhaseVerChroma(Int x, Int val)                 { m_phaseVerChroma[x] = val;          }
  Bool      getResamplePhaseSetPresentFlag(Int x)             { return m_resamplePhaseSetPresentFlag[x]; }
  Void      setResamplePhaseSetPresentFlag(Int x, Bool b)     { m_resamplePhaseSetPresentFlag[x] = b;    }

  TComPic** getIlpList()                                      { return m_cIlpPic;                    }
  Void      setMFMEnabledFlag(Bool flag)                      { m_bMFMEnabledFlag = flag;            }
  Bool      getMFMEnabledFlag()                               { return m_bMFMEnabledFlag;            }    
  Void      setInterLayerWeightedPredFlag(Bool flag)          { m_interLayerWeightedPredFlag = flag; }
  Bool      getInterLayerWeightedPredFlag()                   { return m_interLayerWeightedPredFlag; }

  Void      encode    ( TComPicYuv* pcPicYuvOrg, const InputColourSpaceConversion snrCSC, TComList<TComPicYuv*>& rcListPicYuvRecOut, std::list<AccessUnit>& accessUnitsOut, Int iPicIdInGOP );
  Void      encodePrep( TComPicYuv* pcPicYuvOrg, TComPicYuv* pcPicYuvTrueOrg );
  Void      encode    ( TComPicYuv* pcPicYuvOrg, const InputColourSpaceConversion snrCSC, TComList<TComPicYuv*>& rcListPicYuvRecOut, std::list<AccessUnit>& accessUnitsOut, Int iPicIdInGOP, Bool isTff );
  Void      encodePrep( TComPicYuv* pcPicYuvOrg, TComPicYuv* pcPicYuvTrueOrg, Bool isTff );

  TEncTop*  getRefLayerEnc(UInt refLayerIdx);

  Int       getPocAdjustmentValue()                           { return m_pocAdjustmentValue;}
  Void      setPocAdjustmentValue(Int x)                      { m_pocAdjustmentValue = x;   }
#if NO_CLRAS_OUTPUT_FLAG
  Int       getNoClrasOutputFlag()                            { return m_noClrasOutputFlag;}
  Void      setNoClrasOutputFlag(Bool x)                      { m_noClrasOutputFlag = x;   }
  Int       getLayerInitializedFlag()                         { return m_layerInitializedFlag;}
  Void      setLayerInitializedFlag(Bool x)                   { m_layerInitializedFlag = x;   }
  Int       getFirstPicInLayerDecodedFlag()                   { return m_firstPicInLayerDecodedFlag;}
  Void      setFirstPicInLayerDecodedFlag(Bool x)             { m_firstPicInLayerDecodedFlag = x;   }
  Int       getNoOutputOfPriorPicsFlags()                     { return m_noOutputOfPriorPicsFlags;}
  Void      setNoOutputOfPriorPicsFlags(Bool x)               { m_noOutputOfPriorPicsFlags = x;   }
#endif
  Void      setNumAddLayerSets(Int x)                         { m_numAddLayerSets = x;    }
  Int       getNumAddLayerSets()                              { return m_numAddLayerSets; }
  Void      setPocDecrementedInDPBFlag(Bool x)                { m_pocDecrementedInDPBFlag = x;    }
  Bool      getPocDecrementedInDPBFlag()                      { return m_pocDecrementedInDPBFlag; }
  Void      setCurrPocMsb(Int poc)                            { m_currPocMsb = poc;  }
  Int       getCurrPocMsb()                                   { return m_currPocMsb; }

  TEncAnalyze* getAnalyzeAll()                                { return m_cGOPEncoder.getAnalyzeAll();   }
  TEncAnalyze* getAnalyzeI()                                  { return m_cGOPEncoder.getAnalyzeI();     }
  TEncAnalyze* getAnalyzeP()                                  { return m_cGOPEncoder.getAnalyzeP();     }
  TEncAnalyze* getAnalyzeB()                                  { return m_cGOPEncoder.getAnalyzeB();     }
  TEncAnalyze* getAnalyzeAllin()                              { return m_cGOPEncoder.getAnalyzeAllin(); }
  Double       calculateRVM()                                 { return m_cGOPEncoder.calculateRVM();    }
#else //SVC_EXTENSION
  Void encode( Bool bEos,
               TComPicYuv* pcPicYuvOrg,
               TComPicYuv* pcPicYuvTrueOrg, const InputColourSpaceConversion snrCSC, // used for SNR calculations. Picture in original colour space.
               TComList<TComPicYuv*>& rcListPicYuvRecOut,
               std::list<AccessUnit>& accessUnitsOut, Int& iNumEncoded );

  /// encode several number of pictures until end-of-sequence
  Void encode( Bool bEos, TComPicYuv* pcPicYuvOrg,
               TComPicYuv* pcPicYuvTrueOrg, const InputColourSpaceConversion snrCSC, // used for SNR calculations. Picture in original colour space.
               TComList<TComPicYuv*>& rcListPicYuvRecOut,
               std::list<AccessUnit>& accessUnitsOut, Int& iNumEncoded, Bool isTff);
#endif //#if SVC_EXTENSION

  Void printSummary(Bool isField) { m_cGOPEncoder.printOutSummary (m_uiNumAllPicCoded, isField, m_printMSEBasedSequencePSNR, m_printSequenceMSE, m_cSPS.getBitDepths()); }

};

//! \}

#endif // __TENCTOP__

