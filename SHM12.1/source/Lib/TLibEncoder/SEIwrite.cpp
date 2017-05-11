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

#include "TLibCommon/TComBitCounter.h"
#include "TLibCommon/TComBitStream.h"
#include "TLibCommon/SEI.h"
#include "TLibCommon/TComSlice.h"
#include "TLibCommon/TComPicYuv.h"
#include "SEIwrite.h"

//! \ingroup TLibEncoder
//! \{

#if ENC_DEC_TRACE
Void  xTraceSEIHeader()
{
  fprintf( g_hTrace, "=========== SEI message ===========\n");
}

Void  xTraceSEIMessageType(SEI::PayloadType payloadType)
{
  fprintf( g_hTrace, "=========== %s SEI message ===========\n", SEI::getSEIMessageString(payloadType));
}
#endif

#if O0164_MULTI_LAYER_HRD
Void SEIWriter::xWriteSEIpayloadData(TComBitIf& bs, const SEI& sei, const TComVPS *vps, const TComSPS *sps, const SEIScalableNesting* nestingSeiPtr, const SEIBspNesting* bspNestingSeiPtr)
#else
Void SEIWriter::xWriteSEIpayloadData(TComBitIf& bs, const SEI& sei, const TComSPS *sps)
#endif
{
#if SVC_EXTENSION
  SEIScalableNesting nestingSei;
  SEIBspNesting      bspNestingSei;
  if( nestingSeiPtr )
  {
    nestingSei = *nestingSeiPtr;
  }
  if( bspNestingSeiPtr )
  {
    bspNestingSei = *bspNestingSeiPtr;
  }
#endif
  switch (sei.payloadType())
  {
  case SEI::USER_DATA_UNREGISTERED:
    xWriteSEIuserDataUnregistered(*static_cast<const SEIuserDataUnregistered*>(&sei));
    break;
  case SEI::ACTIVE_PARAMETER_SETS:
    xWriteSEIActiveParameterSets(*static_cast<const SEIActiveParameterSets*>(& sei));
    break;
#if SVC_EXTENSION
  case SEI::DECODING_UNIT_INFO:
    xWriteSEIDecodingUnitInfo(*static_cast<const SEIDecodingUnitInfo*>(& sei), sps, nestingSeiPtr, bspNestingSeiPtr, vps);
    break;
  case SEI::BUFFERING_PERIOD:
    xWriteSEIBufferingPeriod(*static_cast<const SEIBufferingPeriod*>(&sei), sps, nestingSeiPtr, bspNestingSeiPtr, vps);
    break;
  case SEI::PICTURE_TIMING:
    xWriteSEIPictureTiming(*static_cast<const SEIPictureTiming*>(&sei), sps, nestingSeiPtr, bspNestingSeiPtr, vps);
    break;
#else
  case SEI::DECODING_UNIT_INFO:
    xWriteSEIDecodingUnitInfo(*static_cast<const SEIDecodingUnitInfo*>(& sei), sps);
    break;
#endif
  case SEI::DECODED_PICTURE_HASH:
    xWriteSEIDecodedPictureHash(*static_cast<const SEIDecodedPictureHash*>(&sei));
    break;
#if !SVC_EXTENSION
  case SEI::BUFFERING_PERIOD:
    xWriteSEIBufferingPeriod(*static_cast<const SEIBufferingPeriod*>(&sei), sps);
    break;
  case SEI::PICTURE_TIMING:
    xWriteSEIPictureTiming(*static_cast<const SEIPictureTiming*>(&sei), sps);
    break;
#endif
  case SEI::RECOVERY_POINT:
    xWriteSEIRecoveryPoint(*static_cast<const SEIRecoveryPoint*>(&sei));
    break;
  case SEI::FRAME_PACKING:
    xWriteSEIFramePacking(*static_cast<const SEIFramePacking*>(&sei));
    break;
  case SEI::SEGM_RECT_FRAME_PACKING:
    xWriteSEISegmentedRectFramePacking(*static_cast<const SEISegmentedRectFramePacking*>(&sei));
    break;
  case SEI::DISPLAY_ORIENTATION:
    xWriteSEIDisplayOrientation(*static_cast<const SEIDisplayOrientation*>(&sei));
    break;
  case SEI::TEMPORAL_LEVEL0_INDEX:
    xWriteSEITemporalLevel0Index(*static_cast<const SEITemporalLevel0Index*>(&sei));
    break;
  case SEI::REGION_REFRESH_INFO:
    xWriteSEIGradualDecodingRefreshInfo(*static_cast<const SEIGradualDecodingRefreshInfo*>(&sei));
    break;
  case SEI::NO_DISPLAY:
    xWriteSEINoDisplay(*static_cast<const SEINoDisplay*>(&sei));
    break;
  case SEI::TONE_MAPPING_INFO:
    xWriteSEIToneMappingInfo(*static_cast<const SEIToneMappingInfo*>(&sei));
    break;
  case SEI::SOP_DESCRIPTION:
    xWriteSEISOPDescription(*static_cast<const SEISOPDescription*>(&sei));
    break;
  case SEI::SCALABLE_NESTING:
#if O0164_MULTI_LAYER_HRD
    xWriteSEIScalableNesting(bs, *static_cast<const SEIScalableNesting*>(&sei), vps, sps);
#else
    xWriteSEIScalableNesting(bs, *static_cast<const SEIScalableNesting*>(&sei), sps);
#endif
    break;
  case SEI::CHROMA_RESAMPLING_FILTER_HINT:
    xWriteSEIChromaResamplingFilterHint(*static_cast<const SEIChromaResamplingFilterHint*>(&sei));
    break;
  case SEI::TEMP_MOTION_CONSTRAINED_TILE_SETS:
    xWriteSEITempMotionConstrainedTileSets(*static_cast<const SEITempMotionConstrainedTileSets*>(&sei));
    break;
  case SEI::TIME_CODE:
    xWriteSEITimeCode(*static_cast<const SEITimeCode*>(&sei));
    break;
  case SEI::KNEE_FUNCTION_INFO:
    xWriteSEIKneeFunctionInfo(*static_cast<const SEIKneeFunctionInfo*>(&sei));
    break;
  case SEI::COLOUR_REMAPPING_INFO:
    xWriteSEIColourRemappingInfo(*static_cast<const SEIColourRemappingInfo*>(&sei));
    break;
  case SEI::MASTERING_DISPLAY_COLOUR_VOLUME:
    xWriteSEIMasteringDisplayColourVolume(*static_cast<const SEIMasteringDisplayColourVolume*>(&sei));
    break;
#if U0033_ALTERNATIVE_TRANSFER_CHARACTERISTICS_SEI
  case SEI::ALTERNATIVE_TRANSFER_CHARACTERISTICS:
    xWriteSEIAlternativeTransferCharacteristics(*static_cast<const SEIAlternativeTransferCharacteristics*>(&sei));
    break;
#endif
  case SEI::GREEN_METADATA:
      xWriteSEIGreenMetadataInfo(*static_cast<const SEIGreenMetadataInfo*>(&sei));
    break;

#if SVC_EXTENSION
#if LAYERS_NOT_PRESENT_SEI
  case SEI::LAYERS_NOT_PRESENT:
    xWriteSEILayersNotPresent(*static_cast<const SEILayersNotPresent*>(&sei));
    break;
#endif
#if N0383_IL_CONSTRAINED_TILE_SETS_SEI
  case SEI::INTER_LAYER_CONSTRAINED_TILE_SETS:
    xWriteSEIInterLayerConstrainedTileSets(*static_cast<const SEIInterLayerConstrainedTileSets*>(&sei));
    break;
#endif
#if SUB_BITSTREAM_PROPERTY_SEI
   case SEI::SUB_BITSTREAM_PROPERTY:
     xWriteSEISubBitstreamProperty(*static_cast<const SEISubBitstreamProperty*>(&sei));
     break;
#endif
#if O0164_MULTI_LAYER_HRD
   case SEI::BSP_NESTING:
     xWriteSEIBspNesting(bs, *static_cast<const SEIBspNesting*>(&sei), vps, sps, nestingSei);
     break;
   case SEI::BSP_INITIAL_ARRIVAL_TIME:
     xWriteSEIBspInitialArrivalTime(*static_cast<const SEIBspInitialArrivalTime*>(&sei), vps, sps, nestingSei, bspNestingSei);
     break;
#endif
#if Q0189_TMVP_CONSTRAINTS
   case SEI::TMVP_CONSTRAINTS:
     xWriteSEITMVPConstraints(*static_cast<const SEITMVPConstrains*>(&sei));
     break;
#endif
#if Q0247_FRAME_FIELD_INFO
   case SEI::FRAME_FIELD_INFO:
     xWriteSEIFrameFieldInfo(*static_cast<const SEIFrameFieldInfo*>(&sei));
     break;
#endif
#if P0123_ALPHA_CHANNEL_SEI
   case SEI::ALPHA_CHANNEL_INFO:
     xWriteSEIAlphaChannelInfo(*static_cast<const SEIAlphaChannelInfo*>(&sei));
     break;
#endif
#if Q0096_OVERLAY_SEI
   case SEI::OVERLAY_INFO:
     xWriteSEIOverlayInfo(*static_cast<const SEIOverlayInfo*>(&sei));
     break;
#endif
#endif //SVC_EXTENSION
  default:
    assert(!"Trying to write unhandled SEI message");
    break;
  }
  xWriteByteAlign();
}

/**
 * marshal all SEI messages in provided list into one bitstream bs
 */
