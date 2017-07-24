/** \file     TVideoIOYuv.h
    \brief    YUV file I/O class (header)
*/

#ifndef __TVIDEOIOYUV__
#define __TVIDEOIOYUV__

#include <stdio.h>
#include <fstream>
#include <iostream>
#include "lmPicYuv.h"
using namespace std;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// YUV file I/O class
class lmYuvIO
{
private:
  fstream   m_cHandle; ///< file handle
  Int       m_fileBitdepth[MAX_NUM_CHANNEL_TYPE]; ///< bitdepth of input/output video file
  Int       m_MSBExtendedBitDepth[MAX_NUM_CHANNEL_TYPE];  ///< bitdepth after addition of MSBs (with value 0)
  Int       m_bitdepthShift[MAX_NUM_CHANNEL_TYPE];  ///< number of bits to increase or decrease image by before/after write/read

public:
  lmYuvIO()           {}
  virtual ~lmYuvIO()  {}
  const int getBitDepth()const {
	  return m_fileBitdepth[0];
  };
  int  open  ( const std::string &fileName, Bool bWriteMode, const Int fileBitDepth[MAX_NUM_CHANNEL_TYPE], const Int MSBExtendedBitDepth[MAX_NUM_CHANNEL_TYPE], const Int internalBitDepth[MAX_NUM_CHANNEL_TYPE] ); ///< open or create file
  Void  close ();                                           ///< close file
  int getTotalFrames(int &numFrames, int width, int height, ChromaFormat format);
  void setCurpoc(int);
  // if fileFormat<NUM_CHROMA_FORMAT, the format of the file is that format specified, else it is the format of the TComPicYuv.


  // If fileFormat=NUM_CHROMA_FORMAT, use the format defined by pPicYuvTrueOrg
  Bool  read  ( lmPicYuv* pPicYuv, lmPicYuv* pPicYuvTrueOrg, const InputColourSpaceConversion ipcsc, Int aiPad[2], ChromaFormat fileFormat=NUM_CHROMA_FORMAT, const Bool bClipToRec709=false );     ///< read one frame with padding parameter

  // If fileFormat=NUM_CHROMA_FORMAT, use the format defined by pPicYuv
  Bool  write ( lmPicYuv* pPicYuv, const InputColourSpaceConversion ipCSC, Int confLeft=0, Int confRight=0, Int confTop=0, Int confBottom=0, ChromaFormat fileFormat=NUM_CHROMA_FORMAT, const Bool bClipToRec709=false );     ///< write one YUV frame with padding parameter

  // If fileFormat=NUM_CHROMA_FORMAT, use the format defined by pPicYuvTop and pPicYuvBottom
  Bool  write ( lmPicYuv* pPicYuvTop, lmPicYuv* pPicYuvBottom, const InputColourSpaceConversion ipCSC, Int confLeft=0, Int confRight=0, Int confTop=0, Int confBottom=0, ChromaFormat fileFormat=NUM_CHROMA_FORMAT, const Bool isTff=false, const Bool bClipToRec709=false);
  static Void ColourSpaceConvert(const lmPicYuv &src, lmPicYuv &dest, const InputColourSpaceConversion conversion, Bool bIsForwards);

  Bool  isEof ();                                           ///< check for end-of-file
  Bool  isFail();                                           ///< check for failure
private:
	streamoff frameSize;

};

#endif // __TVIDEOIOYUV__

