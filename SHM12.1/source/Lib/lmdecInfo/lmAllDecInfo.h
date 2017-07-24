#ifndef lmAllDecInfo_h__
#define lmAllDecInfo_h__

#include "lmParaData.h"
class TComVPS;
class TComSPS;
class lmAllDecInfo
{
public:
	//使用单例模式,将所有信息存在这里面;
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

