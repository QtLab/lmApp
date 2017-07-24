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

/**
 \file     SEIread.cpp
 \brief    reading funtionality for SEI messages
 */

#include "TLibCommon/CommonDef.h"
#include "TLibCommon/TComBitStream.h"
#include "TLibCommon/SEI.h"
#include "TLibCommon/TComSlice.h"
#include "SyntaxElementParser.h"
#include "SEIread.h"
#include "TLibCommon/TComPicYuv.h"
#include <iomanip>


//! \ingroup TLibDecoder
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

Void SEIReader::sei_read_code(std::ostream *pOS, UInt uiLength, UInt& ruiCode, const TChar *pSymbolName)
{
  READ_CODE(uiLength, ruiCode, pSymbolName);
  if (pOS)
  {
    (*pOS) << "  " << std::setw(55) << pSymbolName << ": " << ruiCode << "\n";
  }
}

Void SEIReader::sei_read_uvlc(std::ostream *pOS, UInt& ruiCode, const TChar *pSymbolName)
{
  READ_UVLC(ruiCode, pSymbolName);
  if (pOS)
  {
    (*pOS) << "  " << std::setw(55) << pSymbolName << ": " << ruiCode << "\n";
  }
}

Void SEIReader::sei_read_svlc(std::ostream *pOS, Int& ruiCode, const TChar *pSymbolName)
{
  READ_SVLC(ruiCode, pSymbolName);
  if (pOS)
  {
    (*pOS) << "  " << std::setw(55) << pSymbolName << ": " << ruiCode << "\n";
  }
}

Void SEIReader::sei_read_flag(std::ostream *pOS, UInt& ruiCode, const TChar *pSymbolName)
{
  READ_FLAG(ruiCode, pSymbolName);
  if (pOS)
  {
    (*pOS) << "  " << std::setw(55) << pSymbolName << ": " << (ruiCode?1:0) << "\n";
  }
}

static inline Void output_sei_message_header(SEI &sei, std::ostream *pDecodedMessageOutputStream, UInt payloadSize)
{
  if (pDecodedMessageOutputStream)
  {
    std::string seiMessageHdr(SEI::getSEIMessageString(sei.payloadType())); seiMessageHdr+=" SEI message";
    (*pDecodedMessageOutputStream) << std::setfill('-') << std::setw(seiMessageHdr.size()) << "-" << std::setfill(' ') << "\n" << seiMessageHdr << " (" << payloadSize << " bytes)"<< "\n";
  }
}

#undef READ_CODE
#undef READ_SVLC
#undef READ_UVLC
#undef READ_FLAG


/**
 * unmarshal a single SEI message from bitstream bs
 */
#if LAYERS_NOT_PRESENT_SEI
Void SEIReader::parseSEImessage(TComInputBitstream* bs, SEIMessages& seis, const NalUnitType nalUnitType, const TComVPS *vps, const TComSPS *sps, std::ostream *pDecodedMessageOutputStream)
#else
Void SEIReader::parseSEImessage(TComInputBitstream* bs, SEIMessages& seis, const NalUnitType nalUnitType, const TComSPS *sps, std::ostream *pDecodedMessageOutputStream)
#endif
{
  setBitstream(bs);

  assert(!m_pcBitstream->getNumBitsUntilByteAligned());
  do
  {
#if LAYERS_NOT_PRESENT_SEI
    xReadSEImessage(seis, nalUnitType, vps, sps, pDecodedMessageOutputStream);
#else
    xReadSEImessage(seis, nalUnitType, sps, pDecodedMessageOutputStream);
#endif
    /* SEI messages are an integer number of bytes, something has failed
    * in the parsing if bitstream not byte-aligned */
    assert(!m_pcBitstream->getNumBitsUntilByteAligned());
  }
  while (m_pcBitstream->getNumBitsLeft() > 8);

  xReadRbspTrailingBits();
}

#if O0164_MULTI_LAYER_HRD
#if LAYERS_NOT_PRESENT_SEI
Void SEIReader::xReadSEImessage(SEIMessages& seis, const NalUnitType nalUnitType, const TComVPS *vps, const TComSPS *sps, std::ostream *pDecodedMessageOutputStream, const SEIScalableNesting *nestingSei, const SEIBspNesting *bspNestingSei)
#else
Void SEIReader::xReadSEImessage(SEIMessages& seis, const NalUnitType nalUnitType, const TComSPS *sps, std::ostream *pDecodedMessageOutputStream, const SEIScalableNesting *nestingSei)
#endif
#else
#if LAYERS_NOT_PRESENT_SEI
Void SEIReader::xReadSEImessage(SEIMessages& seis, const NalUnitType nalUnitType, const TComVPS *vps, const TComSPS *sps, std::ostream *pDecodedMessageOutputStream)
#else
Void SEIReader::xReadSEImessage(SEIMessages& seis, const NalUnitType nalUnitType, const TComSPS *sps, std::ostream *pDecodedMessageOutputStream)
#endif
#endif
{
#if ENC_DEC_TRACE
  xTraceSEIHeader();
#endif
  Int payloadType = 0;
  UInt val = 0;

  do
  {
    sei_read_code(NULL, 8, val, "payload_type");
    payloadType += val;
  } while (val==0xFF);

  UInt payloadSize = 0;
  do
  {
    sei_read_code(NULL, 8, val, "payload_size");
    payloadSize += val;
  } while (val==0xFF);

#if ENC_DEC_TRACE
  xTraceSEIMessageType((SEI::PayloadType)payloadType);
#endif

  /* extract the payload for this single SEI message.
   * This allows greater safety in erroneous parsing of an SEI message
   * from affecting subsequent messages.
   * After parsing the payload, bs needs to be restored as the primary
   * bitstream.
   */
  TComInputBitstream *bs = getBitstream();
  setBitstream(bs->extractSubstream(payloadSize * 8));

  SEI *sei = NULL;

  if(nalUnitType == NAL_UNIT_PREFIX_SEI)
  {
    switch (payloadType)
    {
    case SEI::USER_DATA_UNREGISTERED:
      sei = new SEIuserDataUnregistered;
      xParseSEIuserDataUnregistered((SEIuserDataUnregistered&) *sei, payloadSize, pDecodedMessageOutputStream);
      break;
    case SEI::ACTIVE_PARAMETER_SETS:
      sei = new SEIActiveParameterSets;
      xParseSEIActiveParameterSets((SEIActiveParameterSets&) *sei, payloadSize, pDecodedMessageOutputStream);
      break;
    case SEI::DECODING_UNIT_INFO:
      if (!sps)
      {
        printf ("Warning: Found Decoding unit SEI message, but no active SPS is available. Ignoring.");
      }
      else
      {
        sei = new SEIDecodingUnitInfo;
#if SVC_EXTENSION
        xParseSEIDecodingUnitInfo((SEIDecodingUnitInfo&) *sei, payloadSize, sps, nestingSei, bspNestingSei, vps, pDecodedMessageOutputStream);
#else        
        xParseSEIDecodingUnitInfo((SEIDecodingUnitInfo&) *sei, payloadSize, sps, pDecodedMessageOutputStream);
#endif
      }
      break;
    case SEI::BUFFERING_PERIOD:
      if (!sps)
      {
        printf ("Warning: Found Buffering period SEI message, but no active SPS is available. Ignoring.");
      }
      else
      {
        sei = new SEIBufferingPeriod;
#if SVC_EXTENSION
        xParseSEIBufferingPeriod((SEIBufferingPeriod&) *sei, payloadSize, sps, nestingSei, bspNestingSei, vps, pDecodedMessageOutputStream);
#else
        xParseSEIBufferingPeriod((SEIBufferingPeriod&) *sei, payloadSize, sps, pDecodedMessageOutputStream);
#endif
      }
      break;
    case SEI::PICTURE_TIMING:
      if (!sps)
      {
        printf ("Warning: Found Picture timing SEI message, but no active SPS is available. Ignoring.");
      }
      else
      {
        sei = new SEIPictureTiming;
#if SVC_EXTENSION
        xParseSEIPictureTiming((SEIPictureTiming&)*sei, payloadSize, sps, nestingSei, bspNestingSei, vps, pDecodedMessageOutputStream);
#else
        xParseSEIPictureTiming((SEIPictureTiming&)*sei, payloadSize, sps, pDecodedMessageOutputStream);
#endif
      }
      break;
    case SEI::RECOVERY_POINT:
      sei = new SEIRecoveryPoint;
      xParseSEIRecoveryPoint((SEIRecoveryPoint&) *sei, payloadSize, pDecodedMessageOutputStream);
      break;
    case SEI::FRAME_PACKING:
      sei = new SEIFramePacking;
      xParseSEIFramePacking((SEIFramePacking&) *sei, payloadSize, pDecodedMessageOutputStream);
      break;
    case SEI::SEGM_RECT_FRAME_PACKING:
      sei = new SEISegmentedRectFramePacking;
      xParseSEISegmentedRectFramePacking((SEISegmentedRectFramePacking&) *sei, payloadSize, pDecodedMessageOutputStream);
      break;
    case SEI::DISPLAY_ORIENTATION:
      sei = new SEIDisplayOrientation;
      xParseSEIDisplayOrientation((SEIDisplayOrientation&) *sei, payloadSize, pDecodedMessageOutputStream);
      break;
    case SEI::TEMPORAL_LEVEL0_INDEX:
      sei = new SEITemporalLevel0Index;
      xParseSEITemporalLevel0Index((SEITemporalLevel0Index&) *sei, payloadSize, pDecodedMessageOutputStream);
      break;
    case SEI::REGION_REFRESH_INFO:
      sei = new SEIGradualDecodingRefreshInfo;
      xParseSEIRegionRefreshInfo((SEIGradualDecodingRefreshInfo&) *sei, payloadSize, pDecodedMessageOutputStream);
      break;
    case SEI::NO_DISPLAY:
      sei = new SEINoDisplay;
      xParseSEINoDisplay((SEINoDisplay&) *sei, payloadSize, pDecodedMessageOutputStream);
      break;
    case SEI::TONE_MAPPING_INFO:
      sei = new SEIToneMappingInfo;
      xParseSEIToneMappingInfo((SEIToneMappingInfo&) *sei, payloadSize, pDecodedMessageOutputStream);
      break;
    case SEI::SOP_DESCRIPTION:
      sei = new SEISOPDescription;
      xParseSEISOPDescription((SEISOPDescription&) *sei, payloadSize, pDecodedMessageOutputStream);
      break;
    case SEI::SCALABLE_NESTING:
      sei = new SEIScalableNesting;
#if LAYERS_NOT_PRESENT_SEI
      xParseSEIScalableNesting((SEIScalableNesting&) *sei, nalUnitType, payloadSize, vps, sps, pDecodedMessageOutputStream);
#else
      xParseSEIScalableNesting((SEIScalableNesting&) *sei, nalUnitType, payloadSize, sps, pDecodedMessageOutputStream);
#endif
      break;
    case SEI::TEMP_MOTION_CONSTRAINED_TILE_SETS:
      sei = new SEITempMotionConstrainedTileSets;
      xParseSEITempMotionConstraintsTileSets((SEITempMotionConstrainedTileSets&) *sei, payloadSize, pDecodedMessageOutputStream);
      break;
    case SEI::TIME_CODE:
      sei = new SEITimeCode;
      xParseSEITimeCode((SEITimeCode&) *sei, payloadSize, pDecodedMessageOutputStream);
      break;
    case SEI::CHROMA_RESAMPLING_FILTER_HINT:
      sei = new SEIChromaResamplingFilterHint;
      xParseSEIChromaResamplingFilterHint((SEIChromaResamplingFilterHint&) *sei, payloadSize, pDecodedMessageOutputStream);
      //}
      break;
    case SEI::KNEE_FUNCTION_INFO:
      sei = new SEIKneeFunctionInfo;
      xParseSEIKneeFunctionInfo((SEIKneeFunctionInfo&) *sei, payloadSize, pDecodedMessageOutputStream);
      break;
    case SEI::COLOUR_REMAPPING_INFO:
      sei = new SEIColourRemappingInfo;
      xParseSEIColourRemappingInfo((SEIColourRemappingInfo&) *sei, payloadSize, pDecodedMessageOutputStream);
      break;
    case SEI::MASTERING_DISPLAY_COLOUR_VOLUME:
      sei = new SEIMasteringDisplayColourVolume;
      xParseSEIMasteringDisplayColourVolume((SEIMasteringDisplayColourVolume&) *sei, payloadSize, pDecodedMessageOutputStream);
      break;
#if U0033_ALTERNATIVE_TRANSFER_CHARACTERISTICS_SEI
    case SEI::ALTERNATIVE_TRANSFER_CHARACTERISTICS:
      sei = new SEIAlternativeTransferCharacteristics;
      xParseSEIAlternativeTransferCharacteristics((SEIAlternativeTransferCharacteristics&) *sei, payloadSize, pDecodedMessageOutputStream);
      break;
#endif

#if SVC_EXTENSION
#if LAYERS_NOT_PRESENT_SEI
    case SEI::LAYERS_NOT_PRESENT:
      if (!vps)
      {
        printf ("Warning: Found Layers not present SEI message, but no active VPS is available. Ignoring.");
      }
      else
      {
        sei = new SEILayersNotPresent;
        xParseSEILayersNotPresent((SEILayersNotPresent&) *sei, payloadSize, vps, pDecodedMessageOutputStream);
      }
      break;
#endif
#if N0383_IL_CONSTRAINED_TILE_SETS_SEI
    case SEI::INTER_LAYER_CONSTRAINED_TILE_SETS:
      sei = new SEIInterLayerConstrainedTileSets;
      xParseSEIInterLayerConstrainedTileSets((SEIInterLayerConstrainedTileSets&) *sei, payloadSize, pDecodedMessageOutputStream);
      break;
#endif
#if SUB_BITSTREAM_PROPERTY_SEI
    case SEI::SUB_BITSTREAM_PROPERTY:
      sei = new SEISubBitstreamProperty;
      xParseSEISubBitstreamProperty((SEISubBitstreamProperty&) *sei, vps, pDecodedMessageOutputStream);
      break;
#endif
#if O0164_MULTI_LAYER_HRD
    case SEI::BSP_NESTING:
      sei = new SEIBspNesting;
#if LAYERS_NOT_PRESENT_SEI
      xParseSEIBspNesting((SEIBspNesting&) *sei, nalUnitType, vps, sps, *nestingSei, pDecodedMessageOutputStream);
#else
      xParseSEIBspNesting((SEIBspNesting&) *sei, nalUnitType, sps, *nestingSei, pDecodedMessageOutputStream);
#endif
      break;
    case SEI::BSP_INITIAL_ARRIVAL_TIME:
      sei = new SEIBspInitialArrivalTime;
      xParseSEIBspInitialArrivalTime((SEIBspInitialArrivalTime&) *sei, vps, sps, *nestingSei, *bspNestingSei, pDecodedMessageOutputStream);
      break;
#endif
#if Q0189_TMVP_CONSTRAINTS
    case SEI::TMVP_CONSTRAINTS:
      sei =  new SEITMVPConstrains;
      xParseSEITMVPConstraints((SEITMVPConstrains&) *sei, payloadSize, pDecodedMessageOutputStream);
      break;
#endif
#if Q0247_FRAME_FIELD_INFO
    case SEI::FRAME_FIELD_INFO:
      sei =  new SEIFrameFieldInfo;
      xParseSEIFrameFieldInfo    ((SEIFrameFieldInfo&) *sei, payloadSize, pDecodedMessageOutputStream);
      break;
#endif
#if P0123_ALPHA_CHANNEL_SEI
    case SEI::ALPHA_CHANNEL_INFO:
      sei = new SEIAlphaChannelInfo;
      xParseSEIAlphaChannelInfo((SEIAlphaChannelInfo &) *sei, payloadSize, pDecodedMessageOutputStream);
      break;
#endif
#if Q0096_OVERLAY_SEI
    case SEI::OVERLAY_INFO:
      sei = new SEIOverlayInfo;
      xParseSEIOverlayInfo((SEIOverlayInfo&) *sei, payloadSize, pDecodedMessageOutputStream);
      break;
#endif
#endif //SVC_EXTENSION
    default:
      for (UInt i = 0; i < payloadSize; i++)
      {
        UInt seiByte;
        sei_read_code (NULL, 8, seiByte, "unknown prefix SEI payload byte");
      }
      printf ("Unknown prefix SEI message (payloadType = %d) was found!\n", payloadType);
      if (pDecodedMessageOutputStream)
      {
        (*pDecodedMessageOutputStream) << "Unknown prefix SEI message (payloadType = " << payloadType << ") was found!\n";
      }
      break;
    }
  }
  else
  {
    switch (payloadType)
    {
      case SEI::USER_DATA_UNREGISTERED:
        sei = new SEIuserDataUnregistered;
        xParseSEIuserDataUnregistered((SEIuserDataUnregistered&) *sei, payloadSize, pDecodedMessageOutputStream);
        break;
      case SEI::DECODED_PICTURE_HASH:
        sei = new SEIDecodedPictureHash;
        xParseSEIDecodedPictureHash((SEIDecodedPictureHash&) *sei, payloadSize, pDecodedMessageOutputStream);
        break;
      case SEI::GREEN_METADATA:
        sei = new SEIGreenMetadataInfo;
        xParseSEIGreenMetadataInfo((SEIGreenMetadataInfo&) *sei, payloadSize, pDecodedMessageOutputStream);
        break;
      default:
        for (UInt i = 0; i < payloadSize; i++)
        {
          UInt seiByte;
          sei_read_code( NULL, 8, seiByte, "unknown suffix SEI payload byte");
        }
        printf ("Unknown suffix SEI message (payloadType = %d) was found!\n", payloadType);
        if (pDecodedMessageOutputStream)
        {
          (*pDecodedMessageOutputStream) << "Unknown suffix SEI message (payloadType = " << payloadType << ") was found!\n";
        }
        break;
    }
  }

  if (sei != NULL)
  {
    seis.push_back(sei);
  }

  /* By definition the underlying bitstream terminates in a byte-aligned manner.
   * 1. Extract all bar the last MIN(bitsremaining,nine) bits as reserved_payload_extension_data
   * 2. Examine the final 8 bits to determine the payload_bit_equal_to_one marker
   * 3. Extract the remainingreserved_payload_extension_data bits.
   *
   * If there are fewer than 9 bits available, extract them.
   */
  Int payloadBitsRemaining = getBitstream()->getNumBitsLeft();
  if (payloadBitsRemaining) /* more_data_in_payload() */
  {
    for (; payloadBitsRemaining > 9; payloadBitsRemaining--)
    {
      UInt reservedPayloadExtensionData;
      sei_read_code ( pDecodedMessageOutputStream, 1, reservedPayloadExtensionData, "reserved_payload_extension_data");
    }

    /* 2 */
    Int finalBits = getBitstream()->peekBits(payloadBitsRemaining);
    Int finalPayloadBits = 0;
    for (Int mask = 0xff; finalBits & (mask >> finalPayloadBits); finalPayloadBits++)
    {
      continue;
    }

    /* 3 */
    for (; payloadBitsRemaining > 9 - finalPayloadBits; payloadBitsRemaining--)
    {
      UInt reservedPayloadExtensionData;
      sei_read_flag ( 0, reservedPayloadExtensionData, "reserved_payload_extension_data");
    }

    UInt dummy;
    sei_read_flag( 0, dummy, "payload_bit_equal_to_one"); payloadBitsRemaining--;
    while (payloadBitsRemaining)
    {
      sei_read_flag( 0, dummy, "payload_bit_equal_to_zero"); payloadBitsRemaining--;
    }
  }

  /* restore primary bitstream for sei_message */
  delete getBitstream();
  setBitstream(bs);
}