#if O0164_MULTI_LAYER_HRD
Void SEIWriter::writeSEImessages(TComBitIf& bs, const SEIMessages &seiList, const TComVPS *vps, const TComSPS *sps, Bool isNested, const SEIScalableNesting* nestingSei, const SEIBspNesting* bspNestingSei)
#else
Void SEIWriter::writeSEImessages(TComBitIf& bs, const SEIMessages &seiList, const TComSPS *sps, Bool isNested)
#endif
{
#if ENC_DEC_TRACE
  if (g_HLSTraceEnable)
    xTraceSEIHeader();
#endif

  TComBitCounter bs_count;

  for (SEIMessages::const_iterator sei=seiList.begin(); sei!=seiList.end(); sei++)
  {
    // calculate how large the payload data is
    // TODO: this would be far nicer if it used vectored buffers
    bs_count.resetBits();
    setBitstream(&bs_count);

#if ENC_DEC_TRACE
    Bool traceEnable = g_HLSTraceEnable;
    g_HLSTraceEnable = false;
#endif
#if O0164_MULTI_LAYER_HRD
    xWriteSEIpayloadData(bs_count, **sei, vps, sps, nestingSei, bspNestingSei);
#else
    xWriteSEIpayloadData(bs_count, **sei, sps);
#endif
#if ENC_DEC_TRACE
    g_HLSTraceEnable = traceEnable;
#endif
    UInt payload_data_num_bits = bs_count.getNumberOfWrittenBits();
    assert(0 == payload_data_num_bits % 8);

    setBitstream(&bs);
    UInt payloadType = (*sei)->payloadType();
    for (; payloadType >= 0xff; payloadType -= 0xff)
    {
      WRITE_CODE(0xff, 8, "payload_type");
    }
    WRITE_CODE(payloadType, 8, "payload_type");

    UInt payloadSize = payload_data_num_bits/8;
    for (; payloadSize >= 0xff; payloadSize -= 0xff)
    {
      WRITE_CODE(0xff, 8, "payload_size");
    }
    WRITE_CODE(payloadSize, 8, "payload_size");

    /* payloadData */
#if ENC_DEC_TRACE
    if (g_HLSTraceEnable)
      xTraceSEIMessageType((*sei)->payloadType());
#endif

#if O0164_MULTI_LAYER_HRD
    xWriteSEIpayloadData(bs, **sei, vps, sps, nestingSei, bspNestingSei);
#else
    xWriteSEIpayloadData(bs, **sei, sps);
#endif
  }
  if (!isNested)
  {
    xWriteRbspTrailingBits();
  }
}

/**
 * marshal a user_data_unregistered SEI message sei, storing the marshalled
 * representation in bitstream bs.
 */
Void SEIWriter::xWriteSEIuserDataUnregistered(const SEIuserDataUnregistered &sei)
{
  for (UInt i = 0; i < ISO_IEC_11578_LEN; i++)
  {
    WRITE_CODE(sei.uuid_iso_iec_11578[i], 8 , "sei.uuid_iso_iec_11578[i]");
  }

  for (UInt i = 0; i < sei.userDataLength; i++)
  {
    WRITE_CODE(sei.userData[i], 8 , "user_data");
  }
}

/**
 * marshal a decoded picture hash SEI message, storing the marshalled
 * representation in bitstream bs.
 */
Void SEIWriter::xWriteSEIDecodedPictureHash(const SEIDecodedPictureHash& sei)
{
  const TChar *traceString="\0";
  switch (sei.method)
  {
    case HASHTYPE_MD5: traceString="picture_md5"; break;
    case HASHTYPE_CRC: traceString="picture_crc"; break;
    case HASHTYPE_CHECKSUM: traceString="picture_checksum"; break;
    default: assert(false); break;
  }

  if (traceString != 0) //use of this variable is needed to avoid a compiler error with G++ 4.6.1
  {
    WRITE_CODE(sei.method, 8, "hash_type");
    for(UInt i=0; i<UInt(sei.m_pictureHash.hash.size()); i++)
    {
      WRITE_CODE(sei.m_pictureHash.hash[i], 8, traceString);
    }
  }
}

Void SEIWriter::xWriteSEIActiveParameterSets(const SEIActiveParameterSets& sei)
{
  WRITE_CODE(sei.activeVPSId,     4,         "active_video_parameter_set_id");
  WRITE_FLAG(sei.m_selfContainedCvsFlag,     "self_contained_cvs_flag");
  WRITE_FLAG(sei.m_noParameterSetUpdateFlag, "no_parameter_set_update_flag");
  WRITE_UVLC(sei.numSpsIdsMinus1,            "num_sps_ids_minus1");

  assert (sei.activeSeqParameterSetId.size() == (sei.numSpsIdsMinus1 + 1));

  for (Int i = 0; i < sei.activeSeqParameterSetId.size(); i++)
  {
    WRITE_UVLC(sei.activeSeqParameterSetId[i], "active_seq_parameter_set_id"); 
  }
#if R0247_SEI_ACTIVE
  for (Int i = 1; i < sei.activeSeqParameterSetId.size(); i++)
  {
    WRITE_UVLC(sei.layerSpsIdx[i], "layer_sps_idx"); 
  }
#endif
}

#if SVC_EXTENSION
Void SEIWriter::xWriteSEIDecodingUnitInfo(const SEIDecodingUnitInfo& sei, const TComSPS *sps, const SEIScalableNesting* nestingSei, const SEIBspNesting* bspNestingSei, const TComVPS *vps)
{
  const TComHRD *hrd;
  if( bspNestingSei )   // If DU info SEI contained inside a BSP nesting SEI message
  {
    assert( nestingSei );
    Int psIdx = bspNestingSei->m_seiPartitioningSchemeIdx;
    Int seiOlsIdx = bspNestingSei->m_seiOlsIdx;
    Int maxTemporalId = nestingSei->m_nestingMaxTemporalIdPlus1[0] - 1;
    Int maxValues = vps->getNumBspSchedulesMinus1(seiOlsIdx, psIdx, maxTemporalId) + 1;
    std::vector<Int> hrdIdx(maxValues, 0);
    std::vector<const TComHRD*> hrdVec;
    std::vector<Int> syntaxElemLen(maxValues, 0);
    for(Int i = 0; i < maxValues; i++)
    {
      hrdIdx[i] = vps->getBspHrdIdx( seiOlsIdx, psIdx, maxTemporalId, i, bspNestingSei->m_bspIdx);
      hrdVec.push_back(vps->getBspHrd(hrdIdx[i]));
    
      syntaxElemLen[i] = hrdVec[i]->getInitialCpbRemovalDelayLengthMinus1() + 1;
      if ( !(hrdVec[i]->getNalHrdParametersPresentFlag() || hrdVec[i]->getVclHrdParametersPresentFlag()) )
      {
        assert( syntaxElemLen[i] == 24 ); // Default of value init_cpb_removal_delay_length_minus1 is 23
      }
      if( i > 0 )
      {
        assert( hrdVec[i]->getSubPicCpbParamsPresentFlag()    == hrdVec[i-1]->getSubPicCpbParamsPresentFlag() );
        assert( hrdVec[i]->getSubPicCpbParamsInPicTimingSEIFlag()   == hrdVec[i-1]->getSubPicCpbParamsInPicTimingSEIFlag() );
        assert( hrdVec[i]->getDpbOutputDelayDuLengthMinus1()  == hrdVec[i-1]->getDpbOutputDelayDuLengthMinus1() );
        // To be done: Check CpbDpbDelaysPresentFlag
      }
    }
    hrd = hrdVec[0];
  }
  else
  {
    const TComVUI *vui = sps->getVuiParameters();
    hrd = vui->getHrdParameters();
  }

  WRITE_UVLC(sei.m_decodingUnitIdx, "decoding_unit_idx");
  if(hrd->getSubPicCpbParamsInPicTimingSEIFlag())
  {
    WRITE_CODE( sei.m_duSptCpbRemovalDelay, (hrd->getDuCpbRemovalDelayLengthMinus1() + 1), "du_spt_cpb_removal_delay");
  }
  WRITE_FLAG( sei.m_dpbOutputDuDelayPresentFlag, "dpb_output_du_delay_present_flag");
  if(sei.m_dpbOutputDuDelayPresentFlag)
  {
    WRITE_CODE(sei.m_picSptDpbOutputDuDelay, hrd->getDpbOutputDelayDuLengthMinus1() + 1, "pic_spt_dpb_output_du_delay");
  }
}
#else
Void SEIWriter::xWriteSEIDecodingUnitInfo(const SEIDecodingUnitInfo& sei, const TComSPS *sps)
{
  const TComVUI *vui = sps->getVuiParameters();
  WRITE_UVLC(sei.m_decodingUnitIdx, "decoding_unit_idx");
  if(vui->getHrdParameters()->getSubPicCpbParamsInPicTimingSEIFlag())
  {
    WRITE_CODE( sei.m_duSptCpbRemovalDelay, (vui->getHrdParameters()->getDuCpbRemovalDelayLengthMinus1() + 1), "du_spt_cpb_removal_delay_increment");
  }
  WRITE_FLAG( sei.m_dpbOutputDuDelayPresentFlag, "dpb_output_du_delay_present_flag");
  if(sei.m_dpbOutputDuDelayPresentFlag)
  {
    WRITE_CODE(sei.m_picSptDpbOutputDuDelay, vui->getHrdParameters()->getDpbOutputDelayDuLengthMinus1() + 1, "pic_spt_dpb_output_du_delay");
  }
}
#endif

