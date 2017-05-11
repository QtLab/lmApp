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

/** \file     TAppEncCfg.h
    \brief    Handle encoder configuration parameters (header)
*/

#ifndef __TAPPENCCFG__
#define __TAPPENCCFG__

#include "TLibCommon/CommonDef.h"

#include "TLibEncoder/TEncCfg.h"
#if SVC_EXTENSION
#include "TAppEncLayerCfg.h"
#endif
#include <sstream>
#include <vector>
//! \ingroup TAppEncoder
//! \{

#if SCALABLE_REXT
enum ExtendedProfileName // this is used for determining profile strings, where multiple profiles map to a single profile idc with various constraint flag combinations
{
  NONE = 0,
  MAIN = 1,
  MAIN10 = 2,
  MAINSTILLPICTURE = 3,
  MAINREXT = 4,
  HIGHTHROUGHPUTREXT = 5, // Placeholder profile for development
#if SVC_EXTENSION
  MULTIVIEWMAIN = 6,
  SCALABLEMAIN = 7,
  SCALABLEMAIN10 = 8,
#if SCALABLE_REXT
  SCALABLEREXT = 10,
#endif
#endif
  // The following are RExt profiles, which would map to the MAINREXT profile idc.
  // The enumeration indicates the bit-depth constraint in the bottom 2 digits
  //                           the chroma format in the next digit
  //                           the intra constraint in the next digit
  //                           If it is a RExt still picture, there is a '1' for the next digit,
  //                           If it is a Scalable Rext profile, there is a '1' for the top digit.
  MONOCHROME_8      = 1008,
  MONOCHROME_12     = 1012,
  MONOCHROME_16     = 1016,
  MAIN_12           = 1112,
  MAIN_422_10       = 1210,
  MAIN_422_12       = 1212,
  MAIN_444          = 1308,
  MAIN_444_10       = 1310,
  MAIN_444_12       = 1312,
  MAIN_444_16       = 1316, // non-standard profile definition, used for development purposes
  MAIN_INTRA        = 2108,
  MAIN_10_INTRA     = 2110,
  MAIN_12_INTRA     = 2112,
  MAIN_422_10_INTRA = 2210,
  MAIN_422_12_INTRA = 2212,
  MAIN_444_INTRA    = 2308,
  MAIN_444_10_INTRA = 2310,
  MAIN_444_12_INTRA = 2312,
  MAIN_444_16_INTRA = 2316,
  MAIN_444_STILL_PICTURE = 11308,
  MAIN_444_16_STILL_PICTURE = 12316
#if SCALABLE_REXT
  ,
  SCALABLE_MONOCHROME_8   = 101008,
  SCALABLE_MONOCHROME_12  = 101012,
  SCALABLE_MONOCHROME_16  = 101016,
  SCALABLE_MAIN_444       = 101308
#endif
};
#endif

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// encoder configuration class
class TAppEncCfg
{
protected:
  // file I/O
#if SVC_EXTENSION
  TAppEncLayerCfg* m_apcLayerCfg[MAX_LAYERS];
  Int       m_layerId;
  Int       m_numLayers;                                      ///< number of layers
  Int       m_scalabilityMask[MAX_VPS_NUM_SCALABILITY_TYPES]; ///< scalability_mask
#if VIEW_SCALABILITY
  Int       m_iNumberOfViews;                                 ///< number of views
  std::vector<Int> m_ViewIdVal;
#endif  
  Bool      m_maxTidRefPresentFlag; 

  Int       m_numLayerSets;
  Int       m_numAddLayerSets;
  Int       m_numLayerInIdList[MAX_VPS_LAYER_SETS_PLUS1];
  Int       m_layerSetLayerIdList[MAX_VPS_LAYER_SETS_PLUS1][MAX_VPS_LAYER_IDX_PLUS1];
  Int       m_numHighestLayerIdx[MAX_VPS_LAYER_SETS_PLUS1];
  Int       m_highestLayerIdx[MAX_VPS_LAYER_SETS_PLUS1][MAX_VPS_LAYER_IDX_PLUS1];

  Int       m_defaultTargetOutputLayerIdc;
  Int       m_numOutputLayerSets;
  std::vector<Int>                m_outputLayerSetIdx;
  std::vector<Int>                m_numOutputLayersInOutputLayerSet;
  std::vector< std::vector<Int> > m_listOfOutputLayers;
#else
  std::string m_inputFileName;                                ///< source file name
#endif
  std::string m_bitstreamFileName;                            ///< output bitstream file
#if !SVC_EXTENSION
  std::string m_reconFileName;                                ///< output reconstruction file
#endif
  // Lambda modifiers
  Double    m_adLambdaModifier[ MAX_TLAYER ];                 ///< Lambda modifier array for each temporal layer
  std::vector<Double> m_adIntraLambdaModifier;                ///< Lambda modifier for Intra pictures, one for each temporal layer. If size>temporalLayer, then use [temporalLayer], else if size>0, use [size()-1], else use m_adLambdaModifier.
  Double    m_dIntraQpFactor;                                 ///< Intra Q Factor. If negative, use a default equation: 0.57*(1.0 - Clip3( 0.0, 0.5, 0.05*(Double)(isField ? (GopSize-1)/2 : GopSize-1) ))

