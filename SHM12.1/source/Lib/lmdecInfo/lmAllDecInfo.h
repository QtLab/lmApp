#ifndef lmAllDecInfo_h__
#define lmAllDecInfo_h__

#include "lmParaData.h"
class TComVPS;
class TComSPS;
class TComPPS;
class ParameterSetManager;
class lmAllDecInfo
{
public:
	//使用单例模式,将所有信息存在这里面;
	static lmAllDecInfo* getInstance();
	~lmAllDecInfo();
	bool getVpsDecInfo(const TComVPS* rcvps);
	bool getSpsDecInfo(const TComSPS* rcsps);
	bool getPpsDecInfo(const TComPPS* rcpps);
	//解码调用输出到文件;
	bool OutputPrintInfo(const std::string &pPath);
	const std::string& txtpath() { return mOutTxtpath; }
	void clearAllData() {
		mVpsInf.clear(); 
		mSpsInf.clear();
		mPpsInf.clear();
	};
	bool isPSReady();
	void getPSInfobyPSM(ParameterSetManager& allPS);
	void getPreDecodeInfo(ParameterSetManager& allPS);
	bool isPreDecReady() {return !mPredec.empty();};
	void outputPreDec();
	void readPreDec();
	void setCachepath(const std::string &pPath);
private:
	lmAllDecInfo();
	static lmAllDecInfo* _instance;
#if 1
	const std::string mOutPreDec = "predec.txt";
	const std::string mOutTxtpath = "dec.txt";
#else
	const std::string mOutPreDec = "..\\..\\cache\\predec.txt";
	const std::string mOutTxtpath = "..\\..\\cache\\dec.txt";
#endif
	std::string sCachePath{};
	std::vector<vpsInfo> mVpsInf;
	std::vector<spsInfo> mSpsInf;
	std::vector<ppsInfo> mPpsInf;
	std::vector<vpsInfo> mPredec;
private:
	void xPrintVps(std::ofstream& pf);
	void xPrintSps(std::ofstream& pf);
	void xPrintPps(std::ofstream& pf);
};
#endif // lmAllDecInfo_h__

