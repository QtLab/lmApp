#ifndef lmParaData_h__
#define lmParaData_h__
#include <vector>
class rpsInfo
{
public:
	rpsInfo() :
		rpsIdx(0){};
	~rpsInfo() {};
	void setrpsidx(int p) { rpsIdx = p; };
private:
	int rpsIdx;//rps的Id
};
class vpsInfo
{
public:
	vpsInfo() :
		vpsIdx(0),
		maxlayer(0){};
	~vpsInfo() {};
	void setmaxlayer(int p) { maxlayer = p; };
	void setvpsidx(int p) { vpsIdx = p; };
	int getvpsid()const { return vpsIdx; };
	int getmaxlayer()const { return maxlayer; };
private:
	int vpsIdx;//vps的Id
	int maxlayer;//码流包含的最多层级;
	
};

class spsInfo
{
public:
	spsInfo():
		spsIdx(0),
		layerid(0),
		mFormat(1),
		mLumaWidth(416),
		mLumaHeight(240)
	{};
	~spsInfo() {};
	void setspsIdx(int p) { spsIdx = p; };
	int getspsid()const { return spsIdx; };
	void setlayerIdx(int p) { layerid = p; };
	int getlayerid()const { return layerid; };
	void setformat(int p) { mFormat = p; };
	int getformat()const { return mFormat; };
	void setlumawidth(int p) { mLumaWidth = p; };
	int getlumawidth()const { return mLumaWidth; };
	void setlumaheight(int p) { mLumaHeight = p; };
	int getlumaheight()const { return mLumaHeight; };
	void setbitdepth(int * p) {};
private:
	int spsIdx;//vps的Id
	int layerid;//码流包含的最多层级;
	int mFormat;
	int mLumaWidth;
	int mLumaHeight;
	std::vector<int> mBitDepth{8,8,8};
	std::vector<rpsInfo> mRPSList;
};
class ppsInfo
{
public:
	ppsInfo() :
		ppsIdx(0),
		ppslayerIdx(0){};
	~ppsInfo() {};
	void setppsIdx(int p) { ppsIdx = p; };
	int getppsid()const { return ppsIdx; };
	void setppslayerId(int p) { ppslayerIdx = p; };
	int getlayerid()const { return ppslayerIdx; };
private:
	int ppsIdx;//pps的Id
	int ppslayerIdx;
};
#endif // lmParaData_h__