/**
 * parse bitstream bs and unpack a user_data_unregistered SEI message
 * of payloasSize bytes into sei.
 */

Void SEIReader::xParseSEIuserDataUnregistered(SEIuserDataUnregistered &sei, UInt payloadSize, std::ostream *pDecodedMessageOutputStream)
{
  assert(payloadSize >= ISO_IEC_11578_LEN);
  UInt val;
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);

  for (UInt i = 0; i < ISO_IEC_11578_LEN; i++)
  {
    sei_read_code( pDecodedMessageOutputStream, 8, val, "uuid_iso_iec_11578");
    sei.uuid_iso_iec_11578[i] = val;
  }

  sei.userDataLength = payloadSize - ISO_IEC_11578_LEN;
  if (!sei.userDataLength)
  {
    sei.userData = 0;
    return;
  }

  sei.userData = new UChar[sei.userDataLength];
  for (UInt i = 0; i < sei.userDataLength; i++)
  {
    sei_read_code( NULL, 8, val, "user_data_payload_byte" );
    sei.userData[i] = val;
  }
  if (pDecodedMessageOutputStream)
  {
    (*pDecodedMessageOutputStream) << "  User data payload size: " << sei.userDataLength << "\n";
  }
}

/**
 * parse bitstream bs and unpack a decoded picture hash SEI message
 * of payloadSize bytes into sei.
 */
Void SEIReader::xParseSEIDecodedPictureHash(SEIDecodedPictureHash& sei, UInt payloadSize, std::ostream *pDecodedMessageOutputStream)
{
  UInt bytesRead = 0;
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);

  UInt val;
  sei_read_code( pDecodedMessageOutputStream, 8, val, "hash_type");
  sei.method = static_cast<HashType>(val); bytesRead++;

  const TChar *traceString="\0";
  switch (sei.method)
  {
    case HASHTYPE_MD5: traceString="picture_md5"; break;
    case HASHTYPE_CRC: traceString="picture_crc"; break;
    case HASHTYPE_CHECKSUM: traceString="picture_checksum"; break;
    default: assert(false); break;
  }

  if (pDecodedMessageOutputStream)
  {
    (*pDecodedMessageOutputStream) << "  " << std::setw(55) << traceString << ": " << std::hex << std::setfill('0');
  }

  sei.m_pictureHash.hash.clear();
  for(;bytesRead < payloadSize; bytesRead++)
  {
    sei_read_code( NULL, 8, val, traceString);
    sei.m_pictureHash.hash.push_back((UChar)val);
    if (pDecodedMessageOutputStream)
    {
      (*pDecodedMessageOutputStream) << std::setw(2) << val;
    }
  }

  if (pDecodedMessageOutputStream)
  {
    (*pDecodedMessageOutputStream) << std::dec << std::setfill(' ') << "\n";
  }
}

Void SEIReader::xParseSEIActiveParameterSets(SEIActiveParameterSets& sei, UInt payloadSize, std::ostream *pDecodedMessageOutputStream)
{
  UInt val; 
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);

  sei_read_code( pDecodedMessageOutputStream, 4, val, "active_video_parameter_set_id");   sei.activeVPSId = val;
  sei_read_flag( pDecodedMessageOutputStream,    val, "self_contained_cvs_flag");         sei.m_selfContainedCvsFlag     = (val != 0);
  sei_read_flag( pDecodedMessageOutputStream,    val, "no_parameter_set_update_flag");    sei.m_noParameterSetUpdateFlag = (val != 0);
  sei_read_uvlc( pDecodedMessageOutputStream,    val, "num_sps_ids_minus1");              sei.numSpsIdsMinus1 = val;

  sei.activeSeqParameterSetId.resize(sei.numSpsIdsMinus1 + 1);
#if R0247_SEI_ACTIVE
  sei.layerSpsIdx.resize(sei.numSpsIdsMinus1 + 1);
#endif
  for (Int i=0; i < (sei.numSpsIdsMinus1 + 1); i++)
  {
    sei_read_uvlc( pDecodedMessageOutputStream, val, "active_seq_parameter_set_id[i]");    sei.activeSeqParameterSetId[i] = val;
  }
#if R0247_SEI_ACTIVE
  for (Int i=1; i < (sei.numSpsIdsMinus1 + 1); i++)
  {
    sei_read_uvlc( pDecodedMessageOutputStream, val, "layer_sps_idx"); sei.layerSpsIdx[i] = val; 
  }
#endif  
}

#if SVC_EXTENSION
Void SEIReader::xParseSEIDecodingUnitInfo(SEIDecodingUnitInfo& sei, UInt payloadSize, const TComSPS *sps, const SEIScalableNesting* nestingSei, const SEIBspNesting* bspNestingSei, const TComVPS *vps, std::ostream *pDecodedMessageOutputStream)
#else
Void SEIReader::xParseSEIDecodingUnitInfo(SEIDecodingUnitInfo& sei, UInt payloadSize, const TComSPS *sps, std::ostream *pDecodedMessageOutputStream)
#endif
{
  UInt val;
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);
  sei_read_uvlc( pDecodedMessageOutputStream, val, "decoding_unit_idx");
  sei.m_decodingUnitIdx = val;

#if SVC_EXTENSION
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

  if(hrd->getSubPicCpbParamsInPicTimingSEIFlag())
  {
    sei_read_code( pDecodedMessageOutputStream, ( hrd->getDuCpbRemovalDelayLengthMinus1() + 1 ), val, "du_spt_cpb_removal_delay_increment");
    sei.m_duSptCpbRemovalDelay = val;
  }
  else
  {
    sei.m_duSptCpbRemovalDelay = 0;
  }
  sei_read_flag( pDecodedMessageOutputStream, val, "dpb_output_du_delay_present_flag"); sei.m_dpbOutputDuDelayPresentFlag = (val != 0);
  if(sei.m_dpbOutputDuDelayPresentFlag)
  {
    sei_read_code( pDecodedMessageOutputStream, hrd->getDpbOutputDelayDuLengthMinus1() + 1, val, "pic_spt_dpb_output_du_delay");
    sei.m_picSptDpbOutputDuDelay = val;
  }
