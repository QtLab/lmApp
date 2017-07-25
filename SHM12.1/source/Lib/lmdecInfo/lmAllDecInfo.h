#ifndef lmAllDecInfo_h__
#define lmAllDecInfo_h__

#include "lmParaData.h"
class TComVPS;
class TComSPS;
class TComPPS;
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
private:
	lmAllDecInfo();
	static lmAllDecInfo* _instance;
	const std::string mOutTxtpath = "..\\..\\cache\\decinfo.txt";
	std::vector<vpsInfo> mVpsInf;
	std::vector<spsInfo> mSpsInf;
	std::vector<ppsInfo> mPpsInf;
private:
	void xPrintVps(std::ofstream& pf);
	void xPrintSps(std::ofstream& pf);
	void xPrintPps(std::ofstream& pf);
	bool isReadyOut();
};
#endif // lmAllDecInfo_h__

