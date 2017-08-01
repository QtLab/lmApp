#ifndef lmTydef_h__
#define lmTydef_h__
#include <string>
#include <map>
#include <vector>
#include <fstream>
//���Զ����Լ��ı�����������;
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
	//�չ���;
	lmVar() :valuetype(lmVarTYPE::vnum){ mData.xint = 0; };
	//���캯�����ṩ����ת��;
	lmVar(int pint) :valuetype(lmVarTYPE::intv){ mData.xint = pint; };
	lmVar(const std::string &pstr) :valuetype(lmVarTYPE::str) { mstr = pstr; };
	//lmVar(const char* pch) { mstr = std::string(pch); };
	lmVar(bool pb) :valuetype(lmVarTYPE::boolv) { mData.xb = pb; };
	lmVar(double pd) :valuetype(lmVarTYPE::doublev) { mData.xd = pd; };
	//�ƶ�����,����ô��Ϥ���Ժ���˵;
	//���ƹ���;
	lmVar(const lmVar &rp) { *this = rp.mData; *this = rp.mstr; this->valuetype = rp.valuetype; };
	//��������;
	~lmVar() {};
public:
	//��ֵ;
	const lmVar& operator=(int pint) {
		mData.xint = pint; valuetype = lmVarTYPE::intv; return *this;
	};
	const lmVar& operator=(const std::string &pstr) { mstr = pstr; valuetype = lmVarTYPE::str;  return *this;};
	const lmVar& operator=(bool pb) { mData.xb = pb;  valuetype = lmVarTYPE::boolv; return *this; };
	const lmVar& operator=(double pb) { mData.xd = pb; valuetype = lmVarTYPE::doublev; return *this; };
	//���Ƹ�ֵ;
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

typedef std::map<std::string, lmVar> lmParam;//��������;
typedef lmParam::value_type sParam;//һ�Բ�������;
#endif // lmTydef_h__