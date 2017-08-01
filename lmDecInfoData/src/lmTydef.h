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
#endif // lmTydef_h__