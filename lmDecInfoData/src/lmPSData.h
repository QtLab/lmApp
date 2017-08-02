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
	//添加参数和参数值;
	void addParam(const std::string& pstr,const lmVar &d) { xadd(pstr, d); };
	//返回当前PS的类型;
	paraTYPE getType()const { return mType; };
	//返回当前PS的第n个参数名
	std::string getParamName(int n);
	//返回当前PS的某个参数的参数值类型;
	const lmVarTYPE getValueTypeByName(const std::string& pstr)const;
	//返回当前PS的某个参数的参数值;
	const lmVar& getValueByName(const std::string& pstr)const;
public://operator
	//存入一对参数和参数值;
	lmPSData& operator<<(sParam &p) { addParam(p.first, p.second); return *this; };
	//将PS输出到文件;
	lmPSData& operator>>(std::ofstream& out);
	//暂时使用合成的复制赋值函数;
	//const lmPSData& operator=(lmPSData& ps) { mType = ps.mType; return *this; };
public://static;
	//返回txt文件中类型标志位对应的类型;
	//主要用于txt的读取;
	static paraTYPE PSTypeIdxByflag(const std::string& strptype);
	//返回某个类型PS的第n个参数名
	static std::string getParamName(const  paraTYPE &t, int n);
	//返回输入索引对应的参数类型字符;
	static std::string getPSTypeInString(const  paraTYPE &t);
	//返回参数类型种类数目;
	static paraTYPE getPSNum(){ return paraTYPE::psnum; };
private:
	paraTYPE mType;
	lmParam mparalist;
	void xadd(const std::string& pstr, const lmVar &d);
	void inti(const std::string& pstr);
	std::map<std::string, lmVarTYPE> psParaName{};//存储当前PS的所有参数名称和数据类型;

};
typedef  std::vector<std::vector<lmPSData>> lmPSList;
#endif // lmPSData_h__

