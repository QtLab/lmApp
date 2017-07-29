#ifndef lmPSData_h__
#define lmPSData_h__
#include "lmTydef.h"
enum paraTYPE
{
	psnum = 3,
	vps = 0,
	sps = 1,
	pps = 2

};

class lmPSData
{
public:
	explicit lmPSData(const std::string& str) { inti(str);};
	lmPSData(paraTYPE p):mType(p) {};
	~lmPSData() {};
	void addParam(const std::string& pstr,const lmVar &d) { xadd(pstr, d); };
	paraTYPE getType()const { return mType; };
	paraTYPE getPSNum()const {return paraTYPE::psnum;};
	std::string getParamName(int n);
public:
	static std::string getParamName(const  paraTYPE &t,int n);
	lmPSData& operator<<(sParam &p) { addParam(p.first, p.second); return *this; };
	lmPSData& operator>>(std::ofstream& out);
private:
	paraTYPE mType;
	lmParam mparalist;
	void xadd(const std::string& pstr, const lmVar &d);
	void inti(const std::string& pstr);
	std::vector<std::string> psParaName;
};
#endif // lmPSData_h__

