// test.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "..\lmDecInfoData\src\lmDecInfo.h"
//���ڲ���lib��Ŀ;

int main()
{

	lmYUVInfoList ylist;
	lmYUVInfo myuv1("c//you.yuv",1,2,3);
	
	ylist << myuv1;
	lmYUVInfo myuvt = ylist.getByPath("c//you.yuv");
	
    return 0;
}

