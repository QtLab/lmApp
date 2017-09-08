#ifndef lmPSData_h__
#define lmPSData_h__
#include "lmTydef.h"
//��������(PS)��;
//PS����;
enum paraTYPE
{
	psnum = 5,
	vps = 0,
	sps = 1,
	pps = 2,
	frame=3,
	slice=4

};

class lmPSData
{
public:
	explicit lmPSData(const std::string& str) { inti(str);};
	lmPSData(paraTYPE p):mType(p) {};
	~lmPSData() {};
	//��Ӳ����Ͳ���ֵ;
	void addParam(const std::string& pstr,const lmVar &d) { xadd(pstr, d); };
	//���ص�ǰPS������;
	paraTYPE getType()const { return mType; };
	//���ص�ǰPS�ĵ�n��������
	std::string getParamName(int n);
	//���ص�ǰPS��ĳ�������Ĳ���ֵ����;
	const lmVarTYPE getValueTypeByName(const std::string& pstr)const;
	//���ص�ǰPS��ĳ�������Ĳ���ֵ;
	const lmVar& getValueByName(const std::string& pstr)const;
	bool isempty() { return mparalist.empty(); };
	void clearps() { mparalist.clear(); };
public://operator
	//����һ�Բ����Ͳ���ֵ;
	lmPSData& operator<<(sParam &p) { addParam(p.first, p.second); return *this; };
	//��PS������ļ�;
	lmPSData& operator>>(std::ofstream& out);
	//��ʱʹ�úϳɵĸ��Ƹ�ֵ����;
	//const lmPSData& operator=(lmPSData& ps) { mType = ps.mType; return *this; };
public://static;
	//����txt�ļ������ͱ�־λ��Ӧ������; 
	//��Ҫ����txt�Ķ�ȡ;
	static paraTYPE PSTypeIdxByflag(const std::string& strptype);
	//����ĳ������PS�ĵ�n��������
	static std::string getParamName(const  paraTYPE &t, int n);
	//��������������Ӧ�Ĳ��������ַ�;
	static std::string getPSTypeInString(const  paraTYPE &t);
	//���ز�������������Ŀ;
	static paraTYPE getPSNum(){ return paraTYPE::psnum; };
private:
	paraTYPE mType;
	lmParam mparalist;
	void xadd(const std::string& pstr, const lmVar &d);
	void inti(const std::string& pstr);
	std::map<std::string, lmVarTYPE> psParaName{};//�洢��ǰPS�����в������ƺ���������;

};
typedef  std::vector<std::vector<lmPSData>> lmPSList;
#endif // lmPSData_h__