  // source specification
#if !SVC_EXTENSION
  Int       m_iFrameRate;                                     ///< source frame-rates (Hz)
#endif
  UInt      m_FrameSkip;                                      ///< number of skipped frames from the beginning
  UInt      m_temporalSubsampleRatio;                         ///< temporal subsample ratio, 2 means code every two frames
#if !SVC_EXTENSION
  Int       m_iSourceWidth;                                   ///< source width in pixel
  Int       m_iSourceHeight;                                  ///< source height in pixel (when interlaced = field height)

  Int       m_iSourceHeightOrg;                               ///< original source height in pixel (when interlaced = frame height)
#endif
  Bool      m_isField;                                        ///< enable field coding
  Bool      m_isTopFieldFirst;
  Bool      m_bEfficientFieldIRAPEnabled;                     ///< enable an efficient field IRAP structure.
  Bool      m_bHarmonizeGopFirstFieldCoupleEnabled;

#if !SVC_EXTENSION
  Int       m_conformanceWindowMode;
  Int       m_confWinLeft;
  Int       m_confWinRight;
  Int       m_confWinTop;
  Int       m_confWinBottom;
#endif
  Int       m_framesToBeEncoded;                              ///< number of encoded frames
#if !SVC_EXTENSION
  Int       m_aiPad[2];                                       ///< number of padded pixels for width and height
#endif
  Bool      m_AccessUnitDelimiter;                            ///< add Access Unit Delimiter NAL units
  InputColourSpaceConversion m_inputColourSpaceConvert;       ///< colour space conversion to apply to input video
  Bool      m_snrInternalColourSpace;                       ///< if true, then no colour space conversion is applied for snr calculation, otherwise inverse of input is applied.
  Bool      m_outputInternalColourSpace;                    ///< if true, then no colour space conversion is applied for reconstructed video, otherwise inverse of input is applied.
#if !SVC_EXTENSION
  ChromaFormat m_InputChromaFormatIDC;
#endif

  Bool      m_printMSEBasedSequencePSNR;
  Bool      m_printFrameMSE;
  Bool      m_printSequenceMSE;
  Bool      m_cabacZeroWordPaddingEnabled;
  Bool      m_bClipInputVideoToRec709Range;
  Bool      m_bClipOutputVideoToRec709Range;

#if SVC_EXTENSION
  Int           m_numPTLInfo;
  Profile::Name m_profileList[MAX_NUM_LAYER_IDS + 1];
  Level::Tier   m_levelTierList[MAX_NUM_LAYER_IDS + 1];
  Level::Name   m_levelList[MAX_NUM_LAYER_IDS + 1];
  Profile::Name m_profileCompatibility[MAX_NUM_LAYER_IDS + 1];
  Bool          m_progressiveSourceFlagList[MAX_NUM_LAYER_IDS + 1];
  Bool          m_interlacedSourceFlagList[MAX_NUM_LAYER_IDS + 1];
  Bool          m_nonPackedConstraintFlagList[MAX_NUM_LAYER_IDS + 1];
  Bool          m_frameOnlyConstraintFlagList[MAX_NUM_LAYER_IDS + 1];

  std::vector< std::vector<Int> > m_listOfLayerPTLofOlss;
#else
  // profile/level
  Profile::Name m_profile;
  Level::Tier   m_levelTier;
  Level::Name   m_level;
  UInt          m_bitDepthConstraint;
  ChromaFormat  m_chromaFormatConstraint;
  Bool          m_intraConstraintFlag;
  Bool          m_onePictureOnlyConstraintFlag;
  Bool          m_lowerBitRateConstraintFlag;
  Bool m_progressiveSourceFlag;
  Bool m_interlacedSourceFlag;
  Bool m_nonPackedConstraintFlag;
  Bool m_frameOnlyConstraintFlag;
#endif

