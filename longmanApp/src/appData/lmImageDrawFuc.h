#ifndef lmImageDrawFuc_h__
#define lmImageDrawFuc_h__
//Ϊimagedrawģ��ʵ����ӷ����Ĺ���;
//�̳�lmImageDrawBaseģ��;
//����װ����;
#include "lmImageDrawBase.h"
class lmImageDrawFuc:public lmImageDrawBase
{
public:
	lmImageDrawFuc(lmImageDrawBase * pcDrawBase) { c_lmImageDB = pcDrawBase; };
	~lmImageDrawFuc() { delete c_lmImageDB; };
	lmImageDrawBase *c_lmImageDB;
};
#endif // lmImageDrawFuc_h__

