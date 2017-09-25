/** \file     TComPicYuv.h
    \brief    picture YUV buffer class (header)
*/
//#define _CRT_NONSTDC_NO_WARNING
#ifndef __TCOMPICYUV__
#define __TCOMPICYUV__

#include <stdio.h>
#include "TComChromaFormat.h"

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// picture YUV buffer class
class lmPicYuv
{
public:

  // ------------------------------------------------------------------------------------------------
  //  YUV buffer
  // ------------------------------------------------------------------------------------------------
	UInt g_auiZscanToRaster[MAX_NUM_PART_IDXS_IN_CTU_WIDTH*MAX_NUM_PART_IDXS_IN_CTU_WIDTH];
	UInt g_auiRasterToZscan[MAX_NUM_PART_IDXS_IN_CTU_WIDTH*MAX_NUM_PART_IDXS_IN_CTU_WIDTH];
	UInt g_auiRasterToPelX[MAX_NUM_PART_IDXS_IN_CTU_WIDTH*MAX_NUM_PART_IDXS_IN_CTU_WIDTH];
	UInt g_auiRasterToPelY[MAX_NUM_PART_IDXS_IN_CTU_WIDTH*MAX_NUM_PART_IDXS_IN_CTU_WIDTH];
private:
  Pel*  m_apiPicBuf[MAX_NUM_COMPONENT];             ///< Buffer (including margin)

  Pel*  m_piPicOrg[MAX_NUM_COMPONENT];              ///< m_apiPicBufY + m_iMarginLuma*getStride() + m_iMarginLuma

  // ------------------------------------------------------------------------------------------------
  //  Parameter for general YUV buffer usage
  // ------------------------------------------------------------------------------------------------

  Int   m_picWidth;                                 ///< Width of picture in pixels
  Int   m_picHeight;                                ///< Height of picture in pixels
  ChromaFormat m_chromaFormatIDC;                   ///< Chroma Format

  Int*  m_ctuOffsetInBuffer[MAX_NUM_CHANNEL_TYPE];  ///< Gives an offset in the buffer for a given CTU (and channel)
  Int*  m_subCuOffsetInBuffer[MAX_NUM_CHANNEL_TYPE];///< Gives an offset in the buffer for a given sub-CU (and channel), relative to start of CTU

  Int   m_marginX;                                  ///< margin of Luma channel (chroma's may be smaller, depending on ratio)
  Int   m_marginY;                                  ///< margin of Luma channel (chroma's may be smaller, depending on ratio)

  Bool  m_bIsBorderExtended;

public:
               lmPicYuv         ();
  virtual     ~lmPicYuv         ();

  // ------------------------------------------------------------------------------------------------
  //  Memory management
  // ------------------------------------------------------------------------------------------------

  Void          create            (const Int picWidth,
                                   const Int picHeight,
                                   const ChromaFormat chromaFormatIDC,
                                   const UInt maxCUWidth,  ///< used for generating offsets to CUs.
                                   const UInt maxCUHeight, ///< used for generating offsets to CUs.
                                   const UInt maxCUDepth,  ///< used for generating offsets to CUs.
                                   const Bool bUseMargin);   ///< if true, then a margin of uiMaxCUWidth+16 and uiMaxCUHeight+16 is created around the image.

  Void          createWithoutCUInfo(const Int picWidth,
                                    const Int picHeight,
                                    const ChromaFormat chromaFormatIDC,
                                    const Bool bUseMargin=false, ///< if true, then a margin of uiMaxCUWidth+16 and uiMaxCUHeight+16 is created around the image.
                                    const UInt maxCUWidth=0,   ///< used for margin only
                                    const UInt maxCUHeight=0); ///< used for margin only

  Void          destroy           ();
  // ------------------------------------------------------------------------------------------------
  //  Get information of picture
  // ------------------------------------------------------------------------------------------------

  Int           getWidth          (const ComponentID id) const { return  m_picWidth >> getComponentScaleX(id);   }
  Int           getHeight         (const ComponentID id) const { return  m_picHeight >> getComponentScaleY(id);  }
  ChromaFormat  getChromaFormat   ()                     const { return m_chromaFormatIDC; }
  UInt          getNumberValidComponents() const { return ::getNumberValidComponents(m_chromaFormatIDC); }