  // coding structure
#if !SVC_EXTENSION
  Int       m_iIntraPeriod;                                   ///< period of I-slice (random access period)
#endif
  Int       m_iDecodingRefreshType;                           ///< random access type
  Int       m_iGOPSize;                                       ///< GOP size of hierarchical structure

#if !SVC_EXTENSION
  Int       m_extraRPSs;                                      ///< extra RPSs added to handle CRA
  GOPEntry  m_GOPList[MAX_GOP];                               ///< the coding structure entries from the config file
#endif
  Int       m_numReorderPics[MAX_TLAYER];                     ///< total number of reorder pictures
  Int       m_maxDecPicBuffering[MAX_TLAYER];                 ///< total number of pictures in the decoded picture buffer
  Bool      m_crossComponentPredictionEnabledFlag;            ///< flag enabling the use of cross-component prediction
  Bool      m_reconBasedCrossCPredictionEstimate;             ///< causes the alpha calculation in encoder search to be based on the decoded residual rather than the pre-transform encoder-side residual
  UInt      m_log2SaoOffsetScale[MAX_NUM_CHANNEL_TYPE];       ///< number of bits for the upward bit shift operation on the decoded SAO offsets
  Bool      m_useTransformSkip;                               ///< flag for enabling intra transform skipping
  Bool      m_useTransformSkipFast;                           ///< flag for enabling fast intra transform skipping
  UInt      m_log2MaxTransformSkipBlockSize;                  ///< transform-skip maximum size (minimum of 2)
  Bool      m_transformSkipRotationEnabledFlag;               ///< control flag for transform-skip/transquant-bypass residual rotation
  Bool      m_transformSkipContextEnabledFlag;                ///< control flag for transform-skip/transquant-bypass single significance map context
  Bool      m_rdpcmEnabledFlag[NUMBER_OF_RDPCM_SIGNALLING_MODES];///< control flags for residual DPCM
  Bool      m_enableAMP;
  Bool      m_persistentRiceAdaptationEnabledFlag;            ///< control flag for Golomb-Rice parameter adaptation over each slice
  Bool      m_cabacBypassAlignmentEnabledFlag;

  // coding quality
#if !SVC_EXTENSION
  Double    m_fQP;                                            ///< QP value of key-picture (floating point)
  Int       m_iQP;                                            ///< QP value of key-picture (integer)
  std::string m_dQPFileName;                                  ///< QP offset for each slice (initialized from external file)
  Int*      m_aidQP;                                          ///< array of slice QP values
#endif
  Int       m_iMaxDeltaQP;                                    ///< max. |delta QP|
  UInt      m_uiDeltaQpRD;                                    ///< dQP range for multi-pass slice QP optimization
#if !SVC_EXTENSION
  Int       m_iMaxCuDQPDepth;                                 ///< Max. depth for a minimum CuDQPSize (0:default)
#endif
  Int       m_diffCuChromaQpOffsetDepth;                      ///< If negative, then do not apply chroma qp offsets.
  Bool      m_bFastDeltaQP;                                   ///< Fast Delta QP (false:default)

  Int       m_cbQpOffset;                                     ///< Chroma Cb QP Offset (0:default)
  Int       m_crQpOffset;                                     ///< Chroma Cr QP Offset (0:default)
#if W0038_CQP_ADJ
  UInt      m_sliceChromaQpOffsetPeriodicity;                 ///< Used in conjunction with Slice Cb/Cr QpOffsetIntraOrPeriodic. Use 0 (default) to disable periodic nature.
  Int       m_sliceChromaQpOffsetIntraOrPeriodic[2/*Cb,Cr*/]; ///< Chroma Cb QP Offset at slice level for I slice or for periodic inter slices as defined by SliceChromaQPOffsetPeriodicity. Replaces offset in the GOP table.
#endif
#if ADAPTIVE_QP_SELECTION
  Bool      m_bUseAdaptQpSelect;
#endif
  TComSEIMasteringDisplay m_masteringDisplay;

  Bool      m_bUseAdaptiveQP;                                 ///< Flag for enabling QP adaptation based on a psycho-visual model
  Int       m_iQPAdaptationRange;                             ///< dQP range by QP adaptation

#if !SVC_EXTENSION
  Int       m_maxTempLayer;                                  ///< Max temporal layer
#endif

  // coding unit (CU) definition
  // TODO: Remove MaxCUWidth/MaxCUHeight and replace with MaxCUSize.
#if !SVC_EXTENSION
  UInt      m_uiMaxCUWidth;                                   ///< max. CU width in pixel
  UInt      m_uiMaxCUHeight;                                  ///< max. CU height in pixel
  UInt      m_uiMaxCUDepth;                                   ///< max. CU depth (as specified by command line)
  UInt      m_uiMaxTotalCUDepth;                              ///< max. total CU depth - includes depth of transform-block structure
  UInt      m_uiLog2DiffMaxMinCodingBlockSize;                ///< difference between largest and smallest CU depth

  // transfom unit (TU) definition
  UInt      m_uiQuadtreeTULog2MaxSize;
  UInt      m_uiQuadtreeTULog2MinSize;

  UInt      m_uiQuadtreeTUMaxDepthInter;
  UInt      m_uiQuadtreeTUMaxDepthIntra;
#endif