#else
  const TComVUI *vui = sps->getVuiParameters();
  if(vui->getHrdParameters()->getSubPicCpbParamsInPicTimingSEIFlag())
  {
    sei_read_code( pDecodedMessageOutputStream, ( vui->getHrdParameters()->getDuCpbRemovalDelayLengthMinus1() + 1 ), val, "du_spt_cpb_removal_delay_increment");
    sei.m_duSptCpbRemovalDelay = val;
  }
  else
  {
    sei.m_duSptCpbRemovalDelay = 0;
  }
  sei_read_flag( pDecodedMessageOutputStream, val, "dpb_output_du_delay_present_flag"); sei.m_dpbOutputDuDelayPresentFlag = (val != 0);
  if(sei.m_dpbOutputDuDelayPresentFlag)
  {
    sei_read_code( pDecodedMessageOutputStream, vui->getHrdParameters()->getDpbOutputDelayDuLengthMinus1() + 1, val, "pic_spt_dpb_output_du_delay");
    sei.m_picSptDpbOutputDuDelay = val;
  }
#endif
}

#if SVC_EXTENSION
Void SEIReader::xParseSEIBufferingPeriod(SEIBufferingPeriod& sei, UInt payloadSize, const TComSPS *sps, const SEIScalableNesting* nestingSei, const SEIBspNesting* bspNestingSei, const TComVPS *vps, std::ostream *pDecodedMessageOutputStream)
#else
Void SEIReader::xParseSEIBufferingPeriod(SEIBufferingPeriod& sei, UInt payloadSize, const TComSPS *sps, std::ostream *pDecodedMessageOutputStream)
#endif
{
  Int i, nalOrVcl;
  UInt code;

#if SVC_EXTENSION
  const TComHRD *pHRD;
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
    pHRD = hrdVec[schedSelIdx];
  }
  else
  {
    const TComVUI *vui = sps->getVuiParameters();
    pHRD = vui->getHrdParameters();
  }
  // To be done: When contained in an BSP HRD SEI message, the hrd structure is to be chosen differently.
#else
  const TComVUI *pVUI = sps->getVuiParameters();
  const TComHRD *pHRD = pVUI->getHrdParameters();
#endif

  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);

  sei_read_uvlc( pDecodedMessageOutputStream, code, "bp_seq_parameter_set_id" );                         sei.m_bpSeqParameterSetId     = code;
  if( !pHRD->getSubPicCpbParamsPresentFlag() )
  {
    sei_read_flag( pDecodedMessageOutputStream, code, "irap_cpb_params_present_flag" );                   sei.m_rapCpbParamsPresentFlag = code;
  }
  if( sei.m_rapCpbParamsPresentFlag )
  {
    sei_read_code( pDecodedMessageOutputStream, pHRD->getCpbRemovalDelayLengthMinus1() + 1, code, "cpb_delay_offset" );      sei.m_cpbDelayOffset = code;
    sei_read_code( pDecodedMessageOutputStream, pHRD->getDpbOutputDelayLengthMinus1()  + 1, code, "dpb_delay_offset" );      sei.m_dpbDelayOffset = code;
  }

  //read splicing flag and cpb_removal_delay_delta
  sei_read_flag( pDecodedMessageOutputStream, code, "concatenation_flag");
  sei.m_concatenationFlag = code;
  sei_read_code( pDecodedMessageOutputStream, ( pHRD->getCpbRemovalDelayLengthMinus1() + 1 ), code, "au_cpb_removal_delay_delta_minus1" );
  sei.m_auCpbRemovalDelayDelta = code + 1;

  for( nalOrVcl = 0; nalOrVcl < 2; nalOrVcl ++ )
  {
    if( ( ( nalOrVcl == 0 ) && ( pHRD->getNalHrdParametersPresentFlag() ) ) ||
        ( ( nalOrVcl == 1 ) && ( pHRD->getVclHrdParametersPresentFlag() ) ) )
    {
      for( i = 0; i < ( pHRD->getCpbCntMinus1( 0 ) + 1 ); i ++ )
      {
        sei_read_code( pDecodedMessageOutputStream, ( pHRD->getInitialCpbRemovalDelayLengthMinus1() + 1 ) , code, nalOrVcl?"vcl_initial_cpb_removal_delay":"nal_initial_cpb_removal_delay" );
        sei.m_initialCpbRemovalDelay[i][nalOrVcl] = code;
        sei_read_code( pDecodedMessageOutputStream, ( pHRD->getInitialCpbRemovalDelayLengthMinus1() + 1 ) , code, nalOrVcl?"vcl_initial_cpb_removal_offset":"nal_initial_cpb_removal_offset" );
        sei.m_initialCpbRemovalDelayOffset[i][nalOrVcl] = code;
        if( pHRD->getSubPicCpbParamsPresentFlag() || sei.m_rapCpbParamsPresentFlag )
        {
          sei_read_code( pDecodedMessageOutputStream, ( pHRD->getInitialCpbRemovalDelayLengthMinus1() + 1 ) , code, nalOrVcl?"vcl_initial_alt_cpb_removal_delay":"nal_initial_alt_cpb_removal_delay" );
          sei.m_initialAltCpbRemovalDelay[i][nalOrVcl] = code;
          sei_read_code( pDecodedMessageOutputStream, ( pHRD->getInitialCpbRemovalDelayLengthMinus1() + 1 ) , code, nalOrVcl?"vcl_initial_alt_cpb_removal_offset":"nal_initial_alt_cpb_removal_offset" );
          sei.m_initialAltCpbRemovalDelayOffset[i][nalOrVcl] = code;
        }
      }
    }
  }

#if P0138_USE_ALT_CPB_PARAMS_FLAG
  sei.m_useAltCpbParamsFlag = false;
  sei.m_useAltCpbParamsFlagPresent = false;
  if (xPayloadExtensionPresent())
  {
    sei_read_flag( pDecodedMessageOutputStream, code, "use_alt_cpb_params_flag");
    sei.m_useAltCpbParamsFlag = code;
    sei.m_useAltCpbParamsFlagPresent = true;
  }
#endif
}

#if SVC_EXTENSION
Void SEIReader::xParseSEIPictureTiming(SEIPictureTiming& sei, UInt payloadSize, const TComSPS *sps, const SEIScalableNesting* nestingSei, const SEIBspNesting* bspNestingSei, const TComVPS *vps, std::ostream *pDecodedMessageOutputStream)
#else
Void SEIReader::xParseSEIPictureTiming(SEIPictureTiming& sei, UInt payloadSize, const TComSPS *sps, std::ostream *pDecodedMessageOutputStream)
#endif
{
  Int i;
  UInt code;

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
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);

  if( vui->getFrameFieldInfoPresentFlag() )
  {
    sei_read_code( pDecodedMessageOutputStream, 4, code, "pic_struct" );             sei.m_picStruct            = code;
    sei_read_code( pDecodedMessageOutputStream, 2, code, "source_scan_type" );       sei.m_sourceScanType       = code;
    sei_read_flag( pDecodedMessageOutputStream,    code, "duplicate_flag" );         sei.m_duplicateFlag        = (code == 1);
  }

  if( hrd->getCpbDpbDelaysPresentFlag())
  {
    sei_read_code( pDecodedMessageOutputStream, ( hrd->getCpbRemovalDelayLengthMinus1() + 1 ), code, "au_cpb_removal_delay_minus1" );
    sei.m_auCpbRemovalDelay = code + 1;
    sei_read_code( pDecodedMessageOutputStream, ( hrd->getDpbOutputDelayLengthMinus1() + 1 ), code, "pic_dpb_output_delay" );
    sei.m_picDpbOutputDelay = code;

    if(hrd->getSubPicCpbParamsPresentFlag())
    {
      sei_read_code( pDecodedMessageOutputStream, hrd->getDpbOutputDelayDuLengthMinus1()+1, code, "pic_dpb_output_du_delay" );
      sei.m_picDpbOutputDuDelay = code;
    }

    if( hrd->getSubPicCpbParamsPresentFlag() && hrd->getSubPicCpbParamsInPicTimingSEIFlag() )
    {
      sei_read_uvlc( pDecodedMessageOutputStream, code, "num_decoding_units_minus1");
      sei.m_numDecodingUnitsMinus1 = code;
      sei_read_flag( pDecodedMessageOutputStream, code, "du_common_cpb_removal_delay_flag" );
      sei.m_duCommonCpbRemovalDelayFlag = code;
      if( sei.m_duCommonCpbRemovalDelayFlag )
      {
        sei_read_code( pDecodedMessageOutputStream, ( hrd->getDuCpbRemovalDelayLengthMinus1() + 1 ), code, "du_common_cpb_removal_delay_increment_minus1" );
        sei.m_duCommonCpbRemovalDelayMinus1 = code;
      }
      sei.m_numNalusInDuMinus1.resize(sei.m_numDecodingUnitsMinus1 + 1 );
      sei.m_duCpbRemovalDelayMinus1.resize( sei.m_numDecodingUnitsMinus1 + 1 );

      for( i = 0; i <= sei.m_numDecodingUnitsMinus1; i ++ )
      {
        sei_read_uvlc( pDecodedMessageOutputStream, code, "num_nalus_in_du_minus1[i]");
        sei.m_numNalusInDuMinus1[ i ] = code;
        if( ( !sei.m_duCommonCpbRemovalDelayFlag ) && ( i < sei.m_numDecodingUnitsMinus1 ) )
        {
          sei_read_code( pDecodedMessageOutputStream, ( hrd->getDuCpbRemovalDelayLengthMinus1() + 1 ), code, "du_cpb_removal_delay_minus1[i]" );
          sei.m_duCpbRemovalDelayMinus1[ i ] = code;
        }
      }
    }
  }
}

Void SEIReader::xParseSEIRecoveryPoint(SEIRecoveryPoint& sei, UInt payloadSize, std::ostream *pDecodedMessageOutputStream)
{
  Int  iCode;
  UInt uiCode;
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);

  sei_read_svlc( pDecodedMessageOutputStream, iCode,  "recovery_poc_cnt" );      sei.m_recoveryPocCnt     = iCode;
  sei_read_flag( pDecodedMessageOutputStream, uiCode, "exact_matching_flag" );   sei.m_exactMatchingFlag  = uiCode;
  sei_read_flag( pDecodedMessageOutputStream, uiCode, "broken_link_flag" );      sei.m_brokenLinkFlag     = uiCode;
}

Void SEIReader::xParseSEIFramePacking(SEIFramePacking& sei, UInt payloadSize, std::ostream *pDecodedMessageOutputStream)
{
  UInt val;
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);

  sei_read_uvlc( pDecodedMessageOutputStream, val, "frame_packing_arrangement_id" );                 sei.m_arrangementId = val;
  sei_read_flag( pDecodedMessageOutputStream, val, "frame_packing_arrangement_cancel_flag" );        sei.m_arrangementCancelFlag = val;

  if ( !sei.m_arrangementCancelFlag )
  {
    sei_read_code( pDecodedMessageOutputStream, 7, val, "frame_packing_arrangement_type" );          sei.m_arrangementType = val;
    assert((sei.m_arrangementType > 2) && (sei.m_arrangementType < 6) );

    sei_read_flag( pDecodedMessageOutputStream, val, "quincunx_sampling_flag" );                     sei.m_quincunxSamplingFlag = val;

    sei_read_code( pDecodedMessageOutputStream, 6, val, "content_interpretation_type" );             sei.m_contentInterpretationType = val;
    sei_read_flag( pDecodedMessageOutputStream, val, "spatial_flipping_flag" );                      sei.m_spatialFlippingFlag = val;
    sei_read_flag( pDecodedMessageOutputStream, val, "frame0_flipped_flag" );                        sei.m_frame0FlippedFlag = val;
    sei_read_flag( pDecodedMessageOutputStream, val, "field_views_flag" );                           sei.m_fieldViewsFlag = val;
    sei_read_flag( pDecodedMessageOutputStream, val, "current_frame_is_frame0_flag" );               sei.m_currentFrameIsFrame0Flag = val;
    sei_read_flag( pDecodedMessageOutputStream, val, "frame0_self_contained_flag" );                 sei.m_frame0SelfContainedFlag = val;
    sei_read_flag( pDecodedMessageOutputStream, val, "frame1_self_contained_flag" );                 sei.m_frame1SelfContainedFlag = val;

    if ( sei.m_quincunxSamplingFlag == 0 && sei.m_arrangementType != 5)
    {
      sei_read_code( pDecodedMessageOutputStream, 4, val, "frame0_grid_position_x" );                sei.m_frame0GridPositionX = val;
      sei_read_code( pDecodedMessageOutputStream, 4, val, "frame0_grid_position_y" );                sei.m_frame0GridPositionY = val;
      sei_read_code( pDecodedMessageOutputStream, 4, val, "frame1_grid_position_x" );                sei.m_frame1GridPositionX = val;
      sei_read_code( pDecodedMessageOutputStream, 4, val, "frame1_grid_position_y" );                sei.m_frame1GridPositionY = val;
    }

    sei_read_code( pDecodedMessageOutputStream, 8, val, "frame_packing_arrangement_reserved_byte" );   sei.m_arrangementReservedByte = val;
    sei_read_flag( pDecodedMessageOutputStream, val,  "frame_packing_arrangement_persistence_flag" );  sei.m_arrangementPersistenceFlag = (val != 0);
  }
  sei_read_flag( pDecodedMessageOutputStream, val, "upsampled_aspect_ratio_flag" );                  sei.m_upsampledAspectRatio = val;
}

