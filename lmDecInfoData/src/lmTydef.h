#ifndef lmTydef_h__
#define lmTydef_h__
#include <string>
#include <map>
#include <vector>
//���Զ����Լ��ı�����������;
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
	//���캯�����ṩ����ת��;
	lmVar(int pint) { mData.xint = pint; };
	lmVar(const std::string &pstr) { mstr = pstr; };
	lmVar(const char* pch) { mstr = std::string(pch); };
	lmVar(bool pb) { mData.xb = pb; };
	lmVar(double pd) { mData.xd = pd; };
	//�ƶ�����,����ô��Ϥ���Ժ���˵;
	//���ƹ���
	lmVar(const lmVar &rp) { *this = rp.mData; *this = rp.mstr; };
	//��������;
	~lmVar() {};
public:
	//��ֵ;
	const lmVar& operator=(int pint) { mData.xint = pint; return *this; };
	const lmVar& operator=(const std::string &pstr) { mstr = pstr;  return *this;};
	const lmVar& operator=(bool pb) { mData.xb = pb; return *this; };
	const lmVar& operator=(double pb) { mData.xd = pb; return *this; };
	//���Ƹ�ֵ
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

typedef std::map<std::string, lmVar> lmParam;//��������;
typedef lmParam::value_type sParam;//һ�Բ�������;
#endif // lmTydef_h__