#ifndef __TCOMCHROMAFORMAT__
#define __TCOMCHROMAFORMAT__

#include "CommonDef.h"
#include <iostream>
#include <vector>
#include <assert.h>


static inline ChannelType toChannelType             (const ComponentID id)                         { return (id==COMPONENT_Y)? CHANNEL_TYPE_LUMA : CHANNEL_TYPE_CHROMA; }
static inline Bool        isLuma                    (const ComponentID id)                         { return (id==COMPONENT_Y);                                          }
static inline Bool        isLuma                    (const ChannelType id)                         { return (id==CHANNEL_TYPE_LUMA);                                    }
static inline Bool        isChroma                  (const ComponentID id)                         { return (id!=COMPONENT_Y);                                          }
static inline Bool        isChroma                  (const ChannelType id)                         { return (id!=CHANNEL_TYPE_LUMA);                                    }
static inline UInt        getChannelTypeScaleX      (const ChannelType id, const ChromaFormat fmt) { return (isLuma(id) || (fmt==CHROMA_444)) ? 0 : 1;                  }
static inline UInt        getChannelTypeScaleY      (const ChannelType id, const ChromaFormat fmt) { return (isLuma(id) || (fmt!=CHROMA_420)) ? 0 : 1;                  }
static inline UInt        getComponentScaleX        (const ComponentID id, const ChromaFormat fmt) { return getChannelTypeScaleX(toChannelType(id), fmt);               }
static inline UInt        getComponentScaleY        (const ComponentID id, const ChromaFormat fmt) { return getChannelTypeScaleY(toChannelType(id), fmt);               }
static inline UInt        getNumberValidChannelTypes(const ChromaFormat fmt)                       { return (fmt==CHROMA_400) ? 1 : MAX_NUM_CHANNEL_TYPE;               }
static inline UInt        getNumberValidComponents  (const ChromaFormat fmt)                       { return (fmt==CHROMA_400) ? 1 : MAX_NUM_COMPONENT;                  }
static inline Bool        isChromaEnabled           (const ChromaFormat fmt)                       { return  fmt!=CHROMA_400;                                           }
static inline ComponentID getFirstComponentOfChannel(const ChannelType id)                         { return (isLuma(id) ? COMPONENT_Y : COMPONENT_Cb);                  }

InputColourSpaceConversion stringToInputColourSpaceConvert(const std::string &value, const Bool bIsForward);
std::string getListOfColourSpaceConverts(const Bool bIsForward);

//------------------------------------------------

static inline UInt getTotalSamples(const UInt width, const UInt height, const ChromaFormat format)
{
  const UInt samplesPerChannel = width * height;

  switch (format)
  {
    case CHROMA_400: return  samplesPerChannel;           break;
    case CHROMA_420: return (samplesPerChannel * 3) >> 1; break;
    case CHROMA_422: return  samplesPerChannel * 2;       break;
    case CHROMA_444: return  samplesPerChannel * 3;       break;
    default:
      std::cerr << "ERROR: Unrecognised chroma format in getTotalSamples()" << std::endl;
      exit(1);
      break;
  }

  return MAX_UINT;
}

//------------------------------------------------

static inline UInt getTotalBits(const UInt width, const UInt height, const ChromaFormat format, const Int bitDepths[MAX_NUM_CHANNEL_TYPE])
{
  const UInt samplesPerChannel = width * height;

  switch (format)
  {
    case CHROMA_400: return  samplesPerChannel *  bitDepths[CHANNEL_TYPE_LUMA];                                              break;
    case CHROMA_420: return (samplesPerChannel * (bitDepths[CHANNEL_TYPE_LUMA]*2 +   bitDepths[CHANNEL_TYPE_CHROMA]) ) >> 1; break;
    case CHROMA_422: return  samplesPerChannel * (bitDepths[CHANNEL_TYPE_LUMA]   +   bitDepths[CHANNEL_TYPE_CHROMA]);        break;
    case CHROMA_444: return  samplesPerChannel * (bitDepths[CHANNEL_TYPE_LUMA]   + 2*bitDepths[CHANNEL_TYPE_CHROMA]);        break;
    default:
      std::cerr << "ERROR: Unrecognised chroma format in getTotalSamples()" << std::endl;
      exit(1);
      break;
  }

  return MAX_UINT;
}


//------------------------------------------------


//-----------------------------------------------

static inline UInt getMaxCUDepthOffset(const ChromaFormat chFmt, const UInt quadtreeTULog2MinSize)
{
  return (chFmt==CHROMA_422 && quadtreeTULog2MinSize>2) ? 1 : 0;
}

//======================================================================================================================
//Intra prediction  ====================================================================================================
//======================================================================================================================

static inline Bool filterIntraReferenceSamples (const ChannelType chType, const ChromaFormat chFmt, const Bool intraReferenceSmoothingDisabled)
{
  return (!intraReferenceSmoothingDisabled) && (isLuma(chType) || (chFmt == CHROMA_444));
}


//======================================================================================================================

static inline Int getTransformShift(const Int channelBitDepth, const UInt uiLog2TrSize, const Int maxLog2TrDynamicRange)
{
  return maxLog2TrDynamicRange - channelBitDepth - uiLog2TrSize;
}


//------------------------------------------------

// static inline Int getScaledChromaQP(Int unscaledChromaQP, const ChromaFormat chFmt)
// {
//   return g_aucChromaScale[chFmt][Clip3(0, (chromaQPMappingTableSize - 1), unscaledChromaQP)];
// }


//======================================================================================================================
//Scaling lists  =======================================================================================================
//======================================================================================================================

static inline Int getScalingListType(const PredMode predMode, const ComponentID compID)
{
  return ((predMode != MODE_INTER) ? 0 : MAX_NUM_COMPONENT) + compID;
}
#endif