#if SVC_EXTENSION
Void SEIWriter::xWriteSEIBufferingPeriod(const SEIBufferingPeriod& sei, const TComSPS *sps, const SEIScalableNesting* nestingSei, const SEIBspNesting* bspNestingSei, const TComVPS *vps)
#else
Void SEIWriter::xWriteSEIBufferingPeriod(const SEIBufferingPeriod& sei, const TComSPS *sps)
#endif
{
  Int i, nalOrVcl;
#if SVC_EXTENSION
  const TComHRD *hrd;
  if( bspNestingSei )   // If BP SEI contained inside a BSP nesting SEI message
  {
    assert( nestingSei );
    Int psIdx = bspNestingSei->m_seiPartitioningSchemeIdx;
    Int seiOlsIdx = bspNestingSei->m_seiOlsIdx;
    Int maxTemporalId = nestingSei->m_nestingMaxTemporalIdPlus1[0] - 1;
    Int maxValues = vps->getNumBspSchedulesMinus1(seiOlsIdx, psIdx, maxTemporalId) + 1;
    std::vector<Int> hrdIdx(maxValues, 0);
    std::vector<const TComHRD*> hrdVec;
    std::vector<Int> syntaxElemLen(maxValues, 0);

    Int schedSelIdx = 0;

    for(i = 0; i < maxValues; i++)
    {
      hrdIdx[i] = vps->getBspHrdIdx( seiOlsIdx, psIdx, maxTemporalId, i, bspNestingSei->m_bspIdx);
      hrdVec.push_back(vps->getBspHrd(hrdIdx[i]));
    
      syntaxElemLen[i] = hrdVec[i]->getInitialCpbRemovalDelayLengthMinus1() + 1;
      if ( !(hrdVec[i]->getNalHrdParametersPresentFlag() || hrdVec[i]->getVclHrdParametersPresentFlag()) )
      {
        assert( syntaxElemLen[i] == 24 ); // Default of value init_cpb_removal_delay_length_minus1 is 23
      }
      if( i > 0 )
      {
        assert( hrdVec[i]->getCpbRemovalDelayLengthMinus1()   == hrdVec[i-1]->getCpbRemovalDelayLengthMinus1() );
        assert( hrdVec[i]->getDpbOutputDelayDuLengthMinus1()  == hrdVec[i-1]->getDpbOutputDelayDuLengthMinus1() );
        assert( hrdVec[i]->getSubPicCpbParamsPresentFlag()    == hrdVec[i-1]->getSubPicCpbParamsPresentFlag() );
      }
    }
    hrd = hrdVec[schedSelIdx];
  }
  else
  {
    const TComVUI *vui = sps->getVuiParameters();
    hrd = vui->getHrdParameters();
  }
  // To be done: When contained in an BSP HRD SEI message, the hrd structure is to be chosen differently.
#else
  const TComVUI *vui = sps->getVuiParameters();
  const TComHRD *hrd = vui->getHrdParameters();
#endif

  WRITE_UVLC( sei.m_bpSeqParameterSetId, "bp_seq_parameter_set_id" );
  if( !hrd->getSubPicCpbParamsPresentFlag() )
  {
    WRITE_FLAG( sei.m_rapCpbParamsPresentFlag, "irap_cpb_params_present_flag" );
  }
  if( sei.m_rapCpbParamsPresentFlag )
  {
    WRITE_CODE( sei.m_cpbDelayOffset, hrd->getCpbRemovalDelayLengthMinus1() + 1, "cpb_delay_offset" );
    WRITE_CODE( sei.m_dpbDelayOffset, hrd->getDpbOutputDelayLengthMinus1()  + 1, "dpb_delay_offset" );
  }
  WRITE_FLAG( sei.m_concatenationFlag, "concatenation_flag");
  WRITE_CODE( sei.m_auCpbRemovalDelayDelta - 1, ( hrd->getCpbRemovalDelayLengthMinus1() + 1 ), "au_cpb_removal_delay_delta_minus1" );
  for( nalOrVcl = 0; nalOrVcl < 2; nalOrVcl ++ )
  {
    if( ( ( nalOrVcl == 0 ) && ( hrd->getNalHrdParametersPresentFlag() ) ) ||
        ( ( nalOrVcl == 1 ) && ( hrd->getVclHrdParametersPresentFlag() ) ) )
    {
      for( i = 0; i < ( hrd->getCpbCntMinus1( 0 ) + 1 ); i ++ )
      {
        WRITE_CODE( sei.m_initialCpbRemovalDelay[i][nalOrVcl],( hrd->getInitialCpbRemovalDelayLengthMinus1() + 1 ) ,           "initial_cpb_removal_delay" );
        WRITE_CODE( sei.m_initialCpbRemovalDelayOffset[i][nalOrVcl],( hrd->getInitialCpbRemovalDelayLengthMinus1() + 1 ),      "initial_cpb_removal_delay_offset" );
        if( hrd->getSubPicCpbParamsPresentFlag() || sei.m_rapCpbParamsPresentFlag )
        {
          WRITE_CODE( sei.m_initialAltCpbRemovalDelay[i][nalOrVcl], ( hrd->getInitialCpbRemovalDelayLengthMinus1() + 1 ) ,     "initial_alt_cpb_removal_delay" );
          WRITE_CODE( sei.m_initialAltCpbRemovalDelayOffset[i][nalOrVcl], ( hrd->getInitialCpbRemovalDelayLengthMinus1() + 1 ),"initial_alt_cpb_removal_delay_offset" );
        }
      }
    }
  }
#if P0138_USE_ALT_CPB_PARAMS_FLAG
  if (sei.m_useAltCpbParamsFlagPresent)
  {
    WRITE_FLAG( sei.m_useAltCpbParamsFlag, "use_alt_cpb_params_flag");
  }
#endif
}

