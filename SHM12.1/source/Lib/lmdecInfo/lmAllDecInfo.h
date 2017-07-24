#ifndef lmAllDecInfo_h__
#define lmAllDecInfo_h__

#include "lmParaData.h"
class TComVPS;
class TComSPS;
class lmAllDecInfo
{
public:
	//ʹ�õ���ģʽ,��������Ϣ����������;
	static lmAllDecInfo* getInstance();
	~lmAllDecInfo();
	bool getVpsDecInfo(const TComVPS* rcvps);
	bool getSpsDecInfo(const TComSPS* rcsps);
private:
	lmAllDecInfo();
	static lmAllDecInfo* _instance;
	std::vector<vpsInfo> mVpsInf;
	std::vector<spsInfo> mSpsInf;
};
#endif // lmAllDecInfo_h__