  // coding tools (bit-depth)
#if !SVC_EXTENSION
  Int       m_inputBitDepth   [MAX_NUM_CHANNEL_TYPE];         ///< bit-depth of input file
  Int       m_outputBitDepth  [MAX_NUM_CHANNEL_TYPE];         ///< bit-depth of output file
  Int       m_MSBExtendedBitDepth[MAX_NUM_CHANNEL_TYPE];      ///< bit-depth of input samples after MSB extension
  Int       m_internalBitDepth[MAX_NUM_CHANNEL_TYPE];         ///< bit-depth codec operates at (input/output files will be converted)
#endif
  Bool      m_extendedPrecisionProcessingFlag;
  Bool      m_highPrecisionOffsetsEnabledFlag;

  //coding tools (chroma format)
#if !SVC_EXTENSION
  ChromaFormat m_chromaFormatIDC;
#endif

  // coding tools (PCM bit-depth)
  Bool      m_bPCMInputBitDepthFlag;                          ///< 0: PCM bit-depth is internal bit-depth. 1: PCM bit-depth is input bit-depth.

  // coding tool (SAO)
#if !SVC_EXTENSION
  Bool      m_bUseSAO;
#endif
  Bool      m_bTestSAODisableAtPictureLevel;
  Double    m_saoEncodingRate;                                ///< When >0 SAO early picture termination is enabled for luma and chroma
  Double    m_saoEncodingRateChroma;                          ///< The SAO early picture termination rate to use for chroma (when m_SaoEncodingRate is >0). If <=0, use results for luma.
  Int       m_maxNumOffsetsPerPic;                            ///< SAO maximun number of offset per picture
  Bool      m_saoCtuBoundary;                                 ///< SAO parameter estimation using non-deblocked pixels for CTU bottom and right boundary areas
#if OPTIONAL_RESET_SAO_ENCODING_AFTER_IRAP
  Bool      m_saoResetEncoderStateAfterIRAP;                  ///< When true, SAO encoder state will be reset following an IRAP.
#endif
  // coding tools (loop filter)
  Bool      m_bLoopFilterDisable;                             ///< flag for using deblocking filter
  Bool      m_loopFilterOffsetInPPS;                         ///< offset for deblocking filter in 0 = slice header, 1 = PPS
  Int       m_loopFilterBetaOffsetDiv2;                     ///< beta offset for deblocking filter
  Int       m_loopFilterTcOffsetDiv2;                       ///< tc offset for deblocking filter
#if W0038_DB_OPT
  Int       m_deblockingFilterMetric;                         ///< blockiness metric in encoder
#else
  Bool      m_DeblockingFilterMetric;                         ///< blockiness metric in encoder
#endif
  // coding tools (PCM)
  Bool      m_usePCM;                                         ///< flag for using IPCM
  UInt      m_pcmLog2MaxSize;                                 ///< log2 of maximum PCM block size
  UInt      m_uiPCMLog2MinSize;                               ///< log2 of minimum PCM block size
  Bool      m_bPCMFilterDisableFlag;                          ///< PCM filter disable flag
  Bool      m_enableIntraReferenceSmoothing;                  ///< flag for enabling(default)/disabling intra reference smoothing/filtering

  // coding tools (encoder-only parameters)
  Bool      m_bUseASR;                                        ///< flag for using adaptive motion search range
  Bool      m_bUseHADME;                                      ///< flag for using HAD in sub-pel ME
  Bool      m_useRDOQ;                                       ///< flag for using RD optimized quantization
  Bool      m_useRDOQTS;                                     ///< flag for using RD optimized quantization for transform skip
#if T0196_SELECTIVE_RDOQ
  Bool      m_useSelectiveRDOQ;                               ///< flag for using selective RDOQ
#endif
  Int       m_rdPenalty;                                      ///< RD-penalty for 32x32 TU for intra in non-intra slices (0: no RD-penalty, 1: RD-penalty, 2: maximum RD-penalty)
  Bool      m_bDisableIntraPUsInInterSlices;                  ///< Flag for disabling intra predicted PUs in inter slices.
  MESearchMethod m_motionEstimationSearchMethod;
  Bool      m_bRestrictMESampling;                            ///< Restrict sampling for the Selective ME
  Int       m_iSearchRange;                                   ///< ME search range
  Int       m_bipredSearchRange;                              ///< ME search range for bipred refinement
  Int       m_minSearchWindow;                                ///< ME minimum search window size for the Adaptive Window ME
  Bool      m_bClipForBiPredMeEnabled;                        ///< Enables clipping for Bi-Pred ME.
  Bool      m_bFastMEAssumingSmootherMVEnabled;               ///< Enables fast ME assuming a smoother MV.
  FastInterSearchMode m_fastInterSearchMode;                  ///< Parameter that controls fast encoder settings
  Bool      m_bUseEarlyCU;                                    ///< flag for using Early CU setting
  Bool      m_useFastDecisionForMerge;                        ///< flag for using Fast Decision Merge RD-Cost
  Bool      m_bUseCbfFastMode;                                ///< flag for using Cbf Fast PU Mode Decision
  Bool      m_useEarlySkipDetection;                          ///< flag for using Early SKIP Detection
  SliceConstraint m_sliceMode;
  Int             m_sliceArgument;                            ///< argument according to selected slice mode
  SliceConstraint m_sliceSegmentMode;
  Int             m_sliceSegmentArgument;                     ///< argument according to selected slice segment mode

