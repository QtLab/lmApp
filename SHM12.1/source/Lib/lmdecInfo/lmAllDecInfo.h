#ifndef lmAllDecInfo_h__
#define lmAllDecInfo_h__
#include "../../../lmDecInfoData/src/lmPSData.h"
class ParameterSetManager;
class TComPic;
class lmAllDecInfo
{
public:
	//ʹ�õ���ģʽ,��������Ϣ����������;
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
	void xoutCtus(std::ofstream& po, TComPic *mpc);
};
#endif // lmAllDecInfo_h__

