#ifndef lmTydef_h__
#define lmTydef_h__
#include <string>
#include <map>
#include <vector>
//尝试定义自己的变体数据类型;
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
	//构造函数，提供隐形转换;
	lmVar(int pint) { mData.xint = pint; };
	lmVar(const std::string &pstr) { mstr = pstr; };
	lmVar(const char* pch) { mstr = std::string(pch); };
	lmVar(bool pb) { mData.xb = pb; };
	lmVar(double pd) { mData.xd = pd; };
	//移动构造,不怎么熟悉，以后再说;
	//复制构造
	lmVar(const lmVar &rp) { *this = rp.mData; *this = rp.mstr; };
	//析构函数;
	~lmVar() {};
public:
	//赋值;
	const lmVar& operator=(int pint) { mData.xint = pint; return *this; };
	const lmVar& operator=(const std::string &pstr) { mstr = pstr;  return *this;};
	const lmVar& operator=(bool pb) { mData.xb = pb; return *this; };
	const lmVar& operator=(double pb) { mData.xd = pb; return *this; };
	//复制赋值
	const lmVar& operator=(const lmVar &rp) {
		*this = rp.mData; *this = rp.mstr; return *this;};
public:
	int toInt() const{ return mData.xint; };
	bool toBool()const { return mData.xb; };
	double toDouble() const { return mData.xd; };
	std::string toStr() const { return mstr; };
private:
	//
	const lmVar& operator=(const uData &pb) { mData = pb; return *this; };
	uData mData;
	std::string mstr{};
};

typedef std::map<std::string, lmVar> lmParam;//参数容器;
typedef lmParam::value_type sParam;//一对参数类型;
#endif // lmTydef_h__