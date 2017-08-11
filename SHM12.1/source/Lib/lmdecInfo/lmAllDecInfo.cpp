#include "lmAllDecInfo.h"
#include "../TLibCommon/TComPic.h"
#include <fstream>
const std::string mOutPreDec = "predec.txt";
const std::string mOutTxtpath = "dec.txt";
const std::string mOutTxtFrame = "dec_frame.txt";
static int outflag = 0;
lmAllDecInfo::lmAllDecInfo() 
{
		for (int i = 0; i < paraTYPE::psnum;i++)
			mpsdec.push_back({});
		std::ofstream printTXTclear(mOutTxtFrame, std::ofstream::out);
};

lmAllDecInfo* lmAllDecInfo::_instance = nullptr;

void lmAllDecInfo::xoutCtus(std::ofstream& po, TComPic *mpc)
{
	TComSlice* pcSlice = mpc->getSlice(mpc->getCurrSliceIdx());

	const Int  startCtuTsAddr = pcSlice->getSliceSegmentCurStartCtuTsAddr();
	const Int  startCtuRsAddr = mpc->getPicSym()->getCtuTsToRsAddrMap(startCtuTsAddr);
	const UInt numCtusInFrame = mpc->getNumberOfCtusInFrame();
	const UInt frameWidthInCtus = mpc->getPicSym()->getFrameWidthInCtus();

	for (UInt ctuTsAddr = startCtuTsAddr;ctuTsAddr < numCtusInFrame; ctuTsAddr++)
	{
		const UInt ctuRsAddr = mpc->getPicSym()->getCtuTsToRsAddrMap(ctuTsAddr);
		const TComTile &currentTile = *(mpc->getPicSym()->getTComTile(mpc->getPicSym()->getTileIdxMap(ctuRsAddr)));
		const UInt firstCtuRsAddrOfTile = currentTile.getFirstCtuRsAddr();
		const UInt tileXPosInCtus = firstCtuRsAddrOfTile % frameWidthInCtus;
		const UInt tileYPosInCtus = firstCtuRsAddrOfTile / frameWidthInCtus;
		const UInt ctuXPosInCtus = ctuRsAddr % frameWidthInCtus;
		const UInt ctuYPosInCtus = ctuRsAddr / frameWidthInCtus;

		TComDataCU* pCtu = mpc->getCtu(ctuRsAddr);
		//pCtu->initCtu(mpc, ctuRsAddr);
		for (size_t i = 0; i < pCtu->getTotalNumPart(); i++)
		{
			int td = int(pCtu->getDepth(i));
			switch (td)
			{
			case 0:break;
			case 1:break;
			case 2:break;
			case 3:break;
			default:
				break;
			}
			
			po <<int(pCtu->getDepth(i))<<" ";
			
		}
		po <<"\n";
	}
}

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
	outflag = 0;
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
	++outflag;
}

lmAllDecInfo& lmAllDecInfo::operator<<(const lmPSData& rps)
{
	//std::vector<lmPSData> tvps{ rps };
	mpsdec[rps.getType()].push_back(rps);
	return *this;
}

int lmAllDecInfo::hasOutputDec()
{
	return outflag;
}

void lmAllDecInfo::output(TComPic *mpc)
{
	auto mpcPic = mpc;
	int poc = mpcPic->getPOC();
	int layerId = mpcPic->getLayerId();
	//以追加方式打开;
	std::ofstream printTXTappend(mOutTxtFrame, std::ofstream::out | std::ofstream::app);
	printTXTappend << "[Frame_Begin]" << '\n';
	//帧层信息;
	printTXTappend << mpcPic->getPOC() << '\n';
	printTXTappend << mpcPic->getLayerId() << '\n';
	xoutCtus(printTXTappend, mpcPic);
	printTXTappend << "[Frame_End]" << '\n';

}

lmAllDecInfo* lmAllDecInfo::getInstance()
{
	if (_instance == nullptr)
	{
		_instance = new lmAllDecInfo();
	}
	return _instance;
}