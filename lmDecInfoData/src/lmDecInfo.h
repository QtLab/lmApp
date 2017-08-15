#ifndef lmDecInfo_h__
#define lmDecInfo_h__
#include "lmPSData.h"
#include "lmStructure.h"
typedef std::vector< std::vector<std::vector<std::vector<int>>>> depthtype;
//一个全局函数，用于构建比较函数，供sort算法使用;
bool in_front(const depthtype::value_type::value_type& f1, const depthtype::value_type::value_type& f2);
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
	 bool isempty() { return mInfoList.empty(); };
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
	void read_FrameInfo();
	const depthtype::value_type::value_type &getframeCUSplit(int l, int p)const;
public:
	void insertps(const lmPSData &pp, bool isPerDec = false);
private:
	lmDecInfo();
	static lmDecInfo* _instance;
	lmPSList mPSDec;
	lmPSList mPSPreDec;
	void readPS(lmPSData &pp,std::ifstream& pf);
	void clearPSList(lmPSList& rpsl);
	void xReadDepthInfo(int layernum);
	depthtype mDepth;
	//按照POC进行排序;
	void sortByPOC(depthtype::value_type& pf);
};
#endif // lmDecInfo_h__

