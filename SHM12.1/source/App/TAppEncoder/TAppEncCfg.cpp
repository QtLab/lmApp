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

/** \file     TAppEncCfg.cpp
    \brief    Handle encoder configuration parameters
*/

#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <cstring>
#include <string>
#include <limits>
#include "TLibCommon/TComRom.h"
#include "TAppEncCfg.h"
#include "TAppCommon/program_options_lite.h"
#include "TLibEncoder/TEncRateCtrl.h"
#ifdef WIN32
#define strdup _strdup
#endif

#define MACRO_TO_STRING_HELPER(val) #val
#define MACRO_TO_STRING(val) MACRO_TO_STRING_HELPER(val)

using namespace std;
namespace po = df::program_options_lite;


#if !SCALABLE_REXT
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


//! \ingroup TAppEncoder
//! \{

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================

#if SVC_EXTENSION
TAppEncCfg::TAppEncCfg()
: m_maxTidRefPresentFlag(1)
, m_defaultTargetOutputLayerIdc (-1)
, m_numOutputLayerSets          (-1)
, m_inputColourSpaceConvert(IPCOLOURSPACE_UNCHANGED)
, m_snrInternalColourSpace(false)
, m_outputInternalColourSpace(false)
, m_elRapSliceBEnabled(false)
#if AVC_BASE
, m_nonHEVCBaseLayerFlag(false)
#endif
{
  memset( m_apcLayerCfg, 0, sizeof(m_apcLayerCfg) );
  memset( m_scalabilityMask, 0, sizeof(m_scalabilityMask) );
}
#else
TAppEncCfg::TAppEncCfg()
: m_inputColourSpaceConvert(IPCOLOURSPACE_UNCHANGED)
, m_snrInternalColourSpace(false)
, m_outputInternalColourSpace(false)
{
  m_aidQP = NULL;
  m_startOfCodedInterval = NULL;
  m_codedPivotValue = NULL;
  m_targetPivotValue = NULL;
}
#endif

TAppEncCfg::~TAppEncCfg()
{
#if !SVC_EXTENSION
  if ( m_aidQP )
  {
    delete[] m_aidQP;
  }
  if ( m_startOfCodedInterval )
  {
    delete[] m_startOfCodedInterval;
    m_startOfCodedInterval = NULL;
  }
   if ( m_codedPivotValue )
  {
    delete[] m_codedPivotValue;
    m_codedPivotValue = NULL;
  }
  if ( m_targetPivotValue )
  {
    delete[] m_targetPivotValue;
    m_targetPivotValue = NULL;
  }
#endif  
}

Void TAppEncCfg::create()
{
}

Void TAppEncCfg::destroy()
{
#if SVC_EXTENSION
  for( Int layer = 0; layer < m_numLayers; layer++ )
  {
    if( m_apcLayerCfg[layer] )
    {
      delete m_apcLayerCfg[layer];
      m_apcLayerCfg[layer] = NULL;
    }
  }
#endif
}

std::istringstream &operator>>(std::istringstream &in, GOPEntry &entry)     //input
{
  in>>entry.m_sliceType;
  in>>entry.m_POC;
  in>>entry.m_QPOffset;
#if W0038_CQP_ADJ
  in>>entry.m_CbQPoffset;
  in>>entry.m_CrQPoffset;
#endif
  in>>entry.m_QPFactor;
  in>>entry.m_tcOffsetDiv2;
  in>>entry.m_betaOffsetDiv2;
  in>>entry.m_temporalId;
  in>>entry.m_numRefPicsActive;
  in>>entry.m_numRefPics;
  for ( Int i = 0; i < entry.m_numRefPics; i++ )
  {
    in>>entry.m_referencePics[i];
  }
  in>>entry.m_interRPSPrediction;
  if (entry.m_interRPSPrediction==1)
  {
    in>>entry.m_deltaRPS;
    in>>entry.m_numRefIdc;
    for ( Int i = 0; i < entry.m_numRefIdc; i++ )
    {
      in>>entry.m_refIdc[i];
    }
  }
  else if (entry.m_interRPSPrediction==2)
  {
    in>>entry.m_deltaRPS;
  }
  return in;
}

Bool confirmPara(Bool bflag, const TChar* message);

static inline ChromaFormat numberToChromaFormat(const Int val)
{
  switch (val)
  {
    case 400: return CHROMA_400; break;
    case 420: return CHROMA_420; break;
    case 422: return CHROMA_422; break;
    case 444: return CHROMA_444; break;
    default:  return NUM_CHROMA_FORMAT;
  }
}

static const struct MapStrToProfile
{
  const TChar* str;
  Profile::Name value;
}
strToProfile[] =
{
  {"none",                 Profile::NONE               },
  {"main",                 Profile::MAIN               },
  {"main10",               Profile::MAIN10             },
  {"main-still-picture",   Profile::MAINSTILLPICTURE   },
  {"main-RExt",            Profile::MAINREXT           },
  {"high-throughput-RExt", Profile::HIGHTHROUGHPUTREXT },
#if SVC_EXTENSION
  {"multiview-main",       Profile::MULTIVIEWMAIN      },       //This is not used in this software
  {"scalable-main",        Profile::SCALABLEMAIN       },
  {"scalable-main10",      Profile::SCALABLEMAIN10     },
#if SCALABLE_REXT
  {"scalable-RExt",        Profile::SCALABLEREXT     },
#endif
#endif
};

static const struct MapStrToExtendedProfile
{
  const TChar* str;
  ExtendedProfileName value;
}
strToExtendedProfile[] =
{
    {"none",                      NONE             },
    {"main",                      MAIN             },
    {"main10",                    MAIN10           },
    {"main_still_picture",        MAINSTILLPICTURE },
    {"main-still-picture",        MAINSTILLPICTURE },
    {"main_RExt",                 MAINREXT         },
    {"main-RExt",                 MAINREXT         },
    {"main_rext",                 MAINREXT         },
    {"main-rext",                 MAINREXT         },
    {"high_throughput_RExt",      HIGHTHROUGHPUTREXT },
    {"high-throughput-RExt",      HIGHTHROUGHPUTREXT },
    {"high_throughput_rext",      HIGHTHROUGHPUTREXT },
    {"high-throughput-rext",      HIGHTHROUGHPUTREXT },
    {"monochrome",                MONOCHROME_8     },
    {"monochrome12",              MONOCHROME_12    },
    {"monochrome16",              MONOCHROME_16    },
    {"main12",                    MAIN_12          },
    {"main_422_10",               MAIN_422_10      },
    {"main_422_12",               MAIN_422_12      },
    {"main_444",                  MAIN_444         },
    {"main_444_10",               MAIN_444_10      },
    {"main_444_12",               MAIN_444_12      },
    {"main_444_16",               MAIN_444_16      },
    {"main_intra",                MAIN_INTRA       },
    {"main_10_intra",             MAIN_10_INTRA    },
    {"main_12_intra",             MAIN_12_INTRA    },
    {"main_422_10_intra",         MAIN_422_10_INTRA},
    {"main_422_12_intra",         MAIN_422_12_INTRA},
    {"main_444_intra",            MAIN_444_INTRA   },
    {"main_444_still_picture",    MAIN_444_STILL_PICTURE },
    {"main_444_10_intra",         MAIN_444_10_INTRA},
    {"main_444_12_intra",         MAIN_444_12_INTRA},
    {"main_444_16_intra",         MAIN_444_16_INTRA},
    {"main_444_16_still_picture", MAIN_444_16_STILL_PICTURE },
#if SVC_EXTENSION
    {"multiview-main",            MULTIVIEWMAIN    },
    {"scalable-main",             SCALABLEMAIN     },
    {"scalable-main10",           SCALABLEMAIN10   },
#if SCALABLE_REXT
    {"scalable-monochrome",       SCALABLE_MONOCHROME_8   },
    {"scalable-monochrome12",     SCALABLE_MONOCHROME_12  },
    {"scalable-monochrome16",     SCALABLE_MONOCHROME_16  },
    {"scalable-main_444",         SCALABLE_MAIN_444       },
#endif
#endif
};

static const ExtendedProfileName validRExtProfileNames[2/* intraConstraintFlag*/][4/* bit depth constraint 8=0, 10=1, 12=2, 16=3*/][4/*chroma format*/]=
{
    {
        { MONOCHROME_8,  NONE,          NONE,              MAIN_444          }, // 8-bit  inter for 400, 420, 422 and 444
        { NONE,          NONE,          MAIN_422_10,       MAIN_444_10       }, // 10-bit inter for 400, 420, 422 and 444
        { MONOCHROME_12, MAIN_12,       MAIN_422_12,       MAIN_444_12       }, // 12-bit inter for 400, 420, 422 and 444
        { MONOCHROME_16, NONE,          NONE,              MAIN_444_16       }  // 16-bit inter for 400, 420, 422 and 444 (the latter is non standard used for development)
    },
    {
        { NONE,          MAIN_INTRA,    NONE,              MAIN_444_INTRA    }, // 8-bit  intra for 400, 420, 422 and 444
        { NONE,          MAIN_10_INTRA, MAIN_422_10_INTRA, MAIN_444_10_INTRA }, // 10-bit intra for 400, 420, 422 and 444
        { NONE,          MAIN_12_INTRA, MAIN_422_12_INTRA, MAIN_444_12_INTRA }, // 12-bit intra for 400, 420, 422 and 444
        { NONE,          NONE,          NONE,              MAIN_444_16_INTRA }  // 16-bit intra for 400, 420, 422 and 444
    }
};

static const struct MapStrToTier
{
  const TChar* str;
  Level::Tier value;
}
strToTier[] =
{
  {"main", Level::MAIN},
  {"high", Level::HIGH},
};

static const struct MapStrToLevel
{
  const TChar* str;
  Level::Name value;
}
strToLevel[] =
{
  {"none",Level::NONE},
  {"1",   Level::LEVEL1},
  {"2",   Level::LEVEL2},
  {"2.1", Level::LEVEL2_1},
  {"3",   Level::LEVEL3},
  {"3.1", Level::LEVEL3_1},
  {"4",   Level::LEVEL4},
  {"4.1", Level::LEVEL4_1},
  {"5",   Level::LEVEL5},
  {"5.1", Level::LEVEL5_1},
  {"5.2", Level::LEVEL5_2},
  {"6",   Level::LEVEL6},
  {"6.1", Level::LEVEL6_1},
  {"6.2", Level::LEVEL6_2},
  {"8.5", Level::LEVEL8_5},
};

#if U0132_TARGET_BITS_SATURATION
UInt g_uiMaxCpbSize[2][21] =
{
  //         LEVEL1,        LEVEL2,LEVEL2_1,     LEVEL3, LEVEL3_1,      LEVEL4, LEVEL4_1,       LEVEL5,  LEVEL5_1,  LEVEL5_2,    LEVEL6,  LEVEL6_1,  LEVEL6_2 
  { 0, 0, 0, 350000, 0, 0, 1500000, 3000000, 0, 6000000, 10000000, 0, 12000000, 20000000, 0,  25000000,  40000000,  60000000,  60000000, 120000000, 240000000 },
  { 0, 0, 0,      0, 0, 0,       0,       0, 0,       0,        0, 0, 30000000, 50000000, 0, 100000000, 160000000, 240000000, 240000000, 480000000, 800000000 }
};
#endif

static const struct MapStrToCostMode
{
  const TChar* str;
  CostMode    value;
}
strToCostMode[] =
{
  {"lossy",                     COST_STANDARD_LOSSY},
  {"sequence_level_lossless",   COST_SEQUENCE_LEVEL_LOSSLESS},
  {"lossless",                  COST_LOSSLESS_CODING},
  {"mixed_lossless_lossy",      COST_MIXED_LOSSLESS_LOSSY_CODING}
};

static const struct MapStrToScalingListMode
{
  const TChar* str;
  ScalingListMode value;
}
strToScalingListMode[] =
{
  {"0",       SCALING_LIST_OFF},
  {"1",       SCALING_LIST_DEFAULT},
  {"2",       SCALING_LIST_FILE_READ},
  {"off",     SCALING_LIST_OFF},
  {"default", SCALING_LIST_DEFAULT},
  {"file",    SCALING_LIST_FILE_READ}
};

template<typename T, typename P>
static std::string enumToString(P map[], UInt mapLen, const T val)
{
  for (UInt i = 0; i < mapLen; i++)
  {
    if (val == map[i].value)
    {
      return map[i].str;
    }
  }
  return std::string();
}

template<typename T, typename P>
static istream& readStrToEnum(P map[], UInt mapLen, istream &in, T &val)
{
  string str;
  in >> str;

  for (UInt i = 0; i < mapLen; i++)
  {
    if (str == map[i].str)
    {
      val = map[i].value;
      goto found;
    }
  }
  /* not found */
  in.setstate(ios::failbit);
found:
  return in;
}

//inline to prevent compiler warnings for "unused static function"

static inline istream& operator >> (istream &in, ExtendedProfileName &profile)
{
  return readStrToEnum(strToExtendedProfile, sizeof(strToExtendedProfile)/sizeof(*strToExtendedProfile), in, profile);
}

namespace Level
{
  static inline istream& operator >> (istream &in, Tier &tier)
  {
    return readStrToEnum(strToTier, sizeof(strToTier)/sizeof(*strToTier), in, tier);
  }

  static inline istream& operator >> (istream &in, Name &level)
  {
    return readStrToEnum(strToLevel, sizeof(strToLevel)/sizeof(*strToLevel), in, level);
  }
}

static inline istream& operator >> (istream &in, CostMode &mode)
{
  return readStrToEnum(strToCostMode, sizeof(strToCostMode)/sizeof(*strToCostMode), in, mode);
}

static inline istream& operator >> (istream &in, ScalingListMode &mode)
{
  return readStrToEnum(strToScalingListMode, sizeof(strToScalingListMode)/sizeof(*strToScalingListMode), in, mode);
}

#if SVC_EXTENSION
namespace Profile
{
  static inline istream& operator >> (istream &in, Name &profile)
  {
    return readStrToEnum(strToProfile, sizeof(strToProfile)/sizeof(*strToProfile), in, profile);
  }
}
#endif

template <class T>
struct SMultiValueInput
{
  const T              minValIncl;
  const T              maxValIncl;
  const std::size_t    minNumValuesIncl;
  const std::size_t    maxNumValuesIncl; // Use 0 for unlimited
        std::vector<T> values;
  SMultiValueInput() : minValIncl(0), maxValIncl(0), minNumValuesIncl(0), maxNumValuesIncl(0), values() { }
  SMultiValueInput(std::vector<T> &defaults) : minValIncl(0), maxValIncl(0), minNumValuesIncl(0), maxNumValuesIncl(0), values(defaults) { }
  SMultiValueInput(const T &minValue, const T &maxValue, std::size_t minNumberValues=0, std::size_t maxNumberValues=0)
    : minValIncl(minValue), maxValIncl(maxValue), minNumValuesIncl(minNumberValues), maxNumValuesIncl(maxNumberValues), values()  { }
  SMultiValueInput(const T &minValue, const T &maxValue, std::size_t minNumberValues, std::size_t maxNumberValues, const T* defValues, const UInt numDefValues)
    : minValIncl(minValue), maxValIncl(maxValue), minNumValuesIncl(minNumberValues), maxNumValuesIncl(maxNumberValues), values(defValues, defValues+numDefValues)  { }
  SMultiValueInput<T> &operator=(const std::vector<T> &userValues) { values=userValues; return *this; }
  SMultiValueInput<T> &operator=(const SMultiValueInput<T> &userValues) { values=userValues.values; return *this; }

  T readValue(const TChar *&pStr, Bool &bSuccess);

  istream& readValues(std::istream &in);
};

template <class T>
static inline istream& operator >> (std::istream &in, SMultiValueInput<T> &values)
{
  return values.readValues(in);
}

template<>
UInt SMultiValueInput<UInt>::readValue(const TChar *&pStr, Bool &bSuccess)
{
  TChar *eptr;
  UInt val=strtoul(pStr, &eptr, 0);
  pStr=eptr;
  bSuccess=!(*eptr!=0 && !isspace(*eptr) && *eptr!=',') && !(val<minValIncl || val>maxValIncl);
  return val;
}

template<>
Int SMultiValueInput<Int>::readValue(const TChar *&pStr, Bool &bSuccess)
{
  TChar *eptr;
  Int val=strtol(pStr, &eptr, 0);
  pStr=eptr;
  bSuccess=!(*eptr!=0 && !isspace(*eptr) && *eptr!=',') && !(val<minValIncl || val>maxValIncl);
  return val;
}

template<>
Double SMultiValueInput<Double>::readValue(const TChar *&pStr, Bool &bSuccess)
{
  TChar *eptr;
  Double val=strtod(pStr, &eptr);
  pStr=eptr;
  bSuccess=!(*eptr!=0 && !isspace(*eptr) && *eptr!=',') && !(val<minValIncl || val>maxValIncl);
  return val;
}

template<>
Bool SMultiValueInput<Bool>::readValue(const TChar *&pStr, Bool &bSuccess)
{
  TChar *eptr;
  Int val=strtol(pStr, &eptr, 0);
  pStr=eptr;
  bSuccess=!(*eptr!=0 && !isspace(*eptr) && *eptr!=',') && !(val<Int(minValIncl) || val>Int(maxValIncl));
  return val!=0;
}

template <class T>
istream& SMultiValueInput<T>::readValues(std::istream &in)
{
  values.clear();
  string str;
  while (!in.eof())
  {
    string tmp; in >> tmp; str+=" " + tmp;
  }
  if (!str.empty())
  {
    const TChar *pStr=str.c_str();
    // soak up any whitespace
    for(;isspace(*pStr);pStr++);

    while (*pStr != 0)
    {
      Bool bSuccess=true;
      T val=readValue(pStr, bSuccess);
      if (!bSuccess)
      {
        in.setstate(ios::failbit);
        break;
      }

      if (maxNumValuesIncl != 0 && values.size() >= maxNumValuesIncl)
      {
        in.setstate(ios::failbit);
        break;
      }
      values.push_back(val);
      // soak up any whitespace and up to 1 comma.
      for(;isspace(*pStr);pStr++);
      if (*pStr == ',')
      {
        pStr++;
      }
      for(;isspace(*pStr);pStr++);
    }
  }
  if (values.size() < minNumValuesIncl)
  {
    in.setstate(ios::failbit);
  }
  return in;
}

static Void
automaticallySelectRExtProfile(const Bool bUsingGeneralRExtTools,
                               const Bool bUsingChromaQPAdjustment,
                               const Bool bUsingExtendedPrecision,
                               const Bool bIntraConstraintFlag,
                               UInt &bitDepthConstraint,
                               ChromaFormat &chromaFormatConstraint,
                               const Int  maxBitDepth,
                               const ChromaFormat chromaFormat)
{
  // Try to choose profile, according to table in Q1013.
  UInt trialBitDepthConstraint=maxBitDepth;
  if (trialBitDepthConstraint<8)
  {
    trialBitDepthConstraint=8;
  }
  else if (trialBitDepthConstraint==9 || trialBitDepthConstraint==11)
  {
    trialBitDepthConstraint++;
  }
  else if (trialBitDepthConstraint>12)
  {
    trialBitDepthConstraint=16;
  }

  // both format and bit depth constraints are unspecified
  if (bUsingExtendedPrecision || trialBitDepthConstraint==16)
  {
    bitDepthConstraint = 16;
    chromaFormatConstraint = (!bIntraConstraintFlag && chromaFormat==CHROMA_400) ? CHROMA_400 : CHROMA_444;
  }
  else if (bUsingGeneralRExtTools)
  {
    if (chromaFormat == CHROMA_400 && !bIntraConstraintFlag)
    {
      bitDepthConstraint = 16;
      chromaFormatConstraint = CHROMA_400;
    }
    else
    {
      bitDepthConstraint = trialBitDepthConstraint;
      chromaFormatConstraint = CHROMA_444;
    }
  }
  else if (chromaFormat == CHROMA_400)
  {
    if (bIntraConstraintFlag)
    {
      chromaFormatConstraint = CHROMA_420; // there is no intra 4:0:0 profile.
      bitDepthConstraint     = trialBitDepthConstraint;
    }
    else
    {
      chromaFormatConstraint = CHROMA_400;
      bitDepthConstraint     = trialBitDepthConstraint == 8 ? 8 : 12;
    }
  }
  else
  {
    bitDepthConstraint = trialBitDepthConstraint;
    chromaFormatConstraint = chromaFormat;
    if (bUsingChromaQPAdjustment && chromaFormat == CHROMA_420)
    {
      chromaFormatConstraint = CHROMA_422; // 4:2:0 cannot use the chroma qp tool.
    }
    if (chromaFormatConstraint == CHROMA_422 && bitDepthConstraint == 8)
    {
      bitDepthConstraint = 10; // there is no 8-bit 4:2:2 profile.
    }
    if (chromaFormatConstraint == CHROMA_420 && !bIntraConstraintFlag)
    {
      bitDepthConstraint = 12; // there is no 8 or 10-bit 4:2:0 inter RExt profile.
    }
  }
}
// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/** \param  argc        number of arguments
    \param  argv        array of arguments
    \retval             true when success
 */
