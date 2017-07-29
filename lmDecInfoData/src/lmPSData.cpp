#include "lmPSData.h"
#include <stdexcept>
const static  std::vector<std::vector<std::string>> gParamName = {
	{ "idx","layerIdx" }, //vps
	{ "idx","layerIdx" ,"formatId", "picWidth","picHeight" },//sps
	{ "idx","layerIdx" }//pps
};

std::string  lmPSData::getParamName(const paraTYPE &t, int n)
{
	std::string tr="unknown";
	switch (t)
	{
	case vps:
		if (n<int(gParamName[vps].size()))
			tr = gParamName[vps][n];
		break;
	case sps:
		if (n < int(gParamName[sps].size()))
			tr = gParamName[sps][n];
		break;
	case pps:
		if (n < int(gParamName[pps].size()))
			tr = gParamName[pps][n];
		break;
	default:
		break;
	}
	return tr;
}

std::string lmPSData::getParamName(int n)
{
	if (n<int(psParaName.size()))
	{
		return psParaName[n];
	}
	else
		return "unknown";
	
}

lmPSData& lmPSData::operator>>(std::ofstream& out)
{
	std::string tstr("unknown");
	switch (mType)
	{
	case vps:
		tstr = "VPS";
		break;
	case sps:
		tstr = "SPS";
		break;
	case pps:
		tstr = "PPS";
		break;
	default:
		break;
	}
	out << "[" + tstr + "_Begin]" << '\n';
	for (auto i = mparalist.cbegin(); i != mparalist.cend(); ++i)
	{
		out << i->first << " : ";
		switch (i->second.getValueType())
		{
		case intv:
			out << i->second.toInt();
			break;
		case boolv:
			out << i->second.toInt();
			break;
		case doublev:
			out << i->second.toDouble();
			break;
		case str:
			out << i->second.toStr();
			break;
		default:
			break;
		}
		out << '\n';
	}
	out << "[" + tstr + "_End]" << '\n';
	return *this;
}

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