#if SVC_EXTENSION
Void SEIWriter::xWriteSEIPictureTiming(const SEIPictureTiming& sei, const TComSPS *sps, const SEIScalableNesting* nestingSei, const SEIBspNesting* bspNestingSei, const TComVPS *vps)
#else
Void SEIWriter::xWriteSEIPictureTiming(const SEIPictureTiming& sei, const TComSPS *sps)
#endif
{
  Int i;
#if SVC_EXTENSION
  const TComHRD *hrd;    
  const TComVUI *vui = sps->getVuiParameters(); 
  if( bspNestingSei )   // If BP SEI contained inside a BSP nesting SEI message
  {
    assert( nestingSei );
    Int psIdx = bspNestingSei->m_seiPartitioningSchemeIdx;
    Int seiOlsIdx = bspNestingSei->m_seiOlsIdx;
    Int maxTemporalId = nestingSei->m_nestingMaxTemporalIdPlus1[0] - 1;
    Int maxValues = vps->getNumBspSchedulesMinus1(seiOlsIdx, psIdx, maxTemporalId) + 1;
    std::vector<Int> hrdIdx(maxValues, 0);
    std::vector<const TComHRD*> hrdVec;
    std::vector<Int> syntaxElemLen(maxValues, 0);
    for(i = 0; i < maxValues; i++)
    {
      hrdIdx[i] = vps->getBspHrdIdx( seiOlsIdx, psIdx, maxTemporalId, i, bspNestingSei->m_bspIdx);
      hrdVec.push_back(vps->getBspHrd(hrdIdx[i]));
    
      syntaxElemLen[i] = hrdVec[i]->getInitialCpbRemovalDelayLengthMinus1() + 1;
      if ( !(hrdVec[i]->getNalHrdParametersPresentFlag() || hrdVec[i]->getVclHrdParametersPresentFlag()) )
      {
        assert( syntaxElemLen[i] == 24 ); // Default of value init_cpb_removal_delay_length_minus1 is 23
      }
      if( i > 0 )
      {
        assert( hrdVec[i]->getSubPicCpbParamsPresentFlag()    == hrdVec[i-1]->getSubPicCpbParamsPresentFlag() );
        assert( hrdVec[i]->getSubPicCpbParamsInPicTimingSEIFlag()   == hrdVec[i-1]->getSubPicCpbParamsInPicTimingSEIFlag() );
        assert( hrdVec[i]->getCpbRemovalDelayLengthMinus1()  == hrdVec[i-1]->getCpbRemovalDelayLengthMinus1() );
        assert( hrdVec[i]->getDpbOutputDelayLengthMinus1()  == hrdVec[i-1]->getDpbOutputDelayLengthMinus1() );
        assert( hrdVec[i]->getDpbOutputDelayDuLengthMinus1()  == hrdVec[i-1]->getDpbOutputDelayDuLengthMinus1() );
        assert( hrdVec[i]->getDuCpbRemovalDelayLengthMinus1()  == hrdVec[i-1]->getDuCpbRemovalDelayLengthMinus1() );
        // To be done: Check CpbDpbDelaysPresentFlag
      }
    }
    hrd = hrdVec[0];
  }
  else
  {
    hrd = vui->getHrdParameters();
  }
  // To be done: When contained in an BSP HRD SEI message, the hrd structure is to be chosen differently.
#else
  const TComVUI *vui = sps->getVuiParameters();
  const TComHRD *hrd = vui->getHrdParameters();
#endif

  if( vui->getFrameFieldInfoPresentFlag() )
  {
    WRITE_CODE( sei.m_picStruct, 4,              "pic_struct" );
    WRITE_CODE( sei.m_sourceScanType, 2,         "source_scan_type" );
    WRITE_FLAG( sei.m_duplicateFlag ? 1 : 0,     "duplicate_flag" );
  }

  if( hrd->getCpbDpbDelaysPresentFlag() )
  {
    WRITE_CODE( sei.m_auCpbRemovalDelay - 1, ( hrd->getCpbRemovalDelayLengthMinus1() + 1 ),                                         "au_cpb_removal_delay_minus1" );
    WRITE_CODE( sei.m_picDpbOutputDelay, ( hrd->getDpbOutputDelayLengthMinus1() + 1 ),                                          "pic_dpb_output_delay" );
    if(hrd->getSubPicCpbParamsPresentFlag())
    {
      WRITE_CODE(sei.m_picDpbOutputDuDelay, hrd->getDpbOutputDelayDuLengthMinus1()+1, "pic_dpb_output_du_delay" );
    }
    if( hrd->getSubPicCpbParamsPresentFlag() && hrd->getSubPicCpbParamsInPicTimingSEIFlag() )
    {
      WRITE_UVLC( sei.m_numDecodingUnitsMinus1,     "num_decoding_units_minus1" );
      WRITE_FLAG( sei.m_duCommonCpbRemovalDelayFlag, "du_common_cpb_removal_delay_flag" );
      if( sei.m_duCommonCpbRemovalDelayFlag )
      {
        WRITE_CODE( sei.m_duCommonCpbRemovalDelayMinus1, ( hrd->getDuCpbRemovalDelayLengthMinus1() + 1 ),                       "du_common_cpb_removal_delay_minus1" );
      }
      for( i = 0; i <= sei.m_numDecodingUnitsMinus1; i ++ )
      {
        WRITE_UVLC( sei.m_numNalusInDuMinus1[ i ],  "num_nalus_in_du_minus1");
        if( ( !sei.m_duCommonCpbRemovalDelayFlag ) && ( i < sei.m_numDecodingUnitsMinus1 ) )
        {
          WRITE_CODE( sei.m_duCpbRemovalDelayMinus1[ i ], ( hrd->getDuCpbRemovalDelayLengthMinus1() + 1 ),                        "du_cpb_removal_delay_minus1" );
        }
      }
    }
  }
}
Void SEIWriter::xWriteSEIRecoveryPoint(const SEIRecoveryPoint& sei)
{
  WRITE_SVLC( sei.m_recoveryPocCnt,    "recovery_poc_cnt"    );
  WRITE_FLAG( sei.m_exactMatchingFlag, "exact_matching_flag" );
  WRITE_FLAG( sei.m_brokenLinkFlag,    "broken_link_flag"    );
}
Void SEIWriter::xWriteSEIFramePacking(const SEIFramePacking& sei)
{
  WRITE_UVLC( sei.m_arrangementId,                  "frame_packing_arrangement_id" );
  WRITE_FLAG( sei.m_arrangementCancelFlag,          "frame_packing_arrangement_cancel_flag" );

  if( sei.m_arrangementCancelFlag == 0 )
  {
    WRITE_CODE( sei.m_arrangementType, 7,           "frame_packing_arrangement_type" );

    WRITE_FLAG( sei.m_quincunxSamplingFlag,         "quincunx_sampling_flag" );
    WRITE_CODE( sei.m_contentInterpretationType, 6, "content_interpretation_type" );
    WRITE_FLAG( sei.m_spatialFlippingFlag,          "spatial_flipping_flag" );
    WRITE_FLAG( sei.m_frame0FlippedFlag,            "frame0_flipped_flag" );
    WRITE_FLAG( sei.m_fieldViewsFlag,               "field_views_flag" );
    WRITE_FLAG( sei.m_currentFrameIsFrame0Flag,     "current_frame_is_frame0_flag" );

    WRITE_FLAG( sei.m_frame0SelfContainedFlag,      "frame0_self_contained_flag" );
    WRITE_FLAG( sei.m_frame1SelfContainedFlag,      "frame1_self_contained_flag" );

    if(sei.m_quincunxSamplingFlag == 0 && sei.m_arrangementType != 5)
    {
      WRITE_CODE( sei.m_frame0GridPositionX, 4,     "frame0_grid_position_x" );
      WRITE_CODE( sei.m_frame0GridPositionY, 4,     "frame0_grid_position_y" );
      WRITE_CODE( sei.m_frame1GridPositionX, 4,     "frame1_grid_position_x" );
      WRITE_CODE( sei.m_frame1GridPositionY, 4,     "frame1_grid_position_y" );
    }

    WRITE_CODE( sei.m_arrangementReservedByte, 8,   "frame_packing_arrangement_reserved_byte" );
    WRITE_FLAG( sei.m_arrangementPersistenceFlag,   "frame_packing_arrangement_persistence_flag" );
  }

  WRITE_FLAG( sei.m_upsampledAspectRatio,           "upsampled_aspect_ratio" );
}

Void SEIWriter::xWriteSEISegmentedRectFramePacking(const SEISegmentedRectFramePacking& sei)
{
  WRITE_FLAG( sei.m_arrangementCancelFlag,          "segmented_rect_frame_packing_arrangement_cancel_flag" );
  if( sei.m_arrangementCancelFlag == 0 ) 
  {
    WRITE_CODE( sei.m_contentInterpretationType, 2, "segmented_rect_content_interpretation_type" );
    WRITE_FLAG( sei.m_arrangementPersistenceFlag,   "segmented_rect_frame_packing_arrangement_persistence" );
  }
}

Void SEIWriter::xWriteSEIToneMappingInfo(const SEIToneMappingInfo& sei)
{
  Int i;
  WRITE_UVLC( sei.m_toneMapId,                    "tone_map_id" );
  WRITE_FLAG( sei.m_toneMapCancelFlag,            "tone_map_cancel_flag" );
  if( !sei.m_toneMapCancelFlag )
  {
    WRITE_FLAG( sei.m_toneMapPersistenceFlag,     "tone_map_persistence_flag" );
    WRITE_CODE( sei.m_codedDataBitDepth,    8,    "coded_data_bit_depth" );
    WRITE_CODE( sei.m_targetBitDepth,       8,    "target_bit_depth" );
    WRITE_UVLC( sei.m_modelId,                    "model_id" );
    switch(sei.m_modelId)
    {
    case 0:
      {
        WRITE_CODE( sei.m_minValue,  32,        "min_value" );
        WRITE_CODE( sei.m_maxValue, 32,         "max_value" );
        break;
      }
    case 1:
      {
        WRITE_CODE( sei.m_sigmoidMidpoint, 32,  "sigmoid_midpoint" );
        WRITE_CODE( sei.m_sigmoidWidth,    32,  "sigmoid_width"    );
        break;
      }
    case 2:
      {
        UInt num = 1u << sei.m_targetBitDepth;
        for(i = 0; i < num; i++)
        {
          WRITE_CODE( sei.m_startOfCodedInterval[i], (( sei.m_codedDataBitDepth + 7 ) >> 3 ) << 3,  "start_of_coded_interval" );
        }
        break;
      }
    case 3:
      {
        WRITE_CODE( sei.m_numPivots, 16,          "num_pivots" );
        for(i = 0; i < sei.m_numPivots; i++ )
        {
          WRITE_CODE( sei.m_codedPivotValue[i], (( sei.m_codedDataBitDepth + 7 ) >> 3 ) << 3,       "coded_pivot_value" );
          WRITE_CODE( sei.m_targetPivotValue[i], (( sei.m_targetBitDepth + 7 ) >> 3 ) << 3,         "target_pivot_value");
        }
        break;
      }
    case 4:
      {
        WRITE_CODE( sei.m_cameraIsoSpeedIdc,    8,    "camera_iso_speed_idc" );
        if( sei.m_cameraIsoSpeedIdc == 255) //Extended_ISO
        {
          WRITE_CODE( sei.m_cameraIsoSpeedValue,    32,    "camera_iso_speed_value" );
        }
        WRITE_CODE( sei.m_exposureIndexIdc,     8,    "exposure_index_idc" );
        if( sei.m_exposureIndexIdc == 255) //Extended_ISO
        {
          WRITE_CODE( sei.m_exposureIndexValue,     32,    "exposure_index_value" );
        }
        WRITE_FLAG( sei.m_exposureCompensationValueSignFlag,           "exposure_compensation_value_sign_flag" );
        WRITE_CODE( sei.m_exposureCompensationValueNumerator,     16,  "exposure_compensation_value_numerator" );
        WRITE_CODE( sei.m_exposureCompensationValueDenomIdc,      16,  "exposure_compensation_value_denom_idc" );
        WRITE_CODE( sei.m_refScreenLuminanceWhite,                32,  "ref_screen_luminance_white" );
        WRITE_CODE( sei.m_extendedRangeWhiteLevel,                32,  "extended_range_white_level" );
        WRITE_CODE( sei.m_nominalBlackLevelLumaCodeValue,         16,  "nominal_black_level_luma_code_value" );
        WRITE_CODE( sei.m_nominalWhiteLevelLumaCodeValue,         16,  "nominal_white_level_luma_code_value" );
        WRITE_CODE( sei.m_extendedWhiteLevelLumaCodeValue,        16,  "extended_white_level_luma_code_value" );
        break;
      }
    default:
      {
        assert(!"Undefined SEIToneMapModelId");
        break;
      }
    }//switch m_modelId
  }//if(!sei.m_toneMapCancelFlag)
}

Void SEIWriter::xWriteSEIDisplayOrientation(const SEIDisplayOrientation &sei)
{
  WRITE_FLAG( sei.cancelFlag,           "display_orientation_cancel_flag" );
  if( !sei.cancelFlag )
  {
    WRITE_FLAG( sei.horFlip,                   "hor_flip" );
    WRITE_FLAG( sei.verFlip,                   "ver_flip" );
    WRITE_CODE( sei.anticlockwiseRotation, 16, "anticlockwise_rotation" );
    WRITE_FLAG( sei.persistenceFlag,          "display_orientation_persistence_flag" );
  }
}