Bool TAppEncCfg::parseCfg( Int argc, TChar* argv[] )
{
  Bool do_help = false;
  
#if SVC_EXTENSION  
  string* cfg_InputFile     [MAX_LAYERS];
  string* cfg_ReconFile     [MAX_LAYERS];
  string* cfg_dQPFileName   [MAX_LAYERS];
  Double* cfg_fQP           [MAX_LAYERS];
  Int*    cfg_layerId       [MAX_LAYERS];
  Int*    cfg_repFormatIdx  [MAX_LAYERS];
  Int*    cfg_SourceWidth   [MAX_LAYERS]; 
  Int*    cfg_SourceHeight  [MAX_LAYERS];
  Int*    cfg_FrameRate     [MAX_LAYERS];
  Int*    cfg_IntraPeriod   [MAX_LAYERS];
  Int*    cfg_confWinLeft   [MAX_LAYERS];
  Int*    cfg_confWinRight  [MAX_LAYERS];
  Int*    cfg_confWinTop    [MAX_LAYERS];
  Int*    cfg_confWinBottom [MAX_LAYERS];
  Int*    cfg_aiPadX        [MAX_LAYERS];
  Int*    cfg_aiPadY        [MAX_LAYERS];
  Int*    cfg_conformanceMode  [MAX_LAYERS];

  Int*    cfg_maxCuDQPDepth[MAX_LAYERS];

  // coding unit (CU) definition
  UInt*   cfg_uiMaxCUWidth[MAX_LAYERS];                                   ///< max. CU width in pixel
  UInt*   cfg_uiMaxCUHeight[MAX_LAYERS];                                  ///< max. CU height in pixel
  UInt*   cfg_uiMaxCUDepth[MAX_LAYERS];                                   ///< max. CU depth

  // transfom unit (TU) definition
  UInt*   cfg_uiQuadtreeTULog2MaxSize[MAX_LAYERS];
  UInt*   cfg_uiQuadtreeTULog2MinSize[MAX_LAYERS];
  
  UInt*   cfg_uiQuadtreeTUMaxDepthInter[MAX_LAYERS];
  UInt*   cfg_uiQuadtreeTUMaxDepthIntra[MAX_LAYERS];

#if AUXILIARY_PICTURES
  Int*    cfg_auxId[MAX_LAYERS];
#endif
#if VIEW_SCALABILITY
  Int*     cfg_viewOrderIndex         [MAX_LAYERS];
  Int*     cfg_viewId                 [MAX_LAYERS]; 
#endif

  Int*    cfg_numSamplePredRefLayers  [MAX_LAYERS];
  string  cfg_samplePredRefLayerIds   [MAX_LAYERS];
  string* cfg_samplePredRefLayerIdsPtr[MAX_LAYERS];
  Int*    cfg_numMotionPredRefLayers  [MAX_LAYERS];
  string  cfg_motionPredRefLayerIds   [MAX_LAYERS];
  string* cfg_motionPredRefLayerIdsPtr[MAX_LAYERS];
  Int*    cfg_numActiveRefLayers [MAX_LAYERS];
  string  cfg_predLayerIds       [MAX_LAYERS];
  string* cfg_predLayerIdsPtr    [MAX_LAYERS];

  string  cfg_refLocationOffsetLayerId [MAX_LAYERS];
  string  cfg_scaledRefLayerLeftOffset [MAX_LAYERS];
  string  cfg_scaledRefLayerTopOffset [MAX_LAYERS];
  string  cfg_scaledRefLayerRightOffset [MAX_LAYERS];
  string  cfg_scaledRefLayerBottomOffset [MAX_LAYERS];
  Int*    cfg_numRefLayerLocationOffsets[MAX_LAYERS];
  string  cfg_scaledRefLayerOffsetPresentFlag [MAX_LAYERS];
  string  cfg_refRegionOffsetPresentFlag      [MAX_LAYERS];
  string  cfg_refRegionLeftOffset   [MAX_LAYERS];
  string  cfg_refRegionTopOffset    [MAX_LAYERS];
  string  cfg_refRegionRightOffset  [MAX_LAYERS];
  string  cfg_refRegionBottomOffset [MAX_LAYERS];
  string  cfg_resamplePhaseSetPresentFlag [MAX_LAYERS];
  string  cfg_phaseHorLuma   [MAX_LAYERS];
  string  cfg_phaseVerLuma   [MAX_LAYERS];
  string  cfg_phaseHorChroma [MAX_LAYERS];
  string  cfg_phaseVerChroma [MAX_LAYERS];
  string* cfg_refLocationOffsetLayerIdPtr   [MAX_LAYERS];
  string* cfg_scaledRefLayerLeftOffsetPtr   [MAX_LAYERS];
  string* cfg_scaledRefLayerTopOffsetPtr    [MAX_LAYERS];
  string* cfg_scaledRefLayerRightOffsetPtr  [MAX_LAYERS];
  string* cfg_scaledRefLayerBottomOffsetPtr [MAX_LAYERS];
  string* cfg_scaledRefLayerOffsetPresentFlagPtr [MAX_LAYERS];
  string* cfg_refRegionOffsetPresentFlagPtr      [MAX_LAYERS];
  string* cfg_refRegionLeftOffsetPtr   [MAX_LAYERS];
  string* cfg_refRegionTopOffsetPtr    [MAX_LAYERS];
  string* cfg_refRegionRightOffsetPtr  [MAX_LAYERS];
  string* cfg_refRegionBottomOffsetPtr [MAX_LAYERS];
  string* cfg_resamplePhaseSetPresentFlagPtr [MAX_LAYERS];
  string* cfg_phaseHorLumaPtr   [MAX_LAYERS];
  string* cfg_phaseVerLumaPtr   [MAX_LAYERS];
  string* cfg_phaseHorChromaPtr [MAX_LAYERS];
  string* cfg_phaseVerChromaPtr [MAX_LAYERS];

#if RC_SHVC_HARMONIZATION
  Bool*   cfg_RCEnableRateControl  [MAX_LAYERS];
  Int*    cfg_RCTargetBitRate      [MAX_LAYERS];
  Bool*   cfg_RCKeepHierarchicalBit[MAX_LAYERS];
  Bool*   cfg_RCLCULevelRC         [MAX_LAYERS];
  Bool*   cfg_RCUseLCUSeparateModel[MAX_LAYERS];
  Int*    cfg_RCInitialQP          [MAX_LAYERS];
  Bool*   cfg_RCForceIntraQP       [MAX_LAYERS];

#if U0132_TARGET_BITS_SATURATION
  Bool*   cfg_RCCpbSaturationEnabled[MAX_LAYERS];
  UInt*   cfg_RCCpbSize             [MAX_LAYERS];
  Double* cfg_RCInitialCpbFullness  [MAX_LAYERS];
#endif
#endif

  Int*    cfg_InputBitDepth    [MAX_NUM_CHANNEL_TYPE][MAX_LAYERS];
  Int*    cfg_InternalBitDepth [MAX_NUM_CHANNEL_TYPE][MAX_LAYERS];
  Int*    cfg_OutputBitDepth   [MAX_NUM_CHANNEL_TYPE][MAX_LAYERS];

  Int*    cfg_maxTidIlRefPicsPlus1[MAX_LAYERS]; 
  string* cfg_colourRemapSEIFileRoot[MAX_LAYERS];
  Bool*   cfg_entropyCodingSyncEnabledFlag[MAX_LAYERS];
  Int*    cfg_layerSwitchOffBegin[MAX_LAYERS];
  Int*    cfg_layerSwitchOffEnd[MAX_LAYERS];
  Int*    cfg_layerPTLIdx[MAX_VPS_LAYER_IDX_PLUS1];
  string* cfg_scalingListFileName[MAX_LAYERS];
  ScalingListMode*   cfg_UseScalingListId[MAX_LAYERS];
  Int*    cfg_inheritCodingStruct[MAX_LAYERS];

  Bool*   cfg_bUseSAO[MAX_LAYERS];

#if PER_LAYER_LOSSLESS
  Bool*     cfg_TransquantBypassEnabledFlag[MAX_LAYERS];
  Bool*     cfg_CUTransquantBypassFlagForce[MAX_LAYERS];
  CostMode* cfg_costMode[MAX_LAYERS];  
#endif

  for( UInt layer = 0; layer < m_numLayers; layer++ )
  {
    cfg_InputFile[layer]    = &m_apcLayerCfg[layer]->m_inputFileName;
    cfg_ReconFile[layer]    = &m_apcLayerCfg[layer]->m_reconFileName;
    cfg_dQPFileName[layer]  = &m_apcLayerCfg[layer]->m_dQPFileName;
    cfg_fQP[layer]          = &m_apcLayerCfg[layer]->m_fQP;
    cfg_colourRemapSEIFileRoot[layer] = &m_apcLayerCfg[layer]->m_colourRemapSEIFileRoot;
    cfg_repFormatIdx[layer]         = &m_apcLayerCfg[layer]->m_repFormatIdx;
    cfg_layerId[layer]              = &m_apcLayerCfg[layer]->m_layerId;
    cfg_SourceWidth[layer]          = &m_apcLayerCfg[layer]->m_iSourceWidth;
    cfg_SourceHeight[layer]         = &m_apcLayerCfg[layer]->m_iSourceHeight;
    cfg_FrameRate[layer]            = &m_apcLayerCfg[layer]->m_iFrameRate; 
    cfg_IntraPeriod[layer]          = &m_apcLayerCfg[layer]->m_iIntraPeriod; 
    cfg_conformanceMode[layer]      = &m_apcLayerCfg[layer]->m_conformanceWindowMode;
    cfg_confWinLeft[layer]          = &m_apcLayerCfg[layer]->m_confWinLeft;
    cfg_confWinRight[layer]         = &m_apcLayerCfg[layer]->m_confWinRight;
    cfg_confWinTop[layer]           = &m_apcLayerCfg[layer]->m_confWinTop;
    cfg_confWinBottom[layer]        = &m_apcLayerCfg[layer]->m_confWinBottom;
    cfg_aiPadX[layer]               = &m_apcLayerCfg[layer]->m_aiPad[0];
    cfg_aiPadY[layer]               = &m_apcLayerCfg[layer]->m_aiPad[1];

    cfg_maxCuDQPDepth[layer]        = &m_apcLayerCfg[layer]->m_iMaxCuDQPDepth;

    // coding unit (CU) definition
    cfg_uiMaxCUWidth[layer]  = &m_apcLayerCfg[layer]->m_uiMaxCUWidth;
    cfg_uiMaxCUHeight[layer] = &m_apcLayerCfg[layer]->m_uiMaxCUHeight;
    cfg_uiMaxCUDepth[layer]  = &m_apcLayerCfg[layer]->m_uiMaxCUDepth;

    // transfom unit (TU) definition.
    cfg_uiQuadtreeTULog2MaxSize[layer] = &m_apcLayerCfg[layer]->m_uiQuadtreeTULog2MaxSize;
    cfg_uiQuadtreeTULog2MinSize[layer] = &m_apcLayerCfg[layer]->m_uiQuadtreeTULog2MinSize;

    cfg_uiQuadtreeTUMaxDepthInter[layer] = &m_apcLayerCfg[layer]->m_uiQuadtreeTUMaxDepthInter;
    cfg_uiQuadtreeTUMaxDepthIntra[layer] = &m_apcLayerCfg[layer]->m_uiQuadtreeTUMaxDepthIntra;

    cfg_numSamplePredRefLayers  [layer] = &m_apcLayerCfg[layer]->m_numSamplePredRefLayers;
    cfg_samplePredRefLayerIdsPtr[layer] = &cfg_samplePredRefLayerIds[layer];
    cfg_numMotionPredRefLayers  [layer] = &m_apcLayerCfg[layer]->m_numMotionPredRefLayers;
    cfg_motionPredRefLayerIdsPtr[layer] = &cfg_motionPredRefLayerIds[layer];
    cfg_numActiveRefLayers      [layer] = &m_apcLayerCfg[layer]->m_numActiveRefLayers;
    cfg_predLayerIdsPtr         [layer] = &cfg_predLayerIds[layer];
    cfg_scalingListFileName     [layer] = &m_apcLayerCfg[layer]->m_scalingListFileName;

    cfg_numRefLayerLocationOffsets [layer]  = &m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets;
    cfg_entropyCodingSyncEnabledFlag[layer] = &m_apcLayerCfg[layer]->m_entropyCodingSyncEnabledFlag;
    for(Int i = 0; i < m_numLayers; i++)
    {
      cfg_refLocationOffsetLayerIdPtr  [layer] = &cfg_refLocationOffsetLayerId[layer];
      cfg_scaledRefLayerLeftOffsetPtr  [layer] = &cfg_scaledRefLayerLeftOffset[layer];
      cfg_scaledRefLayerTopOffsetPtr   [layer] = &cfg_scaledRefLayerTopOffset[layer];
      cfg_scaledRefLayerRightOffsetPtr [layer] = &cfg_scaledRefLayerRightOffset[layer];
      cfg_scaledRefLayerBottomOffsetPtr[layer] = &cfg_scaledRefLayerBottomOffset[layer];
      cfg_scaledRefLayerOffsetPresentFlagPtr [layer] = &cfg_scaledRefLayerOffsetPresentFlag [layer];
      cfg_refRegionOffsetPresentFlagPtr      [layer] = &cfg_refRegionOffsetPresentFlag      [layer];
      cfg_refRegionLeftOffsetPtr  [layer] = &cfg_refRegionLeftOffset  [layer];
      cfg_refRegionTopOffsetPtr   [layer] = &cfg_refRegionTopOffset   [layer];
      cfg_refRegionRightOffsetPtr [layer] = &cfg_refRegionRightOffset [layer];
      cfg_refRegionBottomOffsetPtr[layer] = &cfg_refRegionBottomOffset[layer];
      cfg_resamplePhaseSetPresentFlagPtr [layer] = &cfg_resamplePhaseSetPresentFlag [layer];
      cfg_phaseHorLumaPtr   [layer] = &cfg_phaseHorLuma   [layer];
      cfg_phaseVerLumaPtr   [layer] = &cfg_phaseVerLuma   [layer];
      cfg_phaseHorChromaPtr [layer] = &cfg_phaseHorChroma [layer];
      cfg_phaseVerChromaPtr [layer] = &cfg_phaseVerChroma [layer];
    }
#if RC_SHVC_HARMONIZATION
    cfg_RCEnableRateControl[layer]   = &m_apcLayerCfg[layer]->m_RCEnableRateControl;
    cfg_RCTargetBitRate[layer]       = &m_apcLayerCfg[layer]->m_RCTargetBitrate;
    cfg_RCKeepHierarchicalBit[layer] = &m_apcLayerCfg[layer]->m_RCKeepHierarchicalBit;
    cfg_RCLCULevelRC[layer]          = &m_apcLayerCfg[layer]->m_RCLCULevelRC;
    cfg_RCUseLCUSeparateModel[layer] = &m_apcLayerCfg[layer]->m_RCUseLCUSeparateModel;
    cfg_RCInitialQP[layer]           = &m_apcLayerCfg[layer]->m_RCInitialQP;
    cfg_RCForceIntraQP[layer]        = &m_apcLayerCfg[layer]->m_RCForceIntraQP;

#if U0132_TARGET_BITS_SATURATION
    cfg_RCCpbSaturationEnabled[layer] = &m_apcLayerCfg[layer]->m_RCCpbSaturationEnabled;
    cfg_RCCpbSize[layer]              = &m_apcLayerCfg[layer]->m_RCCpbSize;
    cfg_RCInitialCpbFullness[layer]   = &m_apcLayerCfg[layer]->m_RCInitialCpbFullness;
#endif

    cfg_inheritCodingStruct[layer]    = &m_apcLayerCfg[layer]->m_inheritCodingStruct;
#endif

    cfg_InputBitDepth   [CHANNEL_TYPE_LUMA][layer] = &m_apcLayerCfg[layer]->m_inputBitDepth[CHANNEL_TYPE_LUMA];
    cfg_InternalBitDepth[CHANNEL_TYPE_LUMA][layer] = &m_apcLayerCfg[layer]->m_internalBitDepth[CHANNEL_TYPE_LUMA];
    cfg_OutputBitDepth  [CHANNEL_TYPE_LUMA][layer] = &m_apcLayerCfg[layer]->m_outputBitDepth[CHANNEL_TYPE_LUMA];
    cfg_InternalBitDepth[CHANNEL_TYPE_CHROMA][layer] = &m_apcLayerCfg[layer]->m_internalBitDepth[CHANNEL_TYPE_CHROMA];
    cfg_InputBitDepth   [CHANNEL_TYPE_CHROMA][layer] = &m_apcLayerCfg[layer]->m_inputBitDepth[CHANNEL_TYPE_CHROMA];
    cfg_OutputBitDepth  [CHANNEL_TYPE_CHROMA][layer] = &m_apcLayerCfg[layer]->m_outputBitDepth[CHANNEL_TYPE_CHROMA];

    cfg_maxTidIlRefPicsPlus1[layer] = &m_apcLayerCfg[layer]->m_maxTidIlRefPicsPlus1;
#if AUXILIARY_PICTURES
    cfg_auxId[layer]                = &m_apcLayerCfg[layer]->m_auxId; 
#endif
#if VIEW_SCALABILITY
    cfg_viewOrderIndex[layer]       = &m_apcLayerCfg[layer]->m_viewOrderIndex;
    cfg_viewId[layer]               = &m_apcLayerCfg[layer]->m_viewId;
#endif
    cfg_layerSwitchOffBegin[layer]  = &m_apcLayerCfg[layer]->m_layerSwitchOffBegin;
    cfg_layerSwitchOffEnd[layer]    = &m_apcLayerCfg[layer]->m_layerSwitchOffEnd;
    cfg_layerPTLIdx[layer]          = &m_apcLayerCfg[layer]->m_layerPTLIdx;

    cfg_UseScalingListId[layer]     = &m_apcLayerCfg[layer]->m_useScalingListId;

    cfg_bUseSAO[layer]              = &m_apcLayerCfg[layer]->m_bUseSAO;

#if PER_LAYER_LOSSLESS
    cfg_TransquantBypassEnabledFlag[layer]  = &m_apcLayerCfg[layer]->m_TransquantBypassEnabledFlag;
    cfg_CUTransquantBypassFlagForce[layer]  = &m_apcLayerCfg[layer]->m_CUTransquantBypassFlagForce;
    cfg_costMode[layer]                     = &m_apcLayerCfg[layer]->m_costMode;  
#endif
  }

  Int* cfg_numLayerInIdList[MAX_VPS_LAYER_SETS_PLUS1];
  string cfg_layerSetLayerIdList[MAX_VPS_LAYER_SETS_PLUS1];
  string* cfg_layerSetLayerIdListPtr[MAX_VPS_LAYER_SETS_PLUS1];
  Int* cfg_numHighestLayerIdx[MAX_VPS_LAYER_SETS_PLUS1];
  string cfg_highestLayerIdx[MAX_VPS_LAYER_SETS_PLUS1];
  string* cfg_highestLayerIdxPtr[MAX_VPS_LAYER_SETS_PLUS1];

  for (UInt i = 0; i < MAX_VPS_LAYER_SETS_PLUS1; i++)
  {
    cfg_numLayerInIdList[i] = &m_numLayerInIdList[i];
    cfg_layerSetLayerIdListPtr[i] = &cfg_layerSetLayerIdList[i];
    cfg_highestLayerIdxPtr[i] = &cfg_highestLayerIdx[i];
    cfg_numHighestLayerIdx[i] = &m_numHighestLayerIdx[i];
  }

  string* cfg_numOutputLayersInOutputLayerSet = new string;
  string* cfg_listOfOutputLayers     = new string[MAX_VPS_OUTPUT_LAYER_SETS_PLUS1];
  string* cfg_outputLayerSetIdx      = new string;
  string* cfg_listOfLayerPTLOfOlss   = new string[MAX_VPS_OUTPUT_LAYER_SETS_PLUS1];
#if AVC_BASE
  string  m_inputFileNameBL;
#endif
#if N0383_IL_CONSTRAINED_TILE_SETS_SEI
  string  m_tileSets;
#endif 
#endif //SVC_EXTENSION

  Int tmpChromaFormat;
  Int tmpInputChromaFormat;
  Int tmpConstraintChromaFormat;
  Int tmpWeightedPredictionMethod;
  Int tmpFastInterSearchMode;
  Int tmpMotionEstimationSearchMethod;
  Int tmpSliceMode;
  Int tmpSliceSegmentMode;
  Int tmpDecodedPictureHashSEIMappedType;
  string inputColourSpaceConvert;
#if SVC_EXTENSION
  ExtendedProfileName extendedProfile[MAX_NUM_LAYER_IDS + 1];

  UInt tmpBitDepthConstraint;
  Bool tmpIntraConstraintFlag, tmpOnePictureOnlyConstraintFlag, tmpLowerBitRateConstraintFlag;
#else
  ExtendedProfileName extendedProfile;
#endif
  Int saoOffsetBitShift[MAX_NUM_CHANNEL_TYPE];

  // Multi-value input fields:                                // minval, maxval (incl), min_entries, max_entries (incl) [, default values, number of default values]
  SMultiValueInput<UInt> cfg_ColumnWidth                     (0, std::numeric_limits<UInt>::max(), 0, std::numeric_limits<UInt>::max());
  SMultiValueInput<UInt> cfg_RowHeight                       (0, std::numeric_limits<UInt>::max(), 0, std::numeric_limits<UInt>::max());
  SMultiValueInput<Int>  cfg_startOfCodedInterval            (std::numeric_limits<Int>::min(), std::numeric_limits<Int>::max(), 0, 1<<16);
  SMultiValueInput<Int>  cfg_codedPivotValue                 (std::numeric_limits<Int>::min(), std::numeric_limits<Int>::max(), 0, 1<<16);
  SMultiValueInput<Int>  cfg_targetPivotValue                (std::numeric_limits<Int>::min(), std::numeric_limits<Int>::max(), 0, 1<<16);

  SMultiValueInput<Double> cfg_adIntraLambdaModifier         (0, std::numeric_limits<Double>::max(), 0, MAX_TLAYER); ///< Lambda modifier for Intra pictures, one for each temporal layer. If size>temporalLayer, then use [temporalLayer], else if size>0, use [size()-1], else use m_adLambdaModifier.


  const UInt defaultInputKneeCodes[3]  = { 600, 800, 900 };
  const UInt defaultOutputKneeCodes[3] = { 100, 250, 450 };
  SMultiValueInput<UInt> cfg_kneeSEIInputKneePointValue      (1,  999, 0, 999, defaultInputKneeCodes,  sizeof(defaultInputKneeCodes )/sizeof(UInt));
  SMultiValueInput<UInt> cfg_kneeSEIOutputKneePointValue     (0, 1000, 0, 999, defaultOutputKneeCodes, sizeof(defaultOutputKneeCodes)/sizeof(UInt));
  const Int defaultPrimaryCodes[6]     = { 0,50000, 0,0, 50000,0 };
  const Int defaultWhitePointCode[2]   = { 16667, 16667 };
  SMultiValueInput<Int>  cfg_DisplayPrimariesCode            (0, 50000, 6, 6, defaultPrimaryCodes,   sizeof(defaultPrimaryCodes  )/sizeof(Int));
  SMultiValueInput<Int>  cfg_DisplayWhitePointCode           (0, 50000, 2, 2, defaultWhitePointCode, sizeof(defaultWhitePointCode)/sizeof(Int));

  SMultiValueInput<Bool> cfg_timeCodeSeiTimeStampFlag        (0,  1, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Bool> cfg_timeCodeSeiNumUnitFieldBasedFlag(0,  1, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Int>  cfg_timeCodeSeiCountingType         (0,  6, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Bool> cfg_timeCodeSeiFullTimeStampFlag    (0,  1, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Bool> cfg_timeCodeSeiDiscontinuityFlag    (0,  1, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Bool> cfg_timeCodeSeiCntDroppedFlag       (0,  1, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Int>  cfg_timeCodeSeiNumberOfFrames       (0,511, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Int>  cfg_timeCodeSeiSecondsValue         (0, 59, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Int>  cfg_timeCodeSeiMinutesValue         (0, 59, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Int>  cfg_timeCodeSeiHoursValue           (0, 23, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Bool> cfg_timeCodeSeiSecondsFlag          (0,  1, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Bool> cfg_timeCodeSeiMinutesFlag          (0,  1, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Bool> cfg_timeCodeSeiHoursFlag            (0,  1, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Int>  cfg_timeCodeSeiTimeOffsetLength     (0, 31, 0, MAX_TIMECODE_SEI_SETS);
  SMultiValueInput<Int>  cfg_timeCodeSeiTimeOffsetValue      (std::numeric_limits<Int>::min(), std::numeric_limits<Int>::max(), 0, MAX_TIMECODE_SEI_SETS);
  Int warnUnknowParameter = 0;

#if Q0096_OVERLAY_SEI
  const Int CFG_MAX_OVERLAYS = 3;
  UInt   cfg_overlaySEIIdx[CFG_MAX_OVERLAYS];  
  Bool   cfg_overlaySEILanguagePresentFlag[CFG_MAX_OVERLAYS];  
  UInt   cfg_overlaySEIContentLayerId[CFG_MAX_OVERLAYS];
  Bool   cfg_overlaySEILabelPresentFlag[CFG_MAX_OVERLAYS];
  UInt   cfg_overlaySEILabelLayerId[CFG_MAX_OVERLAYS];
  Bool   cfg_overlaySEIAlphaPresentFlag[CFG_MAX_OVERLAYS];
  UInt   cfg_overlaySEIAlphaLayerId[CFG_MAX_OVERLAYS];
  UInt   cfg_overlaySEINumElementsMinus1[CFG_MAX_OVERLAYS];
  string cfg_overlaySEIElementLabelRanges[CFG_MAX_OVERLAYS];  
  string cfg_overlaySEILanguage[CFG_MAX_OVERLAYS];  
  string cfg_overlaySEIName[CFG_MAX_OVERLAYS];  
  string cfg_overlaySEIElementNames[CFG_MAX_OVERLAYS];  
#endif

  po::Options opts;
  opts.addOptions()
  ("help",                                            do_help,                                          false, "this help text")
  ("c",    po::parseConfigFile, "configuration file name")
  ("WarnUnknowParameter,w",                           warnUnknowParameter,                                  0, "warn for unknown configuration parameters instead of failing")

  // File, I/O and source parameters
#if SVC_EXTENSION
  ("InputFile%d,-i%d",                              cfg_InputFile,                    string(""), m_numLayers, "original YUV input file name for layer %d")
  ("ReconFile%d,-o%d",                              cfg_ReconFile,                    string(""), m_numLayers, "reconstruction YUV input file name for layer %d")
  ("SourceWidth%d,-wdt%d",                          cfg_SourceWidth,                           0, m_numLayers, "Source picture width for layer %d")
  ("SourceHeight%d,-hgt%d",                         cfg_SourceHeight,                          0, m_numLayers, "Source picture height for layer %d")
  ("FrameRate%d,-fr%d",                             cfg_FrameRate,                             0, m_numLayers, "Frame rate for layer %d")
  ("LambdaModifier%d,-LM%d",                        m_adLambdaModifier,               Double(1.0), MAX_TLAYER, "Lambda modifier for temporal layer %d")
  ("RepFormatIdx%d",                                cfg_repFormatIdx,                         -1, m_numLayers, "Index to the representation format structure used from the VPS")
  ("LayerId%d",                                     cfg_layerId,                              -1, m_numLayers,  "Layer id")

  ("NumSamplePredRefLayers%d",                      cfg_numSamplePredRefLayers,               -1, m_numLayers, "Number of sample prediction reference layers")
  ("SamplePredRefLayerIds%d",                       cfg_samplePredRefLayerIdsPtr,     string(""), m_numLayers, "sample pred reference layer IDs")
  ("NumMotionPredRefLayers%d",                      cfg_numMotionPredRefLayers,               -1, m_numLayers, "Number of motion prediction reference layers")
  ("MotionPredRefLayerIds%d",                       cfg_motionPredRefLayerIdsPtr,     string(""), m_numLayers, "motion pred reference layer IDs")
  ("NumActiveRefLayers%d",                          cfg_numActiveRefLayers,                   -1, m_numLayers, "Number of active reference layers")
  ("PredLayerIds%d",                                cfg_predLayerIdsPtr,              string(""), m_numLayers, "inter-layer prediction layer IDs")

  ("NumLayers",                                     m_numLayers,                                            1, "Number of layers to code")  

  ("NumLayerSets",                                  m_numLayerSets,                                         1, "Number of layer sets")
  ("NumLayerInIdList%d",                            cfg_numLayerInIdList,          0, MAX_VPS_LAYER_IDX_PLUS1, "Number of layers in the set")
  ("LayerSetLayerIdList%d",                   cfg_layerSetLayerIdListPtr, string(""), MAX_VPS_LAYER_IDX_PLUS1, "Layer IDs for the set")
  ("NumAddLayerSets",                               m_numAddLayerSets,                                      0, "Number of additional layer sets")
  ("NumHighestLayerIdx%d",                        cfg_numHighestLayerIdx,          0, MAX_VPS_LAYER_IDX_PLUS1, "Number of highest layer idx")
  ("HighestLayerIdx%d",                           cfg_highestLayerIdxPtr, string(""), MAX_VPS_LAYER_IDX_PLUS1, "Highest layer idx for an additional layer set")

  ("DefaultTargetOutputLayerIdc",                   m_defaultTargetOutputLayerIdc,                          1, "Default target output layers. 0: All layers are output layer, 1: Only highest layer is output layer, 2 or 3: No default output layers")
  ("NumOutputLayerSets",                            m_numOutputLayerSets,                                   1, "Number of output layer sets excluding the 0-th output layer set")
  ("NumOutputLayersInOutputLayerSet",               cfg_numOutputLayersInOutputLayerSet,        string(""), 1, "List containing number of output layers in the output layer sets")
  ("ListOfOutputLayers%d",                        cfg_listOfOutputLayers, string(""), MAX_VPS_LAYER_IDX_PLUS1, "Layer IDs for the set, in terms of layer ID in the output layer set Range: [0..NumLayersInOutputLayerSet-1]")
  ("OutputLayerSetIdx",                             cfg_outputLayerSetIdx,                      string(""), 1, "Corresponding layer set index, only for non-default output layer sets")
#if AUXILIARY_PICTURES
  ("AuxId%d",                                       cfg_auxId,                                 0, m_numLayers, "Auxilary picture ID for layer %d (0: Not aux pic, 1: Alpha plane, 2: Depth picture, 3: Cb enh, 4: Cr enh")
#endif
#if VIEW_SCALABILITY
  ("ViewOrderIndex%d",                              cfg_viewOrderIndex,                       -1, m_numLayers, "View Order Index per layer")
  ("ViewId%d",                                      cfg_viewId,                               -1, m_numLayers, "View Id per View Order Index")
#endif
  ("ConformanceMode%d",                             cfg_conformanceMode,                       0, m_numLayers, "Window conformance mode (0: no cropping, 1:automatic padding, 2: padding, 3:cropping")
  ("ConfLeft%d",                                    cfg_confWinLeft,                           0, m_numLayers, "Deprecated alias of ConfWinLeft")
  ("ConfRight%d",                                   cfg_confWinRight,                          0, m_numLayers, "Deprecated alias of ConfWinRight")
  ("ConfTop%d",                                     cfg_confWinTop,                            0, m_numLayers, "Deprecated alias of ConfWinTop")
  ("ConfBottom%d",                                  cfg_confWinBottom,                         0, m_numLayers, "Deprecated alias of ConfWinBottom")
  ("ConfWinLeft%d",                                 cfg_confWinLeft,                           0, m_numLayers, "Left offset for window conformance mode 3")
  ("ConfWinRight%d",                                cfg_confWinRight,                          0, m_numLayers, "Right offset for window conformance mode 3")
  ("ConfWinTop%d",                                  cfg_confWinTop,                            0, m_numLayers, "Top offset for window conformance mode 3")
  ("ConfWinBottom%d",                               cfg_confWinBottom,                         0, m_numLayers, "Bottom offset for window conformance mode 3")
  ("HorizontalPadding%d,-pdx%d",                    cfg_aiPadX,                                0, m_numLayers, "Horizontal source padding for conformance window mode 2")
  ("VerticalPadding%d,-pdy%d",                      cfg_aiPadY,                                0, m_numLayers, "Vertical source padding for conformance window mode 2")
  ("ScalabilityMask1",                              m_scalabilityMask[VIEW_ORDER_INDEX],                    0, "scalability_mask[1] (multiview)")
  ("ScalabilityMask2",                              m_scalabilityMask[SCALABILITY_ID],                      1, "scalability_mask[2] (scalable)" )
#if AUXILIARY_PICTURES
  ("ScalabilityMask3",                              m_scalabilityMask[AUX_ID],                              0, "scalability_mask[3] (auxiliary pictures)" )
#endif
  ("BitstreamFile,b",                               m_bitstreamFileName,                           string(""), "Bitstream output file name")
  ("NumRefLocationOffsets%d",                       cfg_numRefLayerLocationOffsets,            0, m_numLayers, "Number of reference layer offset sets ")
  ("RefLocationOffsetLayerId%d",                    cfg_refLocationOffsetLayerIdPtr,  string(""), m_numLayers, "Layer ID of reference location offset")
  ("ScaledRefLayerLeftOffset%d",                    cfg_scaledRefLayerLeftOffsetPtr,  string(""), m_numLayers, "Horizontal offset of top-left luma sample of scaled base layer picture with respect to"
                                                                                                                " top-left luma sample of the EL picture, in units of two luma samples")
  ("ScaledRefLayerTopOffset%d",                     cfg_scaledRefLayerTopOffsetPtr,   string(""), m_numLayers, "Vertical offset of top-left luma sample of scaled base layer picture with respect to"
                                                                                                                " top-left luma sample of the EL picture, in units of two luma samples")
  ("ScaledRefLayerRightOffset%d",                   cfg_scaledRefLayerRightOffsetPtr, string(""), m_numLayers, "Horizontal offset of bottom-right luma sample of scaled base layer picture with respect to"
                                                                                                                " bottom-right luma sample of the EL picture, in units of two luma samples")
  ("ScaledRefLayerBottomOffset%d",                  cfg_scaledRefLayerBottomOffsetPtr,string(""), m_numLayers, "Vertical offset of bottom-right luma sample of scaled base layer picture with respect to"
                                                                                                                "  bottom-right luma sample of the EL picture, in units of two luma samples")
  ("ScaledRefLayerOffsetPresentFlag%d",        cfg_scaledRefLayerOffsetPresentFlagPtr,string(""), m_numLayers, "presense flag of scaled reference layer offsets")
  ("RefRegionOffsetPresentFlag%d",                  cfg_refRegionOffsetPresentFlagPtr,string(""), m_numLayers, "presense flag of reference region offsets")
  ("RefRegionLeftOffset%d",                         cfg_refRegionLeftOffsetPtr,       string(""), m_numLayers, "Horizontal offset of top-left luma sample of ref region with respect to"
                                                                                                                "  top-left luma sample of the BL picture, in units of two luma samples")
  ("RefRegionTopOffset%d",                          cfg_refRegionTopOffsetPtr,        string(""), m_numLayers, "Vertical offset of top-left luma sample of ref region with respect to"
                                                                                                                "  top-left luma sample of the BL picture, in units of two luma samples")
  ("RefRegionRightOffset%d",                        cfg_refRegionRightOffsetPtr,      string(""), m_numLayers, "Horizontal offset of bottom-right luma sample of ref region with respect to"
                                                                                                                "  bottom-right luma sample of the BL picture, in units of two luma samples")
  ("RefRegionBottomOffset%d",                       cfg_refRegionBottomOffsetPtr,     string(""), m_numLayers, "Vertical offset of bottom-right luma sample of ref region with respect to"
                                                                                                                "  bottom-right luma sample of the BL picture, in units of two luma samples")
  ("ResamplePhaseSetPresentFlag%d",                cfg_resamplePhaseSetPresentFlagPtr,string(""), m_numLayers, "presense flag of resample phase set")
  ("PhaseHorLuma%d",                                cfg_phaseHorLumaPtr,              string(""), m_numLayers, "luma shift in the horizontal direction used in resampling proces")
  ("PhaseVerLuma%d",                                cfg_phaseVerLumaPtr,              string(""), m_numLayers, "luma shift in the vertical   direction used in resampling proces")
  ("PhaseHorChroma%d",                              cfg_phaseHorChromaPtr,            string(""), m_numLayers, "chroma shift in the horizontal direction used in resampling proces")
  ("PhaseVerChroma%d",                              cfg_phaseVerChromaPtr,            string(""), m_numLayers, "chroma shift in the vertical   direction used in resampling proces")
  ("SEIColourRemappingInfoFileRoot%d,-cri",         cfg_colourRemapSEIFileRoot,       string(""), m_numLayers, "Colour Remapping Information SEI parameters root file name (wo num ext)")
  ("InputBitDepth%d",                                cfg_InputBitDepth[CHANNEL_TYPE_LUMA],     8, m_numLayers, "Bit-depth of input file for layer %d")
  ("InternalBitDepth%d",                             cfg_InternalBitDepth[CHANNEL_TYPE_LUMA],  0, m_numLayers, "Bit-depth the codec operates at. (default:InputBitDepth) for layer %d "
                                                                                                               " If different to InputBitDepth, source data will be converted")
  ("OutputBitDepth%d",                               cfg_OutputBitDepth[CHANNEL_TYPE_LUMA],    0, m_numLayers, "Bit-depth of output file (default:InternalBitDepth)")
  ("InputBitDepthC%d",                               cfg_InputBitDepth[CHANNEL_TYPE_CHROMA],   0, m_numLayers, "As per InputBitDepth but for chroma component. (default:InputBitDepth) for layer %d")
  ("InternalBitDepthC%d",                            cfg_InternalBitDepth[CHANNEL_TYPE_CHROMA],0, m_numLayers, "As per InternalBitDepth but for chroma component. (default:InternalBitDepth) for layer %d")
  ("OutputBitDepthC%d",                              cfg_OutputBitDepth[CHANNEL_TYPE_CHROMA],  0, m_numLayers, "As per OutputBitDepth but for chroma component. (default:InternalBitDepthC)")

  ("MaxTidRefPresentFlag",                           m_maxTidRefPresentFlag,                            false, "max_tid_ref_present_flag (0: not present, 1: present) " )
  ("MaxTidIlRefPicsPlus1%d",                         cfg_maxTidIlRefPicsPlus1,                 7, m_numLayers, "allowed maximum temporal_id for inter-layer prediction")
  ("CrossLayerPictureTypeAlignFlag",                 m_crossLayerPictureTypeAlignFlag,                   true, "align picture type across layers" )  
  ("CrossLayerIrapAlignFlag",                        m_crossLayerIrapAlignFlag,                          true, "align IRAP across layers" )  
  ("CrossLayerAlignedIdrOnlyFlag",                   m_crossLayerAlignedIdrOnlyFlag,                     true, "only idr for IRAP across layers" )  
  ("InterLayerWeightedPred",                          m_useInterLayerWeightedPred,                      false, "enable IL WP parameters estimation at encoder" )  
#if AVC_BASE
  ("NonHEVCBase,-nonhevc",                            m_nonHEVCBaseLayerFlag,                           false, "BL is available but not internal")
  ("InputBLFile,-ibl",                                m_inputFileNameBL,                           string(""), "Base layer rec YUV input file name")
#endif
  ("EnableElRapB,-use-rap-b",                         m_elRapSliceBEnabled,                             false, "Set ILP over base-layer I picture to B picture (default is P picture)")
  ("InputChromaFormat",                               tmpInputChromaFormat,                               420, "InputChromaFormatIDC")
  ("ChromaFormatIDC,-cf",                             tmpChromaFormat,                                      0, "ChromaFormatIDC (400|420|422|444 or set 0 (default) for same as InputChromaFormat)")
#else //SVC_EXTENSION
  ("InputFile,i",                                     m_inputFileName,                             string(""), "Original YUV input file name")
  ("BitstreamFile,b",                                 m_bitstreamFileName,                         string(""), "Bitstream output file name")
  ("ReconFile,o",                                     m_reconFileName,                             string(""), "Reconstructed YUV output file name")
  ("SourceWidth,-wdt",                                m_iSourceWidth,                                       0, "Source picture width")
  ("SourceHeight,-hgt",                               m_iSourceHeight,                                      0, "Source picture height")
  ("InputBitDepth",                                   m_inputBitDepth[CHANNEL_TYPE_LUMA],                   8, "Bit-depth of input file")
  ("OutputBitDepth",                                  m_outputBitDepth[CHANNEL_TYPE_LUMA],                  0, "Bit-depth of output file (default:InternalBitDepth)")
  ("MSBExtendedBitDepth",                             m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA],             0, "bit depth of luma component after addition of MSBs of value 0 (used for synthesising High Dynamic Range source material). (default:InputBitDepth)")
  ("InternalBitDepth",                                m_internalBitDepth[CHANNEL_TYPE_LUMA],                0, "Bit-depth the codec operates at. (default:MSBExtendedBitDepth). If different to MSBExtendedBitDepth, source data will be converted")
  ("InputBitDepthC",                                  m_inputBitDepth[CHANNEL_TYPE_CHROMA],                 0, "As per InputBitDepth but for chroma component. (default:InputBitDepth)")
  ("OutputBitDepthC",                                 m_outputBitDepth[CHANNEL_TYPE_CHROMA],                0, "As per OutputBitDepth but for chroma component. (default:InternalBitDepthC)")
  ("MSBExtendedBitDepthC",                            m_MSBExtendedBitDepth[CHANNEL_TYPE_CHROMA],           0, "As per MSBExtendedBitDepth but for chroma component. (default:MSBExtendedBitDepth)")
  ("InternalBitDepthC",                               m_internalBitDepth[CHANNEL_TYPE_CHROMA],              0, "As per InternalBitDepth but for chroma component. (default:InternalBitDepth)")
#endif
  ("ExtendedPrecision",                               m_extendedPrecisionProcessingFlag,                false, "Increased internal accuracies to support high bit depths (not valid in V1 profiles)")
  ("HighPrecisionPredictionWeighting",                m_highPrecisionOffsetsEnabledFlag,                false, "Use high precision option for weighted prediction (not valid in V1 profiles)")
#if !SVC_EXTENSION
  ("InputColourSpaceConvert",                         inputColourSpaceConvert,                     string(""), "Colour space conversion to apply to input video. Permitted values are (empty string=UNCHANGED) " + getListOfColourSpaceConverts(true))
  ("SNRInternalColourSpace",                          m_snrInternalColourSpace,                         false, "If true, then no colour space conversion is applied prior to SNR, otherwise inverse of input is applied.")
  ("OutputInternalColourSpace",                       m_outputInternalColourSpace,                      false, "If true, then no colour space conversion is applied for reconstructed video, otherwise inverse of input is applied.")
  ("InputChromaFormat",                               tmpInputChromaFormat,                               420, "InputChromaFormatIDC")
#endif //SVC_EXTENSION
  ("MSEBasedSequencePSNR",                            m_printMSEBasedSequencePSNR,                      false, "0 (default) emit sequence PSNR only as a linear average of the frame PSNRs, 1 = also emit a sequence PSNR based on an average of the frame MSEs")
  ("PrintFrameMSE",                                   m_printFrameMSE,                                  false, "0 (default) emit only bit count and PSNRs for each frame, 1 = also emit MSE values")
  ("PrintSequenceMSE",                                m_printSequenceMSE,                               false, "0 (default) emit only bit rate and PSNRs for the whole sequence, 1 = also emit MSE values")
  ("CabacZeroWordPaddingEnabled",                     m_cabacZeroWordPaddingEnabled,                     true, "0 do not add conforming cabac-zero-words to bit streams, 1 (default) = add cabac-zero-words as required")
#if !SVC_EXTENSION
  ("ChromaFormatIDC,-cf",                             tmpChromaFormat,                                      0, "ChromaFormatIDC (400|420|422|444 or set 0 (default) for same as InputChromaFormat)")
  ("ConformanceMode",                                 m_conformanceWindowMode,                              0, "Deprecated alias of ConformanceWindowMode")
  ("ConformanceWindowMode",                           m_conformanceWindowMode,                              0, "Window conformance mode (0: no window, 1:automatic padding, 2:padding, 3:conformance")
  ("HorizontalPadding,-pdx",                          m_aiPad[0],                                           0, "Horizontal source padding for conformance window mode 2")
  ("VerticalPadding,-pdy",                            m_aiPad[1],                                           0, "Vertical source padding for conformance window mode 2")
  ("ConfLeft",                                        m_confWinLeft,                                        0, "Deprecated alias of ConfWinLeft")
  ("ConfRight",                                       m_confWinRight,                                       0, "Deprecated alias of ConfWinRight")
  ("ConfTop",                                         m_confWinTop,                                         0, "Deprecated alias of ConfWinTop")
  ("ConfBottom",                                      m_confWinBottom,                                      0, "Deprecated alias of ConfWinBottom")
  ("ConfWinLeft",                                     m_confWinLeft,                                        0, "Left offset for window conformance mode 3")
  ("ConfWinRight",                                    m_confWinRight,                                       0, "Right offset for window conformance mode 3")
  ("ConfWinTop",                                      m_confWinTop,                                         0, "Top offset for window conformance mode 3")
  ("ConfWinBottom",                                   m_confWinBottom,                                      0, "Bottom offset for window conformance mode 3")
#endif
  ("AccessUnitDelimiter",                             m_AccessUnitDelimiter,                            false, "Enable Access Unit Delimiter NALUs")
#if !SVC_EXTENSION
  ("FrameRate,-fr",                                   m_iFrameRate,                                         0, "Frame rate")
#endif
  ("FrameSkip,-fs",                                   m_FrameSkip,                                         0u, "Number of frames to skip at start of input YUV")
  ("TemporalSubsampleRatio,-ts",                      m_temporalSubsampleRatio,                            1u, "Temporal sub-sample ratio when reading input YUV")
  ("FramesToBeEncoded,f",                             m_framesToBeEncoded,                                  0, "Number of frames to be encoded (default=all)")
  ("ClipInputVideoToRec709Range",                     m_bClipInputVideoToRec709Range,                   false, "If true then clip input video to the Rec. 709 Range on loading when InternalBitDepth is less than MSBExtendedBitDepth")
  ("ClipOutputVideoToRec709Range",                    m_bClipOutputVideoToRec709Range,                  false, "If true then clip output video to the Rec. 709 Range on saving when OutputBitDepth is less than InternalBitDepth")
  ("SummaryOutFilename",                              m_summaryOutFilename,                          string(), "Filename to use for producing summary output file. If empty, do not produce a file.")
  ("SummaryPicFilenameBase",                          m_summaryPicFilenameBase,                      string(), "Base filename to use for producing summary picture output files. The actual filenames used will have I.txt, P.txt and B.txt appended. If empty, do not produce a file.")
  ("SummaryVerboseness",                              m_summaryVerboseness,                                0u, "Specifies the level of the verboseness of the text output")

  //Field coding parameters
  ("FieldCoding",                                     m_isField,                                        false, "Signals if it's a field based coding")
  ("TopFieldFirst, Tff",                              m_isTopFieldFirst,                                false, "In case of field based coding, signals whether if it's a top field first or not")
  ("EfficientFieldIRAPEnabled",                       m_bEfficientFieldIRAPEnabled,                      true, "Enable to code fields in a specific, potentially more efficient, order.")
  ("HarmonizeGopFirstFieldCoupleEnabled",             m_bHarmonizeGopFirstFieldCoupleEnabled,            true, "Enables harmonization of Gop first field couple")

  // Profile and level
#if SVC_EXTENSION
  ("NumProfileTierLevel",                             m_numPTLInfo,                                         1, "Number of Profile, Tier and Level information")
  ("Profile%d",                                       extendedProfile,          NONE, (MAX_NUM_LAYER_IDS + 1),  "Profile name to use for encoding. Use main (for main), main10 (for main10), main-still-picture, main-RExt (for Range Extensions profile), any of the RExt specific profile names, or none")
  ("Level%d",                                         m_levelList,       Level::NONE, (MAX_NUM_LAYER_IDS + 1), "Level limit to be used, eg 5.1, or none")
  ("Tier%d",                                          m_levelTierList,   Level::MAIN, (MAX_NUM_LAYER_IDS + 1), "Tier to use for interpretation of --Level (main or high only)")

  ("MaxBitDepthConstraint",                           tmpBitDepthConstraint,                               0u, "Bit depth to use for profile-constraint for RExt profiles. 0=automatically choose based upon other parameters")
  ("MaxChromaFormatConstraint",                       tmpConstraintChromaFormat,                            0, "Chroma-format to use for the profile-constraint for RExt profiles. 0=automatically choose based upon other parameters")
  ("IntraConstraintFlag",                             tmpIntraConstraintFlag,                           false, "Value of general_intra_constraint_flag to use for RExt profiles (not used if an explicit RExt sub-profile is specified)")
  ("OnePictureOnlyConstraintFlag",                    tmpOnePictureOnlyConstraintFlag,                  false, "Value of general_one_picture_only_constraint_flag to use for RExt profiles (not used if an explicit RExt sub-profile is specified)")
  ("LowerBitRateConstraintFlag",                      tmpLowerBitRateConstraintFlag,                     true, "Value of general_lower_bit_rate_constraint_flag to use for RExt profiles")

  ("ProfileCompatibility%d",                   m_profileCompatibility, Profile::NONE, (MAX_NUM_LAYER_IDS + 1), "Compatible profile to be used when encoding")
  ("ProgressiveSource%d",                       m_progressiveSourceFlagList,   false, (MAX_NUM_LAYER_IDS + 1), "Indicate that source is progressive")
  ("InterlacedSource%d",                        m_interlacedSourceFlagList,    false, (MAX_NUM_LAYER_IDS + 1), "Indicate that source is interlaced")
  ("NonPackedSource%d",                         m_nonPackedConstraintFlagList, false, (MAX_NUM_LAYER_IDS + 1), "Indicate that source does not contain frame packing")
  ("FrameOnly%d",                               m_frameOnlyConstraintFlagList, false, (MAX_NUM_LAYER_IDS + 1), "Indicate that the bitstream contains only frames")
  
  ("LayerPTLIndex%d",                                 cfg_layerPTLIdx,                         0, m_numLayers, "Index of PTL for each layer")
  ("ListOfProfileTierLevelOls%d",       cfg_listOfLayerPTLOfOlss, string(""), MAX_VPS_OUTPUT_LAYER_SETS_PLUS1, "PTL Index for each layer in each OLS except the first OLS. The PTL index for layer in the first OLS is set to 1")

  // Unit definition parameters
  ("MaxCUWidth%d",                                    cfg_uiMaxCUWidth,                      64u, m_numLayers, "Maximum CU width")
  ("MaxCUHeight%d",                                   cfg_uiMaxCUHeight,                     64u, m_numLayers, "Maximum CU height")
  // todo: remove defaults from MaxCUSize
  ("MaxCUSize%d,s",                                   cfg_uiMaxCUWidth,                      64u, m_numLayers, "Maximum CU size")
  ("MaxCUSize%d,s",                                   cfg_uiMaxCUHeight,                     64u, m_numLayers, "Maximum CU size")
  ("MaxPartitionDepth%d,h%d",                         cfg_uiMaxCUDepth,                       4u, m_numLayers, "CU depth")

  ("QuadtreeTULog2MaxSize%d",                         cfg_uiQuadtreeTULog2MaxSize,            5u, m_numLayers, "Maximum TU size in logarithm base 2")
  ("QuadtreeTULog2MinSize%d",                         cfg_uiQuadtreeTULog2MinSize,            2u, m_numLayers, "Minimum TU size in logarithm base 2")
                                                                                               
  ("QuadtreeTUMaxDepthIntra%d",                       cfg_uiQuadtreeTUMaxDepthIntra,          3u, m_numLayers, "Depth of TU tree for intra CUs")
  ("QuadtreeTUMaxDepthInter%d",                       cfg_uiQuadtreeTUMaxDepthInter,          3u, m_numLayers, "Depth of TU tree for inter CUs") 
#else
  ("Profile",                                         extendedProfile,                                   NONE, "Profile name to use for encoding. Use main (for main), main10 (for main10), main-still-picture, main-RExt (for Range Extensions profile), any of the RExt specific profile names, or none")
  ("Level",                                           m_level,                                    Level::NONE, "Level limit to be used, eg 5.1, or none")
  ("Tier",                                            m_levelTier,                                Level::MAIN, "Tier to use for interpretation of --Level (main or high only)")
  ("MaxBitDepthConstraint",                           m_bitDepthConstraint,                                0u, "Bit depth to use for profile-constraint for RExt profiles. 0=automatically choose based upon other parameters")
  ("MaxChromaFormatConstraint",                       tmpConstraintChromaFormat,                            0, "Chroma-format to use for the profile-constraint for RExt profiles. 0=automatically choose based upon other parameters")
  ("IntraConstraintFlag",                             m_intraConstraintFlag,                            false, "Value of general_intra_constraint_flag to use for RExt profiles (not used if an explicit RExt sub-profile is specified)")
  ("OnePictureOnlyConstraintFlag",                    m_onePictureOnlyConstraintFlag,                   false, "Value of general_one_picture_only_constraint_flag to use for RExt profiles (not used if an explicit RExt sub-profile is specified)")
  ("LowerBitRateConstraintFlag",                      m_lowerBitRateConstraintFlag,                      true, "Value of general_lower_bit_rate_constraint_flag to use for RExt profiles")

  ("ProgressiveSource",                               m_progressiveSourceFlag,                          false, "Indicate that source is progressive")
  ("InterlacedSource",                                m_interlacedSourceFlag,                           false, "Indicate that source is interlaced")
  ("NonPackedSource",                                 m_nonPackedConstraintFlag,                        false, "Indicate that source does not contain frame packing")
  ("FrameOnly",                                       m_frameOnlyConstraintFlag,                        false, "Indicate that the bitstream contains only frames")

  // Unit definition parameters
  ("MaxCUWidth",                                      m_uiMaxCUWidth,                                     64u)
  ("MaxCUHeight",                                     m_uiMaxCUHeight,                                    64u)
  // todo: remove defaults from MaxCUSize
  ("MaxCUSize,s",                                     m_uiMaxCUWidth,                                     64u, "Maximum CU size")
  ("MaxCUSize,s",                                     m_uiMaxCUHeight,                                    64u, "Maximum CU size")
  ("MaxPartitionDepth,h",                             m_uiMaxCUDepth,                                      4u, "CU depth")

  ("QuadtreeTULog2MaxSize",                           m_uiQuadtreeTULog2MaxSize,                           6u, "Maximum TU size in logarithm base 2")
  ("QuadtreeTULog2MinSize",                           m_uiQuadtreeTULog2MinSize,                           2u, "Minimum TU size in logarithm base 2")

  ("QuadtreeTUMaxDepthIntra",                         m_uiQuadtreeTUMaxDepthIntra,                         1u, "Depth of TU tree for intra CUs")
  ("QuadtreeTUMaxDepthInter",                         m_uiQuadtreeTUMaxDepthInter,                         2u, "Depth of TU tree for inter CUs")
#endif
  
  // Coding structure paramters
#if SVC_EXTENSION
  ("IntraPeriod%d,-ip%d",                             cfg_IntraPeriod,                        -1, m_numLayers, "intra period in frames for layer %d, (-1: only first frame)")
#else
  ("IntraPeriod,-ip",                                 m_iIntraPeriod,                                      -1, "Intra period in frames, (-1: only first frame)")
#endif
  ("DecodingRefreshType,-dr",                         m_iDecodingRefreshType,                               0, "Intra refresh type (0:none 1:CRA 2:IDR 3:RecPointSEI)")
  ("GOPSize,g",                                       m_iGOPSize,                                           1, "GOP size of temporal structure")

  // motion search options
  ("DisableIntraInInter",                             m_bDisableIntraPUsInInterSlices,                  false, "Flag to disable intra PUs in inter slices")
  ("FastSearch",                                      tmpMotionEstimationSearchMethod,  Int(MESEARCH_DIAMOND), "0:Full search 1:Diamond 2:Selective 3:Enhanced Diamond")
  ("SearchRange,-sr",                                 m_iSearchRange,                                      96, "Motion search range")
  ("BipredSearchRange",                               m_bipredSearchRange,                                  4, "Motion search range for bipred refinement")
  ("MinSearchWindow",                                 m_minSearchWindow,                                    8, "Minimum motion search window size for the adaptive window ME")
  ("RestrictMESampling",                              m_bRestrictMESampling,                            false, "Restrict ME Sampling for selective inter motion search")
  ("ClipForBiPredMEEnabled",                          m_bClipForBiPredMeEnabled,                        false, "Enables clipping in the Bi-Pred ME. It is disabled to reduce encoder run-time")
  ("FastMEAssumingSmootherMVEnabled",                 m_bFastMEAssumingSmootherMVEnabled,                true, "Enables fast ME assuming a smoother MV.")

  ("HadamardME",                                      m_bUseHADME,                                       true, "Hadamard ME for fractional-pel")
  ("ASR",                                             m_bUseASR,                                        false, "Adaptive motion search range");
  opts.addOptions()

#if SVC_EXTENSION
  ("LambdaModifier%d,-LM%d",                          m_adLambdaModifier,           ( double )1.0, MAX_TLAYER, "Lambda modifier for temporal layer %d")
#else
  // Mode decision parameters
  // Mode decision parameters
  ("LambdaModifier0,-LM0",                            m_adLambdaModifier[ 0 ],                  ( Double )1.0, "Lambda modifier for temporal layer 0")
  ("LambdaModifier1,-LM1",                            m_adLambdaModifier[ 1 ],                  ( Double )1.0, "Lambda modifier for temporal layer 1")
  ("LambdaModifier2,-LM2",                            m_adLambdaModifier[ 2 ],                  ( Double )1.0, "Lambda modifier for temporal layer 2")
  ("LambdaModifier3,-LM3",                            m_adLambdaModifier[ 3 ],                  ( Double )1.0, "Lambda modifier for temporal layer 3")
  ("LambdaModifier4,-LM4",                            m_adLambdaModifier[ 4 ],                  ( Double )1.0, "Lambda modifier for temporal layer 4")
  ("LambdaModifier5,-LM5",                            m_adLambdaModifier[ 5 ],                  ( Double )1.0, "Lambda modifier for temporal layer 5")
  ("LambdaModifier6,-LM6",                            m_adLambdaModifier[ 6 ],                  ( Double )1.0, "Lambda modifier for temporal layer 6")
#endif
  ("LambdaModifierI,-LMI",                            cfg_adIntraLambdaModifier,    cfg_adIntraLambdaModifier, "Lambda modifiers for Intra pictures, comma separated, up to one the number of temporal layer. If entry for temporalLayer exists, then use it, else if some are specified, use the last, else use the standard LambdaModifiers.")
  ("IQPFactor,-IQF",                                  m_dIntraQpFactor,                                  -1.0, "Intra QP Factor for Lambda Computation. If negative, use the default equation: 0.57*(1.0 - Clip3( 0.0, 0.5, 0.05*(Double)(isField ? (GopSize-1)/2 : GopSize-1) ))")

  /* Quantization parameters */
#if SVC_EXTENSION
  ("QP%d,-q%d",                                                                   cfg_fQP,  30.0, m_numLayers, "Qp value for layer %d, if value is float, QP is switched once during encoding")
#else
  ("QP,q",                                            m_fQP,                                             30.0, "Qp value, if value is float, QP is switched once during encoding")
#endif
  ("DeltaQpRD,-dqr",                                  m_uiDeltaQpRD,                                       0u, "max dQp offset for slice")
  ("MaxDeltaQP,d",                                    m_iMaxDeltaQP,                                        0, "max dQp offset for block")
#if SVC_EXTENSION
  ("MaxCuDQPDepth%d,-dqd",                            cfg_maxCuDQPDepth,                       0, m_numLayers, "max depth for a minimum CuDQP")
#else
  ("MaxCuDQPDepth,-dqd",                              m_iMaxCuDQPDepth,                                     0, "max depth for a minimum CuDQP")
#endif
  ("MaxCUChromaQpAdjustmentDepth",                    m_diffCuChromaQpOffsetDepth,                         -1, "Maximum depth for CU chroma Qp adjustment - set less than 0 to disable")
  ("FastDeltaQP",                                     m_bFastDeltaQP,                                   false, "Fast Delta QP Algorithm")

  ("CbQpOffset,-cbqpofs",                             m_cbQpOffset,                                         0, "Chroma Cb QP Offset")
  ("CrQpOffset,-crqpofs",                             m_crQpOffset,                                         0, "Chroma Cr QP Offset")
#if W0038_CQP_ADJ
  ("SliceChromaQPOffsetPeriodicity",                  m_sliceChromaQpOffsetPeriodicity,                    0u, "Used in conjunction with Slice Cb/Cr QpOffsetIntraOrPeriodic. Use 0 (default) to disable periodic nature.")
  ("SliceCbQpOffsetIntraOrPeriodic",                  m_sliceChromaQpOffsetIntraOrPeriodic[0],              0, "Chroma Cb QP Offset at slice level for I slice or for periodic inter slices as defined by SliceChromaQPOffsetPeriodicity. Replaces offset in the GOP table.")
  ("SliceCrQpOffsetIntraOrPeriodic",                  m_sliceChromaQpOffsetIntraOrPeriodic[1],              0, "Chroma Cr QP Offset at slice level for I slice or for periodic inter slices as defined by SliceChromaQPOffsetPeriodicity. Replaces offset in the GOP table.")
#endif
#if ADAPTIVE_QP_SELECTION
  ("AdaptiveQpSelection,-aqps",                       m_bUseAdaptQpSelect,                              false, "AdaptiveQpSelection")
#endif

  ("AdaptiveQP,-aq",                                  m_bUseAdaptiveQP,                                 false, "QP adaptation based on a psycho-visual model")
  ("MaxQPAdaptationRange,-aqr",                       m_iQPAdaptationRange,                                 6, "QP adaptation range")
#if SVC_EXTENSION
  ("dQPFile,m",                                       cfg_dQPFileName,                string(""), m_numLayers, "dQP file name")
#else
  ("dQPFile,m",                                       m_dQPFileName,                               string(""), "dQP file name")
#endif
  ("RDOQ",                                            m_useRDOQ,                                         true)
  ("RDOQTS",                                          m_useRDOQTS,                                       true)
#if T0196_SELECTIVE_RDOQ
  ("SelectiveRDOQ",                                   m_useSelectiveRDOQ,                               false, "Enable selective RDOQ")
#endif
  ("RDpenalty",                                       m_rdPenalty,                                          0,  "RD-penalty for 32x32 TU for intra in non-intra slices. 0:disabled  1:RD-penalty  2:maximum RD-penalty")

  // Deblocking filter parameters
  ("LoopFilterDisable",                               m_bLoopFilterDisable,                             false)
  ("LoopFilterOffsetInPPS",                           m_loopFilterOffsetInPPS,                           true)
  ("LoopFilterBetaOffset_div2",                       m_loopFilterBetaOffsetDiv2,                           0)
  ("LoopFilterTcOffset_div2",                         m_loopFilterTcOffsetDiv2,                             0)
#if W0038_DB_OPT
  ("DeblockingFilterMetric",                          m_deblockingFilterMetric,                             0)
#else
  ("DeblockingFilterMetric",                          m_DeblockingFilterMetric,                         false)
#endif
  // Coding tools
  ("AMP",                                             m_enableAMP,                                       true, "Enable asymmetric motion partitions")
  ("CrossComponentPrediction",                        m_crossComponentPredictionEnabledFlag,            false, "Enable the use of cross-component prediction (not valid in V1 profiles)")
  ("ReconBasedCrossCPredictionEstimate",              m_reconBasedCrossCPredictionEstimate,             false, "When determining the alpha value for cross-component prediction, use the decoded residual rather than the pre-transform encoder-side residual")
  ("SaoLumaOffsetBitShift",                           saoOffsetBitShift[CHANNEL_TYPE_LUMA],                 0, "Specify the luma SAO bit-shift. If negative, automatically calculate a suitable value based upon bit depth and initial QP")
  ("SaoChromaOffsetBitShift",                         saoOffsetBitShift[CHANNEL_TYPE_CHROMA],               0, "Specify the chroma SAO bit-shift. If negative, automatically calculate a suitable value based upon bit depth and initial QP")
  ("TransformSkip",                                   m_useTransformSkip,                               false, "Intra transform skipping")
  ("TransformSkipFast",                               m_useTransformSkipFast,                           false, "Fast intra transform skipping")
  ("TransformSkipLog2MaxSize",                        m_log2MaxTransformSkipBlockSize,                     2U, "Specify transform-skip maximum size. Minimum 2. (not valid in V1 profiles)")
  ("ImplicitResidualDPCM",                            m_rdpcmEnabledFlag[RDPCM_SIGNAL_IMPLICIT],        false, "Enable implicitly signalled residual DPCM for intra (also known as sample-adaptive intra predict) (not valid in V1 profiles)")
  ("ExplicitResidualDPCM",                            m_rdpcmEnabledFlag[RDPCM_SIGNAL_EXPLICIT],        false, "Enable explicitly signalled residual DPCM for inter (not valid in V1 profiles)")
  ("ResidualRotation",                                m_transformSkipRotationEnabledFlag,               false, "Enable rotation of transform-skipped and transquant-bypassed TUs through 180 degrees prior to entropy coding (not valid in V1 profiles)")
  ("SingleSignificanceMapContext",                    m_transformSkipContextEnabledFlag,                false, "Enable, for transform-skipped and transquant-bypassed TUs, the selection of a single significance map context variable for all coefficients (not valid in V1 profiles)")
  ("GolombRiceParameterAdaptation",                   m_persistentRiceAdaptationEnabledFlag,            false, "Enable the adaptation of the Golomb-Rice parameter over the course of each slice")
  ("AlignCABACBeforeBypass",                          m_cabacBypassAlignmentEnabledFlag,                false, "Align the CABAC engine to a defined fraction of a bit prior to coding bypass data. Must be 1 in high bit rate profile, 0 otherwise" )
#if SVC_EXTENSION
  ("SAO%d",                                           cfg_bUseSAO,                          true, m_numLayers, "Enable Sample Adaptive Offset")
#else
  ("SAO",                                             m_bUseSAO,                                         true, "Enable Sample Adaptive Offset")
#endif
  ("TestSAODisableAtPictureLevel",                    m_bTestSAODisableAtPictureLevel,                  false, "Enables the testing of disabling SAO at the picture level after having analysed all blocks")
  ("SaoEncodingRate",                                 m_saoEncodingRate,                                 0.75, "When >0 SAO early picture termination is enabled for luma and chroma")
  ("SaoEncodingRateChroma",                           m_saoEncodingRateChroma,                            0.5, "The SAO early picture termination rate to use for chroma (when m_SaoEncodingRate is >0). If <=0, use results for luma")
  ("MaxNumOffsetsPerPic",                             m_maxNumOffsetsPerPic,                             2048, "Max number of SAO offset per picture (Default: 2048)")
  ("SAOLcuBoundary",                                  m_saoCtuBoundary,                                 false, "0: right/bottom CTU boundary areas skipped from SAO parameter estimation, 1: non-deblocked pixels are used for those areas")
#if OPTIONAL_RESET_SAO_ENCODING_AFTER_IRAP
  ("SAOResetEncoderStateAfterIRAP",                   m_saoResetEncoderStateAfterIRAP,                  false, "When true, resets the encoder's SAO state after an IRAP (POC order). Disabled by default.")
#endif
  ("SliceMode",                                       tmpSliceMode,                            Int(NO_SLICES), "0: Disable all Recon slice limits, 1: Enforce max # of CTUs, 2: Enforce max # of bytes, 3:specify tiles per dependent slice")
  ("SliceArgument",                                   m_sliceArgument,                                      0, "Depending on SliceMode being:"
                                                                                                               "\t1: max number of CTUs per slice"
                                                                                                               "\t2: max number of bytes per slice"
                                                                                                               "\t3: max number of tiles per slice")
  ("SliceSegmentMode",                                tmpSliceSegmentMode,                     Int(NO_SLICES), "0: Disable all slice segment limits, 1: Enforce max # of CTUs, 2: Enforce max # of bytes, 3:specify tiles per dependent slice")
  ("SliceSegmentArgument",                            m_sliceSegmentArgument,                               0, "Depending on SliceSegmentMode being:"
                                                                                                               "\t1: max number of CTUs per slice segment"
                                                                                                               "\t2: max number of bytes per slice segment"
                                                                                                               "\t3: max number of tiles per slice segment")
  ("LFCrossSliceBoundaryFlag",                        m_bLFCrossSliceBoundaryFlag,                       true)

  ("ConstrainedIntraPred",                            m_bUseConstrainedIntraPred,                       false, "Constrained Intra Prediction")
  ("FastUDIUseMPMEnabled",                            m_bFastUDIUseMPMEnabled,                           true, "If enabled, adapt intra direction search, accounting for MPM")
  ("FastMEForGenBLowDelayEnabled",                    m_bFastMEForGenBLowDelayEnabled,                   true, "If enabled use a fast ME for generalised B Low Delay slices")
  ("UseBLambdaForNonKeyLowDelayPictures",             m_bUseBLambdaForNonKeyLowDelayPictures,            true, "Enables use of B-Lambda for non-key low-delay pictures")
  ("PCMEnabledFlag",                                  m_usePCM,                                         false)
  ("PCMLog2MaxSize",                                  m_pcmLog2MaxSize,                                    5u)
  ("PCMLog2MinSize",                                  m_uiPCMLog2MinSize,                                  3u)

  ("PCMInputBitDepthFlag",                            m_bPCMInputBitDepthFlag,                           true)
  ("PCMFilterDisableFlag",                            m_bPCMFilterDisableFlag,                          false)
  ("IntraReferenceSmoothing",                         m_enableIntraReferenceSmoothing,                   true, "0: Disable use of intra reference smoothing (not valid in V1 profiles). 1: Enable use of intra reference smoothing (same as V1)")
  ("WeightedPredP,-wpP",                              m_useWeightedPred,                                false, "Use weighted prediction in P slices")
  ("WeightedPredB,-wpB",                              m_useWeightedBiPred,                              false, "Use weighted (bidirectional) prediction in B slices")
  ("WeightedPredMethod,-wpM",                         tmpWeightedPredictionMethod, Int(WP_PER_PICTURE_WITH_SIMPLE_DC_COMBINED_COMPONENT), "Weighted prediction method")
  ("Log2ParallelMergeLevel",                          m_log2ParallelMergeLevel,                            2u, "Parallel merge estimation region")
    //deprecated copies of renamed tile parameters
  ("UniformSpacingIdc",                               m_tileUniformSpacingFlag,                         false,      "deprecated alias of TileUniformSpacing")
  ("ColumnWidthArray",                                cfg_ColumnWidth,                        cfg_ColumnWidth, "deprecated alias of TileColumnWidthArray")
  ("RowHeightArray",                                  cfg_RowHeight,                            cfg_RowHeight, "deprecated alias of TileRowHeightArray")

  ("TileUniformSpacing",                              m_tileUniformSpacingFlag,                         false,      "Indicates that tile columns and rows are distributed uniformly")
  ("NumTileColumnsMinus1",                            m_numTileColumnsMinus1,                               0,          "Number of tile columns in a picture minus 1")
  ("NumTileRowsMinus1",                               m_numTileRowsMinus1,                                  0,          "Number of rows in a picture minus 1")
  ("TileColumnWidthArray",                            cfg_ColumnWidth,                        cfg_ColumnWidth, "Array containing tile column width values in units of CTU")
  ("TileRowHeightArray",                              cfg_RowHeight,                            cfg_RowHeight, "Array containing tile row height values in units of CTU")
  ("LFCrossTileBoundaryFlag",                         m_bLFCrossTileBoundaryFlag,                        true, "1: cross-tile-boundary loop filtering. 0:non-cross-tile-boundary loop filtering")
#if SVC_EXTENSION
#if FAST_INTRA_SHVC
  ("FIS",                                             m_useFastIntraScalable,                           false, "Fast Intra Decision for Scalable HEVC")
#endif
  ("WaveFrontSynchro%d",                              cfg_entropyCodingSyncEnabledFlag,   false,  m_numLayers, "0: entropy coding sync disabled; 1 entropy coding sync enabled")
  ("ScalingList%d",                                   cfg_UseScalingListId,     SCALING_LIST_OFF, m_numLayers, "0/off: no scaling list, 1/default: default scaling lists, 2/file: scaling lists specified in ScalingListFile")
  ("ScalingListFile%d",                               cfg_scalingListFileName,        string(""), m_numLayers, "Scaling list file name. Use an empty string to produce help.")
#else
  ("WaveFrontSynchro",                                m_entropyCodingSyncEnabledFlag,                   false, "0: entropy coding sync disabled; 1 entropy coding sync enabled")
  ("ScalingList",                                     m_useScalingListId,                    SCALING_LIST_OFF, "0/off: no scaling list, 1/default: default scaling lists, 2/file: scaling lists specified in ScalingListFile")
  ("ScalingListFile",                                 m_scalingListFileName,                       string(""), "Scaling list file name. Use an empty string to produce help.")
#endif
  ("SignHideFlag,-SBH",                               m_signDataHidingEnabledFlag,                                    true)
  ("MaxNumMergeCand",                                 m_maxNumMergeCand,                                   5u, "Maximum number of merge candidates")
  /* Misc. */
  ("SEIDecodedPictureHash",                           tmpDecodedPictureHashSEIMappedType,                   0, "Control generation of decode picture hash SEI messages\n"
                                                                                                               "\t3: checksum\n"
                                                                                                               "\t2: CRC\n"
                                                                                                               "\t1: use MD5\n"
                                                                                                               "\t0: disable")
  ("TMVPMode",                                        m_TMVPModeId,                                         1, "TMVP mode 0: TMVP disable for all slices. 1: TMVP enable for all slices (default) 2: TMVP enable for certain slices only")
  ("FEN",                                             tmpFastInterSearchMode,   Int(FASTINTERSEARCH_DISABLED), "fast encoder setting")
  ("ECU",                                             m_bUseEarlyCU,                                    false, "Early CU setting")
  ("FDM",                                             m_useFastDecisionForMerge,                         true, "Fast decision for Merge RD Cost")
  ("CFM",                                             m_bUseCbfFastMode,                                false, "Cbf fast mode setting")
  ("ESD",                                             m_useEarlySkipDetection,                          false, "Early SKIP detection setting")
#if RC_SHVC_HARMONIZATION
  ("RateControl%d",                                   cfg_RCEnableRateControl,             false, m_numLayers, "Rate control: enable rate control for layer %d")
  ("TargetBitrate%d",                                 cfg_RCTargetBitRate,                     0, m_numLayers, "Rate control: target bitrate for layer %d")
  ("KeepHierarchicalBit%d",                           cfg_RCKeepHierarchicalBit,           false, m_numLayers, "Rate control: keep hierarchical bit allocation for layer %d")
  ("LCULevelRateControl%d",                           cfg_RCLCULevelRC,                     true, m_numLayers, "Rate control: LCU level RC")
  ("RCLCUSeparateModel%d",                            cfg_RCUseLCUSeparateModel,            true, m_numLayers, "Rate control: Use LCU level separate R-lambda model")
  ("InitialQP%d",                                     cfg_RCInitialQP,                         0, m_numLayers, "Rate control: initial QP")
  ("RCForceIntraQP%d",                                cfg_RCForceIntraQP,                  false, m_numLayers, "Rate control: force intra QP to be equal to initial QP")
#if U0132_TARGET_BITS_SATURATION
  ( "RCCpbSaturation%d",                              cfg_RCCpbSaturationEnabled,          false, m_numLayers, "Rate control: enable target bits saturation to avoid CPB overflow and underflow" )
  ( "RCCpbSize%d",                                    cfg_RCCpbSize,                          0u, m_numLayers, "Rate control: CPB size" )
  ( "RCInitialCpbFullness%d",                         cfg_RCInitialCpbFullness,              0.9, m_numLayers, "Rate control: initial CPB fullness" )
#endif
#else
  ( "RateControl",                                    m_RCEnableRateControl,                            false, "Rate control: enable rate control" )
  ( "TargetBitrate",                                  m_RCTargetBitrate,                                    0, "Rate control: target bit-rate" )
  ( "KeepHierarchicalBit",                            m_RCKeepHierarchicalBit,                              0, "Rate control: 0: equal bit allocation; 1: fixed ratio bit allocation; 2: adaptive ratio bit allocation" )
  ( "LCULevelRateControl",                            m_RCLCULevelRC,                                    true, "Rate control: true: CTU level RC; false: picture level RC" )
  ( "RCLCUSeparateModel",                             m_RCUseLCUSeparateModel,                           true, "Rate control: use CTU level separate R-lambda model" )
  ( "InitialQP",                                      m_RCInitialQP,                                        0, "Rate control: initial QP" )
  ( "RCForceIntraQP",                                 m_RCForceIntraQP,                                 false, "Rate control: force intra QP to be equal to initial QP" )
#if U0132_TARGET_BITS_SATURATION
  ( "RCCpbSaturation",                                m_RCCpbSaturationEnabled,                         false, "Rate control: enable target bits saturation to avoid CPB overflow and underflow" )
  ( "RCCpbSize",                                      m_RCCpbSize,                                         0u, "Rate control: CPB size" )
  ( "RCInitialCpbFullness",                           m_RCInitialCpbFullness,                             0.9, "Rate control: initial CPB fullness" )
#endif
#endif

#if PER_LAYER_LOSSLESS
  ("TransquantBypassEnable%d",                        cfg_TransquantBypassEnabledFlag,     false, m_numLayers, "transquant_bypass_enabled_flag indicator in PPS")
  ("TransquantBypassEnableFlag%d",                    cfg_TransquantBypassEnabledFlag,     false, m_numLayers, "deprecated alias for TransquantBypassEnable")
  ("CUTransquantBypassFlagForce%d",                   cfg_CUTransquantBypassFlagForce,     false, m_numLayers, "Force transquant bypass mode, when transquant_bypass_enabled_flag is enabled")
  ("CostMode%d",                                      cfg_costMode,          COST_STANDARD_LOSSY, m_numLayers, "Use alternative cost functions: choose between 'lossy', 'sequence_level_lossless', 'lossless' (which forces QP to " MACRO_TO_STRING(LOSSLESS_AND_MIXED_LOSSLESS_RD_COST_TEST_QP) ") and 'mixed_lossless_lossy' (which used QP'=" MACRO_TO_STRING(LOSSLESS_AND_MIXED_LOSSLESS_RD_COST_TEST_QP_PRIME) " for pre-estimates of transquant-bypass blocks).")
#else
  ("TransquantBypassEnable",                          m_TransquantBypassEnabledFlag,                    false, "transquant_bypass_enabled_flag indicator in PPS")
  ("TransquantBypassEnableFlag",                      m_TransquantBypassEnabledFlag,                    false, "deprecated alias for TransquantBypassEnable")
  ("CUTransquantBypassFlagForce",                     m_CUTransquantBypassFlagForce,                    false, "Force transquant bypass mode, when transquant_bypass_enabled_flag is enabled")
  ("CostMode",                                        m_costMode,                         COST_STANDARD_LOSSY, "Use alternative cost functions: choose between 'lossy', 'sequence_level_lossless', 'lossless' (which forces QP to " MACRO_TO_STRING(LOSSLESS_AND_MIXED_LOSSLESS_RD_COST_TEST_QP) ") and 'mixed_lossless_lossy' (which used QP'=" MACRO_TO_STRING(LOSSLESS_AND_MIXED_LOSSLESS_RD_COST_TEST_QP_PRIME) " for pre-estimates of transquant-bypass blocks).")
#endif
  ("RecalculateQPAccordingToLambda",                  m_recalculateQPAccordingToLambda,                 false, "Recalculate QP values according to lambda values. Do not suggest to be enabled in all intra case")
  ("StrongIntraSmoothing,-sis",                       m_useStrongIntraSmoothing,                         true, "Enable strong intra smoothing for 32x32 blocks")
  ("SEIActiveParameterSets",                          m_activeParameterSetsSEIEnabled,                      0, "Enable generation of active parameter sets SEI messages");
  opts.addOptions()
  ("VuiParametersPresent,-vui",                       m_vuiParametersPresentFlag,                       false, "Enable generation of vui_parameters()")
  ("AspectRatioInfoPresent",                          m_aspectRatioInfoPresentFlag,                     false, "Signals whether aspect_ratio_idc is present")
  ("AspectRatioIdc",                                  m_aspectRatioIdc,                                     0, "aspect_ratio_idc")
  ("SarWidth",                                        m_sarWidth,                                           0, "horizontal size of the sample aspect ratio")
  ("SarHeight",                                       m_sarHeight,                                          0, "vertical size of the sample aspect ratio")
  ("OverscanInfoPresent",                             m_overscanInfoPresentFlag,                        false, "Indicates whether conformant decoded pictures are suitable for display using overscan\n")
  ("OverscanAppropriate",                             m_overscanAppropriateFlag,                        false, "Indicates whether conformant decoded pictures are suitable for display using overscan\n")
  ("VideoSignalTypePresent",                          m_videoSignalTypePresentFlag,                     false, "Signals whether video_format, video_full_range_flag, and colour_description_present_flag are present")
  ("VideoFormat",                                     m_videoFormat,                                        5, "Indicates representation of pictures")
  ("VideoFullRange",                                  m_videoFullRangeFlag,                             false, "Indicates the black level and range of luma and chroma signals")
  ("ColourDescriptionPresent",                        m_colourDescriptionPresentFlag,                   false, "Signals whether colour_primaries, transfer_characteristics and matrix_coefficients are present")
  ("ColourPrimaries",                                 m_colourPrimaries,                                    2, "Indicates chromaticity coordinates of the source primaries")
  ("TransferCharacteristics",                         m_transferCharacteristics,                            2, "Indicates the opto-electronic transfer characteristics of the source")
  ("MatrixCoefficients",                              m_matrixCoefficients,                                 2, "Describes the matrix coefficients used in deriving luma and chroma from RGB primaries")
  ("ChromaLocInfoPresent",                            m_chromaLocInfoPresentFlag,                       false, "Signals whether chroma_sample_loc_type_top_field and chroma_sample_loc_type_bottom_field are present")
  ("ChromaSampleLocTypeTopField",                     m_chromaSampleLocTypeTopField,                        0, "Specifies the location of chroma samples for top field")
  ("ChromaSampleLocTypeBottomField",                  m_chromaSampleLocTypeBottomField,                     0, "Specifies the location of chroma samples for bottom field")
  ("NeutralChromaIndication",                         m_neutralChromaIndicationFlag,                    false, "Indicates that the value of all decoded chroma samples is equal to 1<<(BitDepthCr-1)")
  ("DefaultDisplayWindowFlag",                        m_defaultDisplayWindowFlag,                       false, "Indicates the presence of the Default Window parameters")
  ("DefDispWinLeftOffset",                            m_defDispWinLeftOffset,                               0, "Specifies the left offset of the default display window from the conformance window")
  ("DefDispWinRightOffset",                           m_defDispWinRightOffset,                              0, "Specifies the right offset of the default display window from the conformance window")
  ("DefDispWinTopOffset",                             m_defDispWinTopOffset,                                0, "Specifies the top offset of the default display window from the conformance window")
  ("DefDispWinBottomOffset",                          m_defDispWinBottomOffset,                             0, "Specifies the bottom offset of the default display window from the conformance window")
  ("FrameFieldInfoPresentFlag",                       m_frameFieldInfoPresentFlag,                      false, "Indicates that pic_struct and field coding related values are present in picture timing SEI messages")
  ("PocProportionalToTimingFlag",                     m_pocProportionalToTimingFlag,                    false, "Indicates that the POC value is proportional to the output time w.r.t. first picture in CVS")
  ("NumTicksPocDiffOneMinus1",                        m_numTicksPocDiffOneMinus1,                           0, "Number of ticks minus 1 that for a POC difference of one")
  ("BitstreamRestriction",                            m_bitstreamRestrictionFlag,                       false, "Signals whether bitstream restriction parameters are present")
  ("TilesFixedStructure",                             m_tilesFixedStructureFlag,                        false, "Indicates that each active picture parameter set has the same values of the syntax elements related to tiles")
  ("MotionVectorsOverPicBoundaries",                  m_motionVectorsOverPicBoundariesFlag,             false, "Indicates that no samples outside the picture boundaries are used for inter prediction")
  ("MaxBytesPerPicDenom",                             m_maxBytesPerPicDenom,                                2, "Indicates a number of bytes not exceeded by the sum of the sizes of the VCL NAL units associated with any coded picture")
  ("MaxBitsPerMinCuDenom",                            m_maxBitsPerMinCuDenom,                               1, "Indicates an upper bound for the number of bits of coding_unit() data")
  ("Log2MaxMvLengthHorizontal",                       m_log2MaxMvLengthHorizontal,                         15, "Indicate the maximum absolute value of a decoded horizontal MV component in quarter-pel luma units")
  ("Log2MaxMvLengthVertical",                         m_log2MaxMvLengthVertical,                           15, "Indicate the maximum absolute value of a decoded vertical MV component in quarter-pel luma units");
  opts.addOptions()
#if !SVC_EXTENSION
  ("SEIColourRemappingInfoFileRoot,-cri",             m_colourRemapSEIFileRoot,                    string(""), "Colour Remapping Information SEI parameters root file name (wo num ext)")
#endif
  ("SEIRecoveryPoint",                                m_recoveryPointSEIEnabled,                        false, "Control generation of recovery point SEI messages")
  ("SEIBufferingPeriod",                              m_bufferingPeriodSEIEnabled,                      false, "Control generation of buffering period SEI messages")
  ("SEIPictureTiming",                                m_pictureTimingSEIEnabled,                        false, "Control generation of picture timing SEI messages")
  ("SEIToneMappingInfo",                              m_toneMappingInfoSEIEnabled,                      false, "Control generation of Tone Mapping SEI messages")
  ("SEIToneMapId",                                    m_toneMapId,                                          0, "Specifies Id of Tone Mapping SEI message for a given session")
  ("SEIToneMapCancelFlag",                            m_toneMapCancelFlag,                              false, "Indicates that Tone Mapping SEI message cancels the persistence or follows")
  ("SEIToneMapPersistenceFlag",                       m_toneMapPersistenceFlag,                          true, "Specifies the persistence of the Tone Mapping SEI message")
  ("SEIToneMapCodedDataBitDepth",                     m_toneMapCodedDataBitDepth,                           8, "Specifies Coded Data BitDepth of Tone Mapping SEI messages")
  ("SEIToneMapTargetBitDepth",                        m_toneMapTargetBitDepth,                              8, "Specifies Output BitDepth of Tone mapping function")
  ("SEIToneMapModelId",                               m_toneMapModelId,                                     0, "Specifies Model utilized for mapping coded data into target_bit_depth range\n"
                                                                                                               "\t0:  linear mapping with clipping\n"
                                                                                                               "\t1:  sigmoidal mapping\n"
                                                                                                               "\t2:  user-defined table mapping\n"
                                                                                                               "\t3:  piece-wise linear mapping\n"
                                                                                                               "\t4:  luminance dynamic range information ")
  ("SEIToneMapMinValue",                              m_toneMapMinValue,                                    0, "Specifies the minimum value in mode 0")
  ("SEIToneMapMaxValue",                              m_toneMapMaxValue,                                 1023, "Specifies the maximum value in mode 0")
  ("SEIToneMapSigmoidMidpoint",                       m_sigmoidMidpoint,                                  512, "Specifies the centre point in mode 1")
  ("SEIToneMapSigmoidWidth",                          m_sigmoidWidth,                                     960, "Specifies the distance between 5% and 95% values of the target_bit_depth in mode 1")
  ("SEIToneMapStartOfCodedInterval",                  cfg_startOfCodedInterval,      cfg_startOfCodedInterval, "Array of user-defined mapping table")
  ("SEIToneMapNumPivots",                             m_numPivots,                                          0, "Specifies the number of pivot points in mode 3")
  ("SEIToneMapCodedPivotValue",                       cfg_codedPivotValue,                cfg_codedPivotValue, "Array of pivot point")
  ("SEIToneMapTargetPivotValue",                      cfg_targetPivotValue,              cfg_targetPivotValue, "Array of pivot point")
  ("SEIToneMapCameraIsoSpeedIdc",                     m_cameraIsoSpeedIdc,                                  0, "Indicates the camera ISO speed for daylight illumination")
  ("SEIToneMapCameraIsoSpeedValue",                   m_cameraIsoSpeedValue,                              400, "Specifies the camera ISO speed for daylight illumination of Extended_ISO")
  ("SEIToneMapExposureIndexIdc",                      m_exposureIndexIdc,                                   0, "Indicates the exposure index setting of the camera")
  ("SEIToneMapExposureIndexValue",                    m_exposureIndexValue,                               400, "Specifies the exposure index setting of the camera of Extended_ISO")
  ("SEIToneMapExposureCompensationValueSignFlag",     m_exposureCompensationValueSignFlag,               false, "Specifies the sign of ExposureCompensationValue")
  ("SEIToneMapExposureCompensationValueNumerator",    m_exposureCompensationValueNumerator,                 0, "Specifies the numerator of ExposureCompensationValue")
  ("SEIToneMapExposureCompensationValueDenomIdc",     m_exposureCompensationValueDenomIdc,                  2, "Specifies the denominator of ExposureCompensationValue")
  ("SEIToneMapRefScreenLuminanceWhite",               m_refScreenLuminanceWhite,                          350, "Specifies reference screen brightness setting in units of candela per square metre")
  ("SEIToneMapExtendedRangeWhiteLevel",               m_extendedRangeWhiteLevel,                          800, "Indicates the luminance dynamic range")
  ("SEIToneMapNominalBlackLevelLumaCodeValue",        m_nominalBlackLevelLumaCodeValue,                    16, "Specifies luma sample value of the nominal black level assigned decoded pictures")
  ("SEIToneMapNominalWhiteLevelLumaCodeValue",        m_nominalWhiteLevelLumaCodeValue,                   235, "Specifies luma sample value of the nominal white level assigned decoded pictures")
  ("SEIToneMapExtendedWhiteLevelLumaCodeValue",       m_extendedWhiteLevelLumaCodeValue,                  300, "Specifies luma sample value of the extended dynamic range assigned decoded pictures")
  ("SEIChromaResamplingFilterHint",                   m_chromaResamplingFilterSEIenabled,               false, "Control generation of the chroma sampling filter hint SEI message")
  ("SEIChromaResamplingHorizontalFilterType",         m_chromaResamplingHorFilterIdc,                       2, "Defines the Index of the chroma sampling horizontal filter\n"
                                                                                                               "\t0: unspecified  - Chroma filter is unknown or is determined by the application"
                                                                                                               "\t1: User-defined - Filter coefficients are specified in the chroma sampling filter hint SEI message"
                                                                                                               "\t2: Standards-defined - ITU-T Rec. T.800 | ISO/IEC15444-1, 5/3 filter")
  ("SEIChromaResamplingVerticalFilterType",           m_chromaResamplingVerFilterIdc,                         2, "Defines the Index of the chroma sampling vertical filter\n"
                                                                                                               "\t0: unspecified  - Chroma filter is unknown or is determined by the application"
                                                                                                               "\t1: User-defined - Filter coefficients are specified in the chroma sampling filter hint SEI message"
                                                                                                               "\t2: Standards-defined - ITU-T Rec. T.800 | ISO/IEC15444-1, 5/3 filter")
  ("SEIFramePacking",                                 m_framePackingSEIEnabled,                         false, "Control generation of frame packing SEI messages")
  ("SEIFramePackingType",                             m_framePackingSEIType,                                0, "Define frame packing arrangement\n"
                                                                                                               "\t3: side by side - frames are displayed horizontally\n"
                                                                                                               "\t4: top bottom - frames are displayed vertically\n"
                                                                                                               "\t5: frame alternation - one frame is alternated with the other")
  ("SEIFramePackingId",                               m_framePackingSEIId,                                  0, "Id of frame packing SEI message for a given session")
  ("SEIFramePackingQuincunx",                         m_framePackingSEIQuincunx,                            0, "Indicate the presence of a Quincunx type video frame")
  ("SEIFramePackingInterpretation",                   m_framePackingSEIInterpretation,                      0, "Indicate the interpretation of the frame pair\n"
                                                                                                               "\t0: unspecified\n"
                                                                                                               "\t1: stereo pair, frame0 represents left view\n"
                                                                                                               "\t2: stereo pair, frame0 represents right view")
  ("SEISegmentedRectFramePacking",                    m_segmentedRectFramePackingSEIEnabled,            false, "Controls generation of segmented rectangular frame packing SEI messages")
  ("SEISegmentedRectFramePackingCancel",              m_segmentedRectFramePackingSEICancel,             false, "If equal to 1, cancels the persistence of any previous SRFPA SEI message")
  ("SEISegmentedRectFramePackingType",                m_segmentedRectFramePackingSEIType,                   0, "Specifies the arrangement of the frames in the reconstructed picture")
  ("SEISegmentedRectFramePackingPersistence",         m_segmentedRectFramePackingSEIPersistence,        false, "If equal to 0, the SEI applies to the current frame only")
  ("SEIDisplayOrientation",                           m_displayOrientationSEIAngle,                         0, "Control generation of display orientation SEI messages\n"
                                                                                                               "\tN: 0 < N < (2^16 - 1) enable display orientation SEI message with anticlockwise_rotation = N and display_orientation_repetition_period = 1\n"
                                                                                                               "\t0: disable")
  ("SEITemporalLevel0Index",                          m_temporalLevel0IndexSEIEnabled,                  false, "Control generation of temporal level 0 index SEI messages")
  ("SEIGradualDecodingRefreshInfo",                   m_gradualDecodingRefreshInfoEnabled,              false, "Control generation of gradual decoding refresh information SEI message")
  ("SEINoDisplay",                                    m_noDisplaySEITLayer,                                 0, "Control generation of no display SEI message\n"
                                                                                                               "\tN: 0 < N enable no display SEI message for temporal layer N or higher\n"
                                                                                                               "\t0: disable")
  ("SEIDecodingUnitInfo",                             m_decodingUnitInfoSEIEnabled,                     false, "Control generation of decoding unit information SEI message.")
  ("SEISOPDescription",                               m_SOPDescriptionSEIEnabled,                       false, "Control generation of SOP description SEI messages")
  ("SEIScalableNesting",                              m_scalableNestingSEIEnabled,                      false, "Control generation of scalable nesting SEI messages")
  ("SEITempMotionConstrainedTileSets",                m_tmctsSEIEnabled,                                false, "Control generation of temporal motion constrained tile sets SEI message")
  ("SEITimeCodeEnabled",                              m_timeCodeSEIEnabled,                             false, "Control generation of time code information SEI message")
  ("SEITimeCodeNumClockTs",                           m_timeCodeSEINumTs,                                   0, "Number of clock time sets [0..3]")
  ("SEITimeCodeTimeStampFlag",                        cfg_timeCodeSeiTimeStampFlag,          cfg_timeCodeSeiTimeStampFlag,         "Time stamp flag associated to each time set")
  ("SEITimeCodeFieldBasedFlag",                       cfg_timeCodeSeiNumUnitFieldBasedFlag,  cfg_timeCodeSeiNumUnitFieldBasedFlag, "Field based flag associated to each time set")
  ("SEITimeCodeCountingType",                         cfg_timeCodeSeiCountingType,           cfg_timeCodeSeiCountingType,          "Counting type associated to each time set")
  ("SEITimeCodeFullTsFlag",                           cfg_timeCodeSeiFullTimeStampFlag,      cfg_timeCodeSeiFullTimeStampFlag,     "Full time stamp flag associated to each time set")
  ("SEITimeCodeDiscontinuityFlag",                    cfg_timeCodeSeiDiscontinuityFlag,      cfg_timeCodeSeiDiscontinuityFlag,     "Discontinuity flag associated to each time set")
  ("SEITimeCodeCntDroppedFlag",                       cfg_timeCodeSeiCntDroppedFlag,         cfg_timeCodeSeiCntDroppedFlag,        "Counter dropped flag associated to each time set")
  ("SEITimeCodeNumFrames",                            cfg_timeCodeSeiNumberOfFrames,         cfg_timeCodeSeiNumberOfFrames,        "Number of frames associated to each time set")
  ("SEITimeCodeSecondsValue",                         cfg_timeCodeSeiSecondsValue,           cfg_timeCodeSeiSecondsValue,          "Seconds value for each time set")
  ("SEITimeCodeMinutesValue",                         cfg_timeCodeSeiMinutesValue,           cfg_timeCodeSeiMinutesValue,          "Minutes value for each time set")
  ("SEITimeCodeHoursValue",                           cfg_timeCodeSeiHoursValue,             cfg_timeCodeSeiHoursValue,            "Hours value for each time set")
  ("SEITimeCodeSecondsFlag",                          cfg_timeCodeSeiSecondsFlag,            cfg_timeCodeSeiSecondsFlag,           "Flag to signal seconds value presence in each time set")
  ("SEITimeCodeMinutesFlag",                          cfg_timeCodeSeiMinutesFlag,            cfg_timeCodeSeiMinutesFlag,           "Flag to signal minutes value presence in each time set")
  ("SEITimeCodeHoursFlag",                            cfg_timeCodeSeiHoursFlag,              cfg_timeCodeSeiHoursFlag,             "Flag to signal hours value presence in each time set")
  ("SEITimeCodeOffsetLength",                         cfg_timeCodeSeiTimeOffsetLength,       cfg_timeCodeSeiTimeOffsetLength,      "Time offset length associated to each time set")
  ("SEITimeCodeTimeOffset",                           cfg_timeCodeSeiTimeOffsetValue,        cfg_timeCodeSeiTimeOffsetValue,       "Time offset associated to each time set")
  ("SEIKneeFunctionInfo",                             m_kneeSEIEnabled,                                 false, "Control generation of Knee function SEI messages")
  ("SEIKneeFunctionId",                               m_kneeSEIId,                                          0, "Specifies Id of Knee function SEI message for a given session")
  ("SEIKneeFunctionCancelFlag",                       m_kneeSEICancelFlag,                              false, "Indicates that Knee function SEI message cancels the persistence or follows")
  ("SEIKneeFunctionPersistenceFlag",                  m_kneeSEIPersistenceFlag,                          true, "Specifies the persistence of the Knee function SEI message")
  ("SEIKneeFunctionInputDrange",                      m_kneeSEIInputDrange,                              1000, "Specifies the peak luminance level for the input picture of Knee function SEI messages")
  ("SEIKneeFunctionInputDispLuminance",               m_kneeSEIInputDispLuminance,                        100, "Specifies the expected display brightness for the input picture of Knee function SEI messages")
  ("SEIKneeFunctionOutputDrange",                     m_kneeSEIOutputDrange,                             4000, "Specifies the peak luminance level for the output picture of Knee function SEI messages")
  ("SEIKneeFunctionOutputDispLuminance",              m_kneeSEIOutputDispLuminance,                       800, "Specifies the expected display brightness for the output picture of Knee function SEI messages")
  ("SEIKneeFunctionNumKneePointsMinus1",              m_kneeSEINumKneePointsMinus1,                         2, "Specifies the number of knee points - 1")
  ("SEIKneeFunctionInputKneePointValue",              cfg_kneeSEIInputKneePointValue,   cfg_kneeSEIInputKneePointValue, "Array of input knee point")
  ("SEIKneeFunctionOutputKneePointValue",             cfg_kneeSEIOutputKneePointValue, cfg_kneeSEIOutputKneePointValue, "Array of output knee point")
  ("SEIMasteringDisplayColourVolume",                 m_masteringDisplay.colourVolumeSEIEnabled,         false, "Control generation of mastering display colour volume SEI messages")
  ("SEIMasteringDisplayMaxLuminance",                 m_masteringDisplay.maxLuminance,                  10000u, "Specifies the mastering display maximum luminance value in units of 1/10000 candela per square metre (32-bit code value)")
  ("SEIMasteringDisplayMinLuminance",                 m_masteringDisplay.minLuminance,                      0u, "Specifies the mastering display minimum luminance value in units of 1/10000 candela per square metre (32-bit code value)")
  ("SEIMasteringDisplayPrimaries",                    cfg_DisplayPrimariesCode,       cfg_DisplayPrimariesCode, "Mastering display primaries for all three colour planes in CIE xy coordinates in increments of 1/50000 (results in the ranges 0 to 50000 inclusive)")
  ("SEIMasteringDisplayWhitePoint",                   cfg_DisplayWhitePointCode,     cfg_DisplayWhitePointCode, "Mastering display white point CIE xy coordinates in normalised increments of 1/50000 (e.g. 0.333 = 16667)")
#if U0033_ALTERNATIVE_TRANSFER_CHARACTERISTICS_SEI
  ("SEIPreferredTransferCharacterisics",              m_preferredTransferCharacteristics,                   -1, "Value for the preferred_transfer_characteristics field of the Alternative transfer characteristics SEI which will override the corresponding entry in the VUI. If negative, do not produce the respective SEI message")
#endif
  ("SEIGreenMetadataType",                            m_greenMetadataType,                   0u, "Value for the green_metadata_type specifies the type of metadata that is present in the SEI message. If green_metadata_type is 1, then metadata enabling quality recovery after low-power encoding is present")
  ("SEIXSDMetricType",                                m_xsdMetricType,                      0u, "Value for the xsd_metric_type indicates the type of the objective quality metric. PSNR is the only type currently supported")

#if LAYERS_NOT_PRESENT_SEI
  ("SEILayersNotPresent",                             m_layersNotPresentSEIEnabled,             0, "Control generation of layers not present SEI message")
#endif   
#if P0123_ALPHA_CHANNEL_SEI
  ("SEIAlphaChannelInfo",                     m_alphaSEIEnabled,                        false, "Enables transmission of information associated with alpha channel (default : 0)")
  ("SEIAlphaCancelFlag",                      m_alphaCancelFlag,                         true, "Denotes that this SEI message cancels the persistence of any previously received alpha channel SEI message (default : 1)")
  ("SEIAlphaUseIdc",                          m_alphaUseIdc,                                2, "Denotes the use of the received alpha channel in final picture composition (e.g. pre-multiplied alpha, default : 2)")
  ("SEIAlphaBitDepthMinus8",                  m_alphaBitDepthMinus8,                        0, "Denotes the bit depth associated with the received alpha channel (default : 0)")
  ("SEIAlphaTransparentValue",                m_alphaTransparentValue,                      0, "Denotes the sample value which is considered transparent for alpha blending purposes (default : 0)")
  ("SEIAlphaOpaqueValue",                     m_alphaOpaqueValue,                         255, "Denotes the sample value which is considered opaque for alpha blending purposes (default : 255)")
  ("SEIAlphaIncrementFlag",                   m_alphaIncrementFlag,                     false, "Denotes whether the sample values should be incremented by one for the purposes of alpha blending (default : 0)")
  ("SEIAlphaClipFlag",                        m_alphaClipFlag,                          false, "Denotes whether clipping is applied to the sample values (default : 0)")
  ("SEIAlphaClipType",                        m_alphaClipTypeFlag,                      false, "Denotes the type of clipping applied to the sample values (0 = binary, 1 = linear, default : 0)")
#endif
#if Q0096_OVERLAY_SEI
  ("SEIOverlayInfo",                          m_overlaySEIEnabled,                      false, "Control generation of Selectable Overlays SEI messages")
  ("SEIOverlayCancelFlag",                    m_overlayInfoCancelFlag,                   true, "Indicates that Selectable Overlay SEI message cancels the persistance or follows (default: 1)")
  ("SEIOverlayContentAuxIdMinus128",          m_overlayContentAuxIdMinus128,          UInt(0), "Indicates the AuxId value of auxiliary pictures containing overlay content - 128")
  ("SEIOverlayLabelAuxIdMinus128",            m_overlayLabelAuxIdMinus128,            UInt(1), "Indicates the AuxId value of auxiliary pictures containing label content - 128")
  ("SEIOverlayAlphaAuxIdMinus128",            m_overlayAlphaAuxIdMinus128,            UInt(2), "Indicates the AuxId value of auxiliary pictures containing alpha content - 128")
  ("SEIOverlayElementLabelValueLengthMinus8", m_overlayElementLabelValueLengthMinus8, UInt(0), "Indicates the number of bits used for coding min and max label values - 8")
  ("SEIOverlayNumOverlaysMinus1",             m_numOverlaysMinus1,                    UInt(0), "Specifies the number of overlays described by this SEI message - 1")
  ("SEIOverlayIdx0",                          cfg_overlaySEIIdx[0],                   UInt(0), "Indicates the index of overlay 0")
  ("SEIOverlayIdx1",                          cfg_overlaySEIIdx[1],                   UInt(1), "Indicates the index of overlay 1")
  ("SEIOverlayIdx2",                          cfg_overlaySEIIdx[2],                   UInt(2), "Indicates the index of overlay 2")
  ("SEIOverlayLanguagePresentFlag0",          cfg_overlaySEILanguagePresentFlag[0],     false, "Indicates if the language for overlay 0 is specified")
  ("SEIOverlayLanguagePresentFlag1",          cfg_overlaySEILanguagePresentFlag[1],     false, "Indicates if the language for overlay 1 is specified")
  ("SEIOverlayLanguagePresentFlag2",          cfg_overlaySEILanguagePresentFlag[2],     false, "Indicates if the language for overlay 2 is specified")
  ("SEIOverlayContentLayerId0",               cfg_overlaySEIContentLayerId[0],        UInt(0), "Indicates the nuh_layer_id value of the overlay content of overlay 0")  
  ("SEIOverlayContentLayerId1",               cfg_overlaySEIContentLayerId[1],        UInt(0), "Indicates the nuh_layer_id value of the overlay content of overlay 1")  
  ("SEIOverlayContentLayerId2",               cfg_overlaySEIContentLayerId[2],        UInt(0), "Indicates the nuh_layer_id value of the overlay content of overlay 2")  
  ("SEIOverlayLabelPresentFlag0",             cfg_overlaySEILabelPresentFlag[0],        false, "Specifies if label content 0 is present")  
  ("SEIOverlayLabelPresentFlag1",             cfg_overlaySEILabelPresentFlag[1],        false, "Specifies if label content 1 is present")  
  ("SEIOverlayLabelPresentFlag2",             cfg_overlaySEILabelPresentFlag[2],        false, "Specifies if label content 2 is present")  
  ("SEIOverlayLabelLayerId0",                 cfg_overlaySEILabelLayerId[0],          UInt(0), "Specifies the nuh_layer_id value of the label content of overlay 0")  
  ("SEIOverlayLabelLayerId1",                 cfg_overlaySEILabelLayerId[1],          UInt(0), "Specifies the nuh_layer_id value of the label content of overlay 1")  
  ("SEIOverlayLabelLayerId2",                 cfg_overlaySEILabelLayerId[2],          UInt(0), "Specifies the nuh_layer_id value of the label content of overlay 2")  
  ("SEIOverlayAlphaPresentFlag0",             cfg_overlaySEIAlphaPresentFlag[0],        false, "Specifies if alpha content 0 is present")  
  ("SEIOverlayAlphaPresentFlag1",             cfg_overlaySEIAlphaPresentFlag[1],        false, "Specifies if alpha content 1 is present")  
  ("SEIOverlayAlphaPresentFlag2",             cfg_overlaySEIAlphaPresentFlag[2],        false, "Specifies if alpha content 2 is present")  
  ("SEIOverlayAlphaLayerId0",                 cfg_overlaySEIAlphaLayerId[0],          UInt(0), "Specifies the nuh_layer_id value of the alpha content of overlay 0")  
  ("SEIOverlayAlphaLayerId1",                 cfg_overlaySEIAlphaLayerId[1],          UInt(0), "Specifies the nuh_layer_id value of the alpha content of overlay 1")  
  ("SEIOverlayAlphaLayerId2",                 cfg_overlaySEIAlphaLayerId[2],          UInt(0), "Specifies the nuh_layer_id value of the alpha content of overlay 2")  
  ("SEIOverlayNumElementsMinus1_0",           cfg_overlaySEINumElementsMinus1[0],     UInt(0), "Specifies the number of overlay elements in overlay 0")  
  ("SEIOverlayNumElementsMinus1_1",           cfg_overlaySEINumElementsMinus1[1],     UInt(0), "Specifies the number of overlay elements in overlay 1")  
  ("SEIOverlayNumElementsMinus1_2",           cfg_overlaySEINumElementsMinus1[2],     UInt(0), "Specifies the number of overlay elements in overlay 2")  
  ("SEIOverlayElementLabelRange0",            cfg_overlaySEIElementLabelRanges[0], string(""), "Array of minimum and maximum values in order min0, max0, min1, max1, etc.\n"""
                                                                                               "indicating the range of sample values corresponding to overlay elements of overlay 0")    
  ("SEIOverlayElementLabelRange1",            cfg_overlaySEIElementLabelRanges[1], string(""), "Array of minimum and maximum values in order min0, max0, min1, max1, etc.\n"""
                                                                                               "indicating the range of sample values corresponding to overlay elements of overlay 1")    
  ("SEIOverlayElementLabelRange2",            cfg_overlaySEIElementLabelRanges[2], string(""), "Array of minimum and maximum values in order min0, max0, min1, max1, etc.\n"""
                                                                                               "indicating the range of sample values corresponding to overlay elements of overlay 2")  
  ("SEIOverlayLanguage0",                     cfg_overlaySEILanguage[0],           string(""), "Indicates the language of overlay 0 by a language tag according to RFC 5646")
  ("SEIOverlayLanguage1",                     cfg_overlaySEILanguage[1],           string(""), "Indicates the language of overlay 1 by a language tag according to RFC 5646")
  ("SEIOverlayLanguage2",                     cfg_overlaySEILanguage[2],           string(""), "Indicates the language of overlay 2 by a language tag according to RFC 5646")
  ("SEIOverlayName0",                         cfg_overlaySEIName[0],       string("Overlay0"), "Indicates the name of overlay 0")
  ("SEIOverlayName1",                         cfg_overlaySEIName[1],       string("Overlay1"), "Indicates the name of overlay 1")
  ("SEIOverlayName2",                         cfg_overlaySEIName[2],       string("Overlay2"), "Indicates the name of overlay 2")
  ("SEIOverlayElementNames0",                 cfg_overlaySEIElementNames[0],       string(""), "Indicates the names of all overlay elements of overlay 0 in the format name1|name2|name3|...")                                                                                       
  ("SEIOverlayElementNames1",                 cfg_overlaySEIElementNames[1],       string(""), "Indicates the names of all overlay elements of overlay 1 in the format name1|name2|name3|...")                                                                                       
  ("SEIOverlayElementNames2",                 cfg_overlaySEIElementNames[2],       string(""), "Indicates the names of all overlay elements of overlay 2 in the format name1|name2|name3|...")                                                                                       
  ("SEIOverlayPersistenceFlag",               m_overlayInfoPersistenceFlag,              true, "Indicates if the SEI message applies to the current picture only (0) or also to following pictures (1)")
#endif
#if SVC_EXTENSION
#if Q0189_TMVP_CONSTRAINTS
  ("SEITemporalMotionVectorPredictionConstraints",             m_TMVPConstraintsSEIEnabled,              0, "Control generation of TMVP constrants SEI message")
#endif
  ("AdaptiveResolutionChange",                                  m_adaptiveResolutionChange, 0, "Adaptive resolution change frame number. Should coincide with EL RAP picture. (0: disable)")
  ("LayerSwitchOffBegin%d",                           cfg_layerSwitchOffBegin, 0, m_numLayers, "Switch layer %d off after given poc")
  ("LayerSwitchOffEnd%d",                               cfg_layerSwitchOffEnd, 0, m_numLayers, "Switch layer %d on at given poc")
  ("SkipPictureAtArcSwitch",                                  m_skipPictureAtArcSwitch, false, "Code the higher layer picture in ARC up-switching as a skip picture. (0: disable)")
#if N0383_IL_CONSTRAINED_TILE_SETS_SEI
  ("SEIInterLayerConstrainedTileSets",       m_interLayerConstrainedTileSetsSEIEnabled, false, "Control generation of inter layer constrained tile sets SEI message")
  ("IlNumSetsInMessage",                     m_ilNumSetsInMessage,                         0u, "Number of inter layer constrained tile sets")
  ("TileSetsArray",                          m_tileSets,                           string(""), "Array containing tile sets params (TopLeftTileIndex, BottonRightTileIndex and ilcIdc for each set) ")
#endif
  ("AltOutputLayerFlag",                     m_altOutputLayerFlag,                      false, "Specifies the value of alt_output_layer_flag in VPS extension")
  ("CrossLayerBLAFlag",                      m_crossLayerBLAFlag,                       false, "Specifies the value of cross_layer_bla_flag in VPS")
#if CGS_3D_ASYMLUT
  ("CGS",                                                                       m_nCGSFlag, 0, "whether CGS is enabled")
  ("CGSMaxOctantDepth",                                               m_nCGSMaxOctantDepth, 1, "max octant depth")
  ("CGSMaxYPartNumLog",                                              m_nCGSMaxYPartNumLog2, 2, "max Y part number ")
  ("CGSLUTBit",                                                              m_nCGSLUTBit, 12, "bit depth of CGS LUT")
  ("CGSAdaptC",                                                       m_nCGSAdaptiveChroma, 1, "adaptive chroma partition (only for the case of two chroma partitions)")
#if R0179_ENC_OPT_3DLUT_SIZE
  ("CGSSizeRDO",                                                          m_nCGSLutSizeRDO, 0, "RDOpt selection of best table size (effective when large maximum table size such as 8x8x8 is used)")
#endif
#endif
  ("InheritCodingStruct%d",                           cfg_inheritCodingStruct, 0, m_numLayers, "Predicts the GOP structure of one layer for another layer")
#endif //SVC_EXTENSION
  ;

#if SVC_EXTENSION
  for (Int j=0; j<m_numLayers; j++)
  {
    GOPEntry* m_GOPList = m_apcLayerCfg[j]->m_GOPList;
#endif
  for(Int i=1; i<MAX_GOP+1; i++)
  {
    std::ostringstream cOSS;
    cOSS<<"Frame"<<i;
    opts.addOptions()(cOSS.str(), m_GOPList[i-1], GOPEntry());
  }
#if SVC_EXTENSION
  }
#endif

  po::setDefaults(opts);
  po::ErrorReporter err;
  const list<const TChar*>& argv_unhandled = po::scanArgv(opts, argc, (const TChar**) argv, err);

#if SVC_EXTENSION
#if AVC_BASE
  if( m_nonHEVCBaseLayerFlag )
  {
    *cfg_InputFile[0] = m_inputFileNameBL;
  }
#endif

  for( Int i=1; i<m_numLayers; i++ )
  {    
    if( m_apcLayerCfg[i]->m_inheritCodingStruct > 0)
    {
      for( Int j=1; j<MAX_GOP+1; j++ )
      {
        m_apcLayerCfg[i]->m_GOPList[j-1] = m_apcLayerCfg[m_apcLayerCfg[i]->m_inheritCodingStruct]->m_GOPList[j-1];
      }
    }
  }
#endif 
  
  for (list<const TChar*>::const_iterator it = argv_unhandled.begin(); it != argv_unhandled.end(); it++)
  {
    fprintf(stderr, "Unhandled argument ignored: `%s'\n", *it);
  }

  if (argc == 1 || do_help)
  {
    /* argc == 1: no options have been specified */
    po::doHelp(cout, opts);
    return false;
  }

  if (err.is_errored)
  {
    if (!warnUnknowParameter)
    {
      /* error report has already been printed on stderr */
      return false;
    }
  }

  /*
   * Set any derived parameters
   */

  m_framesToBeEncoded = ( m_framesToBeEncoded + m_temporalSubsampleRatio - 1 ) / m_temporalSubsampleRatio;
  m_adIntraLambdaModifier = cfg_adIntraLambdaModifier.values;
  if(m_isField)
  {
#if SVC_EXTENSION
    for(Int layer = 0; layer < m_numLayers; layer++)
    {
      //Frame height
      m_apcLayerCfg[layer]->m_iSourceHeightOrg = m_apcLayerCfg[layer]->m_iSourceHeight;
      //Field height
      m_apcLayerCfg[layer]->m_iSourceHeight = m_apcLayerCfg[layer]->m_iSourceHeight >> 1;
    }
#else
    //Frame height
    m_iSourceHeightOrg = m_iSourceHeight;
    //Field height
    m_iSourceHeight = m_iSourceHeight >> 1;
#endif
    //number of fields to encode
    m_framesToBeEncoded *= 2;
  }

  if( !m_tileUniformSpacingFlag && m_numTileColumnsMinus1 > 0 )
  {
    if (cfg_ColumnWidth.values.size() > m_numTileColumnsMinus1)
    {
      printf( "The number of columns whose width are defined is larger than the allowed number of columns.\n" );
      exit( EXIT_FAILURE );
    }
    else if (cfg_ColumnWidth.values.size() < m_numTileColumnsMinus1)
    {
      printf( "The width of some columns is not defined.\n" );
      exit( EXIT_FAILURE );
    }
    else
    {
      m_tileColumnWidth.resize(m_numTileColumnsMinus1);
      for(UInt i=0; i<cfg_ColumnWidth.values.size(); i++)
      {
        m_tileColumnWidth[i]=cfg_ColumnWidth.values[i];
      }
    }
  }
  else
  {
    m_tileColumnWidth.clear();
  }

  if( !m_tileUniformSpacingFlag && m_numTileRowsMinus1 > 0 )
  {
    if (cfg_RowHeight.values.size() > m_numTileRowsMinus1)
    {
      printf( "The number of rows whose height are defined is larger than the allowed number of rows.\n" );
      exit( EXIT_FAILURE );
    }
    else if (cfg_RowHeight.values.size() < m_numTileRowsMinus1)
    {
      printf( "The height of some rows is not defined.\n" );
      exit( EXIT_FAILURE );
    }
    else
    {
      m_tileRowHeight.resize(m_numTileRowsMinus1);
      for(UInt i=0; i<cfg_RowHeight.values.size(); i++)
      {
        m_tileRowHeight[i]=cfg_RowHeight.values[i];
      }
    }
  }
  else
  {
    m_tileRowHeight.clear();
  }
  
#if SVC_EXTENSION
#if SCALABLE_REXT
  Int cfgTmpConstraintChromaFormat = tmpConstraintChromaFormat;
#endif
  for( Int layer = 0; layer < m_numLayers; layer++ )
  {
    if( m_apcLayerCfg[layer]->m_layerId < 0 )
    {
      m_apcLayerCfg[layer]->m_layerId = layer;
    }

    m_apcLayerCfg[layer]->m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA  ] = m_apcLayerCfg[layer]->m_inputBitDepth      [CHANNEL_TYPE_LUMA  ];
    m_apcLayerCfg[layer]->m_MSBExtendedBitDepth[CHANNEL_TYPE_CHROMA] = m_apcLayerCfg[layer]->m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA  ];

    if (m_apcLayerCfg[layer]->m_internalBitDepth   [CHANNEL_TYPE_LUMA  ] == 0)
    { 
      m_apcLayerCfg[layer]->m_internalBitDepth   [CHANNEL_TYPE_LUMA  ] = m_apcLayerCfg[layer]->m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA  ];
    }
    if (m_apcLayerCfg[layer]->m_internalBitDepth   [CHANNEL_TYPE_CHROMA] == 0)
    { 
      m_apcLayerCfg[layer]->m_internalBitDepth   [CHANNEL_TYPE_CHROMA] = m_apcLayerCfg[layer]->m_internalBitDepth   [CHANNEL_TYPE_LUMA  ];
    }
    if (m_apcLayerCfg[layer]->m_inputBitDepth      [CHANNEL_TYPE_CHROMA] == 0)
    { 
      m_apcLayerCfg[layer]->m_inputBitDepth      [CHANNEL_TYPE_CHROMA] = m_apcLayerCfg[layer]->m_inputBitDepth      [CHANNEL_TYPE_LUMA  ];
    }
    if (m_apcLayerCfg[layer]->m_outputBitDepth     [CHANNEL_TYPE_LUMA  ] == 0)
    { 
      m_apcLayerCfg[layer]->m_outputBitDepth     [CHANNEL_TYPE_LUMA  ] = m_apcLayerCfg[layer]->m_internalBitDepth   [CHANNEL_TYPE_LUMA  ];
    }
    if (m_apcLayerCfg[layer]->m_outputBitDepth     [CHANNEL_TYPE_CHROMA] == 0)
    { 
      m_apcLayerCfg[layer]->m_outputBitDepth     [CHANNEL_TYPE_CHROMA] = m_apcLayerCfg[layer]->m_internalBitDepth   [CHANNEL_TYPE_CHROMA];
    }

    m_apcLayerCfg[layer]->m_InputChromaFormatIDC = numberToChromaFormat(tmpInputChromaFormat);
    m_apcLayerCfg[layer]->m_chromaFormatIDC      = ((tmpChromaFormat == 0) ? (m_apcLayerCfg[layer]->m_InputChromaFormatIDC) : (numberToChromaFormat(tmpChromaFormat)));

    if( m_apcLayerCfg[layer]->m_layerSwitchOffBegin < m_apcLayerCfg[layer]->m_layerSwitchOffEnd )
    {
      if( m_iGOPSize > 0 && (m_apcLayerCfg[layer]->m_layerSwitchOffBegin % m_iGOPSize) != 0 )
      {
        printf("LayerSwitchOffBegin%d: Must be multiple of GOP size.\n", layer);
        exit(EXIT_FAILURE);
      }
      if( m_apcLayerCfg[layer]->m_iIntraPeriod > 0 && (m_apcLayerCfg[layer]->m_layerSwitchOffEnd % m_apcLayerCfg[layer]->m_iIntraPeriod) != 0 )
      {
        printf("LayerSwitchOffEnd%d: Must be IRAP picture.\n", layer);
        exit(EXIT_FAILURE);
      }
    }

    m_apcLayerCfg[layer]->m_bitDepthConstraint = tmpBitDepthConstraint;
    m_apcLayerCfg[layer]->m_intraConstraintFlag = tmpIntraConstraintFlag;
    m_apcLayerCfg[layer]->m_lowerBitRateConstraintFlag = tmpLowerBitRateConstraintFlag;    

    for (UInt channelType = 0; channelType < MAX_NUM_CHANNEL_TYPE; channelType++)
    {
      m_log2SaoOffsetScale[channelType] = 0;
    }
        
    Int layerPTLIdx = m_apcLayerCfg[layer]->m_layerPTLIdx;

    if( extendedProfile[layerPTLIdx] >= 1000 && extendedProfile[layerPTLIdx] <= 2316 )
    {
      m_profileList[layerPTLIdx] = Profile::MAINREXT;            

#if SCALABLE_REXT
      if(m_numLayers > 1 && layer == 0 )
      {
        m_profileList[0] = extendedToShortProfileName(extendedProfile[0]);
      }

      if( m_apcLayerCfg[layer]->m_bitDepthConstraint != 0 || cfgTmpConstraintChromaFormat != 0)
#else
      if( m_apcLayerCfg[layer]->m_bitDepthConstraint != 0 || tmpConstraintChromaFormat != 0)
#endif
      {
        fprintf(stderr, "Error: The bit depth and chroma format constraints are not used when an explicit RExt profile is specified\n");
        exit(EXIT_FAILURE);
      }
      m_apcLayerCfg[layer]->m_bitDepthConstraint  = (extendedProfile[layerPTLIdx]%100);
      m_apcLayerCfg[layer]->m_intraConstraintFlag = (extendedProfile[layerPTLIdx]>=2000);
      switch ((extendedProfile[layerPTLIdx]/100)%10)
      {
      case 0:  tmpConstraintChromaFormat=400; break;
      case 1:  tmpConstraintChromaFormat=420; break;
      case 2:  tmpConstraintChromaFormat=422; break;
      default: tmpConstraintChromaFormat=444; break;
      }
    }
#if SCALABLE_REXT
    else if(extendedProfile[layerPTLIdx] >= 101008 && extendedProfile[layerPTLIdx] <= 101308)
    {
      m_profileList[layerPTLIdx] = Profile::SCALABLEREXT;
      
      if( m_apcLayerCfg[layer]->m_bitDepthConstraint != 0 || cfgTmpConstraintChromaFormat != 0)
      {
        fprintf(stderr, "Error: The bit depth and chroma format constraints are not used when an explicit Scalabe-RExt profile is specified\n");
        exit(EXIT_FAILURE);
      }
      m_apcLayerCfg[layer]->m_bitDepthConstraint  = (extendedProfile[layerPTLIdx]%100);
      m_apcLayerCfg[layer]->m_intraConstraintFlag = 0; // no all-intra constraint for scalable-Rext profiles 
      switch ((extendedProfile[layerPTLIdx]/100)%10)
      {
        case 0:  tmpConstraintChromaFormat=400; break;
        case 1:  tmpConstraintChromaFormat=420; break;
        case 2:  tmpConstraintChromaFormat=422; break;
        default: tmpConstraintChromaFormat=444; break;
      }
    }
#endif
    else
    {
      if( layer == 0 )
      {
#if SCALABLE_REXT
        m_profileList[0] = extendedToShortProfileName(extendedProfile[0]);
#else
        m_profileList[0] = Profile::Name(extendedProfile[0]);
#endif
      }

      m_profileList[layerPTLIdx] = Profile::Name(extendedProfile[layerPTLIdx]);
    }

    if( m_profileList[layerPTLIdx] == Profile::HIGHTHROUGHPUTREXT )
    {
      if( m_apcLayerCfg[layer]->m_bitDepthConstraint == 0 ) m_apcLayerCfg[layer]->m_bitDepthConstraint = 16;
      m_apcLayerCfg[layer]->m_chromaFormatConstraint = (tmpConstraintChromaFormat == 0) ? CHROMA_444 : numberToChromaFormat(tmpConstraintChromaFormat);
    }
#if SCALABLE_REXT
    else if( m_profileList[layerPTLIdx] == Profile::MAINREXT || m_profileList[layerPTLIdx] == Profile::SCALABLEREXT )
#else
    else if( m_profileList[layerPTLIdx] == Profile::MAINREXT )
#endif
    {
      if( m_apcLayerCfg[layer]->m_bitDepthConstraint == 0 && tmpConstraintChromaFormat == 0 )
      {
        // produce a valid combination, if possible.
        const Bool bUsingGeneralRExtTools  = m_transformSkipRotationEnabledFlag  ||
          m_transformSkipContextEnabledFlag         ||
          m_rdpcmEnabledFlag[RDPCM_SIGNAL_IMPLICIT] ||
          m_rdpcmEnabledFlag[RDPCM_SIGNAL_EXPLICIT] ||
          !m_enableIntraReferenceSmoothing          ||
          m_persistentRiceAdaptationEnabledFlag     ||
          m_log2MaxTransformSkipBlockSize!=2;
        const Bool bUsingChromaQPAdjustment= m_diffCuChromaQpOffsetDepth >= 0;
        const Bool bUsingExtendedPrecision = m_extendedPrecisionProcessingFlag;
        m_apcLayerCfg[layer]->m_chromaFormatConstraint = NUM_CHROMA_FORMAT;
        automaticallySelectRExtProfile(bUsingGeneralRExtTools,
          bUsingChromaQPAdjustment,
          bUsingExtendedPrecision,
          m_apcLayerCfg[layer]->m_intraConstraintFlag,
          m_apcLayerCfg[layer]->m_bitDepthConstraint,
          m_apcLayerCfg[layer]->m_chromaFormatConstraint,
          m_apcLayerCfg[layer]->m_chromaFormatIDC==CHROMA_400 ? m_apcLayerCfg[layer]->m_internalBitDepth[CHANNEL_TYPE_LUMA] : std::max(m_apcLayerCfg[layer]->m_internalBitDepth[CHANNEL_TYPE_LUMA], m_apcLayerCfg[layer]->m_internalBitDepth[CHANNEL_TYPE_CHROMA]),
          m_apcLayerCfg[layer]->m_chromaFormatIDC);
      }
      else if( m_apcLayerCfg[layer]->m_bitDepthConstraint == 0 || tmpConstraintChromaFormat == 0)
      {
        fprintf(stderr, "Error: The bit depth and chroma format constraints must either both be specified or both be configured automatically\n");
        exit(EXIT_FAILURE);
      }
      else
      {
        m_apcLayerCfg[layer]->m_chromaFormatConstraint = numberToChromaFormat(tmpConstraintChromaFormat);
      }
    }
    else
    {
      m_apcLayerCfg[layer]->m_chromaFormatConstraint = (tmpConstraintChromaFormat == 0) ? m_apcLayerCfg[layer]->m_chromaFormatIDC : numberToChromaFormat(tmpConstraintChromaFormat);
      m_apcLayerCfg[layer]->m_bitDepthConstraint = (m_profileList[layerPTLIdx] == Profile::MAIN10 || m_profileList[layerPTLIdx] == Profile::SCALABLEMAIN10) ? 10 : 8;
    }
  }
#else
  /* rules for input, output and internal bitdepths as per help text */
  if (m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA  ] == 0)
  {
    m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA  ] = m_inputBitDepth      [CHANNEL_TYPE_LUMA  ];
  }
  if (m_MSBExtendedBitDepth[CHANNEL_TYPE_CHROMA] == 0)
  {
    m_MSBExtendedBitDepth[CHANNEL_TYPE_CHROMA] = m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA  ];
  }
  if (m_internalBitDepth   [CHANNEL_TYPE_LUMA  ] == 0)
  {
    m_internalBitDepth   [CHANNEL_TYPE_LUMA  ] = m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA  ];
  }
  if (m_internalBitDepth   [CHANNEL_TYPE_CHROMA] == 0)
  {
    m_internalBitDepth   [CHANNEL_TYPE_CHROMA] = m_internalBitDepth   [CHANNEL_TYPE_LUMA  ];
  }
  if (m_inputBitDepth      [CHANNEL_TYPE_CHROMA] == 0)
  {
    m_inputBitDepth      [CHANNEL_TYPE_CHROMA] = m_inputBitDepth      [CHANNEL_TYPE_LUMA  ];
  }
  if (m_outputBitDepth     [CHANNEL_TYPE_LUMA  ] == 0)
  {
    m_outputBitDepth     [CHANNEL_TYPE_LUMA  ] = m_internalBitDepth   [CHANNEL_TYPE_LUMA  ];
  }
  if (m_outputBitDepth     [CHANNEL_TYPE_CHROMA] == 0)
  {
    m_outputBitDepth     [CHANNEL_TYPE_CHROMA] = m_internalBitDepth   [CHANNEL_TYPE_CHROMA];
  }

  m_InputChromaFormatIDC = numberToChromaFormat(tmpInputChromaFormat);
  m_chromaFormatIDC      = ((tmpChromaFormat == 0) ? (m_InputChromaFormatIDC) : (numberToChromaFormat(tmpChromaFormat)));
#endif
  assert(tmpWeightedPredictionMethod>=0 && tmpWeightedPredictionMethod<=WP_PER_PICTURE_WITH_HISTOGRAM_AND_PER_COMPONENT_AND_CLIPPING_AND_EXTENSION);
  if (!(tmpWeightedPredictionMethod>=0 && tmpWeightedPredictionMethod<=WP_PER_PICTURE_WITH_HISTOGRAM_AND_PER_COMPONENT_AND_CLIPPING_AND_EXTENSION))
  {
    exit(EXIT_FAILURE);
  }
  m_weightedPredictionMethod = WeightedPredictionMethod(tmpWeightedPredictionMethod);

  assert(tmpFastInterSearchMode>=0 && tmpFastInterSearchMode<=FASTINTERSEARCH_MODE3);
  if (tmpFastInterSearchMode<0 || tmpFastInterSearchMode>FASTINTERSEARCH_MODE3)
  {
    exit(EXIT_FAILURE);
  }
  m_fastInterSearchMode = FastInterSearchMode(tmpFastInterSearchMode);

  assert(tmpMotionEstimationSearchMethod>=0 && tmpMotionEstimationSearchMethod<MESEARCH_NUMBER_OF_METHODS);
  if (tmpMotionEstimationSearchMethod<0 || tmpMotionEstimationSearchMethod>=MESEARCH_NUMBER_OF_METHODS)
  {
    exit(EXIT_FAILURE);
  }
  m_motionEstimationSearchMethod=MESearchMethod(tmpMotionEstimationSearchMethod);

#if !SVC_EXTENSION
  if (extendedProfile >= 1000 && extendedProfile <= 12316)
  {
    m_profile = Profile::MAINREXT;
    if (m_bitDepthConstraint != 0 || tmpConstraintChromaFormat != 0)
    {
      fprintf(stderr, "Error: The bit depth and chroma format constraints are not used when an explicit RExt profile is specified\n");
      exit(EXIT_FAILURE);
    }
    m_bitDepthConstraint           = (extendedProfile%100);
    m_intraConstraintFlag          = ((extendedProfile%10000)>=2000);
    m_onePictureOnlyConstraintFlag = (extendedProfile >= 10000);
    switch ((extendedProfile/100)%10)
    {
      case 0:  tmpConstraintChromaFormat=400; break;
      case 1:  tmpConstraintChromaFormat=420; break;
      case 2:  tmpConstraintChromaFormat=422; break;
      default: tmpConstraintChromaFormat=444; break;
    }
  }
  else
  {
    m_profile = Profile::Name(extendedProfile);
  }

  if (m_profile == Profile::HIGHTHROUGHPUTREXT )
  {
    if (m_bitDepthConstraint == 0)
    {
      m_bitDepthConstraint = 16;
    }
    m_chromaFormatConstraint = (tmpConstraintChromaFormat == 0) ? CHROMA_444 : numberToChromaFormat(tmpConstraintChromaFormat);
  }
  else if (m_profile == Profile::MAINREXT)
  {
    if (m_bitDepthConstraint == 0 && tmpConstraintChromaFormat == 0)
    {
      // produce a valid combination, if possible.
      const Bool bUsingGeneralRExtTools  = m_transformSkipRotationEnabledFlag        ||
                                           m_transformSkipContextEnabledFlag         ||
                                           m_rdpcmEnabledFlag[RDPCM_SIGNAL_IMPLICIT] ||
                                           m_rdpcmEnabledFlag[RDPCM_SIGNAL_EXPLICIT] ||
                                           !m_enableIntraReferenceSmoothing          ||
                                           m_persistentRiceAdaptationEnabledFlag     ||
                                           m_log2MaxTransformSkipBlockSize!=2;
      const Bool bUsingChromaQPAdjustment= m_diffCuChromaQpOffsetDepth >= 0;
      const Bool bUsingExtendedPrecision = m_extendedPrecisionProcessingFlag;
      if (m_onePictureOnlyConstraintFlag)
      {
        m_chromaFormatConstraint = CHROMA_444;
        if (m_intraConstraintFlag != true)
        {
          fprintf(stderr, "Error: Intra constraint flag must be true when one_picture_only_constraint_flag is true\n");
          exit(EXIT_FAILURE);
        }
        const Int maxBitDepth = m_chromaFormatIDC==CHROMA_400 ? m_internalBitDepth[CHANNEL_TYPE_LUMA] : std::max(m_internalBitDepth[CHANNEL_TYPE_LUMA], m_internalBitDepth[CHANNEL_TYPE_CHROMA]);
        m_bitDepthConstraint = maxBitDepth>8 ? 16:8;
      }
      else
      {
        m_chromaFormatConstraint = NUM_CHROMA_FORMAT;
        automaticallySelectRExtProfile(bUsingGeneralRExtTools,
                                       bUsingChromaQPAdjustment,
                                       bUsingExtendedPrecision,
                                       m_intraConstraintFlag,
                                       m_bitDepthConstraint,
                                       m_chromaFormatConstraint,
                                       m_chromaFormatIDC==CHROMA_400 ? m_internalBitDepth[CHANNEL_TYPE_LUMA] : std::max(m_internalBitDepth[CHANNEL_TYPE_LUMA], m_internalBitDepth[CHANNEL_TYPE_CHROMA]),
                                       m_chromaFormatIDC);
      }
    }
    else if (m_bitDepthConstraint == 0 || tmpConstraintChromaFormat == 0)
    {
      fprintf(stderr, "Error: The bit depth and chroma format constraints must either both be specified or both be configured automatically\n");
      exit(EXIT_FAILURE);
    }
    else
    {
      m_chromaFormatConstraint = numberToChromaFormat(tmpConstraintChromaFormat);
    }
  }
  else
  {
    m_chromaFormatConstraint = (tmpConstraintChromaFormat == 0) ? m_chromaFormatIDC : numberToChromaFormat(tmpConstraintChromaFormat);
    m_bitDepthConstraint = (m_profile == Profile::MAIN10?10:8);
  }
#endif

  m_inputColourSpaceConvert = stringToInputColourSpaceConvert(inputColourSpaceConvert, true);

#if SVC_EXTENSION
  for(Int layer = 0; layer < m_numLayers; layer++)
  {
    // If number of scaled ref. layer offsets is non-zero, at least one of the offsets should be specified
    if(m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets)
    {
      assert( strcmp(cfg_refLocationOffsetLayerId[layer].c_str(),  ""));

      Bool srloFlag =
        strcmp(cfg_scaledRefLayerLeftOffset   [layer].c_str(), "") ||
        strcmp(cfg_scaledRefLayerRightOffset  [layer].c_str(), "") ||
        strcmp(cfg_scaledRefLayerTopOffset    [layer].c_str(), "") ||
        strcmp(cfg_scaledRefLayerBottomOffset [layer].c_str(), "");

      Bool rroFlag =
        strcmp(cfg_refRegionLeftOffset   [layer].c_str(), "") ||
        strcmp(cfg_refRegionRightOffset  [layer].c_str(), "") ||
        strcmp(cfg_refRegionTopOffset    [layer].c_str(), "") ||
        strcmp(cfg_refRegionBottomOffset [layer].c_str(), "");

      Bool phaseSetFlag =
        strcmp(cfg_phaseHorLuma   [layer].c_str(), "") ||
        strcmp(cfg_phaseVerLuma   [layer].c_str(), "") ||
        strcmp(cfg_phaseHorChroma [layer].c_str(), "") ||
        strcmp(cfg_phaseVerChroma [layer].c_str(), "");
      assert( srloFlag || rroFlag || phaseSetFlag);
    }

    Int *tempArray = NULL;   // Contain the value 

    // ID //
    if(strcmp(cfg_refLocationOffsetLayerId[layer].c_str(),  ""))
    {
      cfgStringToArray( &tempArray, cfg_refLocationOffsetLayerId[layer], m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets, "RefLocationOffsetLayerId");
      if(tempArray)
      {
        for(Int i = 0; i < m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets; i++)
        {
          m_apcLayerCfg[layer]->m_refLocationOffsetLayerId[i] = tempArray[i];
        }
        delete [] tempArray; tempArray = NULL;
      }
    }

    // Presense Flag //
    if(strcmp(cfg_scaledRefLayerOffsetPresentFlag[layer].c_str(),  ""))
    {
      cfgStringToArray( &tempArray, cfg_scaledRefLayerOffsetPresentFlag[layer], m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets, "ScaledRefLayerOffsetPresentFlag");
      if(tempArray)
      {
        for(Int i = 0; i < m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets; i++)
        {
          m_apcLayerCfg[layer]->m_scaledRefLayerOffsetPresentFlag[i] = tempArray[i];
        }
        delete [] tempArray; tempArray = NULL;
      }
    }

    // Left offset //
    if(strcmp(cfg_scaledRefLayerLeftOffset[layer].c_str(),  ""))
    {
      cfgStringToArray( &tempArray, cfg_scaledRefLayerLeftOffset[layer], m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets, "LeftOffset");
      if(tempArray)
      {
        for(Int i = 0; i < m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets; i++)
        {
          m_apcLayerCfg[layer]->m_scaledRefLayerLeftOffset[i] = tempArray[i];
        }
        delete [] tempArray; tempArray = NULL;
      }
    }

    // Top offset //
    if(strcmp(cfg_scaledRefLayerTopOffset[layer].c_str(),  ""))
    {
      cfgStringToArray( &tempArray, cfg_scaledRefLayerTopOffset[layer], m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets, "TopOffset");
      if(tempArray)
      {
        for(Int i = 0; i < m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets; i++)
        {
          m_apcLayerCfg[layer]->m_scaledRefLayerTopOffset[i] = tempArray[i];
        }
        delete [] tempArray; tempArray = NULL;
      }
    }

    // Right offset //
    if(strcmp(cfg_scaledRefLayerRightOffset[layer].c_str(),  ""))
    {
      cfgStringToArray( &tempArray, cfg_scaledRefLayerRightOffset[layer], m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets, "RightOffset");
      if(tempArray)
      {
        for(Int i = 0; i < m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets; i++)
        {
          m_apcLayerCfg[layer]->m_scaledRefLayerRightOffset[i] = tempArray[i];
        }
        delete [] tempArray; tempArray = NULL;
      }
    }

    // Bottom offset //
    if(strcmp(cfg_scaledRefLayerBottomOffset[layer].c_str(),  ""))
    {
      cfgStringToArray( &tempArray, cfg_scaledRefLayerBottomOffset[layer], m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets, "BottomOffset");
      if(tempArray)
      {
        for(Int i = 0; i < m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets; i++)
        {
          m_apcLayerCfg[layer]->m_scaledRefLayerBottomOffset[i] = tempArray[i];
        }
        delete [] tempArray; tempArray = NULL;
      }
    }

    // Presense Flag //
    if(strcmp(cfg_refRegionOffsetPresentFlag[layer].c_str(),  ""))
    {
      cfgStringToArray( &tempArray, cfg_refRegionOffsetPresentFlag[layer], m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets, "RefRegionOffsetPresentFlag");
      if(tempArray)
      {
        for(Int i = 0; i < m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets; i++)
        {
          m_apcLayerCfg[layer]->m_refRegionOffsetPresentFlag[i] = tempArray[i];
        }
        delete [] tempArray; tempArray = NULL;
      }
    }

    // Left offset //
    if(strcmp(cfg_refRegionLeftOffset[layer].c_str(),  ""))
    {
      cfgStringToArray( &tempArray, cfg_refRegionLeftOffset[layer], m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets, "RefRegionLeftOffset");
      if(tempArray)
      {
        for(Int i = 0; i < m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets; i++)
        {
          m_apcLayerCfg[layer]->m_refRegionLeftOffset[i] = tempArray[i];
        }
        delete [] tempArray; tempArray = NULL;
      }
    }

    // Top offset //
    if(strcmp(cfg_refRegionTopOffset[layer].c_str(),  ""))
    {
      cfgStringToArray( &tempArray, cfg_refRegionTopOffset[layer], m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets, "RefRegionTopOffset");
      if(tempArray)
      {
        for(Int i = 0; i < m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets; i++)
        {
          m_apcLayerCfg[layer]->m_refRegionTopOffset[i] = tempArray[i];
        }
        delete [] tempArray; tempArray = NULL;
      }
    }

    // Right offset //
    if(strcmp(cfg_refRegionRightOffset[layer].c_str(),  ""))
    {
      cfgStringToArray( &tempArray, cfg_refRegionRightOffset[layer], m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets, "RefRegionRightOffset");
      if(tempArray)
      {
        for(Int i = 0; i < m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets; i++)
        {
          m_apcLayerCfg[layer]->m_refRegionRightOffset[i] = tempArray[i];
        }
        delete [] tempArray; tempArray = NULL;
      }
    }

    // Bottom offset //
    if(strcmp(cfg_refRegionBottomOffset[layer].c_str(),  ""))
    {
      cfgStringToArray( &tempArray, cfg_refRegionBottomOffset[layer], m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets, "RefRegionBottomOffset");
      if(tempArray)
      {
        for(Int i = 0; i < m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets; i++)
        {
          m_apcLayerCfg[layer]->m_refRegionBottomOffset[i] = tempArray[i];
        }
        delete [] tempArray; tempArray = NULL;
      }
    }

    Int numPhaseSet = m_apcLayerCfg[layer]->m_numRefLayerLocationOffsets;

    // Presense Flag //
    if(strcmp(cfg_resamplePhaseSetPresentFlag[layer].c_str(),  ""))
    {
      cfgStringToArray( &tempArray, cfg_resamplePhaseSetPresentFlag[layer], numPhaseSet, "resamplePhaseSetPresentFlag");
      if(tempArray)
      {
        for(Int i = 0; i < numPhaseSet; i++)
        {
          m_apcLayerCfg[layer]->m_resamplePhaseSetPresentFlag[i] = tempArray[i];
        }
        delete [] tempArray; tempArray = NULL;
      }
    }

    // Luma horizontal phase //
    if(strcmp(cfg_phaseHorLuma[layer].c_str(),  ""))
    {
      cfgStringToArray( &tempArray, cfg_phaseHorLuma[layer], numPhaseSet, "phaseHorLuma");
      if(tempArray)
      {
        for(Int i = 0; i < numPhaseSet; i++)
        {
          m_apcLayerCfg[layer]->m_phaseHorLuma[i] = tempArray[i];
        }
        delete [] tempArray; tempArray = NULL;
      }
    }

    // Luma vertical phase //
    if(strcmp(cfg_phaseVerLuma[layer].c_str(),  ""))
    {
      cfgStringToArray( &tempArray, cfg_phaseVerLuma[layer], numPhaseSet, "phaseVerLuma");
      if(tempArray)
      {
        for(Int i = 0; i < numPhaseSet; i++)
        {
          m_apcLayerCfg[layer]->m_phaseVerLuma[i] = tempArray[i];
        }
        delete [] tempArray; tempArray = NULL;
      }
    }

    // Chroma horizontal phase //
    if(strcmp(cfg_phaseHorChroma[layer].c_str(),  ""))
    {
      cfgStringToArray( &tempArray, cfg_phaseHorChroma[layer], numPhaseSet, "phaseHorChroma");
      if(tempArray)
      {
        for(Int i = 0; i < numPhaseSet; i++)
        {
          m_apcLayerCfg[layer]->m_phaseHorChroma[i] = tempArray[i];
        }
        delete [] tempArray; tempArray = NULL;
      }
    }

    // Chroma vertical phase //
    if(strcmp(cfg_phaseVerChroma[layer].c_str(),  ""))
    {
      cfgStringToArray( &tempArray, cfg_phaseVerChroma[layer], numPhaseSet, "phaseVerChroma");
      if(tempArray)
      {
        for(Int i = 0; i < numPhaseSet; i++)
        {
          m_apcLayerCfg[layer]->m_phaseVerChroma[i] = tempArray[i];
        }
        delete [] tempArray; tempArray = NULL;
      }
    }

    TChar* pSamplePredRefLayerIds = cfg_samplePredRefLayerIds[layer].empty() ? NULL: strdup(cfg_samplePredRefLayerIds[layer].c_str());
    if( m_apcLayerCfg[layer]->m_numSamplePredRefLayers > 0 )
    {
      char *samplePredRefLayerId;
      int  i=0;
      m_apcLayerCfg[layer]->m_samplePredRefLayerIds = new Int[m_apcLayerCfg[layer]->m_numSamplePredRefLayers];
      samplePredRefLayerId = strtok(pSamplePredRefLayerIds, " ,-");
      while(samplePredRefLayerId != NULL)
      {
        if( i >= m_apcLayerCfg[layer]->m_numSamplePredRefLayers )
        {
          printf( "NumSamplePredRefLayers%d: The number of columns whose width are defined is larger than the allowed number of columns.\n", layer );
          exit( EXIT_FAILURE );
        }
        *( m_apcLayerCfg[layer]->m_samplePredRefLayerIds + i ) = atoi( samplePredRefLayerId );
        samplePredRefLayerId = strtok(NULL, " ,-");
        i++;
      }
      if( i < m_apcLayerCfg[layer]->m_numSamplePredRefLayers )
      {
        printf( "NumSamplePredRefLayers%d: The width of some columns is not defined.\n", layer );
        exit( EXIT_FAILURE );
      }
    }
    else
    {
      m_apcLayerCfg[layer]->m_samplePredRefLayerIds = NULL;
    }

    if( pSamplePredRefLayerIds )
    {
      free( pSamplePredRefLayerIds );
      pSamplePredRefLayerIds = NULL;
    }

    TChar* pMotionPredRefLayerIds = cfg_motionPredRefLayerIds[layer].empty() ? NULL: strdup(cfg_motionPredRefLayerIds[layer].c_str());
    if( m_apcLayerCfg[layer]->m_numMotionPredRefLayers > 0 )
    {
      char *motionPredRefLayerId;
      int  i=0;
      m_apcLayerCfg[layer]->m_motionPredRefLayerIds = new Int[m_apcLayerCfg[layer]->m_numMotionPredRefLayers];
      motionPredRefLayerId = strtok(pMotionPredRefLayerIds, " ,-");
      while(motionPredRefLayerId != NULL)
      {
        if( i >= m_apcLayerCfg[layer]->m_numMotionPredRefLayers )
        {
          printf( "NumMotionPredRefLayers%d: The number of columns whose width are defined is larger than the allowed number of columns.\n", layer );
          exit( EXIT_FAILURE );
        }
        *( m_apcLayerCfg[layer]->m_motionPredRefLayerIds + i ) = atoi( motionPredRefLayerId );
        motionPredRefLayerId = strtok(NULL, " ,-");
        i++;
      }
      if( i < m_apcLayerCfg[layer]->m_numMotionPredRefLayers )
      {
        printf( "NumMotionPredRefLayers%d: The width of some columns is not defined.\n", layer );
        exit( EXIT_FAILURE );
      }
    }
    else
    {
      m_apcLayerCfg[layer]->m_motionPredRefLayerIds = NULL;
    }

    if( pMotionPredRefLayerIds )
    {
      free( pMotionPredRefLayerIds );
      pMotionPredRefLayerIds = NULL;
    }

    TChar* pPredLayerIds = cfg_predLayerIds[layer].empty() ? NULL: strdup(cfg_predLayerIds[layer].c_str());
    if( m_apcLayerCfg[layer]->m_numActiveRefLayers > 0 )
    {
      TChar *refLayerId;
      Int  i=0;
      m_apcLayerCfg[layer]->m_predLayerIds = new Int[m_apcLayerCfg[layer]->m_numActiveRefLayers];
      refLayerId = strtok(pPredLayerIds, " ,-");
      while(refLayerId != NULL)
      {
        if( i >= m_apcLayerCfg[layer]->m_numActiveRefLayers )
        {
          printf( "NumActiveRefLayers%d: The number of columns whose width are defined is larger than the allowed number of columns.\n", layer );
          exit( EXIT_FAILURE );
        }
        *( m_apcLayerCfg[layer]->m_predLayerIds + i ) = atoi( refLayerId );
        refLayerId = strtok(NULL, " ,-");
        i++;
      }
      if( i < m_apcLayerCfg[layer]->m_numActiveRefLayers )
      {
        printf( "NumActiveRefLayers%d: The width of some columns is not defined.\n", layer );
        exit( EXIT_FAILURE );
      }
    }
    else
    {
      m_apcLayerCfg[layer]->m_predLayerIds = NULL;
    }

    if( pPredLayerIds )
    {
      free( pPredLayerIds );
      pPredLayerIds = NULL;
    }
  } //for(Int layer = 0; layer < m_numLayers; layer++)

  for (Int layerSet = 1; layerSet < m_numLayerSets; layerSet++)
  {
    // Simplifying the code in the #else section, and allowing 0-th layer set t
    assert( scanStringToArray( cfg_layerSetLayerIdList[layerSet], m_numLayerInIdList[layerSet], "NumLayerInIdList", m_layerSetLayerIdList[layerSet] ) );
  }

  for (Int addLayerSet = 0; addLayerSet < m_numAddLayerSets; addLayerSet++)
  {
    // Simplifying the code in the #else section
    assert( scanStringToArray( cfg_highestLayerIdx[addLayerSet], m_numHighestLayerIdx[addLayerSet], "HighestLayerIdx", m_highestLayerIdx[addLayerSet] ) );
  }

  if( m_defaultTargetOutputLayerIdc != -1 )
  {
    assert( m_defaultTargetOutputLayerIdc >= 0 && m_defaultTargetOutputLayerIdc <= 3 );
  }

  assert( m_numOutputLayerSets != 0 );
  assert( m_numOutputLayerSets >= m_numLayerSets + m_numAddLayerSets ); // Number of output layer sets must be at least as many as layer sets.

  // If output layer Set Idx is specified, only specify it for the non-default output layer sets
  Int numNonDefaultOls = m_numOutputLayerSets - (m_numLayerSets + m_numAddLayerSets);
  if( numNonDefaultOls )
  {
    assert( scanStringToArray( *cfg_outputLayerSetIdx, numNonDefaultOls, "OutputLayerSetIdx", m_outputLayerSetIdx ) ); 
    for(Int i = 0; i < numNonDefaultOls; i++)
    {
      assert( m_outputLayerSetIdx[i] >= 0 && m_outputLayerSetIdx[i] < (m_numLayerSets + m_numAddLayerSets) );
    }
  }

  // Number of output layers in output layer sets
  scanStringToArray( *cfg_numOutputLayersInOutputLayerSet, m_numOutputLayerSets - 1, "NumOutputLayersInOutputLayerSets", m_numOutputLayersInOutputLayerSet );
  m_numOutputLayersInOutputLayerSet.insert(m_numOutputLayersInOutputLayerSet.begin(), 1);

  // Layers in the output layer set
  m_listOfOutputLayers.resize(m_numOutputLayerSets);
  m_listOfLayerPTLofOlss.resize(m_numOutputLayerSets);

  Int startOlsCtr = 1;
  if( m_defaultTargetOutputLayerIdc == 0 || m_defaultTargetOutputLayerIdc == 1 )
  {
    // Default output layer sets defined
    startOlsCtr = m_numLayerSets;
  }

  std::vector<Profile::Name> profiles;

  if( m_scalabilityMask[VIEW_ORDER_INDEX] )
  {
    profiles.push_back( Profile::MULTIVIEWMAIN );
  }

  if( m_scalabilityMask[SCALABILITY_ID] )
  {
    profiles.push_back( Profile::SCALABLEMAIN );
  }

  for( Int olsCtr = 1; olsCtr < m_numOutputLayerSets; olsCtr++ )
  {
    if( olsCtr < startOlsCtr )
    {
      if(scanStringToArray( cfg_listOfOutputLayers[olsCtr], m_numOutputLayersInOutputLayerSet[olsCtr], "ListOfOutputLayers", m_listOfOutputLayers[olsCtr] ) )
      {
        std::cout << "Default OLS defined. Ignoring ListOfOutputLayers" << olsCtr << endl;
      }
    }
    else
    {
      assert( scanStringToArray( cfg_listOfOutputLayers[olsCtr], m_numOutputLayersInOutputLayerSet[olsCtr], "ListOfOutputLayers", m_listOfOutputLayers[olsCtr] ) );
    }

    Int olsToLsIndex = (olsCtr >= (m_numLayerSets + m_numAddLayerSets)) ? m_outputLayerSetIdx[olsCtr - (m_numLayerSets + m_numAddLayerSets)] : olsCtr;

    // This is a fix to allow setting of PTL for additional layer sets
    if( olsCtr >= m_numLayerSets && olsCtr < (m_numLayerSets + m_numAddLayerSets) )
    {
      scanStringToArrayNumEntries(cfg_listOfLayerPTLOfOlss[olsCtr], m_numLayerInIdList[olsToLsIndex], "List of PTL for each layers in OLS", m_listOfLayerPTLofOlss[olsCtr]);
    }
    else
    {
      scanStringToArray(cfg_listOfLayerPTLOfOlss[olsCtr], m_numLayerInIdList[olsToLsIndex], "List of PTL for each layers in OLS", m_listOfLayerPTLofOlss[olsCtr]);
    }

    //For conformance checking
    //Conformance of a layer in an output operation point associated with an OLS in a bitstream to the Scalable Main profile is indicated as follows:
    //If OpTid of the output operation point is equal to vps_max_sub_layer_minus1, the conformance is indicated by general_profile_idc being equal to 7 or general_profile_compatibility_flag[ 7 ] being equal to 1
    //Conformance of a layer in an output operation point associated with an OLS in a bitstream to the Scalable Main 10 profile is indicated as follows:
    //If OpTid of the output operation point is equal to vps_max_sub_layer_minus1, the conformance is indicated by general_profile_idc being equal to 7 or general_profile_compatibility_flag[ 7 ] being equal to 1
    //The following assert may be updated / upgraded to take care of general_profile_compatibility_flag.
    if( olsToLsIndex < m_numLayerSets )
    {
      std::vector<UInt> passedCheck(profiles.size(), 0);
      UInt numIncludedLayers = 0;

      for( Int ii = 1; ii < m_numLayerInIdList[olsToLsIndex]; ii++ )
      {
        if( m_layerSetLayerIdList[olsToLsIndex][ii - 1] != 0 && m_layerSetLayerIdList[olsToLsIndex][ii] != 0 )  //Profile / profile compatibility of enhancement layers must indicate the same profile.
        {
          numIncludedLayers++;
          const Profile::Name profileName = m_profileList[m_listOfLayerPTLofOlss[olsCtr][ii]] == Profile::SCALABLEMAIN10 ? Profile::SCALABLEMAIN : m_profileList[m_listOfLayerPTLofOlss[olsCtr][ii]];

          for( Int p = 0; p < profiles.size(); p++ )
          {
            passedCheck[p] += profileName == profiles[p] || m_profileCompatibility[Int(profiles[p])];
          }
        }
      }

      // check whether all layers are compatible to at least one extension profile
      assert( std::find(passedCheck.begin(), passedCheck.end(), numIncludedLayers) != passedCheck.end() );
    }
  }

  m_listOfLayerPTLofOlss[0].push_back(*cfg_layerPTLIdx[0]);
  delete [] cfg_listOfLayerPTLOfOlss;
  delete cfg_numOutputLayersInOutputLayerSet;
  delete [] cfg_listOfOutputLayers;
  delete cfg_outputLayerSetIdx;

  for( Int layer = 0; layer < m_numLayers; layer++ )
  {
    Int& m_conformanceWindowMode    = m_apcLayerCfg[layer]->m_conformanceWindowMode;
    UInt& m_uiMaxCUHeight           = m_apcLayerCfg[layer]->m_uiMaxCUHeight;
    UInt& m_uiMaxCUDepth            = m_apcLayerCfg[layer]->m_uiMaxCUDepth;
    ChromaFormat& m_chromaFormatIDC = m_apcLayerCfg[layer]->m_chromaFormatIDC;

    Int& m_confWinLeft              = m_apcLayerCfg[layer]->m_confWinLeft;
    Int& m_confWinRight             = m_apcLayerCfg[layer]->m_confWinRight;
    Int& m_confWinTop               = m_apcLayerCfg[layer]->m_confWinTop;
    Int& m_confWinBottom            = m_apcLayerCfg[layer]->m_confWinBottom;

    Int* m_aiPad                    = m_apcLayerCfg[layer]->m_aiPad;
    Int*& m_aidQP                   = m_apcLayerCfg[layer]->m_aidQP;

    Int& m_iSourceWidth             = m_apcLayerCfg[layer]->m_iSourceWidth;
    Int& m_iSourceHeight            = m_apcLayerCfg[layer]->m_iSourceHeight;
    Int& m_iSourceHeightOrg         = m_apcLayerCfg[layer]->m_iSourceHeightOrg;

    Int& m_iQP                      = m_apcLayerCfg[layer]->m_iQP;
    Double& m_fQP                   = m_apcLayerCfg[layer]->m_fQP;

    string& m_dQPFileName           = m_apcLayerCfg[layer]->m_dQPFileName;
    Int* m_internalBitDepth         = m_apcLayerCfg[layer]->m_internalBitDepth;
#endif //SVC_EXTENSION

  switch (m_conformanceWindowMode)
  {
  case 0:
    {
      // no conformance or padding
      m_confWinLeft = m_confWinRight = m_confWinTop = m_confWinBottom = 0;
      m_aiPad[1] = m_aiPad[0] = 0;
      break;
    }
  case 1:
    {
      // automatic padding to minimum CU size
      Int minCuSize = m_uiMaxCUHeight >> (m_uiMaxCUDepth - 1);
      if (m_iSourceWidth % minCuSize)
      {
        m_aiPad[0] = m_confWinRight  = ((m_iSourceWidth / minCuSize) + 1) * minCuSize - m_iSourceWidth;
        m_iSourceWidth  += m_confWinRight;
#if SVC_EXTENSION
        m_confWinRight /= TComSPS::getWinUnitX( m_chromaFormatIDC );
#endif
      }
      if (m_iSourceHeight % minCuSize)
      {
        m_aiPad[1] = m_confWinBottom = ((m_iSourceHeight / minCuSize) + 1) * minCuSize - m_iSourceHeight;
        m_iSourceHeight += m_confWinBottom;
        if ( m_isField )
        {
          m_iSourceHeightOrg += m_confWinBottom << 1;
          m_aiPad[1] = m_confWinBottom << 1;
        }
#if SVC_EXTENSION
        m_confWinBottom /= TComSPS::getWinUnitY( m_chromaFormatIDC );
#endif
      }
      if (m_aiPad[0] % TComSPS::getWinUnitX(m_chromaFormatIDC) != 0)
      {
        fprintf(stderr, "Error: picture width is not an integer multiple of the specified chroma subsampling\n");
        exit(EXIT_FAILURE);
      }
      if (m_aiPad[1] % TComSPS::getWinUnitY(m_chromaFormatIDC) != 0)
      {
        fprintf(stderr, "Error: picture height is not an integer multiple of the specified chroma subsampling\n");
        exit(EXIT_FAILURE);
      }
      break;
    }
  case 2:
    {
      //padding
      m_iSourceWidth  += m_aiPad[0];
      m_iSourceHeight += m_aiPad[1];
      m_confWinRight  = m_aiPad[0];
      m_confWinBottom = m_aiPad[1];

#if SVC_EXTENSION
      m_confWinRight /= TComSPS::getWinUnitX( m_chromaFormatIDC );
      m_confWinBottom /= TComSPS::getWinUnitY( m_chromaFormatIDC );
#endif
      break;
    }
  case 3:
    {
      // conformance
      if ((m_confWinLeft == 0) && (m_confWinRight == 0) && (m_confWinTop == 0) && (m_confWinBottom == 0))
      {
        fprintf(stderr, "Warning: Conformance window enabled, but all conformance window parameters set to zero\n");
      }
      if ((m_aiPad[1] != 0) || (m_aiPad[0]!=0))
      {
        fprintf(stderr, "Warning: Conformance window enabled, padding parameters will be ignored\n");
      }
      m_aiPad[1] = m_aiPad[0] = 0;
      break;
    }
  }

  if (tmpSliceMode<0 || tmpSliceMode>=Int(NUMBER_OF_SLICE_CONSTRAINT_MODES))
  {
    fprintf(stderr, "Error: bad slice mode\n");
    exit(EXIT_FAILURE);
  }
  m_sliceMode = SliceConstraint(tmpSliceMode);
  if (tmpSliceSegmentMode<0 || tmpSliceSegmentMode>=Int(NUMBER_OF_SLICE_CONSTRAINT_MODES))
  {
    fprintf(stderr, "Error: bad slice segment mode\n");
    exit(EXIT_FAILURE);
  }
  m_sliceSegmentMode = SliceConstraint(tmpSliceSegmentMode);

  if (tmpDecodedPictureHashSEIMappedType<0 || tmpDecodedPictureHashSEIMappedType>=Int(NUMBER_OF_HASHTYPES))
  {
    fprintf(stderr, "Error: bad checksum mode\n");
    exit(EXIT_FAILURE);
  }
  // Need to map values to match those of the SEI message:
  if (tmpDecodedPictureHashSEIMappedType==0)
  {
    m_decodedPictureHashSEIType=HASHTYPE_NONE;
  }
  else
  {
    m_decodedPictureHashSEIType=HashType(tmpDecodedPictureHashSEIMappedType-1);
  }

  // allocate slice-based dQP values
  m_aidQP = new Int[ m_framesToBeEncoded + m_iGOPSize + 1 ];
  ::memset( m_aidQP, 0, sizeof(Int)*( m_framesToBeEncoded + m_iGOPSize + 1 ) );

  // handling of floating-point QP values
  // if QP is not integer, sequence is split into two sections having QP and QP+1
  m_iQP = (Int)( m_fQP );
  if ( m_iQP < m_fQP )
  {
    Int iSwitchPOC = (Int)( m_framesToBeEncoded - (m_fQP - m_iQP)*m_framesToBeEncoded + 0.5 );

    iSwitchPOC = (Int)( (Double)iSwitchPOC / m_iGOPSize + 0.5 )*m_iGOPSize;
    for ( Int i=iSwitchPOC; i<m_framesToBeEncoded + m_iGOPSize + 1; i++ )
    {
      m_aidQP[i] = 1;
    }
  }

  for(UInt ch=0; ch<MAX_NUM_CHANNEL_TYPE; ch++)
  {
    if (saoOffsetBitShift[ch]<0)
    {
      if (m_internalBitDepth[ch]>10)
      {
        m_log2SaoOffsetScale[ch]=UInt(Clip3<Int>(0, m_internalBitDepth[ch]-10, Int(m_internalBitDepth[ch]-10 + 0.165*m_iQP - 3.22 + 0.5) ) );
      }
      else
      {
        m_log2SaoOffsetScale[ch]=0;
      }
    }
    else
    {
      m_log2SaoOffsetScale[ch]=UInt(saoOffsetBitShift[ch]);
    }
  }

  // reading external dQP description from file
  if ( !m_dQPFileName.empty() )
  {
    FILE* fpt=fopen( m_dQPFileName.c_str(), "r" );
    if ( fpt )
    {
      Int iValue;
      Int iPOC = 0;
      while ( iPOC < m_framesToBeEncoded )
      {
        if ( fscanf(fpt, "%d", &iValue ) == EOF )
        {
          break;
        }
        m_aidQP[ iPOC ] = iValue;
        iPOC++;
      }
      fclose(fpt);
    }
  }
#if SVC_EXTENSION
  }
#endif

  if( m_masteringDisplay.colourVolumeSEIEnabled )
  {
    for(UInt idx=0; idx<6; idx++)
    {
      m_masteringDisplay.primaries[idx/2][idx%2] = UShort((cfg_DisplayPrimariesCode.values.size() > idx) ? cfg_DisplayPrimariesCode.values[idx] : 0);
    }
    for(UInt idx=0; idx<2; idx++)
    {
      m_masteringDisplay.whitePoint[idx] = UShort((cfg_DisplayWhitePointCode.values.size() > idx) ? cfg_DisplayWhitePointCode.values[idx] : 0);
    }
  }
    
  if( m_toneMappingInfoSEIEnabled && !m_toneMapCancelFlag )
  {
    if( m_toneMapModelId == 2 && !cfg_startOfCodedInterval.values.empty() )
    {
      const UInt num = 1u<< m_toneMapTargetBitDepth;
      m_startOfCodedInterval = new Int[num];
      for(UInt i=0; i<num; i++)
      {
        m_startOfCodedInterval[i] = cfg_startOfCodedInterval.values.size() > i ? cfg_startOfCodedInterval.values[i] : 0;
      }
    }
    else
    {
      m_startOfCodedInterval = NULL;
    }
    if( ( m_toneMapModelId == 3 ) && ( m_numPivots > 0 ) )
    {
      if( !cfg_codedPivotValue.values.empty() && !cfg_targetPivotValue.values.empty() )
      {
        m_codedPivotValue  = new Int[m_numPivots];
        m_targetPivotValue = new Int[m_numPivots];
        for(UInt i=0; i<m_numPivots; i++)
        {
          m_codedPivotValue[i]  = cfg_codedPivotValue.values.size()  > i ? cfg_codedPivotValue.values [i] : 0;
          m_targetPivotValue[i] = cfg_targetPivotValue.values.size() > i ? cfg_targetPivotValue.values[i] : 0;
        }
      }
    }
    else
    {
      m_codedPivotValue = NULL;
      m_targetPivotValue = NULL;
    }
  }

  if( m_kneeSEIEnabled && !m_kneeSEICancelFlag )
  {
    assert ( m_kneeSEINumKneePointsMinus1 >= 0 && m_kneeSEINumKneePointsMinus1 < 999 );
    m_kneeSEIInputKneePoint  = new Int[m_kneeSEINumKneePointsMinus1+1];
    m_kneeSEIOutputKneePoint = new Int[m_kneeSEINumKneePointsMinus1+1];
    for(Int i=0; i<(m_kneeSEINumKneePointsMinus1+1); i++)
    {
      m_kneeSEIInputKneePoint[i]  = cfg_kneeSEIInputKneePointValue.values.size()  > i ? cfg_kneeSEIInputKneePointValue.values[i]  : 1;
      m_kneeSEIOutputKneePoint[i] = cfg_kneeSEIOutputKneePointValue.values.size() > i ? cfg_kneeSEIOutputKneePointValue.values[i] : 0;
    }
  }

  if(m_timeCodeSEIEnabled)
  {
    for(Int i = 0; i < m_timeCodeSEINumTs && i < MAX_TIMECODE_SEI_SETS; i++)
    {
      m_timeSetArray[i].clockTimeStampFlag    = cfg_timeCodeSeiTimeStampFlag        .values.size()>i ? cfg_timeCodeSeiTimeStampFlag        .values [i] : false;
      m_timeSetArray[i].numUnitFieldBasedFlag = cfg_timeCodeSeiNumUnitFieldBasedFlag.values.size()>i ? cfg_timeCodeSeiNumUnitFieldBasedFlag.values [i] : 0;
      m_timeSetArray[i].countingType          = cfg_timeCodeSeiCountingType         .values.size()>i ? cfg_timeCodeSeiCountingType         .values [i] : 0;
      m_timeSetArray[i].fullTimeStampFlag     = cfg_timeCodeSeiFullTimeStampFlag    .values.size()>i ? cfg_timeCodeSeiFullTimeStampFlag    .values [i] : 0;
      m_timeSetArray[i].discontinuityFlag     = cfg_timeCodeSeiDiscontinuityFlag    .values.size()>i ? cfg_timeCodeSeiDiscontinuityFlag    .values [i] : 0;
      m_timeSetArray[i].cntDroppedFlag        = cfg_timeCodeSeiCntDroppedFlag       .values.size()>i ? cfg_timeCodeSeiCntDroppedFlag       .values [i] : 0;
      m_timeSetArray[i].numberOfFrames        = cfg_timeCodeSeiNumberOfFrames       .values.size()>i ? cfg_timeCodeSeiNumberOfFrames       .values [i] : 0;
      m_timeSetArray[i].secondsValue          = cfg_timeCodeSeiSecondsValue         .values.size()>i ? cfg_timeCodeSeiSecondsValue         .values [i] : 0;
      m_timeSetArray[i].minutesValue          = cfg_timeCodeSeiMinutesValue         .values.size()>i ? cfg_timeCodeSeiMinutesValue         .values [i] : 0;
      m_timeSetArray[i].hoursValue            = cfg_timeCodeSeiHoursValue           .values.size()>i ? cfg_timeCodeSeiHoursValue           .values [i] : 0;
      m_timeSetArray[i].secondsFlag           = cfg_timeCodeSeiSecondsFlag          .values.size()>i ? cfg_timeCodeSeiSecondsFlag          .values [i] : 0;
      m_timeSetArray[i].minutesFlag           = cfg_timeCodeSeiMinutesFlag          .values.size()>i ? cfg_timeCodeSeiMinutesFlag          .values [i] : 0;
      m_timeSetArray[i].hoursFlag             = cfg_timeCodeSeiHoursFlag            .values.size()>i ? cfg_timeCodeSeiHoursFlag            .values [i] : 0;
      m_timeSetArray[i].timeOffsetLength      = cfg_timeCodeSeiTimeOffsetLength     .values.size()>i ? cfg_timeCodeSeiTimeOffsetLength     .values [i] : 0;
      m_timeSetArray[i].timeOffsetValue       = cfg_timeCodeSeiTimeOffsetValue      .values.size()>i ? cfg_timeCodeSeiTimeOffsetValue      .values [i] : 0;
    }
  }
#if Q0096_OVERLAY_SEI
  if( m_overlaySEIEnabled && !m_overlayInfoCancelFlag )
  {
    m_overlayIdx.resize                 ( m_numOverlaysMinus1+1 );  
    m_overlayLanguagePresentFlag.resize ( m_numOverlaysMinus1+1 );
    m_overlayContentLayerId.resize      ( m_numOverlaysMinus1+1 );
    m_overlayLabelPresentFlag.resize    ( m_numOverlaysMinus1+1 );
    m_overlayLabelLayerId.resize        ( m_numOverlaysMinus1+1 );
    m_overlayAlphaPresentFlag.resize    ( m_numOverlaysMinus1+1 );
    m_overlayAlphaLayerId.resize        ( m_numOverlaysMinus1+1 );
    m_numOverlayElementsMinus1.resize   ( m_numOverlaysMinus1+1 );
    m_overlayElementLabelMin.resize     ( m_numOverlaysMinus1+1 );
    m_overlayElementLabelMax.resize     ( m_numOverlaysMinus1+1 );  
    m_overlayLanguage.resize            ( m_numOverlaysMinus1+1 );     
    m_overlayName.resize                ( m_numOverlaysMinus1+1 );     
    m_overlayElementName.resize         ( m_numOverlaysMinus1+1 );     
    for (Int i=0 ; i<=m_numOverlaysMinus1 ; i++)
    {
      m_overlayIdx[i]                  = cfg_overlaySEIIdx[i];
      m_overlayLanguagePresentFlag[i]  = cfg_overlaySEILanguagePresentFlag[i];
      m_overlayContentLayerId[i]       = cfg_overlaySEIContentLayerId[i];
      m_overlayLabelPresentFlag[i]     = cfg_overlaySEILabelPresentFlag[i];
      m_overlayLabelLayerId[i]         = cfg_overlaySEILabelLayerId[i];
      m_overlayAlphaPresentFlag[i]     = cfg_overlaySEIAlphaPresentFlag[i];
      m_overlayAlphaLayerId[i]         = cfg_overlaySEIAlphaLayerId[i];
      m_numOverlayElementsMinus1[i]    = cfg_overlaySEINumElementsMinus1[i];
      m_overlayLanguage[i]             = cfg_overlaySEILanguage[i];
      m_overlayName[i]                 = cfg_overlaySEIName[i];
      
      //parse min and max values of label elements
      istringstream ranges(cfg_overlaySEIElementLabelRanges[i]);
      string range;          
      UInt val;        
      vector<UInt> vRanges;
      while ( getline(ranges, range, ' ') ) 
      {        
        istringstream(range) >> val;        
        vRanges.push_back(val);
      }      
      assert( vRanges.size()%2==0 );
      assert( vRanges.size()==2*(m_numOverlayElementsMinus1[i]+1) );
      m_overlayElementLabelMin[i].resize( m_numOverlayElementsMinus1[i]+1 );
      m_overlayElementLabelMax[i].resize( m_numOverlayElementsMinus1[i]+1 );
      for (Int j=0 ; j<=m_numOverlayElementsMinus1[i] ; j++)
      {
        m_overlayElementLabelMin[i][j] = vRanges[2*j];
        m_overlayElementLabelMax[i][j] = vRanges[2*j+1];
      }
      
      //parse overlay element names
      istringstream elementNames(cfg_overlaySEIElementNames[i]);
      string elementName;                
      vector<string> vElementName;                
      while ( getline(elementNames, elementName, '|') ) 
      {        
        vElementName.push_back(elementName);         
      }      
      if ( m_overlayLabelPresentFlag[i] )
      {
        m_overlayElementName[i].resize( m_numOverlayElementsMinus1[i]+1 );     
        for (Int j=0 ; j<=m_numOverlayElementsMinus1[i] ; j++)
        {
          if (j < vElementName.size())
          {
            m_overlayElementName[i][j] = vElementName[j];
          }
          else
          {
            m_overlayElementName[i][j] = string("NoElementName");
          }
        }
      }            
    }        
  }
#endif

#if N0383_IL_CONSTRAINED_TILE_SETS_SEI
  if (m_interLayerConstrainedTileSetsSEIEnabled)
  {
    if (m_numTileColumnsMinus1 == 0 && m_numTileRowsMinus1 == 0)
    {
      printf( "Tiles are not defined (needed for inter-layer comnstrained tile sets SEI).\n" );
      exit( EXIT_FAILURE );
    }
    TChar* pTileSets = m_tileSets.empty() ? NULL : strdup(m_tileSets.c_str());
    int i = 0;
    TChar *topLeftTileIndex = strtok(pTileSets, " ,");
    while(topLeftTileIndex != NULL)
    {
      if( i >= m_ilNumSetsInMessage )
      {
        printf( "The number of tile sets is larger than defined by IlNumSetsInMessage.\n" );
        exit( EXIT_FAILURE );
      }
      *( m_topLeftTileIndex + i ) = atoi( topLeftTileIndex );
      char *bottonRightTileIndex = strtok(NULL, " ,");
      if( bottonRightTileIndex == NULL )
      {
        printf( "BottonRightTileIndex is missing in the tile sets.\n" );
        exit( EXIT_FAILURE );
      }
      *( m_bottomRightTileIndex + i ) = atoi( bottonRightTileIndex );
      char *ilcIdc = strtok(NULL, " ,");
      if( ilcIdc == NULL )
      {
        printf( "IlcIdc is missing in the tile sets.\n" );
        exit( EXIT_FAILURE );
      }
      *( m_ilcIdc + i ) = atoi( ilcIdc );
      topLeftTileIndex = strtok(NULL, " ,");
      i++;
    }
    if( i < m_ilNumSetsInMessage )
    {
      printf( "The number of tile sets is smaller than defined by IlNumSetsInMessage.\n" );
      exit( EXIT_FAILURE );
    }
    m_skippedTileSetPresentFlag = false;

    if( pTileSets )
    {
      free( pTileSets );
      pTileSets = NULL;
    }
  }
#endif
  // check validity of input parameters
#if SVC_EXTENSION
#if VIEW_SCALABILITY
  if( m_scalabilityMask[VIEW_ORDER_INDEX] )
  {

    std::vector<Int> uniqueViewOrderIndices;
    std::vector<Int> uniqueViewIds;
    for( Int layer_idx = 0; layer_idx < m_numLayers; layer_idx++ )
    {
      Bool isIn = false;
      for( Int uni = 0; uni < uniqueViewOrderIndices.size(); uni++ )
      {
        isIn = isIn || ( *(cfg_viewOrderIndex[ layer_idx ]) == uniqueViewOrderIndices[ uni ] ); 
      }
      if ( !isIn ) 
      {
        uniqueViewOrderIndices.push_back( *(cfg_viewOrderIndex[ layer_idx ]) ); 
      } 

      isIn = false;
      for( Int uni = 0; uni < uniqueViewIds.size(); uni++ )
      {
        isIn = isIn || ( *(cfg_viewId[ layer_idx ]) == uniqueViewIds[ uni ] ); 
      }
      if( !isIn ) 
      {
        uniqueViewIds.push_back( *(cfg_viewId[ layer_idx ]) ); 
      } 

      assert(uniqueViewIds.size()==uniqueViewOrderIndices.size());

      m_iNumberOfViews=uniqueViewIds.size();

    }

    m_ViewIdVal = uniqueViewIds;
    m_iNumberOfViews = (Int)uniqueViewOrderIndices.size();
  }
#endif

  for( UInt layerIdx = 0; layerIdx < m_numLayers; layerIdx++ )
  {
    xCheckParameter(layerIdx);
  }
#else
  xCheckParameter();
#endif

  // compute actual CU depth with respect to config depth and max transform size
#if SVC_EXTENSION
  for(Int layerIdx = 0; layerIdx < m_numLayers; layerIdx++)
  {
    UInt uiAddCUDepth  = 0;
    while( (m_apcLayerCfg[layerIdx]->m_uiMaxCUWidth>>m_apcLayerCfg[layerIdx]->m_uiMaxCUDepth) > ( 1 << ( m_apcLayerCfg[layerIdx]->m_uiQuadtreeTULog2MinSize + uiAddCUDepth )  ) )
    {
      uiAddCUDepth++;
    }

    m_apcLayerCfg[layerIdx]->m_uiMaxTotalCUDepth = m_apcLayerCfg[layerIdx]->m_uiMaxCUDepth + uiAddCUDepth + getMaxCUDepthOffset(m_apcLayerCfg[layerIdx]->m_chromaFormatIDC, m_apcLayerCfg[layerIdx]->m_uiQuadtreeTULog2MinSize); // if minimum TU larger than 4x4, allow for additional part indices for 4:2:2 SubTUs.    
    m_apcLayerCfg[layerIdx]->m_uiLog2DiffMaxMinCodingBlockSize = m_apcLayerCfg[layerIdx]->m_uiMaxCUDepth - 1;
  }
#else  
  UInt uiAddCUDepth  = 0;
  while( (m_uiMaxCUWidth>>m_uiMaxCUDepth) > ( 1 << ( m_uiQuadtreeTULog2MinSize + uiAddCUDepth )  ) )
  {
    uiAddCUDepth++;
  }

  m_uiMaxTotalCUDepth = m_uiMaxCUDepth + uiAddCUDepth + getMaxCUDepthOffset(m_chromaFormatIDC, m_uiQuadtreeTULog2MinSize); // if minimum TU larger than 4x4, allow for additional part indices for 4:2:2 SubTUs.
  m_uiLog2DiffMaxMinCodingBlockSize = m_uiMaxCUDepth - 1;
#endif

  // print-out parameters
  xPrintParameter();

  return true;
}


// ====================================================================================================================
// Private member functions
// ====================================================================================================================

#if SVC_EXTENSION
Void TAppEncCfg::xCheckParameter(UInt layerIdx)
{
  ChromaFormat& m_chromaFormatIDC             = m_apcLayerCfg[layerIdx]->m_chromaFormatIDC;
  ChromaFormat& m_chromaFormatConstraint      = m_apcLayerCfg[layerIdx]->m_chromaFormatConstraint;
  ChromaFormat& m_InputChromaFormatIDC        = m_apcLayerCfg[layerIdx]->m_InputChromaFormatIDC;

  Int* m_inputBitDepth                        = m_apcLayerCfg[layerIdx]->m_inputBitDepth;
  Int* m_internalBitDepth                     = m_apcLayerCfg[layerIdx]->m_internalBitDepth;
  Int* m_MSBExtendedBitDepth                  = m_apcLayerCfg[layerIdx]->m_MSBExtendedBitDepth;

  Int& layerPTLIdx                            = m_apcLayerCfg[layerIdx]->m_layerPTLIdx;
  Profile::Name& m_profile                    = m_profileList[layerPTLIdx];
  UInt& m_bitDepthConstraint                  = m_apcLayerCfg[layerIdx]->m_bitDepthConstraint;
  Bool& m_intraConstraintFlag                 = m_apcLayerCfg[layerIdx]->m_intraConstraintFlag;
  Bool& m_lowerBitRateConstraintFlag          = m_apcLayerCfg[layerIdx]->m_lowerBitRateConstraintFlag;
  Bool& m_onePictureOnlyConstraintFlag        = m_apcLayerCfg[layerIdx]->m_onePictureOnlyConstraintFlag;

#if RC_SHVC_HARMONIZATION
  Bool& m_RCEnableRateControl                 = m_apcLayerCfg[layerIdx]->m_RCEnableRateControl;
  Bool& m_RCForceIntraQP                      = m_apcLayerCfg[layerIdx]->m_RCForceIntraQP;
  Int&  m_RCInitialQP                         = m_apcLayerCfg[layerIdx]->m_RCInitialQP;

#if U0132_TARGET_BITS_SATURATION
  Bool& m_RCCpbSaturationEnabled              = m_apcLayerCfg[layerIdx]->m_RCCpbSaturationEnabled;
  UInt& m_RCCpbSize                           = m_apcLayerCfg[layerIdx]->m_RCCpbSize;
  Double& m_RCInitialCpbFullness              = m_apcLayerCfg[layerIdx]->m_RCInitialCpbFullness;
  Level::Name& m_level                        = m_levelList[layerPTLIdx];
  Level::Tier& m_levelTier                    = m_levelTierList[layerPTLIdx];
#endif
#endif

  Int& m_iFrameRate                           = m_apcLayerCfg[layerIdx]->m_iFrameRate;
  Int& m_iQP                                  = m_apcLayerCfg[layerIdx]->m_iQP;
  Int& m_iSourceWidth                         = m_apcLayerCfg[layerIdx]->m_iSourceWidth;
  Int& m_iSourceHeight                        = m_apcLayerCfg[layerIdx]->m_iSourceHeight;
  Int* m_aiPad                                = m_apcLayerCfg[layerIdx]->m_aiPad;
  Int& m_iMaxCuDQPDepth                       = m_apcLayerCfg[layerIdx]->m_iMaxCuDQPDepth;
  UInt& m_uiMaxCUDepth                        = m_apcLayerCfg[layerIdx]->m_uiMaxCUDepth;
  UInt& m_uiQuadtreeTULog2MinSize             = m_apcLayerCfg[layerIdx]->m_uiQuadtreeTULog2MinSize;

  Int& m_iIntraPeriod                         = m_apcLayerCfg[layerIdx]->m_iIntraPeriod;
  UInt& m_uiMaxCUWidth                        = m_apcLayerCfg[layerIdx]->m_uiMaxCUWidth;
  UInt& m_uiMaxCUHeight                       = m_apcLayerCfg[layerIdx]->m_uiMaxCUHeight;
  UInt& m_uiQuadtreeTULog2MaxSize             = m_apcLayerCfg[layerIdx]->m_uiQuadtreeTULog2MaxSize;
  UInt& m_uiQuadtreeTUMaxDepthInter           = m_apcLayerCfg[layerIdx]->m_uiQuadtreeTUMaxDepthInter;
  UInt& m_uiQuadtreeTUMaxDepthIntra           = m_apcLayerCfg[layerIdx]->m_uiQuadtreeTUMaxDepthIntra;

  Bool& m_entropyCodingSyncEnabledFlag        = m_apcLayerCfg[layerIdx]->m_entropyCodingSyncEnabledFlag;

  Int& m_maxTempLayer                         = m_apcLayerCfg[layerIdx]->m_maxTempLayer;
  Int& m_extraRPSs                            = m_apcLayerCfg[layerIdx]->m_extraRPSs;
  GOPEntry* m_GOPList                         = m_apcLayerCfg[layerIdx]->m_GOPList;

#if PER_LAYER_LOSSLESS
  Bool& m_CUTransquantBypassFlagForce         = m_apcLayerCfg[layerIdx]->m_CUTransquantBypassFlagForce;
#endif

#if U0132_TARGET_BITS_SATURATION
  Int& m_RCTargetBitrate                      = m_apcLayerCfg[layerIdx]->m_RCTargetBitrate;
#endif
#else
Void TAppEncCfg::xCheckParameter()
{
#endif

  if (m_decodedPictureHashSEIType==HASHTYPE_NONE)
  {
    fprintf(stderr, "******************************************************************\n");
    fprintf(stderr, "** WARNING: --SEIDecodedPictureHash is now disabled by default. **\n");
    fprintf(stderr, "**          Automatic verification of decoded pictures by a     **\n");
    fprintf(stderr, "**          decoder requires this option to be enabled.         **\n");
    fprintf(stderr, "******************************************************************\n");
  }
#if SVC_EXTENSION
  Int ii = 0;
  while( ii < m_numPTLInfo )
  {
    if( m_profileList[ii] == Profile::NONE )
    {
      fprintf(stderr, "***************************************************************************\n");
      fprintf(stderr, "** WARNING: For conforming bitstreams a valid Profile value must be set! **\n");
      fprintf(stderr, "***************************************************************************\n");
    }
    if( m_levelList[ii] == Level::NONE )
    {
      fprintf(stderr, "***************************************************************************\n");
      fprintf(stderr, "** WARNING: For conforming bitstreams a valid Level value must be set!   **\n");
      fprintf(stderr, "***************************************************************************\n");
    }
    ii++;
  }
#if AVC_BASE
  if( m_numLayers > 1 && m_numPTLInfo > 1 && !m_nonHEVCBaseLayerFlag )
#else
  if( m_numLayers > 1 && m_numPTLInfo > 1 )
#endif
  {
    assert(m_profileList[0] <= Profile::MULTIVIEWMAIN);  //Profile IDC of PTL in VPS shall be one of single-layer profile IDCs
    assert(m_profileList[0] == m_profileList[1]);        //Profile IDC of VpsProfileTierLevel[ 0 ] and VpsProfileTierLevel[ 1 ] shall be the same when BL is HEVC compatible
    assert(m_levelList[0] >= m_levelList[1]);            //Level IDC of VpsProfileTierLevel[ 0 ] should not be less than level IDC of VpsProfileTierLevel[ 1 ]. 
                                                         //NOTE that this is not conformance constraint but it would be nice if our encoder can prevent inefficient level IDC assignment
    if (m_levelList[0] == m_levelList[1])
    {
      printf("Warning: Level0 is set the same as Level1\n");
    }
  }
#else
  if( m_profile==Profile::NONE )
  {
    fprintf(stderr, "***************************************************************************\n");
    fprintf(stderr, "** WARNING: For conforming bitstreams a valid Profile value must be set! **\n");
    fprintf(stderr, "***************************************************************************\n");
  }
  if( m_level==Level::NONE )
  {
    fprintf(stderr, "***************************************************************************\n");
    fprintf(stderr, "** WARNING: For conforming bitstreams a valid Level value must be set!   **\n");
    fprintf(stderr, "***************************************************************************\n");
  }
#endif

  Bool check_failed = false; /* abort if there is a fatal configuration problem */
#define xConfirmPara(a,b) check_failed |= confirmPara(a,b)

  xConfirmPara(m_bitstreamFileName.empty(), "A bitstream file name must be specified (BitstreamFile)");
  const UInt maxBitDepth=(m_chromaFormatIDC==CHROMA_400) ? m_internalBitDepth[CHANNEL_TYPE_LUMA] : std::max(m_internalBitDepth[CHANNEL_TYPE_LUMA], m_internalBitDepth[CHANNEL_TYPE_CHROMA]);
  xConfirmPara(m_bitDepthConstraint<maxBitDepth, "The internalBitDepth must not be greater than the bitDepthConstraint value");
  xConfirmPara(m_chromaFormatConstraint<m_chromaFormatIDC, "The chroma format used must not be greater than the chromaFormatConstraint value");

#if SCALABLE_REXT
  if (m_profile==Profile::MAINREXT || m_profile==Profile::HIGHTHROUGHPUTREXT || m_profile==Profile::SCALABLEREXT)
#else
  if (m_profile==Profile::MAINREXT || m_profile==Profile::HIGHTHROUGHPUTREXT)
#endif
  {
    xConfirmPara(m_lowerBitRateConstraintFlag==false && m_intraConstraintFlag==false, "The lowerBitRateConstraint flag cannot be false when intraConstraintFlag is false");
    xConfirmPara(m_cabacBypassAlignmentEnabledFlag && m_profile!=Profile::HIGHTHROUGHPUTREXT, "AlignCABACBeforeBypass must not be enabled unless the high throughput profile is being used.");
#if SCALABLE_REXT
    if (m_profile == Profile::MAINREXT || m_profile==Profile::SCALABLEREXT)
#else
    if (m_profile == Profile::MAINREXT)
#endif
    {
      const UInt intraIdx = m_intraConstraintFlag ? 1:0;
      const UInt bitDepthIdx = (m_bitDepthConstraint == 8 ? 0 : (m_bitDepthConstraint ==10 ? 1 : (m_bitDepthConstraint == 12 ? 2 : (m_bitDepthConstraint == 16 ? 3 : 4 ))));
      const UInt chromaFormatIdx = UInt(m_chromaFormatConstraint);
      const Bool bValidProfile = (bitDepthIdx > 3 || chromaFormatIdx>3) ? false : (validRExtProfileNames[intraIdx][bitDepthIdx][chromaFormatIdx] != NONE);
      xConfirmPara(!bValidProfile, "Invalid intra constraint flag, bit depth constraint flag and chroma format constraint flag combination for a RExt profile");
      const Bool bUsingGeneralRExtTools  = m_transformSkipRotationEnabledFlag        ||
                                           m_transformSkipContextEnabledFlag         ||
                                           m_rdpcmEnabledFlag[RDPCM_SIGNAL_IMPLICIT] ||
                                           m_rdpcmEnabledFlag[RDPCM_SIGNAL_EXPLICIT] ||
                                           !m_enableIntraReferenceSmoothing          ||
                                           m_persistentRiceAdaptationEnabledFlag     ||
                                           m_log2MaxTransformSkipBlockSize!=2;
      const Bool bUsingChromaQPTool      = m_diffCuChromaQpOffsetDepth >= 0;
      const Bool bUsingExtendedPrecision = m_extendedPrecisionProcessingFlag;

      xConfirmPara((m_chromaFormatConstraint==CHROMA_420 || m_chromaFormatConstraint==CHROMA_400) && bUsingChromaQPTool, "CU Chroma QP adjustment cannot be used for 4:0:0 or 4:2:0 RExt profiles");
      xConfirmPara(m_bitDepthConstraint != 16 && bUsingExtendedPrecision, "Extended precision can only be used in 16-bit RExt profiles");
      if (!(m_chromaFormatConstraint == CHROMA_400 && m_bitDepthConstraint == 16) && m_chromaFormatConstraint!=CHROMA_444)
      {
        xConfirmPara(bUsingGeneralRExtTools, "Combination of tools and profiles are not possible in the specified RExt profile.");
      }
      xConfirmPara( m_onePictureOnlyConstraintFlag && m_chromaFormatConstraint!=CHROMA_444, "chroma format constraint must be 4:4:4 when one-picture-only constraint flag is 1");
      xConfirmPara( m_onePictureOnlyConstraintFlag && m_bitDepthConstraint != 8 && m_bitDepthConstraint != 16, "bit depth constraint must be 8 or 16 when one-picture-only constraint flag is 1");
      xConfirmPara( m_onePictureOnlyConstraintFlag && m_framesToBeEncoded > 1, "Number of frames to be encoded must be 1 when one-picture-only constraint flag is 1.");

      if (!m_intraConstraintFlag && m_bitDepthConstraint==16 && m_chromaFormatConstraint==CHROMA_444)
      {
        fprintf(stderr, "********************************************************************************************************\n");
        fprintf(stderr, "** WARNING: The RExt constraint flags describe a non standard combination (used for development only) **\n");
        fprintf(stderr, "********************************************************************************************************\n");
      }
    }
    else
    {
      xConfirmPara( m_chromaFormatConstraint != CHROMA_444, "chroma format constraint must be 4:4:4 in the High Throughput 4:4:4 16-bit Intra profile.");
      xConfirmPara( m_bitDepthConstraint     != 16,         "bit depth constraint must be 4:4:4 in the High Throughput 4:4:4 16-bit Intra profile.");
      xConfirmPara( m_intraConstraintFlag    != 1,          "intra constraint flag must be 1 in the High Throughput 4:4:4 16-bit Intra profile.");
    }
  }
  else
  {
#if SVC_EXTENSION
    xConfirmPara(m_bitDepthConstraint!=((m_profile==Profile::MAIN10 || m_profile==Profile::SCALABLEMAIN10)?10:8), "BitDepthConstraint must be 8 for MAIN profile and 10 for MAIN10 or Scalable-main10 profile.");
#else
    xConfirmPara(m_bitDepthConstraint!=((m_profile==Profile::MAIN10)?10:8), "BitDepthConstraint must be 8 for MAIN profile and 10 for MAIN10 profile.");
#endif
#if SCALABLE_REXT // changes below are only about displayed text
    xConfirmPara(m_chromaFormatConstraint!=CHROMA_420, "ChromaFormatConstraint must be 420 for non main-RExt and non scalable-Rext profiles.");
    xConfirmPara(m_intraConstraintFlag==true, "IntraConstraintFlag must be false for non main_RExt and non scalable-Rext profiles.");
    xConfirmPara(m_lowerBitRateConstraintFlag==false, "LowerBitrateConstraintFlag must be true for non main-RExt and non scalable-Rext profiles.");
    xConfirmPara(m_profile == Profile::MAINSTILLPICTURE && m_framesToBeEncoded > 1, "Number of frames to be encoded must be 1 when main still picture profile is used.");

    xConfirmPara(m_crossComponentPredictionEnabledFlag==true, "CrossComponentPrediction must not be used for non main-RExt and non scalable-Rext profiles.");
    xConfirmPara(m_log2MaxTransformSkipBlockSize!=2, "Transform Skip Log2 Max Size must be 2 for V1 profiles.");
    xConfirmPara(m_transformSkipRotationEnabledFlag==true, "UseResidualRotation must not be enabled for non main-RExt and non scalable-Rext profiles.");
    xConfirmPara(m_transformSkipContextEnabledFlag==true, "UseSingleSignificanceMapContext must not be enabled for non main-RExt nand on scalable-Rext profiles.");
    xConfirmPara(m_rdpcmEnabledFlag[RDPCM_SIGNAL_IMPLICIT]==true, "ImplicitResidualDPCM must not be enabled for non main-RExt and non scalable-Rext profiles.");
    xConfirmPara(m_rdpcmEnabledFlag[RDPCM_SIGNAL_EXPLICIT]==true, "ExplicitResidualDPCM must not be enabled for non main-RExt and non scalable-Rext profiles.");
    xConfirmPara(m_persistentRiceAdaptationEnabledFlag==true, "GolombRiceParameterAdaption must not be enabled for non main-RExt and non scalable-Rext profiles.");
    xConfirmPara(m_extendedPrecisionProcessingFlag==true, "UseExtendedPrecision must not be enabled for non main-RExt and non scalable-Rext profiles.");
    xConfirmPara(m_highPrecisionOffsetsEnabledFlag==true, "UseHighPrecisionPredictionWeighting must not be enabled for non main-RExt and non scalable-Rext profiles.");
    xConfirmPara(m_enableIntraReferenceSmoothing==false, "EnableIntraReferenceSmoothing must be enabled for non main-RExt and non scalable-Rext profiles.");
    xConfirmPara(m_cabacBypassAlignmentEnabledFlag, "AlignCABACBeforeBypass cannot be enabled for non main-RExt and non scalable-Rext profiles.");
#else
    xConfirmPara(m_chromaFormatConstraint!=CHROMA_420, "ChromaFormatConstraint must be 420 for non main-RExt profiles.");
    xConfirmPara(m_intraConstraintFlag==true, "IntraConstraintFlag must be false for non main_RExt profiles.");
    xConfirmPara(m_lowerBitRateConstraintFlag==false, "LowerBitrateConstraintFlag must be true for non main-RExt profiles.");
    xConfirmPara(m_profile == Profile::MAINSTILLPICTURE && m_framesToBeEncoded > 1, "Number of frames to be encoded must be 1 when main still picture profile is used.");

    xConfirmPara(m_crossComponentPredictionEnabledFlag==true, "CrossComponentPrediction must not be used for non main-RExt profiles.");
    xConfirmPara(m_log2MaxTransformSkipBlockSize!=2, "Transform Skip Log2 Max Size must be 2 for V1 profiles.");
    xConfirmPara(m_transformSkipRotationEnabledFlag==true, "UseResidualRotation must not be enabled for non main-RExt profiles.");
    xConfirmPara(m_transformSkipContextEnabledFlag==true, "UseSingleSignificanceMapContext must not be enabled for non main-RExt profiles.");
    xConfirmPara(m_rdpcmEnabledFlag[RDPCM_SIGNAL_IMPLICIT]==true, "ImplicitResidualDPCM must not be enabled for non main-RExt profiles.");
    xConfirmPara(m_rdpcmEnabledFlag[RDPCM_SIGNAL_EXPLICIT]==true, "ExplicitResidualDPCM must not be enabled for non main-RExt profiles.");
    xConfirmPara(m_persistentRiceAdaptationEnabledFlag==true, "GolombRiceParameterAdaption must not be enabled for non main-RExt profiles.");
    xConfirmPara(m_extendedPrecisionProcessingFlag==true, "UseExtendedPrecision must not be enabled for non main-RExt profiles.");
    xConfirmPara(m_highPrecisionOffsetsEnabledFlag==true, "UseHighPrecisionPredictionWeighting must not be enabled for non main-RExt profiles.");
    xConfirmPara(m_enableIntraReferenceSmoothing==false, "EnableIntraReferenceSmoothing must be enabled for non main-RExt profiles.");
    xConfirmPara(m_cabacBypassAlignmentEnabledFlag, "AlignCABACBeforeBypass cannot be enabled for non main-RExt profiles.");
#endif
  }

  // check range of parameters
  xConfirmPara( m_inputBitDepth[CHANNEL_TYPE_LUMA  ] < 8,                                   "InputBitDepth must be at least 8" );
  xConfirmPara( m_inputBitDepth[CHANNEL_TYPE_CHROMA] < 8,                                   "InputBitDepthC must be at least 8" );

#if !RExt__HIGH_BIT_DEPTH_SUPPORT
  if (m_extendedPrecisionProcessingFlag)
  {
    for (UInt channelType = 0; channelType < MAX_NUM_CHANNEL_TYPE; channelType++)
    {
      xConfirmPara((m_internalBitDepth[channelType] > 8) , "Model is not configured to support high enough internal accuracies - enable RExt__HIGH_BIT_DEPTH_SUPPORT to use increased precision internal data types etc...");
    }
  }
  else
  {
    for (UInt channelType = 0; channelType < MAX_NUM_CHANNEL_TYPE; channelType++)
    {
      xConfirmPara((m_internalBitDepth[channelType] > 12) , "Model is not configured to support high enough internal accuracies - enable RExt__HIGH_BIT_DEPTH_SUPPORT to use increased precision internal data types etc...");
    }
  }
#endif

  xConfirmPara( (m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA  ] < m_inputBitDepth[CHANNEL_TYPE_LUMA  ]), "MSB-extended bit depth for luma channel (--MSBExtendedBitDepth) must be greater than or equal to input bit depth for luma channel (--InputBitDepth)" );
  xConfirmPara( (m_MSBExtendedBitDepth[CHANNEL_TYPE_CHROMA] < m_inputBitDepth[CHANNEL_TYPE_CHROMA]), "MSB-extended bit depth for chroma channel (--MSBExtendedBitDepthC) must be greater than or equal to input bit depth for chroma channel (--InputBitDepthC)" );

  xConfirmPara( m_log2SaoOffsetScale[CHANNEL_TYPE_LUMA]   > (m_internalBitDepth[CHANNEL_TYPE_LUMA  ]<10?0:(m_internalBitDepth[CHANNEL_TYPE_LUMA  ]-10)), "SaoLumaOffsetBitShift must be in the range of 0 to InternalBitDepth-10, inclusive");
  xConfirmPara( m_log2SaoOffsetScale[CHANNEL_TYPE_CHROMA] > (m_internalBitDepth[CHANNEL_TYPE_CHROMA]<10?0:(m_internalBitDepth[CHANNEL_TYPE_CHROMA]-10)), "SaoChromaOffsetBitShift must be in the range of 0 to InternalBitDepthC-10, inclusive");

  xConfirmPara( m_chromaFormatIDC >= NUM_CHROMA_FORMAT,                                     "ChromaFormatIDC must be either 400, 420, 422 or 444" );
  std::string sTempIPCSC="InputColourSpaceConvert must be empty, "+getListOfColourSpaceConverts(true);
  xConfirmPara( m_inputColourSpaceConvert >= NUMBER_INPUT_COLOUR_SPACE_CONVERSIONS,         sTempIPCSC.c_str() );
  xConfirmPara( m_InputChromaFormatIDC >= NUM_CHROMA_FORMAT,                                "InputChromaFormatIDC must be either 400, 420, 422 or 444" );
  xConfirmPara( m_iFrameRate <= 0,                                                          "Frame rate must be more than 1" );
  xConfirmPara( m_temporalSubsampleRatio < 1,                                               "Temporal subsample rate must be no less than 1" );
  xConfirmPara( m_framesToBeEncoded <= 0,                                                   "Total Number Of Frames encoded must be more than 0" );
  xConfirmPara( m_iGOPSize < 1 ,                                                            "GOP Size must be greater or equal to 1" );
  xConfirmPara( m_iGOPSize > 1 &&  m_iGOPSize % 2,                                          "GOP Size must be a multiple of 2, if GOP Size is greater than 1" );
  xConfirmPara( (m_iIntraPeriod > 0 && m_iIntraPeriod < m_iGOPSize) || m_iIntraPeriod == 0, "Intra period must be more than GOP size, or -1 , not 0" );
  xConfirmPara( m_iDecodingRefreshType < 0 || m_iDecodingRefreshType > 3,                   "Decoding Refresh Type must be comprised between 0 and 3 included" );
  if(m_iDecodingRefreshType == 3)
  {
    xConfirmPara( !m_recoveryPointSEIEnabled,                                               "When using RecoveryPointSEI messages as RA points, recoveryPointSEI must be enabled" );
  }

  if (m_isField)
  {
    if (!m_pictureTimingSEIEnabled)
    {
      fprintf(stderr, "****************************************************************************\n");
      fprintf(stderr, "** WARNING: Picture Timing SEI should be enabled for field coding!        **\n");
      fprintf(stderr, "****************************************************************************\n");
    }
  }

  if(m_crossComponentPredictionEnabledFlag && (m_chromaFormatIDC != CHROMA_444))
  {
    fprintf(stderr, "****************************************************************************\n");
    fprintf(stderr, "** WARNING: Cross-component prediction is specified for 4:4:4 format only **\n");
    fprintf(stderr, "****************************************************************************\n");

    m_crossComponentPredictionEnabledFlag = false;
  }

  if ( m_CUTransquantBypassFlagForce && m_bUseHADME )
  {
    fprintf(stderr, "****************************************************************************\n");
    fprintf(stderr, "** WARNING: --HadamardME has been disabled due to the enabling of         **\n");
    fprintf(stderr, "**          --CUTransquantBypassFlagForce                                 **\n");
    fprintf(stderr, "****************************************************************************\n");

    m_bUseHADME = false; // this has been disabled so that the lambda is calculated slightly differently for lossless modes (as a result of JCTVC-R0104).
  }

  xConfirmPara (m_log2MaxTransformSkipBlockSize < 2, "Transform Skip Log2 Max Size must be at least 2 (4x4)");

  if (m_log2MaxTransformSkipBlockSize!=2 && m_useTransformSkipFast)
  {
    fprintf(stderr, "***************************************************************************\n");
    fprintf(stderr, "** WARNING: Transform skip fast is enabled (which only tests NxN splits),**\n");
    fprintf(stderr, "**          but transform skip log2 max size is not 2 (4x4)              **\n");
    fprintf(stderr, "**          It may be better to disable transform skip fast mode         **\n");
    fprintf(stderr, "***************************************************************************\n");
  }

  xConfirmPara( m_iQP <  -6 * (m_internalBitDepth[CHANNEL_TYPE_LUMA] - 8) || m_iQP > 51,    "QP exceeds supported range (-QpBDOffsety to 51)" );
#if W0038_DB_OPT
  xConfirmPara( m_deblockingFilterMetric!=0 && (m_bLoopFilterDisable || m_loopFilterOffsetInPPS), "If DeblockingFilterMetric is non-zero then both LoopFilterDisable and LoopFilterOffsetInPPS must be 0");
#else
  xConfirmPara( m_DeblockingFilterMetric && (m_bLoopFilterDisable || m_loopFilterOffsetInPPS), "If DeblockingFilterMetric is true then both LoopFilterDisable and LoopFilterOffsetInPPS must be 0");
#endif
  xConfirmPara( m_loopFilterBetaOffsetDiv2 < -6 || m_loopFilterBetaOffsetDiv2 > 6,        "Loop Filter Beta Offset div. 2 exceeds supported range (-6 to 6)");
  xConfirmPara( m_loopFilterTcOffsetDiv2 < -6 || m_loopFilterTcOffsetDiv2 > 6,            "Loop Filter Tc Offset div. 2 exceeds supported range (-6 to 6)");
  xConfirmPara( m_iSearchRange < 0 ,                                                        "Search Range must be more than 0" );
  xConfirmPara( m_bipredSearchRange < 0 ,                                                   "Bi-prediction refinement search range must be more than 0" );
  xConfirmPara( m_minSearchWindow < 0,                                                      "Minimum motion search window size for the adaptive window ME must be greater than or equal to 0" );
  xConfirmPara( m_iMaxDeltaQP > 7,                                                          "Absolute Delta QP exceeds supported range (0 to 7)" );
  xConfirmPara( m_iMaxCuDQPDepth > m_uiMaxCUDepth - 1,                                          "Absolute depth for a minimum CuDQP exceeds maximum coding unit depth" );

  xConfirmPara( m_cbQpOffset < -12,   "Min. Chroma Cb QP Offset is -12" );
  xConfirmPara( m_cbQpOffset >  12,   "Max. Chroma Cb QP Offset is  12" );
  xConfirmPara( m_crQpOffset < -12,   "Min. Chroma Cr QP Offset is -12" );
  xConfirmPara( m_crQpOffset >  12,   "Max. Chroma Cr QP Offset is  12" );

  xConfirmPara( m_iQPAdaptationRange <= 0,                                                  "QP Adaptation Range must be more than 0" );
  if (m_iDecodingRefreshType == 2)
  {
    xConfirmPara( m_iIntraPeriod > 0 && m_iIntraPeriod <= m_iGOPSize ,                      "Intra period must be larger than GOP size for periodic IDR pictures");
  }
#if OPTIONAL_RESET_SAO_ENCODING_AFTER_IRAP
  if (m_saoResetEncoderStateAfterIRAP)
  {
    xConfirmPara( m_iIntraPeriod > 0 && m_iIntraPeriod <= m_iGOPSize ,                      "Intra period must be larger than GOP size when SAOResetEncoderStateAfterIRAP is enabled");
  }
#endif
  xConfirmPara( m_uiMaxCUDepth < 1,                                                         "MaxPartitionDepth must be greater than zero");
  xConfirmPara( (m_uiMaxCUWidth  >> m_uiMaxCUDepth) < 4,                                    "Minimum partition width size should be larger than or equal to 8");
  xConfirmPara( (m_uiMaxCUHeight >> m_uiMaxCUDepth) < 4,                                    "Minimum partition height size should be larger than or equal to 8");
  xConfirmPara( m_uiMaxCUWidth < 16,                                                        "Maximum partition width size should be larger than or equal to 16");
  xConfirmPara( m_uiMaxCUHeight < 16,                                                       "Maximum partition height size should be larger than or equal to 16");
  xConfirmPara( (m_iSourceWidth  % (m_uiMaxCUWidth  >> (m_uiMaxCUDepth-1)))!=0,             "Resulting coded frame width must be a multiple of the minimum CU size");
  xConfirmPara( (m_iSourceHeight % (m_uiMaxCUHeight >> (m_uiMaxCUDepth-1)))!=0,             "Resulting coded frame height must be a multiple of the minimum CU size");

  xConfirmPara( m_uiQuadtreeTULog2MinSize < 2,                                        "QuadtreeTULog2MinSize must be 2 or greater.");
  xConfirmPara( m_uiQuadtreeTULog2MaxSize > 5,                                        "QuadtreeTULog2MaxSize must be 5 or smaller.");
  xConfirmPara( m_uiQuadtreeTULog2MaxSize < m_uiQuadtreeTULog2MinSize,                "QuadtreeTULog2MaxSize must be greater than or equal to m_uiQuadtreeTULog2MinSize.");
  xConfirmPara( (1<<m_uiQuadtreeTULog2MaxSize) > m_uiMaxCUWidth,                      "QuadtreeTULog2MaxSize must be log2(maxCUSize) or smaller.");
  xConfirmPara( ( 1 << m_uiQuadtreeTULog2MinSize ) >= ( m_uiMaxCUWidth  >> (m_uiMaxCUDepth-1)), "QuadtreeTULog2MinSize must not be greater than or equal to minimum CU size" );
  xConfirmPara( ( 1 << m_uiQuadtreeTULog2MinSize ) >= ( m_uiMaxCUHeight >> (m_uiMaxCUDepth-1)), "QuadtreeTULog2MinSize must not be greater than or equal to minimum CU size" );
  xConfirmPara( m_uiQuadtreeTUMaxDepthInter < 1,                                                         "QuadtreeTUMaxDepthInter must be greater than or equal to 1" );
  xConfirmPara( m_uiMaxCUWidth < ( 1 << (m_uiQuadtreeTULog2MinSize + m_uiQuadtreeTUMaxDepthInter - 1) ), "QuadtreeTUMaxDepthInter must be less than or equal to the difference between log2(maxCUSize) and QuadtreeTULog2MinSize plus 1" );
  xConfirmPara( m_uiQuadtreeTUMaxDepthIntra < 1,                                                         "QuadtreeTUMaxDepthIntra must be greater than or equal to 1" );
  xConfirmPara( m_uiMaxCUWidth < ( 1 << (m_uiQuadtreeTULog2MinSize + m_uiQuadtreeTUMaxDepthIntra - 1) ), "QuadtreeTUMaxDepthInter must be less than or equal to the difference between log2(maxCUSize) and QuadtreeTULog2MinSize plus 1" );

  xConfirmPara(  m_maxNumMergeCand < 1,  "MaxNumMergeCand must be 1 or greater.");
  xConfirmPara(  m_maxNumMergeCand > 5,  "MaxNumMergeCand must be 5 or smaller.");

#if ADAPTIVE_QP_SELECTION
  xConfirmPara( m_bUseAdaptQpSelect == true && m_iQP < 0,                                              "AdaptiveQpSelection must be disabled when QP < 0.");
  xConfirmPara( m_bUseAdaptQpSelect == true && (m_cbQpOffset !=0 || m_crQpOffset != 0 ),               "AdaptiveQpSelection must be disabled when ChromaQpOffset is not equal to 0.");
#endif

  if( m_usePCM)
  {
    for (UInt channelType = 0; channelType < MAX_NUM_CHANNEL_TYPE; channelType++)
    {
      xConfirmPara(((m_MSBExtendedBitDepth[channelType] > m_internalBitDepth[channelType]) && m_bPCMInputBitDepthFlag), "PCM bit depth cannot be greater than internal bit depth (PCMInputBitDepthFlag cannot be used when InputBitDepth or MSBExtendedBitDepth > InternalBitDepth)");
    }
    xConfirmPara(  m_uiPCMLog2MinSize < 3,                                      "PCMLog2MinSize must be 3 or greater.");
    xConfirmPara(  m_uiPCMLog2MinSize > 5,                                      "PCMLog2MinSize must be 5 or smaller.");
    xConfirmPara(  m_pcmLog2MaxSize > 5,                                        "PCMLog2MaxSize must be 5 or smaller.");
    xConfirmPara(  m_pcmLog2MaxSize < m_uiPCMLog2MinSize,                       "PCMLog2MaxSize must be equal to or greater than m_uiPCMLog2MinSize.");
  }

  if (m_sliceMode!=NO_SLICES)
  {
    xConfirmPara( m_sliceArgument < 1 ,         "SliceArgument should be larger than or equal to 1" );
  }
  if (m_sliceSegmentMode!=NO_SLICES)
  {
    xConfirmPara( m_sliceSegmentArgument < 1 ,         "SliceSegmentArgument should be larger than or equal to 1" );
  }

  Bool tileFlag = (m_numTileColumnsMinus1 > 0 || m_numTileRowsMinus1 > 0 );
  if (m_profile!=Profile::HIGHTHROUGHPUTREXT)
  {
    xConfirmPara( tileFlag && m_entropyCodingSyncEnabledFlag, "Tiles and entropy-coding-sync (Wavefronts) can not be applied together, except in the High Throughput Intra 4:4:4 16 profile");
  }

  xConfirmPara( m_iSourceWidth  % TComSPS::getWinUnitX(m_chromaFormatIDC) != 0, "Picture width must be an integer multiple of the specified chroma subsampling");
  xConfirmPara( m_iSourceHeight % TComSPS::getWinUnitY(m_chromaFormatIDC) != 0, "Picture height must be an integer multiple of the specified chroma subsampling");

  xConfirmPara( m_aiPad[0] % TComSPS::getWinUnitX(m_chromaFormatIDC) != 0, "Horizontal padding must be an integer multiple of the specified chroma subsampling");
  xConfirmPara( m_aiPad[1] % TComSPS::getWinUnitY(m_chromaFormatIDC) != 0, "Vertical padding must be an integer multiple of the specified chroma subsampling");

#if !SVC_EXTENSION
  xConfirmPara( m_confWinLeft   % TComSPS::getWinUnitX(m_chromaFormatIDC) != 0, "Left conformance window offset must be an integer multiple of the specified chroma subsampling");
  xConfirmPara( m_confWinRight  % TComSPS::getWinUnitX(m_chromaFormatIDC) != 0, "Right conformance window offset must be an integer multiple of the specified chroma subsampling");
  xConfirmPara( m_confWinTop    % TComSPS::getWinUnitY(m_chromaFormatIDC) != 0, "Top conformance window offset must be an integer multiple of the specified chroma subsampling");
  xConfirmPara( m_confWinBottom % TComSPS::getWinUnitY(m_chromaFormatIDC) != 0, "Bottom conformance window offset must be an integer multiple of the specified chroma subsampling");
#endif

  xConfirmPara( m_defaultDisplayWindowFlag && !m_vuiParametersPresentFlag, "VUI needs to be enabled for default display window");

  if (m_defaultDisplayWindowFlag)
  {
    xConfirmPara( m_defDispWinLeftOffset   % TComSPS::getWinUnitX(m_chromaFormatIDC) != 0, "Left default display window offset must be an integer multiple of the specified chroma subsampling");
    xConfirmPara( m_defDispWinRightOffset  % TComSPS::getWinUnitX(m_chromaFormatIDC) != 0, "Right default display window offset must be an integer multiple of the specified chroma subsampling");
    xConfirmPara( m_defDispWinTopOffset    % TComSPS::getWinUnitY(m_chromaFormatIDC) != 0, "Top default display window offset must be an integer multiple of the specified chroma subsampling");
    xConfirmPara( m_defDispWinBottomOffset % TComSPS::getWinUnitY(m_chromaFormatIDC) != 0, "Bottom default display window offset must be an integer multiple of the specified chroma subsampling");
  }

  // max CU width and height should be power of 2
  UInt ui = m_uiMaxCUWidth;
  while(ui)
  {
    ui >>= 1;
    if( (ui & 1) == 1)
    {
      xConfirmPara( ui != 1 , "Width should be 2^n");
    }
  }
  ui = m_uiMaxCUHeight;
  while(ui)
  {
    ui >>= 1;
    if( (ui & 1) == 1)
    {
      xConfirmPara( ui != 1 , "Height should be 2^n");
    }
  }

  /* if this is an intra-only sequence, ie IntraPeriod=1, don't verify the GOP structure
   * This permits the ability to omit a GOP structure specification */
  if (m_iIntraPeriod == 1 && m_GOPList[0].m_POC == -1)
  {
    m_GOPList[0] = GOPEntry();
    m_GOPList[0].m_QPFactor = 1;
    m_GOPList[0].m_betaOffsetDiv2 = 0;
    m_GOPList[0].m_tcOffsetDiv2 = 0;
    m_GOPList[0].m_POC = 1;
    m_GOPList[0].m_numRefPicsActive = 4;
  }
  else
  {
    xConfirmPara( m_intraConstraintFlag, "IntraConstraintFlag cannot be 1 for inter sequences");
  }

  Bool verifiedGOP=false;
  Bool errorGOP=false;
  Int checkGOP=1;
  Int numRefs = m_isField ? 2 : 1;
  Int refList[MAX_NUM_REF_PICS+1];
  refList[0]=0;
  if(m_isField)
  {
    refList[1] = 1;
  }
  Bool isOK[MAX_GOP];
  for(Int i=0; i<MAX_GOP; i++)
  {
    isOK[i]=false;
  }
  Int numOK=0;
  xConfirmPara( m_iIntraPeriod >=0&&(m_iIntraPeriod%m_iGOPSize!=0), "Intra period must be a multiple of GOPSize, or -1" );

  for(Int i=0; i<m_iGOPSize; i++)
  {
    if(m_GOPList[i].m_POC==m_iGOPSize)
    {
      xConfirmPara( m_GOPList[i].m_temporalId!=0 , "The last frame in each GOP must have temporal ID = 0 " );
    }
  }

#if SVC_EXTENSION
  xConfirmPara( m_numLayers > MAX_LAYERS , "Number of layers in config file is greater than MAX_LAYERS" );
  m_numLayers = m_numLayers > MAX_LAYERS ? MAX_LAYERS : m_numLayers;
  
#if AVC_BASE
  if( m_nonHEVCBaseLayerFlag )
  {
    m_crossLayerIrapAlignFlag = false;
    m_crossLayerPictureTypeAlignFlag = false;
    m_crossLayerAlignedIdrOnlyFlag = false;
  }
#endif

  // verify layer configuration parameters
#endif
  if ( (m_iIntraPeriod != 1) && !m_loopFilterOffsetInPPS && (!m_bLoopFilterDisable) )
  {
    for(Int i=0; i<m_iGOPSize; i++)
    {
      xConfirmPara( (m_GOPList[i].m_betaOffsetDiv2 + m_loopFilterBetaOffsetDiv2) < -6 || (m_GOPList[i].m_betaOffsetDiv2 + m_loopFilterBetaOffsetDiv2) > 6, "Loop Filter Beta Offset div. 2 for one of the GOP entries exceeds supported range (-6 to 6)" );
      xConfirmPara( (m_GOPList[i].m_tcOffsetDiv2 + m_loopFilterTcOffsetDiv2) < -6 || (m_GOPList[i].m_tcOffsetDiv2 + m_loopFilterTcOffsetDiv2) > 6, "Loop Filter Tc Offset div. 2 for one of the GOP entries exceeds supported range (-6 to 6)" );
    }
  }

#if W0038_CQP_ADJ
  for(Int i=0; i<m_iGOPSize; i++)
  {
    xConfirmPara( abs(m_GOPList[i].m_CbQPoffset               ) > 12, "Cb QP Offset for one of the GOP entries exceeds supported range (-12 to 12)" );
    xConfirmPara( abs(m_GOPList[i].m_CbQPoffset + m_cbQpOffset) > 12, "Cb QP Offset for one of the GOP entries, when combined with the PPS Cb offset, exceeds supported range (-12 to 12)" );
    xConfirmPara( abs(m_GOPList[i].m_CrQPoffset               ) > 12, "Cr QP Offset for one of the GOP entries exceeds supported range (-12 to 12)" );
    xConfirmPara( abs(m_GOPList[i].m_CrQPoffset + m_crQpOffset) > 12, "Cr QP Offset for one of the GOP entries, when combined with the PPS Cr offset, exceeds supported range (-12 to 12)" );
  }
  xConfirmPara( abs(m_sliceChromaQpOffsetIntraOrPeriodic[0]                 ) > 12, "Intra/periodic Cb QP Offset exceeds supported range (-12 to 12)" );
  xConfirmPara( abs(m_sliceChromaQpOffsetIntraOrPeriodic[0]  + m_cbQpOffset ) > 12, "Intra/periodic Cb QP Offset, when combined with the PPS Cb offset, exceeds supported range (-12 to 12)" );
  xConfirmPara( abs(m_sliceChromaQpOffsetIntraOrPeriodic[1]                 ) > 12, "Intra/periodic Cr QP Offset exceeds supported range (-12 to 12)" );
  xConfirmPara( abs(m_sliceChromaQpOffsetIntraOrPeriodic[1]  + m_crQpOffset ) > 12, "Intra/periodic Cr QP Offset, when combined with the PPS Cr offset, exceeds supported range (-12 to 12)" );
#endif

  m_extraRPSs=0;
  //start looping through frames in coding order until we can verify that the GOP structure is correct.
  while(!verifiedGOP&&!errorGOP)
  {
    Int curGOP = (checkGOP-1)%m_iGOPSize;
    Int curPOC = ((checkGOP-1)/m_iGOPSize)*m_iGOPSize + m_GOPList[curGOP].m_POC;
    if(m_GOPList[curGOP].m_POC<0)
    {
      printf("\nError: found fewer Reference Picture Sets than GOPSize\n");
      errorGOP=true;
    }
    else
    {
      //check that all reference pictures are available, or have a POC < 0 meaning they might be available in the next GOP.
      Bool beforeI = false;
      for(Int i = 0; i< m_GOPList[curGOP].m_numRefPics; i++)
      {
        Int absPOC = curPOC+m_GOPList[curGOP].m_referencePics[i];
        if(absPOC < 0)
        {
          beforeI=true;
        }
        else
        {
          Bool found=false;
          for(Int j=0; j<numRefs; j++)
          {
            if(refList[j]==absPOC)
            {
              found=true;
              for(Int k=0; k<m_iGOPSize; k++)
              {
                if(absPOC%m_iGOPSize == m_GOPList[k].m_POC%m_iGOPSize)
                {
                  if(m_GOPList[k].m_temporalId==m_GOPList[curGOP].m_temporalId)
                  {
                    m_GOPList[k].m_refPic = true;
                  }
                  m_GOPList[curGOP].m_usedByCurrPic[i]=m_GOPList[k].m_temporalId<=m_GOPList[curGOP].m_temporalId;
                }
              }
            }
          }
          if(!found)
          {
            printf("\nError: ref pic %d is not available for GOP frame %d\n",m_GOPList[curGOP].m_referencePics[i],curGOP+1);
            errorGOP=true;
          }
        }
      }
      if(!beforeI&&!errorGOP)
      {
        //all ref frames were present
        if(!isOK[curGOP])
        {
          numOK++;
          isOK[curGOP]=true;
          if(numOK==m_iGOPSize)
          {
            verifiedGOP=true;
          }
        }
      }
      else
      {
        //create a new GOPEntry for this frame containing all the reference pictures that were available (POC > 0)
        m_GOPList[m_iGOPSize+m_extraRPSs]=m_GOPList[curGOP];
        Int newRefs=0;
        for(Int i = 0; i< m_GOPList[curGOP].m_numRefPics; i++)
        {
          Int absPOC = curPOC+m_GOPList[curGOP].m_referencePics[i];
          if(absPOC>=0)
          {
            m_GOPList[m_iGOPSize+m_extraRPSs].m_referencePics[newRefs]=m_GOPList[curGOP].m_referencePics[i];
            m_GOPList[m_iGOPSize+m_extraRPSs].m_usedByCurrPic[newRefs]=m_GOPList[curGOP].m_usedByCurrPic[i];
            newRefs++;
          }
        }
        Int numPrefRefs = m_GOPList[curGOP].m_numRefPicsActive;

        for(Int offset = -1; offset>-checkGOP; offset--)
        {
          //step backwards in coding order and include any extra available pictures we might find useful to replace the ones with POC < 0.
          Int offGOP = (checkGOP-1+offset)%m_iGOPSize;
          Int offPOC = ((checkGOP-1+offset)/m_iGOPSize)*m_iGOPSize + m_GOPList[offGOP].m_POC;
          if(offPOC>=0&&m_GOPList[offGOP].m_temporalId<=m_GOPList[curGOP].m_temporalId)
          {
            Bool newRef=false;
            for(Int i=0; i<numRefs; i++)
            {
              if(refList[i]==offPOC)
              {
                newRef=true;
              }
            }
            for(Int i=0; i<newRefs; i++)
            {
              if(m_GOPList[m_iGOPSize+m_extraRPSs].m_referencePics[i]==offPOC-curPOC)
              {
                newRef=false;
              }
            }
            if(newRef)
            {
              Int insertPoint=newRefs;
              //this picture can be added, find appropriate place in list and insert it.
              if(m_GOPList[offGOP].m_temporalId==m_GOPList[curGOP].m_temporalId)
              {
                m_GOPList[offGOP].m_refPic = true;
              }
              for(Int j=0; j<newRefs; j++)
              {
                if(m_GOPList[m_iGOPSize+m_extraRPSs].m_referencePics[j]<offPOC-curPOC||m_GOPList[m_iGOPSize+m_extraRPSs].m_referencePics[j]>0)
                {
                  insertPoint = j;
                  break;
                }
              }
              Int prev = offPOC-curPOC;
              Int prevUsed = m_GOPList[offGOP].m_temporalId<=m_GOPList[curGOP].m_temporalId;
              for(Int j=insertPoint; j<newRefs+1; j++)
              {
                Int newPrev = m_GOPList[m_iGOPSize+m_extraRPSs].m_referencePics[j];
                Int newUsed = m_GOPList[m_iGOPSize+m_extraRPSs].m_usedByCurrPic[j];
                m_GOPList[m_iGOPSize+m_extraRPSs].m_referencePics[j]=prev;
                m_GOPList[m_iGOPSize+m_extraRPSs].m_usedByCurrPic[j]=prevUsed;
                prevUsed=newUsed;
                prev=newPrev;
              }
              newRefs++;
            }
          }
          if(newRefs>=numPrefRefs)
          {
            break;
          }
        }
        m_GOPList[m_iGOPSize+m_extraRPSs].m_numRefPics=newRefs;
        m_GOPList[m_iGOPSize+m_extraRPSs].m_POC = curPOC;
        if (m_extraRPSs == 0)
        {
          m_GOPList[m_iGOPSize+m_extraRPSs].m_interRPSPrediction = 0;
          m_GOPList[m_iGOPSize+m_extraRPSs].m_numRefIdc = 0;
        }
        else
        {
          Int rIdx =  m_iGOPSize + m_extraRPSs - 1;
          Int refPOC = m_GOPList[rIdx].m_POC;
          Int refPics = m_GOPList[rIdx].m_numRefPics;
          Int newIdc=0;
          for(Int i = 0; i<= refPics; i++)
          {
            Int deltaPOC = ((i != refPics)? m_GOPList[rIdx].m_referencePics[i] : 0);  // check if the reference abs POC is >= 0
            Int absPOCref = refPOC+deltaPOC;
            Int refIdc = 0;
            for (Int j = 0; j < m_GOPList[m_iGOPSize+m_extraRPSs].m_numRefPics; j++)
            {
              if ( (absPOCref - curPOC) == m_GOPList[m_iGOPSize+m_extraRPSs].m_referencePics[j])
              {
                if (m_GOPList[m_iGOPSize+m_extraRPSs].m_usedByCurrPic[j])
                {
                  refIdc = 1;
                }
                else
                {
                  refIdc = 2;
                }
              }
            }
            m_GOPList[m_iGOPSize+m_extraRPSs].m_refIdc[newIdc]=refIdc;
            newIdc++;
          }
          m_GOPList[m_iGOPSize+m_extraRPSs].m_interRPSPrediction = 1;
          m_GOPList[m_iGOPSize+m_extraRPSs].m_numRefIdc = newIdc;
          m_GOPList[m_iGOPSize+m_extraRPSs].m_deltaRPS = refPOC - m_GOPList[m_iGOPSize+m_extraRPSs].m_POC;
        }
        curGOP=m_iGOPSize+m_extraRPSs;
        m_extraRPSs++;
      }
      numRefs=0;
      for(Int i = 0; i< m_GOPList[curGOP].m_numRefPics; i++)
      {
        Int absPOC = curPOC+m_GOPList[curGOP].m_referencePics[i];
        if(absPOC >= 0)
        {
          refList[numRefs]=absPOC;
          numRefs++;
        }
      }
      refList[numRefs]=curPOC;
      numRefs++;
    }
    checkGOP++;
  }
  xConfirmPara(errorGOP,"Invalid GOP structure given");
  m_maxTempLayer = 1;
  for(Int i=0; i<m_iGOPSize; i++)
  {
    if(m_GOPList[i].m_temporalId >= m_maxTempLayer)
    {
      m_maxTempLayer = m_GOPList[i].m_temporalId+1;
    }
    xConfirmPara(m_GOPList[i].m_sliceType!='B' && m_GOPList[i].m_sliceType!='P' && m_GOPList[i].m_sliceType!='I', "Slice type must be equal to B or P or I");
  }
  for(Int i=0; i<MAX_TLAYER; i++)
  {
    m_numReorderPics[i] = 0;
    m_maxDecPicBuffering[i] = 1;
  }
  for(Int i=0; i<m_iGOPSize; i++)
  {
    if(m_GOPList[i].m_numRefPics+1 > m_maxDecPicBuffering[m_GOPList[i].m_temporalId])
    {
      m_maxDecPicBuffering[m_GOPList[i].m_temporalId] = m_GOPList[i].m_numRefPics + 1;
    }
    Int highestDecodingNumberWithLowerPOC = 0;
    for(Int j=0; j<m_iGOPSize; j++)
    {
      if(m_GOPList[j].m_POC <= m_GOPList[i].m_POC)
      {
        highestDecodingNumberWithLowerPOC = j;
      }
    }
    Int numReorder = 0;
    for(Int j=0; j<highestDecodingNumberWithLowerPOC; j++)
    {
      if(m_GOPList[j].m_temporalId <= m_GOPList[i].m_temporalId &&
        m_GOPList[j].m_POC > m_GOPList[i].m_POC)
      {
        numReorder++;
      }
    }
    if(numReorder > m_numReorderPics[m_GOPList[i].m_temporalId])
    {
      m_numReorderPics[m_GOPList[i].m_temporalId] = numReorder;
    }
  }
  for(Int i=0; i<MAX_TLAYER-1; i++)
  {
    // a lower layer can not have higher value of m_numReorderPics than a higher layer
    if(m_numReorderPics[i+1] < m_numReorderPics[i])
    {
      m_numReorderPics[i+1] = m_numReorderPics[i];
    }
    // the value of num_reorder_pics[ i ] shall be in the range of 0 to max_dec_pic_buffering[ i ] - 1, inclusive
    if(m_numReorderPics[i] > m_maxDecPicBuffering[i] - 1)
    {
      m_maxDecPicBuffering[i] = m_numReorderPics[i] + 1;
    }
    // a lower layer can not have higher value of m_uiMaxDecPicBuffering than a higher layer
    if(m_maxDecPicBuffering[i+1] < m_maxDecPicBuffering[i])
    {
      m_maxDecPicBuffering[i+1] = m_maxDecPicBuffering[i];
    }
  }

  // the value of num_reorder_pics[ i ] shall be in the range of 0 to max_dec_pic_buffering[ i ] -  1, inclusive
  if(m_numReorderPics[MAX_TLAYER-1] > m_maxDecPicBuffering[MAX_TLAYER-1] - 1)
  {
    m_maxDecPicBuffering[MAX_TLAYER-1] = m_numReorderPics[MAX_TLAYER-1] + 1;
  }

  if(m_vuiParametersPresentFlag && m_bitstreamRestrictionFlag)
  {
    Int PicSizeInSamplesY =  m_iSourceWidth * m_iSourceHeight;
    if(tileFlag)
    {
      Int maxTileWidth = 0;
      Int maxTileHeight = 0;
      Int widthInCU = (m_iSourceWidth % m_uiMaxCUWidth) ? m_iSourceWidth/m_uiMaxCUWidth + 1: m_iSourceWidth/m_uiMaxCUWidth;
      Int heightInCU = (m_iSourceHeight % m_uiMaxCUHeight) ? m_iSourceHeight/m_uiMaxCUHeight + 1: m_iSourceHeight/m_uiMaxCUHeight;
      if(m_tileUniformSpacingFlag)
      {
        maxTileWidth = m_uiMaxCUWidth*((widthInCU+m_numTileColumnsMinus1)/(m_numTileColumnsMinus1+1));
        maxTileHeight = m_uiMaxCUHeight*((heightInCU+m_numTileRowsMinus1)/(m_numTileRowsMinus1+1));
        // if only the last tile-row is one treeblock higher than the others
        // the maxTileHeight becomes smaller if the last row of treeblocks has lower height than the others
        if(!((heightInCU-1)%(m_numTileRowsMinus1+1)))
        {
          maxTileHeight = maxTileHeight - m_uiMaxCUHeight + (m_iSourceHeight % m_uiMaxCUHeight);
        }
        // if only the last tile-column is one treeblock wider than the others
        // the maxTileWidth becomes smaller if the last column of treeblocks has lower width than the others
        if(!((widthInCU-1)%(m_numTileColumnsMinus1+1)))
        {
          maxTileWidth = maxTileWidth - m_uiMaxCUWidth + (m_iSourceWidth % m_uiMaxCUWidth);
        }
      }
      else // not uniform spacing
      {
        if(m_numTileColumnsMinus1<1)
        {
          maxTileWidth = m_iSourceWidth;
        }
        else
        {
          Int accColumnWidth = 0;
          for(Int col=0; col<(m_numTileColumnsMinus1); col++)
          {
            maxTileWidth = m_tileColumnWidth[col]>maxTileWidth ? m_tileColumnWidth[col]:maxTileWidth;
            accColumnWidth += m_tileColumnWidth[col];
          }
          maxTileWidth = (widthInCU-accColumnWidth)>maxTileWidth ? m_uiMaxCUWidth*(widthInCU-accColumnWidth):m_uiMaxCUWidth*maxTileWidth;
        }
        if(m_numTileRowsMinus1<1)
        {
          maxTileHeight = m_iSourceHeight;
        }
        else
        {
          Int accRowHeight = 0;
          for(Int row=0; row<(m_numTileRowsMinus1); row++)
          {
            maxTileHeight = m_tileRowHeight[row]>maxTileHeight ? m_tileRowHeight[row]:maxTileHeight;
            accRowHeight += m_tileRowHeight[row];
          }
          maxTileHeight = (heightInCU-accRowHeight)>maxTileHeight ? m_uiMaxCUHeight*(heightInCU-accRowHeight):m_uiMaxCUHeight*maxTileHeight;
        }
      }
      Int maxSizeInSamplesY = maxTileWidth*maxTileHeight;
      m_minSpatialSegmentationIdc = 4*PicSizeInSamplesY/maxSizeInSamplesY-4;
    }
    else if(m_entropyCodingSyncEnabledFlag)
    {
      m_minSpatialSegmentationIdc = 4*PicSizeInSamplesY/((2*m_iSourceHeight+m_iSourceWidth)*m_uiMaxCUHeight)-4;
    }
    else if(m_sliceMode == FIXED_NUMBER_OF_CTU)
    {
      m_minSpatialSegmentationIdc = 4*PicSizeInSamplesY/(m_sliceArgument*m_uiMaxCUWidth*m_uiMaxCUHeight)-4;
    }
    else
    {
      m_minSpatialSegmentationIdc = 0;
    }
  }

  if (m_toneMappingInfoSEIEnabled)
  {
    xConfirmPara( m_toneMapCodedDataBitDepth < 8 || m_toneMapCodedDataBitDepth > 14 , "SEIToneMapCodedDataBitDepth must be in rage 8 to 14");
    xConfirmPara( m_toneMapTargetBitDepth < 1 || (m_toneMapTargetBitDepth > 16 && m_toneMapTargetBitDepth < 255) , "SEIToneMapTargetBitDepth must be in rage 1 to 16 or equal to 255");
    xConfirmPara( m_toneMapModelId < 0 || m_toneMapModelId > 4 , "SEIToneMapModelId must be in rage 0 to 4");
    xConfirmPara( m_cameraIsoSpeedValue == 0, "SEIToneMapCameraIsoSpeedValue shall not be equal to 0");
    xConfirmPara( m_exposureIndexValue  == 0, "SEIToneMapExposureIndexValue shall not be equal to 0");
    xConfirmPara( m_extendedRangeWhiteLevel < 100, "SEIToneMapExtendedRangeWhiteLevel should be greater than or equal to 100");
    xConfirmPara( m_nominalBlackLevelLumaCodeValue >= m_nominalWhiteLevelLumaCodeValue, "SEIToneMapNominalWhiteLevelLumaCodeValue shall be greater than SEIToneMapNominalBlackLevelLumaCodeValue");
    xConfirmPara( m_extendedWhiteLevelLumaCodeValue < m_nominalWhiteLevelLumaCodeValue, "SEIToneMapExtendedWhiteLevelLumaCodeValue shall be greater than or equal to SEIToneMapNominalWhiteLevelLumaCodeValue");
  }

  if (m_kneeSEIEnabled && !m_kneeSEICancelFlag)
  {
    xConfirmPara( m_kneeSEINumKneePointsMinus1 < 0 || m_kneeSEINumKneePointsMinus1 > 998, "SEIKneeFunctionNumKneePointsMinus1 must be in the range of 0 to 998");
    for ( UInt i=0; i<=m_kneeSEINumKneePointsMinus1; i++ )
    {
      xConfirmPara( m_kneeSEIInputKneePoint[i] < 1 || m_kneeSEIInputKneePoint[i] > 999, "SEIKneeFunctionInputKneePointValue must be in the range of 1 to 999");
      xConfirmPara( m_kneeSEIOutputKneePoint[i] < 0 || m_kneeSEIOutputKneePoint[i] > 1000, "SEIKneeFunctionInputKneePointValue must be in the range of 0 to 1000");
      if ( i > 0 )
      {
        xConfirmPara( m_kneeSEIInputKneePoint[i-1] >= m_kneeSEIInputKneePoint[i],  "The i-th SEIKneeFunctionInputKneePointValue must be greater than the (i-1)-th value");
        xConfirmPara( m_kneeSEIOutputKneePoint[i-1] > m_kneeSEIOutputKneePoint[i],  "The i-th SEIKneeFunctionOutputKneePointValue must be greater than or equal to the (i-1)-th value");
      }
    }
  }

  if (m_chromaResamplingFilterSEIenabled)
  {
    xConfirmPara( (m_chromaFormatIDC == CHROMA_400 ), "chromaResamplingFilterSEI is not allowed to be present when ChromaFormatIDC is equal to zero (4:0:0)" );
    xConfirmPara(m_vuiParametersPresentFlag && m_chromaLocInfoPresentFlag && (m_chromaSampleLocTypeTopField != m_chromaSampleLocTypeBottomField ), "When chromaResamplingFilterSEI is enabled, ChromaSampleLocTypeTopField has to be equal to ChromaSampleLocTypeBottomField" );
  }

#if P0123_ALPHA_CHANNEL_SEI
  if( m_alphaSEIEnabled && !m_alphaCancelFlag )
  {
    xConfirmPara(0 < m_alphaUseIdc || m_alphaUseIdc > 2, "SEIAlphaUseIdc greater than 2 is reserved for future use by ITU-T | ISO/IEC");
    xConfirmPara(m_alphaBitDepthMinus8 < 0 || m_alphaBitDepthMinus8 > 7, "SEIAlphaBitDepthMinus8 shall be in the range 0 to 7 inclusive");
  }
#endif
#if Q0096_OVERLAY_SEI
  if( m_overlaySEIEnabled && !m_overlayInfoCancelFlag )
  {
    xConfirmPara( m_overlayContentAuxIdMinus128 > 31, "SEIOverlayContentAuxIdMinus128 must be in the range of 0 to 31");
    xConfirmPara( m_overlayLabelAuxIdMinus128 > 31, "SEIOverlayLabelAuxIdMinus128 must be in the range of 0 to 31");
    xConfirmPara( m_overlayAlphaAuxIdMinus128 > 31, "SEIOverlayAlphaAuxIdMinus128 must be in the range of 0 to 31");
    xConfirmPara( m_numOverlaysMinus1 > 15, "SEIOverlayNumOverlaysMinus1 must be in the range of 0 to 15");
    for (Int i=0 ; i<=m_numOverlaysMinus1 ; i++ )
    {
      xConfirmPara( m_overlayIdx[i] > 255, "SEIOverlayIdx must be in the range of 0 to 255");
      xConfirmPara( m_numOverlayElementsMinus1[i] > 255, "SEIOverlayNumElementsMinus1 must be in the range of 0 to 255");
    }
  }
#endif

  if ( m_RCEnableRateControl )
  {
    if ( m_RCForceIntraQP )
    {
      if ( m_RCInitialQP == 0 )
      {
        printf( "\nInitial QP for rate control is not specified. Reset not to use force intra QP!" );
        m_RCForceIntraQP = false;
      }
    }
    xConfirmPara( m_uiDeltaQpRD > 0, "Rate control cannot be used together with slice level multiple-QP optimization!\n" );
#if U0132_TARGET_BITS_SATURATION
    if ((m_RCCpbSaturationEnabled) && (m_level!=Level::NONE) && (m_profile!=Profile::NONE))
    {
      UInt uiLevelIdx = (m_level / 10) + (UInt)((m_level % 10) / 3);    // (m_level / 30)*3 + ((m_level % 10) / 3);
      xConfirmPara(m_RCCpbSize > g_uiMaxCpbSize[m_levelTier][uiLevelIdx], "RCCpbSize should be smaller than or equal to Max CPB size according to tier and level");
      xConfirmPara(m_RCInitialCpbFullness > 1, "RCInitialCpbFullness should be smaller than or equal to 1");
    }
#endif
  }
#if U0132_TARGET_BITS_SATURATION
  else
  {
    xConfirmPara( m_RCCpbSaturationEnabled != 0, "Target bits saturation cannot be processed without Rate control" );
  }
  if (m_vuiParametersPresentFlag)
  {
    xConfirmPara(m_RCTargetBitrate == 0, "A target bit rate is required to be set for VUI/HRD parameters.");
    if (m_RCCpbSize == 0)
    {
      printf ("Warning: CPB size is set equal to zero. Adjusting value to be equal to TargetBitrate!\n");
      m_RCCpbSize = m_RCTargetBitrate;
    }
  }
#endif
  
#if PER_LAYER_LOSSLESS
  xConfirmPara(!m_apcLayerCfg[layerIdx]->m_TransquantBypassEnabledFlag && m_apcLayerCfg[layerIdx]->m_CUTransquantBypassFlagForce, "CUTransquantBypassFlagForce cannot be 1 when TransquantBypassEnableFlag is 0");
#else
  xConfirmPara(!m_TransquantBypassEnabledFlag && m_CUTransquantBypassFlagForce, "CUTransquantBypassFlagForce cannot be 1 when TransquantBypassEnableFlag is 0");
#endif

  xConfirmPara(m_log2ParallelMergeLevel < 2, "Log2ParallelMergeLevel should be larger than or equal to 2");

  if (m_framePackingSEIEnabled)
  {
    xConfirmPara(m_framePackingSEIType < 3 || m_framePackingSEIType > 5 , "SEIFramePackingType must be in rage 3 to 5");
  }

  if (m_segmentedRectFramePackingSEIEnabled)
  {
    xConfirmPara(m_framePackingSEIEnabled , "SEISegmentedRectFramePacking must be 0 when SEIFramePacking is 1");
  }

  if((m_numTileColumnsMinus1 <= 0) && (m_numTileRowsMinus1 <= 0) && m_tmctsSEIEnabled)
  {
    printf("Warning: SEITempMotionConstrainedTileSets is set to false to disable temporal motion-constrained tile sets SEI message because there are no tiles enabled.\n");
    m_tmctsSEIEnabled = false;
  }

  if(m_timeCodeSEIEnabled)
  {
    xConfirmPara(m_timeCodeSEINumTs > MAX_TIMECODE_SEI_SETS, "Number of time sets cannot exceed 3");
  }

#if U0033_ALTERNATIVE_TRANSFER_CHARACTERISTICS_SEI
  xConfirmPara(m_preferredTransferCharacteristics > 255, "transfer_characteristics_idc should not be greater than 255.");
#endif

#if SVC_EXTENSION
  xConfirmPara( (m_apcLayerCfg[0]->m_numSamplePredRefLayers != 0) && (m_apcLayerCfg[0]->m_numSamplePredRefLayers != -1), "Layer 0 cannot have any reference layers" );
  // NOTE: m_numSamplePredRefLayers  (for any layer) could be -1 (not signalled in cfg), in which case only the "previous layer" would be taken for reference
  if( layerIdx > 0 )
  {
    xConfirmPara(m_apcLayerCfg[layerIdx]->m_numSamplePredRefLayers > layerIdx, "Cannot reference more layers than before current layer");
    for(Int i = 0; i < m_apcLayerCfg[layerIdx]->m_numSamplePredRefLayers; i++)
    {
      xConfirmPara(m_apcLayerCfg[layerIdx]->m_samplePredRefLayerIds[i] > m_apcLayerCfg[layerIdx]->m_layerId, "Cannot reference higher layers");
      xConfirmPara(m_apcLayerCfg[layerIdx]->m_samplePredRefLayerIds[i] == m_apcLayerCfg[layerIdx]->m_layerId, "Cannot reference the current layer itself");
    }
  }
  xConfirmPara( (m_apcLayerCfg[0]->m_numMotionPredRefLayers != 0) && (m_apcLayerCfg[0]->m_numMotionPredRefLayers != -1), "Layer 0 cannot have any reference layers" );
  // NOTE: m_numMotionPredRefLayers  (for any layer) could be -1 (not signalled in cfg), in which case only the "previous layer" would be taken for reference
  if( layerIdx > 0 )
  {
    xConfirmPara(m_apcLayerCfg[layerIdx]->m_numMotionPredRefLayers > layerIdx, "Cannot reference more layers than before current layer");
    for(Int i = 0; i < m_apcLayerCfg[layerIdx]->m_numMotionPredRefLayers; i++)
    {
      xConfirmPara(m_apcLayerCfg[layerIdx]->m_motionPredRefLayerIds[i] > m_apcLayerCfg[layerIdx]->m_layerId, "Cannot reference higher layers");
      xConfirmPara(m_apcLayerCfg[layerIdx]->m_motionPredRefLayerIds[i] == m_apcLayerCfg[layerIdx]->m_layerId, "Cannot reference the current layer itself");
    }
  }

  xConfirmPara( (m_apcLayerCfg[0]->m_numActiveRefLayers != 0) && (m_apcLayerCfg[0]->m_numActiveRefLayers != -1), "Layer 0 cannot have any active reference layers" );
  // NOTE: m_numActiveRefLayers  (for any layer) could be -1 (not signalled in cfg), in which case only the "previous layer" would be taken for reference
  if( layerIdx > 0 )
  {
    Bool predEnabledFlag[MAX_LAYERS];
    for (Int refLayer = 0; refLayer < layerIdx; refLayer++)
    {
      predEnabledFlag[refLayer] = false;
    }
    for(Int i = 0; i < m_apcLayerCfg[layerIdx]->m_numSamplePredRefLayers; i++)
    {
      predEnabledFlag[m_apcLayerCfg[layerIdx]->m_samplePredRefLayerIds[i]] = true;
    }
    for(Int i = 0; i < m_apcLayerCfg[layerIdx]->m_numMotionPredRefLayers; i++)
    {
      predEnabledFlag[m_apcLayerCfg[layerIdx]->m_motionPredRefLayerIds[i]] = true;
    }
    Int numDirectRefLayers = 0;
    for (Int refLayer = 0; refLayer < layerIdx; refLayer++)
    {
      if (predEnabledFlag[refLayer] == true) numDirectRefLayers++;
    }
    xConfirmPara(m_apcLayerCfg[layerIdx]->m_numActiveRefLayers > numDirectRefLayers, "Cannot reference more layers than NumDirectRefLayers");
    for(Int i = 0; i < m_apcLayerCfg[layerIdx]->m_numActiveRefLayers; i++)
    {
      xConfirmPara(m_apcLayerCfg[layerIdx]->m_predLayerIds[i] >= numDirectRefLayers, "Cannot reference higher layers");
    }
  }

  if( m_adaptiveResolutionChange > 0 )
  {
    xConfirmPara(m_numLayers != 2, "Adaptive resolution change works with 2 layers only");
    xConfirmPara(m_apcLayerCfg[1]->m_iIntraPeriod == 0 || (m_adaptiveResolutionChange % m_apcLayerCfg[1]->m_iIntraPeriod) != 0, "Adaptive resolution change must happen at enhancement layer RAP picture");
  }

  if( m_adaptiveResolutionChange > 0 )
  {
    xConfirmPara(m_crossLayerIrapAlignFlag != 0, "Cross layer IRAP alignment must be disabled when using adaptive resolution change.");
  }

  if( m_skipPictureAtArcSwitch )
  {
    xConfirmPara(m_adaptiveResolutionChange <= 0, "Skip picture at ARC switching only works when Adaptive Resolution Change is active (AdaptiveResolutionChange > 0)");
  }

  if( layerIdx < MAX_LAYERS-1 )
  {
    xConfirmPara(m_apcLayerCfg[layerIdx]->m_maxTidIlRefPicsPlus1 < 0 || m_apcLayerCfg[layerIdx]->m_maxTidIlRefPicsPlus1 > 7, "MaxTidIlRefPicsPlus1 must be in range 0 to 7");
  }
#if AUXILIARY_PICTURES
  if( layerIdx < MAX_LAYERS-1 )
  {
    xConfirmPara(m_apcLayerCfg[layerIdx]->m_auxId < 0 || m_apcLayerCfg[layerIdx]->m_auxId > 2, "AuxId must be in range 0 to 2");
  }
#endif 
#if VIEW_SCALABILITY
  if (m_scalabilityMask[VIEW_ORDER_INDEX])
  {   
    xConfirmPara(m_apcLayerCfg[layerIdx]->m_viewId < 0 , "ViewId must be greater than or equal to 0");
    xConfirmPara(m_apcLayerCfg[layerIdx]->m_viewOrderIndex < 0 , "ViewOrderIndex must be greater than or equal to 0");

    if( layerIdx > 0 )
    {
      xConfirmPara(m_apcLayerCfg[layerIdx]->m_viewOrderIndex < m_apcLayerCfg[layerIdx-1]->m_viewOrderIndex, "ViewOrderIndex shall be increasing");
    }

  }
#endif
#if CGS_3D_ASYMLUT
  xConfirmPara( m_nCGSFlag < 0 || m_nCGSFlag > 1 , "0<=CGS<=1" );
#endif
#endif //SVC_EXTENSION
#undef xConfirmPara
  if (check_failed)
  {
    exit(EXIT_FAILURE);
  }
}

const TChar *profileToString(const Profile::Name profile)
{
  static const UInt numberOfProfiles = sizeof(strToProfile)/sizeof(*strToProfile);

  for (UInt profileIndex = 0; profileIndex < numberOfProfiles; profileIndex++)
  {
    if (strToProfile[profileIndex].value == profile)
    {
      return strToProfile[profileIndex].str;
    }
  }

  //if we get here, we didn't find this profile in the list - so there is an error
  std::cerr << "ERROR: Unknown profile \"" << profile << "\" in profileToString" << std::endl;
  assert(false);
  exit(1);
  return "";
}

Void TAppEncCfg::xPrintParameter()
{
  printf("\n");
#if SVC_EXTENSION
  printf("Total number of layers            : %d\n", m_numLayers       );
  printf("Multiview                         : %d\n", m_scalabilityMask[VIEW_ORDER_INDEX] );
  printf("Scalable                          : %d\n", m_scalabilityMask[SCALABILITY_ID] );
#if AVC_BASE
  printf("Base layer                        : %s\n", m_nonHEVCBaseLayerFlag ? "Non-HEVC" : "HEVC");
#endif
#if AUXILIARY_PICTURES
  printf("Auxiliary pictures                : %d\n", m_scalabilityMask[AUX_ID] );
#endif
  printf("Adaptive Resolution Change        : %d\n", m_adaptiveResolutionChange );
  printf("Skip picture at ARC switch        : %d\n", m_skipPictureAtArcSwitch );
  printf("Align picture type                : %d\n", m_crossLayerPictureTypeAlignFlag );
  printf("Cross layer IRAP alignment        : %d\n", m_crossLayerIrapAlignFlag );
  printf("IDR only for IRAP                 : %d\n", m_crossLayerAlignedIdrOnlyFlag );
  printf("InterLayerWeightedPred            : %d\n", m_useInterLayerWeightedPred );

  for( UInt layer = 0; layer < m_numLayers + 1; layer++ )
  {
    UInt layerIdx                          = layer == m_numLayers ? 0 : layer;
    Int& layerPTLIdx                       = m_apcLayerCfg[layerIdx]->m_layerPTLIdx;
    Profile::Name& m_profile               = m_profileList[layerPTLIdx];
    Bool m_onePictureOnlyConstraintFlag    = m_apcLayerCfg[layerIdx]->m_onePictureOnlyConstraintFlag;
    UInt& m_bitDepthConstraint             = m_apcLayerCfg[layerIdx]->m_bitDepthConstraint;
    Bool& m_intraConstraintFlag            = m_apcLayerCfg[layerIdx]->m_intraConstraintFlag;
    ChromaFormat& m_chromaFormatConstraint = m_apcLayerCfg[layerIdx]->m_chromaFormatConstraint;

    UInt& m_uiMaxCUWidth                   = m_apcLayerCfg[layerIdx]->m_uiMaxCUWidth;
    UInt& m_uiMaxCUDepth                   = m_apcLayerCfg[layerIdx]->m_uiMaxCUDepth;

    Int& m_confWinLeft                     = m_apcLayerCfg[layerIdx]->m_confWinLeft;
    Int& m_confWinRight                    = m_apcLayerCfg[layerIdx]->m_confWinRight;
    Int& m_confWinTop                      = m_apcLayerCfg[layerIdx]->m_confWinTop;
    Int& m_confWinBottom                   = m_apcLayerCfg[layerIdx]->m_confWinBottom;

    Int& m_iSourceWidth                    = m_apcLayerCfg[layerIdx]->m_iSourceWidth;
    Int& m_iSourceHeight                   = m_apcLayerCfg[layerIdx]->m_iSourceHeight;

    string& m_inputFileName                = m_apcLayerCfg[layerIdx]->m_inputFileName;
    string& m_reconFileName                = m_apcLayerCfg[layerIdx]->m_reconFileName;

    Int& m_iFrameRate                      = m_apcLayerCfg[layerIdx]->m_iFrameRate;
    Int& m_iIntraPeriod                    = m_apcLayerCfg[layerIdx]->m_iIntraPeriod;

    UInt& m_uiMaxTotalCUDepth              = m_apcLayerCfg[layerIdx]->m_uiMaxTotalCUDepth;
    UInt& m_uiQuadtreeTULog2MinSize        = m_apcLayerCfg[layerIdx]->m_uiQuadtreeTULog2MinSize;
    UInt& m_uiQuadtreeTULog2MaxSize        = m_apcLayerCfg[layerIdx]->m_uiQuadtreeTULog2MaxSize;
    UInt& m_uiQuadtreeTUMaxDepthInter      = m_apcLayerCfg[layerIdx]->m_uiQuadtreeTUMaxDepthInter;
    UInt& m_uiQuadtreeTUMaxDepthIntra      = m_apcLayerCfg[layerIdx]->m_uiQuadtreeTUMaxDepthIntra;

    Double& m_fQP                          = m_apcLayerCfg[layerIdx]->m_fQP;
    Int& m_iMaxCuDQPDepth                  = m_apcLayerCfg[layerIdx]->m_iMaxCuDQPDepth;

    Bool& m_RCEnableRateControl            = m_apcLayerCfg[layerIdx]->m_RCEnableRateControl;
    Int& m_RCTargetBitrate                 = m_apcLayerCfg[layerIdx]->m_RCTargetBitrate;
    Bool& m_RCKeepHierarchicalBit          = m_apcLayerCfg[layerIdx]->m_RCKeepHierarchicalBit;
    Bool& m_RCLCULevelRC                   = m_apcLayerCfg[layerIdx]->m_RCLCULevelRC;
    Bool& m_RCUseLCUSeparateModel          = m_apcLayerCfg[layerIdx]->m_RCUseLCUSeparateModel;
    Int& m_RCInitialQP                     = m_apcLayerCfg[layerIdx]->m_RCInitialQP;
    Bool& m_RCForceIntraQP                 = m_apcLayerCfg[layerIdx]->m_RCForceIntraQP;

    Bool& m_bUseSAO                         = m_apcLayerCfg[layerIdx]->m_bUseSAO;

    Int* m_inputBitDepth                    = m_apcLayerCfg[layerIdx]->m_inputBitDepth;
    Int* m_MSBExtendedBitDepth              = m_apcLayerCfg[layerIdx]->m_MSBExtendedBitDepth;
    Int* m_internalBitDepth                 = m_apcLayerCfg[layerIdx]->m_internalBitDepth;

#if U0132_TARGET_BITS_SATURATION
    Bool& m_RCCpbSaturationEnabled          = m_apcLayerCfg[layerIdx]->m_RCCpbSaturationEnabled;
    UInt& m_RCCpbSize                       = m_apcLayerCfg[layerIdx]->m_RCCpbSize;
    Double& m_RCInitialCpbFullness          = m_apcLayerCfg[layerIdx]->m_RCInitialCpbFullness;
#endif

#if PER_LAYER_LOSSLESS
    CostMode& m_costMode                    = m_apcLayerCfg[layerIdx]->m_costMode;
#endif

    if( layer == m_numLayers )
    {
      printf("\n=== Common configuration settings === \n");
    }
    
    if( layer < m_numLayers )
    {
      printf("\n=== Layer %d settings ===\n", layer);
#endif

  printf("Input          File                    : %s\n", m_inputFileName.c_str()          );

#if SVC_EXTENSION
    }
    if( layer == m_numLayers )
    {
#endif

  printf("Bitstream      File                    : %s\n", m_bitstreamFileName.c_str()      );

#if SVC_EXTENSION
    }
    if( layer < m_numLayers )
    {
#endif
  
  printf("Reconstruction File                    : %s\n", m_reconFileName.c_str()          );
#if SVC_EXTENSION
  printf("Real     Format                        : %dx%d %gHz\n", m_iSourceWidth - ( m_confWinLeft + m_confWinRight ) * TComSPS::getWinUnitX( m_apcLayerCfg[layerIdx]->m_chromaFormatIDC ), m_iSourceHeight - ( m_confWinTop + m_confWinBottom ) * TComSPS::getWinUnitY( m_apcLayerCfg[layerIdx]->m_chromaFormatIDC ), (Double)m_iFrameRate/m_temporalSubsampleRatio );
#else
  printf("Real     Format                        : %dx%d %gHz\n", m_iSourceWidth - m_confWinLeft - m_confWinRight, m_iSourceHeight - m_confWinTop - m_confWinBottom, (Double)m_iFrameRate/m_temporalSubsampleRatio );
#endif
  printf("Internal Format                        : %dx%d %gHz\n", m_iSourceWidth, m_iSourceHeight, (Double)m_iFrameRate/m_temporalSubsampleRatio );

#if SVC_EXTENSION
#if VIEW_SCALABILITY
  if( m_scalabilityMask[VIEW_ORDER_INDEX] )
  {
    printf("ViewOrderIndex                         : %d\n", m_apcLayerCfg[layer]->m_viewOrderIndex );
    printf("ViewId                                 : %d\n", m_apcLayerCfg[layer]->m_viewId );
  }
#endif
      printf("PTL index                              : %d\n", m_apcLayerCfg[layerIdx]->m_layerPTLIdx );
    }

    if( layer == m_numLayers )
    {
#endif
  printf("Sequence PSNR output                   : %s\n", (m_printMSEBasedSequencePSNR ? "Linear average, MSE-based" : "Linear average only") );
  printf("Sequence MSE output                    : %s\n", (m_printSequenceMSE ? "Enabled" : "Disabled") );
  printf("Frame MSE output                       : %s\n", (m_printFrameMSE    ? "Enabled" : "Disabled") );
  printf("Cabac-zero-word-padding                : %s\n", (m_cabacZeroWordPaddingEnabled? "Enabled" : "Disabled") );
  if (m_isField)
  {
    printf("Frame/Field                            : Field based coding\n");
    printf("Field index                            : %u - %d (%d fields)\n", m_FrameSkip, m_FrameSkip+m_framesToBeEncoded-1, m_framesToBeEncoded );
    printf("Field Order                            : %s field first\n", m_isTopFieldFirst?"Top":"Bottom");

  }
  else
  {
    printf("Frame/Field                            : Frame based coding\n");
    printf("Frame index                            : %u - %d (%d frames)\n", m_FrameSkip, m_FrameSkip+m_framesToBeEncoded-1, m_framesToBeEncoded );
  }
#if SVC_EXTENSION
    }

    if( layer < m_numLayers )
    {
#endif

  if (m_profile == Profile::MAINREXT)
  {
    ExtendedProfileName validProfileName;
    if (m_onePictureOnlyConstraintFlag)
    {
      validProfileName = m_bitDepthConstraint == 8 ? MAIN_444_STILL_PICTURE : (m_bitDepthConstraint == 16 ? MAIN_444_16_STILL_PICTURE : NONE);
    }
    else
    {
      const UInt intraIdx = m_intraConstraintFlag ? 1:0;
      const UInt bitDepthIdx = (m_bitDepthConstraint == 8 ? 0 : (m_bitDepthConstraint ==10 ? 1 : (m_bitDepthConstraint == 12 ? 2 : (m_bitDepthConstraint == 16 ? 3 : 4 ))));
      const UInt chromaFormatIdx = UInt(m_chromaFormatConstraint);
      validProfileName = (bitDepthIdx > 3 || chromaFormatIdx>3) ? NONE : validRExtProfileNames[intraIdx][bitDepthIdx][chromaFormatIdx];
    }
    std::string rextSubProfile;
    if (validProfileName!=NONE)
    {
      rextSubProfile=enumToString(strToExtendedProfile, sizeof(strToExtendedProfile)/sizeof(*strToExtendedProfile), validProfileName);
    }
    if (rextSubProfile == "main_444_16")
    {
      rextSubProfile="main_444_16 [NON STANDARD]";
    }
    printf("Profile                                : %s (%s)\n", profileToString(m_profile), (rextSubProfile.empty())?"INVALID REXT PROFILE":rextSubProfile.c_str() );
  }
  else
  {
    printf("Profile                                : %s\n", profileToString(m_profile) );
  }
  printf("CU size / depth / total-depth          : %d / %d / %d\n", m_uiMaxCUWidth, m_uiMaxCUDepth, m_uiMaxTotalCUDepth );
  printf("RQT trans. size (min / max)            : %d / %d\n", 1 << m_uiQuadtreeTULog2MinSize, 1 << m_uiQuadtreeTULog2MaxSize );
  printf("Max RQT depth inter                    : %d\n", m_uiQuadtreeTUMaxDepthInter);
  printf("Max RQT depth intra                    : %d\n", m_uiQuadtreeTUMaxDepthIntra);
#if SVC_EXTENSION
    }

    if( layer == m_numLayers )
    {
#endif
  printf("Min PCM size                           : %d\n", 1 << m_uiPCMLog2MinSize);
  printf("Motion search range                    : %d\n", m_iSearchRange );
#if SVC_EXTENSION
    }

    if( layer < m_numLayers )
    {
#endif
  printf("Intra period                           : %d\n", m_iIntraPeriod );
#if SVC_EXTENSION
    }

    if( layer == m_numLayers )
    {
#endif
  printf("Decoding refresh type                  : %d\n", m_iDecodingRefreshType );
#if SVC_EXTENSION
    }

    if( layer < m_numLayers )
    {
#endif
  printf("QP                                     : %5.2f\n", m_fQP );
  printf("Max dQP signaling depth                : %d\n", m_iMaxCuDQPDepth);
#if SVC_EXTENSION
    }

    if( layer == m_numLayers )
    {
#endif

  printf("Cb QP Offset                           : %d\n", m_cbQpOffset   );
  printf("Cr QP Offset                           : %d\n", m_crQpOffset);
  printf("QP adaptation                          : %d (range=%d)\n", m_bUseAdaptiveQP, (m_bUseAdaptiveQP ? m_iQPAdaptationRange : 0) );
  printf("GOP size                               : %d\n", m_iGOPSize );

#if SVC_EXTENSION
    }

    if( layer < m_numLayers )
    {
#endif

  printf("Input bit depth                        : (Y:%d, C:%d)\n", m_inputBitDepth[CHANNEL_TYPE_LUMA], m_inputBitDepth[CHANNEL_TYPE_CHROMA] );
  printf("MSB-extended bit depth                 : (Y:%d, C:%d)\n", m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA], m_MSBExtendedBitDepth[CHANNEL_TYPE_CHROMA] );
  printf("Internal bit depth                     : (Y:%d, C:%d)\n", m_internalBitDepth[CHANNEL_TYPE_LUMA], m_internalBitDepth[CHANNEL_TYPE_CHROMA] );
  printf("PCM sample bit depth                   : (Y:%d, C:%d)\n", m_bPCMInputBitDepthFlag ? m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA] : m_internalBitDepth[CHANNEL_TYPE_LUMA],
                                                                    m_bPCMInputBitDepthFlag ? m_MSBExtendedBitDepth[CHANNEL_TYPE_CHROMA] : m_internalBitDepth[CHANNEL_TYPE_CHROMA] );
#if SVC_EXTENSION
  printChromaFormat( m_apcLayerCfg[layerIdx]->m_InputChromaFormatIDC, m_apcLayerCfg[layerIdx]->m_chromaFormatIDC );
    }

    if( layer == m_numLayers )
    {
#endif

  printf("Intra reference smoothing              : %s\n", (m_enableIntraReferenceSmoothing           ? "Enabled" : "Disabled") );
  printf("diff_cu_chroma_qp_offset_depth         : %d\n", m_diffCuChromaQpOffsetDepth);
  printf("extended_precision_processing_flag     : %s\n", (m_extendedPrecisionProcessingFlag         ? "Enabled" : "Disabled") );
  printf("implicit_rdpcm_enabled_flag            : %s\n", (m_rdpcmEnabledFlag[RDPCM_SIGNAL_IMPLICIT] ? "Enabled" : "Disabled") );
  printf("explicit_rdpcm_enabled_flag            : %s\n", (m_rdpcmEnabledFlag[RDPCM_SIGNAL_EXPLICIT] ? "Enabled" : "Disabled") );
  printf("transform_skip_rotation_enabled_flag   : %s\n", (m_transformSkipRotationEnabledFlag        ? "Enabled" : "Disabled") );
  printf("transform_skip_context_enabled_flag    : %s\n", (m_transformSkipContextEnabledFlag         ? "Enabled" : "Disabled") );
  printf("cross_component_prediction_enabled_flag: %s\n", (m_crossComponentPredictionEnabledFlag     ? (m_reconBasedCrossCPredictionEstimate ? "Enabled (reconstructed-residual-based estimate)" : "Enabled (encoder-side-residual-based estimate)") : "Disabled") );
  printf("high_precision_offsets_enabled_flag    : %s\n", (m_highPrecisionOffsetsEnabledFlag         ? "Enabled" : "Disabled") );
  printf("persistent_rice_adaptation_enabled_flag: %s\n", (m_persistentRiceAdaptationEnabledFlag     ? "Enabled" : "Disabled") );
  printf("cabac_bypass_alignment_enabled_flag    : %s\n", (m_cabacBypassAlignmentEnabledFlag         ? "Enabled" : "Disabled") );
  if (m_bUseSAO)
  {
    printf("log2_sao_offset_scale_luma             : %d\n", m_log2SaoOffsetScale[CHANNEL_TYPE_LUMA]);
    printf("log2_sao_offset_scale_chroma           : %d\n", m_log2SaoOffsetScale[CHANNEL_TYPE_CHROMA]);
  }

  switch (m_costMode)
  {
    case COST_STANDARD_LOSSY:               printf("Cost function:                         : Lossy coding (default)\n"); break;
    case COST_SEQUENCE_LEVEL_LOSSLESS:      printf("Cost function:                         : Sequence_level_lossless coding\n"); break;
    case COST_LOSSLESS_CODING:              printf("Cost function:                         : Lossless coding with fixed QP of %d\n", LOSSLESS_AND_MIXED_LOSSLESS_RD_COST_TEST_QP); break;
    case COST_MIXED_LOSSLESS_LOSSY_CODING:  printf("Cost function:                         : Mixed_lossless_lossy coding with QP'=%d for lossless evaluation\n", LOSSLESS_AND_MIXED_LOSSLESS_RD_COST_TEST_QP_PRIME); break;
    default:                                printf("Cost function:                         : Unknown\n"); break;
  }

#if SVC_EXTENSION
    }

    if( layer < m_numLayers )
    {
#endif

  printf("RateControl                            : %d\n", m_RCEnableRateControl );

#if SVC_EXTENSION
    }

    if( layer == m_numLayers )
    {
#endif
  printf("WPMethod                               : %d\n", Int(m_weightedPredictionMethod));

#if SVC_EXTENSION
    }

    if( layer < m_numLayers )
    {
#endif

  if(m_RCEnableRateControl)
  {
    printf("TargetBitrate                          : %d\n", m_RCTargetBitrate );
    printf("KeepHierarchicalBit                    : %d\n", m_RCKeepHierarchicalBit );
    printf("LCULevelRC                             : %d\n", m_RCLCULevelRC );
    printf("UseLCUSeparateModel                    : %d\n", m_RCUseLCUSeparateModel );
    printf("InitialQP                              : %d\n", m_RCInitialQP );
    printf("ForceIntraQP                           : %d\n", m_RCForceIntraQP );
#if U0132_TARGET_BITS_SATURATION
    printf("CpbSaturation                          : %d\n", m_RCCpbSaturationEnabled );
    if (m_RCCpbSaturationEnabled)
    {
      printf("CpbSize                                : %d\n", m_RCCpbSize);
      printf("InitalCpbFullness                      : %.2f\n", m_RCInitialCpbFullness);
    }
#endif
  }

#if SVC_EXTENSION
    }

    if( layer == m_numLayers )
    {
#endif

  printf("Max Num Merge Candidates               : %d\n", m_maxNumMergeCand);
  printf("\n");

#if SVC_EXTENSION
    }
  }

  for( UInt layer = 0; layer < m_numLayers; layer++ )
  {
    Int* m_internalBitDepth               = m_apcLayerCfg[layer]->m_internalBitDepth;
    Int* m_MSBExtendedBitDepth            = m_apcLayerCfg[layer]->m_MSBExtendedBitDepth;
    Bool& m_bUseSAO                       = m_apcLayerCfg[layer]->m_bUseSAO;
    UInt& m_uiMaxCUWidth                  = m_apcLayerCfg[layer]->m_uiMaxCUWidth;
    UInt& m_uiMaxCUHeight                 = m_apcLayerCfg[layer]->m_uiMaxCUHeight;
    Int& m_iSourceHeight                  = m_apcLayerCfg[layer]->m_iSourceHeight;
    Bool& m_entropyCodingSyncEnabledFlag  = m_apcLayerCfg[layer]->m_entropyCodingSyncEnabledFlag;
    ScalingListMode& m_useScalingListId   = m_apcLayerCfg[layer]->m_useScalingListId;
    
    printf("Layer%d ", layer);
#endif

  printf("TOOL CFG: ");
  printf("IBD:%d ", ((m_internalBitDepth[CHANNEL_TYPE_LUMA] > m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA]) || (m_internalBitDepth[CHANNEL_TYPE_CHROMA] > m_MSBExtendedBitDepth[CHANNEL_TYPE_CHROMA])));
  printf("HAD:%d ", m_bUseHADME                          );
  printf("RDQ:%d ", m_useRDOQ                            );
  printf("RDQTS:%d ", m_useRDOQTS                        );
  printf("RDpenalty:%d ", m_rdPenalty                    );
  printf("SQP:%d ", m_uiDeltaQpRD                        );
  printf("ASR:%d ", m_bUseASR                            );
  printf("MinSearchWindow:%d ", m_minSearchWindow        );
  printf("RestrictMESampling:%d ", m_bRestrictMESampling );
  printf("FEN:%d ", Int(m_fastInterSearchMode)           );
  printf("ECU:%d ", m_bUseEarlyCU                        );
  printf("FDM:%d ", m_useFastDecisionForMerge            );
  printf("CFM:%d ", m_bUseCbfFastMode                    );
  printf("ESD:%d ", m_useEarlySkipDetection              );
  printf("RQT:%d ", 1                                    );
  printf("TransformSkip:%d ",     m_useTransformSkip     );
  printf("TransformSkipFast:%d ", m_useTransformSkipFast );
  printf("TransformSkipLog2MaxSize:%d ", m_log2MaxTransformSkipBlockSize);
  printf("Slice: M=%d ", Int(m_sliceMode));
  if (m_sliceMode!=NO_SLICES)
  {
    printf("A=%d ", m_sliceArgument);
  }
  printf("SliceSegment: M=%d ",m_sliceSegmentMode);
  if (m_sliceSegmentMode!=NO_SLICES)
  {
    printf("A=%d ", m_sliceSegmentArgument);
  }
  printf("CIP:%d ", m_bUseConstrainedIntraPred);
  printf("SAO:%d ", (m_bUseSAO)?(1):(0));
  printf("PCM:%d ", (m_usePCM && (1<<m_uiPCMLog2MinSize) <= m_uiMaxCUWidth)? 1 : 0);

#if PER_LAYER_LOSSLESS
  if (m_apcLayerCfg[layer]->m_TransquantBypassEnabledFlag && m_apcLayerCfg[layer]->m_CUTransquantBypassFlagForce)
  {
    printf("TransQuantBypassEnabled: =1");
  }
  else
  {
    printf("TransQuantBypassEnabled:%d ", (m_apcLayerCfg[layer]->m_TransquantBypassEnabledFlag)? 1:0 );
  }
#else
  if (m_TransquantBypassEnabledFlag && m_CUTransquantBypassFlagForce)
  {
    printf("TransQuantBypassEnabled: =1");
  }
  else
  {
    printf("TransQuantBypassEnabled:%d ", (m_TransquantBypassEnabledFlag)? 1:0 );
  }
#endif

  printf("WPP:%d ", (Int)m_useWeightedPred);
  printf("WPB:%d ", (Int)m_useWeightedBiPred);
  printf("PME:%d ", m_log2ParallelMergeLevel);
  const Int iWaveFrontSubstreams = m_entropyCodingSyncEnabledFlag ? (m_iSourceHeight + m_uiMaxCUHeight - 1) / m_uiMaxCUHeight : 1;
  printf(" WaveFrontSynchro:%d WaveFrontSubstreams:%d", m_entropyCodingSyncEnabledFlag?1:0, iWaveFrontSubstreams);
  printf(" ScalingList:%d ", m_useScalingListId );
  printf("TMVPMode:%d ", m_TMVPModeId     );
#if ADAPTIVE_QP_SELECTION
  printf("AQpS:%d", m_bUseAdaptQpSelect   );
#endif

  printf(" SignBitHidingFlag:%d ", m_signDataHidingEnabledFlag);
  printf("RecalQP:%d", m_recalculateQPAccordingToLambda ? 1 : 0 );

#if SVC_EXTENSION
    printf("\n\n");
  }

  printf("SHVC TOOL CFG: ");
  printf("ElRapSliceType: %c-slice ", m_elRapSliceBEnabled ? 'B' : 'P');
  printf("REF_IDX_ME_ZEROMV: %d ", REF_IDX_ME_ZEROMV);
  printf("ENCODER_FAST_MODE: %d ", ENCODER_FAST_MODE);
#if FAST_INTRA_SHVC
  printf("FIS:%d ", m_useFastIntraScalable  );
#endif
#if CGS_3D_ASYMLUT
  printf("CGS: %d CGSMaxOctantDepth: %d CGSMaxYPartNumLog2: %d CGSLUTBit:%d ", m_nCGSFlag, m_nCGSMaxOctantDepth, m_nCGSMaxYPartNumLog2, m_nCGSLUTBit );
  printf("CGSAdaptC:%d ", m_nCGSAdaptiveChroma );
#if R0179_ENC_OPT_3DLUT_SIZE
  printf("CGSSizeRDO:%d ", m_nCGSLutSizeRDO );
#endif
#endif
#endif

  printf("\n\n");

  fflush(stdout);
}

Bool confirmPara(Bool bflag, const TChar* message)
{
  if (!bflag)
  {
    return false;
  }

  printf("Error: %s\n",message);
  return true;
}

#if SVC_EXTENSION
Void TAppEncCfg::printChromaFormat( const ChromaFormat inputChromaFormatIDC, const ChromaFormat chromaFormatIDC )
{
  std::cout << "Input ChromaFormatIDC                  : ";
  switch (inputChromaFormatIDC)
  {
  case CHROMA_400:  std::cout << "4:0:0"; break;
  case CHROMA_420:  std::cout << "4:2:0"; break;
  case CHROMA_422:  std::cout << "4:2:2"; break;
  case CHROMA_444:  std::cout << "4:4:4"; break;
  default:
    std::cerr << "Invalid";
    exit(1);
  }
  std::cout << std::endl;

  std::cout << "Output (internal) ChromaFormatIDC      : ";
  switch (chromaFormatIDC)
  {
  case CHROMA_400:  std::cout << "4:0:0"; break;
  case CHROMA_420:  std::cout << "4:2:0"; break;
  case CHROMA_422:  std::cout << "4:2:2"; break;
  case CHROMA_444:  std::cout << "4:4:4"; break;
  default:
    std::cerr << "Invalid";
    exit(1);
  }
  std::cout << "\n" << std::endl;
}

Void TAppEncCfg::cfgStringToArray(Int **arr, string const cfgString, Int const numEntries, const char* logString)
{
  TChar *tempChar = cfgString.empty() ? NULL : strdup(cfgString.c_str());
  if( numEntries > 0 )
  {
    TChar *arrayEntry;
    Int i = 0;
    *arr = new Int[numEntries];

    if( tempChar == NULL )
    {
      arrayEntry = NULL;
    }
    else
    {
      arrayEntry = strtok( tempChar, " ,");
    }

    while(arrayEntry != NULL)
    {
      if( i >= numEntries )
      {
        printf( "%s: The number of entries specified is larger than the allowed number.\n", logString );
        exit( EXIT_FAILURE );
      }
      *( *arr + i ) = atoi( arrayEntry );
      arrayEntry = strtok(NULL, " ,");
      i++;
    }
    if( i < numEntries )
    {
      printf( "%s: Some entries are not specified.\n", logString );
      exit( EXIT_FAILURE );
    }
  }
  else
  {
    *arr = NULL;
  }

  if( tempChar )
  {
    free( tempChar );
    tempChar = NULL;
  }
}

Bool TAppEncCfg::scanStringToArray(string const cfgString, Int const numEntries, const char* logString, Int * const returnArray)
{
  Int *tempArray = NULL;
  // For all layer sets
  cfgStringToArray( &tempArray, cfgString, numEntries, logString );
  if(tempArray)
  {
    for(Int i = 0; i < numEntries; i++)
    {
      returnArray[i] = tempArray[i];
    }
    delete [] tempArray; tempArray = NULL;
    return true;
  }
  return false;
}

Bool TAppEncCfg::scanStringToArray(string const cfgString, Int const numEntries, const char* logString, std::vector<Int> & returnVector)
{
  Int *tempArray = NULL;
  // For all layer sets
  cfgStringToArray( &tempArray, cfgString, numEntries, logString );
  if(tempArray)
  {
    returnVector.empty();
    for(Int i = 0; i < numEntries; i++)
    {
      returnVector.push_back(tempArray[i]);
    }
    delete [] tempArray; tempArray = NULL;
    return true;
  }
  return false;
}

Void TAppEncCfg::cfgStringToArrayNumEntries(Int **arr, string const cfgString, Int &numEntries, const char* logString)
{
  TChar *tempChar = cfgString.empty() ? NULL : strdup(cfgString.c_str());
  if (numEntries > 0)
  {
    TChar *arrayEntry;
    Int i = 0;
    *arr = new Int[numEntries];

    if (tempChar == NULL)
    {
      arrayEntry = NULL;
    }
    else
    {
      arrayEntry = strtok(tempChar, " ,");
    }
    while (arrayEntry != NULL)
    {
      if (i >= numEntries)
      {
        printf("%s: The number of entries specified is larger than the allowed number.\n", logString);
        exit(EXIT_FAILURE);
      }
      *(*arr + i) = atoi(arrayEntry);
      arrayEntry = strtok(NULL, " ,");
      i++;
    }
    numEntries = i;
    /*
    if (i < numEntries)
    {
      printf("%s: Some entries are not specified.\n", logString);
      exit(EXIT_FAILURE);
    }
    */
  }
  else
  {
    *arr = NULL;
  }

  if (tempChar)
  {
    free(tempChar);
    tempChar = NULL;
  }
}

Bool TAppEncCfg::scanStringToArrayNumEntries(string const cfgString, Int &numEntries, const char* logString, std::vector<Int> & returnVector)
{
  Int *tempArray = NULL;
  numEntries = m_numLayers;
  // For all layer sets
  cfgStringToArrayNumEntries(&tempArray, cfgString, numEntries, logString);
  if (tempArray)
  {
    returnVector.empty();
    for (Int i = 0; i < numEntries; i++)
    {
      returnVector.push_back(tempArray[i]);
    }
    delete[] tempArray; tempArray = NULL;
    return true;
  }
  return false;
}

void TAppEncCfg::getDirFilename(string& filename, string& dir, const string path)
{
  size_t pos = path.find_last_of("\\");
  if(pos != std::string::npos)
  {
    filename.assign(path.begin() + pos + 1, path.end());
    dir.assign(path.begin(), path.begin() + pos + 1);
  }
  else
  {
    pos = path.find_last_of("/");
    if(pos != std::string::npos)
    {
      filename.assign(path.begin() + pos + 1, path.end());
      dir.assign(path.begin(), path.begin() + pos + 1);
    }
    else
    {
      filename = path;
      dir.assign("");
    }
  }
}

/** \param  argc        number of arguments
    \param  argv        array of arguments
    \retval             true when success
 */
Bool TAppEncCfg::parseCfgNumLayersAndInit( Int argc, TChar* argv[] )
{
  po::Options opts;
  opts.addOptions()
  ("c",    po::parseConfigFile, "configuration file name")
  ("NumLayers",                                     m_numLayers,                                             1, "Number of layers to code")
  ;

  po::setDefaults(opts);
  po::ErrorReporter err;
  err.verbose = false;
  po::scanArgv(opts, argc, (const TChar**) argv, err);

  if( m_numLayers <= 0 )
  {
    printf("Wrong number of layers %d\n", m_numLayers);
    return false;
  }

  for( Int layer = 0; layer < m_numLayers; layer++ )
  {
    m_apcLayerCfg[layer] = new TAppEncLayerCfg;
    m_apcLayerCfg[layer]->setAppEncCfg(this);
  }

  return true;
}
#endif //SVC_EXTENSION
//! \}
