#ifndef lmImageDrawFuc_h__
#define lmImageDrawFuc_h__
//为imagedraw模块实现添加方法的功能;
//继承lmImageDrawBase模块;
//抽象装饰者;
#include "lmImageDrawBase.h"
class lmImageDrawFuc:public lmImageDrawBase
{
public:
	lmImageDrawFuc(lmImageDrawBase * pcDrawBase) { c_lmImageDB = pcDrawBase; };
	~lmImageDrawFuc() { delete c_lmImageDB; };
	lmImageDrawBase *c_lmImageDB;
};
#endif // lmImageDrawFuc_h__

