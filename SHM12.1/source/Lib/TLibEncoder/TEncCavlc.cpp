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

/** \file     TEncCavlc.cpp
    \brief    CAVLC encoder class
*/

#include "../TLibCommon/CommonDef.h"
#include "TEncCavlc.h"
#include "SEIwrite.h"

//! \ingroup TLibEncoder
//! \{

#if ENC_DEC_TRACE

Void  xTraceVPSHeader ()
{
  fprintf( g_hTrace, "=========== Video Parameter Set     ===========\n" );
}

Void  xTraceSPSHeader ()
{
  fprintf( g_hTrace, "=========== Sequence Parameter Set  ===========\n" );
}

Void  xTracePPSHeader ()
{
  fprintf( g_hTrace, "=========== Picture Parameter Set  ===========\n");
}

Void  xTraceSliceHeader ()
{
  fprintf( g_hTrace, "=========== Slice ===========\n");
}

Void  xTraceAccessUnitDelimiter ()
{
  fprintf( g_hTrace, "=========== Access Unit Delimiter ===========\n");
}

#endif

Void AUDWriter::codeAUD(TComBitIf& bs, const Int pictureType)
{
#if ENC_DEC_TRACE
  xTraceAccessUnitDelimiter();
#endif

  assert (pictureType < 3);
  setBitstream(&bs);
  WRITE_CODE(pictureType, 3, "pic_type");
  xWriteRbspTrailingBits();
}

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TEncCavlc::TEncCavlc()
{
  m_pcBitIf           = NULL;
}

TEncCavlc::~TEncCavlc()
{
}


// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TEncCavlc::resetEntropy(const TComSlice* /*pSlice*/)
{
}


Void TEncCavlc::codeShortTermRefPicSet( const TComReferencePictureSet* rps, Bool calledFromSliceHeader, Int idx)
{
#if PRINT_RPS_INFO
  Int lastBits = getNumberOfWrittenBits();
#endif
  if (idx > 0)
  {
  WRITE_FLAG( rps->getInterRPSPrediction(), "inter_ref_pic_set_prediction_flag" ); // inter_RPS_prediction_flag
  }
  if (rps->getInterRPSPrediction())
  {
    Int deltaRPS = rps->getDeltaRPS();
    if(calledFromSliceHeader)
    {
      WRITE_UVLC( rps->getDeltaRIdxMinus1(), "delta_idx_minus1" ); // delta index of the Reference Picture Set used for prediction minus 1
    }

    WRITE_CODE( (deltaRPS >=0 ? 0: 1), 1, "delta_rps_sign" ); //delta_rps_sign
    WRITE_UVLC( abs(deltaRPS) - 1, "abs_delta_rps_minus1"); // absolute delta RPS minus 1

    for(Int j=0; j < rps->getNumRefIdc(); j++)
    {
      Int refIdc = rps->getRefIdc(j);
      WRITE_CODE( (refIdc==1? 1: 0), 1, "used_by_curr_pic_flag" ); //first bit is "1" if Idc is 1
      if (refIdc != 1)
      {
        WRITE_CODE( refIdc>>1, 1, "use_delta_flag" ); //second bit is "1" if Idc is 2, "0" otherwise.
      }
    }
  }
  else
  {
    WRITE_UVLC( rps->getNumberOfNegativePictures(), "num_negative_pics" );
    WRITE_UVLC( rps->getNumberOfPositivePictures(), "num_positive_pics" );
    Int prev = 0;
    for(Int j=0 ; j < rps->getNumberOfNegativePictures(); j++)
    {
      WRITE_UVLC( prev-rps->getDeltaPOC(j)-1, "delta_poc_s0_minus1" );
      prev = rps->getDeltaPOC(j);
      WRITE_FLAG( rps->getUsed(j), "used_by_curr_pic_s0_flag");
    }
    prev = 0;
    for(Int j=rps->getNumberOfNegativePictures(); j < rps->getNumberOfNegativePictures()+rps->getNumberOfPositivePictures(); j++)
    {
      WRITE_UVLC( rps->getDeltaPOC(j)-prev-1, "delta_poc_s1_minus1" );
      prev = rps->getDeltaPOC(j);
      WRITE_FLAG( rps->getUsed(j), "used_by_curr_pic_s1_flag" );
    }
  }

#if PRINT_RPS_INFO
  printf("irps=%d (%2d bits) ", rps->getInterRPSPrediction(), getNumberOfWrittenBits() - lastBits);
  rps->printDeltaPOC();
#endif
}


#if CGS_3D_ASYMLUT
Void TEncCavlc::codePPS( const TComPPS* pcPPS, TEnc3DAsymLUT * pc3DAsymLUT )
#else
Void TEncCavlc::codePPS( const TComPPS* pcPPS )
#endif
{
#if ENC_DEC_TRACE
  xTracePPSHeader ();
#endif

  WRITE_UVLC( pcPPS->getPPSId(),                             "pps_pic_parameter_set_id" );
  WRITE_UVLC( pcPPS->getSPSId(),                             "pps_seq_parameter_set_id" );
  WRITE_FLAG( pcPPS->getDependentSliceSegmentsEnabledFlag()    ? 1 : 0, "dependent_slice_segments_enabled_flag" );
  WRITE_FLAG( pcPPS->getOutputFlagPresentFlag() ? 1 : 0,     "output_flag_present_flag" );
  WRITE_CODE( pcPPS->getNumExtraSliceHeaderBits(), 3,        "num_extra_slice_header_bits");
  WRITE_FLAG( pcPPS->getSignDataHidingEnabledFlag(),         "sign_data_hiding_enabled_flag" );
  WRITE_FLAG( pcPPS->getCabacInitPresentFlag() ? 1 : 0,   "cabac_init_present_flag" );
  WRITE_UVLC( pcPPS->getNumRefIdxL0DefaultActive()-1,     "num_ref_idx_l0_default_active_minus1");
  WRITE_UVLC( pcPPS->getNumRefIdxL1DefaultActive()-1,     "num_ref_idx_l1_default_active_minus1");

  WRITE_SVLC( pcPPS->getPicInitQPMinus26(),                  "init_qp_minus26");
  WRITE_FLAG( pcPPS->getConstrainedIntraPred() ? 1 : 0,      "constrained_intra_pred_flag" );
  WRITE_FLAG( pcPPS->getUseTransformSkip() ? 1 : 0,  "transform_skip_enabled_flag" );
  WRITE_FLAG( pcPPS->getUseDQP() ? 1 : 0, "cu_qp_delta_enabled_flag" );
  if ( pcPPS->getUseDQP() )
  {
    WRITE_UVLC( pcPPS->getMaxCuDQPDepth(), "diff_cu_qp_delta_depth" );
  }

  WRITE_SVLC( pcPPS->getQpOffset(COMPONENT_Cb), "pps_cb_qp_offset" );
  WRITE_SVLC( pcPPS->getQpOffset(COMPONENT_Cr), "pps_cr_qp_offset" );

  WRITE_FLAG( pcPPS->getSliceChromaQpFlag() ? 1 : 0,          "pps_slice_chroma_qp_offsets_present_flag" );

  WRITE_FLAG( pcPPS->getUseWP() ? 1 : 0,  "weighted_pred_flag" );   // Use of Weighting Prediction (P_SLICE)
  WRITE_FLAG( pcPPS->getWPBiPred() ? 1 : 0, "weighted_bipred_flag" );  // Use of Weighting Bi-Prediction (B_SLICE)
  WRITE_FLAG( pcPPS->getTransquantBypassEnabledFlag()  ? 1 : 0, "transquant_bypass_enabled_flag" );
  WRITE_FLAG( pcPPS->getTilesEnabledFlag()             ? 1 : 0, "tiles_enabled_flag" );
  WRITE_FLAG( pcPPS->getEntropyCodingSyncEnabledFlag() ? 1 : 0, "entropy_coding_sync_enabled_flag" );
  if( pcPPS->getTilesEnabledFlag() )
  {
    WRITE_UVLC( pcPPS->getNumTileColumnsMinus1(),                                    "num_tile_columns_minus1" );
    WRITE_UVLC( pcPPS->getNumTileRowsMinus1(),                                       "num_tile_rows_minus1" );
    WRITE_FLAG( pcPPS->getTileUniformSpacingFlag(),                                  "uniform_spacing_flag" );
    if( !pcPPS->getTileUniformSpacingFlag() )
    {
      for(UInt i=0; i<pcPPS->getNumTileColumnsMinus1(); i++)
      {
        WRITE_UVLC( pcPPS->getTileColumnWidth(i)-1,                                  "column_width_minus1" );
      }
      for(UInt i=0; i<pcPPS->getNumTileRowsMinus1(); i++)
      {
        WRITE_UVLC( pcPPS->getTileRowHeight(i)-1,                                    "row_height_minus1" );
      }
    }
    assert ((pcPPS->getNumTileColumnsMinus1() + pcPPS->getNumTileRowsMinus1()) != 0);
    WRITE_FLAG( pcPPS->getLoopFilterAcrossTilesEnabledFlag()?1 : 0,       "loop_filter_across_tiles_enabled_flag");
  }
  WRITE_FLAG( pcPPS->getLoopFilterAcrossSlicesEnabledFlag()?1 : 0,        "pps_loop_filter_across_slices_enabled_flag");
  WRITE_FLAG( pcPPS->getDeblockingFilterControlPresentFlag()?1 : 0,       "deblocking_filter_control_present_flag");
  if(pcPPS->getDeblockingFilterControlPresentFlag())
  {
    WRITE_FLAG( pcPPS->getDeblockingFilterOverrideEnabledFlag() ? 1 : 0,  "deblocking_filter_override_enabled_flag" );
    WRITE_FLAG( pcPPS->getPPSDeblockingFilterDisabledFlag() ? 1 : 0,      "pps_deblocking_filter_disabled_flag" );
    if(!pcPPS->getPPSDeblockingFilterDisabledFlag())
    {
      WRITE_SVLC( pcPPS->getDeblockingFilterBetaOffsetDiv2(),             "pps_beta_offset_div2" );
      WRITE_SVLC( pcPPS->getDeblockingFilterTcOffsetDiv2(),               "pps_tc_offset_div2" );
    }
  }
  WRITE_FLAG( pcPPS->getScalingListPresentFlag() ? 1 : 0,                          "pps_scaling_list_data_present_flag" );
  if( pcPPS->getScalingListPresentFlag() )
  {
    codeScalingList( pcPPS->getScalingList() );
  }
  WRITE_FLAG( pcPPS->getListsModificationPresentFlag(), "lists_modification_present_flag");
  WRITE_UVLC( pcPPS->getLog2ParallelMergeLevelMinus2(), "log2_parallel_merge_level_minus2");
  WRITE_FLAG( pcPPS->getSliceHeaderExtensionPresentFlag() ? 1 : 0, "slice_segment_header_extension_present_flag");

  Bool pps_extension_present_flag=false;
  Bool pps_extension_flags[NUM_PPS_EXTENSION_FLAGS]={false};

  pps_extension_flags[PPS_EXT__REXT] = pcPPS->getPpsRangeExtension().settingsDifferFromDefaults(pcPPS->getUseTransformSkip());

  // Other PPS extension flags checked here.

#if SVC_EXTENSION
  pps_extension_flags[PPS_EXT__MLAYER] = pcPPS->getExtensionFlag() ? 1 : 0;
#if CGS_3D_ASYMLUT
  UInt bits = 0;
#endif
#endif

  for(Int i=0; i<NUM_PPS_EXTENSION_FLAGS; i++)
  {
    pps_extension_present_flag|=pps_extension_flags[i];
  }

  WRITE_FLAG( (pps_extension_present_flag?1:0), "pps_extension_present_flag" );

  if (pps_extension_present_flag)
  {
#if ENC_DEC_TRACE || RExt__DECODER_DEBUG_BIT_STATISTICS
    static const TChar *syntaxStrings[]={ "pps_range_extension_flag",
                                          "pps_multilayer_extension_flag",
                                          "pps_extension_6bits[0]",
                                          "pps_extension_6bits[1]",
                                          "pps_extension_6bits[2]",
                                          "pps_extension_6bits[3]",
                                          "pps_extension_6bits[4]",
                                          "pps_extension_6bits[5]" };
#endif

    for(Int i=0; i<NUM_PPS_EXTENSION_FLAGS; i++)
    {
      WRITE_FLAG( pps_extension_flags[i]?1:0, syntaxStrings[i] );
    }

    for(Int i=0; i<NUM_PPS_EXTENSION_FLAGS; i++) // loop used so that the order is determined by the enum.
    {
      if (pps_extension_flags[i])
      {
        switch (PPSExtensionFlagIndex(i))
        {
          case PPS_EXT__REXT:
            {
              const TComPPSRExt &ppsRangeExtension = pcPPS->getPpsRangeExtension();
              if (pcPPS->getUseTransformSkip())
              {
                WRITE_UVLC( ppsRangeExtension.getLog2MaxTransformSkipBlockSize()-2,            "log2_max_transform_skip_block_size_minus2");
              }

              WRITE_FLAG((ppsRangeExtension.getCrossComponentPredictionEnabledFlag() ? 1 : 0), "cross_component_prediction_enabled_flag" );

              WRITE_FLAG(UInt(ppsRangeExtension.getChromaQpOffsetListEnabledFlag()),           "chroma_qp_offset_list_enabled_flag" );
              if (ppsRangeExtension.getChromaQpOffsetListEnabledFlag())
              {
                WRITE_UVLC(ppsRangeExtension.getDiffCuChromaQpOffsetDepth(),                   "diff_cu_chroma_qp_offset_depth");
                WRITE_UVLC(ppsRangeExtension.getChromaQpOffsetListLen() - 1,                   "chroma_qp_offset_list_len_minus1");
                /* skip zero index */
                for (Int cuChromaQpOffsetIdx = 0; cuChromaQpOffsetIdx < ppsRangeExtension.getChromaQpOffsetListLen(); cuChromaQpOffsetIdx++)
                {
                  WRITE_SVLC(ppsRangeExtension.getChromaQpOffsetListEntry(cuChromaQpOffsetIdx+1).u.comp.CbOffset,     "cb_qp_offset_list[i]");
                  WRITE_SVLC(ppsRangeExtension.getChromaQpOffsetListEntry(cuChromaQpOffsetIdx+1).u.comp.CrOffset,     "cr_qp_offset_list[i]");
                }
              }

              WRITE_UVLC( ppsRangeExtension.getLog2SaoOffsetScale(CHANNEL_TYPE_LUMA),           "log2_sao_offset_scale_luma"   );
              WRITE_UVLC( ppsRangeExtension.getLog2SaoOffsetScale(CHANNEL_TYPE_CHROMA),         "log2_sao_offset_scale_chroma" );
            }
            break;

#if SVC_EXTENSION
          case PPS_EXT__MLAYER:
            WRITE_FLAG( pcPPS->getPocResetInfoPresentFlag() ? 1 : 0, "poc_reset_info_present_flag" );

            WRITE_FLAG( pcPPS->getInferScalingListFlag() ? 1 : 0, "pps_infer_scaling_list_flag" );
            if( pcPPS->getInferScalingListFlag() )
            {
              // The value of pps_scaling_list_ref_layer_id shall be in the range of 0 to 62, inclusive
              assert( pcPPS->getScalingListRefLayerId() <= 62 );
              WRITE_CODE( pcPPS->getScalingListRefLayerId(), 6, "pps_scaling_list_ref_layer_id" );
            }

            WRITE_UVLC( pcPPS->getNumRefLayerLocationOffsets(),      "num_ref_loc_offsets" );
            for(Int k = 0; k < pcPPS->getNumRefLayerLocationOffsets(); k++)
            {
              WRITE_CODE( pcPPS->getRefLocationOffsetLayerId(k), 6, "ref_loc_offset_layer_id" );
              WRITE_FLAG( pcPPS->getScaledRefLayerOffsetPresentFlag(k) ? 1 : 0, "scaled_ref_layer_offset_prsent_flag" );
              if( pcPPS->getScaledRefLayerOffsetPresentFlag(k) )
              {
                Window scaledWindow = pcPPS->getScaledRefLayerWindow(k);
                WRITE_SVLC( scaledWindow.getWindowLeftOffset()   >> 1, "scaled_ref_layer_left_offset" );
                WRITE_SVLC( scaledWindow.getWindowTopOffset()    >> 1, "scaled_ref_layer_top_offset" );
                WRITE_SVLC( scaledWindow.getWindowRightOffset()  >> 1, "scaled_ref_layer_right_offset" );
                WRITE_SVLC( scaledWindow.getWindowBottomOffset() >> 1, "scaled_ref_layer_bottom_offset" );
              }

              WRITE_FLAG( pcPPS->getRefRegionOffsetPresentFlag(k) ? 1 : 0, "ref_region_offset_prsent_flag" );

              if( pcPPS->getRefRegionOffsetPresentFlag(k) )
              {
                const Window refWindow = pcPPS->getRefLayerWindow(k);
                WRITE_SVLC( refWindow.getWindowLeftOffset()   >> 1, "ref_region_left_offset" );
                WRITE_SVLC( refWindow.getWindowTopOffset()    >> 1, "ref_region_top_offset" );
                WRITE_SVLC( refWindow.getWindowRightOffset()  >> 1, "ref_region_right_offset" );
                WRITE_SVLC( refWindow.getWindowBottomOffset() >> 1, "ref_region_bottom_offset" );
              }

              WRITE_FLAG( pcPPS->getResamplePhaseSetPresentFlag(k) ? 1 : 0, "resample_phase_set_present_flag" );

              if( pcPPS->getResamplePhaseSetPresentFlag(k) )
              {
                WRITE_UVLC( pcPPS->getPhaseHorLuma(k), "phase_hor_luma" );
                WRITE_UVLC( pcPPS->getPhaseVerLuma(k), "phase_ver_luma" );
                WRITE_UVLC( pcPPS->getPhaseHorChroma(k) + 8, "phase_hor_chroma_plus8" );
                WRITE_UVLC( pcPPS->getPhaseVerChroma(k) + 8, "phase_ver_chroma_plus8" );
              }
            }
#if CGS_3D_ASYMLUT
            bits = getNumberOfWrittenBits();
            WRITE_FLAG( pcPPS->getCGSFlag() , "colour_mapping_enabled_flag" );
            if( pcPPS->getCGSFlag() )
            {
              assert( pc3DAsymLUT != NULL );
              xCode3DAsymLUT( pc3DAsymLUT );
            }
            pc3DAsymLUT->setPPSBit( getNumberOfWrittenBits() - bits );
#endif
            break;
#endif
          default:
            assert(pps_extension_flags[i]==false); // Should never get here with an active PPS extension flag.
            break;
        } // switch
      } // if flag present
    } // loop over PPS flags
  } // pps_extension_present_flag is non-zero
  xWriteRbspTrailingBits();
}