  Bool      m_bLFCrossSliceBoundaryFlag;  ///< 1: filter across slice boundaries 0: do not filter across slice boundaries
  Bool      m_bLFCrossTileBoundaryFlag;   ///< 1: filter across tile boundaries  0: do not filter across tile boundaries
  Bool      m_tileUniformSpacingFlag;
  Int       m_numTileColumnsMinus1;
  Int       m_numTileRowsMinus1;
  std::vector<Int> m_tileColumnWidth;
  std::vector<Int> m_tileRowHeight;

#if !SVC_EXTENSION
  Bool      m_entropyCodingSyncEnabledFlag;
#endif

  Bool      m_bUseConstrainedIntraPred;                       ///< flag for using constrained intra prediction
  Bool      m_bFastUDIUseMPMEnabled;
  Bool      m_bFastMEForGenBLowDelayEnabled;
  Bool      m_bUseBLambdaForNonKeyLowDelayPictures;

  HashType  m_decodedPictureHashSEIType;                      ///< Checksum mode for decoded picture hash SEI message
  Bool      m_recoveryPointSEIEnabled;
  Bool      m_bufferingPeriodSEIEnabled;
  Bool      m_pictureTimingSEIEnabled;
  Bool      m_toneMappingInfoSEIEnabled;
  Bool      m_chromaResamplingFilterSEIenabled;
  Int       m_chromaResamplingHorFilterIdc;
  Int       m_chromaResamplingVerFilterIdc;
  Int       m_toneMapId;
  Bool      m_toneMapCancelFlag;
  Bool      m_toneMapPersistenceFlag;
  Int       m_toneMapCodedDataBitDepth;
  Int       m_toneMapTargetBitDepth;
  Int       m_toneMapModelId;
  Int       m_toneMapMinValue;
  Int       m_toneMapMaxValue;
  Int       m_sigmoidMidpoint;
  Int       m_sigmoidWidth;
  Int       m_numPivots;
  Int       m_cameraIsoSpeedIdc;
  Int       m_cameraIsoSpeedValue;
  Int       m_exposureIndexIdc;
  Int       m_exposureIndexValue;
  Bool      m_exposureCompensationValueSignFlag;
  Int       m_exposureCompensationValueNumerator;
  Int       m_exposureCompensationValueDenomIdc;
  Int       m_refScreenLuminanceWhite;
  Int       m_extendedRangeWhiteLevel;
  Int       m_nominalBlackLevelLumaCodeValue;
  Int       m_nominalWhiteLevelLumaCodeValue;
  Int       m_extendedWhiteLevelLumaCodeValue;
  Int*      m_startOfCodedInterval;
  Int*      m_codedPivotValue;
  Int*      m_targetPivotValue;
  Bool      m_framePackingSEIEnabled;
  Int       m_framePackingSEIType;
  Int       m_framePackingSEIId;
  Int       m_framePackingSEIQuincunx;
  Int       m_framePackingSEIInterpretation;
  Bool      m_segmentedRectFramePackingSEIEnabled;
  Bool      m_segmentedRectFramePackingSEICancel;
  Int       m_segmentedRectFramePackingSEIType;
  Bool      m_segmentedRectFramePackingSEIPersistence;
  Int       m_displayOrientationSEIAngle;
  Bool      m_temporalLevel0IndexSEIEnabled;
  Bool      m_gradualDecodingRefreshInfoEnabled;
  Int       m_noDisplaySEITLayer;
  Bool      m_decodingUnitInfoSEIEnabled;
  Bool      m_SOPDescriptionSEIEnabled;
  Bool      m_scalableNestingSEIEnabled;
  Bool      m_tmctsSEIEnabled;
  Bool      m_timeCodeSEIEnabled;
  Int       m_timeCodeSEINumTs;
  TComSEITimeSet m_timeSetArray[MAX_TIMECODE_SEI_SETS];
  Bool      m_kneeSEIEnabled;
  Int       m_kneeSEIId;
  Bool      m_kneeSEICancelFlag;
  Bool      m_kneeSEIPersistenceFlag;
  Int       m_kneeSEIInputDrange;
  Int       m_kneeSEIInputDispLuminance;
  Int       m_kneeSEIOutputDrange;
  Int       m_kneeSEIOutputDispLuminance;
  Int       m_kneeSEINumKneePointsMinus1;
  Int*      m_kneeSEIInputKneePoint;
  Int*      m_kneeSEIOutputKneePoint;
#if U0033_ALTERNATIVE_TRANSFER_CHARACTERISTICS_SEI
  Int       m_preferredTransferCharacteristics;
#endif
  UInt      m_greenMetadataType;
  UInt      m_xsdMetricType;

