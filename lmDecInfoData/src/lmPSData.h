#ifndef lmPSData_h__
#define lmPSData_h__
#include "lmTydef.h"
//参数集合(PS)类
//PS类型;
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
	const lmVarTYPE getValueTypeByName(const std::string& pstr)const;
public://operator
	lmPSData& operator<<(sParam &p) { addParam(p.first, p.second); return *this; };
	lmPSData& operator>>(std::ofstream& out);
public://static;
	static paraTYPE PSTypeIdxByflag(const std::string& strptype);
	static std::string getParamName(const  paraTYPE &t, int n);
	static std::string getPSTypeInString(const  paraTYPE &t);
private:
	paraTYPE mType;
	lmParam mparalist;
	void xadd(const std::string& pstr, const lmVar &d);
	void inti(const std::string& pstr);
	std::map<std::string, lmVarTYPE> psParaName;

};
typedef  std::vector<std::vector<lmPSData>> lmPSList;
#endif // lmPSData_h__

