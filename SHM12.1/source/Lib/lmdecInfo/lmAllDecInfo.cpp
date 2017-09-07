#include "lmAllDecInfo.h"
#include "../TLibCommon/TComPic.h"
#include "../../../../lmDecInfoData/src/lmCodeInfo.h"
#include <fstream>
const std::string mOutPreDec = "predec.txt";
const std::string mOutTxtpath = "dec.txt";
const std::vector<std::string> mOutTxtFrame = { "dec_frame_CU_Split.txt","dec_frame_CTU_Bit.txt" };
static int outflag = 0;
lmAllDecInfo::lmAllDecInfo() 
{
		for (int i = 0; i < paraTYPE::psnum;i++)
			mpsdec.push_back({});
		for (size_t i = 0; i < mOutTxtFrame.size(); i++)
			std::ofstream tf2(mOutTxtFrame[i], std::ofstream::out);
};

lmAllDecInfo* lmAllDecInfo::_instance = nullptr;

void lmAllDecInfo::xoutCtus(std::ofstream& po,TComPic *mpc,int idx)
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
		//std::vector<std::vector<int>> CU_split{ {} ,{},{},{} };
		//std::vector<int> CTU_Bit{};
		switch (idx)
		{
		case 0:xoutCtu_split(po,pCtu); break;
		case 1:po<<pCtu->getBytes()<<" "; break;
		default:
			break;
		}

	}
	switch (idx)
	{
	case 0: break;
	case 1:po << "\n"; break;
	default:
		break;
	}

}

void lmAllDecInfo::xoutCtu_split(std::ofstream& pfo, TComDataCU* mCtu/*, std::vector<std::vector<int>>& c*/)
{
	int td = 0;
	int skippos = 0;
	int rasPos = 0;
	int ctuRidx= mCtu->getCtuRsAddr();
	//获取Z索引
	std::vector<int> c;
	for (size_t i = 0; i < mCtu->getTotalNumPart();)
	{
		td = int(mCtu->getDepth(i));
		skippos = static_cast<int>( pow(4, 4 - td));
		rasPos = g_auiZscanToRaster[i];
		c.push_back(i);
// 		c[1].push_back(td);
// 		c[2].push_back(g_auiRasterToPelX[rasPos]);
// 		c[3].push_back(g_auiRasterToPelY[rasPos]);
		i += skippos;

	}
	std::vector<int> dzIsx;
	gCode_zIdx(c, dzIsx);
	for (size_t i = 0; i < dzIsx.size(); i++)
	{
		pfo << dzIsx[i] << " ";
	}
	pfo << '\n';
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
	for (size_t i = 0; i < mOutTxtFrame.size(); i++)
	{
		std::ofstream printTXTappend(mOutTxtFrame[i], std::ofstream::out | std::ofstream::app);
		//printTXTappend << "[Frame_Begin]" << '\n';
		//帧层信息;
		lmPSData mfPS(lmPSData::getPSTypeInString(paraTYPE::frame));
		mfPS << sParam(mfPS.getParamName(0), mpcPic->getPOC())
			<< sParam(mfPS.getParamName(1), int(mpcPic->getLayerId()));
// 		printTXTappend << mpcPic->getPOC() << '\n';
// 		printTXTappend << mpcPic->getLayerId() << '\n';
		mfPS >> printTXTappend;
		xoutCtus(printTXTappend, mpcPic,i);

	//	printTXTappend << "[Frame_End]" << '\n';
	}

}

lmAllDecInfo* lmAllDecInfo::getInstance()
{
	if (_instance == nullptr)
	{
		_instance = new lmAllDecInfo();
	}
	return _instance;
}