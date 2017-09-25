#ifndef lmImageDrawFuc_h__
#define lmImageDrawFuc_h__
//为imagedraw模块实现添加方法的功能;
//继承lmImageDrawBase模块;
//抽象装饰者;
#include "lmImageDrawBase.h"
class lmImageDrawFuc:public lmImageDrawBase
{
public:
	virtual ~lmImageDrawFuc() {};
	lmImageDrawFuc(lmImageDrawBase * pcDrawBase, int pl = 0, int pp = 0, int ppx = 0, int ppy = 0) 
		:c_lmImageDB(pcDrawBase),
		mLayer(pl),
		mPoc(pp),
		px(ppx),
		py(ppy)
		{}
	lmImageDrawBase *c_lmImageDB;
	int mLayer;
	int mPoc;
	int imageWidth = 0;
	int imageHeight = 0;
	int px;
	int py;
};
#endif // lmImageDrawFuc_h__

