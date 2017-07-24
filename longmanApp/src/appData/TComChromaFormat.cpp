#include "TComChromaFormat.h"
//#include "TComPic.h"




//----------------------------------------------------------------------------------------------------------------------

InputColourSpaceConversion stringToInputColourSpaceConvert(const std::string &value, const Bool bIsForward)
{
  if (value.empty() || value=="UNCHANGED")
  {
    return IPCOLOURSPACE_UNCHANGED;
  }
  if (bIsForward)
  {
    if (value=="YCbCrtoYYY")
    {
      return IPCOLOURSPACE_YCbCrtoYYY;
    }
    if (value=="YCbCrtoYCrCb")
    {
      return IPCOLOURSPACE_YCbCrtoYCrCb;
    }
    if (value=="RGBtoGBR")
    {
      return IPCOLOURSPACE_RGBtoGBR;
    }
  }
  else
  {
    if (value=="YCrCbtoYCbCr")
    {
      return IPCOLOURSPACE_YCbCrtoYCrCb;
    }
    if (value=="GBRtoRGB")
    {
      return IPCOLOURSPACE_RGBtoGBR;
    }
  }
  return NUMBER_INPUT_COLOUR_SPACE_CONVERSIONS;
}

std::string getListOfColourSpaceConverts(const Bool bIsForward)
{
  if (bIsForward)
  {
    return "UNCHANGED, YCbCrtoYCrCb, YCbCrtoYYY or RGBtoGBR";
  }
  else
  {
    return "UNCHANGED, YCrCbtoYCbCr or GBRtoRGB";
  }
}


//----------------------------------------------------------------------------------------------------------------------

// Void getTUEntropyCodingParameters(      TUEntropyCodingParameters &result,
//                                         TComTU                    &rTu,
//                                   const ComponentID                component)
// {
//   //------------------------------------------------
// 
//   //set the local parameters
// 
//         TComDataCU    *const pcCU            = rTu.getCU();
//   const TComRectangle &      area            = rTu.getRect(component);
//   const UInt                 uiAbsPartIdx    = rTu.GetAbsPartIdxTU(component);
//   const UInt                 log2BlockWidth  = g_aucConvertToBit[area.width]  + 2;
//   const UInt                 log2BlockHeight = g_aucConvertToBit[area.height] + 2;
//   const ChannelType          channelType     = toChannelType(component);
// 
//   result.scanType = COEFF_SCAN_TYPE(pcCU->getCoefScanIdx(uiAbsPartIdx, area.width, area.height, component));
// 
//   //------------------------------------------------
// 
//   //set the group layout
// 
//   result.widthInGroups  = area.width  >> MLS_CG_LOG2_WIDTH;
//   result.heightInGroups = area.height >> MLS_CG_LOG2_HEIGHT;
// 
//   //------------------------------------------------
// 
//   //set the scan orders
// 
//   const UInt log2WidthInGroups  = g_aucConvertToBit[result.widthInGroups  * 4];
//   const UInt log2HeightInGroups = g_aucConvertToBit[result.heightInGroups * 4];
// 
//   result.scan   = g_scanOrder[ SCAN_GROUPED_4x4 ][ result.scanType ][ log2BlockWidth    ][ log2BlockHeight    ];
//   result.scanCG = g_scanOrder[ SCAN_UNGROUPED   ][ result.scanType ][ log2WidthInGroups ][ log2HeightInGroups ];
// 
//   //------------------------------------------------
// 
//   //set the significance map context selection parameters
// 
//   if (pcCU->getSlice()->getSPS()->getSpsRangeExtension().getTransformSkipContextEnabledFlag()
//       && (pcCU->getCUTransquantBypass(uiAbsPartIdx) || (pcCU->getTransformSkip(uiAbsPartIdx, component) != 0)))
//   {
//     result.firstSignificanceMapContext = significanceMapContextSetStart[channelType][CONTEXT_TYPE_SINGLE];
//   }
//   else
//   {
//     if ((area.width == 4) && (area.height == 4))
//     {
//       result.firstSignificanceMapContext = significanceMapContextSetStart[channelType][CONTEXT_TYPE_4x4];
//     }
//     else if ((area.width == 8) && (area.height == 8))
//     {
//       result.firstSignificanceMapContext = significanceMapContextSetStart[channelType][CONTEXT_TYPE_8x8];
//       if (result.scanType != SCAN_DIAG)
//       {
//         result.firstSignificanceMapContext += nonDiagonalScan8x8ContextOffset[channelType];
//       }
//     }
//     else
//     {
//       result.firstSignificanceMapContext = significanceMapContextSetStart[channelType][CONTEXT_TYPE_NxN];
//     }
//   }
// 
//   //------------------------------------------------
// }
// 

//----------------------------------------------------------------------------------------------------------------------