Void SEIWriter::xWriteSEITemporalLevel0Index(const SEITemporalLevel0Index &sei)
{
  WRITE_CODE( sei.tl0Idx, 8 , "tl0_idx" );
  WRITE_CODE( sei.rapIdx, 8 , "rap_idx" );
}

Void SEIWriter::xWriteSEIGradualDecodingRefreshInfo(const SEIGradualDecodingRefreshInfo &sei)
{
  WRITE_FLAG( sei.m_gdrForegroundFlag, "gdr_foreground_flag");
}

Void SEIWriter::xWriteSEINoDisplay(const SEINoDisplay& /*sei*/)
{
}

Void SEIWriter::xWriteSEISOPDescription(const SEISOPDescription& sei)
{
  WRITE_UVLC( sei.m_sopSeqParameterSetId,           "sop_seq_parameter_set_id"               );
  WRITE_UVLC( sei.m_numPicsInSopMinus1,             "num_pics_in_sop_minus1"               );
  for (UInt i = 0; i <= sei.m_numPicsInSopMinus1; i++)
  {
    WRITE_CODE( sei.m_sopDescVclNaluType[i], 6, "sop_desc_vcl_nalu_type" );
    WRITE_CODE( sei.m_sopDescTemporalId[i],  3, "sop_desc_temporal_id" );
    if (sei.m_sopDescVclNaluType[i] != NAL_UNIT_CODED_SLICE_IDR_W_RADL && sei.m_sopDescVclNaluType[i] != NAL_UNIT_CODED_SLICE_IDR_N_LP)
    {
      WRITE_UVLC( sei.m_sopDescStRpsIdx[i],           "sop_desc_st_rps_idx"               );
    }
    if (i > 0)
    {
      WRITE_SVLC( sei.m_sopDescPocDelta[i],           "sop_desc_poc_delta"               );
    }
  }
}

#if O0164_MULTI_LAYER_HRD
Void SEIWriter::xWriteSEIScalableNesting(TComBitIf& bs, const SEIScalableNesting& sei, const TComVPS *vps, const TComSPS *sps)
#else
Void SEIWriter::xWriteSEIScalableNesting(TComBitIf& bs, const SEIScalableNesting& sei, const TComSPS *sps)
#endif
{
  WRITE_FLAG( sei.m_bitStreamSubsetFlag,             "bitstream_subset_flag"         );
  WRITE_FLAG( sei.m_nestingOpFlag,                   "nesting_op_flag      "         );
  if (sei.m_nestingOpFlag)
  {
    WRITE_FLAG( sei.m_defaultOpFlag,                 "default_op_flag"               );
    WRITE_UVLC( sei.m_nestingNumOpsMinus1,           "nesting_num_ops_minus1"        );
    for (UInt i = (sei.m_defaultOpFlag ? 1 : 0); i <= sei.m_nestingNumOpsMinus1; i++)
    {
      WRITE_CODE( sei.m_nestingMaxTemporalIdPlus1[i], 3,  "nesting_max_temporal_id_plus1" );
      WRITE_UVLC( sei.m_nestingOpIdx[i],                  "nesting_op_idx"                );
    }
  }
  else
  {
    WRITE_FLAG( sei.m_allLayersFlag,                      "all_layers_flag"               );
    if (!sei.m_allLayersFlag)
    {
      WRITE_CODE( sei.m_nestingNoOpMaxTemporalIdPlus1, 3, "nesting_no_op_max_temporal_id_plus1" );
      WRITE_UVLC( sei.m_nestingNumLayersMinus1,           "nesting_num_layers"                  );
      for (UInt i = 0; i <= sei.m_nestingNumLayersMinus1; i++)
      {
        WRITE_CODE( sei.m_nestingLayerId[i], 6,           "nesting_layer_id"              );
      }
    }
  }

  // byte alignment
  while ( m_pcBitIf->getNumberOfWrittenBits() % 8 != 0 )
  {
    WRITE_FLAG( 0, "nesting_zero_bit" );
  }

  // write nested SEI messages
#if O0164_MULTI_LAYER_HRD
  writeSEImessages(bs, sei.m_nestedSEIs, vps, sps, true, &sei);
#else
  writeSEImessages(bs, sei.m_nestedSEIs, sps, true);
#endif
}

Void SEIWriter::xWriteSEITempMotionConstrainedTileSets(const SEITempMotionConstrainedTileSets& sei)
{
  //UInt code;
  WRITE_FLAG((sei.m_mc_all_tiles_exact_sample_value_match_flag ? 1 : 0), "mc_all_tiles_exact_sample_value_match_flag"); 
  WRITE_FLAG((sei.m_each_tile_one_tile_set_flag                ? 1 : 0), "each_tile_one_tile_set_flag"               );

  if(!sei.m_each_tile_one_tile_set_flag)
  {
    WRITE_FLAG((sei.m_limited_tile_set_display_flag ? 1 : 0), "limited_tile_set_display_flag");
    WRITE_UVLC((sei.getNumberOfTileSets() - 1),               "num_sets_in_message_minus1"   );

    if(sei.getNumberOfTileSets() > 0)
    {
      for(Int i = 0; i < sei.getNumberOfTileSets(); i++)
      {
        WRITE_UVLC(sei.tileSetData(i).m_mcts_id, "mcts_id");

        if(sei.m_limited_tile_set_display_flag)
        { 
          WRITE_FLAG((sei.tileSetData(i).m_display_tile_set_flag ? 1 : 0), "display_tile_set_flag");  
        }

        WRITE_UVLC((sei.tileSetData(i).getNumberOfTileRects() - 1), "num_tile_rects_in_set_minus1"); 
        
        for(Int j = 0; j < sei.tileSetData(i).getNumberOfTileRects(); j++)
        {
          WRITE_UVLC(sei.tileSetData(i).topLeftTileIndex    (j), "top_left_tile_index");  
          WRITE_UVLC(sei.tileSetData(i).bottomRightTileIndex(j), "bottom_right_tile_index");  
        }

        if(!sei.m_mc_all_tiles_exact_sample_value_match_flag)
        {
          WRITE_FLAG((sei.tileSetData(i).m_exact_sample_value_match_flag ? 1 : 0), "exact_sample_value_match_flag");  
        }

        WRITE_FLAG((sei.tileSetData(i).m_mcts_tier_level_idc_present_flag ? 1 : 0), "mcts_tier_level_idc_present_flag");

        if(sei.tileSetData(i).m_mcts_tier_level_idc_present_flag)
        {
          WRITE_FLAG((sei.tileSetData(i).m_mcts_tier_flag ? 1 : 0), "mcts_tier_flag");
          WRITE_CODE( sei.tileSetData(i).m_mcts_level_idc, 8,       "mcts_level_idc"); 
        }
      }
    }
  }
  else
  {
    WRITE_FLAG((sei.m_max_mcs_tier_level_idc_present_flag ? 1 : 0), "max_mcs_tier_level_idc_present_flag");

    if(sei.m_max_mcs_tier_level_idc_present_flag)
    {
      WRITE_FLAG((sei.m_max_mcts_tier_flag ? 1 : 0), "max_mcts_tier_flag");  
      WRITE_CODE( sei.m_max_mcts_level_idc, 8,       "max_mcts_level_idc"); 
    }
  }
}

Void SEIWriter::xWriteSEITimeCode(const SEITimeCode& sei)
{
  WRITE_CODE(sei.numClockTs, 2, "num_clock_ts");
  for(Int i = 0; i < sei.numClockTs; i++)
  {
    const TComSEITimeSet &currentTimeSet = sei.timeSetArray[i];
    WRITE_FLAG(currentTimeSet.clockTimeStampFlag, "clock_time_stamp_flag");
    if(currentTimeSet.clockTimeStampFlag)
    {
      WRITE_FLAG(currentTimeSet.numUnitFieldBasedFlag, "units_field_based_flag");
      WRITE_CODE(currentTimeSet.countingType, 5, "counting_type");
      WRITE_FLAG(currentTimeSet.fullTimeStampFlag, "full_timestamp_flag");
      WRITE_FLAG(currentTimeSet.discontinuityFlag, "discontinuity_flag");
      WRITE_FLAG(currentTimeSet.cntDroppedFlag, "cnt_dropped_flag");
      WRITE_CODE(currentTimeSet.numberOfFrames, 9, "n_frames");
      if(currentTimeSet.fullTimeStampFlag)
      {
        WRITE_CODE(currentTimeSet.secondsValue, 6, "seconds_value");
        WRITE_CODE(currentTimeSet.minutesValue, 6, "minutes_value");
        WRITE_CODE(currentTimeSet.hoursValue, 5, "hours_value");
      }
      else
      {
        WRITE_FLAG(currentTimeSet.secondsFlag, "seconds_flag");
        if(currentTimeSet.secondsFlag)
        {
          WRITE_CODE(currentTimeSet.secondsValue, 6, "seconds_value");
          WRITE_FLAG(currentTimeSet.minutesFlag, "minutes_flag");
          if(currentTimeSet.minutesFlag)
          {
            WRITE_CODE(currentTimeSet.minutesValue, 6, "minutes_value");
            WRITE_FLAG(currentTimeSet.hoursFlag, "hours_flag");
            if(currentTimeSet.hoursFlag)
            {
              WRITE_CODE(currentTimeSet.hoursValue, 5, "hours_value");
            }
          }
        }
      }
      WRITE_CODE(currentTimeSet.timeOffsetLength, 5, "time_offset_length");
      if(currentTimeSet.timeOffsetLength > 0)
      {
        if(currentTimeSet.timeOffsetValue >= 0)
        {
          WRITE_CODE((UInt)currentTimeSet.timeOffsetValue, currentTimeSet.timeOffsetLength, "time_offset_value");
        }
        else
        {
          //  Two's complement conversion
          UInt offsetValue = ~(currentTimeSet.timeOffsetValue) + 1;
          offsetValue |= (1 << (currentTimeSet.timeOffsetLength-1));
          WRITE_CODE(offsetValue, currentTimeSet.timeOffsetLength, "time_offset_value");
        }
      }
    }
  }
}

