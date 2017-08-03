#ifndef lmDecInfo_h__
#define lmDecInfo_h__
#include "lmPSData.h"
//最终决定，还是采用单例模式;
//以便在不同线程中访问同一份数据;

class lmDecInfo
{
public:
	//对于是写入信息，还是读取信息，进行严格const限定;
	static lmDecInfo *getInstanceForChange();
	const static lmDecInfo *getInstanceForReadonly();
	~lmDecInfo();
	//void readDec();
	void readDec(bool isPreDec=false);
	void setInfoSoluPath(const std::string& pstr);
	std::string retSoluPath()const;
	bool preDecFailed() const { return mPSPreDec[paraTYPE::vps].empty(); };
	void getPS(lmPSData &ps,int pl,bool ispre=false)const;
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

