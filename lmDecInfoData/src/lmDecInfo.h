#ifndef lmDecInfo_h__
#define lmDecInfo_h__
#include "lmPSData.h"


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
public:
	void insertps(const lmPSData &pp, bool isPerDec = false);
private:
	lmDecInfo();
	static lmDecInfo* _instance;
	lmPSList mPSDec;
	lmPSList mPSPreDec;
	void readPS(lmPSData &pp,std::ifstream& pf);
	void clearPSList(lmPSList& rpsl);
	void xReadBityInfo();
};
#endif // lmDecInfo_h__

