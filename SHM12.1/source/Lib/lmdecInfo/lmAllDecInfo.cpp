#include "lmAllDecInfo.h"
#include "../TLibCommon/TComSlice.h"
#include <fstream>

lmAllDecInfo::lmAllDecInfo() 
{
		for (int i = 0; i < paraTYPE::psnum;i++)
			mpsdec.push_back({});
};

lmAllDecInfo* lmAllDecInfo::_instance = nullptr;
//CHROMA_400 = 0,
//CHROMA_420 = 1,
//CHROMA_422 = 2,
//CHROMA_444 = 3,
//NUM_CHROMA_FORMAT = 4

lmAllDecInfo::~lmAllDecInfo()
{

}

void lmAllDecInfo::getPsInfo(const lmPSData& rps)
{
	std::vector<lmPSData> tvps{ rps };
	mpsdec[rps.getType()].push_back(rps);
}

void lmAllDecInfo::outputPreDec()
{
	if (!preDecReady())return;
	std::ofstream printTXTclear(mOutPreDec, std::ofstream::out);
	printTXTclear.close();//清除所有原先内容;
	std::ofstream printTXTappend(mOutPreDec, std::ofstream::out | std::ofstream::app);
	printTXTappend << "[PreDec_BEGIN]" << '\n';
	mpsdec[0][0] >> printTXTappend;
	printTXTappend << "[PreDec_END]" << '\n';
}

void lmAllDecInfo::outputDec()
{
	std::ofstream printTXTclear(mOutTxtpath, std::ofstream::out);
	printTXTclear.close();//清除所有原先内容;
	std::ofstream printTXTappend(mOutTxtpath, std::ofstream::out | std::ofstream::app);
	printTXTappend << "[Dec_BEGIN]" << '\n';
	for (auto i = 0; i <mpsdec.size(); ++i)
	{
		for (auto j = 0; j <mpsdec[i].size(); ++j)
		{
			mpsdec[i][j] >> printTXTappend;
		}
	}
	printTXTappend << "[Dec_END]" << '\n';
}

lmAllDecInfo& lmAllDecInfo::operator<<(const lmPSData& rps)
{
	std::vector<lmPSData> tvps{ rps };
	mpsdec[rps.getType()].push_back(rps);
	return *this;
}

lmAllDecInfo* lmAllDecInfo::getInstance()
{
	if (_instance == nullptr)
	{
		_instance = new lmAllDecInfo();
	}
	return _instance;
}