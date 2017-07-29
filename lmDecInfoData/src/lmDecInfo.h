#ifndef lmDecInfo_h__
#define lmDecInfo_h__
#include "lmPSData.h"
class lmDecInfo
{
public:
	lmDecInfo();
	~lmDecInfo();
private:
	std::vector<lmPSData> mps;
};
#endif // lmDecInfo_h__

