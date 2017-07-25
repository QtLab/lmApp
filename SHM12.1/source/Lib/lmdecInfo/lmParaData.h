#ifndef lmParaData_h__
#define lmParaData_h__
#include <vector>
class rpsInfo
{
public:
	rpsInfo() :
		rpsIdx(-1) {};
	~rpsInfo(){};
	void setrpsidx(int p) { rpsIdx = p; };
private:
	int rpsIdx;//vps的Id
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
	void setlayerIdx(int p) { layerid = p; };
	void setformat(int p) { mFormat = p; };
	void setlumawidth(int p) { mLumaWidth = p; };
	void setlumaheight(int p) { mLumaHeight = p; };
	void setbitdepth(int * p) {};
private:
	int spsIdx;//vps的Id
	int layerid;//码流包含的最多层级;
	int mFormat;
	int mLumaWidth;
	int mLumaHeight;
	std::vector<int> mBitDepth{8,8,8};
	std::vector<rpsInfo> rpslist;
};
#endif // lmParaData_h__