  Int           getStride         (const ComponentID id) const { return ((m_picWidth     ) + (m_marginX  <<1)) >> getComponentScaleX(id); }
private:
  Int           getStride         (const ChannelType id) const { return ((m_picWidth     ) + (m_marginX  <<1)) >> getChannelTypeScaleX(id); }
  Void initZscanToRaster(Int iMaxDepth, Int iDepth, UInt uiStartVal, UInt*& rpuiCurrIdx);
  Void initRasterToZscan(UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxDepth);
  Void initRasterToPelXY(UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxDepth);
public:
  Int           getTotalHeight    (const ComponentID id) const { return ((m_picHeight    ) + (m_marginY  <<1)) >> getComponentScaleY(id); }

  Int           getMarginX        (const ComponentID id) const { return m_marginX >> getComponentScaleX(id);  }
  Int           getMarginY        (const ComponentID id) const { return m_marginY >> getComponentScaleY(id);  }

  // ------------------------------------------------------------------------------------------------
  //  Access function for picture buffer
  // ------------------------------------------------------------------------------------------------

  //  Access starting position of picture buffer with margin
  Pel*          getBuf            (const ComponentID ch)       { return  m_apiPicBuf[ch];   }
  const Pel*    getBuf            (const ComponentID ch) const { return  m_apiPicBuf[ch];   }

  //  Access starting position of original picture
  Pel*          getAddr           (const ComponentID ch)       { return  m_piPicOrg[ch];   }
  const Pel*    getAddr           (const ComponentID ch) const { return  m_piPicOrg[ch];   }

  //  Access starting position of original picture for specific coding unit (CU) or partition unit (PU)
  Pel*          getAddr           (const ComponentID ch, const Int ctuRSAddr )       { return m_piPicOrg[ch] + m_ctuOffsetInBuffer[ch==0?0:1][ ctuRSAddr ]; }
  const Pel*    getAddr           (const ComponentID ch, const Int ctuRSAddr ) const { return m_piPicOrg[ch] + m_ctuOffsetInBuffer[ch==0?0:1][ ctuRSAddr ]; }
  Pel*          getAddr           (const ComponentID ch, const Int ctuRSAddr, const Int uiAbsZorderIdx )
                                     { return m_piPicOrg[ch] + m_ctuOffsetInBuffer[ch==0?0:1][ctuRSAddr] + m_subCuOffsetInBuffer[ch==0?0:1][g_auiZscanToRaster[uiAbsZorderIdx]]; }
  const Pel*    getAddr           (const ComponentID ch, const Int ctuRSAddr, const Int uiAbsZorderIdx ) const
                                     { return m_piPicOrg[ch] + m_ctuOffsetInBuffer[ch==0?0:1][ctuRSAddr] + m_subCuOffsetInBuffer[ch==0?0:1][g_auiZscanToRaster[uiAbsZorderIdx]]; }

  UInt          getComponentScaleX(const ComponentID id) const { return ::getComponentScaleX(id, m_chromaFormatIDC); }
  UInt          getComponentScaleY(const ComponentID id) const { return ::getComponentScaleY(id, m_chromaFormatIDC); }

  UInt          getChannelTypeScaleX(const ChannelType id) const { return ::getChannelTypeScaleX(id, m_chromaFormatIDC); }
  UInt          getChannelTypeScaleY(const ChannelType id) const { return ::getChannelTypeScaleY(id, m_chromaFormatIDC); }

  // ------------------------------------------------------------------------------------------------
  //  Miscellaneous
  // ------------------------------------------------------------------------------------------------

  //  Copy function to picture
  Void          copyToPic         ( lmPicYuv*  pcPicYuvDst ) const ;

  //  Extend function of picture buffer
  Void          extendPicBorder   ();

  //  Dump picture
  Void          dump              (const std::string &fileName, const Int fileBitDepth[MAX_NUM_CHANNEL_TYPE], const Bool bAppend=false, const Bool bForceTo8Bit=false) const ;

  // Set border extension flag
  Void          setBorderExtension(Bool b) { m_bIsBorderExtended = b; }
};// END CLASS DEFINITION TComPicYuv


// These functions now return the length of the digest strings.
// UInt calcChecksum(const TComPicYuv& pic, TComPictureHash &digest, const BitDepths &bitDepths);
// UInt calcCRC     (const TComPicYuv& pic, TComPictureHash &digest, const BitDepths &bitDepths);
// UInt calcMD5     (const TComPicYuv& pic, TComPictureHash &digest, const BitDepths &bitDepths);
// std::string hashToString(const TComPictureHash &digest, Int numChar);
//! \}

#endif // __TCOMPICYUV__
