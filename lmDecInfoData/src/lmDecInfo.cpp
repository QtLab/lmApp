#include "lmDecInfo.h"
static std::string gAbsPath("/");
static std::string gyuvprefix;
const std::string mOutPreDec = "predec.txt";
const std::string mOutTxtpath = "dec.txt";
const std::vector<std::string> mOutTxtFrame = {
	"dec_frame_CU_Split.txt",
	"dec_frame_CTU_Bit.txt" };
lmDecInfo * lmDecInfo::_instance = nullptr;
lmDecInfo::lmDecInfo()
{
	for (size_t i = 0; i < paraTYPE::psnum; i++)
	{
		mPSDec.push_back({});
		mPSPreDec.push_back({});
	}
}


lmDecInfo * lmDecInfo::getInstanceForChange()
{
	if (_instance == nullptr)
		_instance= new lmDecInfo();
	return _instance;
}

const lmDecInfo * lmDecInfo::getInstanceForReadonly()
{
	if (_instance == nullptr)
		_instance = new lmDecInfo();
	return _instance;
}

lmDecInfo::~lmDecInfo()
{
}

void lmDecInfo::readDec(bool isPreDec)
{
	//每次调用都会清空原有数据;
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

void lmDecInfo::setInfoSoluPath(const std::string& pstr, const std::string& pyuv)
{
	setInfoSoluPath(pstr);
	gyuvprefix = pyuv;
}

std::string lmDecInfo::retSoluPath() const
{
	return gAbsPath;
}

void lmDecInfo::getPS(lmPSData &ps, int pl, bool ispre)const
{
	paraTYPE mftype = ps.getType();
	std::vector<lmPSData> psl;
	if (ispre)
		psl = mPSPreDec[mftype];
	else
		psl = mPSDec[mftype]; 
	for (size_t i = 0; i < psl.size(); i++)
	{
		if (psl[i].getValueByName(ps.getParamName(1)).toInt() == pl)
			ps = psl[i];
	}

}

std::string lmDecInfo::getyuvPath(int iLayerIdx)const
{
	std::string str = gAbsPath;
	str += gyuvprefix;
	str = str + std::to_string(iLayerIdx)+".yuv";
	return str;
}

void lmDecInfo::read_FrameInfo()
{
	//把所有信息读取进来;
	xReadBityInfo();
}

void lmDecInfo::insertps(const lmPSData &pp, bool isPerDec)
{
	if (pp.getType() == paraTYPE::psnum)
		return;
	//将PS插入列表;
	if (isPerDec)
		mPSPreDec[pp.getType()].push_back(pp);
	else
		mPSDec[pp.getType()].push_back(pp);
}

void lmDecInfo::readPS(lmPSData &pp, std::ifstream& pf)
{
	//读取PS中的所有参数;
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

void lmDecInfo::xReadBityInfo()
{
	std::ifstream pf(mOutTxtFrame[1], std::ifstream::in);
}

lmYUVInfoList& lmYUVInfoList::operator<<(const lmYUVInfo& pyuv)
{
	std::string pn = pyuv.absyuvPath();
	auto mit=mInfoList.find(pn);
	if (mit==mInfoList.end())
	{
		mInfoList.insert({ pn,pyuv });
	}
	return *this;
}

lmYUVInfoList& lmYUVInfoList::operator>>(lmYUVInfo& pyuv)
{
	std::string pnstr = pyuv.absyuvPath();
	if(pnstr.empty())throw std::runtime_error("NULL Name!");
	auto mit = mInfoList.find(pnstr);
	if (mit == mInfoList.end())
		throw std::runtime_error("No this yuvinfo!");
	auto mit2 = mit->second;
	lmYUVInfo tyuv(pnstr,mit2.getWidth(),mit2.getHeight(), mit2.getFormat(),mit2.getLayer());
	pyuv = tyuv;
	return *this;
}

const lmYUVInfo &lmYUVInfoList::getByPath(const std::string &pPath) const
{
	auto mit = mInfoList.find(pPath);
	if (mit == mInfoList.end())
		throw std::runtime_error("No this yuvinfo!");

	return mit->second;
}