Void SEIWriter::xWriteSEIChromaResamplingFilterHint(const SEIChromaResamplingFilterHint &sei)
{
  WRITE_CODE(sei.m_verChromaFilterIdc, 8, "ver_chroma_filter_idc");
  WRITE_CODE(sei.m_horChromaFilterIdc, 8, "hor_chroma_filter_idc");
  WRITE_FLAG(sei.m_verFilteringFieldProcessingFlag, "ver_filtering_field_processing_flag");
  if(sei.m_verChromaFilterIdc == 1 || sei.m_horChromaFilterIdc == 1)
  {
    WRITE_UVLC(sei.m_targetFormatIdc, "target_format_idc");
    if(sei.m_verChromaFilterIdc == 1)
    {
      const Int numVerticalFilter = (Int)sei.m_verFilterCoeff.size();
      WRITE_UVLC(numVerticalFilter, "num_vertical_filters");
      if(numVerticalFilter > 0)
      {
        for(Int i = 0; i < numVerticalFilter; i ++)
        {
          const Int verTapLengthMinus1 = (Int) sei.m_verFilterCoeff[i].size() - 1;
          WRITE_UVLC(verTapLengthMinus1, "ver_tap_length_minus_1");
          for(Int j = 0; j < (verTapLengthMinus1 + 1); j ++)
          {
            WRITE_SVLC(sei.m_verFilterCoeff[i][j], "ver_filter_coeff");
          }
        }
      }
    }
    if(sei.m_horChromaFilterIdc == 1)
    {
      const Int numHorizontalFilter = (Int) sei.m_horFilterCoeff.size();
      WRITE_UVLC(numHorizontalFilter, "num_horizontal_filters");
      if(numHorizontalFilter > 0)
      {
        for(Int i = 0; i < numHorizontalFilter; i ++)
        {
          const Int horTapLengthMinus1 = (Int) sei.m_horFilterCoeff[i].size() - 1;
          WRITE_UVLC(horTapLengthMinus1, "hor_tap_length_minus_1");
          for(Int j = 0; j < (horTapLengthMinus1 + 1); j ++)
          {
            WRITE_SVLC(sei.m_horFilterCoeff[i][j], "hor_filter_coeff");
          }
        }
      }
    }
  }
}

Void SEIWriter::xWriteSEIKneeFunctionInfo(const SEIKneeFunctionInfo &sei)
{
  WRITE_UVLC( sei.m_kneeId, "knee_function_id" );
  WRITE_FLAG( sei.m_kneeCancelFlag, "knee_function_cancel_flag" ); 
  if ( !sei.m_kneeCancelFlag )
  {
    WRITE_FLAG( sei.m_kneePersistenceFlag, "knee_function_persistence_flag" );
    WRITE_CODE( (UInt)sei.m_kneeInputDrange , 32,  "input_d_range" );
    WRITE_CODE( (UInt)sei.m_kneeInputDispLuminance, 32,  "input_disp_luminance" );
    WRITE_CODE( (UInt)sei.m_kneeOutputDrange, 32,  "output_d_range" );
    WRITE_CODE( (UInt)sei.m_kneeOutputDispLuminance, 32,  "output_disp_luminance" );
    WRITE_UVLC( sei.m_kneeNumKneePointsMinus1, "num_knee_points_minus1" );
    for(Int i = 0; i <= sei.m_kneeNumKneePointsMinus1; i++ )
    {
      WRITE_CODE( (UInt)sei.m_kneeInputKneePoint[i], 10,"input_knee_point" );
      WRITE_CODE( (UInt)sei.m_kneeOutputKneePoint[i], 10, "output_knee_point" );
    }
  }
}

Void SEIWriter::xWriteSEIColourRemappingInfo(const SEIColourRemappingInfo& sei)
{
  WRITE_UVLC( sei.m_colourRemapId,                             "colour_remap_id" );
  WRITE_FLAG( sei.m_colourRemapCancelFlag,                     "colour_remap_cancel_flag" );
  if( !sei.m_colourRemapCancelFlag ) 
  {
    WRITE_FLAG( sei.m_colourRemapPersistenceFlag,              "colour_remap_persistence_flag" );
    WRITE_FLAG( sei.m_colourRemapVideoSignalInfoPresentFlag,   "colour_remap_video_signal_info_present_flag" );
    if ( sei.m_colourRemapVideoSignalInfoPresentFlag )
    {
      WRITE_FLAG( sei.m_colourRemapFullRangeFlag,              "colour_remap_full_range_flag" );
      WRITE_CODE( sei.m_colourRemapPrimaries,               8, "colour_remap_primaries" );
      WRITE_CODE( sei.m_colourRemapTransferFunction,        8, "colour_remap_transfer_function" );
      WRITE_CODE( sei.m_colourRemapMatrixCoefficients,      8, "colour_remap_matrix_coefficients" );
    }
    WRITE_CODE( sei.m_colourRemapInputBitDepth,             8, "colour_remap_input_bit_depth" );
    WRITE_CODE( sei.m_colourRemapBitDepth,                  8, "colour_remap_bit_depth" );
    for( Int c=0 ; c<3 ; c++ )
    {
      WRITE_CODE( sei.m_preLutNumValMinus1[c],              8, "pre_lut_num_val_minus1[c]" );
      if( sei.m_preLutNumValMinus1[c]>0 )
      {
        for( Int i=0 ; i<=sei.m_preLutNumValMinus1[c] ; i++ )
        {
          WRITE_CODE( sei.m_preLut[c][i].codedValue,  (( sei.m_colourRemapInputBitDepth + 7 ) >> 3 ) << 3, "pre_lut_coded_value[c][i]" );
          WRITE_CODE( sei.m_preLut[c][i].targetValue, (( sei.m_colourRemapBitDepth      + 7 ) >> 3 ) << 3, "pre_lut_target_value[c][i]" );
        }
      }
    }
    WRITE_FLAG( sei.m_colourRemapMatrixPresentFlag,            "colour_remap_matrix_present_flag" );
    if( sei.m_colourRemapMatrixPresentFlag )
    {
      WRITE_CODE( sei.m_log2MatrixDenom,                    4, "log2_matrix_denom" );
      for( Int c=0 ; c<3 ; c++ )
      {
        for( Int i=0 ; i<3 ; i++ )
        {
          WRITE_SVLC( sei.m_colourRemapCoeffs[c][i],           "colour_remap_coeffs[c][i]" );
        }
      }
    }

    for( Int c=0 ; c<3 ; c++ )
    {
      WRITE_CODE( sei.m_postLutNumValMinus1[c],             8, "m_postLutNumValMinus1[c]" );
      if( sei.m_postLutNumValMinus1[c]>0 )
      {
        for( Int i=0 ; i<=sei.m_postLutNumValMinus1[c] ; i++ )
        {
          WRITE_CODE( sei.m_postLut[c][i].codedValue, (( sei.m_colourRemapBitDepth + 7 ) >> 3 ) << 3, "post_lut_coded_value[c][i]" );
          WRITE_CODE( sei.m_postLut[c][i].targetValue, (( sei.m_colourRemapBitDepth + 7 ) >> 3 ) << 3, "post_lut_target_value[c][i]" );
        }
      }
    }
  }
}

Void SEIWriter::xWriteSEIMasteringDisplayColourVolume(const SEIMasteringDisplayColourVolume& sei)
{
  WRITE_CODE( sei.values.primaries[0][0],  16,  "display_primaries_x[0]" );
  WRITE_CODE( sei.values.primaries[0][1],  16,  "display_primaries_y[0]" );

  WRITE_CODE( sei.values.primaries[1][0],  16,  "display_primaries_x[1]" );
  WRITE_CODE( sei.values.primaries[1][1],  16,  "display_primaries_y[1]" );

  WRITE_CODE( sei.values.primaries[2][0],  16,  "display_primaries_x[2]" );
  WRITE_CODE( sei.values.primaries[2][1],  16,  "display_primaries_y[2]" );

  WRITE_CODE( sei.values.whitePoint[0],    16,  "white_point_x" );
  WRITE_CODE( sei.values.whitePoint[1],    16,  "white_point_y" );
    
  WRITE_CODE( sei.values.maxLuminance,     32,  "max_display_mastering_luminance" );
  WRITE_CODE( sei.values.minLuminance,     32,  "min_display_mastering_luminance" );
}


Void SEIWriter::xWriteByteAlign()
{
  if( m_pcBitIf->getNumberOfWrittenBits() % 8 != 0)
  {
    WRITE_FLAG( 1, "payload_bit_equal_to_one" );
    while( m_pcBitIf->getNumberOfWrittenBits() % 8 != 0 )
    {
      WRITE_FLAG( 0, "payload_bit_equal_to_zero" );
    }
  }
}

