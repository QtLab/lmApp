#include "lmAllDecInfo.h"
#include "../TLibCommon/TComSlice.h"
#include <fstream>
lmAllDecInfo* lmAllDecInfo::_instance = nullptr;
//CHROMA_400 = 0,
//CHROMA_420 = 1,
//CHROMA_422 = 2,
//CHROMA_444 = 3,
//NUM_CHROMA_FORMAT = 4
void lmAllDecInfo::xPrintVps(std::ofstream& pf)
{
	for (auto i = mVpsInf.begin(); i != mVpsInf.end(); ++i)
	{
		//pf << "\n";
		pf << "[VPS_Begin]" << "\n";
		pf <<"vpsId:" << i->getvpsid() << "\n";
		pf <<"maxLayer:" << i->getmaxlayer() << "\n";
		pf << "[VPS_End]" << "\n";
		//pf << "\n";
	}
}

void lmAllDecInfo::xPrintSps(std::ofstream& pf)
{
	for (auto i = mSpsInf.begin(); i != mSpsInf.end(); ++i)
	{
		//pf << "\n";
		pf << "[SPS_Begin]" << "\n";
		pf << "spsId:" << i->getspsid() << "\n";
		pf << "layerId:" << i->getlayerid() << "\n";
		pf << "formatId:" << i->getformat() <</* "\t"<<"0:400,1:420,2:422,3:444"<<*/"\n";
		pf << "picWidth:" << i->getlumawidth() << "\n";
		pf << "picHeight:" << i->getlumaheight() << "\n";
		pf <<"[SPS_End]" << "\n";
		//pf << "\n";
	}
}

void lmAllDecInfo::xPrintPps(std::ofstream& pf)
{
	for (auto i = mPpsInf.begin(); i != mPpsInf.end(); ++i)
	{
		//pf << "\n";
		pf << "[PPS_Begin]" << "\n";
		pf << "ppsId:" << i->getppsid() << "\n";
		pf << "layerId:" << i->getlayerid() << "\n";
		pf << "[PPS_End]" << "\n";
		//pf << "\n";
	}
}

void lmAllDecInfo::xreadvps(std::ifstream& pf, bool isforPreDec)
{
	if (isforPreDec)
	{
		std::string temps;
		bool readyRead=false;
		while (getline(pf,temps))
		{
			if (temps=="[VPS_Begin]")
			{
				readyRead = true;
			}
			if (temps == "[VPS_End]")
			{
				break;
			}
			if (readyRead)
			{

			}
		}
	}
	else
	{
		int x = 0;
	}
}

bool lmAllDecInfo::isPSReady()
{
	//仅当从最高层级的ParameterSetManager中获得所有PS后;
	//才返回TRUE;
	if (!mVpsInf.empty()&&mSpsInf.size()==mVpsInf[0].getmaxlayer()/*&& mPpsInf.size() == mVpsInf[0].getmaxlayer()*/)
	{
		return true;
	}
	else
	{
		return false;
	}
	
}

void lmAllDecInfo::getPSInfobyPSM(ParameterSetManager& allPS)
{
	if (!isPSReady())
	{
		clearAllData();
		int i = 0;
		TComVPS*      fvps = allPS.getVPS(i);
		while (fvps != nullptr)
		{
			getVpsDecInfo(fvps);
			i++;
			fvps = allPS.getVPS(i);
		}
		i = 0;
		TComSPS*      fsps = allPS.getSPS(i);
		while (fsps != nullptr)
		{
			getSpsDecInfo(fsps);
			i++;
			fsps = allPS.getSPS(i);
		}
		i = 0;
		TComPPS*      fpps = allPS.getPPS(i);
		while (fpps != nullptr)
		{
			getPpsDecInfo(fpps);
			i++;
			fpps = allPS.getPPS(i);
		}
	}
}

void lmAllDecInfo::getPreDecodeInfo(ParameterSetManager& allPS)
{
	int i = 0;
	TComVPS*      fvps = allPS.getVPS(i);
	while (fvps != nullptr)
	{
		vpsInfo mvps;
		mvps.setmaxlayer(fvps->getMaxLayers());
		mvps.setvpsidx(fvps->getVPSId());
		mPredec.push_back(mvps);
		i++;
		fvps = allPS.getVPS(i);
	}
}

void lmAllDecInfo::outputPreDec()
{
	std::ofstream printTXTclear(mOutPreDec, std::ofstream::out);
	printTXTclear << "[PreDec_BEGIN]" << '\n';
	printTXTclear.close();
	std::ofstream printTXTappend(mOutPreDec, std::ofstream::out | std::ofstream::app);
	for (auto i = mPredec.begin(); i != mPredec.end(); ++i)
	{
		//pf << "\n";
		printTXTappend << "[VPS_Begin]" << "\n";
		printTXTappend << "vpsId:" << i->getvpsid() << "\n";
		printTXTappend << "maxLayer:" << i->getmaxlayer() << "\n";
		printTXTappend << "[VPS_End]" << "\n";
		//pf << "\n";
	}
	printTXTappend << "[PreDec_END]" << '\n';

}

void lmAllDecInfo::readPreDec()
{
	std::string predeci = sCachePath + mOutPreDec;
	std::ifstream infotxt(predeci, std::ifstream::in);
	if (infotxt.fail())
		return;
	xreadvps(infotxt, true);
	
}

void lmAllDecInfo::setCachepath(const std::string &pPath)
{
	sCachePath = pPath;
}

lmAllDecInfo::lmAllDecInfo():
	mVpsInf{},
	mSpsInf{},
	mPpsInf{},
	mPredec{}
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
	msps.setformat(rcsps->getChromaFormatIdc());
	msps.setlumaheight(rcsps->getPicHeightInLumaSamples());
	msps.setlumawidth(rcsps->getPicWidthInLumaSamples());
	mSpsInf.push_back(msps);
	return true;
}

bool lmAllDecInfo::getPpsDecInfo(const TComPPS* rcpps)
{
	ppsInfo mPPS;
	mPPS.setppsIdx(rcpps->getPPSId());
	mPPS.setppslayerId(rcpps->getLayerId());
	mPpsInf.push_back(mPPS);
	return true;
}
bool lmAllDecInfo::OutputPrintInfo(const std::string &pPath)
{
	if (!isPSReady())
	{
		return false;
	}
	std::ofstream printTXTclear(pPath,std::ofstream::out);
	printTXTclear << "[PS_BEGIN]" << '\n';
	printTXTclear.close();
	std::ofstream printTXTappend(pPath, std::ofstream::out | std::ofstream::app);
	xPrintVps(printTXTappend);
	xPrintSps(printTXTappend);
	xPrintPps(printTXTappend);
	printTXTappend << "[PS_END]" << '\n';
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