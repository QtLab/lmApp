#include "lmAllDecInfo.h"
#include "../TLibCommon/TComSlice.h"
lmAllDecInfo* lmAllDecInfo::_instance = nullptr;
lmAllDecInfo::lmAllDecInfo()
{

}
lmAllDecInfo::~lmAllDecInfo()
{
}

bool lmAllDecInfo::getVpsDecInfo(const TComVPS *rcvps)
{
	vpsInfo mvps;
	mvps.setmaxlayer(rcvps->getMaxLayers());
	mvps.setvpsidx(rcvps->getVPSId());
	mVpsInf.push_back(mvps);
	return true;
}

bool lmAllDecInfo::getSpsDecInfo(const TComSPS* rcsps)
{
	spsInfo msps;
	msps.setspsIdx(rcsps->getSPSId());
	msps.setlayerIdx(rcsps->getLayerId());
	mSpsInf.push_back(msps);
	return true;
}

lmAllDecInfo* lmAllDecInfo::getInstance()
{
	if (_instance == nullptr)
	{
		_instance = new lmAllDecInfo();
	}
	return _instance;
}