#ifndef lmAllDecInfo_h__
#define lmAllDecInfo_h__
#include "../../../lmDecInfoData/src/lmPSData.h"
class ParameterSetManager;
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
private:
	lmAllDecInfo();
	static lmAllDecInfo* _instance;
	const std::string mOutPreDec = "predec.txt";
	const std::string mOutTxtpath = "dec.txt";
	lmPSList mpsdec;
};
#endif // lmAllDecInfo_h__

