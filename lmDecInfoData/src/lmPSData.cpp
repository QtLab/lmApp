#include "lmPSData.h"
#include <stdexcept>
const std::vector<std::vector<std::string>> gParamName = {
	{ "idx","layerIdx" }, //vps
	{ "idx","layerIdx" ,"formatId", "picWidth","picHeight" },//sps
	{ "idx","layerIdx" }//pps
};
void lmPSData::xadd(const std::string& pstr, const lmVar &d)
{
	std::vector<std::string> &pspn = psParaName;
	auto i = pspn.cbegin();
	for (; i != pspn.cend(); ++i)
	{
		if ((*i)== pstr)
		{
			break;
		}
	}
	//添加的参数名不符合要求，终止编译;
	if (i == pspn.cend())
	{
		throw std::runtime_error("unknown param name!");
	}
	mparalist.insert(sParam(pstr,d));
}

void lmPSData::inti(const std::string& pstr)
{
	if (pstr=="vps")
		mType = paraTYPE::vps;
	else if (pstr == "sps")
		mType = paraTYPE::sps;
	else if (pstr == "pps")
		mType = paraTYPE::pps;
	else
	{
		throw std::runtime_error("unknown PS type!");
	}
	psParaName = gParamName[mType];
}
