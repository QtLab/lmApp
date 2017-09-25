// test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "..\lmDecInfoData\src\lmDecInfo.h"
//用于测试lib项目;

int main()
{

	//lmYUVInfoList ylist;
	//lmYUVInfo myuv1("c//you.yuv",1,2,3);
	//
	//ylist << myuv1;
	//lmYUVInfo myuvt = ylist.getByPath("c//you.yuv");
	lmStructure xs;
	lmDecInfo *mDec=lmDecInfo::getInstanceForChange();
	mDec->readDec(true);
	if (!mDec->preDecFailed())
		mDec->read_FrameInfo();
    return 0;
}