  // weighted prediction
  Bool      m_useWeightedPred;                    ///< Use of weighted prediction in P slices
  Bool      m_useWeightedBiPred;                  ///< Use of bi-directional weighted prediction in B slices
  WeightedPredictionMethod m_weightedPredictionMethod;

  UInt      m_log2ParallelMergeLevel;                         ///< Parallel merge estimation region
  UInt      m_maxNumMergeCand;                                ///< Max number of merge candidates

  Int       m_TMVPModeId;
  Bool      m_signDataHidingEnabledFlag;
#if !RC_SHVC_HARMONIZATION
  Bool      m_RCEnableRateControl;                ///< enable rate control or not
  Int       m_RCTargetBitrate;                    ///< target bitrate when rate control is enabled
  Int       m_RCKeepHierarchicalBit;              ///< 0: equal bit allocation; 1: fixed ratio bit allocation; 2: adaptive ratio bit allocation
  Bool      m_RCLCULevelRC;                       ///< true: LCU level rate control; false: picture level rate control NOTE: code-tidy - rename to m_RCCtuLevelRC
  Bool      m_RCUseLCUSeparateModel;              ///< use separate R-lambda model at LCU level                        NOTE: code-tidy - rename to m_RCUseCtuSeparateModel
  Int       m_RCInitialQP;                        ///< inital QP for rate control
  Bool      m_RCForceIntraQP;                     ///< force all intra picture to use initial QP or not
#if U0132_TARGET_BITS_SATURATION
  Bool      m_RCCpbSaturationEnabled;             ///< enable target bits saturation to avoid CPB overflow and underflow
  UInt      m_RCCpbSize;                          ///< CPB size
  Double    m_RCInitialCpbFullness;               ///< initial CPB fullness 
#endif
#endif
#if !SVC_EXTENSION
  ScalingListMode m_useScalingListId;                         ///< using quantization matrix
  std::string m_scalingListFileName;                          ///< quantization matrix file name
#endif

#if !PER_LAYER_LOSSLESS
  Bool      m_TransquantBypassEnabledFlag;                    ///< transquant_bypass_enabled_flag setting in PPS.
  Bool      m_CUTransquantBypassFlagForce;                    ///< if transquant_bypass_enabled_flag, then, if true, all CU transquant bypass flags will be set to true.
  CostMode  m_costMode;                                       ///< Cost mode to use
#endif

  Bool      m_recalculateQPAccordingToLambda;                 ///< recalculate QP value according to the lambda value
  Bool      m_useStrongIntraSmoothing;                        ///< enable strong intra smoothing for 32x32 blocks where the reference samples are flat
  Int       m_activeParameterSetsSEIEnabled;

  Bool      m_vuiParametersPresentFlag;                       ///< enable generation of VUI parameters
  Bool      m_aspectRatioInfoPresentFlag;                     ///< Signals whether aspect_ratio_idc is present
  Int       m_aspectRatioIdc;                                 ///< aspect_ratio_idc
  Int       m_sarWidth;                                       ///< horizontal size of the sample aspect ratio
  Int       m_sarHeight;                                      ///< vertical size of the sample aspect ratio
  Bool      m_overscanInfoPresentFlag;                        ///< Signals whether overscan_appropriate_flag is present
  Bool      m_overscanAppropriateFlag;                        ///< Indicates whether conformant decoded pictures are suitable for display using overscan
  Bool      m_videoSignalTypePresentFlag;                     ///< Signals whether video_format, video_full_range_flag, and colour_description_present_flag are present
  Int       m_videoFormat;                                    ///< Indicates representation of pictures
  Bool      m_videoFullRangeFlag;                             ///< Indicates the black level and range of luma and chroma signals
  Bool      m_colourDescriptionPresentFlag;                   ///< Signals whether colour_primaries, transfer_characteristics and matrix_coefficients are present
  Int       m_colourPrimaries;                                ///< Indicates chromaticity coordinates of the source primaries
  Int       m_transferCharacteristics;                        ///< Indicates the opto-electronic transfer characteristics of the source
  Int       m_matrixCoefficients;                             ///< Describes the matrix coefficients used in deriving luma and chroma from RGB primaries
  Bool      m_chromaLocInfoPresentFlag;                       ///< Signals whether chroma_sample_loc_type_top_field and chroma_sample_loc_type_bottom_field are present
  Int       m_chromaSampleLocTypeTopField;                    ///< Specifies the location of chroma samples for top field
  Int       m_chromaSampleLocTypeBottomField;                 ///< Specifies the location of chroma samples for bottom field
  Bool      m_neutralChromaIndicationFlag;                    ///< Indicates that the value of all decoded chroma samples is equal to 1<<(BitDepthCr-1)
  Bool      m_defaultDisplayWindowFlag;                       ///< Indicates the presence of the default window parameters
  Int       m_defDispWinLeftOffset;                           ///< Specifies the left offset from the conformance window of the default window
  Int       m_defDispWinRightOffset;                          ///< Specifies the right offset from the conformance window of the default window
  Int       m_defDispWinTopOffset;                            ///< Specifies the top offset from the conformance window of the default window
  Int       m_defDispWinBottomOffset;                         ///< Specifies the bottom offset from the conformance window of the default window
  Bool      m_frameFieldInfoPresentFlag;                      ///< Indicates that pic_struct values are present in picture timing SEI messages
  Bool      m_pocProportionalToTimingFlag;                    ///< Indicates that the POC value is proportional to the output time w.r.t. first picture in CVS
  Int       m_numTicksPocDiffOneMinus1;                       ///< Number of ticks minus 1 that for a POC difference of one
  Bool      m_bitstreamRestrictionFlag;                       ///< Signals whether bitstream restriction parameters are present
  Bool      m_tilesFixedStructureFlag;                        ///< Indicates that each active picture parameter set has the same values of the syntax elements related to tiles
  Bool      m_motionVectorsOverPicBoundariesFlag;             ///< Indicates that no samples outside the picture boundaries are used for inter prediction
  Int       m_minSpatialSegmentationIdc;                      ///< Indicates the maximum size of the spatial segments in the pictures in the coded video sequence
  Int       m_maxBytesPerPicDenom;                            ///< Indicates a number of bytes not exceeded by the sum of the sizes of the VCL NAL units associated with any coded picture
  Int       m_maxBitsPerMinCuDenom;                           ///< Indicates an upper bound for the number of bits of coding_unit() data
  Int       m_log2MaxMvLengthHorizontal;                      ///< Indicate the maximum absolute value of a decoded horizontal MV component in quarter-pel luma units
  Int       m_log2MaxMvLengthVertical;                        ///< Indicate the maximum absolute value of a decoded vertical MV component in quarter-pel luma units
#if !SVC_EXTENSION
  std::string m_colourRemapSEIFileRoot;
#endif

