#ifndef lmDecInfo_h__
#define lmDecInfo_h__
#include "lmPSData.h"
#include "lmStructure.h"
typedef std::vector< std::vector<std::vector<std::vector<int>>>> depthtype;
//һ��ȫ�ֺ��������ڹ����ȽϺ�������sort�㷨ʹ��;
bool in_front(const depthtype::value_type::value_type& f1, const depthtype::value_type::value_type& f2);
//yuv�ļ���Ϣ�б���yuv�ļ�����·��Ϊ�ؼ���;

class lmYUVInfoList
{
public:
	lmYUVInfoList() :mInfoList{} {};
	~lmYUVInfoList() {};
	lmYUVInfoList& operator<<(const lmYUVInfo& pyuv);
	lmYUVInfoList& operator>>(lmYUVInfo& pyuv);
	const lmYUVInfo &getByPath(const std::string &pPath)const;
	 lmYUVInfo &getlast()  {return mInfoList.rbegin()->second;};
	 bool isempty() { return mInfoList.empty(); };
private:
	std::map < std::string, lmYUVInfo > mInfoList;

};
//���վ��������ǲ��õ���ģʽ;
//�Ա��ڲ�ͬ�߳��з���ͬһ������;

class lmDecInfo
{
public:
	//������д����Ϣ�����Ƕ�ȡ��Ϣ�������ϸ�const�޶�;
	//д����������Ԥ����ͽ�����֮��ʹ��;
	static lmDecInfo *getInstanceForChange();
	//��ȡ���������ʹ��;
	const static lmDecInfo *getInstanceForReadonly();
	~lmDecInfo();
	//void readDec();
	void readDec(bool isPreDec=false);
	void setInfoSoluPath(const std::string& pstr);
	void setInfoSoluPath(const std::string& pstr, const std::string& pyuv);
	std::string retSoluPath()const;
	bool preDecFailed() const { return mPSPreDec[paraTYPE::vps].empty(); };
	void getPS(lmPSData &ps,int pl,bool ispre=false)const;
	std::string getyuvPath(int iLayerIdx)const;
	void read_FrameInfo();
	const depthtype::value_type::value_type &getframeCUSplit(int l, int p)const;
public:
	void insertps(const lmPSData &pp, bool isPerDec = false);
private:
	lmDecInfo();
	static lmDecInfo* _instance;
	lmPSList mPSDec;
	lmPSList mPSPreDec;
	void readPS(lmPSData &pp,std::ifstream& pf);
	void clearPSList(lmPSList& rpsl);
	void xReadDepthInfo(int layernum);
	depthtype mDepth;
	//����POC��������;
	void sortByPOC(depthtype::value_type& pf);
};
#endif // lmDecInfo_h__

