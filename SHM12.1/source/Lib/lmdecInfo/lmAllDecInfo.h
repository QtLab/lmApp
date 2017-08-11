#ifndef lmAllDecInfo_h__
#define lmAllDecInfo_h__
#include "../../../lmDecInfoData/src/lmPSData.h"
class ParameterSetManager;
class TComPic;
class TComDataCU;
class lmAllDecInfo
{
public:
	//使用单例模式,将所有信息存在这里面;
	static lmAllDecInfo* getInstance();
	~lmAllDecInfo();
	void getPsInfo(const lmPSData& rps);
	bool preDecReady() { return !mpsdec[paraTYPE::vps].empty(); };
	void outputPreDec();
	void outputDec();
public:
	lmAllDecInfo& operator<<(const lmPSData& rps);
	int hasOutputDec();
	void output(TComPic *mpc);
private:
	lmAllDecInfo();
	static lmAllDecInfo* _instance;
	lmPSList mpsdec;
	void xoutCtus(std::ofstream& po, TComPic *mpc,int);
	void xoutCtu_split(std::ofstream& pfo, TComDataCU*mCtu, std::vector<std::vector<int>>&);
	void xoutCtu_Bit(std::ofstream& pfo, TComDataCU*mCtu, std::vector<int>&);
};
#endif // lmAllDecInfo_h__