Void SEIReader::xParseSEISegmentedRectFramePacking(SEISegmentedRectFramePacking& sei, UInt payloadSize, std::ostream *pDecodedMessageOutputStream)
{
  UInt val;
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);
  sei_read_flag( pDecodedMessageOutputStream, val,       "segmented_rect_frame_packing_arrangement_cancel_flag" );       sei.m_arrangementCancelFlag            = val;
  if( !sei.m_arrangementCancelFlag )
  {
    sei_read_code( pDecodedMessageOutputStream, 2, val, "segmented_rect_content_interpretation_type" );                sei.m_contentInterpretationType = val;
    sei_read_flag( pDecodedMessageOutputStream, val,     "segmented_rect_frame_packing_arrangement_persistence" );                              sei.m_arrangementPersistenceFlag               = val;
  }
}

Void SEIReader::xParseSEIDisplayOrientation(SEIDisplayOrientation& sei, UInt payloadSize, std::ostream *pDecodedMessageOutputStream)
{
  UInt val;
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);
  sei_read_flag( pDecodedMessageOutputStream, val,       "display_orientation_cancel_flag" );       sei.cancelFlag            = val;
  if( !sei.cancelFlag )
  {
    sei_read_flag( pDecodedMessageOutputStream, val,     "hor_flip" );                              sei.horFlip               = val;
    sei_read_flag( pDecodedMessageOutputStream, val,     "ver_flip" );                              sei.verFlip               = val;
    sei_read_code( pDecodedMessageOutputStream, 16, val, "anticlockwise_rotation" );                sei.anticlockwiseRotation = val;
    sei_read_flag( pDecodedMessageOutputStream, val,     "display_orientation_persistence_flag" );  sei.persistenceFlag       = val;
  }
}

Void SEIReader::xParseSEITemporalLevel0Index(SEITemporalLevel0Index& sei, UInt payloadSize, std::ostream *pDecodedMessageOutputStream)
{
  UInt val;
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);
  sei_read_code( pDecodedMessageOutputStream, 8, val, "temporal_sub_layer_zero_idx" );  sei.tl0Idx = val;
  sei_read_code( pDecodedMessageOutputStream, 8, val, "irap_pic_id" );  sei.rapIdx = val;
}

Void SEIReader::xParseSEIRegionRefreshInfo(SEIGradualDecodingRefreshInfo& sei, UInt payloadSize, std::ostream *pDecodedMessageOutputStream)
{
  UInt val;
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);
  sei_read_flag( pDecodedMessageOutputStream, val, "refreshed_region_flag" ); sei.m_gdrForegroundFlag = val ? 1 : 0;
}

Void SEIReader::xParseSEINoDisplay(SEINoDisplay& sei, UInt payloadSize, std::ostream *pDecodedMessageOutputStream)
{
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);
  sei.m_noDisplay = true;
}

Void SEIReader::xParseSEIToneMappingInfo(SEIToneMappingInfo& sei, UInt payloadSize, std::ostream *pDecodedMessageOutputStream)
{
  Int i;
  UInt val;
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);
  sei_read_uvlc( pDecodedMessageOutputStream, val, "tone_map_id" );                         sei.m_toneMapId = val;
  sei_read_flag( pDecodedMessageOutputStream, val, "tone_map_cancel_flag" );                sei.m_toneMapCancelFlag = val;

  if ( !sei.m_toneMapCancelFlag )
  {
    sei_read_flag( pDecodedMessageOutputStream, val, "tone_map_persistence_flag" );         sei.m_toneMapPersistenceFlag = val;
    sei_read_code( pDecodedMessageOutputStream, 8, val, "coded_data_bit_depth" );           sei.m_codedDataBitDepth = val;
    sei_read_code( pDecodedMessageOutputStream, 8, val, "target_bit_depth" );               sei.m_targetBitDepth = val;
    sei_read_uvlc( pDecodedMessageOutputStream, val, "tone_map_model_id" );                 sei.m_modelId = val;
    switch(sei.m_modelId)
    {
    case 0:
      {
        sei_read_code( pDecodedMessageOutputStream, 32, val, "min_value" );                 sei.m_minValue = val;
        sei_read_code( pDecodedMessageOutputStream, 32, val, "max_value" );                 sei.m_maxValue = val;
        break;
      }
    case 1:
      {
        sei_read_code( pDecodedMessageOutputStream, 32, val, "sigmoid_midpoint" );          sei.m_sigmoidMidpoint = val;
        sei_read_code( pDecodedMessageOutputStream, 32, val, "sigmoid_width" );             sei.m_sigmoidWidth = val;
        break;
      }
    case 2:
      {
        UInt num = 1u << sei.m_targetBitDepth;
        sei.m_startOfCodedInterval.resize(num+1);
        for(i = 0; i < num; i++)
        {
          sei_read_code( pDecodedMessageOutputStream, ((( sei.m_codedDataBitDepth + 7 ) >> 3 ) << 3), val, "start_of_coded_interval[i]" );
          sei.m_startOfCodedInterval[i] = val;
        }
        sei.m_startOfCodedInterval[num] = 1u << sei.m_codedDataBitDepth;
        break;
      }
    case 3:
      {
        sei_read_code( pDecodedMessageOutputStream, 16, val,  "num_pivots" );                       sei.m_numPivots = val;
        sei.m_codedPivotValue.resize(sei.m_numPivots);
        sei.m_targetPivotValue.resize(sei.m_numPivots);
        for(i = 0; i < sei.m_numPivots; i++ )
        {
          sei_read_code( pDecodedMessageOutputStream, ((( sei.m_codedDataBitDepth + 7 ) >> 3 ) << 3), val, "coded_pivot_value[i]" );
          sei.m_codedPivotValue[i] = val;
          sei_read_code( pDecodedMessageOutputStream, ((( sei.m_targetBitDepth + 7 ) >> 3 ) << 3),    val, "target_pivot_value[i]" );
          sei.m_targetPivotValue[i] = val;
        }
        break;
      }
    case 4:
      {
        sei_read_code( pDecodedMessageOutputStream, 8, val, "camera_iso_speed_idc" );                     sei.m_cameraIsoSpeedIdc = val;
        if( sei.m_cameraIsoSpeedIdc == 255) //Extended_ISO
        {
          sei_read_code( pDecodedMessageOutputStream, 32,   val,   "camera_iso_speed_value" );            sei.m_cameraIsoSpeedValue = val;
        }
        sei_read_code( pDecodedMessageOutputStream, 8, val, "exposure_index_idc" );                       sei.m_exposureIndexIdc = val;
        if( sei.m_exposureIndexIdc == 255) //Extended_ISO
        {
          sei_read_code( pDecodedMessageOutputStream, 32,   val,   "exposure_index_value" );              sei.m_exposureIndexValue = val;
        }
        sei_read_flag( pDecodedMessageOutputStream, val, "exposure_compensation_value_sign_flag" );       sei.m_exposureCompensationValueSignFlag = val;
        sei_read_code( pDecodedMessageOutputStream, 16, val, "exposure_compensation_value_numerator" );   sei.m_exposureCompensationValueNumerator = val;
        sei_read_code( pDecodedMessageOutputStream, 16, val, "exposure_compensation_value_denom_idc" );   sei.m_exposureCompensationValueDenomIdc = val;
        sei_read_code( pDecodedMessageOutputStream, 32, val, "ref_screen_luminance_white" );              sei.m_refScreenLuminanceWhite = val;
        sei_read_code( pDecodedMessageOutputStream, 32, val, "extended_range_white_level" );              sei.m_extendedRangeWhiteLevel = val;
        sei_read_code( pDecodedMessageOutputStream, 16, val, "nominal_black_level_code_value" );          sei.m_nominalBlackLevelLumaCodeValue = val;
        sei_read_code( pDecodedMessageOutputStream, 16, val, "nominal_white_level_code_value" );          sei.m_nominalWhiteLevelLumaCodeValue= val;
        sei_read_code( pDecodedMessageOutputStream, 16, val, "extended_white_level_code_value" );         sei.m_extendedWhiteLevelLumaCodeValue = val;
        break;
      }
    default:
      {
        assert(!"Undefined SEIToneMapModelId");
        break;
      }
    }//switch model id
  }// if(!sei.m_toneMapCancelFlag)
}

Void SEIReader::xParseSEISOPDescription(SEISOPDescription &sei, UInt payloadSize, std::ostream *pDecodedMessageOutputStream)
{
  Int iCode;
  UInt uiCode;
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);

  sei_read_uvlc( pDecodedMessageOutputStream, uiCode,           "sop_seq_parameter_set_id"            ); sei.m_sopSeqParameterSetId = uiCode;
  sei_read_uvlc( pDecodedMessageOutputStream, uiCode,           "num_pics_in_sop_minus1"              ); sei.m_numPicsInSopMinus1 = uiCode;
  for (UInt i = 0; i <= sei.m_numPicsInSopMinus1; i++)
  {
    sei_read_code( pDecodedMessageOutputStream, 6, uiCode,                     "sop_vcl_nut[i]" );  sei.m_sopDescVclNaluType[i] = uiCode;
    sei_read_code( pDecodedMessageOutputStream, 3, sei.m_sopDescTemporalId[i], "sop_temporal_id[i]"   );  sei.m_sopDescTemporalId[i] = uiCode;
    if (sei.m_sopDescVclNaluType[i] != NAL_UNIT_CODED_SLICE_IDR_W_RADL && sei.m_sopDescVclNaluType[i] != NAL_UNIT_CODED_SLICE_IDR_N_LP)
    {
      sei_read_uvlc( pDecodedMessageOutputStream, sei.m_sopDescStRpsIdx[i],    "sop_short_term_rps_idx[i]"    ); sei.m_sopDescStRpsIdx[i] = uiCode;
    }
    if (i > 0)
    {
      sei_read_svlc( pDecodedMessageOutputStream, iCode,                       "sop_poc_delta[i]"     ); sei.m_sopDescPocDelta[i] = iCode;
    }
  }
}