  std::string m_summaryOutFilename;                           ///< filename to use for producing summary output file.
  std::string m_summaryPicFilenameBase;                       ///< Base filename to use for producing summary picture output files. The actual filenames used will have I.txt, P.txt and B.txt appended.
  UInt        m_summaryVerboseness;                           ///< Specifies the level of the verboseness of the text output.

#if LAYERS_NOT_PRESENT_SEI
  Int       m_layersNotPresentSEIEnabled;
#endif
#if P0123_ALPHA_CHANNEL_SEI
  Bool      m_alphaSEIEnabled;
  Bool      m_alphaCancelFlag;
  Int       m_alphaUseIdc;
  Int       m_alphaBitDepthMinus8;
  Int       m_alphaTransparentValue;
  Int       m_alphaOpaqueValue;
  Bool      m_alphaIncrementFlag;
  Bool      m_alphaClipFlag;
  Bool      m_alphaClipTypeFlag;
#endif
#if SVC_EXTENSION
#if FAST_INTRA_SHVC
  Bool      m_useFastIntraScalable;                          ///< flag for using Fast Intra Decision for Scalable HEVC
#endif
#if Q0189_TMVP_CONSTRAINTS
  Int       m_TMVPConstraintsSEIEnabled;
#endif
  Bool      m_altOutputLayerFlag;                             ///< Specifies the value of alt_output_laye_flag in VPS extension
  Bool      m_elRapSliceBEnabled;
#if AVC_BASE
  Bool      m_nonHEVCBaseLayerFlag;                           ///< non HEVC BL
#endif
#endif

  // internal member functions
#if SVC_EXTENSION
  Void  xSetGlobal      (UInt layerId);                       ///< set global variables
  Void  xCheckParameter (UInt layerIdx);                       ///< check validity of configuration values per layer
#else
  Void  xSetGlobal      ();                                   ///< set global variables
  Void  xCheckParameter ();                                   ///< check validity of configuration values
#endif
  Void  xPrintParameter ();                                   ///< print configuration values
  Void  xPrintUsage     ();                                   ///< print usage
#if SVC_EXTENSION
  Int       m_adaptiveResolutionChange;                       ///< Indicate adaptive resolution change frame
  Bool      m_skipPictureAtArcSwitch;                         ///< Indicates that when ARC up-switching is performed the higher layer picture is a skip picture

