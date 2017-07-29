#ifndef lmPSData_h__
#define lmPSData_h__
#include "lmTydef.h"
enum paraTYPE
{
	otherv = 4,
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

private:
	paraTYPE mType;
	lmParam mparalist;
	void xadd(const std::string& pstr, const lmVar &d);
	void inti(const std::string& pstr);
	std::vector<std::string> psParaName;
};
#endif // lmPSData_h__