#if U0033_ALTERNATIVE_TRANSFER_CHARACTERISTICS_SEI
Void SEIWriter::xWriteSEIAlternativeTransferCharacteristics(const SEIAlternativeTransferCharacteristics& sei)
{
  WRITE_CODE(sei.m_preferredTransferCharacteristics, 8, "preferred_transfer_characteristics");
}
#endif

Void SEIWriter::xWriteSEIGreenMetadataInfo(const SEIGreenMetadataInfo& sei)
{
  WRITE_CODE(sei.m_greenMetadataType, 8, "green_metadata_type");
  
  WRITE_CODE(sei.m_xsdMetricType, 8, "xsd_metric_type");
  WRITE_CODE(sei.m_xsdMetricValue, 16, "xsd_metric_value");
}

#if SVC_EXTENSION
#if LAYERS_NOT_PRESENT_SEI
Void SEIWriter::xWriteSEILayersNotPresent(const SEILayersNotPresent& sei)
{
  WRITE_UVLC( sei.m_activeVpsId,           "lp_sei_active_vps_id" );
  for (UInt i = 0; i < sei.m_vpsMaxLayers; i++)
  {
    WRITE_FLAG( sei.m_layerNotPresentFlag[i], "layer_not_present_flag"   );
  }  
}
#endif

#if N0383_IL_CONSTRAINED_TILE_SETS_SEI
Void SEIWriter::xWriteSEIInterLayerConstrainedTileSets(const SEIInterLayerConstrainedTileSets& sei)
{
  WRITE_FLAG( sei.m_ilAllTilesExactSampleValueMatchFlag,  "il_all_tiles_exact_sample_value_match_flag"   );
  WRITE_FLAG( sei.m_ilOneTilePerTileSetFlag,              "il_one_tile_per_tile_set_flag"                );
  if( !sei.m_ilOneTilePerTileSetFlag )
  {
    WRITE_UVLC( sei.m_ilNumSetsInMessageMinus1,             "il_num_sets_in_message_minus1"                );
    if( sei.m_ilNumSetsInMessageMinus1 )
    {
      WRITE_FLAG( sei.m_skippedTileSetPresentFlag,            "skipped_tile_set_present_flag"                );
    }
    UInt numSignificantSets = sei.m_ilNumSetsInMessageMinus1 - (sei.m_skippedTileSetPresentFlag ? 1 : 0) + 1;
    for( UInt i = 0; i < numSignificantSets; i++ )
    {
      WRITE_UVLC( sei.m_ilctsId[i],                           "ilcts_id"                                     );
      WRITE_UVLC( sei.m_ilNumTileRectsInSetMinus1[i],         "il_num_tile_rects_in_set_minus1"              );
      for( UInt j = 0; j <= sei.m_ilNumTileRectsInSetMinus1[i]; j++ )
      {
        WRITE_UVLC( sei.m_ilTopLeftTileIndex[i][j],             "il_top_left_tile_index"                       );
        WRITE_UVLC( sei.m_ilBottomRightTileIndex[i][j],         "il_bottom_right_tile_index"                   );
      }
      WRITE_CODE( sei.m_ilcIdc[i], 2,                         "ilc_idc"                                      );
      if( sei.m_ilAllTilesExactSampleValueMatchFlag )
      {
        WRITE_FLAG( sei.m_ilExactSampleValueMatchFlag[i],        "il_exact_sample_value_match_flag"            );
      }
    }
  }
  else
  {
    WRITE_CODE( sei.m_allTilesIlcIdc, 2,                    "all_tiles_ilc_idc"                          );
  }
}
#endif
#if SUB_BITSTREAM_PROPERTY_SEI
Void SEIWriter::xWriteSEISubBitstreamProperty(const SEISubBitstreamProperty &sei)
{
  WRITE_CODE( sei.m_activeVpsId, 4, "active_vps_id" );
  assert( sei.m_numAdditionalSubStreams >= 1 );
  WRITE_UVLC( sei.m_numAdditionalSubStreams - 1, "num_additional_sub_streams_minus1" );

  for( Int i = 0; i < sei.m_numAdditionalSubStreams; i++ )
  {
    WRITE_CODE( sei.m_subBitstreamMode[i],       2, "sub_bitstream_mode[i]"           );
    WRITE_UVLC( sei.m_outputLayerSetIdxToVps[i],    "output_layer_set_idx_to_vps[i]"  );
    WRITE_CODE( sei.m_highestSublayerId[i],      3, "highest_sub_layer_id[i]"         );
    WRITE_CODE( sei.m_avgBitRate[i],            16, "avg_bit_rate[i]"                 );
    WRITE_CODE( sei.m_maxBitRate[i],            16, "max_bit_rate[i]"                 );
  }  
}
#endif

#if Q0189_TMVP_CONSTRAINTS 
Void SEIWriter::xWriteSEITMVPConstraints (const SEITMVPConstrains &sei)
{
  WRITE_UVLC( sei.prev_pics_not_used_flag ,    "prev_pics_not_used_flag"  );
  WRITE_UVLC( sei.no_intra_layer_col_pic_flag ,    "no_intra_layer_col_pic_flag"  ); 
}
#endif

#if Q0247_FRAME_FIELD_INFO
Void SEIWriter::xWriteSEIFrameFieldInfo  (const SEIFrameFieldInfo &sei)
{
  WRITE_CODE( sei.m_ffinfo_picStruct , 4,             "ffinfo_pic_struct" );
  WRITE_CODE( sei.m_ffinfo_sourceScanType, 2,         "ffinfo_source_scan_type" );
  WRITE_FLAG( sei.m_ffinfo_duplicateFlag ? 1 : 0,     "ffinfo_duplicate_flag" );
}
#endif

#if O0164_MULTI_LAYER_HRD
Void SEIWriter::xWriteSEIBspNesting(TComBitIf& bs, const SEIBspNesting &sei, const TComVPS *vps, const TComSPS *sps, const SEIScalableNesting &nestingSei)
{
  WRITE_UVLC( sei.m_bspIdx, "bsp_idx" );

  while ( m_pcBitIf->getNumberOfWrittenBits() % 8 != 0 )
  {
    WRITE_FLAG( 0, "bsp_nesting_zero_bit" );
  }

  assert( sei.m_nestedSEIs.size() <= MAX_SEIS_IN_BSP_NESTING );
  WRITE_UVLC( (UInt)sei.m_nestedSEIs.size(), "num_seis_in_bsp_minus1" );
  
  // write nested SEI messages
  writeSEImessages(bs, sei.m_nestedSEIs, vps, sps, true, &nestingSei, &sei);
}

Void SEIWriter::xWriteSEIBspInitialArrivalTime(const SEIBspInitialArrivalTime &sei, const TComVPS *vps, const TComSPS *sps, const SEIScalableNesting &nestingSei, const SEIBspNesting &bspNestingSei)
{
  assert(vps->getVpsVuiPresentFlag());

  Int psIdx = bspNestingSei.m_seiPartitioningSchemeIdx;
  Int seiOlsIdx = bspNestingSei.m_seiOlsIdx;
  Int maxTemporalId = nestingSei.m_nestingMaxTemporalIdPlus1[0] - 1;
  Int maxValues = vps->getNumBspSchedulesMinus1(seiOlsIdx, psIdx, maxTemporalId) + 1;
  std::vector<Int> hrdIdx(maxValues, 0);
  std::vector<const TComHRD*> hrdVec;
  std::vector<Int> syntaxElemLen(maxValues, 0);
  for(Int i = 0; i < maxValues; i++)
  {
    hrdIdx[i] = vps->getBspHrdIdx( seiOlsIdx, psIdx, maxTemporalId, i, bspNestingSei.m_bspIdx);
    hrdVec.push_back(vps->getBspHrd(hrdIdx[i]));
    
    syntaxElemLen[i] = hrdVec[i]->getInitialCpbRemovalDelayLengthMinus1() + 1;
    if ( !(hrdVec[i]->getNalHrdParametersPresentFlag() || hrdVec[i]->getVclHrdParametersPresentFlag()) )
    {
      assert( syntaxElemLen[i] == 24 ); // Default of value init_cpb_removal_delay_length_minus1 is 23
    }
    if( i > 0 )
    {
      assert( hrdVec[i]->getNalHrdParametersPresentFlag() == hrdVec[i-1]->getNalHrdParametersPresentFlag() );
      assert( hrdVec[i]->getVclHrdParametersPresentFlag() == hrdVec[i-1]->getVclHrdParametersPresentFlag() );
    }
  }
  if (hrdVec[0]->getNalHrdParametersPresentFlag())
  {
    for(UInt i = 0; i < maxValues; i++)
    {
      WRITE_CODE( sei.m_nalInitialArrivalDelay[i], syntaxElemLen[i], "nal_initial_arrival_delay[i]" );
    }
  }
  if( hrdVec[0]->getVclHrdParametersPresentFlag() )
  {
    for(UInt i = 0; i < maxValues; i++)
    {
      WRITE_CODE( sei.m_vclInitialArrivalDelay[i], syntaxElemLen[i], "vcl_initial_arrival_delay[i]" );
    }
  }
}