#if LAYERS_NOT_PRESENT_SEI
Void SEIReader::xParseSEIScalableNesting(SEIScalableNesting& sei, const NalUnitType nalUnitType, UInt payloadSize, const TComVPS *vps, const TComSPS *sps, std::ostream *pDecodedMessageOutputStream)
#else
Void SEIReader::xParseSEIScalableNesting(SEIScalableNesting& sei, const NalUnitType nalUnitType, UInt payloadSize, const TComSPS *sps, std::ostream *pDecodedMessageOutputStream)
#endif
{
  UInt uiCode;
  SEIMessages seis;
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);

  sei_read_flag( pDecodedMessageOutputStream, uiCode,            "bitstream_subset_flag"         ); sei.m_bitStreamSubsetFlag = uiCode;
  sei_read_flag( pDecodedMessageOutputStream, uiCode,            "nesting_op_flag"               ); sei.m_nestingOpFlag = uiCode;
  if (sei.m_nestingOpFlag)
  {
    sei_read_flag( pDecodedMessageOutputStream, uiCode,            "default_op_flag"               ); sei.m_defaultOpFlag = uiCode;
    sei_read_uvlc( pDecodedMessageOutputStream, uiCode,            "nesting_num_ops_minus1"        ); sei.m_nestingNumOpsMinus1 = uiCode;
    for (UInt i = sei.m_defaultOpFlag; i <= sei.m_nestingNumOpsMinus1; i++)
    {
      sei_read_code( pDecodedMessageOutputStream, 3,        uiCode,  "nesting_max_temporal_id_plus1[i]"   ); sei.m_nestingMaxTemporalIdPlus1[i] = uiCode;
      sei_read_uvlc( pDecodedMessageOutputStream, uiCode,            "nesting_op_idx[i]"                  ); sei.m_nestingOpIdx[i] = uiCode;
    }
  }
  else
  {
    sei_read_flag( pDecodedMessageOutputStream, uiCode,            "all_layers_flag"               ); sei.m_allLayersFlag       = uiCode;
    if (!sei.m_allLayersFlag)
    {
      sei_read_code( pDecodedMessageOutputStream, 3,        uiCode,  "nesting_no_op_max_temporal_id_plus1"  ); sei.m_nestingNoOpMaxTemporalIdPlus1 = uiCode;
      sei_read_uvlc( pDecodedMessageOutputStream, uiCode,            "nesting_num_layers_minus1"            ); sei.m_nestingNumLayersMinus1        = uiCode;
      for (UInt i = 0; i <= sei.m_nestingNumLayersMinus1; i++)
      {
        sei_read_code( pDecodedMessageOutputStream, 6,           uiCode,     "nesting_layer_id[i]"      ); sei.m_nestingLayerId[i]   = uiCode;
      }
    }
  }

  // byte alignment
  while ( m_pcBitstream->getNumBitsRead() % 8 != 0 )
  {
    UInt code;
    sei_read_flag( pDecodedMessageOutputStream, code, "nesting_zero_bit" );
  }

  // read nested SEI messages
  do
  {
#if O0164_MULTI_LAYER_HRD
#if LAYERS_NOT_PRESENT_SEI
    xReadSEImessage(sei.m_nestedSEIs, nalUnitType, vps, sps, pDecodedMessageOutputStream, &sei);
#else
    xReadSEImessage(sei.m_nestedSEIs, nalUnitType, sps, pDecodedMessageOutputStream, &sei);
#endif
#else
#if LAYERS_NOT_PRESENT_SEI
    xReadSEImessage(sei.m_nestedSEIs, nalUnitType, vps, sps, pDecodedMessageOutputStream);
#else
    xReadSEImessage(sei.m_nestedSEIs, nalUnitType, sps, pDecodedMessageOutputStream);
#endif
#endif
  } while (m_pcBitstream->getNumBitsLeft() > 8);

  if (pDecodedMessageOutputStream)
  {
    (*pDecodedMessageOutputStream) << "End of scalable nesting SEI message\n";
  }
}

Void SEIReader::xParseSEITempMotionConstraintsTileSets(SEITempMotionConstrainedTileSets& sei, UInt payloadSize, std::ostream *pDecodedMessageOutputStream)
{
  UInt code;
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);
  sei_read_flag( pDecodedMessageOutputStream, code, "mc_all_tiles_exact_sample_value_match_flag");  sei.m_mc_all_tiles_exact_sample_value_match_flag = (code != 0);
  sei_read_flag( pDecodedMessageOutputStream, code, "each_tile_one_tile_set_flag");                 sei.m_each_tile_one_tile_set_flag                = (code != 0);

  if(!sei.m_each_tile_one_tile_set_flag)
  {
    sei_read_flag( pDecodedMessageOutputStream, code, "limited_tile_set_display_flag");  sei.m_limited_tile_set_display_flag = (code != 0);
    sei_read_uvlc( pDecodedMessageOutputStream, code, "num_sets_in_message_minus1");     sei.setNumberOfTileSets(code + 1);

    if(sei.getNumberOfTileSets() != 0)
    {
      for(Int i = 0; i < sei.getNumberOfTileSets(); i++)
      {
        sei_read_uvlc( pDecodedMessageOutputStream, code, "mcts_id");  sei.tileSetData(i).m_mcts_id = code;

        if(sei.m_limited_tile_set_display_flag)
        {
          sei_read_flag( pDecodedMessageOutputStream, code, "display_tile_set_flag");  sei.tileSetData(i).m_display_tile_set_flag = (code != 1);
        }

        sei_read_uvlc( pDecodedMessageOutputStream, code, "num_tile_rects_in_set_minus1");  sei.tileSetData(i).setNumberOfTileRects(code + 1);

        for(Int j=0; j<sei.tileSetData(i).getNumberOfTileRects(); j++)
        {
          sei_read_uvlc( pDecodedMessageOutputStream, code, "top_left_tile_index");      sei.tileSetData(i).topLeftTileIndex(j)     = code;
          sei_read_uvlc( pDecodedMessageOutputStream, code, "bottom_right_tile_index");  sei.tileSetData(i).bottomRightTileIndex(j) = code;
        }

        if(!sei.m_mc_all_tiles_exact_sample_value_match_flag)
        {
          sei_read_flag( pDecodedMessageOutputStream, code, "exact_sample_value_match_flag");   sei.tileSetData(i).m_exact_sample_value_match_flag    = (code != 0);
        }
        sei_read_flag( pDecodedMessageOutputStream, code, "mcts_tier_level_idc_present_flag");  sei.tileSetData(i).m_mcts_tier_level_idc_present_flag = (code != 0);

        if(sei.tileSetData(i).m_mcts_tier_level_idc_present_flag)
        {
          sei_read_flag( pDecodedMessageOutputStream, code,    "mcts_tier_flag"); sei.tileSetData(i).m_mcts_tier_flag = (code != 0);
          sei_read_code( pDecodedMessageOutputStream, 8, code, "mcts_level_idc"); sei.tileSetData(i).m_mcts_level_idc =  code;
        }
      }
    }
  }
  else
  {
    sei_read_flag( pDecodedMessageOutputStream, code, "max_mcs_tier_level_idc_present_flag");  sei.m_max_mcs_tier_level_idc_present_flag = code;
    if(sei.m_max_mcs_tier_level_idc_present_flag)
    {
      sei_read_flag( pDecodedMessageOutputStream, code, "max_mcts_tier_flag");  sei.m_max_mcts_tier_flag = code;
      sei_read_code( pDecodedMessageOutputStream, 8, code, "max_mcts_level_idc"); sei.m_max_mcts_level_idc = code;
    }
  }
}

Void SEIReader::xParseSEITimeCode(SEITimeCode& sei, UInt payloadSize, std::ostream *pDecodedMessageOutputStream)
{
  UInt code;
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);
  sei_read_code( pDecodedMessageOutputStream, 2, code, "num_clock_ts"); sei.numClockTs = code;
  for(Int i = 0; i < sei.numClockTs; i++)
  {
    TComSEITimeSet currentTimeSet;
    sei_read_flag( pDecodedMessageOutputStream, code, "clock_time_stamp_flag[i]"); currentTimeSet.clockTimeStampFlag = code;
    if(currentTimeSet.clockTimeStampFlag)
    {
      sei_read_flag( pDecodedMessageOutputStream, code, "nuit_field_based_flag"); currentTimeSet.numUnitFieldBasedFlag = code;
      sei_read_code( pDecodedMessageOutputStream, 5, code, "counting_type"); currentTimeSet.countingType = code;
      sei_read_flag( pDecodedMessageOutputStream, code, "full_timestamp_flag"); currentTimeSet.fullTimeStampFlag = code;
      sei_read_flag( pDecodedMessageOutputStream, code, "discontinuity_flag"); currentTimeSet.discontinuityFlag = code;
      sei_read_flag( pDecodedMessageOutputStream, code, "cnt_dropped_flag"); currentTimeSet.cntDroppedFlag = code;
      sei_read_code( pDecodedMessageOutputStream, 9, code, "n_frames"); currentTimeSet.numberOfFrames = code;
      if(currentTimeSet.fullTimeStampFlag)
      {
        sei_read_code( pDecodedMessageOutputStream, 6, code, "seconds_value"); currentTimeSet.secondsValue = code;
        sei_read_code( pDecodedMessageOutputStream, 6, code, "minutes_value"); currentTimeSet.minutesValue = code;
        sei_read_code( pDecodedMessageOutputStream, 5, code, "hours_value"); currentTimeSet.hoursValue = code;
      }
      else
      {
        sei_read_flag( pDecodedMessageOutputStream, code, "seconds_flag"); currentTimeSet.secondsFlag = code;
        if(currentTimeSet.secondsFlag)
        {
          sei_read_code( pDecodedMessageOutputStream, 6, code, "seconds_value"); currentTimeSet.secondsValue = code;
          sei_read_flag( pDecodedMessageOutputStream, code, "minutes_flag"); currentTimeSet.minutesFlag = code;
          if(currentTimeSet.minutesFlag)
          {
            sei_read_code( pDecodedMessageOutputStream, 6, code, "minutes_value"); currentTimeSet.minutesValue = code;
            sei_read_flag( pDecodedMessageOutputStream, code, "hours_flag"); currentTimeSet.hoursFlag = code;
            if(currentTimeSet.hoursFlag)
            {
              sei_read_code( pDecodedMessageOutputStream, 5, code, "hours_value"); currentTimeSet.hoursValue = code;
            }
          }
        }
      }
      sei_read_code( pDecodedMessageOutputStream, 5, code, "time_offset_length"); currentTimeSet.timeOffsetLength = code;
      if(currentTimeSet.timeOffsetLength > 0)
      {
        sei_read_code( pDecodedMessageOutputStream, currentTimeSet.timeOffsetLength, code, "time_offset_value");
        if((code & (1 << (currentTimeSet.timeOffsetLength-1))) == 0)
        {
          currentTimeSet.timeOffsetValue = code;
        }
        else
        {
          code &= (1<< (currentTimeSet.timeOffsetLength-1)) - 1;
          currentTimeSet.timeOffsetValue = ~code + 1;
        }
      }
    }
    sei.timeSetArray[i] = currentTimeSet;
  }
}

Void SEIReader::xParseSEIChromaResamplingFilterHint(SEIChromaResamplingFilterHint& sei, UInt payloadSize, std::ostream *pDecodedMessageOutputStream)
{
  UInt uiCode;
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);

  sei_read_code( pDecodedMessageOutputStream, 8, uiCode, "ver_chroma_filter_idc"); sei.m_verChromaFilterIdc = uiCode;
  sei_read_code( pDecodedMessageOutputStream, 8, uiCode, "hor_chroma_filter_idc"); sei.m_horChromaFilterIdc = uiCode;
  sei_read_flag( pDecodedMessageOutputStream, uiCode, "ver_filtering_field_processing_flag"); sei.m_verFilteringFieldProcessingFlag = uiCode;
  if(sei.m_verChromaFilterIdc == 1 || sei.m_horChromaFilterIdc == 1)
  {
    sei_read_uvlc( pDecodedMessageOutputStream, uiCode, "target_format_idc"); sei.m_targetFormatIdc = uiCode;
    if(sei.m_verChromaFilterIdc == 1)
    {
      UInt numVerticalFilters;
      sei_read_uvlc( pDecodedMessageOutputStream, numVerticalFilters, "num_vertical_filters"); sei.m_verFilterCoeff.resize(numVerticalFilters);
      if(numVerticalFilters > 0)
      {
        for(Int i = 0; i < numVerticalFilters; i++)
        {
          UInt verTapLengthMinus1;
          sei_read_uvlc( pDecodedMessageOutputStream, verTapLengthMinus1, "ver_tap_length_minus_1"); sei.m_verFilterCoeff[i].resize(verTapLengthMinus1+1);
          for(Int j = 0; j < (verTapLengthMinus1 + 1); j++)
          {
            sei_read_svlc( pDecodedMessageOutputStream, sei.m_verFilterCoeff[i][j], "ver_filter_coeff");
          }
        }
      }
    }
    if(sei.m_horChromaFilterIdc == 1)
    {
      UInt numHorizontalFilters;
      sei_read_uvlc( pDecodedMessageOutputStream, numHorizontalFilters, "num_horizontal_filters"); sei.m_horFilterCoeff.resize(numHorizontalFilters);
      if(numHorizontalFilters  > 0)
      {
        for(Int i = 0; i < numHorizontalFilters; i++)
        {
          UInt horTapLengthMinus1;
          sei_read_uvlc( pDecodedMessageOutputStream, horTapLengthMinus1, "hor_tap_length_minus_1"); sei.m_horFilterCoeff[i].resize(horTapLengthMinus1+1);
          for(Int j = 0; j < (horTapLengthMinus1 + 1); j++)
          {
            sei_read_svlc( pDecodedMessageOutputStream, sei.m_horFilterCoeff[i][j], "hor_filter_coeff");
          }
        }
      }
    }
  }
}

