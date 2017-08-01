#ifndef lmDecInfo_h__
#define lmDecInfo_h__
#include "lmPSData.h"
class lmDecInfo
{
public:
	lmDecInfo();
	~lmDecInfo();
	//void readDec();
	void readDec(bool isPreDec=false);
	void setInfoSoluPath(const std::string& pstr);
public:
	void insertps(const lmPSData &pp, bool isPerDec = false);
private:
	lmPSList mPSDec;
	lmPSList mPSPreDec;
	std::string mOutPreDec = "predec.txt";
	std::string mOutTxtpath = "dec.txt";
	void readPS(lmPSData &pp,std::ifstream& pf);
	void clearPSList(lmPSList& rpsl);
};
#endif // lmDecInfo_h__