Void TEncCavlc::codeVUI( const TComVUI *pcVUI, const TComSPS* pcSPS )
{
#if ENC_DEC_TRACE
  fprintf( g_hTrace, "----------- vui_parameters -----------\n");
#endif
  WRITE_FLAG(pcVUI->getAspectRatioInfoPresentFlag(),            "aspect_ratio_info_present_flag");
  if (pcVUI->getAspectRatioInfoPresentFlag())
  {
    WRITE_CODE(pcVUI->getAspectRatioIdc(), 8,                   "aspect_ratio_idc" );
    if (pcVUI->getAspectRatioIdc() == 255)
    {
      WRITE_CODE(pcVUI->getSarWidth(), 16,                      "sar_width");
      WRITE_CODE(pcVUI->getSarHeight(), 16,                     "sar_height");
    }
  }
  WRITE_FLAG(pcVUI->getOverscanInfoPresentFlag(),               "overscan_info_present_flag");
  if (pcVUI->getOverscanInfoPresentFlag())
  {
    WRITE_FLAG(pcVUI->getOverscanAppropriateFlag(),             "overscan_appropriate_flag");
  }
  WRITE_FLAG(pcVUI->getVideoSignalTypePresentFlag(),            "video_signal_type_present_flag");
  if (pcVUI->getVideoSignalTypePresentFlag())
  {
    WRITE_CODE(pcVUI->getVideoFormat(), 3,                      "video_format");
    WRITE_FLAG(pcVUI->getVideoFullRangeFlag(),                  "video_full_range_flag");
    WRITE_FLAG(pcVUI->getColourDescriptionPresentFlag(),        "colour_description_present_flag");
    if (pcVUI->getColourDescriptionPresentFlag())
    {
      WRITE_CODE(pcVUI->getColourPrimaries(), 8,                "colour_primaries");
      WRITE_CODE(pcVUI->getTransferCharacteristics(), 8,        "transfer_characteristics");
      WRITE_CODE(pcVUI->getMatrixCoefficients(), 8,             "matrix_coeffs");
    }
  }

  WRITE_FLAG(pcVUI->getChromaLocInfoPresentFlag(),              "chroma_loc_info_present_flag");
  if (pcVUI->getChromaLocInfoPresentFlag())
  {
    WRITE_UVLC(pcVUI->getChromaSampleLocTypeTopField(),         "chroma_sample_loc_type_top_field");
    WRITE_UVLC(pcVUI->getChromaSampleLocTypeBottomField(),      "chroma_sample_loc_type_bottom_field");
  }

  WRITE_FLAG(pcVUI->getNeutralChromaIndicationFlag(),           "neutral_chroma_indication_flag");
  WRITE_FLAG(pcVUI->getFieldSeqFlag(),                          "field_seq_flag");
  WRITE_FLAG(pcVUI->getFrameFieldInfoPresentFlag(),             "frame_field_info_present_flag");

  Window defaultDisplayWindow = pcVUI->getDefaultDisplayWindow();
  WRITE_FLAG(defaultDisplayWindow.getWindowEnabledFlag(),       "default_display_window_flag");
  if( defaultDisplayWindow.getWindowEnabledFlag() )
  {
    WRITE_UVLC(defaultDisplayWindow.getWindowLeftOffset()  / TComSPS::getWinUnitX(pcSPS->getChromaFormatIdc()), "def_disp_win_left_offset");
    WRITE_UVLC(defaultDisplayWindow.getWindowRightOffset() / TComSPS::getWinUnitX(pcSPS->getChromaFormatIdc()), "def_disp_win_right_offset");
    WRITE_UVLC(defaultDisplayWindow.getWindowTopOffset()   / TComSPS::getWinUnitY(pcSPS->getChromaFormatIdc()), "def_disp_win_top_offset");
    WRITE_UVLC(defaultDisplayWindow.getWindowBottomOffset()/ TComSPS::getWinUnitY(pcSPS->getChromaFormatIdc()), "def_disp_win_bottom_offset");
  }
  const TimingInfo *timingInfo = pcVUI->getTimingInfo();
  WRITE_FLAG(timingInfo->getTimingInfoPresentFlag(),          "vui_timing_info_present_flag");
  if(timingInfo->getTimingInfoPresentFlag())
  {
    WRITE_CODE(timingInfo->getNumUnitsInTick(), 32,           "vui_num_units_in_tick");
    WRITE_CODE(timingInfo->getTimeScale(),      32,           "vui_time_scale");
    WRITE_FLAG(timingInfo->getPocProportionalToTimingFlag(),  "vui_poc_proportional_to_timing_flag");
    if(timingInfo->getPocProportionalToTimingFlag())
    {
      WRITE_UVLC(timingInfo->getNumTicksPocDiffOneMinus1(),   "vui_num_ticks_poc_diff_one_minus1");
    }
    WRITE_FLAG(pcVUI->getHrdParametersPresentFlag(),              "vui_hrd_parameters_present_flag");
    if( pcVUI->getHrdParametersPresentFlag() )
    {
      codeHrdParameters(pcVUI->getHrdParameters(), 1, pcSPS->getMaxTLayers() - 1 );
    }
  }

  WRITE_FLAG(pcVUI->getBitstreamRestrictionFlag(),              "bitstream_restriction_flag");
  if (pcVUI->getBitstreamRestrictionFlag())
  {
    WRITE_FLAG(pcVUI->getTilesFixedStructureFlag(),             "tiles_fixed_structure_flag");
    WRITE_FLAG(pcVUI->getMotionVectorsOverPicBoundariesFlag(),  "motion_vectors_over_pic_boundaries_flag");
    WRITE_FLAG(pcVUI->getRestrictedRefPicListsFlag(),           "restricted_ref_pic_lists_flag");
    WRITE_UVLC(pcVUI->getMinSpatialSegmentationIdc(),           "min_spatial_segmentation_idc");
    WRITE_UVLC(pcVUI->getMaxBytesPerPicDenom(),                 "max_bytes_per_pic_denom");
    WRITE_UVLC(pcVUI->getMaxBitsPerMinCuDenom(),                "max_bits_per_min_cu_denom");
    WRITE_UVLC(pcVUI->getLog2MaxMvLengthHorizontal(),           "log2_max_mv_length_horizontal");
    WRITE_UVLC(pcVUI->getLog2MaxMvLengthVertical(),             "log2_max_mv_length_vertical");
  }
}