Void SEIReader::xParseSEIKneeFunctionInfo(SEIKneeFunctionInfo& sei, UInt payloadSize, std::ostream *pDecodedMessageOutputStream)
{
  Int i;
  UInt val;
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);

  sei_read_uvlc( pDecodedMessageOutputStream, val, "knee_function_id" );                   sei.m_kneeId = val;
  sei_read_flag( pDecodedMessageOutputStream, val, "knee_function_cancel_flag" );          sei.m_kneeCancelFlag = val;
  if ( !sei.m_kneeCancelFlag )
  {
    sei_read_flag( pDecodedMessageOutputStream, val, "knee_function_persistence_flag" );   sei.m_kneePersistenceFlag = val;
    sei_read_code( pDecodedMessageOutputStream, 32, val, "input_d_range" );                sei.m_kneeInputDrange = val;
    sei_read_code( pDecodedMessageOutputStream, 32, val, "input_disp_luminance" );         sei.m_kneeInputDispLuminance = val;
    sei_read_code( pDecodedMessageOutputStream, 32, val, "output_d_range" );               sei.m_kneeOutputDrange = val;
    sei_read_code( pDecodedMessageOutputStream, 32, val, "output_disp_luminance" );        sei.m_kneeOutputDispLuminance = val;
    sei_read_uvlc( pDecodedMessageOutputStream, val, "num_knee_points_minus1" );           sei.m_kneeNumKneePointsMinus1 = val;
    assert( sei.m_kneeNumKneePointsMinus1 > 0 );
    sei.m_kneeInputKneePoint.resize(sei.m_kneeNumKneePointsMinus1+1);
    sei.m_kneeOutputKneePoint.resize(sei.m_kneeNumKneePointsMinus1+1);
    for(i = 0; i <= sei.m_kneeNumKneePointsMinus1; i++ )
    {
      sei_read_code( pDecodedMessageOutputStream, 10, val, "input_knee_point" );           sei.m_kneeInputKneePoint[i] = val;
      sei_read_code( pDecodedMessageOutputStream, 10, val, "output_knee_point" );          sei.m_kneeOutputKneePoint[i] = val;
    }
  }
}

Void SEIReader::xParseSEIColourRemappingInfo(SEIColourRemappingInfo& sei, UInt payloadSize, std::ostream *pDecodedMessageOutputStream)
{
  UInt  uiVal;
  Int   iVal;
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);

  sei_read_uvlc( pDecodedMessageOutputStream, uiVal, "colour_remap_id" );          sei.m_colourRemapId = uiVal;
  sei_read_flag( pDecodedMessageOutputStream, uiVal, "colour_remap_cancel_flag" ); sei.m_colourRemapCancelFlag = uiVal;
  if( !sei.m_colourRemapCancelFlag ) 
  {
    sei_read_flag( pDecodedMessageOutputStream, uiVal, "colour_remap_persistence_flag" );                sei.m_colourRemapPersistenceFlag = uiVal;
    sei_read_flag( pDecodedMessageOutputStream, uiVal, "colour_remap_video_signal_info_present_flag" );  sei.m_colourRemapVideoSignalInfoPresentFlag = uiVal;
    if ( sei.m_colourRemapVideoSignalInfoPresentFlag )
    {
      sei_read_flag( pDecodedMessageOutputStream, uiVal,    "colour_remap_full_range_flag" );            sei.m_colourRemapFullRangeFlag = uiVal;
      sei_read_code( pDecodedMessageOutputStream, 8, uiVal, "colour_remap_primaries" );                  sei.m_colourRemapPrimaries = uiVal;
      sei_read_code( pDecodedMessageOutputStream, 8, uiVal, "colour_remap_transfer_function" );          sei.m_colourRemapTransferFunction = uiVal;
      sei_read_code( pDecodedMessageOutputStream, 8, uiVal, "colour_remap_matrix_coefficients" );        sei.m_colourRemapMatrixCoefficients = uiVal;
    }
    sei_read_code( pDecodedMessageOutputStream, 8, uiVal, "colour_remap_input_bit_depth" );              sei.m_colourRemapInputBitDepth = uiVal;
    sei_read_code( pDecodedMessageOutputStream, 8, uiVal, "colour_remap_bit_depth" );                    sei.m_colourRemapBitDepth = uiVal;
  
    for( Int c=0 ; c<3 ; c++ )
    {
      sei_read_code( pDecodedMessageOutputStream, 8, uiVal, "pre_lut_num_val_minus1[c]" ); sei.m_preLutNumValMinus1[c] = (uiVal==0) ? 1 : uiVal;
      sei.m_preLut[c].resize(sei.m_preLutNumValMinus1[c]+1);
      if( uiVal> 0 )
      {
        for ( Int i=0 ; i<=sei.m_preLutNumValMinus1[c] ; i++ )
        {
          sei_read_code( pDecodedMessageOutputStream, (( sei.m_colourRemapInputBitDepth   + 7 ) >> 3 ) << 3, uiVal, "pre_lut_coded_value[c][i]" );  sei.m_preLut[c][i].codedValue  = uiVal;
          sei_read_code( pDecodedMessageOutputStream, (( sei.m_colourRemapBitDepth + 7 ) >> 3 ) << 3, uiVal, "pre_lut_target_value[c][i]" ); sei.m_preLut[c][i].targetValue = uiVal;
        }
      }
      else // pre_lut_num_val_minus1[c] == 0
      {
        sei.m_preLut[c][0].codedValue  = 0;
        sei.m_preLut[c][0].targetValue = 0;
        sei.m_preLut[c][1].codedValue  = (1 << sei.m_colourRemapInputBitDepth) - 1 ;
        sei.m_preLut[c][1].targetValue = (1 << sei.m_colourRemapBitDepth) - 1 ;
      }
    }

    sei_read_flag( pDecodedMessageOutputStream, uiVal,      "colour_remap_matrix_present_flag" ); sei.m_colourRemapMatrixPresentFlag = uiVal;
    if( sei.m_colourRemapMatrixPresentFlag )
    {
      sei_read_code( pDecodedMessageOutputStream, 4, uiVal, "log2_matrix_denom" ); sei.m_log2MatrixDenom = uiVal;
      for ( Int c=0 ; c<3 ; c++ )
      {
        for ( Int i=0 ; i<3 ; i++ )
        {
          sei_read_svlc( pDecodedMessageOutputStream, iVal, "colour_remap_coeffs[c][i]" ); sei.m_colourRemapCoeffs[c][i] = iVal;
        }
      }
    }
    else // setting default matrix (I3)
    {
      sei.m_log2MatrixDenom = 10;
      for ( Int c=0 ; c<3 ; c++ )
      {
        for ( Int i=0 ; i<3 ; i++ )
        {
          sei.m_colourRemapCoeffs[c][i] = (c==i) << sei.m_log2MatrixDenom;
        }
      }
    }
    for( Int c=0 ; c<3 ; c++ )
    {
      sei_read_code( pDecodedMessageOutputStream, 8, uiVal, "post_lut_num_val_minus1[c]" ); sei.m_postLutNumValMinus1[c] = (uiVal==0) ? 1 : uiVal;
      sei.m_postLut[c].resize(sei.m_postLutNumValMinus1[c]+1);
      if( uiVal > 0 )
      {
        for ( Int i=0 ; i<=sei.m_postLutNumValMinus1[c] ; i++ )
        {
          sei_read_code( pDecodedMessageOutputStream, (( sei.m_colourRemapBitDepth + 7 ) >> 3 ) << 3, uiVal, "post_lut_coded_value[c][i]" );  sei.m_postLut[c][i].codedValue = uiVal;
          sei_read_code( pDecodedMessageOutputStream, (( sei.m_colourRemapBitDepth + 7 ) >> 3 ) << 3, uiVal, "post_lut_target_value[c][i]" ); sei.m_postLut[c][i].targetValue = uiVal;
        }
      }
      else
      {
        sei.m_postLut[c][0].codedValue  = 0;
        sei.m_postLut[c][0].targetValue = 0;
        sei.m_postLut[c][1].targetValue = (1 << sei.m_colourRemapBitDepth) - 1;
        sei.m_postLut[c][1].codedValue  = (1 << sei.m_colourRemapBitDepth) - 1;
      }
    }
  }
}

Void SEIReader::xParseSEIMasteringDisplayColourVolume(SEIMasteringDisplayColourVolume& sei, UInt payloadSize, std::ostream *pDecodedMessageOutputStream)
{
  UInt code;
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);

  sei_read_code( pDecodedMessageOutputStream, 16, code, "display_primaries_x[0]" ); sei.values.primaries[0][0] = code;
  sei_read_code( pDecodedMessageOutputStream, 16, code, "display_primaries_y[0]" ); sei.values.primaries[0][1] = code;

  sei_read_code( pDecodedMessageOutputStream, 16, code, "display_primaries_x[1]" ); sei.values.primaries[1][0] = code;
  sei_read_code( pDecodedMessageOutputStream, 16, code, "display_primaries_y[1]" ); sei.values.primaries[1][1] = code;

  sei_read_code( pDecodedMessageOutputStream, 16, code, "display_primaries_x[2]" ); sei.values.primaries[2][0] = code;
  sei_read_code( pDecodedMessageOutputStream, 16, code, "display_primaries_y[2]" ); sei.values.primaries[2][1] = code;


  sei_read_code( pDecodedMessageOutputStream, 16, code, "white_point_x" ); sei.values.whitePoint[0] = code;
  sei_read_code( pDecodedMessageOutputStream, 16, code, "white_point_y" ); sei.values.whitePoint[1] = code;

  sei_read_code( pDecodedMessageOutputStream, 32, code, "max_display_mastering_luminance" ); sei.values.maxLuminance = code;
  sei_read_code( pDecodedMessageOutputStream, 32, code, "min_display_mastering_luminance" ); sei.values.minLuminance = code;
}

#if U0033_ALTERNATIVE_TRANSFER_CHARACTERISTICS_SEI
Void SEIReader::xParseSEIAlternativeTransferCharacteristics(SEIAlternativeTransferCharacteristics& sei, UInt payloadSize, ostream* pDecodedMessageOutputStream)
{
  UInt code;
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);

  sei_read_code(pDecodedMessageOutputStream, 8, code, "preferred_transfer_characteristics"); sei.m_preferredTransferCharacteristics = code;
}
#endif

Void SEIReader::xParseSEIGreenMetadataInfo(SEIGreenMetadataInfo& sei, UInt payloadSize, ostream* pDecodedMessageOutputStream)
{
  UInt code;
  output_sei_message_header(sei, pDecodedMessageOutputStream, payloadSize);
  
  sei_read_code(pDecodedMessageOutputStream, 8, code, "green_metadata_type");
  sei.m_greenMetadataType = code;
  
  sei_read_code(pDecodedMessageOutputStream, 8, code, "xsd_metric_type");
  sei.m_xsdMetricType = code;
  
  sei_read_code(pDecodedMessageOutputStream, 16, code, "xsd_metric_value");
  sei.m_xsdMetricValue = code;
}

#if SVC_EXTENSION
#if LAYERS_NOT_PRESENT_SEI
Void SEIReader::xParseSEILayersNotPresent(SEILayersNotPresent &sei, UInt payloadSize, const TComVPS *vps, std::ostream *pDecodedMessageOutputStream)
{
  UInt uiCode;
  UInt i = 0;

  sei_read_uvlc( pDecodedMessageOutputStream, uiCode,           "lp_sei_active_vps_id" ); sei.m_activeVpsId = uiCode;
  assert(vps->getVPSId() == sei.m_activeVpsId);
  sei.m_vpsMaxLayers = vps->getMaxLayers();
  for (; i < sei.m_vpsMaxLayers; i++)
  {
    sei_read_flag( pDecodedMessageOutputStream, uiCode,         "layer_not_present_flag"   ); sei.m_layerNotPresentFlag[i] = uiCode ? true : false;
  }
  for (; i < MAX_LAYERS; i++)
  {
    sei.m_layerNotPresentFlag[i] = false;
  }
}
#endif

