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

#pragma once

#ifndef __SEIWRITE__
#define __SEIWRITE__

#include "SyntaxElementWriter.h"
#include "TLibCommon/SEI.h"

class TComBitIf;

//! \ingroup TLibEncoder
//! \{
class SEIWriter:public SyntaxElementWriter
{
public:
  SEIWriter() {};
  virtual ~SEIWriter() {};

#if O0164_MULTI_LAYER_HRD
  Void writeSEImessages(TComBitIf& bs, const SEIMessages &seiList, const TComVPS *vps, const TComSPS *sps, Bool isNested, const SEIScalableNesting* nestingSei=NULL, const SEIBspNesting* bspNestingSei=NULL);
#else
  Void writeSEImessages(TComBitIf& bs, const SEIMessages &seiList, const TComSPS *sps, Bool isNested);
#endif

protected:
  Void xWriteSEIuserDataUnregistered(const SEIuserDataUnregistered &sei);
  Void xWriteSEIActiveParameterSets(const SEIActiveParameterSets& sei);
#if SVC_EXTENSION
  Void xWriteSEIDecodingUnitInfo(const SEIDecodingUnitInfo& sei, const TComSPS *sps, const SEIScalableNesting* nestingSei, const SEIBspNesting* bspNestingSei, const TComVPS *vps);
  Void xWriteSEIBufferingPeriod(const SEIBufferingPeriod& sei, const TComSPS *sps, const SEIScalableNesting* nestingSei, const SEIBspNesting* bspNestingSei, const TComVPS *vps);
  Void xWriteSEIPictureTiming(const SEIPictureTiming& sei, const TComSPS *sps, const SEIScalableNesting* nestingSei, const SEIBspNesting* bspNestingSei, const TComVPS *vps);
#else
  Void xWriteSEIDecodingUnitInfo(const SEIDecodingUnitInfo& sei, const TComSPS *sps);
#endif
  Void xWriteSEIDecodedPictureHash(const SEIDecodedPictureHash& sei);
#if !SVC_EXTENSION
  Void xWriteSEIBufferingPeriod(const SEIBufferingPeriod& sei, const TComSPS *sps);
  Void xWriteSEIPictureTiming(const SEIPictureTiming& sei, const TComSPS *sps);
#endif  
  Void xWriteSEIRecoveryPoint(const SEIRecoveryPoint& sei);
  Void xWriteSEIFramePacking(const SEIFramePacking& sei);
  Void xWriteSEISegmentedRectFramePacking(const SEISegmentedRectFramePacking& sei);
  Void xWriteSEIDisplayOrientation(const SEIDisplayOrientation &sei);
  Void xWriteSEITemporalLevel0Index(const SEITemporalLevel0Index &sei);
  Void xWriteSEIGradualDecodingRefreshInfo(const SEIGradualDecodingRefreshInfo &sei);
  Void xWriteSEINoDisplay(const SEINoDisplay &sei);
  Void xWriteSEIToneMappingInfo(const SEIToneMappingInfo& sei);
  Void xWriteSEISOPDescription(const SEISOPDescription& sei);
#if O0164_MULTI_LAYER_HRD
  Void xWriteSEIScalableNesting(TComBitIf& bs, const SEIScalableNesting& sei, const TComVPS *vps, const TComSPS *sps);
#else
  Void xWriteSEIScalableNesting(TComBitIf& bs, const SEIScalableNesting& sei, const TComSPS *sps);
#endif
  Void xWriteSEITempMotionConstrainedTileSets(const SEITempMotionConstrainedTileSets& sei);
  Void xWriteSEITimeCode(const SEITimeCode& sei);
  Void xWriteSEIChromaResamplingFilterHint(const SEIChromaResamplingFilterHint& sei);
  Void xWriteSEIKneeFunctionInfo(const SEIKneeFunctionInfo &sei);
  Void xWriteSEIColourRemappingInfo(const SEIColourRemappingInfo& sei);
  Void xWriteSEIMasteringDisplayColourVolume( const SEIMasteringDisplayColourVolume& sei);
#if U0033_ALTERNATIVE_TRANSFER_CHARACTERISTICS_SEI
  Void xWriteSEIAlternativeTransferCharacteristics(const SEIAlternativeTransferCharacteristics& sei);
#endif
  Void xWriteSEIGreenMetadataInfo(const SEIGreenMetadataInfo &sei);

#if O0164_MULTI_LAYER_HRD
  Void xWriteSEIpayloadData(TComBitIf& bs, const SEI& sei, const TComVPS *vps, const TComSPS *sps, const SEIScalableNesting* nestingSei, const SEIBspNesting* bspNestingSei);
#else
  Void xWriteSEIpayloadData(TComBitIf& bs, const SEI& sei, const TComSPS *sps);
#endif
  Void xWriteByteAlign();

#if SVC_EXTENSION
#if LAYERS_NOT_PRESENT_SEI
  Void xWriteSEILayersNotPresent(const SEILayersNotPresent& sei);
#endif
#if N0383_IL_CONSTRAINED_TILE_SETS_SEI
  Void xWriteSEIInterLayerConstrainedTileSets(const SEIInterLayerConstrainedTileSets& sei);
#endif
#if SUB_BITSTREAM_PROPERTY_SEI
  Void xWriteSEISubBitstreamProperty(const SEISubBitstreamProperty &sei);
#endif
#if Q0189_TMVP_CONSTRAINTS 
  Void xWriteSEITMVPConstraints(const SEITMVPConstrains &sei);
#endif
#if Q0247_FRAME_FIELD_INFO
  Void xWriteSEIFrameFieldInfo(const SEIFrameFieldInfo &sei);
#endif
#if O0164_MULTI_LAYER_HRD
  Void xWriteSEIBspNesting(TComBitIf& bs, const SEIBspNesting &sei, const TComVPS *vps, const TComSPS *sps, const SEIScalableNesting &nestingSei);
  Void xWriteSEIBspInitialArrivalTime(const SEIBspInitialArrivalTime &sei, const TComVPS *vps, const TComSPS *sps, const SEIScalableNesting &nestingSei, const SEIBspNesting &bspNestingSei);
  Void xCodeHrdParameters( TComHRD *hrd, Bool commonInfPresentFlag, UInt maxNumSubLayersMinus1 );
#endif
#if P0123_ALPHA_CHANNEL_SEI
  Void xWriteSEIAlphaChannelInfo(const SEIAlphaChannelInfo &sei);
#endif
#if Q0096_OVERLAY_SEI
  Void xWriteSEIOverlayInfo(const SEIOverlayInfo &sei);
#endif
#endif //SVC_EXTENSION
};

//! \}

#endif