  RepFormatCfg m_repFormatCfg[16];                            ///< Rep_format structures
#if N0383_IL_CONSTRAINED_TILE_SETS_SEI
  Bool      m_interLayerConstrainedTileSetsSEIEnabled;
  UInt      m_ilNumSetsInMessage;
  Bool      m_skippedTileSetPresentFlag;
  UInt      m_topLeftTileIndex[1024];
  UInt      m_bottomRightTileIndex[1024];
  UInt      m_ilcIdc[1024];
#endif
  Bool      m_crossLayerPictureTypeAlignFlag;
  Bool      m_crossLayerIrapAlignFlag;
#if Q0096_OVERLAY_SEI
  Bool                                m_overlaySEIEnabled;
  Bool                                m_overlayInfoCancelFlag;
  UInt                                m_overlayContentAuxIdMinus128;
  UInt                                m_overlayLabelAuxIdMinus128;
  UInt                                m_overlayAlphaAuxIdMinus128;
  UInt                                m_overlayElementLabelValueLengthMinus8;
  UInt                                m_numOverlaysMinus1;
  std::vector<UInt>                   m_overlayIdx;  
  std::vector<Bool>                   m_overlayLanguagePresentFlag;
  std::vector<UInt>                   m_overlayContentLayerId;
  std::vector<Bool>                   m_overlayLabelPresentFlag;
  std::vector<UInt>                   m_overlayLabelLayerId;
  std::vector<Bool>                   m_overlayAlphaPresentFlag;
  std::vector<UInt>                   m_overlayAlphaLayerId;
  std::vector<UInt>                   m_numOverlayElementsMinus1;
  std::vector< std::vector<UInt> >    m_overlayElementLabelMin;
  std::vector< std::vector<UInt> >    m_overlayElementLabelMax;
  std::vector<string>                 m_overlayLanguage;  
  std::vector<string>                 m_overlayName;  
  std::vector< std::vector<string> >  m_overlayElementName;  
  Bool                                m_overlayInfoPersistenceFlag;
#endif
  Bool      m_crossLayerAlignedIdrOnlyFlag;
  Bool      m_crossLayerBLAFlag;
  Bool      m_useInterLayerWeightedPred;
#if CGS_3D_ASYMLUT
  Int       m_nCGSFlag;
  Int       m_nCGSMaxOctantDepth;
  Int       m_nCGSMaxYPartNumLog2;
  Int       m_nCGSLUTBit;
  Int       m_nCGSAdaptiveChroma;
#if R0179_ENC_OPT_3DLUT_SIZE
  Int       m_nCGSLutSizeRDO;
#endif
#endif 
#endif //SVC_EXTENSION
public:
  TAppEncCfg();
  virtual ~TAppEncCfg();

public:
  Void  create    ();                                         ///< create option handling class
  Void  destroy   ();                                         ///< destroy option handling class
  Bool  parseCfg  ( Int argc, TChar* argv[] );                ///< parse configuration file to fill member variables

#if SVC_EXTENSION
  Bool parseCfgNumLayersAndInit( Int argc, TChar* argv[] );   ///< parse configuration file to to get number of layers and allocate memory
  Int  getNumFrameToBeEncoded()                               { return m_framesToBeEncoded;                          }
  Int  getNumLayer()                                          { return m_numLayers;                                  }
  Int  getGOPSize()                                           { return m_iGOPSize;                                   }

  RepFormatCfg* getRepFormatCfg(Int i)                        { return &m_repFormatCfg[i];                           }
  Bool getUsePCM()                                            { return m_usePCM;                                     }
  UInt getPCMLog2MinSize  ()                                  { return  m_uiPCMLog2MinSize;                          }

  UInt getInternalBitDepth(Int layer, ChannelType type)       { return m_apcLayerCfg[layer]->m_internalBitDepth[type];}
  Bool getPCMInputBitDepthFlag()                              { return m_bPCMInputBitDepthFlag;                       }

  Int  getDecodingRefreshType()                               { return m_iDecodingRefreshType;                       }
  Void getDirFilename(string& filename, string& dir, const string path);

  Bool scanStringToArray(string const cfgString, Int const numEntries, const char* logString, Int * const returnArray);
  Bool scanStringToArray(string const cfgString, Int const numEntries, const char* logString, std::vector<Int> &  returnVector);
  Void cfgStringToArray(Int **arr, string const cfgString, Int const numEntries, const char* logString);
  Bool scanStringToArrayNumEntries(string const cfgString, Int &numEntries, const char* logString, Int * const returnArray);
  Bool scanStringToArrayNumEntries(string const cfgString, Int &numEntries, const char* logString, std::vector<Int> &  returnVector);
  Void cfgStringToArrayNumEntries(Int **arr, string const cfgString, Int &numEntries, const char* logString);
  Void printChromaFormat( const ChromaFormat inputChromaFormatIDC, const ChromaFormat chromaFormatIDC );

#if SCALABLE_REXT
  Profile::Name extendedToShortProfileName(ExtendedProfileName uiExtendedProfileName)
  {
    if(uiExtendedProfileName < MONOCHROME_8)
    {
      return Profile::Name(uiExtendedProfileName);
    }
    else if(uiExtendedProfileName < SCALABLE_MONOCHROME_8)
    {
      return Profile::MAINREXT;
    }
    else
    {
      return Profile::SCALABLEREXT;
    }
  }
#endif
#endif
};// END CLASS DEFINITION TAppEncCfg

//! \}

#endif // __TAPPENCCFG__