#if N0383_IL_CONSTRAINED_TILE_SETS_SEI
Void SEIReader::xParseSEIInterLayerConstrainedTileSets (SEIInterLayerConstrainedTileSets &sei, UInt payloadSize, std::ostream *pDecodedMessageOutputStream)
{
  UInt uiCode;

  sei_read_flag( pDecodedMessageOutputStream, uiCode, "il_all_tiles_exact_sample_value_match_flag"   ); sei.m_ilAllTilesExactSampleValueMatchFlag = uiCode;
  sei_read_flag( pDecodedMessageOutputStream, uiCode, "il_one_tile_per_tile_set_flag"                ); sei.m_ilOneTilePerTileSetFlag = uiCode;
  if( !sei.m_ilOneTilePerTileSetFlag )
  {
    sei_read_uvlc( pDecodedMessageOutputStream, uiCode, "il_num_sets_in_message_minus1"                ); sei.m_ilNumSetsInMessageMinus1 = uiCode;
    if( sei.m_ilNumSetsInMessageMinus1 )
    {
      sei_read_flag( pDecodedMessageOutputStream, uiCode, "skipped_tile_set_present_flag"                ); sei.m_skippedTileSetPresentFlag = uiCode;
    }
    else
    {
      sei.m_skippedTileSetPresentFlag = false;
    }
    UInt numSignificantSets = sei.m_ilNumSetsInMessageMinus1 - (sei.m_skippedTileSetPresentFlag ? 1 : 0) + 1;
    for( UInt i = 0; i < numSignificantSets; i++ )
    {
      sei_read_uvlc( pDecodedMessageOutputStream, uiCode, "ilcts_id"                                     ); sei.m_ilctsId[i] = uiCode;
      sei_read_uvlc( pDecodedMessageOutputStream, uiCode, "il_num_tile_rects_in_set_minus1"              ) ;sei.m_ilNumTileRectsInSetMinus1[i] = uiCode;
      for( UInt j = 0; j <= sei.m_ilNumTileRectsInSetMinus1[i]; j++ )
      {
        sei_read_uvlc( pDecodedMessageOutputStream, uiCode, "il_top_left_tile_index"                       ); sei.m_ilTopLeftTileIndex[i][j] = uiCode;
        sei_read_uvlc( pDecodedMessageOutputStream, uiCode, "il_bottom_right_tile_index"                   ); sei.m_ilBottomRightTileIndex[i][j] = uiCode;
      }
      sei_read_code( pDecodedMessageOutputStream, 2, uiCode, "ilc_idc"                                   ); sei.m_ilcIdc[i] = uiCode;
      if( sei.m_ilAllTilesExactSampleValueMatchFlag )
      {
        sei_read_flag( pDecodedMessageOutputStream, uiCode, "il_exact_sample_value_match_flag"             ); sei.m_ilExactSampleValueMatchFlag[i] = uiCode;
      }
    }
  }
  else
  {
    sei_read_code( pDecodedMessageOutputStream, 2, uiCode, "all_tiles_ilc_idc"                         ); sei.m_allTilesIlcIdc = uiCode;
  }
}
#endif

#if SUB_BITSTREAM_PROPERTY_SEI
Void SEIReader::xParseSEISubBitstreamProperty(SEISubBitstreamProperty &sei, const TComVPS *vps, std::ostream *pDecodedMessageOutputStream)
{
  UInt uiCode;
  sei_read_code( pDecodedMessageOutputStream, 4, uiCode, "active_vps_id" );                      sei.m_activeVpsId = uiCode;
  sei_read_uvlc( pDecodedMessageOutputStream,    uiCode, "num_additional_sub_streams_minus1" );  sei.m_numAdditionalSubStreams = uiCode + 1;

  for( Int i = 0; i < sei.m_numAdditionalSubStreams; i++ )
  {
    sei_read_code( pDecodedMessageOutputStream,  2, uiCode, "sub_bitstream_mode[i]"           ); sei.m_subBitstreamMode[i] = uiCode;
    sei_read_uvlc( pDecodedMessageOutputStream,     uiCode, "output_layer_set_idx_to_vps[i]"  );

    // The value of output_layer_set_idx_to_vps[ i ]  shall be in the range of 0 to NumOutputLayerSets ? 1, inclusive.
    assert(uiCode > 0 && uiCode <= vps->getNumOutputLayerSets()-1);

    sei.m_outputLayerSetIdxToVps[i] = uiCode;

    sei_read_code( pDecodedMessageOutputStream,  3, uiCode, "highest_sub_layer_id[i]"         ); sei.m_highestSublayerId[i] = uiCode;
    sei_read_code( pDecodedMessageOutputStream, 16, uiCode, "avg_bit_rate[i]"                 ); sei.m_avgBitRate[i] = uiCode;
    sei_read_code( pDecodedMessageOutputStream, 16, uiCode, "max_bit_rate[i]"                 ); sei.m_maxBitRate[i] = uiCode;
  }  
}
#endif

#if O0164_MULTI_LAYER_HRD
#if LAYERS_NOT_PRESENT_SEI
Void SEIReader::xParseSEIBspNesting(SEIBspNesting &sei, const NalUnitType nalUnitType, const TComVPS *vps, const TComSPS *sps, const SEIScalableNesting &nestingSei, std::ostream *pDecodedMessageOutputStream)
#else
Void SEIReader::xParseSEIBspNesting(SEIBspNesting &sei, const NalUnitType nalUnitType, const TComSPS *sps, const SEIScalableNesting &nestingSei, std::ostream *pDecodedMessageOutputStream)
#endif
{
  UInt uiCode;
  sei_read_uvlc( pDecodedMessageOutputStream, uiCode, "bsp_idx" ); sei.m_bspIdx = uiCode;

  // byte alignment
  while ( m_pcBitstream->getNumBitsRead() % 8 != 0 )
  {
    UInt code;
    sei_read_flag( pDecodedMessageOutputStream, code, "bsp_nesting_zero_bit" );
  }

  sei.m_callerOwnsSEIs = false;

  // read nested SEI messages
  Int numSeiMessages = 0;
  sei_read_uvlc( pDecodedMessageOutputStream, uiCode, "num_seis_in_bsp_minus1" );  assert( uiCode <= MAX_SEIS_IN_BSP_NESTING );
  numSeiMessages = uiCode;
  for(Int i = 0; i < numSeiMessages; i++)
  {
    xReadSEImessage(sei.m_nestedSEIs, nalUnitType, vps, sps, pDecodedMessageOutputStream, &nestingSei, &sei);
  }
}

Void SEIReader::xParseSEIBspInitialArrivalTime(SEIBspInitialArrivalTime &sei, const TComVPS *vps, const TComSPS *sps, const SEIScalableNesting &nestingSei, const SEIBspNesting &bspNestingSei, std::ostream *pDecodedMessageOutputStream)
{
  assert(vps->getVpsVuiPresentFlag());

  UInt uiCode;
  Int psIdx         = bspNestingSei.m_seiPartitioningSchemeIdx;
  Int seiOlsIdx     = bspNestingSei.m_seiOlsIdx;
  Int maxTemporalId = nestingSei.m_nestingMaxTemporalIdPlus1[0];
  Int maxValues     = vps->getNumBspSchedulesMinus1(seiOlsIdx, psIdx, maxTemporalId) + 1;
  std::vector<Int> hrdIdx(0, maxValues);
  std::vector<const TComHRD*> hrdVec;
  std::vector<Int> syntaxElemLen;
  for(Int i = 0; i < maxValues; i++)
  {
    hrdIdx[i] = vps->getBspHrdIdx( seiOlsIdx, psIdx, maxTemporalId, i, bspNestingSei.m_bspIdx);
    hrdVec[i] = vps->getBspHrd(hrdIdx[i]);
    
    syntaxElemLen[i] = hrdVec[i]->getInitialCpbRemovalDelayLengthMinus1() + 1;
    if ( !(hrdVec[i]->getNalHrdParametersPresentFlag() || hrdVec[i]->getVclHrdParametersPresentFlag()) )
    {
      assert( syntaxElemLen[i] == 24 ); // Default value of init_cpb_removal_delay_length_minus1 is 23
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
      sei_read_code( pDecodedMessageOutputStream, syntaxElemLen[i], uiCode, "nal_initial_arrival_delay[i]" ); sei.m_nalInitialArrivalDelay[i] = uiCode;
    }
  }
  if( hrdVec[0]->getVclHrdParametersPresentFlag() )
  {
    for(UInt i = 0; i < maxValues; i++)
    {
      sei_read_code( pDecodedMessageOutputStream, syntaxElemLen[i], uiCode, "vcl_initial_arrival_delay[i]" ); sei.m_vclInitialArrivalDelay[i] = uiCode;
    }
  }
}

Void SEIReader::xParseHrdParameters(TComHRD *hrd, Bool commonInfPresentFlag, UInt maxNumSubLayersMinus1, std::ostream *pDecodedMessageOutputStream)
{
  UInt  uiCode;
  if( commonInfPresentFlag )
  {
    sei_read_flag( pDecodedMessageOutputStream, uiCode, "nal_hrd_parameters_present_flag" );           hrd->setNalHrdParametersPresentFlag( uiCode == 1 ? true : false );
    sei_read_flag( pDecodedMessageOutputStream, uiCode, "vcl_hrd_parameters_present_flag" );           hrd->setVclHrdParametersPresentFlag( uiCode == 1 ? true : false );
    if( hrd->getNalHrdParametersPresentFlag() || hrd->getVclHrdParametersPresentFlag() )
    {
      sei_read_flag( pDecodedMessageOutputStream, uiCode, "sub_pic_cpb_params_present_flag" );         hrd->setSubPicCpbParamsPresentFlag( uiCode == 1 ? true : false );
      if( hrd->getSubPicCpbParamsPresentFlag() )
      {
        sei_read_code( pDecodedMessageOutputStream, 8, uiCode, "tick_divisor_minus2" );                hrd->setTickDivisorMinus2( uiCode );
        sei_read_code( pDecodedMessageOutputStream, 5, uiCode, "du_cpb_removal_delay_length_minus1" ); hrd->setDuCpbRemovalDelayLengthMinus1( uiCode );
        sei_read_flag( pDecodedMessageOutputStream, uiCode, "sub_pic_cpb_params_in_pic_timing_sei_flag" ); hrd->setSubPicCpbParamsInPicTimingSEIFlag( uiCode == 1 ? true : false );
        sei_read_code( pDecodedMessageOutputStream, 5, uiCode, "dpb_output_delay_du_length_minus1"  ); hrd->setDpbOutputDelayDuLengthMinus1( uiCode );
      }
      sei_read_code( pDecodedMessageOutputStream, 4, uiCode, "bit_rate_scale" );                       hrd->setBitRateScale( uiCode );
      sei_read_code( pDecodedMessageOutputStream, 4, uiCode, "cpb_size_scale" );                       hrd->setCpbSizeScale( uiCode );
      if( hrd->getSubPicCpbParamsPresentFlag() )
      {
        sei_read_code( pDecodedMessageOutputStream, 4, uiCode, "cpb_size_du_scale" );                  hrd->setDuCpbSizeScale( uiCode );
      }
      sei_read_code( pDecodedMessageOutputStream, 5, uiCode, "initial_cpb_removal_delay_length_minus1" ); hrd->setInitialCpbRemovalDelayLengthMinus1( uiCode );
      sei_read_code( pDecodedMessageOutputStream, 5, uiCode, "au_cpb_removal_delay_length_minus1" );      hrd->setCpbRemovalDelayLengthMinus1( uiCode );
      sei_read_code( pDecodedMessageOutputStream, 5, uiCode, "dpb_output_delay_length_minus1" );       hrd->setDpbOutputDelayLengthMinus1( uiCode );
    }
  }
  Int i, j, nalOrVcl;
  for( i = 0; i <= maxNumSubLayersMinus1; i ++ )
  {
    sei_read_flag( pDecodedMessageOutputStream, uiCode, "fixed_pic_rate_general_flag" );                     hrd->setFixedPicRateFlag( i, uiCode == 1 ? true : false  );
    if( !hrd->getFixedPicRateFlag( i ) )
    {
       sei_read_flag( pDecodedMessageOutputStream, uiCode, "fixed_pic_rate_within_cvs_flag" );                hrd->setFixedPicRateWithinCvsFlag( i, uiCode == 1 ? true : false  );
    }
    else
    {
      hrd->setFixedPicRateWithinCvsFlag( i, true );
    }
    hrd->setLowDelayHrdFlag( i, 0 ); // Infered to be 0 when not present
    hrd->setCpbCntMinus1   ( i, 0 ); // Infered to be 0 when not present
    if( hrd->getFixedPicRateWithinCvsFlag( i ) )
    {
      sei_read_uvlc( pDecodedMessageOutputStream, uiCode, "elemental_duration_in_tc_minus1" );             hrd->setPicDurationInTcMinus1( i, uiCode );
    }
    else
    {
      sei_read_flag( pDecodedMessageOutputStream, uiCode, "low_delay_hrd_flag" );                      hrd->setLowDelayHrdFlag( i, uiCode == 1 ? true : false  );
    }
    if (!hrd->getLowDelayHrdFlag( i ))
    {
      sei_read_uvlc( pDecodedMessageOutputStream, uiCode, "cpb_cnt_minus1" );                          hrd->setCpbCntMinus1( i, uiCode );
    }
    for( nalOrVcl = 0; nalOrVcl < 2; nalOrVcl ++ )
    {
      if( ( ( nalOrVcl == 0 ) && ( hrd->getNalHrdParametersPresentFlag() ) ) ||
          ( ( nalOrVcl == 1 ) && ( hrd->getVclHrdParametersPresentFlag() ) ) )
      {
        for( j = 0; j <= ( hrd->getCpbCntMinus1( i ) ); j ++ )
        {
          sei_read_uvlc( pDecodedMessageOutputStream, uiCode, "bit_rate_value_minus1" );             hrd->setBitRateValueMinus1( i, j, nalOrVcl, uiCode );
          sei_read_uvlc( pDecodedMessageOutputStream, uiCode, "cpb_size_value_minus1" );             hrd->setCpbSizeValueMinus1( i, j, nalOrVcl, uiCode );
          if( hrd->getSubPicCpbParamsPresentFlag() )
          {
            sei_read_uvlc( pDecodedMessageOutputStream, uiCode, "cpb_size_du_value_minus1" );       hrd->setDuCpbSizeValueMinus1( i, j, nalOrVcl, uiCode );
            sei_read_uvlc( pDecodedMessageOutputStream, uiCode, "bit_rate_du_value_minus1" );       hrd->setDuBitRateValueMinus1( i, j, nalOrVcl, uiCode );
          }
           sei_read_flag( pDecodedMessageOutputStream, uiCode, "cbr_flag" );                          hrd->setCbrFlag( i, j, nalOrVcl, uiCode == 1 ? true : false  );
        }
      }
    }
  }
}
#endif

