#include "lmData.h"
#include "xyuv2rgb.h"
#include <time.h>
ChromaFormat formatmap[] = {
	ChromaFormat::CHROMA_400,
	ChromaFormat::CHROMA_420,
	ChromaFormat::CHROMA_444
};

lmData::lmData()
{
	is_inWindow = false;
	windowsBeg = 0;
	mCurrentImagePOC = 0;
	for (size_t i = 0; i < maxWindowSize; i++)
	{
		mPicYUV[i] = nullptr;
	}
	m_pCurrentImagePtr = nullptr;
	mMargin = 0;
}


lmData::~lmData()
{
	for (size_t i = 0; i < maxWindowSize; i++)
	{
		if (mPicYUV[i])
			delete mPicYUV[i];
		mPicYUV[i] = nullptr;
	}
	if (m_pCurrentImagePtr)
		delete m_pCurrentImagePtr;
}
//读取yuv文件，获取yuv信息，返回读取状态;
int lmData::openyuv_r(const std::string &fileName, Bool bWriteMode, int formatType, int pwidth, int pheight)
{
// 	if (formatType==0)
// 	{
// 		return 1;
// 	}
	closeandclear();
	mwidth = pwidth; mheight = pheight; mformattype = formatType;
	int m_aiPad[2] = { 0 };
	int m_inputBitDepth[MAX_NUM_CHANNEL_TYPE] = { 8 ,8 };
	int m_MSBExtendedBitDepth[MAX_NUM_CHANNEL_TYPE] = { 8 ,8 };
	int m_internalBitDepth[MAX_NUM_CHANNEL_TYPE] = { 8 ,8 };
	int openstatus1=mIOYUV.open(fileName, false, m_inputBitDepth, m_MSBExtendedBitDepth, m_internalBitDepth);
	if (openstatus1<0)
		return openstatus1;
	int openstatus2=mIOYUV.getTotalFrames(mTotalFrames, mwidth, mheight, formatmap[mformattype]);
	if (openstatus2 < 0)
		return openstatus2;
	xAllocaBuffer();
	return 0;
}
//关闭文件句柄关联，清除yuv存储空间;
void lmData::closeandclear()
{
	mIOYUV.close();
	for (size_t i = 0; i < maxWindowSize; ++i)
	{
		if (mPicYUV[i])
		{
			delete mPicYUV[i];
			mPicYUV[i] = nullptr;
		}

	}
	if (m_pCurrentImagePtr)
		{
			delete m_pCurrentImagePtr;
			m_pCurrentImagePtr = nullptr;
		}
}
//计算poc的位置信息，判断是否在存储窗口内;
void lmData::setPOC(int poc, bool forceInWindow)
{
	if (poc<0)
	{
		is_inWindow = false;
		windowsBeg = 0;
		mCurrentImagePOC = 0;
		return;
	}
	poc = poc > mTotalFrames ? mTotalFrames - 1: poc;
	int oldpoc = mCurrentImagePOC;
	int oldwinbeg = int(oldpoc / double(mnumwindows))*mnumwindows;
	bool mInwin = poc>=oldwinbeg&&poc < oldwinbeg + mnumwindows;
	if (mInwin)
		{
			is_inWindow = true;
			mCurrentImagePOC = poc;
			windowsBeg = oldwinbeg;
		}
	else
	{
		windowsBeg = int(poc / double(mnumwindows))*mnumwindows;
		mCurrentImagePOC = poc;
		is_inWindow = false;
	}
	if (forceInWindow)
	{
		is_inWindow = false;
	}
}
//创建内存，读取yuv，转换为RGB;
void lmData::readPic()
{
	if (!is_inWindow)
	{
		int endpoc = windowsBeg + mnumwindows;
		endpoc = endpoc > mTotalFrames ? mTotalFrames-1: endpoc;
		Int       m_aiPad[2] = { 0 };
		lmPicYuv mYuvTrue;
		mYuvTrue.createWithoutCUInfo(mwidth, mheight, formatmap[mformattype]);
		for (size_t i = windowsBeg; i < endpoc; i++)
		{
			mIOYUV.setCurpoc(i);
			mIOYUV.read(&mYuvTrue, mPicYUV[i-windowsBeg],  IPCOLOURSPACE_UNCHANGED, m_aiPad, formatmap[mformattype]);
		}
	}
	int offsetFrame = mCurrentImagePOC - windowsBeg;
	auto yPtr = mPicYUV[offsetFrame]->getBuf(COMPONENT_Y);
	auto uPtr = mPicYUV[offsetFrame]->getBuf(COMPONENT_Cb);
	auto vPtr = mPicYUV[offsetFrame]->getBuf(COMPONENT_Cr);
	if (mformattype)
		xyuv2rgb(yPtr, uPtr, vPtr, m_pCurrentImagePtr,true, mformattype);
	else
		xint2uchar(yPtr, m_pCurrentImagePtr);
}