Void SEIWriter::xCodeHrdParameters( TComHRD *hrd, Bool commonInfPresentFlag, UInt maxNumSubLayersMinus1 )
{
  if( commonInfPresentFlag )
  {
    WRITE_FLAG( hrd->getNalHrdParametersPresentFlag() ? 1 : 0 ,  "nal_hrd_parameters_present_flag" );
    WRITE_FLAG( hrd->getVclHrdParametersPresentFlag() ? 1 : 0 ,  "vcl_hrd_parameters_present_flag" );
    if( hrd->getNalHrdParametersPresentFlag() || hrd->getVclHrdParametersPresentFlag() )
    {
      WRITE_FLAG( hrd->getSubPicCpbParamsPresentFlag() ? 1 : 0,  "sub_pic_cpb_params_present_flag" );
      if( hrd->getSubPicCpbParamsPresentFlag() )
      {
        WRITE_CODE( hrd->getTickDivisorMinus2(), 8,              "tick_divisor_minus2" );
        WRITE_CODE( hrd->getDuCpbRemovalDelayLengthMinus1(), 5,  "du_cpb_removal_delay_length_minus1" );
        WRITE_FLAG( hrd->getSubPicCpbParamsInPicTimingSEIFlag() ? 1 : 0, "sub_pic_cpb_params_in_pic_timing_sei_flag" );
        WRITE_CODE( hrd->getDpbOutputDelayDuLengthMinus1(), 5,   "dpb_output_delay_du_length_minus1"  );
      }
      WRITE_CODE( hrd->getBitRateScale(), 4,                     "bit_rate_scale" );
      WRITE_CODE( hrd->getCpbSizeScale(), 4,                     "cpb_size_scale" );
      if( hrd->getSubPicCpbParamsPresentFlag() )
      {
        WRITE_CODE( hrd->getDuCpbSizeScale(), 4,                "du_cpb_size_scale" ); 
      }
      WRITE_CODE( hrd->getInitialCpbRemovalDelayLengthMinus1(), 5, "initial_cpb_removal_delay_length_minus1" );
      WRITE_CODE( hrd->getCpbRemovalDelayLengthMinus1(),        5, "au_cpb_removal_delay_length_minus1" );
      WRITE_CODE( hrd->getDpbOutputDelayLengthMinus1(),         5, "dpb_output_delay_length_minus1" );
    }
  }
  Int i, j, nalOrVcl;
  for( i = 0; i <= maxNumSubLayersMinus1; i ++ )
  {
    WRITE_FLAG( hrd->getFixedPicRateFlag( i ) ? 1 : 0,          "fixed_pic_rate_general_flag");
    if( !hrd->getFixedPicRateFlag( i ) )
    {
      WRITE_FLAG( hrd->getFixedPicRateWithinCvsFlag( i ) ? 1 : 0, "fixed_pic_rate_within_cvs_flag");
    }
    else
    {
      hrd->setFixedPicRateWithinCvsFlag( i, true );
    }
    if( hrd->getFixedPicRateWithinCvsFlag( i ) )
    {
      WRITE_UVLC( hrd->getPicDurationInTcMinus1( i ),           "elemental_duration_in_tc_minus1");
    }
    else
    {
      WRITE_FLAG( hrd->getLowDelayHrdFlag( i ) ? 1 : 0,           "low_delay_hrd_flag");
    }
    if (!hrd->getLowDelayHrdFlag( i ))
    {
      WRITE_UVLC( hrd->getCpbCntMinus1( i ),                      "cpb_cnt_minus1");
    }
    
    for( nalOrVcl = 0; nalOrVcl < 2; nalOrVcl ++ )
    {
      if( ( ( nalOrVcl == 0 ) && ( hrd->getNalHrdParametersPresentFlag() ) ) ||
          ( ( nalOrVcl == 1 ) && ( hrd->getVclHrdParametersPresentFlag() ) ) )
      {
        for( j = 0; j <= ( hrd->getCpbCntMinus1( i ) ); j ++ )
        {
          WRITE_UVLC( hrd->getBitRateValueMinus1( i, j, nalOrVcl ), "bit_rate_value_minus1");
          WRITE_UVLC( hrd->getCpbSizeValueMinus1( i, j, nalOrVcl ), "cpb_size_value_minus1");
          if( hrd->getSubPicCpbParamsPresentFlag() )
          {
            WRITE_UVLC( hrd->getDuCpbSizeValueMinus1( i, j, nalOrVcl ), "cpb_size_du_value_minus1");  
            WRITE_UVLC( hrd->getDuBitRateValueMinus1( i, j, nalOrVcl ), "bit_rate_du_value_minus1");
          }
          WRITE_FLAG( hrd->getCbrFlag( i, j, nalOrVcl ) ? 1 : 0, "cbr_flag");
        }
      }
    }
  }
}

#endif

#if P0123_ALPHA_CHANNEL_SEI
Void SEIWriter::xWriteSEIAlphaChannelInfo(const SEIAlphaChannelInfo &sei)
{
  WRITE_FLAG(sei.m_alphaChannelCancelFlag, "alpha_channel_cancel_flag");
  if(!sei.m_alphaChannelCancelFlag)
  {
    WRITE_CODE(sei.m_alphaChannelUseIdc, 3, "alpha_channel_use_idc");
    WRITE_CODE(sei.m_alphaChannelBitDepthMinus8, 3, "alpha_channel_bit_depth_minus8");
    WRITE_CODE(sei.m_alphaTransparentValue, sei.m_alphaChannelBitDepthMinus8 + 9, "alpha_transparent_value");
    WRITE_CODE(sei.m_alphaOpaqueValue, sei.m_alphaChannelBitDepthMinus8 + 9, "alpha_opaque_value");
    WRITE_FLAG(sei.m_alphaChannelIncrFlag, "alpha_channel_incr_flag");
    WRITE_FLAG(sei.m_alphaChannelClipFlag, "alpha_channel_clip_flag");
    if(sei.m_alphaChannelClipFlag)
    {
      WRITE_FLAG(sei.m_alphaChannelClipTypeFlag, "alpha_channel_clip_type_flag");
    }
  }
  xWriteByteAlign();
}
#endif

#if Q0096_OVERLAY_SEI
Void SEIWriter::xWriteSEIOverlayInfo(const SEIOverlayInfo &sei)
{
  Int i,j;
  WRITE_FLAG( sei.m_overlayInfoCancelFlag, "overlay_info_cancel_flag" );
  if ( !sei.m_overlayInfoCancelFlag )
  {
    WRITE_UVLC( sei.m_overlayContentAuxIdMinus128, "overlay_content_aux_id_minus128" );
    WRITE_UVLC( sei.m_overlayLabelAuxIdMinus128, "overlay_label_aux_id_minus128" );
    WRITE_UVLC( sei.m_overlayAlphaAuxIdMinus128, "overlay_alpha_aux_id_minus128" );
    WRITE_UVLC( sei.m_overlayElementLabelValueLengthMinus8, "overlay_element_label_value_length_minus8" );
    assert( sei.m_numOverlaysMinus1 < MAX_OVERLAYS );
    WRITE_UVLC( sei.m_numOverlaysMinus1, "num_overlays_minus1" );
    for (i=0 ; i<=sei.m_numOverlaysMinus1 ; i++)
    {
      WRITE_UVLC( sei.m_overlayIdx[i], "overlay_idx" );
      WRITE_FLAG( sei.m_languageOverlayPresentFlag[i], "language_overlay_present_flag" );
      WRITE_CODE( sei.m_overlayContentLayerId[i], 6, "overlay_content_layer_id");
      WRITE_FLAG( sei.m_overlayLabelPresentFlag[i], "overlay_label_present_flag" );
      if ( sei.m_overlayLabelPresentFlag[i] )
      {
        WRITE_CODE(  sei.m_overlayLabelLayerId[i], 6, "overlay_label_layer_id");
      }
      WRITE_FLAG( sei.m_overlayAlphaPresentFlag[i], "overlay_alpha_present_flag" );
      if ( sei.m_overlayAlphaPresentFlag[i] )
      {
        WRITE_CODE( sei.m_overlayAlphaLayerId[i], 6, "overlay_alpha_layer_id");
      }
      if ( sei.m_overlayLabelPresentFlag[i] )
      {
        assert( sei.m_numOverlayElementsMinus1[i] < MAX_OVERLAY_ELEMENTS );
        WRITE_UVLC( sei.m_numOverlayElementsMinus1[i], "num_overlay_elements_minus1");
        for ( j=0 ; j<=sei.m_numOverlayElementsMinus1[i] ; j++ )
        {
          WRITE_CODE(sei.m_overlayElementLabelMin[i][j], sei.m_overlayElementLabelValueLengthMinus8 + 8, "overlay_element_label_min");
          WRITE_CODE(sei.m_overlayElementLabelMax[i][j], sei.m_overlayElementLabelValueLengthMinus8 + 8, "overlay_element_label_max"); 
        }
      }
    }


    // byte alignment
    while ( m_pcBitIf->getNumberOfWrittenBits() % 8 != 0 )
    {
      WRITE_FLAG( 0, "overlay_zero_bit" );
    }

    for ( i=0 ; i<=sei.m_numOverlaysMinus1 ; i++ )
    {
      if ( sei.m_languageOverlayPresentFlag[i] )
      {
        WRITE_STRING( sei.m_overlayLanguage[i], sei.m_overlayLanguageLength[i], "overlay_language" );    //WRITE_STRING adds zero-termination byte
      }
      WRITE_STRING( sei.m_overlayName[i], sei.m_overlayNameLength[i], "overlay_name" );
      if ( sei.m_overlayLabelPresentFlag[i] )
      {
        for ( j=0 ; j<=sei.m_numOverlayElementsMinus1[i] ; j++)
        {
          WRITE_STRING( sei.m_overlayElementName[i][j], sei.m_overlayElementNameLength[i][j], "overlay_element_name" );
        }
      }
    }
    WRITE_FLAG( sei.m_overlayInfoPersistenceFlag, "overlay_info_persistence_flag" );
  }
}
#endif

#endif //SVC_EXTENSION

//! \}