#if P0123_ALPHA_CHANNEL_SEI
void SEIReader::xParseSEIAlphaChannelInfo(SEIAlphaChannelInfo &sei, UInt payloadSize, std::ostream *pDecodedMessageOutputStream)
{
  UInt value;
  sei_read_flag(pDecodedMessageOutputStream, value, "alpha_channel_cancel_flag"); sei.m_alphaChannelCancelFlag = value;
  if(!sei.m_alphaChannelCancelFlag)
  {
    sei_read_code(pDecodedMessageOutputStream, 3, value, "alpha_channel_use_idc");          sei.m_alphaChannelUseIdc = value;
    sei_read_code(pDecodedMessageOutputStream, 3, value, "alpha_channel_bit_depth_minus8"); sei.m_alphaChannelBitDepthMinus8 = value;
    sei_read_code(pDecodedMessageOutputStream, sei.m_alphaChannelBitDepthMinus8 + 9, value, "alpha_transparent_value"); sei.m_alphaTransparentValue = value;
    sei_read_code(pDecodedMessageOutputStream, sei.m_alphaChannelBitDepthMinus8 + 9, value, "alpha_opaque_value"); sei.m_alphaOpaqueValue = value;
    sei_read_flag(pDecodedMessageOutputStream, value, "alpha_channel_incr_flag");        sei.m_alphaChannelIncrFlag = value;
    sei_read_flag(pDecodedMessageOutputStream, value, "alpha_channel_clip_flag");        sei.m_alphaChannelClipFlag = value;
    if(sei.m_alphaChannelClipFlag)
    {
      sei_read_flag(pDecodedMessageOutputStream, value, "alpha_channel_clip_type_flag"); sei.m_alphaChannelClipTypeFlag = value;
    }
  }  
}
#endif

#if Q0096_OVERLAY_SEI
Void SEIReader::xParseSEIOverlayInfo(SEIOverlayInfo& sei, UInt /*payloadSize*/, std::ostream *pDecodedMessageOutputStream)
{
  Int i, j;
  UInt val;
  sei_read_flag( pDecodedMessageOutputStream, val, "overlay_info_cancel_flag" );                 sei.m_overlayInfoCancelFlag = val;
  if ( !sei.m_overlayInfoCancelFlag )
  {
    sei_read_uvlc( pDecodedMessageOutputStream, val, "overlay_content_aux_id_minus128" );            sei.m_overlayContentAuxIdMinus128 = val;
    sei_read_uvlc( pDecodedMessageOutputStream, val, "overlay_label_aux_id_minus128" );              sei.m_overlayLabelAuxIdMinus128 = val;
    sei_read_uvlc( pDecodedMessageOutputStream, val, "overlay_alpha_aux_id_minus128" );              sei.m_overlayAlphaAuxIdMinus128 = val;
    sei_read_uvlc( pDecodedMessageOutputStream, val, "overlay_element_label_value_length_minus8" );  sei.m_overlayElementLabelValueLengthMinus8 = val;
    sei_read_uvlc( pDecodedMessageOutputStream, val, "num_overlays_minus1" );                        sei.m_numOverlaysMinus1 = val;

    assert( sei.m_numOverlaysMinus1 < MAX_OVERLAYS );
    sei.m_overlayIdx.resize( sei.m_numOverlaysMinus1+1 );
    sei.m_languageOverlayPresentFlag.resize( sei.m_numOverlaysMinus1+1 );
    sei.m_overlayContentLayerId.resize( sei.m_numOverlaysMinus1+1 );
    sei.m_overlayLabelPresentFlag.resize( sei.m_numOverlaysMinus1+1 );
    sei.m_overlayLabelLayerId.resize( sei.m_numOverlaysMinus1+1 );
    sei.m_overlayAlphaPresentFlag.resize( sei.m_numOverlaysMinus1+1 );
    sei.m_overlayAlphaLayerId.resize( sei.m_numOverlaysMinus1+1 );
    sei.m_numOverlayElementsMinus1.resize( sei.m_numOverlaysMinus1+1 );
    sei.m_overlayElementLabelMin.resize( sei.m_numOverlaysMinus1+1 );
    sei.m_overlayElementLabelMax.resize( sei.m_numOverlaysMinus1+1 );
    for ( i=0 ; i<=sei.m_numOverlaysMinus1 ; i++ )
    {
      sei_read_uvlc( pDecodedMessageOutputStream, val, "overlay_idx" );                      sei.m_overlayIdx[i] = val;
      sei_read_flag( pDecodedMessageOutputStream, val, "language_overlay_present_flag" );    sei.m_languageOverlayPresentFlag[i] = val;
      sei_read_code( pDecodedMessageOutputStream, 6, val, "overlay_content_layer_id");       sei.m_overlayContentLayerId[i] = val;
      sei_read_flag( pDecodedMessageOutputStream, val, "overlay_label_present_flag" );       sei.m_overlayLabelPresentFlag[i] = val;
      if ( sei.m_overlayLabelPresentFlag[i] )
      {
        sei_read_code( pDecodedMessageOutputStream, 6, val, "overlay_label_layer_id");     sei.m_overlayLabelLayerId[i] = val;
      }
      sei_read_flag( pDecodedMessageOutputStream, val, "overlay_alpha_present_flag" );       sei.m_overlayAlphaPresentFlag[i] = val;
      if ( sei.m_overlayAlphaPresentFlag[i] )
      {
        sei_read_code( pDecodedMessageOutputStream, 6, val, "overlay_alpha_layer_id");     sei.m_overlayAlphaLayerId[i] = val;
      }
      if ( sei.m_overlayLabelPresentFlag[i] )
      {
        sei_read_uvlc( pDecodedMessageOutputStream, val, "num_overlay_elements_minus1");   sei.m_numOverlayElementsMinus1[i] = val;
        assert( sei.m_numOverlayElementsMinus1[i] < MAX_OVERLAY_ELEMENTS );
        sei.m_overlayElementLabelMin[i].resize( sei.m_numOverlayElementsMinus1[i]+1 );
        sei.m_overlayElementLabelMax[i].resize( sei.m_numOverlayElementsMinus1[i]+1 );
        for ( j=0 ; j<=sei.m_numOverlayElementsMinus1[i] ; j++ )
        {
          sei_read_code( pDecodedMessageOutputStream, sei.m_overlayElementLabelValueLengthMinus8 + 8, val, "overlay_element_label_min"); sei.m_overlayElementLabelMin[i][j] = val;
          sei_read_code( pDecodedMessageOutputStream, sei.m_overlayElementLabelValueLengthMinus8 + 8, val, "overlay_element_label_max"); sei.m_overlayElementLabelMax[i][j] = val;
        }      
      }
      else
      {
        sei.m_numOverlayElementsMinus1[i] = 0;
      }
    }

    // byte alignment
    while ( m_pcBitstream->getNumBitsRead() % 8 != 0 )
    {
      sei_read_flag( pDecodedMessageOutputStream, val, "overlay_zero_bit" );
      assert( val==0 );
    }

    UChar* sval = new UChar[MAX_OVERLAY_STRING_BYTES];
    UInt slen;    
    sei.m_overlayLanguage.resize( sei.m_numOverlaysMinus1+1, NULL );
    sei.m_overlayLanguageLength.resize( sei.m_numOverlaysMinus1+1 );
    sei.m_overlayName.resize( sei.m_numOverlaysMinus1+1, NULL );
    sei.m_overlayNameLength.resize( sei.m_numOverlaysMinus1+1 );
    sei.m_overlayElementName.resize( sei.m_numOverlaysMinus1+1 );
    sei.m_overlayElementNameLength.resize( sei.m_numOverlaysMinus1+1 );
    for ( i=0 ; i<=sei.m_numOverlaysMinus1 ; i++ )
    {
      if ( sei.m_languageOverlayPresentFlag[i] )
      {
        READ_STRING( MAX_OVERLAY_STRING_BYTES, sval, slen, "overlay_language" );
        sei.m_overlayLanguage[i] = new UChar[slen];
        memcpy(sei.m_overlayLanguage[i], sval, slen);
        sei.m_overlayLanguageLength[i] = slen;
      }
      READ_STRING( MAX_OVERLAY_STRING_BYTES, sval, slen, "overlay_name" );
      sei.m_overlayName[i] = new UChar[slen];
      memcpy(sei.m_overlayName[i], sval, slen);
      sei.m_overlayNameLength[i] = slen;
      if ( sei.m_overlayLabelPresentFlag[i] )
      {
        sei.m_overlayElementName[i].resize( sei.m_numOverlayElementsMinus1[i]+1, NULL );
        sei.m_overlayElementNameLength[i].resize( sei.m_numOverlayElementsMinus1[i]+1 );
        for ( j=0 ; j<=sei.m_numOverlayElementsMinus1[i] ; j++)
        {
          READ_STRING( MAX_OVERLAY_STRING_BYTES, sval, slen, "overlay_element_name" );
          sei.m_overlayElementName[i][j] = new UChar[slen];
          memcpy(sei.m_overlayElementName[i][j], sval, slen);
          sei.m_overlayElementNameLength[i][j] = slen;
        }
      }
    }
    sei_read_flag( pDecodedMessageOutputStream, val, "overlay_info_persistence_flag" );        sei.m_overlayInfoPersistenceFlag = val;
  }  
}
#endif

#if P0138_USE_ALT_CPB_PARAMS_FLAG
/**
 * Check if SEI message contains payload extension
 */
Bool SEIReader::xPayloadExtensionPresent()
{
  Int payloadBitsRemaining = getBitstream()->getNumBitsLeft();
  Bool payloadExtensionPresent = false;

  if (payloadBitsRemaining > 8)
  {
    payloadExtensionPresent = true;
  }
  else
  {
    Int finalBits = getBitstream()->peekBits(payloadBitsRemaining);
    while (payloadBitsRemaining && (finalBits & 1) == 0)
    {
      payloadBitsRemaining--;
      finalBits >>= 1;
    }
    payloadBitsRemaining--;
    if (payloadBitsRemaining > 0)
    {
      payloadExtensionPresent = true;
    }
  }

  return payloadExtensionPresent;
}
#endif

#if Q0189_TMVP_CONSTRAINTS 
Void SEIReader::xParseSEITMVPConstraints   (SEITMVPConstrains& sei, UInt payloadSize, std::ostream *pDecodedMessageOutputStream)
{
  UInt uiCode;
  sei_read_uvlc( pDecodedMessageOutputStream, uiCode,           "prev_pics_not_used_flag"              ); sei.prev_pics_not_used_flag = uiCode;
  sei_read_uvlc( pDecodedMessageOutputStream, uiCode,           "no_intra_layer_col_pic_flag"          ); sei.no_intra_layer_col_pic_flag = uiCode;
}
#endif

#if Q0247_FRAME_FIELD_INFO
Void SEIReader::xParseSEIFrameFieldInfo    (SEIFrameFieldInfo& sei, UInt payloadSize, std::ostream *pDecodedMessageOutputStream)
{
  UInt code;
  sei_read_code( pDecodedMessageOutputStream, 4, code, "ffinfo_pic_struct"       );       sei.m_ffinfo_picStruct      = code;
  sei_read_code( pDecodedMessageOutputStream, 2, code, "ffinfo_source_scan_type" );       sei.m_ffinfo_sourceScanType = code;
  sei_read_flag( pDecodedMessageOutputStream,    code, "ffinfo_duplicate_flag"   );       sei.m_ffinfo_duplicateFlag  = ( code == 1 ? true : false );
}
#endif


#endif //SVC_EXTENSION

//! \}