Void TEncCavlc::codeHrdParameters( const TComHRD *hrd, Bool commonInfPresentFlag, UInt maxNumSubLayersMinus1 )
{
  if( commonInfPresentFlag )
  {
    WRITE_FLAG( hrd->getNalHrdParametersPresentFlag() ? 1 : 0 ,  "nal_hrd_parameters_present_flag" );
    WRITE_FLAG( hrd->getVclHrdParametersPresentFlag() ? 1 : 0 ,  "vcl_hrd_parameters_present_flag" );
    if( hrd->getNalHrdParametersPresentFlag() || hrd->getVclHrdParametersPresentFlag() )
    {
      WRITE_FLAG( hrd->getSubPicCpbParamsPresentFlag() ? 1 : 0,  "sub_pic_hrd_params_present_flag" );
      if( hrd->getSubPicCpbParamsPresentFlag() )
      {
        WRITE_CODE( hrd->getTickDivisorMinus2(), 8,              "tick_divisor_minus2" );
        WRITE_CODE( hrd->getDuCpbRemovalDelayLengthMinus1(), 5,  "du_cpb_removal_delay_increment_length_minus1" );
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
    Bool fixedPixRateWithinCvsFlag = true;
    if( !hrd->getFixedPicRateFlag( i ) )
    {
      fixedPixRateWithinCvsFlag = hrd->getFixedPicRateWithinCvsFlag( i );
      WRITE_FLAG( hrd->getFixedPicRateWithinCvsFlag( i ) ? 1 : 0, "fixed_pic_rate_within_cvs_flag");
    }
    if( fixedPixRateWithinCvsFlag )
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

Void TEncCavlc::codeSPS( const TComSPS* pcSPS )
{
#if SVC_EXTENSION
  Bool V1CompatibleSPSFlag = !(pcSPS->getLayerId() != 0 && pcSPS->getNumDirectRefLayers() != 0);
#endif

  const ChromaFormat format                = pcSPS->getChromaFormatIdc();
  const Bool         chromaEnabled         = isChromaEnabled(format);

#if ENC_DEC_TRACE
  xTraceSPSHeader ();
#endif
  WRITE_CODE( pcSPS->getVPSId (),          4,       "sps_video_parameter_set_id" );
#if SVC_EXTENSION
  if(pcSPS->getLayerId() == 0)
  {
#endif
  WRITE_CODE( pcSPS->getMaxTLayers() - 1,  3,       "sps_max_sub_layers_minus1" );
#if SVC_EXTENSION
  }
  else
  {
    WRITE_CODE( V1CompatibleSPSFlag ? (pcSPS->getMaxTLayers() - 1) : 7,  3,       "sps_ext_or_max_sub_layers_minus1" );
  }

  if( V1CompatibleSPSFlag )
  {
#endif
  WRITE_FLAG( pcSPS->getTemporalIdNestingFlag() ? 1 : 0, "sps_temporal_id_nesting_flag" );
  codePTL(pcSPS->getPTL(), 1, pcSPS->getMaxTLayers() - 1);
#if SVC_EXTENSION
  }
#endif
  WRITE_UVLC( pcSPS->getSPSId (),                   "sps_seq_parameter_set_id" );
#if SVC_EXTENSION
  if( !V1CompatibleSPSFlag )
  {
    WRITE_FLAG( pcSPS->getUpdateRepFormatFlag(), "update_rep_format_flag" );
  
    if( pcSPS->getUpdateRepFormatFlag())
    {
      WRITE_CODE( pcSPS->getUpdateRepFormatIndex(), 8,   "sps_rep_format_idx");
    }
  }
  else
  {
#endif
  WRITE_UVLC( Int(pcSPS->getChromaFormatIdc ()),    "chroma_format_idc" );
  if( format == CHROMA_444 )
  {
    WRITE_FLAG( 0,                                  "separate_colour_plane_flag");
  }

  WRITE_UVLC( pcSPS->getPicWidthInLumaSamples (),   "pic_width_in_luma_samples" );
  WRITE_UVLC( pcSPS->getPicHeightInLumaSamples(),   "pic_height_in_luma_samples" );
  Window conf = pcSPS->getConformanceWindow();

  WRITE_FLAG( conf.getWindowEnabledFlag(),          "conformance_window_flag" );
  if (conf.getWindowEnabledFlag())
  {
#if SVC_EXTENSION
    WRITE_UVLC( conf.getWindowLeftOffset(),   "conf_win_left_offset"   );
    WRITE_UVLC( conf.getWindowRightOffset(),  "conf_win_right_offset"  );
    WRITE_UVLC( conf.getWindowTopOffset(),    "conf_win_top_offset"    );
    WRITE_UVLC( conf.getWindowBottomOffset(), "conf_win_bottom_offset" );
#else
    WRITE_UVLC( conf.getWindowLeftOffset()   / TComSPS::getWinUnitX(pcSPS->getChromaFormatIdc() ), "conf_win_left_offset" );
    WRITE_UVLC( conf.getWindowRightOffset()  / TComSPS::getWinUnitX(pcSPS->getChromaFormatIdc() ), "conf_win_right_offset" );
    WRITE_UVLC( conf.getWindowTopOffset()    / TComSPS::getWinUnitY(pcSPS->getChromaFormatIdc() ), "conf_win_top_offset" );
    WRITE_UVLC( conf.getWindowBottomOffset() / TComSPS::getWinUnitY(pcSPS->getChromaFormatIdc() ), "conf_win_bottom_offset" );
#endif
  }
#if SVC_EXTENSION
  }

  if( V1CompatibleSPSFlag )
  {
#endif
  WRITE_UVLC( pcSPS->getBitDepth(CHANNEL_TYPE_LUMA) - 8,                      "bit_depth_luma_minus8" );

  WRITE_UVLC( chromaEnabled ? (pcSPS->getBitDepth(CHANNEL_TYPE_CHROMA) - 8):0,  "bit_depth_chroma_minus8" );
#if SVC_EXTENSION
  }
#endif

  WRITE_UVLC( pcSPS->getBitsForPOC()-4,                 "log2_max_pic_order_cnt_lsb_minus4" );

#if SVC_EXTENSION
  if( V1CompatibleSPSFlag )
  {
#endif
  const Bool subLayerOrderingInfoPresentFlag = 1;
  WRITE_FLAG(subLayerOrderingInfoPresentFlag,       "sps_sub_layer_ordering_info_present_flag");
  for(UInt i=0; i <= pcSPS->getMaxTLayers()-1; i++)
  {
    WRITE_UVLC( pcSPS->getMaxDecPicBuffering(i) - 1,       "sps_max_dec_pic_buffering_minus1[i]" );
    WRITE_UVLC( pcSPS->getNumReorderPics(i),               "sps_max_num_reorder_pics[i]" );
    WRITE_UVLC( pcSPS->getMaxLatencyIncreasePlus1(i),      "sps_max_latency_increase_plus1[i]" );
    if (!subLayerOrderingInfoPresentFlag)
    {
      break;
    }
  }
#if SVC_EXTENSION
  }
#endif
  assert( pcSPS->getMaxCUWidth() == pcSPS->getMaxCUHeight() );
  WRITE_UVLC( pcSPS->getLog2MinCodingBlockSize() - 3,                                "log2_min_luma_coding_block_size_minus3" );
  WRITE_UVLC( pcSPS->getLog2DiffMaxMinCodingBlockSize(),                             "log2_diff_max_min_luma_coding_block_size" );
  WRITE_UVLC( pcSPS->getQuadtreeTULog2MinSize() - 2,                                 "log2_min_luma_transform_block_size_minus2" );
  WRITE_UVLC( pcSPS->getQuadtreeTULog2MaxSize() - pcSPS->getQuadtreeTULog2MinSize(), "log2_diff_max_min_luma_transform_block_size" );
  WRITE_UVLC( pcSPS->getQuadtreeTUMaxDepthInter() - 1,                               "max_transform_hierarchy_depth_inter" );
  WRITE_UVLC( pcSPS->getQuadtreeTUMaxDepthIntra() - 1,                               "max_transform_hierarchy_depth_intra" );
  WRITE_FLAG( pcSPS->getScalingListFlag() ? 1 : 0,                                   "scaling_list_enabled_flag" );
  if(pcSPS->getScalingListFlag())
  {
#if SVC_EXTENSION
    if( !V1CompatibleSPSFlag )
    {
      WRITE_FLAG( pcSPS->getInferScalingListFlag() ? 1 : 0, "sps_infer_scaling_list_flag" );
    }

    if( pcSPS->getInferScalingListFlag() )
    {
      // The value of pps_scaling_list_ref_layer_id shall be in the range of 0 to 62, inclusive
      assert( pcSPS->getScalingListRefLayerId() <= 62 );

      WRITE_CODE( pcSPS->getScalingListRefLayerId(), 6, "sps_scaling_list_ref_layer_id" );
    }
    else
    {
#endif
    WRITE_FLAG( pcSPS->getScalingListPresentFlag() ? 1 : 0,                          "sps_scaling_list_data_present_flag" );
    if(pcSPS->getScalingListPresentFlag())
    {
      codeScalingList( pcSPS->getScalingList() );
    }
#if SVC_EXTENSION
    }
#endif
  }
  WRITE_FLAG( pcSPS->getUseAMP() ? 1 : 0,                                            "amp_enabled_flag" );
  WRITE_FLAG( pcSPS->getUseSAO() ? 1 : 0,                                            "sample_adaptive_offset_enabled_flag");

  WRITE_FLAG( pcSPS->getUsePCM() ? 1 : 0,                                            "pcm_enabled_flag");
  if( pcSPS->getUsePCM() )
  {
    WRITE_CODE( pcSPS->getPCMBitDepth(CHANNEL_TYPE_LUMA) - 1, 4,                            "pcm_sample_bit_depth_luma_minus1" );
    WRITE_CODE( chromaEnabled ? (pcSPS->getPCMBitDepth(CHANNEL_TYPE_CHROMA) - 1) : 0, 4,    "pcm_sample_bit_depth_chroma_minus1" );
    WRITE_UVLC( pcSPS->getPCMLog2MinSize() - 3,                                      "log2_min_pcm_luma_coding_block_size_minus3" );
    WRITE_UVLC( pcSPS->getPCMLog2MaxSize() - pcSPS->getPCMLog2MinSize(),             "log2_diff_max_min_pcm_luma_coding_block_size" );
    WRITE_FLAG( pcSPS->getPCMFilterDisableFlag()?1 : 0,                              "pcm_loop_filter_disable_flag");
  }

  assert( pcSPS->getMaxTLayers() > 0 );

  const TComRPSList* rpsList = pcSPS->getRPSList();

  WRITE_UVLC(rpsList->getNumberOfReferencePictureSets(), "num_short_term_ref_pic_sets" );
  for(Int i=0; i < rpsList->getNumberOfReferencePictureSets(); i++)
  {
    const TComReferencePictureSet*rps = rpsList->getReferencePictureSet(i);
    codeShortTermRefPicSet( rps,false, i);
  }
  WRITE_FLAG( pcSPS->getLongTermRefsPresent() ? 1 : 0,         "long_term_ref_pics_present_flag" );
  if (pcSPS->getLongTermRefsPresent())
  {
    WRITE_UVLC(pcSPS->getNumLongTermRefPicSPS(), "num_long_term_ref_pics_sps" );
    for (UInt k = 0; k < pcSPS->getNumLongTermRefPicSPS(); k++)
    {
      WRITE_CODE( pcSPS->getLtRefPicPocLsbSps(k), pcSPS->getBitsForPOC(), "lt_ref_pic_poc_lsb_sps");
      WRITE_FLAG( pcSPS->getUsedByCurrPicLtSPSFlag(k), "used_by_curr_pic_lt_sps_flag[i]");
    }
  }
  WRITE_FLAG( pcSPS->getSPSTemporalMVPEnabledFlag()  ? 1 : 0,  "sps_temporal_mvp_enabled_flag" );

  WRITE_FLAG( pcSPS->getUseStrongIntraSmoothing(),             "strong_intra_smoothing_enable_flag" );

  WRITE_FLAG( pcSPS->getVuiParametersPresentFlag(),            "vui_parameters_present_flag" );
  if (pcSPS->getVuiParametersPresentFlag())
  {
      codeVUI(pcSPS->getVuiParameters(), pcSPS);
  }

  Bool sps_extension_present_flag=false;
  Bool sps_extension_flags[NUM_SPS_EXTENSION_FLAGS]={false};

  sps_extension_flags[SPS_EXT__REXT] = pcSPS->getSpsRangeExtension().settingsDifferFromDefaults();

  // Other SPS extension flags checked here.
#if SVC_EXTENSION
  sps_extension_flags[SPS_EXT__MLAYER] = pcSPS->getExtensionFlag() ? 1 : 0;
#endif

  for(Int i=0; i<NUM_SPS_EXTENSION_FLAGS; i++)
  {
    sps_extension_present_flag|=sps_extension_flags[i];
  }

  WRITE_FLAG( (sps_extension_present_flag?1:0), "sps_extension_present_flag" );

  if (sps_extension_present_flag)
  {
#if ENC_DEC_TRACE || RExt__DECODER_DEBUG_BIT_STATISTICS
    static const TChar *syntaxStrings[]={ "sps_range_extension_flag",
                                          "sps_multilayer_extension_flag",
                                          "sps_extension_6bits[0]",
                                          "sps_extension_6bits[1]",
                                          "sps_extension_6bits[2]",
                                          "sps_extension_6bits[3]",
                                          "sps_extension_6bits[4]",
                                          "sps_extension_6bits[5]" };
#endif

    for(Int i=0; i<NUM_SPS_EXTENSION_FLAGS; i++)
    {
      WRITE_FLAG( sps_extension_flags[i]?1:0, syntaxStrings[i] );
    }

    for(Int i=0; i<NUM_SPS_EXTENSION_FLAGS; i++) // loop used so that the order is determined by the enum.
    {
      if (sps_extension_flags[i])
      {
        switch (SPSExtensionFlagIndex(i))
        {
          case SPS_EXT__REXT:
          {
            const TComSPSRExt &spsRangeExtension=pcSPS->getSpsRangeExtension();

            WRITE_FLAG( (spsRangeExtension.getTransformSkipRotationEnabledFlag() ? 1 : 0),      "transform_skip_rotation_enabled_flag");
            WRITE_FLAG( (spsRangeExtension.getTransformSkipContextEnabledFlag() ? 1 : 0),       "transform_skip_context_enabled_flag");
            WRITE_FLAG( (spsRangeExtension.getRdpcmEnabledFlag(RDPCM_SIGNAL_IMPLICIT) ? 1 : 0), "implicit_rdpcm_enabled_flag" );
            WRITE_FLAG( (spsRangeExtension.getRdpcmEnabledFlag(RDPCM_SIGNAL_EXPLICIT) ? 1 : 0), "explicit_rdpcm_enabled_flag" );
            WRITE_FLAG( (spsRangeExtension.getExtendedPrecisionProcessingFlag() ? 1 : 0),       "extended_precision_processing_flag" );
            WRITE_FLAG( (spsRangeExtension.getIntraSmoothingDisabledFlag() ? 1 : 0),            "intra_smoothing_disabled_flag" );
            WRITE_FLAG( (spsRangeExtension.getHighPrecisionOffsetsEnabledFlag() ? 1 : 0),       "high_precision_offsets_enabled_flag" );
            WRITE_FLAG( (spsRangeExtension.getPersistentRiceAdaptationEnabledFlag() ? 1 : 0),   "persistent_rice_adaptation_enabled_flag" );
            WRITE_FLAG( (spsRangeExtension.getCabacBypassAlignmentEnabledFlag() ? 1 : 0),       "cabac_bypass_alignment_enabled_flag" );
            break;
          }
#if SVC_EXTENSION
          case SPS_EXT__MLAYER:
            codeSPSExtension( pcSPS ); //it is sps_multilayer_extension
            break;
#endif
          default:
            assert(sps_extension_flags[i]==false); // Should never get here with an active SPS extension flag.
            break;
        }
      }
    }
  }
  xWriteRbspTrailingBits();
}

Void TEncCavlc::codeVPS( const TComVPS* pcVPS )
{
#if ENC_DEC_TRACE
  xTraceVPSHeader();
#endif
  WRITE_CODE( pcVPS->getVPSId(),                    4,        "vps_video_parameter_set_id" );
#if SVC_EXTENSION
  WRITE_FLAG( pcVPS->getBaseLayerInternalFlag(),              "vps_base_layer_internal_flag");
  WRITE_FLAG( pcVPS->getBaseLayerAvailableFlag(),             "vps_base_layer_available_flag");
  WRITE_CODE( pcVPS->getMaxLayers() - 1,            6,        "vps_max_layers_minus1" );
  assert( pcVPS->getBaseLayerInternalFlag() || pcVPS->getMaxLayers() > 1 );
#else
  WRITE_FLAG(                                       1,        "vps_base_layer_internal_flag" );
  WRITE_FLAG(                                       1,        "vps_base_layer_available_flag" );
  WRITE_CODE( 0,                                    6,        "vps_max_layers_minus1" );
#endif
  WRITE_CODE( pcVPS->getMaxTLayers() - 1,           3,        "vps_max_sub_layers_minus1" );
  WRITE_FLAG( pcVPS->getTemporalNestingFlag(),                "vps_temporal_id_nesting_flag" );
  assert (pcVPS->getMaxTLayers()>1||pcVPS->getTemporalNestingFlag());
  WRITE_CODE( 0xffff,                              16,        "vps_reserved_0xffff_16bits" );
  codePTL( pcVPS->getPTL(), true, pcVPS->getMaxTLayers() - 1 );
  const Bool subLayerOrderingInfoPresentFlag = 1;
  WRITE_FLAG(subLayerOrderingInfoPresentFlag,              "vps_sub_layer_ordering_info_present_flag");
  for(UInt i=0; i <= pcVPS->getMaxTLayers()-1; i++)
  {
    WRITE_UVLC( pcVPS->getMaxDecPicBuffering(i) - 1,       "vps_max_dec_pic_buffering_minus1[i]" );
    WRITE_UVLC( pcVPS->getNumReorderPics(i),               "vps_max_num_reorder_pics[i]" );
    WRITE_UVLC( pcVPS->getMaxLatencyIncrease(i),           "vps_max_latency_increase_plus1[i]" );
    if (!subLayerOrderingInfoPresentFlag)
    {
      break;
    }
  }

#if SVC_EXTENSION
  assert( pcVPS->getNumHrdParameters() <= MAX_VPS_LAYER_SETS_PLUS1 );
  assert( pcVPS->getMaxLayerId() < MAX_VPS_LAYER_IDX_PLUS1 );

  WRITE_CODE( pcVPS->getMaxLayerId(), 6,                       "vps_max_layer_id" );
  WRITE_UVLC(pcVPS->getVpsNumLayerSetsMinus1(),                "vps_num_layer_sets_minus1");

  for( UInt opsIdx = 1; opsIdx <= pcVPS->getVpsNumLayerSetsMinus1(); opsIdx++ )
  {
    // Operation point set
    for( UInt i = 0; i <= pcVPS->getMaxLayerId(); i ++ )
    {
#else
  assert( pcVPS->getNumHrdParameters() <= MAX_VPS_NUM_HRD_PARAMETERS );
  assert( pcVPS->getMaxNuhReservedZeroLayerId() < MAX_VPS_NUH_RESERVED_ZERO_LAYER_ID_PLUS1 );
  WRITE_CODE( pcVPS->getMaxNuhReservedZeroLayerId(), 6,     "vps_max_layer_id" );
  WRITE_UVLC( pcVPS->getMaxOpSets() - 1,                    "vps_num_layer_sets_minus1" );
  for( UInt opsIdx = 1; opsIdx <= ( pcVPS->getMaxOpSets() - 1 ); opsIdx ++ )
  {
    // Operation point set
    for( UInt i = 0; i <= pcVPS->getMaxNuhReservedZeroLayerId(); i ++ )
    {
      // Only applicable for version 1
      // pcVPS->setLayerIdIncludedFlag( true, opsIdx, i );
#endif
      WRITE_FLAG( pcVPS->getLayerIdIncludedFlag( opsIdx, i ) ? 1 : 0, "layer_id_included_flag[opsIdx][i]" );
    }
  }
  const TimingInfo *timingInfo = pcVPS->getTimingInfo();
  WRITE_FLAG(timingInfo->getTimingInfoPresentFlag(),          "vps_timing_info_present_flag");
  if(timingInfo->getTimingInfoPresentFlag())
  {
    WRITE_CODE(timingInfo->getNumUnitsInTick(), 32,           "vps_num_units_in_tick");
    WRITE_CODE(timingInfo->getTimeScale(),      32,           "vps_time_scale");
    WRITE_FLAG(timingInfo->getPocProportionalToTimingFlag(),  "vps_poc_proportional_to_timing_flag");
    if(timingInfo->getPocProportionalToTimingFlag())
    {
      WRITE_UVLC(timingInfo->getNumTicksPocDiffOneMinus1(),   "vps_num_ticks_poc_diff_one_minus1");
    }
    WRITE_UVLC( pcVPS->getNumHrdParameters(),                 "vps_num_hrd_parameters" );

    if( pcVPS->getNumHrdParameters() > 0 )
    {
      for( UInt i = 0; i < pcVPS->getNumHrdParameters(); i ++ )
      {
        // Only applicable for version 1
        WRITE_UVLC( pcVPS->getHrdOpSetIdx( i ),                "hrd_layer_set_idx" );
        if( i > 0 )
        {
          WRITE_FLAG( pcVPS->getCprmsPresentFlag( i ) ? 1 : 0, "cprms_present_flag[i]" );
        }
        codeHrdParameters(pcVPS->getHrdParameters(i), pcVPS->getCprmsPresentFlag( i ), pcVPS->getMaxTLayers() - 1);
      }
    }
  }
#if SVC_EXTENSION
  // When MaxLayersMinus1 is greater than 0, vps_extension_flag shall be equal to 1.
  if( pcVPS->getMaxLayers() > 1 )
  {
    assert( pcVPS->getVpsExtensionFlag() == true );
  }

  WRITE_FLAG( pcVPS->getVpsExtensionFlag() ? 1 : 0,                     "vps_extension_flag" );

  if( pcVPS->getVpsExtensionFlag() )
  {
    while ( m_pcBitIf->getNumberOfWrittenBits() % 8 != 0 )
    {
      WRITE_FLAG(1,                  "vps_extension_alignment_bit_equal_to_one");
    }
    codeVPSExtension(pcVPS);
    WRITE_FLAG( 0,                     "vps_extension2_flag" );   // Flag value of 1 reserved
  }
#else
  WRITE_FLAG( 0,                     "vps_extension_flag" );
#endif    

  //future extensions here..
  xWriteRbspTrailingBits();
}

Void TEncCavlc::codeSliceHeader         ( TComSlice* pcSlice )
{
#if ENC_DEC_TRACE
  xTraceSliceHeader ();
#endif

  const ChromaFormat format                = pcSlice->getSPS()->getChromaFormatIdc();
  const UInt         numberValidComponents = getNumberValidComponents(format);
  const Bool         chromaEnabled         = isChromaEnabled(format);

  //calculate number of bits required for slice address
  Int maxSliceSegmentAddress = pcSlice->getPic()->getNumberOfCtusInFrame();
  Int bitsSliceSegmentAddress = 0;
  while(maxSliceSegmentAddress>(1<<bitsSliceSegmentAddress))
  {
    bitsSliceSegmentAddress++;
  }
  const Int ctuTsAddress = pcSlice->getSliceSegmentCurStartCtuTsAddr();

  //write slice address
  const Int sliceSegmentRsAddress = pcSlice->getPic()->getPicSym()->getCtuTsToRsAddrMap(ctuTsAddress);

  WRITE_FLAG( sliceSegmentRsAddress==0, "first_slice_segment_in_pic_flag" );
  if ( pcSlice->getRapPicFlag() )
  {
    WRITE_FLAG( pcSlice->getNoOutputPriorPicsFlag() ? 1 : 0, "no_output_of_prior_pics_flag" );
  }
  WRITE_UVLC( pcSlice->getPPS()->getPPSId(), "slice_pic_parameter_set_id" );
  if ( pcSlice->getPPS()->getDependentSliceSegmentsEnabledFlag() && (sliceSegmentRsAddress!=0) )
  {
    WRITE_FLAG( pcSlice->getDependentSliceSegmentFlag() ? 1 : 0, "dependent_slice_segment_flag" );
  }
  if(sliceSegmentRsAddress>0)
  {
    WRITE_CODE( sliceSegmentRsAddress, bitsSliceSegmentAddress, "slice_segment_address" );
  }
  if ( !pcSlice->getDependentSliceSegmentFlag() )
  {
#if SVC_EXTENSION
    Int iBits = 0;
    if(pcSlice->getPPS()->getNumExtraSliceHeaderBits() > iBits)
    {
      assert(!!"discardable_flag");

      if (pcSlice->getDiscardableFlag())
      {
        assert(pcSlice->getNalUnitType() != NAL_UNIT_CODED_SLICE_TRAIL_R &&
          pcSlice->getNalUnitType() != NAL_UNIT_CODED_SLICE_TSA_R &&
          pcSlice->getNalUnitType() != NAL_UNIT_CODED_SLICE_STSA_R &&
          pcSlice->getNalUnitType() != NAL_UNIT_CODED_SLICE_RADL_R &&
          pcSlice->getNalUnitType() != NAL_UNIT_CODED_SLICE_RASL_R);
      }

      WRITE_FLAG(pcSlice->getDiscardableFlag(), "discardable_flag");
      iBits++;
    }

    if( pcSlice->getPPS()->getNumExtraSliceHeaderBits() > iBits )
    {
      assert(!!"cross_layer_bla_flag");
      WRITE_FLAG(pcSlice->getCrossLayerBLAFlag(), "cross_layer_bla_flag");
      iBits++;
    }

    for (; iBits < pcSlice->getPPS()->getNumExtraSliceHeaderBits(); iBits++)
    {
      assert(!!"slice_reserved_undetermined_flag[]");
      WRITE_FLAG(0, "slice_reserved_undetermined_flag[]");
    }
#else //SVC_EXTENSION
    for (Int i = 0; i < pcSlice->getPPS()->getNumExtraSliceHeaderBits(); i++)
    {
      WRITE_FLAG(0, "slice_reserved_flag[]");
    }
#endif //SVC_EXTENSION

    WRITE_UVLC( pcSlice->getSliceType(),       "slice_type" );

    if( pcSlice->getPPS()->getOutputFlagPresentFlag() )
    {
      WRITE_FLAG( pcSlice->getPicOutputFlag() ? 1 : 0, "pic_output_flag" );
    }

#if SVC_EXTENSION
    if( (pcSlice->getLayerId() > 0 && !pcSlice->getVPS()->getPocLsbNotPresentFlag( pcSlice->getVPS()->getLayerIdxInVps(pcSlice->getLayerId())) ) || !pcSlice->getIdrPicFlag())
#else
    if( !pcSlice->getIdrPicFlag() )
#endif
    {
#if SVC_POC
      Int picOrderCntLSB;
      if( pcSlice->getPocResetIdc() == 2 )  // i.e. the LSB is reset
      {
        picOrderCntLSB = pcSlice->getPicOrderCntLsb();  // This will be the LSB value w.r.t to the previous POC reset period.
      }
      else
      {
        picOrderCntLSB = (pcSlice->getPOC() + (1<<pcSlice->getSPS()->getBitsForPOC())) & ((1<<pcSlice->getSPS()->getBitsForPOC())-1);
      }
#else
      Int picOrderCntLSB = (pcSlice->getPOC()-pcSlice->getLastIDR()+(1<<pcSlice->getSPS()->getBitsForPOC())) & ((1<<pcSlice->getSPS()->getBitsForPOC())-1);
#endif
      WRITE_CODE( picOrderCntLSB, pcSlice->getSPS()->getBitsForPOC(), "slice_pic_order_cnt_lsb");

#if SVC_EXTENSION
    }
    if( !pcSlice->getIdrPicFlag() )
    {
#endif
      const TComReferencePictureSet* rps = pcSlice->getRPS();

      // check for bitstream restriction stating that:
      // If the current picture is a BLA or CRA picture, the value of NumPocTotalCurr shall be equal to 0.
      // Ideally this process should not be repeated for each slice in a picture
#if SVC_EXTENSION
      if( pcSlice->getLayerId() == 0 )
#endif
      if (pcSlice->isIRAP())
      {
        for (Int picIdx = 0; picIdx < rps->getNumberOfPictures(); picIdx++)
        {
          assert (!rps->getUsed(picIdx));
        }
      }

      if(pcSlice->getRPSidx() < 0)
      {
        WRITE_FLAG( 0, "short_term_ref_pic_set_sps_flag");
        codeShortTermRefPicSet( rps, true, pcSlice->getSPS()->getRPSList()->getNumberOfReferencePictureSets());
      }
      else
      {
        WRITE_FLAG( 1, "short_term_ref_pic_set_sps_flag");
        Int numBits = 0;
        while ((1 << numBits) < pcSlice->getSPS()->getRPSList()->getNumberOfReferencePictureSets())
        {
          numBits++;
        }
        if (numBits > 0)
        {
          WRITE_CODE( pcSlice->getRPSidx(), numBits, "short_term_ref_pic_set_idx" );
        }
      }
      if(pcSlice->getSPS()->getLongTermRefsPresent())
      {
        Int numLtrpInSH = rps->getNumberOfLongtermPictures();
        Int ltrpInSPS[MAX_NUM_REF_PICS];
        Int numLtrpInSPS = 0;
        UInt ltrpIndex;
        Int counter = 0;
        // WARNING: The following code only works only if a matching long-term RPS is 
        //          found in the SPS for ALL long-term pictures
        //          The problem is that the SPS coded long-term pictures are moved to the
        //          beginning of the list which causes a mismatch when no reference picture
        //          list reordering is used
        //          NB: Long-term coding is currently not supported in general by the HM encoder
        for(Int k = rps->getNumberOfPictures()-1; k > rps->getNumberOfPictures()-rps->getNumberOfLongtermPictures()-1; k--)
        {
          if (findMatchingLTRP(pcSlice, &ltrpIndex, rps->getPOC(k), rps->getUsed(k)))
          {
            ltrpInSPS[numLtrpInSPS] = ltrpIndex;
            numLtrpInSPS++;
          }
          else
          {
            counter++;
          }
        }
        numLtrpInSH -= numLtrpInSPS;
        // check that either all long-term pictures are coded in SPS or in slice header (no mixing)
        assert (numLtrpInSH==0 || numLtrpInSPS==0); 

        Int bitsForLtrpInSPS = 0;
        while (pcSlice->getSPS()->getNumLongTermRefPicSPS() > (1 << bitsForLtrpInSPS))
        {
          bitsForLtrpInSPS++;
        }
        if (pcSlice->getSPS()->getNumLongTermRefPicSPS() > 0)
        {
          WRITE_UVLC( numLtrpInSPS, "num_long_term_sps");
        }
        WRITE_UVLC( numLtrpInSH, "num_long_term_pics");
        // Note that the LSBs of the LT ref. pic. POCs must be sorted before.
        // Not sorted here because LT ref indices will be used in setRefPicList()
        Int prevDeltaMSB = 0, prevLSB = 0;
        Int offset = rps->getNumberOfNegativePictures() + rps->getNumberOfPositivePictures();
        counter = 0;
        // Warning: If some pictures are moved to ltrpInSPS, i is referring to a wrong index 
        //          (mapping would be required)
        for(Int i=rps->getNumberOfPictures()-1 ; i > offset-1; i--, counter++)
        {
          if (counter < numLtrpInSPS)
          {
            if (bitsForLtrpInSPS > 0)
            {
              WRITE_CODE( ltrpInSPS[counter], bitsForLtrpInSPS, "lt_idx_sps[i]");
            }
          }
          else
          {
            WRITE_CODE( rps->getPocLSBLT(i), pcSlice->getSPS()->getBitsForPOC(), "poc_lsb_lt");
            WRITE_FLAG( rps->getUsed(i), "used_by_curr_pic_lt_flag");
          }
          WRITE_FLAG( rps->getDeltaPocMSBPresentFlag(i), "delta_poc_msb_present_flag");

          if(rps->getDeltaPocMSBPresentFlag(i))
          {
            Bool deltaFlag = false;
            //  First LTRP from SPS                 ||  First LTRP from SH                              || curr LSB            != prev LSB
            if( (i == rps->getNumberOfPictures()-1) || (i == rps->getNumberOfPictures()-1-numLtrpInSPS) || (rps->getPocLSBLT(i) != prevLSB) )
            {
              deltaFlag = true;
            }
            if(deltaFlag)
            {
              WRITE_UVLC( rps->getDeltaPocMSBCycleLT(i), "delta_poc_msb_cycle_lt[i]" );
            }
            else
            {
              Int differenceInDeltaMSB = rps->getDeltaPocMSBCycleLT(i) - prevDeltaMSB;
              assert(differenceInDeltaMSB >= 0);
              WRITE_UVLC( differenceInDeltaMSB, "delta_poc_msb_cycle_lt[i]" );
            }
            prevLSB = rps->getPocLSBLT(i);
            prevDeltaMSB = rps->getDeltaPocMSBCycleLT(i);
          }
        }
      }
      if (pcSlice->getSPS()->getSPSTemporalMVPEnabledFlag())
      {
        WRITE_FLAG( pcSlice->getEnableTMVPFlag() ? 1 : 0, "slice_temporal_mvp_enabled_flag" );
      }
    }

#if SVC_EXTENSION
    if((pcSlice->getLayerId() > 0) && !(pcSlice->getVPS()->getDefaultRefLayersActiveFlag()) && (pcSlice->getNumILRRefIdx() > 0) )
    {
      WRITE_FLAG(pcSlice->getInterLayerPredEnabledFlag(),"inter_layer_pred_enabled_flag");
      if( pcSlice->getInterLayerPredEnabledFlag())
      {
        if(pcSlice->getNumILRRefIdx() > 1)
        {
          Int numBits = 1;
          while ((1 << numBits) < pcSlice->getNumILRRefIdx())
          {
            numBits++;
          }
          if( !pcSlice->getVPS()->getMaxOneActiveRefLayerFlag()) 
          {
            WRITE_CODE(pcSlice->getActiveNumILRRefIdx() - 1, numBits,"num_inter_layer_ref_pics_minus1");
          }       

          if( pcSlice->getNumILRRefIdx() != pcSlice->getActiveNumILRRefIdx() )
          {
            for(Int i = 0; i < pcSlice->getActiveNumILRRefIdx(); i++ )
            {
              WRITE_CODE(pcSlice->getInterLayerPredLayerIdc(i),numBits,"inter_layer_pred_layer_idc[i]");   
            }
          }
        }
      }
    }     
#endif //SVC_EXTENSION

    if(pcSlice->getSPS()->getUseSAO())
    {
       WRITE_FLAG( pcSlice->getSaoEnabledFlag(CHANNEL_TYPE_LUMA), "slice_sao_luma_flag" );
       if (chromaEnabled)
       {
         WRITE_FLAG( pcSlice->getSaoEnabledFlag(CHANNEL_TYPE_CHROMA), "slice_sao_chroma_flag" );
       }
    }

    //check if numrefidxes match the defaults. If not, override

    if (!pcSlice->isIntra())
    {
      Bool overrideFlag = (pcSlice->getNumRefIdx( REF_PIC_LIST_0 )!=pcSlice->getPPS()->getNumRefIdxL0DefaultActive()||(pcSlice->isInterB()&&pcSlice->getNumRefIdx( REF_PIC_LIST_1 )!=pcSlice->getPPS()->getNumRefIdxL1DefaultActive()));
      WRITE_FLAG( overrideFlag ? 1 : 0,                               "num_ref_idx_active_override_flag");
      if (overrideFlag)
      {
        WRITE_UVLC( pcSlice->getNumRefIdx( REF_PIC_LIST_0 ) - 1,      "num_ref_idx_l0_active_minus1" );
        if (pcSlice->isInterB())
        {
          WRITE_UVLC( pcSlice->getNumRefIdx( REF_PIC_LIST_1 ) - 1,    "num_ref_idx_l1_active_minus1" );
        }
        else
        {
          pcSlice->setNumRefIdx(REF_PIC_LIST_1, 0);
        }
      }
    }
    else
    {
      pcSlice->setNumRefIdx(REF_PIC_LIST_0, 0);
      pcSlice->setNumRefIdx(REF_PIC_LIST_1, 0);
    }

    if( pcSlice->getPPS()->getListsModificationPresentFlag() && pcSlice->getNumRpsCurrTempList() > 1)
    {
      TComRefPicListModification* refPicListModification = pcSlice->getRefPicListModification();
      if(!pcSlice->isIntra())
      {
        WRITE_FLAG(pcSlice->getRefPicListModification()->getRefPicListModificationFlagL0() ? 1 : 0,       "ref_pic_list_modification_flag_l0" );
        if (pcSlice->getRefPicListModification()->getRefPicListModificationFlagL0())
        {
          Int numRpsCurrTempList0 = pcSlice->getNumRpsCurrTempList();
          if (numRpsCurrTempList0 > 1)
          {
            Int length = 1;
            numRpsCurrTempList0 --;
            while ( numRpsCurrTempList0 >>= 1)
            {
              length ++;
            }
            for(Int i = 0; i < pcSlice->getNumRefIdx( REF_PIC_LIST_0 ); i++)
            {
              WRITE_CODE( refPicListModification->getRefPicSetIdxL0(i), length, "list_entry_l0");
            }
          }
        }
      }
      if(pcSlice->isInterB())
      {
        WRITE_FLAG(pcSlice->getRefPicListModification()->getRefPicListModificationFlagL1() ? 1 : 0,       "ref_pic_list_modification_flag_l1" );
        if (pcSlice->getRefPicListModification()->getRefPicListModificationFlagL1())
        {
          Int numRpsCurrTempList1 = pcSlice->getNumRpsCurrTempList();
          if ( numRpsCurrTempList1 > 1 )
          {
            Int length = 1;
            numRpsCurrTempList1 --;
            while ( numRpsCurrTempList1 >>= 1)
            {
              length ++;
            }
            for(Int i = 0; i < pcSlice->getNumRefIdx( REF_PIC_LIST_1 ); i++)
            {
              WRITE_CODE( refPicListModification->getRefPicSetIdxL1(i), length, "list_entry_l1");
            }
          }
        }
      }
    }

    if (pcSlice->isInterB())
    {
      WRITE_FLAG( pcSlice->getMvdL1ZeroFlag() ? 1 : 0,   "mvd_l1_zero_flag");
    }

    if(!pcSlice->isIntra())
    {
      if (!pcSlice->isIntra() && pcSlice->getPPS()->getCabacInitPresentFlag())
      {
        SliceType sliceType   = pcSlice->getSliceType();
        SliceType  encCABACTableIdx = pcSlice->getEncCABACTableIdx();
        Bool encCabacInitFlag = (sliceType!=encCABACTableIdx && encCABACTableIdx!=I_SLICE) ? true : false;
        pcSlice->setCabacInitFlag( encCabacInitFlag );
        WRITE_FLAG( encCabacInitFlag?1:0, "cabac_init_flag" );
      }
    }

    if ( pcSlice->getEnableTMVPFlag() )
    {
      if ( pcSlice->getSliceType() == B_SLICE )
      {
        WRITE_FLAG( pcSlice->getColFromL0Flag(), "collocated_from_l0_flag" );
      }

      if ( pcSlice->getSliceType() != I_SLICE &&
        ((pcSlice->getColFromL0Flag()==1 && pcSlice->getNumRefIdx(REF_PIC_LIST_0)>1)||
        (pcSlice->getColFromL0Flag()==0  && pcSlice->getNumRefIdx(REF_PIC_LIST_1)>1)))
      {
        WRITE_UVLC( pcSlice->getColRefIdx(), "collocated_ref_idx" );
      }
    }
    if ( (pcSlice->getPPS()->getUseWP() && pcSlice->getSliceType()==P_SLICE) || (pcSlice->getPPS()->getWPBiPred() && pcSlice->getSliceType()==B_SLICE) )
    {
      xCodePredWeightTable( pcSlice );
    }
    assert(pcSlice->getMaxNumMergeCand()<=MRG_MAX_NUM_CANDS);
    if (!pcSlice->isIntra())
    {
      WRITE_UVLC(MRG_MAX_NUM_CANDS - pcSlice->getMaxNumMergeCand(), "five_minus_max_num_merge_cand");
    }
    Int iCode = pcSlice->getSliceQp() - ( pcSlice->getPPS()->getPicInitQPMinus26() + 26 );
    WRITE_SVLC( iCode, "slice_qp_delta" );
    if (pcSlice->getPPS()->getSliceChromaQpFlag())
    {
      if (numberValidComponents > COMPONENT_Cb)
      {
        WRITE_SVLC( pcSlice->getSliceChromaQpDelta(COMPONENT_Cb), "slice_cb_qp_offset" );
      }
      if (numberValidComponents > COMPONENT_Cr)
      {
        WRITE_SVLC( pcSlice->getSliceChromaQpDelta(COMPONENT_Cr), "slice_cr_qp_offset" );
      }
      assert(numberValidComponents <= COMPONENT_Cr+1);
    }

    if (pcSlice->getPPS()->getPpsRangeExtension().getChromaQpOffsetListEnabledFlag())
    {
      WRITE_FLAG(pcSlice->getUseChromaQpAdj(), "cu_chroma_qp_offset_enabled_flag");
    }

    if (pcSlice->getPPS()->getDeblockingFilterControlPresentFlag())
    {
      if (pcSlice->getPPS()->getDeblockingFilterOverrideEnabledFlag() )
      {
        WRITE_FLAG(pcSlice->getDeblockingFilterOverrideFlag(), "deblocking_filter_override_flag");
      }
      if (pcSlice->getDeblockingFilterOverrideFlag())
      {
        WRITE_FLAG(pcSlice->getDeblockingFilterDisable(), "slice_deblocking_filter_disabled_flag");
        if(!pcSlice->getDeblockingFilterDisable())
        {
          WRITE_SVLC (pcSlice->getDeblockingFilterBetaOffsetDiv2(), "slice_beta_offset_div2");
          WRITE_SVLC (pcSlice->getDeblockingFilterTcOffsetDiv2(),   "slice_tc_offset_div2");
        }
      }
    }

    Bool isSAOEnabled = pcSlice->getSPS()->getUseSAO() && (pcSlice->getSaoEnabledFlag(CHANNEL_TYPE_LUMA) || (chromaEnabled && pcSlice->getSaoEnabledFlag(CHANNEL_TYPE_CHROMA)));
    Bool isDBFEnabled = (!pcSlice->getDeblockingFilterDisable());

    if(pcSlice->getPPS()->getLoopFilterAcrossSlicesEnabledFlag() && ( isSAOEnabled || isDBFEnabled ))
    {
      WRITE_FLAG(pcSlice->getLFCrossSliceBoundaryFlag()?1:0, "slice_loop_filter_across_slices_enabled_flag");
    }
  }
#if !SVC_EXTENSION
  if(pcSlice->getPPS()->getSliceHeaderExtensionPresentFlag())
  {
    WRITE_UVLC(0,"slice_segment_header_extension_length");
  }
#endif
}

Void TEncCavlc::codePTL( const TComPTL* pcPTL, Bool profilePresentFlag, Int maxNumSubLayersMinus1)
{
  if(profilePresentFlag)
  {
    codeProfileTier(pcPTL->getGeneralPTL(), false);    // general_...
  }
  WRITE_CODE( Int(pcPTL->getGeneralPTL()->getLevelIdc()), 8, "general_level_idc" );

  for (Int i = 0; i < maxNumSubLayersMinus1; i++)
  {
    WRITE_FLAG( pcPTL->getSubLayerProfilePresentFlag(i), "sub_layer_profile_present_flag[i]" );
    WRITE_FLAG( pcPTL->getSubLayerLevelPresentFlag(i),   "sub_layer_level_present_flag[i]" );
  }

  if (maxNumSubLayersMinus1 > 0)
  {
    for (Int i = maxNumSubLayersMinus1; i < 8; i++)
    {
      WRITE_CODE(0, 2, "reserved_zero_2bits");
    }
  }

  for(Int i = 0; i < maxNumSubLayersMinus1; i++)
  {
    if( pcPTL->getSubLayerProfilePresentFlag(i) )
    {
      codeProfileTier(pcPTL->getSubLayerPTL(i), true);  // sub_layer_...
    }
    if( pcPTL->getSubLayerLevelPresentFlag(i) )
    {
      WRITE_CODE( Int(pcPTL->getSubLayerPTL(i)->getLevelIdc()), 8, "sub_layer_level_idc[i]" );
    }
  }
}

#if ENC_DEC_TRACE || RExt__DECODER_DEBUG_BIT_STATISTICS
Void TEncCavlc::codeProfileTier( const ProfileTierLevel* ptl, const Bool bIsSubLayer )
#define PTL_TRACE_TEXT(txt) bIsSubLayer?("sub_layer_" txt) : ("general_" txt)
#else
Void TEncCavlc::codeProfileTier( const ProfileTierLevel* ptl, const Bool /*bIsSubLayer*/ )
#define PTL_TRACE_TEXT(txt) txt
#endif
{
  WRITE_CODE( ptl->getProfileSpace(), 2 ,      PTL_TRACE_TEXT("profile_space"                   ));
  WRITE_FLAG( ptl->getTierFlag()==Level::HIGH, PTL_TRACE_TEXT("tier_flag"                       ));
#if SVC_EXTENSION
  WRITE_CODE( (ptl->getProfileIdc() == Profile::SCALABLEMAIN || ptl->getProfileIdc() == Profile::SCALABLEMAIN10) ? 7 : Int(ptl->getProfileIdc()), 5 ,  PTL_TRACE_TEXT("profile_idc")  );
#else
  WRITE_CODE( Int(ptl->getProfileIdc()), 5 ,   PTL_TRACE_TEXT("profile_idc"                     ));
#endif
  for(Int j = 0; j < 32; j++)
  {
    WRITE_FLAG( ptl->getProfileCompatibilityFlag(j), PTL_TRACE_TEXT("profile_compatibility_flag[][j]" ));
  }

  WRITE_FLAG(ptl->getProgressiveSourceFlag(),   PTL_TRACE_TEXT("progressive_source_flag"         ));
  WRITE_FLAG(ptl->getInterlacedSourceFlag(),    PTL_TRACE_TEXT("interlaced_source_flag"          ));
  WRITE_FLAG(ptl->getNonPackedConstraintFlag(), PTL_TRACE_TEXT("non_packed_constraint_flag"      ));
  WRITE_FLAG(ptl->getFrameOnlyConstraintFlag(), PTL_TRACE_TEXT("frame_only_constraint_flag"      ));

  if (ptl->getProfileIdc() == Profile::MAINREXT || ptl->getProfileIdc() == Profile::HIGHTHROUGHPUTREXT 
#if SCALABLE_REXT
    || ptl->getProfileIdc() == Profile::SCALABLEREXT
#endif
    )
  {
    const UInt         bitDepthConstraint=ptl->getBitDepthConstraint();
    WRITE_FLAG(bitDepthConstraint<=12,          PTL_TRACE_TEXT("max_12bit_constraint_flag"       ));
    WRITE_FLAG(bitDepthConstraint<=10,          PTL_TRACE_TEXT("max_10bit_constraint_flag"       ));
    WRITE_FLAG(bitDepthConstraint<= 8,          PTL_TRACE_TEXT("max_8bit_constraint_flag"        ));
    const ChromaFormat chromaFmtConstraint=ptl->getChromaFormatConstraint();
    WRITE_FLAG(chromaFmtConstraint==CHROMA_422||chromaFmtConstraint==CHROMA_420||chromaFmtConstraint==CHROMA_400, PTL_TRACE_TEXT("max_422chroma_constraint_flag" ));
    WRITE_FLAG(chromaFmtConstraint==CHROMA_420||chromaFmtConstraint==CHROMA_400,                                  PTL_TRACE_TEXT("max_420chroma_constraint_flag" ));
    WRITE_FLAG(chromaFmtConstraint==CHROMA_400,                                                                   PTL_TRACE_TEXT("max_monochrome_constraint_flag"));
    WRITE_FLAG(ptl->getIntraConstraintFlag(),          PTL_TRACE_TEXT("intra_constraint_flag"           ));
    WRITE_FLAG(ptl->getOnePictureOnlyConstraintFlag(), PTL_TRACE_TEXT("one_picture_only_constraint_flag"));
    WRITE_FLAG(ptl->getLowerBitRateConstraintFlag(),   PTL_TRACE_TEXT("lower_bit_rate_constraint_flag"  ));
#if SVC_EXTENSION
    WRITE_CODE(0, 32,  "general_reserved_zero_34bits");  WRITE_CODE(0, 2,  "general_reserved_zero_34bits");
  }
  else if( ptl->getProfileIdc() == Profile::SCALABLEMAIN || ptl->getProfileIdc() == Profile::SCALABLEMAIN10 )      // at encoder side, scalable-main10 profile has a profile idc equal to 8, which is converted to 7 during signalling
  {
    WRITE_FLAG(true,   "general_max_12bit_constraint_flag");
    WRITE_FLAG(true,   "general_max_10bit_constraint_flag");
    WRITE_FLAG((ptl->getProfileIdc() == Profile::SCALABLEMAIN) ? true : false, "general_max_8bit_constraint_flag");
    WRITE_FLAG(true,   "general_max_422chroma_constraint_flag");
    WRITE_FLAG(true,   "general_max_420chroma_constraint_flag");
    WRITE_FLAG(false,  "general_max_monochrome_constraint_flag");
    WRITE_FLAG(false,  "general_intra_constraint_flag");
    WRITE_FLAG(false,  "general_one_picture_only_constraint_flag");
    WRITE_FLAG(true,   "general_lower_bit_rate_constraint_flag");
    WRITE_CODE(0, 32,  "general_reserved_zero_34bits");  WRITE_CODE(0, 2,  "general_reserved_zero_34bits");
  }
#if VIEW_SCALABILITY
  else if( ptl->getProfileIdc() == Profile::MULTIVIEWMAIN )
  {
    WRITE_FLAG(true,   "general_max_12bit_constraint_flag");
    WRITE_FLAG(true,   "general_max_10bit_constraint_flag");
    WRITE_FLAG(true,   "general_max_8bit_constraint_flag");
    WRITE_FLAG(true,   "general_max_422chroma_constraint_flag");
    WRITE_FLAG(true,   "general_max_420chroma_constraint_flag");
    WRITE_FLAG(false,  "general_max_monochrome_constraint_flag");
    WRITE_FLAG(false,  "general_intra_constraint_flag");
    WRITE_FLAG(false,  "general_one_picture_only_constraint_flag");
    WRITE_FLAG(true,   "general_lower_bit_rate_constraint_flag");
    WRITE_CODE(0, 32,  "general_reserved_zero_34bits");  WRITE_CODE(0, 2,  "general_reserved_zero_34bits");
  }
#endif
  else
  {
    WRITE_CODE(0, 32,  "general_reserved_zero_43bits");  WRITE_CODE(0, 11,  "general_reserved_zero_43bits");
  }
#else
    WRITE_CODE(0 , 16, PTL_TRACE_TEXT("reserved_zero_34bits[0..15]"     ));
    WRITE_CODE(0 , 16, PTL_TRACE_TEXT("reserved_zero_34bits[16..31]"    ));
    WRITE_CODE(0 ,  2, PTL_TRACE_TEXT("reserved_zero_34bits[32..33]"    ));
  }
  else
  {
    WRITE_CODE(0x0000 , 16, PTL_TRACE_TEXT("reserved_zero_43bits[0..15]"     ));
    WRITE_CODE(0x0000 , 16, PTL_TRACE_TEXT("reserved_zero_43bits[16..31]"    ));
    WRITE_CODE(0x000  , 11, PTL_TRACE_TEXT("reserved_zero_43bits[32..42]"    ));
  }
#endif
  WRITE_FLAG(false,   PTL_TRACE_TEXT("inbld_flag" ));
#undef PTL_TRACE_TEXT
}

/**
 * Write tiles and wavefront substreams sizes for the slice header (entry points).
 *
 * \param pSlice TComSlice structure that contains the substream size information.
 */
Void  TEncCavlc::codeTilesWPPEntryPoint( TComSlice* pSlice )
{
  if (!pSlice->getPPS()->getTilesEnabledFlag() && !pSlice->getPPS()->getEntropyCodingSyncEnabledFlag())
  {
    return;
  }
  UInt maxOffset = 0;
  for(Int idx=0; idx<pSlice->getNumberOfSubstreamSizes(); idx++)
  {
    UInt offset=pSlice->getSubstreamSize(idx);
    if ( offset > maxOffset )
    {
      maxOffset = offset;
    }
  }

  // Determine number of bits "offsetLenMinus1+1" required for entry point information
  UInt offsetLenMinus1 = 0;
  while (maxOffset >= (1u << (offsetLenMinus1 + 1)))
  {
    offsetLenMinus1++;
    assert(offsetLenMinus1 + 1 < 32);
  }

  WRITE_UVLC(pSlice->getNumberOfSubstreamSizes(), "num_entry_point_offsets");
  if (pSlice->getNumberOfSubstreamSizes()>0)
  {
    WRITE_UVLC(offsetLenMinus1, "offset_len_minus1");

    for (UInt idx=0; idx<pSlice->getNumberOfSubstreamSizes(); idx++)
    {
      WRITE_CODE(pSlice->getSubstreamSize(idx)-1, offsetLenMinus1+1, "entry_point_offset_minus1");
    }
  }
}

Void TEncCavlc::codeTerminatingBit      ( UInt /*uilsLast*/ )
{
}

Void TEncCavlc::codeSliceFinish ()
{
}

Void TEncCavlc::codeMVPIdx ( TComDataCU* /*pcCU*/, UInt /*uiAbsPartIdx*/, RefPicList /*eRefList*/ )
{
  assert(0);
}

Void TEncCavlc::codePartSize( TComDataCU* /*pcCU*/, UInt /*uiAbsPartIdx*/, UInt /*uiDepth*/ )
{
  assert(0);
}

Void TEncCavlc::codePredMode( TComDataCU* /*pcCU*/, UInt /*uiAbsPartIdx*/ )
{
  assert(0);
}

Void TEncCavlc::codeMergeFlag    ( TComDataCU* /*pcCU*/, UInt /*uiAbsPartIdx*/ )
{
  assert(0);
}

Void TEncCavlc::codeMergeIndex    ( TComDataCU* /*pcCU*/, UInt /*uiAbsPartIdx*/ )
{
  assert(0);
}

Void TEncCavlc::codeInterModeFlag( TComDataCU* /*pcCU*/, UInt /*uiAbsPartIdx*/, UInt /*uiDepth*/, UInt /*uiEncMode*/ )
{
  assert(0);
}

Void TEncCavlc::codeCUTransquantBypassFlag( TComDataCU* /*pcCU*/, UInt /*uiAbsPartIdx*/ )
{
  assert(0);
}

Void TEncCavlc::codeSkipFlag( TComDataCU* /*pcCU*/, UInt /*uiAbsPartIdx*/ )
{
  assert(0);
}

Void TEncCavlc::codeSplitFlag   ( TComDataCU* /*pcCU*/, UInt /*uiAbsPartIdx*/, UInt /*uiDepth*/ )
{
  assert(0);
}

Void TEncCavlc::codeTransformSubdivFlag( UInt /*uiSymbol*/, UInt /*uiCtx*/ )
{
  assert(0);
}

Void TEncCavlc::codeQtCbf( TComTU& /*rTu*/, const ComponentID /*compID*/, const Bool /*lowestLevel*/ )
{
  assert(0);
}

Void TEncCavlc::codeQtRootCbf( TComDataCU* /*pcCU*/, UInt /*uiAbsPartIdx*/ )
{
  assert(0);
}

Void TEncCavlc::codeQtCbfZero( TComTU& /*rTu*/, const ChannelType /*chType*/ )
{
  assert(0);
}
Void TEncCavlc::codeQtRootCbfZero( )
{
  assert(0);
}

Void TEncCavlc::codeTransformSkipFlags (TComTU& /*rTu*/, ComponentID /*component*/ )
{
  assert(0);
}

/** Code I_PCM information.
 * \param pcCU pointer to CU
 * \param uiAbsPartIdx CU index
 * \returns Void
 */
Void TEncCavlc::codeIPCMInfo( TComDataCU* /*pcCU*/, UInt /*uiAbsPartIdx*/ )
{
  assert(0);
}

Void TEncCavlc::codeIntraDirLumaAng( TComDataCU* /*pcCU*/, UInt /*uiAbsPartIdx*/, Bool /*isMultiple*/)
{
  assert(0);
}

Void TEncCavlc::codeIntraDirChroma( TComDataCU* /*pcCU*/, UInt /*uiAbsPartIdx*/ )
{
  assert(0);
}

Void TEncCavlc::codeInterDir( TComDataCU* /*pcCU*/, UInt /*uiAbsPartIdx*/ )
{
  assert(0);
}

Void TEncCavlc::codeRefFrmIdx( TComDataCU* /*pcCU*/, UInt /*uiAbsPartIdx*/, RefPicList /*eRefList*/ )
{
  assert(0);
}

Void TEncCavlc::codeMvd( TComDataCU* /*pcCU*/, UInt /*uiAbsPartIdx*/, RefPicList /*eRefList*/ )
{
  assert(0);
}

Void TEncCavlc::codeCrossComponentPrediction( TComTU& /*rTu*/, ComponentID /*compID*/ )
{
  assert(0);
}

Void TEncCavlc::codeDeltaQP( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  Int iDQp  = pcCU->getQP( uiAbsPartIdx ) - pcCU->getRefQP( uiAbsPartIdx );

  Int qpBdOffsetY =  pcCU->getSlice()->getSPS()->getQpBDOffset(CHANNEL_TYPE_LUMA);
  iDQp = (iDQp + 78 + qpBdOffsetY + (qpBdOffsetY/2)) % (52 + qpBdOffsetY) - 26 - (qpBdOffsetY/2);

  xWriteSvlc( iDQp );

  return;
}

Void TEncCavlc::codeChromaQpAdjustment( TComDataCU* /*pcCU*/, UInt /*uiAbsPartIdx*/ )
{
  assert(0);
}

Void TEncCavlc::codeCoeffNxN    ( TComTU& /*rTu*/, TCoeff* /*pcCoef*/, const ComponentID /*compID*/ )
{
  assert(0);
}

Void TEncCavlc::estBit( estBitsSbacStruct* /*pcEstBitsCabac*/, Int /*width*/, Int /*height*/, ChannelType /*chType*/ )
{
  // printf("error : no VLC mode support in this version\n");
  return;
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

//! Code weighted prediction tables
Void TEncCavlc::xCodePredWeightTable( TComSlice* pcSlice )
{
  WPScalingParam  *wp;
  const ChromaFormat    format                = pcSlice->getPic()->getChromaFormat();
  const UInt            numberValidComponents = getNumberValidComponents(format);
  const Bool            bChroma               = isChromaEnabled(format);
  const Int             iNbRef                = (pcSlice->getSliceType() == B_SLICE ) ? (2) : (1);
        Bool            bDenomCoded           = false;
        UInt            uiTotalSignalledWeightFlags = 0;

  if ( (pcSlice->getSliceType()==P_SLICE && pcSlice->getPPS()->getUseWP()) || (pcSlice->getSliceType()==B_SLICE && pcSlice->getPPS()->getWPBiPred()) )
  {
    for ( Int iNumRef=0 ; iNumRef<iNbRef ; iNumRef++ ) // loop over l0 and l1 syntax elements
    {
      RefPicList  eRefPicList = ( iNumRef ? REF_PIC_LIST_1 : REF_PIC_LIST_0 );

      // NOTE: wp[].uiLog2WeightDenom and wp[].bPresentFlag are actually per-channel-type settings.

      for ( Int iRefIdx=0 ; iRefIdx<pcSlice->getNumRefIdx(eRefPicList) ; iRefIdx++ )
      {
        pcSlice->getWpScaling(eRefPicList, iRefIdx, wp);
        if ( !bDenomCoded )
        {
          Int iDeltaDenom;
          WRITE_UVLC( wp[COMPONENT_Y].uiLog2WeightDenom, "luma_log2_weight_denom" );

          if( bChroma )
          {
            assert(wp[COMPONENT_Cb].uiLog2WeightDenom == wp[COMPONENT_Cr].uiLog2WeightDenom); // check the channel-type settings are consistent across components.
            iDeltaDenom = (wp[COMPONENT_Cb].uiLog2WeightDenom - wp[COMPONENT_Y].uiLog2WeightDenom);
            WRITE_SVLC( iDeltaDenom, "delta_chroma_log2_weight_denom" );
          }
          bDenomCoded = true;
        }
        WRITE_FLAG( wp[COMPONENT_Y].bPresentFlag, iNumRef==0?"luma_weight_l0_flag[i]":"luma_weight_l1_flag[i]" );
        uiTotalSignalledWeightFlags += wp[COMPONENT_Y].bPresentFlag;
      }
      if (bChroma)
      {
        for ( Int iRefIdx=0 ; iRefIdx<pcSlice->getNumRefIdx(eRefPicList) ; iRefIdx++ )
        {
          pcSlice->getWpScaling(eRefPicList, iRefIdx, wp);
          assert(wp[COMPONENT_Cb].bPresentFlag == wp[COMPONENT_Cr].bPresentFlag); // check the channel-type settings are consistent across components.
          WRITE_FLAG( wp[COMPONENT_Cb].bPresentFlag, iNumRef==0?"chroma_weight_l0_flag[i]":"chroma_weight_l1_flag[i]" );
          uiTotalSignalledWeightFlags += 2*wp[COMPONENT_Cb].bPresentFlag;
        }
      }

      for ( Int iRefIdx=0 ; iRefIdx<pcSlice->getNumRefIdx(eRefPicList) ; iRefIdx++ )
      {
        pcSlice->getWpScaling(eRefPicList, iRefIdx, wp);
        if ( wp[COMPONENT_Y].bPresentFlag )
        {
          Int iDeltaWeight = (wp[COMPONENT_Y].iWeight - (1<<wp[COMPONENT_Y].uiLog2WeightDenom));
          WRITE_SVLC( iDeltaWeight, iNumRef==0?"delta_luma_weight_l0[i]":"delta_luma_weight_l1[i]" );
          WRITE_SVLC( wp[COMPONENT_Y].iOffset, iNumRef==0?"luma_offset_l0[i]":"luma_offset_l1[i]" );
        }

        if ( bChroma )
        {
          if ( wp[COMPONENT_Cb].bPresentFlag )
          {
            for ( Int j = COMPONENT_Cb ; j < numberValidComponents ; j++ )
            {
              assert(wp[COMPONENT_Cb].uiLog2WeightDenom == wp[COMPONENT_Cr].uiLog2WeightDenom);
              Int iDeltaWeight = (wp[j].iWeight - (1<<wp[COMPONENT_Cb].uiLog2WeightDenom));
              WRITE_SVLC( iDeltaWeight, iNumRef==0?"delta_chroma_weight_l0[i]":"delta_chroma_weight_l1[i]" );

              Int range=pcSlice->getSPS()->getSpsRangeExtension().getHighPrecisionOffsetsEnabledFlag() ? (1<<pcSlice->getSPS()->getBitDepth(CHANNEL_TYPE_CHROMA))/2 : 128;
              Int pred = ( range - ( ( range*wp[j].iWeight)>>(wp[j].uiLog2WeightDenom) ) );
              Int iDeltaChroma = (wp[j].iOffset - pred);
              WRITE_SVLC( iDeltaChroma, iNumRef==0?"delta_chroma_offset_l0[i]":"delta_chroma_offset_l1[i]" );
            }
          }
        }
      }
    }
    assert(uiTotalSignalledWeightFlags<=24);
  }
}

/** code quantization matrix
 *  \param scalingList quantization matrix information
 */
Void TEncCavlc::codeScalingList( const TComScalingList &scalingList )
{
  //for each size
  for(UInt sizeId = 0; sizeId < SCALING_LIST_SIZE_NUM; sizeId++)
  {
    const Int predListStep = (sizeId == SCALING_LIST_32x32? (SCALING_LIST_NUM/NUMBER_OF_PREDICTION_MODES) : 1); // if 32x32, skip over chroma entries.

    for(UInt listId = 0; listId < SCALING_LIST_NUM; listId+=predListStep)
    {
      Bool scalingListPredModeFlag = scalingList.getScalingListPredModeFlag(sizeId, listId);
      WRITE_FLAG( scalingListPredModeFlag, "scaling_list_pred_mode_flag" );
      if(!scalingListPredModeFlag)// Copy Mode
      {
        if (sizeId == SCALING_LIST_32x32)
        {
          // adjust the code, to cope with the missing chroma entries
          WRITE_UVLC( ((Int)listId - (Int)scalingList.getRefMatrixId (sizeId,listId)) / (SCALING_LIST_NUM/NUMBER_OF_PREDICTION_MODES), "scaling_list_pred_matrix_id_delta");
        }
        else
        {
          WRITE_UVLC( (Int)listId - (Int)scalingList.getRefMatrixId (sizeId,listId), "scaling_list_pred_matrix_id_delta");
        }
      }
      else// DPCM Mode
      {
        xCodeScalingList(&scalingList, sizeId, listId);
      }
    }
  }
  return;
}
/** code DPCM
 * \param scalingList quantization matrix information
 * \param sizeId      size index
 * \param listId      list index
 */
Void TEncCavlc::xCodeScalingList(const TComScalingList* scalingList, UInt sizeId, UInt listId)
{
  Int coefNum = min(MAX_MATRIX_COEF_NUM,(Int)g_scalingListSize[sizeId]);
  UInt* scan  = g_scanOrder[SCAN_UNGROUPED][SCAN_DIAG][sizeId==0 ? 2 : 3][sizeId==0 ? 2 : 3];
  Int nextCoef = SCALING_LIST_START_VALUE;
  Int data;
  const Int *src = scalingList->getScalingListAddress(sizeId, listId);
  if( sizeId > SCALING_LIST_8x8 )
  {
    WRITE_SVLC( scalingList->getScalingListDC(sizeId,listId) - 8, "scaling_list_dc_coef_minus8");
    nextCoef = scalingList->getScalingListDC(sizeId,listId);
  }
  for(Int i=0;i<coefNum;i++)
  {
    data = src[scan[i]] - nextCoef;
    nextCoef = src[scan[i]];
    if(data > 127)
    {
      data = data - 256;
    }
    if(data < -128)
    {
      data = data + 256;
    }

    WRITE_SVLC( data,  "scaling_list_delta_coef");
  }
}

Bool TEncCavlc::findMatchingLTRP ( TComSlice* pcSlice, UInt *ltrpsIndex, Int ltrpPOC, Bool usedFlag )
{
  // Bool state = true, state2 = false;
  Int lsb = ltrpPOC & ((1<<pcSlice->getSPS()->getBitsForPOC())-1);
  for (Int k = 0; k < pcSlice->getSPS()->getNumLongTermRefPicSPS(); k++)
  {
    if ( (lsb == pcSlice->getSPS()->getLtRefPicPocLsbSps(k)) && (usedFlag == pcSlice->getSPS()->getUsedByCurrPicLtSPSFlag(k)) )
    {
      *ltrpsIndex = k;
      return true;
    }
  }
  return false;
}

Void TEncCavlc::codeExplicitRdpcmMode( TComTU& /*rTu*/, const ComponentID /*compID*/ )
 {
   assert(0);
 }

#if SVC_EXTENSION
Void TEncCavlc::codeSliceHeaderExtn( TComSlice* slice, Int shBitsWrittenTillNow )
{
  Int tmpBitsBeforeWriting = getNumberOfWrittenBits();
  Int maxPocLsb = 1 << slice->getSPS()->getBitsForPOC();
  if(slice->getPPS()->getSliceHeaderExtensionPresentFlag())
  {
    // Derive the value of PocMsbValRequiredFlag
    slice->setPocMsbValRequiredFlag( (slice->getCraPicFlag() || slice->getBlaPicFlag())
                                  && (!slice->getVPS()->getVpsPocLsbAlignedFlag() ||
                                      (slice->getVPS()->getVpsPocLsbAlignedFlag() && slice->getVPS()->getNumDirectRefLayers(slice->getLayerId()) == 0))
                                   );

    // Determine value of SH extension length.
    Int shExtnLengthInBit = 0;
    if( slice->getPPS()->getPocResetInfoPresentFlag() )
    {
      shExtnLengthInBit += 2;
    }

    if( slice->getPocResetIdc() > 0 )
    {
      shExtnLengthInBit += 6;
    }

    if( slice->getPocResetIdc() == 3 )
    {
      shExtnLengthInBit += (slice->getSPS()->getBitsForPOC() + 1);
    }

    if( !slice->getPocMsbValRequiredFlag() && slice->getVPS()->getVpsPocLsbAlignedFlag() )
    {
      shExtnLengthInBit++;
    }
    else
    {
      if( slice->getPocMsbValRequiredFlag() )
      {
        slice->setPocMsbValPresentFlag( true );
      }
      else
      {
        slice->setPocMsbValPresentFlag( false );
      }
    }

    if( slice->getPocMsbNeeded() )
    {
      slice->setPocMsbValPresentFlag(true);
    }

    if( slice->getPocMsbValPresentFlag() )
    {
      UInt lengthVal = 1;
      UInt tempVal = (slice->getPocMsbVal() / maxPocLsb) + 1;
      assert ( tempVal );
      while( 1 != tempVal )
      {
        tempVal >>= 1;
        lengthVal += 2;
      }
      shExtnLengthInBit += lengthVal;
    }

    Int shExtnAdditionalBits = 0;

    if( shExtnLengthInBit % 8 != 0 )
    {
      shExtnAdditionalBits = 8 - (shExtnLengthInBit % 8);
    }

    Int shExtnLength = (shExtnLengthInBit + shExtnAdditionalBits) / 8;
    WRITE_UVLC( shExtnLength, "slice_header_extension_length" );

    if( slice->getPPS()->getPocResetInfoPresentFlag() )
    {
      WRITE_CODE( slice->getPocResetIdc(), 2,                                 "poc_reset_idc");
    }
    if( slice->getPocResetIdc() > 0 )
    {
      WRITE_CODE( slice->getPocResetPeriodId(), 6,                            "poc_reset_period_id");
    }
    if( slice->getPocResetIdc() == 3 ) 
    {
      WRITE_FLAG( slice->getFullPocResetFlag() ? 1 : 0,                       "full_poc_reset_flag");
      WRITE_CODE( slice->getPocLsbVal(), slice->getSPS()->getBitsForPOC(),  "poc_lsb_val");
    }

    if( !slice->getPocMsbValRequiredFlag() && slice->getVPS()->getVpsPocLsbAlignedFlag() )
    {
      WRITE_FLAG( slice->getPocMsbValPresentFlag(),                           "poc_msb_cycle_val_present_flag" );
    }

    if( slice->getPocMsbValPresentFlag() )
    {
      assert(slice->getPocMsbVal() % maxPocLsb == 0);
      WRITE_UVLC(slice->getPocMsbVal() / maxPocLsb, "poc_msb_cycle_val");
    }

    for(Int i = 0; i < shExtnAdditionalBits; i++)
    {
      WRITE_FLAG( 1, "slice_segment_header_extension_data_bit");
    }
  }

  shBitsWrittenTillNow += ( getNumberOfWrittenBits() - tmpBitsBeforeWriting );
  
  // Slice header byte_alignment() included in xAttachSliceDataToNalUnit
}

Void TEncCavlc::codeVPSExtension( const TComVPS *vps )
{
  Int NumOutputLayersInOutputLayerSet[MAX_VPS_LAYER_SETS_PLUS1];
  Int OlsHighestOutputLayerId[MAX_VPS_LAYER_SETS_PLUS1];

  if( vps->getMaxLayers() > 1 && vps->getBaseLayerInternalFlag() )
  {
    codePTL( vps->getPTL(1), false, vps->getMaxTLayers() - 1 );
  }

  UInt i = 0, j = 0;

  WRITE_FLAG( vps->getSplittingFlag(),                 "splitting_flag" );

  for(i = 0; i < MAX_VPS_NUM_SCALABILITY_TYPES; i++)
  {
    WRITE_FLAG( vps->getScalabilityMask(i),            "scalability_mask_flag[i]" );
  }

  for(j = 0; j < vps->getNumScalabilityTypes() - vps->getSplittingFlag(); j++)
  {
    WRITE_CODE( vps->getDimensionIdLen(j) - 1, 3,      "dimension_id_len_minus1[j]" );
  }

  // The value of dimBitOffset[ NumScalabilityTypes ] is set equal to 6.
  if(vps->getSplittingFlag())
  {
    UInt splDimSum=0;
    for(j = 0; j < vps->getNumScalabilityTypes(); j++)
    {
      splDimSum+=(vps->getDimensionIdLen(j));
    }
    assert(splDimSum<=6);
  }

  WRITE_FLAG( vps->getNuhLayerIdPresentFlag(),         "vps_nuh_layer_id_present_flag" );
  for(i = 1; i < vps->getMaxLayers(); i++)
  {
    if( vps->getNuhLayerIdPresentFlag() )
    {
      WRITE_CODE( vps->getLayerIdInNuh(i),     6,      "layer_id_in_nuh[i]" );
    }

    if( !vps->getSplittingFlag() )
    {
      for(j = 0; j < vps->getNumScalabilityTypes(); j++)
      {
        UInt bits = vps->getDimensionIdLen(j);
        WRITE_CODE( vps->getDimensionId(i, j),   bits,   "dimension_id[i][j]" );
      }
    }
  }

  WRITE_CODE( vps->getViewIdLen( ), 4, "view_id_len" );
#if !VIEW_SCALABILITY
  assert ( vps->getNumViews() >= (1<<vps->getViewIdLen()) );
#endif

  if ( vps->getViewIdLen() > 0 )
  {
    for( i = 0; i < vps->getNumViews(); i++ )
    {
      WRITE_CODE( vps->getViewIdVal( i ), vps->getViewIdLen( ), "view_id_val[i]" );
    }
  }

  for( Int layerCtr = 1; layerCtr < vps->getMaxLayers(); layerCtr++)
  {
    for( Int refLayerCtr = 0; refLayerCtr < layerCtr; refLayerCtr++)
    {
      WRITE_FLAG(vps->getDirectDependencyFlag(layerCtr, refLayerCtr), "direct_dependency_flag[i][j]" );
    }
  }

  if( vps->getNumIndependentLayers() > 1 )
  {
    WRITE_UVLC( vps->getNumAddLayerSets(), "num_add_layer_sets" );

    for( i = 0; i < vps->getNumAddLayerSets(); i++ )
    {
      for( j = 1; j < vps->getNumIndependentLayers(); j++ )
      {
        Int len = 1;
        while( (1 << len) < (vps->getNumLayersInTreePartition(j) + 1) )
        {
          len++;
        }
        WRITE_CODE(vps->getHighestLayerIdxPlus1(i, j), len, "highest_layer_idx_plus1[i][j]");
      }
    }
  }

  WRITE_FLAG( vps->getMaxTSLayersPresentFlag(), "vps_sub_layers_max_minus1_present_flag");
  if( vps->getMaxTSLayersPresentFlag() )
  {
    for( i = 0; i < vps->getMaxLayers(); i++ )
    {
      WRITE_CODE(vps->getMaxTSLayersMinus1(i), 3, "sub_layers_vps_max_minus1[i]" );
    }
  }

   WRITE_FLAG( vps->getMaxTidRefPresentFlag(), "max_tid_ref_present_flag");
   if (vps->getMaxTidRefPresentFlag())
   {
     for( i = 0; i < vps->getMaxLayers() - 1; i++)
     {
       for( j = i+1; j <= vps->getMaxLayers() - 1; j++)
       {
         if(vps->getDirectDependencyFlag(j, i))
         {
           WRITE_CODE(vps->getMaxTidIlRefPicsPlus1(i,j), 3, "max_tid_il_ref_pics_plus1[i][j]" );
         }
       }
     }
   }
   WRITE_FLAG( vps->getDefaultRefLayersActiveFlag(), "default_ref_layers_active_flag" );

  // Profile-tier-level signalling
  WRITE_UVLC( vps->getNumProfileTierLevel() - 1, "vps_num_profile_tier_level_minus1"); 

  Int const numBitsForPtlIdx = vps->calculateLenOfSyntaxElement( vps->getNumProfileTierLevel() );

  //Do something here to make sure the loop is correct to consider base layer internal stuff

  for( Int idx = vps->getBaseLayerInternalFlag() ? 2 : 1; idx < vps->getNumProfileTierLevel(); idx++ )
  {
    WRITE_FLAG( vps->getProfilePresentFlag(idx),       "vps_profile_present_flag[i]" );

    codePTL( vps->getPTL(idx), vps->getProfilePresentFlag(idx), vps->getMaxTLayers() - 1 );
  }

  Int numOutputLayerSets = vps->getNumOutputLayerSets();
  Int numAddOutputLayerSets = numOutputLayerSets - (Int)vps->getNumLayerSets();

  // The value of num_add_olss shall be in the range of 0 to 1023, inclusive.
  assert( numAddOutputLayerSets >= 0 && numAddOutputLayerSets < 1024 );

  if( vps->getNumLayerSets() > 1 )
  {
    WRITE_UVLC( numAddOutputLayerSets, "num_add_olss" );
    WRITE_CODE( vps->getDefaultTargetOutputLayerIdc(), 2, "default_output_layer_idc" );
  }

  for(i = 1; i < numOutputLayerSets; i++)
  {
    Int layerSetIdxForOutputLayerSet = vps->getOutputLayerSetIdx(i);
    if( vps->getNumLayerSets() > 2 && i >= vps->getNumLayerSets() )
    {
      Int numBits = 1;
      while ((1 << numBits) < (vps->getNumLayerSets() - 1))
      {
        numBits++;
      }
      WRITE_CODE( vps->getOutputLayerSetIdx(i) - 1, numBits, "layer_set_idx_for_ols_minus1"); 
    }

    if( i > vps->getVpsNumLayerSetsMinus1() || vps->getDefaultTargetOutputLayerIdc() >= 2 ) //Instead of == 2, >= 2 is used to follow the agreement that value 3 should be interpreted as 2
    {
      for( j = 0; j < vps->getNumLayersInIdList(layerSetIdxForOutputLayerSet); j++ )
      {
        WRITE_FLAG( vps->getOutputLayerFlag(i,j), "output_layer_flag[i][j]");
      }
    }

    for( j = 0; j < vps->getNumLayersInIdList(layerSetIdxForOutputLayerSet); j++ )
    {
      if( vps->getNecessaryLayerFlag(i, j) && (vps->getNumProfileTierLevel() - 1) > 0 )
      {
        WRITE_CODE( vps->getProfileLevelTierIdx(i, j), numBitsForPtlIdx, "profile_tier_level_idx[i]" );
      }
    }

    NumOutputLayersInOutputLayerSet[i] = 0;

    for( j = 0; j < vps->getNumLayersInIdList(layerSetIdxForOutputLayerSet); j++ )
    {
      NumOutputLayersInOutputLayerSet[i] += vps->getOutputLayerFlag(i, j);
      if( vps->getOutputLayerFlag(i, j) )
      {
        OlsHighestOutputLayerId[i] = vps->getLayerSetLayerIdList(layerSetIdxForOutputLayerSet, j);
      }
    }
    if( NumOutputLayersInOutputLayerSet[i] == 1 && vps->getNumDirectRefLayers(OlsHighestOutputLayerId[i]) > 0 )
    {
      WRITE_FLAG(vps->getAltOuputLayerFlag(i), "alt_output_layer_flag[i]");
    }

    assert( NumOutputLayersInOutputLayerSet[i] > 0 );
  }

  // The value of vps_num_rep_formats_minus1 shall be in the range of 0 to 255, inclusive.
  assert( vps->getVpsNumRepFormats() > 0 && vps->getVpsNumRepFormats() <= 256 );
  
  WRITE_UVLC( vps->getVpsNumRepFormats() - 1, "vps_num_rep_formats_minus1" );

  for(i = 0; i < vps->getVpsNumRepFormats(); i++)
  {
    // Write rep_format_structures
    codeRepFormat( vps->getVpsRepFormat(i) );
  }

  if( vps->getVpsNumRepFormats() > 1 )
  {
    WRITE_FLAG( vps->getRepFormatIdxPresentFlag(), "rep_format_idx_present_flag"); 
  }
  else
  {
    // When not present, the value of rep_format_idx_present_flag is inferred to be equal to 0
    assert( !vps->getRepFormatIdxPresentFlag() );
  }

  if( vps->getRepFormatIdxPresentFlag() )
  {
    for( i = vps->getBaseLayerInternalFlag() ? 1 : 0; i < vps->getMaxLayers(); i++ )
    {
      Int numBits = 1;
      while ((1 << numBits) < (vps->getVpsNumRepFormats()))
      {
        numBits++;
      }
      WRITE_CODE( vps->getVpsRepFormatIdx(i), numBits, "vps_rep_format_idx[i]" );
    }
  }

  WRITE_FLAG(vps->getMaxOneActiveRefLayerFlag(), "max_one_active_ref_layer_flag");

  WRITE_FLAG(vps->getVpsPocLsbAlignedFlag(), "vps_poc_lsb_aligned_flag");

  for( i = 1; i< vps->getMaxLayers(); i++ )
  {
    if( vps->getNumDirectRefLayers( vps->getLayerIdInNuh(i) ) == 0  )
    {
      WRITE_FLAG(vps->getPocLsbNotPresentFlag(i), "poc_lsb_not_present_flag[i]");
    }
  }
 
  codeVpsDpbSizeTable(vps);

  WRITE_UVLC( vps->getDirectDepTypeLen()-2,                           "direct_dep_type_len_minus2");

  WRITE_FLAG(vps->getDefaultDirectDependencyTypeFlag(), "direct_dependency_all_layers_flag");

  if( vps->getDefaultDirectDependencyTypeFlag() )
  {
    WRITE_CODE( vps->getDefaultDirectDependencyType(), vps->getDirectDepTypeLen(), "direct_dependency_all_layers_type" );
  }
  else
  {
    for( i = vps->getBaseLayerInternalFlag() ? 1 : 2; i < vps->getMaxLayers(); i++ )
    {
      for( j = vps->getBaseLayerInternalFlag() ? 0 : 1; j < i; j++ )
      {
        if (vps->getDirectDependencyFlag(i, j))
        {
          WRITE_CODE( vps->getDirectDependencyType(i, j), vps->getDirectDepTypeLen(), "direct_dependency_type[i][j]" );
        }
      }
    }
  }

  // The value of vps_non_vui_extension_length shall be in the range of 0 to 4096, inclusive.
  assert( vps->getVpsNonVuiExtLength() >= 0 && vps->getVpsNonVuiExtLength() <= 4096 );

  WRITE_UVLC( vps->getVpsNonVuiExtLength(), "vps_non_vui_extension_length" );

  for( i = 1; i <= vps->getVpsNonVuiExtLength(); i++ )
  {
    WRITE_CODE(1, 8, "vps_non_vui_extension_data_byte");
  }
    
  WRITE_FLAG( vps->getVpsVuiPresentFlag() ? 1 : 0,                     "vps_vui_present_flag" );

  if(vps->getVpsVuiPresentFlag())   // Should be conditioned on the value of vps_vui_present_flag
  {
    while ( m_pcBitIf->getNumberOfWrittenBits() % 8 != 0 )
    {
      WRITE_FLAG(1,                  "vps_vui_alignment_bit_equal_to_one");
    }

    codeVPSVUI(vps);  
  }
}

Void  TEncCavlc::codeRepFormat( const RepFormat *repFormat )
{
  WRITE_CODE( repFormat->getPicWidthVpsInLumaSamples (), 16, "pic_width_vps_in_luma_samples" );    
  WRITE_CODE( repFormat->getPicHeightVpsInLumaSamples(), 16, "pic_height_vps_in_luma_samples" );  
  WRITE_FLAG( repFormat->getChromaAndBitDepthVpsPresentFlag(), "chroma_and_bit_depth_vps_present_flag" );

  if( repFormat->getChromaAndBitDepthVpsPresentFlag() )
  {
    WRITE_CODE( repFormat->getChromaFormatVpsIdc(), 2, "chroma_format_vps_idc" );   

    if( repFormat->getChromaFormatVpsIdc() == 3 )
    {
      WRITE_FLAG( repFormat->getSeparateColourPlaneVpsFlag(), "separate_colour_plane_vps_flag" );      
    }

    assert( repFormat->getBitDepthVpsLuma() >= 8 );
    assert( repFormat->getBitDepthVpsChroma() >= 8 );
    WRITE_CODE( repFormat->getBitDepthVpsLuma() - 8,   4, "bit_depth_vps_luma_minus8" );           
    WRITE_CODE( repFormat->getBitDepthVpsChroma() - 8, 4, "bit_depth_vps_chroma_minus8" );
  }

  Window conf = repFormat->getConformanceWindowVps();

  WRITE_FLAG( conf.getWindowEnabledFlag(),    "conformance_window_vps_flag" );
  if (conf.getWindowEnabledFlag())
  {
    WRITE_UVLC( conf.getWindowLeftOffset(),   "conf_win_vps_left_offset"   );
    WRITE_UVLC( conf.getWindowRightOffset(),  "conf_win_vps_right_offset"  );
    WRITE_UVLC( conf.getWindowTopOffset(),    "conf_win_vps_top_offset"    );
    WRITE_UVLC( conf.getWindowBottomOffset(), "conf_win_vps_bottom_offset" );
  }
}

Void TEncCavlc::codeVpsDpbSizeTable( const TComVPS *vps )
{
  for( Int i = 1; i < vps->getNumOutputLayerSets(); i++ )
  {
    Int layerSetIdxForOutputLayerSet = vps->getOutputLayerSetIdx( i );

    WRITE_FLAG( vps->getSubLayerFlagInfoPresentFlag( i ), "sub_layer_flag_info_present_flag[i]");

    for(Int j = 0; j <= vps->getMaxSLayersInLayerSetMinus1( layerSetIdxForOutputLayerSet ); j++)
    {
      if( j > 0 && vps->getSubLayerFlagInfoPresentFlag(i) )
      {
        WRITE_FLAG( vps->getSubLayerDpbInfoPresentFlag( i, j), "sub_layer_dpb_info_present_flag[i]");  
      }

      if( vps->getSubLayerDpbInfoPresentFlag(i, j) )
      {
        for(Int k = 0; k < vps->getNumLayersInIdList( layerSetIdxForOutputLayerSet ); k++)
        {
          if( vps->getNecessaryLayerFlag(i, k) && (vps->getBaseLayerInternalFlag() || (vps->getLayerSetLayerIdList(layerSetIdxForOutputLayerSet, k) != 0)) )
          {
            WRITE_UVLC( vps->getMaxVpsDecPicBufferingMinus1( i, k, j ), "max_vps_dec_pic_buffering_minus1[i][k][j]" );
          }
        }

        WRITE_UVLC( vps->getMaxVpsNumReorderPics( i, j), "max_vps_num_reorder_pics[i][j]" );              

        WRITE_UVLC( vps->getMaxVpsLatencyIncreasePlus1( i, j), "max_vps_latency_increase_plus1[i][j]" );        
      }
    }
  }
}

Void TEncCavlc::codeVPSVUI( const TComVPS *vps )
{
  Int i,j;
  WRITE_FLAG(vps->getCrossLayerPictureTypeAlignFlag(), "cross_layer_pic_type_aligned_flag");

  if( !vps->getCrossLayerPictureTypeAlignFlag() )
  {
    WRITE_FLAG(vps->getCrossLayerIrapAlignFlag(), "cross_layer_irap_aligned_flag");
  }
  else
  {
    // When not present, the value of cross_layer_irap_aligned_flag is inferred to be equal to vps_vui_present_flag,
    assert( vps->getCrossLayerIrapAlignFlag() == true );
  }

  if( vps->getCrossLayerIrapAlignFlag() )
  {
    WRITE_FLAG(vps->getCrossLayerAlignedIdrOnlyFlag(), "all_layers_idr_aligned_flag");
  }

  WRITE_FLAG( vps->getBitRatePresentVpsFlag(),        "bit_rate_present_vps_flag" );
  WRITE_FLAG( vps->getPicRatePresentVpsFlag(),        "pic_rate_present_vps_flag" );

  if( vps->getBitRatePresentVpsFlag() || vps->getPicRatePresentVpsFlag() )
  {
    for( i = vps->getBaseLayerInternalFlag() ? 0 : 1; i < vps->getNumLayerSets(); i++ )
    {
      for( j = 0; j <= vps->getMaxSLayersInLayerSetMinus1(i); j++ )
      {
        if( vps->getBitRatePresentVpsFlag() )
        {
          WRITE_FLAG( vps->getBitRatePresentFlag( i, j),        "bit_rate_present_flag[i][j]" );
        }

        if( vps->getPicRatePresentVpsFlag() )
        {
          WRITE_FLAG( vps->getPicRatePresentFlag( i, j),        "pic_rate_present_flag[i][j]" );
        }

        if( vps->getBitRatePresentFlag(i, j) )
        {
          WRITE_CODE( vps->getAvgBitRate( i, j ), 16, "avg_bit_rate[i][j]" );
          WRITE_CODE( vps->getAvgBitRate( i, j ), 16, "max_bit_rate[i][j]" );
        }

        if( vps->getPicRatePresentFlag(i, j) )
        {
          WRITE_CODE( vps->getConstPicRateIdc( i, j), 2 , "constant_pic_rate_idc[i][j]" ); 
          WRITE_CODE( vps->getConstPicRateIdc( i, j), 16, "avg_pic_rate[i][j]"          ); 
        }
      }
    }
  }

  WRITE_FLAG( vps->getVideoSigPresentVpsFlag(), "video_signal_info_idx_present_flag" );
  if (vps->getVideoSigPresentVpsFlag())
  {
    WRITE_CODE(vps->getNumVideoSignalInfo()-1, 4, "vps_num_video_signal_info_minus1" );
  }

  for(i = 0; i < vps->getNumVideoSignalInfo(); i++)
  {
    WRITE_CODE(vps->getVideoVPSFormat(i), 3, "video_vps_format" );
    WRITE_FLAG(vps->getVideoFullRangeVpsFlag(i), "video_full_range_vps_flag" );
    WRITE_CODE(vps->getColorPrimaries(i), 8, "colour_primaries_vps" );
    WRITE_CODE(vps->getTransCharacter(i), 8, "transfer_characteristics_vps" );
    WRITE_CODE(vps->getMaxtrixCoeff(i), 8, "matrix_coeffs_vps" );
  }

  if( vps->getVideoSigPresentVpsFlag() && vps->getNumVideoSignalInfo() > 1 )
  {
    for( i = vps->getBaseLayerInternalFlag() ? 0 : 1; i < vps->getMaxLayers(); i++ )
    {
      WRITE_CODE( vps->getVideoSignalInfoIdx(i), 4, "vps_video_signal_info_idx" );
    }
  }

  WRITE_FLAG( vps->getTilesNotInUseFlag() ? 1 : 0 , "tiles_not_in_use_flag" );

  if( !vps->getTilesNotInUseFlag() )
  {
    for( i = vps->getBaseLayerInternalFlag() ? 0 : 1; i < vps->getMaxLayers(); i++ )
    {
      WRITE_FLAG( vps->getTilesInUseFlag(i) ? 1 : 0 , "tiles_in_use_flag[ i ]" );

      if( vps->getTilesInUseFlag(i) )
      {
        WRITE_FLAG( vps->getLoopFilterNotAcrossTilesFlag(i) ? 1 : 0 , "loop_filter_not_across_tiles_flag[ i ]" );
      }
    }

    for( i = vps->getBaseLayerInternalFlag() ? 1 : 2; i < vps->getMaxLayers(); i++ )
    {
      for(j = 0; j < vps->getNumDirectRefLayers(vps->getLayerIdInNuh(i)); j++)
      {
        UInt layerIdx = vps->getLayerIdxInVps(vps->getRefLayerId(vps->getLayerIdInNuh(i), j));

        if( vps->getTilesInUseFlag(i) && vps->getTilesInUseFlag(layerIdx) )
        {
          WRITE_FLAG( vps->getTileBoundariesAlignedFlag(i,j) ? 1 : 0 , "tile_boundaries_aligned_flag[i][j]" );
        }
      }
    }  
  }

  WRITE_FLAG( vps->getWppNotInUseFlag() ? 1 : 0 , "wpp_not_in_use_flag" );

  if( !vps->getWppNotInUseFlag() )
  {
    for( i = vps->getBaseLayerInternalFlag() ? 0 : 1; i < vps->getMaxLayers(); i++ )
    {
      WRITE_FLAG( vps->getWppInUseFlag(i) ? 1 : 0 , "wpp_in_use_flag[ i ]" );
    }
  }

  WRITE_FLAG(vps->getSingleLayerForNonIrapFlag(), "single_layer_for_non_irap_flag" );

  // When single_layer_for_non_irap_flag is equal to 0, higher_layer_irap_skip_flag shall be equal to 0
  if( !vps->getSingleLayerForNonIrapFlag() )
  {
    assert( !vps->getHigherLayerIrapSkipFlag() );
  }

  WRITE_FLAG(vps->getHigherLayerIrapSkipFlag(), "higher_layer_irap_skip_flag" );

  WRITE_FLAG( vps->getIlpRestrictedRefLayersFlag() ? 1 : 0 , "ilp_restricted_ref_layers_flag" );    
  if( vps->getIlpRestrictedRefLayersFlag())
  {
    for(i = 1; i < vps->getMaxLayers(); i++)
    {
      for(j = 0; j < vps->getNumDirectRefLayers(vps->getLayerIdInNuh(i)); j++)
      {
        if (vps->getBaseLayerInternalFlag() || vps->getRefLayerId(vps->getLayerIdInNuh(i), j))
        {
          WRITE_UVLC(vps->getMinSpatialSegmentOffsetPlus1( i, j),    "min_spatial_segment_offset_plus1[i][j]");

          if( vps->getMinSpatialSegmentOffsetPlus1(i,j ) > 0 ) 
          {  
            WRITE_FLAG( vps->getCtuBasedOffsetEnabledFlag( i, j) ? 1 : 0 , "ctu_based_offset_enabled_flag[i][j]" );    

            if(vps->getCtuBasedOffsetEnabledFlag(i,j))  
            {
              WRITE_UVLC(vps->getMinHorizontalCtuOffsetPlus1( i, j),    "min_horizontal_ctu_offset_plus1[i][j]");            
            }
          }
        }
      }  
    }
  }

#if O0164_MULTI_LAYER_HRD
  WRITE_FLAG(vps->getVpsVuiBspHrdPresentFlag(), "vps_vui_bsp_hrd_present_flag" );

  if( vps->getVpsVuiBspHrdPresentFlag() )
  {
    codeVpsVuiBspHrdParams(vps);
  }
#endif

  for( i = 1; i < vps->getMaxLayers(); i++ )
  {
    if( vps->getNumRefLayers(vps->getLayerIdInNuh(i)) == 0 ) 
    {
      WRITE_FLAG(vps->getBaseLayerPSCompatibilityFlag(i), "base_layer_parameter_set_compatibility_flag" );
    }
  }
}

Void TEncCavlc::codeSPSExtension( const TComSPS* pcSPS )
{
  // more syntax elements to be written here

  // Vertical MV component restriction is not used in SHVC CTC
#if VIEW_SCALABILITY 
  WRITE_FLAG( pcSPS->getInterViewMvVertConstraintFlag() ? 1 : 0 , "inter_view_mv_vert_constraint_flag" );
#else
  WRITE_FLAG( 0, "inter_view_mv_vert_constraint_flag" );
#endif
}

Void TEncCavlc::codeVpsVuiBspHrdParams( const TComVPS* vps )
{
  WRITE_UVLC( vps->getVpsNumAddHrdParams(), "vps_num_add_hrd_params" );

  for( Int i = vps->getNumHrdParameters(), j = 0; i < vps->getNumHrdParameters() + vps->getVpsNumAddHrdParams(); i++, j++ ) // j = i - vps->getNumHrdParameters()
  {
    if( i > 0 )
    {
      WRITE_FLAG( vps->getCprmsAddPresentFlag(j), "cprms_add_present_flag[i]" );
    }

    WRITE_UVLC( vps->getNumSubLayerHrdMinus1(j), "num_sub_layer_hrd_minus1[i]" );

    codeHrdParameters(vps->getBspHrd(j), i == 0 ? true : vps->getCprmsAddPresentFlag(j), vps->getNumSubLayerHrdMinus1(j));
  }

  if( vps->getNumHrdParameters() + vps->getVpsNumAddHrdParams() > 0 )
  {
    for( Int h = 1; h < vps->getNumOutputLayerSets(); h++ )
    {
      Int lsIdx = vps->getOutputLayerSetIdx( h );

      WRITE_UVLC( vps->getNumSignalledPartitioningSchemes(h), "num_signalled_partitioning_schemes[h]");

      for( Int j = 1; j < vps->getNumSignalledPartitioningSchemes(h) + 1; j++ )
      {
        WRITE_UVLC( vps->getNumPartitionsInSchemeMinus1(h, j), "num_partitions_in_scheme_minus1[h][j]" );

        for( Int k = 0; k <= vps->getNumPartitionsInSchemeMinus1(h, j); k++ )
        {
          for( Int r = 0; r < vps->getNumLayersInIdList( lsIdx ); r++ )
          {
            WRITE_FLAG( vps->getLayerIncludedInPartitionFlag(h, j, k, r), "layer_included_in_partition_flag[h][j][k][r]" );
          }
        }
      }

      for( Int i = 0; i < vps->getNumSignalledPartitioningSchemes(h) + 1; i++ )
      {
        for( Int t = 0; t <= vps->getMaxSLayersInLayerSetMinus1(lsIdx); t++ )
        {
          WRITE_UVLC(vps->getNumBspSchedulesMinus1(h, i, t), "num_bsp_schedules_minus1[h][i][t]");

          for( Int j = 0; j <= vps->getNumBspSchedulesMinus1(h, i, t); j++ )
          {
            for( Int k = 0; k <= vps->getNumPartitionsInSchemeMinus1(h, i); k++ )
            {
              if( vps->getNumHrdParameters() + vps->getVpsNumAddHrdParams() > 1 )
              {
                Int numBits = 1;
                while ((1 << numBits) < (vps->getNumHrdParameters() + vps->getVpsNumAddHrdParams()))
                {
                  numBits++;
                }

                WRITE_CODE(vps->getBspHrdIdx(h, i, t, j, k), numBits, "bsp_hrd_idx[h][i][t][j][k]");
              }

              WRITE_UVLC( vps->getBspSchedIdx(h, i, t, j, k), "bsp_sched_idx[h][i][t][j][k]");
            }
          }
        }
      }
    }
  }
}


#if CGS_3D_ASYMLUT
Void TEncCavlc::xCode3DAsymLUT( TCom3DAsymLUT * pc3DAsymLUT )
{
  UInt uiNumRefLayers = ( UInt )pc3DAsymLUT->getRefLayerNum();
  WRITE_UVLC( uiNumRefLayers - 1 , "num_cm_ref_layers_minus1" );
  for( UInt i = 0 ; i < uiNumRefLayers ; i++ )
  {
    WRITE_CODE( pc3DAsymLUT->getRefLayerId( i ) , 6 , "cm_ref_layer_id" );
  }

  assert( pc3DAsymLUT->getCurOctantDepth() < 4 );
  WRITE_CODE( pc3DAsymLUT->getCurOctantDepth() , 2 , "cm_octant_depth" );
  assert( pc3DAsymLUT->getCurYPartNumLog2() < 4 );
  WRITE_CODE( pc3DAsymLUT->getCurYPartNumLog2() , 2 , "cm_y_part_num_log2" );
  assert( pc3DAsymLUT->getInputBitDepthY() < 16 );

  WRITE_UVLC( pc3DAsymLUT->getInputBitDepthY() - 8 , "cm_input_luma_bit_depth_minus8" );
  WRITE_UVLC( pc3DAsymLUT->getInputBitDepthC() - 8 , "cm_input_chroma_bit_depth_minus8" );
  WRITE_UVLC( pc3DAsymLUT->getOutputBitDepthY() - 8 , "cm_output_luma_bit_depth_minus8" );
  WRITE_UVLC( pc3DAsymLUT->getOutputBitDepthC() - 8 , "cm_output_chroma_bit_depth_minus8" );

  assert( pc3DAsymLUT->getResQuantBit() < 4 );
  WRITE_CODE( pc3DAsymLUT->getResQuantBit() , 2 , "cm_res_quant_bit" );

  xFindDeltaBits( pc3DAsymLUT );
  assert(pc3DAsymLUT->getDeltaBits() >=1 && pc3DAsymLUT->getDeltaBits() <= 4);
  WRITE_CODE( pc3DAsymLUT->getDeltaBits()-1 , 2 , "cm_delta_bit" );

  if( pc3DAsymLUT->getCurOctantDepth() == 1 )
  {
    WRITE_SVLC( pc3DAsymLUT->getAdaptChromaThresholdU() - ( 1 << ( pc3DAsymLUT->getInputBitDepthC() - 1 ) ) , "cm_adapt_threshold_u_delta" );
    WRITE_SVLC( pc3DAsymLUT->getAdaptChromaThresholdV() - ( 1 << ( pc3DAsymLUT->getInputBitDepthC() - 1 ) ) , "cm_adapt_threshold_v_delta" );
  }

#if R0164_CGS_LUT_BUGFIX_CHECK
  pc3DAsymLUT->xInitCuboids();
#endif
  xCode3DAsymLUTOctant( pc3DAsymLUT , 0 , 0 , 0 , 0 , 1 << pc3DAsymLUT->getCurOctantDepth() );
#if R0164_CGS_LUT_BUGFIX_CHECK
  xCuboidsFilledCheck( false );
  pc3DAsymLUT->display( false );
#endif
}

Void TEncCavlc::xCode3DAsymLUTOctant( TCom3DAsymLUT * pc3DAsymLUT , Int nDepth , Int yIdx , Int uIdx , Int vIdx , Int nLength )
{
  UInt uiOctantSplit = nDepth < pc3DAsymLUT->getCurOctantDepth();
  if( nDepth < pc3DAsymLUT->getCurOctantDepth() )
    WRITE_FLAG( uiOctantSplit , "split_octant_flag" );
  Int nYPartNum = 1 << pc3DAsymLUT->getCurYPartNumLog2();
  if( uiOctantSplit )
  {
    Int nHalfLength = nLength >> 1;
    for( Int l = 0 ; l < 2 ; l++ )
    {
      for( Int m = 0 ; m < 2 ; m++ )
      {
        for( Int n = 0 ; n < 2 ; n++ )
        {
          xCode3DAsymLUTOctant( pc3DAsymLUT , nDepth + 1 , yIdx + l * nHalfLength * nYPartNum , uIdx + m * nHalfLength , vIdx + n * nHalfLength , nHalfLength );
        }
      }
    }
  }
  else
  {
    Int nFLCbits = pc3DAsymLUT->getMappingShift()-pc3DAsymLUT->getResQuantBit()-pc3DAsymLUT->getDeltaBits() ; 
    nFLCbits = nFLCbits >= 0 ? nFLCbits : 0;

    for( Int l = 0; l < nYPartNum; l++ )
    {
      Int shift = pc3DAsymLUT->getCurOctantDepth() - nDepth;

      for( Int nVertexIdx = 0; nVertexIdx < 4; nVertexIdx++ )
      {
        SYUVP sRes = pc3DAsymLUT->getCuboidVertexResTree( yIdx + (l<<shift), uIdx, vIdx, nVertexIdx );

        UInt uiCodeVertex = sRes.Y != 0 || sRes.U != 0 || sRes.V != 0;
        WRITE_FLAG( uiCodeVertex, "coded_res_flag" );
        if( uiCodeVertex )
        {
          xWriteParam( sRes.Y, nFLCbits );
          xWriteParam( sRes.U, nFLCbits );
          xWriteParam( sRes.V, nFLCbits );
        }
      }
#if R0164_CGS_LUT_BUGFIX_CHECK
      pc3DAsymLUT->xSetExplicit( yIdx + (l<<shift) , uIdx , vIdx );
#endif
    }
  }
}

Void TEncCavlc::xWriteParam( Int param, UInt rParam)
{
  Int codeNumber = abs(param);
  WRITE_UVLC(codeNumber / (1 << rParam), "res_coeff_q");
  WRITE_CODE((codeNumber % (1 << rParam)), rParam, "res_coeff_r");
  if (abs(param))
    WRITE_FLAG( param < 0, "res_coeff_s");
}

Void TEncCavlc::xFindDeltaBits( TCom3DAsymLUT * pc3DAsymLUT )
{
  Int nDeltaBits; 
  Int nBestDeltaBits = -1; 
  Int nBestBits = MAX_INT; 
  for( nDeltaBits = 1; nDeltaBits < 5; nDeltaBits++)
  {
    Int nCurBits = 0;
    xTally3DAsymLUTOctantBits( pc3DAsymLUT , 0 , 0 , 0 , 0 , 1 << pc3DAsymLUT->getCurOctantDepth(), nDeltaBits, nCurBits );
    //printf("%d, %d, %d\n", nDeltaBits, nCurBits, nBestBits); 
    if(nCurBits < nBestBits)
    {
      nBestDeltaBits = nDeltaBits; 
      nBestBits = nCurBits;
    }
  }

  assert(nBestDeltaBits >=1 && nBestDeltaBits < 5);
  pc3DAsymLUT->setDeltaBits(nBestDeltaBits); 
}

Void TEncCavlc::xTally3DAsymLUTOctantBits( TCom3DAsymLUT * pc3DAsymLUT , Int nDepth , Int yIdx , Int uIdx , Int vIdx , Int nLength, Int nDeltaBits, Int& nCurBits )
{
  UInt uiOctantSplit = nDepth < pc3DAsymLUT->getCurOctantDepth();
  if( nDepth < pc3DAsymLUT->getCurOctantDepth() )
    nCurBits ++; 
  Int nYPartNum = 1 << pc3DAsymLUT->getCurYPartNumLog2();
  if( uiOctantSplit )
  {
    Int nHalfLength = nLength >> 1;
    for( Int l = 0 ; l < 2 ; l++ )
    {
      for( Int m = 0 ; m < 2 ; m++ )
      {
        for( Int n = 0 ; n < 2 ; n++ )
        {
          xTally3DAsymLUTOctantBits( pc3DAsymLUT , nDepth + 1 , yIdx + l * nHalfLength * nYPartNum , uIdx + m * nHalfLength , vIdx + n * nHalfLength , nHalfLength, nDeltaBits, nCurBits );
        }
      }
    }
  }
  else
  {
    Int nFLCbits = pc3DAsymLUT->getMappingShift()-pc3DAsymLUT->getResQuantBit()-nDeltaBits ; 
    nFLCbits = nFLCbits >= 0 ? nFLCbits:0;
    //printf("nFLCbits = %d\n", nFLCbits);

    for( Int l = 0 ; l < nYPartNum ; l++ )
    {
      for( Int nVertexIdx = 0 ; nVertexIdx < 4 ; nVertexIdx++ )
      {
        SYUVP sRes = pc3DAsymLUT->getCuboidVertexResTree( yIdx + l , uIdx , vIdx , nVertexIdx );

        UInt uiCodeVertex = sRes.Y != 0 || sRes.U != 0 || sRes.V != 0;
        nCurBits++;
        if( uiCodeVertex )
        {
          xCheckParamBits( sRes.Y, nFLCbits, nCurBits );
          xCheckParamBits( sRes.U, nFLCbits, nCurBits );
          xCheckParamBits( sRes.V, nFLCbits, nCurBits );
        }
      }
    }
  }
}

Void TEncCavlc::xCheckParamBits( Int param, Int rParam, Int &nBits)
{
  Int codeNumber = abs(param);
  Int codeQuotient = codeNumber >> rParam;
  Int qLen; 

  UInt uiLength = 1;
  UInt uiTemp = ++codeQuotient;
    
  while( 1 != uiTemp )
  {
    uiTemp >>= 1;
    uiLength += 2;
  }

  qLen  = (uiLength >> 1);
  qLen += ((uiLength+1) >> 1);

  nBits += qLen; 
  nBits += rParam; 
  if (abs(param))
    nBits++; 
}
#endif //CGS_3D_ASYMLUT

#endif //SVC_EXTENSION
//! \}
