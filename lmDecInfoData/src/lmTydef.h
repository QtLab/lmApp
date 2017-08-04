#ifndef lmTydef_h__
#define lmTydef_h__
#include <string>
#include <map>
#include <vector>
#include <fstream>
//尝试定义自己的变体数据类型;
enum lmVarTYPE
{
	vnum = 4,
	intv = 0,
	boolv = 1,
	doublev = 2,
	str = 3

};
class lmVar
{
private:
	union uData
	{
		int xint;
		bool xb;
		double xd = 0;
	};
public:
	//空构造;
	lmVar() :valuetype(lmVarTYPE::vnum){ mData.xint = 0; };
	//构造函数，提供隐形转换;
	lmVar(int pint) :valuetype(lmVarTYPE::intv){ mData.xint = pint; };
	lmVar(const std::string &pstr) :valuetype(lmVarTYPE::str) { mstr = pstr; };
	//lmVar(const char* pch) { mstr = std::string(pch); };
	lmVar(bool pb) :valuetype(lmVarTYPE::boolv) { mData.xb = pb; };
	lmVar(double pd) :valuetype(lmVarTYPE::doublev) { mData.xd = pd; };
	//移动构造,不怎么熟悉，以后再说;
	//复制构造;
	lmVar(const lmVar &rp) { *this = rp.mData; *this = rp.mstr; this->valuetype = rp.valuetype; };
	//析构函数;
	~lmVar() {};
public:
	//赋值;
	const lmVar& operator=(int pint) {
		mData.xint = pint; valuetype = lmVarTYPE::intv; return *this;
	};
	const lmVar& operator=(const std::string &pstr) { mstr = pstr; valuetype = lmVarTYPE::str;  return *this;};
	const lmVar& operator=(bool pb) { mData.xb = pb;  valuetype = lmVarTYPE::boolv; return *this; };
	const lmVar& operator=(double pb) { mData.xd = pb; valuetype = lmVarTYPE::doublev; return *this; };
	//复制赋值;
	const lmVar& operator=(const lmVar &rp) {
		*this = rp.mData; *this = rp.mstr; this->valuetype = rp.valuetype; return *this;};
public:
	int toInt() const{ return mData.xint; };
	bool toBool()const { return mData.xb; };
	double toDouble() const { return mData.xd; };
	std::string toStr() const { return mstr; };
	const lmVarTYPE & getValueType()const { return valuetype; };
private:
	//
	const lmVar& operator=(const uData &pb) { mData = pb; return *this; };
	uData mData;
	std::string mstr{};
	lmVarTYPE valuetype = lmVarTYPE::vnum;
};

typedef std::map<std::string, lmVar> lmParam;//参数容器;
typedef lmParam::value_type sParam;//一对参数类型;
//一个存储yuv文件信息的类;
class lmYUVInfo
{
public:
	lmYUVInfo(const std::string pa, int pw, int ph, int pf, int pl = 0,int ppco=0) :
		mWidth(pw),
		mHeight(ph),
		mFormat(pf),
		mName(),
		lmabsyuvPath{ pa },
		mLayer(pl)
	{
		if (pw < 0 || ph < 0 || pf<CHROMA_400 || pf>CHROMA_444)
			throw std::runtime_error("attention!");
		mName = xGetNameFromabsPath(pa);
	};
	lmYUVInfo() :mWidth(0),
		mHeight(0),
		mFormat(1),
		mName{},
		lmabsyuvPath{"//"},
		mLayer(0) {};
	lmYUVInfo(std::string pabsPath) :mWidth(0),
		mHeight(0),
		mFormat(1),
		mName(),
		lmabsyuvPath{ pabsPath },
		mLayer(0) {mName = xGetNameFromabsPath(pabsPath); };
	~lmYUVInfo() {};
	const std::string & getName()const { return mName; };
	const std::string & absyuvPath()const { return lmabsyuvPath; };
	const int getWidth()const { return mWidth; };
	const int getHeight()const { return mHeight; };
	const int getFormat()const { return mFormat; };
	const int getLayer()const { return mLayer; };
	const int getPoc()const { return mpoc; };
	void setPOC(int pc) { mpoc = pc; };
	void setLayer(int pl) { mLayer = pl; }
public:
	enum yuvFormat
	{
		CHROMA_400 = 0,
		CHROMA_420 = 1,
		CHROMA_422 = 2,
		CHROMA_444 = 3,
		NUM_CHROMA_FORMAT = 4

	};
private:
	int mWidth;
	int mHeight;
	int mFormat;
	int mLayer;
	int mpoc = 0;
	std::string mName;
	std::string lmabsyuvPath;
	std::string xGetNameFromabsPath(const std::string& pstr) 
	{
		std::string tstr;
		std::string tstrname;
		for (auto i = pstr.rbegin(); i!= pstr.rend(); ++i)
		{
			tstr = (*i);
			if (tstr=="/")
				break;
			tstrname.push_back(*i);
		}
		if (tstrname.empty())
			throw std::runtime_error("attention!");
		std::string tstrnamer;
		for (auto i = tstrname.rbegin(); i != tstrname.rend(); ++i)
			tstrnamer.push_back(*i);
		return tstrnamer;
	};
	
};

#endif // lmTydef_h__