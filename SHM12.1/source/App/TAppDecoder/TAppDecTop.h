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

/** \file     TAppDecTop.h
    \brief    Decoder application class (header)
*/

#ifndef __TAPPDECTOP__
#define __TAPPDECTOP__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TLibVideoIO/TVideoIOYuv.h"
#include "TLibCommon/TComList.h"
#include "TLibCommon/TComPicYuv.h"
#include "TLibDecoder/TDecTop.h"
#include "TAppDecCfg.h"

//! \ingroup TAppDecoder
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================
#if ALIGNED_BUMPING
struct DpbStatus;
#endif
/// decoder application class
class TAppDecTop : public TAppDecCfg
{
private:
  // class interface
#if SVC_EXTENSION
  TDecTop*                        m_apcTDecTop[MAX_NUM_LAYER_IDS];                     ///< decoder point class 
  TVideoIOYuv*                    m_apcTVideoIOYuvReconFile[MAX_NUM_LAYER_IDS];        ///< reconstruction YUV class
#if CONFORMANCE_BITSTREAM_MODE
  TVideoIOYuv                     m_confReconFile[63];        ///< decode YUV files
#endif 
  // for output control
  Int                             m_aiPOCLastDisplay [MAX_LAYERS]; ///< last POC in display order
#else
  TDecTop                         m_cTDecTop;                     ///< decoder class
  TVideoIOYuv                     m_cTVideoIOYuvReconFile;        ///< reconstruction YUV class

  // for output control
  Int                             m_iPOCLastDisplay;              ///< last POC in display order
#endif

  std::ofstream                   m_seiMessageFileStream;         ///< Used for outputing SEI messages.

  SEIColourRemappingInfo*         m_pcSeiColourRemappingInfoPrevious;

public:
  TAppDecTop();
  virtual ~TAppDecTop() {}

  Void  create            (); ///< create internal members
  Void  destroy           (); ///< destroy internal members
  Void  decode            (); ///< main decoding function
#if SVC_EXTENSION
  UInt  getNumberOfChecksumErrorsDetected() const
  { 
    UInt sum = 0;

    for( UInt layerId = 0; layerId < MAX_NUM_LAYER_IDS; layerId++ )
    {
      if( m_apcTDecTop[layerId] )
      {
        sum += m_apcTDecTop[layerId]->getNumberOfChecksumErrorsDetected();
      }
    }

    return sum;
  }
#else
  UInt  getNumberOfChecksumErrorsDetected() const { return m_cTDecTop.getNumberOfChecksumErrorsDetected(); }
#endif

protected:
  Void  xCreateDecLib     (); ///< create internal classes
  Void  xDestroyDecLib    (); ///< destroy internal classes
  Void  xInitDecLib       (); ///< initialize decoder class

#if SVC_EXTENSION
  Void  xWriteOutput      ( TComList<TComPic*>* pcListPic, UInt layerId, UInt tId ); ///< write YUV to file
  Void  xFlushOutput      ( TComList<TComPic*>* pcListPic, UInt layerId ); ///< flush all remaining decoded pictures to file
#else
  Void  xWriteOutput      ( TComList<TComPic*>* pcListPic , UInt tId); ///< write YUV to file
  Void  xFlushOutput      ( TComList<TComPic*>* pcListPic ); ///< flush all remaining decoded pictures to file
#endif
  Bool  isNaluWithinTargetDecLayerIdSet ( InputNALUnit* nalu ); ///< check whether given Nalu is within targetDecLayerIdSet

private:
  Void applyColourRemapping(const TComPicYuv& pic, SEIColourRemappingInfo& pCriSEI, const TComSPS &activeSPS);
  Void xOutputColourRemapPic(TComPic* pcPic);

#if ALIGNED_BUMPING
  Void checkOutputBeforeDecoding(Int layerIdx);
  Void checkOutputAfterDecoding();
  Void flushAllPictures(Bool outputPictures); 
  Void flushAllPictures(Int layerId, Bool outputPictures);

  Void xOutputAndMarkPic( TComPic *pic, std::string& reconFileName, const Int layerId, Int &pocLastDisplay, DpbStatus &dpbStatus);
  Void outputAllPictures(Int layerId, Bool notOutputCurrAu);
  Void xFindDPBStatus( std::vector<Int> &listOfPocs
                            , std::vector<Int> *listOfPocsInEachLayer
                            , std::vector<Int> *listOfPocsPositionInEachLayer
                            , DpbStatus &dpbStatus
                            , Bool notOutputCurrAu = true
                            );

  Bool ifInvokeBumpingBeforeDecoding( const DpbStatus &dpbStatus, const DpbStatus &dpbLimit, const Int layerIdx, const Int subDpbIdx );
  Bool ifInvokeBumpingAfterDecoding ( const DpbStatus &dpbStatus, const DpbStatus &dpbLimit );
  Void bumpingProcess(std::vector<Int> &listOfPocs, std::vector<Int> *listOfPocsInEachLayer, std::vector<Int> *listOfPocsPositionInEachLayer, DpbStatus &dpbStatus);
  Void emptyUnusedPicturesNotNeededForOutput();
  Void markAllPicturesAsErased();
  Void markAllPicturesAsErased(Int layerIdx);
  const TComVPS* findDpbParametersFromVps(std::vector<Int> const &listOfPocs, std::vector<Int> const *listOfPocsInEachLayer, std::vector<Int> const *listOfPocsPositionInEachLayer, DpbStatus &maxDpbLimit);
#endif
};

#if ALIGNED_BUMPING
struct DpbStatus
{
  // Number of AUs and pictures
  Int m_numAUsNotDisplayed;
  Int m_numPicsNotDisplayedInLayer[MAX_LAYERS];
  Int m_numPicsInSubDpb[MAX_LAYERS];  // Pictures marked as used_for_reference or needed for output in the sub-DPB
  Int m_layerIdToSubDpbIdMap[MAX_VPS_LAYER_IDX_PLUS1];
  Int m_targetDecLayerIdList[MAX_LAYERS];
  Bool m_maxLatencyIncrease;
  Int m_maxLatencyPictures;
  
  Int m_numSubDpbs;
  Int m_numLayers;

  DpbStatus()
  {
    init();
  }
  Void init()
  {
    m_numAUsNotDisplayed = 0;
    m_maxLatencyIncrease  = false;
    m_maxLatencyPictures  = 0;
    ::memset( m_numPicsInSubDpb, 0, sizeof(m_numPicsInSubDpb) );
    ::memset(m_numPicsNotDisplayedInLayer, 0, sizeof(m_numPicsNotDisplayedInLayer) );
    m_numSubDpbs = -1;
    m_numLayers = -1;
    ::memset( m_targetDecLayerIdList, 0, sizeof(m_targetDecLayerIdList) );

    for( Int i = 0; i < MAX_VPS_LAYER_IDX_PLUS1; i++ )
    {
      m_layerIdToSubDpbIdMap[i] = -1;
    }
  }
};
#endif
//! \}

#endif

