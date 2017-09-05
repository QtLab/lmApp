#include "lmDecInfo.h"
#include "lmCodeInfo.h"
#include <algorithm>
static std::string gAbsPath;
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
	lmPSData mvps(lmPSData::getPSTypeInString(paraTYPE::vps));
	getPS(mvps, 0,true);
	int mLayerNum = mvps.getValueByName(mvps.getParamName(0)).toInt();
	xReadDepthInfo(mLayerNum);
	xReadBitInfo(mLayerNum);
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
		if (ts== "[" + lmPSData::getPSTypeInString(pp.getType()) + "_Begin]")
		{
			continue;
		}
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

void lmDecInfo::xReadDepthInfo(int layernum)
{
	std::ifstream pf(gAbsPath + mOutTxtFrame[0], std::ifstream::in);
	if (pf.fail())
		return;
	mDepth.clear();
	for (int i = 0; i < layernum; i++)
	{
		//为每层分配空间;
		mDepth.push_back({});
	}
	lmPSData mfps(lmPSData::getPSTypeInString(paraTYPE::frame));
	int layerIdx = 0;
	int ValRead = 0;
	std::string str;
	std::string str1;
	int frmeIdx[8] = {0};
	readPS(mfps, pf);
	
	while (!mfps.isempty())
	{

		layerIdx =mfps.getValueByName(mfps.getParamName(1)).toInt();
		int poc = mfps.getValueByName(mfps.getParamName(0)).toInt();
		//为帧分配空间;
		mDepth[layerIdx].push_back({});
		int ctuNUm = 0;
		std::vector<int> ctu;
		while(getline(pf, str) && str[0] == '1')
			{
				//为CTU分配空间;
				mDepth[layerIdx][frmeIdx[layerIdx]].push_back({});
				for (size_t i = 0; i < str.size(); i++)
				{

						if (str[i] != ' ')
							str1 += str[i];
						else
						{
							ctu.push_back(std::stoi(str1));
							str1.clear();
						}
				}
				auto &out = mDepth[layerIdx][frmeIdx[layerIdx]][ctuNUm];
				gdeCode_zIdx(ctu, out);
				
				out[0] = poc;
				ctuNUm++;
				ctu.clear();

			}
		
		frmeIdx[layerIdx]++;
		mfps.clearps();
		readPS(mfps, pf);
	}
	for (size_t i = 0; i < mDepth.size(); i++)
		std::sort(mDepth[i].begin(), mDepth[i].end(), ::in_front_4LayerVec);
}

void lmDecInfo::xReadBitInfo(int layernum)
{
	std::ifstream pf(gAbsPath + mOutTxtFrame[1], std::ifstream::in);
	if (pf.fail())
		return;
	m_Bit.clear();
	for (int i = 0; i < layernum; i++)
		//为每层分配空间;
		m_Bit.push_back({});
	lmPSData mfps(lmPSData::getPSTypeInString(paraTYPE::frame));
	int layerIdx = 0;
	int ValRead = 0;
	std::string str;
	std::string str1;
	int frmeIdx[8] = { 0 };
	readPS(mfps, pf);
	while (!mfps.isempty())
	{

		layerIdx = mfps.getValueByName(mfps.getParamName(1)).toInt();
		int poc = mfps.getValueByName(mfps.getParamName(0)).toInt();
		//为帧分配空间;
		m_Bit[layerIdx].push_back({});
		m_Bit[layerIdx][frmeIdx[layerIdx]].push_back(poc);
		int ctuNUm = 0;
		int  ctu = 0;
		/*	while (*/getline(pf, str)/*)*/;
/*		{*/
			for (size_t i = 0; i < str.size(); i++)
			{

				if (str[i] != ' ')
					str1 += str[i];
				else
				{

					ctu = std::stoi(str1);
					m_Bit[layerIdx][frmeIdx[layerIdx]].push_back(ctu);
					str1.clear();
				}
			}
			frmeIdx[layerIdx]++;
			mfps.clearps();
			readPS(mfps, pf);
	}
	for (size_t i = 0; i < m_Bit.size(); i++)
		std::sort(m_Bit[i].begin(), m_Bit[i].end(), ::in_front_3LayerVec);
}
bool in_front_4LayerVec(const depthtype::value_type::value_type& f1, const depthtype::value_type::value_type& f2)
{
	return f1[0] < f2[0];
}
bool in_front_3LayerVec(const depthtype::value_type::value_type::value_type& f1, const depthtype::value_type::value_type::value_type& f2)
{
	return f1[0] < f2[0];
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

const depthtype::value_type::value_type & lmDecInfo::getframeCUSplit(int l, int p) const
{
	return mDepth[l][p];
}

const depthtype::value_type::value_type::value_type & lmDecInfo::getframeBit(int l, int p) const
{
	return m_Bit[l][p];
}
