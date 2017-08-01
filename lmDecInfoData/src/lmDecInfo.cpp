#include "lmDecInfo.h"
std::string gAbsPath("\\");
lmDecInfo::lmDecInfo()
{
	for (size_t i = 0; i < paraTYPE::psnum; i++)
	{
		mPSDec.push_back({});
		mPSPreDec.push_back({});
	}
}


lmDecInfo::~lmDecInfo()
{
}

void lmDecInfo::readDec(bool isPreDec)
{
	//ÿ�ε��ö������ԭ������;
	std::ifstream tf;
	std::string prefalag[2];
	if (isPreDec)
		 {
		 	tf.open(gAbsPath+mOutPreDec, std::ifstream::in);
			prefalag[0] = "[PreDec_BEGIN]";
			prefalag[1] = "[PreDec_END]";
			clearPSList(mPSPreDec);
		 }
	else
		{
			tf.open(gAbsPath + mOutTxtpath, std::ifstream::in);
			prefalag[0] = "[Dec_BEGIN]";
			prefalag[1] = "[Dec_END]";
			clearPSList(mPSDec);
		}
	if (tf.fail())
		return;
	
	bool inDec = false;
	std::string lineintxt;
	while (getline(tf,lineintxt))
	{
		if (lineintxt == prefalag[0])
			inDec = true;
		if (lineintxt == prefalag[1])
			inDec = false;
		if (inDec)
		{
			if (lineintxt== "[" + lmPSData::getPSTypeInString(paraTYPE::vps) + "_Begin]"
				|| lineintxt == "[" + lmPSData::getPSTypeInString(paraTYPE::sps) + "_Begin]"
				|| lineintxt == "[" + lmPSData::getPSTypeInString(paraTYPE::pps) + "_Begin]")
			{
				lmPSData ps(lmPSData::getPSTypeInString(lmPSData::PSTypeIdxByflag(lineintxt)));
				readPS(ps, tf);
				insertps(ps, isPreDec);
			}
		}
	}
}

void lmDecInfo::setInfoSoluPath(const std::string& pstr)
{
	gAbsPath = pstr;
}

void lmDecInfo::insertps(const lmPSData &pp, bool isPerDec)
{
	if (pp.getType() == paraTYPE::psnum)
		return;
	//��PS�����б�;
	if (isPerDec)
		mPSPreDec[pp.getType()].push_back(pp);
	else
		mPSDec[pp.getType()].push_back(pp);
}

void lmDecInfo::readPS(lmPSData &pp, std::ifstream& pf)
{
	//��ȡPS�е����в���;
	std::string ts;
	std::string maohao(":");

	while (getline(pf, ts)&&ts != "[" + lmPSData::getPSTypeInString(pp.getType()) + "_End]")
	{
		std::string sparaname;
		std::string sparavalue;
		bool isvalue = false;
		std::string s;
		for (int i = 0; i < int(ts.size()); ++i)
		{
			s = ts[i];
			if (s==":")
			{
				isvalue = true;
				continue;
			}
			if (s==" ")
				continue;
			if (isvalue)
				sparavalue += s;
			else
				sparaname += s;
		}
		lmVarTYPE mvt = pp.getValueTypeByName(sparaname);
		lmVar mParamValueInVar;
		switch (mvt)
		{
		case intv:
			mParamValueInVar = std::stoi(sparavalue);
			break;
		case boolv:
			mParamValueInVar = std::stoi(sparavalue)!=0;
			break;
		case doublev:
			mParamValueInVar = std::stod(sparavalue);
			break;
		case str:
			mParamValueInVar = sparavalue;
			break;
		default:
			break;
		}
		pp << sParam(sparaname, mParamValueInVar);
	}
	
}

void lmDecInfo::clearPSList(lmPSList& rpsl)
{
	for (auto i = rpsl.begin(); i != rpsl.end(); ++i)
	{
		i->clear();
	}
}