void lmData::getyuvPtr(Pel *&yp, Pel *& up, Pel*& vp)const
{
	yp = mPicYUV[mCurrentImagePOC - windowsBeg]->getBuf(COMPONENT_Y);
	up = mPicYUV[mCurrentImagePOC - windowsBeg]->getBuf(COMPONENT_Cr);
	vp = mPicYUV[mCurrentImagePOC - windowsBeg]->getBuf(COMPONENT_Cb);
}

//分配内存空间;
void lmData::xAllocaBuffer()
{
	streamoff frameSize = 0;
	UInt wordsize = 1; // default to 8-bit, unless a channel with more than 8-bits is detected.
	for (UInt component = 0; component < getNumberValidComponents(formatmap[mformattype]); component++)
	{
		ComponentID compID = ComponentID(component);
		frameSize += (mwidth >> getComponentScaleX(compID, formatmap[mformattype])) * (mheight >> getComponentScaleY(compID, formatmap[mformattype]));
		if (mIOYUV.getBitDepth() > 8)
		{
			wordsize = 2;
		}
	}
	mnumwindows = MaxMem / frameSize;
	mnumwindows = mnumwindows > maxWindowSize ? maxWindowSize : mnumwindows;
	//mPicYUV = new lmPicYuv *[mnumwindows];
	for (size_t i = 0; i < mnumwindows; i++)
	{
		mPicYUV[i] = new lmPicYuv;
		mPicYUV[i]->createWithoutCUInfo(mwidth, mheight, formatmap[mformattype],true, mMargin, mMargin);
	}
	int stride=mPicYUV[0]->getStride(COMPONENT_Y);
	int heigths = mPicYUV[0]->getTotalHeight(COMPONENT_Y);
	int ch = mformattype ? 3 : 1;
	m_pCurrentImagePtr = new unsigned char[ch * stride * heigths];
	memset(m_pCurrentImagePtr, 0, ch * stride * heigths *sizeof(unsigned char));
	return;
}

void lmData::xyuv2rgb(Pel* src0, Pel* src1, Pel* src2, unsigned char * rgbI)
{
	Double dResult;
	clock_t lBefore = clock();
	int uWidth = mwidth + mMargin;
	int uHeight = mheight + mMargin;
	int mframesizeInPel = uWidth*uHeight;

	long lYOffset, lUVOffset;
	int iY, iU, iV;
	int tempR, tempG, tempB;

	unsigned char * iCurRgbPixelOffset = nullptr;
	for (int y = 0; y < uHeight; ++y)
	{
		for (int x = 0; x < uWidth; ++x)
		{
			lYOffset = uWidth*y + x;
			lUVOffset = uWidth / 2 * (y / 2) + (x / 2);

			iY = src0[lYOffset];
			iU = src1[lUVOffset];
			iV = src2[lUVOffset];
			tempR = iY
				+ 1.402 * iV - 179;
			tempG = iY
				- 0.34414 * iU + 44
				- 0.71414 * iV + 91;
			tempB = iY
				+ 1.772 * iU - 227;


			tempR = (unsigned char)Clip3<int>(0, 255, tempR);
			tempG = (unsigned char)Clip3<int>(0, 255, tempG);
			tempB = (unsigned char)Clip3<int>(0, 255, tempB);
			iCurRgbPixelOffset = rgbI + 3 * (uWidth*y + x);
			*(iCurRgbPixelOffset) = tempR;
			*(iCurRgbPixelOffset + 1) = tempG;
			*(iCurRgbPixelOffset + 2) = tempB;
		}
	}
	dResult = (Double)(clock() - lBefore) / CLOCKS_PER_SEC;
	std::cout << "RGB转换处理时间：" << dResult << "s" << std::endl;
}

