
#ifndef lmData_h__
#define lmData_h__
#define MaxMem 6144000//2560*1600*1.5*10如何实现动态读取...
#define  maxWindowSize 1
#include "lmYuvIO.h"
#include "..\lmTypeDef.h"
class lmData
{
public:
	lmData();
	~lmData();
	int  openyuv_r(const std::string &fileName, Bool bWriteMode,int formatType, int pwidth, int pheight);
	void closeandclear();
	void setPOC(int poc,bool forceInWindow);
	inline const int getPOC() const { return mCurrentImagePOC; };
	inline const int getTotalFrames() const { return mTotalFrames; };
	void readPic();
	inline unsigned char* getcurrimage() const { return m_pCurrentImagePtr; };
	inline const int getimageWidth() const { return mwidth + mMargin; };
	inline const int getimageHeight() const { return mheight + mMargin; };
	inline const int getformat() const { return mformattype; };
	void getyuvPtr(Pel *&yp, Pel *& up, Pel*& vp)const;
private:
	lmYuvIO mIOYUV;
	lmPicYuv *mPicYUV[maxWindowSize];
	int mTotalFrames;
	int mwidth;
	int mheight;
	int mformattype;
	int mnumwindows;
	int windowsBeg;
	void xAllocaBuffer();
	void xyuv2rgb(Pel* src0, Pel* src1, Pel* src2, unsigned char * rgbI);
	void xyuv2rgb(Pel* src0, Pel* src1, Pel* src2, unsigned char * rgbI,bool fast,int formarType);
	
	void xint2uchar(Pel* src,unsigned char * &dst);
	int mCurrentImagePOC;
	bool is_inWindow;
	unsigned char *m_pCurrentImagePtr;
	int mMargin;
	
};
#endif // lmData_h__

