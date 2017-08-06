#ifndef lmDecInfo_h__
#define lmDecInfo_h__
#include "lmPSData.h"


//yuv文件信息列表，以yuv文件完整路径为关键字;
class lmYUVInfoList
{
public:
	lmYUVInfoList() :mInfoList{} {};
	~lmYUVInfoList() {};
	lmYUVInfoList& operator<<(const lmYUVInfo& pyuv);
	lmYUVInfoList& operator>>(lmYUVInfo& pyuv);
	const lmYUVInfo &getByPath(const std::string &pPath)const;
	 lmYUVInfo &getlast()  {return mInfoList.rbegin()->second;};
private:
	std::map < std::string, lmYUVInfo > mInfoList;

};
//最终决定，还是采用单例模式;
//以便在不同线程中访问同一份数据;

class lmDecInfo
{
public:
	//对于是写入信息，还是读取信息，进行严格const限定;
	//写入句柄，仅在预解码和解码完之后使用;
	static lmDecInfo *getInstanceForChange();
	//读取句柄，都可使用;
	const static lmDecInfo *getInstanceForReadonly();
	~lmDecInfo();
	//void readDec();
	void readDec(bool isPreDec=false);
	void setInfoSoluPath(const std::string& pstr);
	void setInfoSoluPath(const std::string& pstr, const std::string& pyuv);
	std::string retSoluPath()const;
	bool preDecFailed() const { return mPSPreDec[paraTYPE::vps].empty(); };
	void getPS(lmPSData &ps,int pl,bool ispre=false)const;
	std::string getyuvPath(int iLayerIdx)const;
public:
	void insertps(const lmPSData &pp, bool isPerDec = false);
private:
	lmDecInfo();
	static lmDecInfo* _instance;
	lmPSList mPSDec;
	lmPSList mPSPreDec;
	std::string mOutPreDec = "predec.txt";
	std::string mOutTxtpath = "dec.txt";
	void readPS(lmPSData &pp,std::ifstream& pf);
	void clearPSList(lmPSList& rpsl);
};
#endif // lmDecInfo_h__