void lmData::xyuv2rgb(Pel* src0, Pel* src1, Pel* src2, unsigned char * rgbI, bool fast, int formarType)
{
	//用整数运算替代浮点运算，播放1920*1080能够达到50Hz.
	Double dResult;
	clock_t lBefore = clock();
	int uWidth = mwidth + mMargin;
	int uHeight = mheight + mMargin;
	int mframesizeInPel = uWidth*uHeight;

	long lYOffset, lUVOffset;
	int iY, iU, iV;
	int tempR, tempG, tempB;

	unsigned char * iCurRgbPixelOffset = nullptr;
	for (int y = 0; y < uHeight; ++y)
	{
		for (int x = 0; x < uWidth; ++x)
		{
			lYOffset = uWidth*y + x;
			if (formarType == 1)
				lUVOffset = uWidth / 2 * (y / 2) + (x / 2);
			else if (formarType == 2)
				lUVOffset = lYOffset;
			iY = src0[lYOffset];
			iV = src1[lUVOffset];
			iU = src2[lUVOffset];

			//tempR = iY
			//	+ 1.402 * iV - 179;
			//tempG = iY
			//	- 0.34414 * iU + 44
			//	- 0.71414 * iV + 91;
			//tempB = iY
			//	+ 1.772 * iU - 227;
			tempR = iY + ((RGBRCrI * (iU-128) + HalfShiftValue) >> Shift);
			tempG = iY + ((RGBGCbI * (iV - 128) + RGBGCrI * (iU - 128) + HalfShiftValue) >> Shift);
			tempB = iY + ((RGBBCbI * (iV - 128) + HalfShiftValue) >> Shift);

			//tempR = (unsigned char)Clip3<int>(0, 255, tempR);
			//tempG = (unsigned char)Clip3<int>(0, 255, tempG);
			//tempB = (unsigned char)Clip3<int>(0, 255, tempB);
			if (tempR > 255) tempR = 255; else if (tempR < 0) tempR = 0;
			if (tempG > 255) tempG = 255; else if (tempG < 0) tempG = 0;    // 编译后应该比三目运算符的效率高
			if (tempB > 255) tempB = 255; else if (tempB < 0) tempB = 0;
			iCurRgbPixelOffset = rgbI + 3 * (uWidth*y + x);
			*(iCurRgbPixelOffset) = tempR;
			*(iCurRgbPixelOffset + 1) = tempG;
			*(iCurRgbPixelOffset + 2) = tempB;
		}
	}
	dResult = (Double)(clock() - lBefore) / CLOCKS_PER_SEC;
	std::cout << "RGB转换处理时间：" << dResult << "s" << std::endl;

}

void lmData::xint2uchar(Pel* src, unsigned char * &dst)
{
	int uWidth = mwidth + mMargin;
	int uHeight = mheight + mMargin;
	long lYOffset = 0;
	for (int y = 0; y < uHeight; ++y)
	{
		for (int x = 0; x < uWidth; ++x)
		{
			lYOffset = uWidth*y + x;
			dst[lYOffset] = (unsigned char)Clip3<int>(0, 255, src[lYOffset]);
		}
	}
}

