#include "lmPSData.h"
#include <stdexcept>
const static  std::vector<std::vector<std::string>> gParamName = {
	{ "0_MaxLayerIdx","1_layerIdx" }, //vps
	{ "0_idx","1_layerIdx" ,"2_formatId", "3_picWidth","4_picHeight" },//sps
	{ "0_idx","1_layerIdx" }//pps
};
const static  std::vector<std::vector<lmVarTYPE>> gParamValueType = {
	{ lmVarTYPE::intv,lmVarTYPE::intv }, //vps
	{ lmVarTYPE::intv,lmVarTYPE::intv,lmVarTYPE::intv,lmVarTYPE::intv,lmVarTYPE::intv },//sps
	{ lmVarTYPE::intv,lmVarTYPE::intv }//pps
};
const static std::vector<std::string> lmPStypeInString = { "vps","sps","pps" };
std::string  lmPSData::getParamName(const paraTYPE &t, int n)
{
	//std::string tr= gParamName[t][n];
	//switch (t)
	//{
	//case vps:
	//	if (n<int(gParamName[vps].size()))
	//		tr = gParamName[vps][n];
	//	break;
	//case sps:
	//	if (n < int(gParamName[sps].size()))
	//		tr = gParamName[sps][n];
	//	break;
	//case pps:
	//	if (n < int(gParamName[pps].size()))
	//		tr = gParamName[pps][n];
	//	break;
	//default:
	//	break;
	//}
	return  gParamName[t][n];
}

std::string lmPSData::getParamName(int n)
{
	//返回第n个参数名称的字符;
	return gParamName[mType][n];
	//int in = 0;
	//for (auto i = psParaName.cbegin(); i != psParaName.cend(); ++i, ++in)
	//	if (in == n)
	//		return i->first;
	//	return "unknown";
	
}

std::string lmPSData::getPSTypeInString(const paraTYPE &t)
{
	return lmPStypeInString[t];
}

const lmVarTYPE  lmPSData::getValueTypeByName(const std::string& pstr)const
{
	auto i = psParaName.find(pstr);
	if (i != psParaName.cend())
	{
		return i->second;
	}
	return lmVarTYPE::vnum;
}



const lmVar& lmPSData::getValueByName(const std::string& pstr) const
{
	auto i = mparalist.find(pstr);
	if (i != mparalist.cend())
	{
		return i->second;
	}
	throw std::runtime_error("unknown param name!");
}

lmPSData& lmPSData::operator>>(std::ofstream& out)
{
	std::string tstr = lmPStypeInString[mType];
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

paraTYPE  lmPSData::PSTypeIdxByflag(const std::string& strptype)
{
	if (strptype == "[" + lmPStypeInString[paraTYPE::vps] + "_Begin]")
		return paraTYPE::vps;
	if (strptype == "[" + lmPStypeInString[paraTYPE::sps] + "_Begin]")
		return paraTYPE::sps;
	if (strptype == "[" + lmPStypeInString[paraTYPE::pps] + "_Begin]")
		return paraTYPE::pps;
	return paraTYPE::psnum;
}

void lmPSData::xadd(const std::string& pstr, const lmVar &d)
{
	auto i = psParaName.find(pstr);
	if (i == psParaName.cend())
		throw std::runtime_error("unknown param name!");
	mparalist.insert(sParam(pstr,d));
}

void lmPSData::inti(const std::string& pstr)
{
	if (pstr==lmPStypeInString[paraTYPE::vps])
		mType = paraTYPE::vps;
	else if (pstr == lmPStypeInString[paraTYPE::sps])
		mType = paraTYPE::sps;
	else if (pstr == lmPStypeInString[paraTYPE::pps])
		mType = paraTYPE::pps;
	else
	{
		throw std::runtime_error("unknown PS type!");
	}
	std::vector<std::string> ps_para = gParamName[mType];
	std::vector<lmVarTYPE> ps_paraVType = gParamValueType[mType];
	for (auto i = 0; i < ps_para.size(); ++i)
	{
		psParaName.insert({ ps_para[i], ps_paraVType[i] });
	}
}
