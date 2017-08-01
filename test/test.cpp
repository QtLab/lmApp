// test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "..\lmDecInfoData\src\lmDecInfo.h"
//用于测试lib项目;

int main()
{
	lmDecInfo tesstp;
	//tesstp.readDec();
	tesstp.readDec(true);
    return 0;
}

