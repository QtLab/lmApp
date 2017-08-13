#include "lmCodeInfo.h"
void gPre_differential_encoding(std::vector<int> &pIn, std::vector<int> &pOut)
{
	
	int outV = 0;
	for (size_t i = 0; i < pIn.size(); i++)
	{
		if (i == 0)
		{
			outV = pIn[i];
		}
		else
		{
			outV = pIn[i] - pIn[i - 1];
		}
		pOut.push_back(outV);
	}
};

void gRunlength_coding(std::vector<int> &pIn, std::vector<int> &pOut)
{
	int mc = 1;
	for (size_t i = 0; i < pIn.size(); i++)
	{

		if (i == pIn.size() - 1 || pIn[i] != pIn[i + 1])
		{
			pOut.push_back(pIn[i]);	//0到64，7bit;
			pOut.push_back(mc);		//0到64，7bit;
			mc = 1;
		}
		else
		{
			mc++;
		}
	}
}

void gMerge2int(std::vector<int> &pIn, std::vector<int> &pOut, int p /*= 7*/)
{
	if (p > 7)throw std::runtime_error("halt of sizeof(int)");
	int tempv = 0;//前7位为差分索引，后7位为个数;
	for (size_t i = 0; i < (pIn.size() >> 1); i++)
	{
		tempv = pIn[2 * i];
		tempv = tempv << p;
		tempv += pIn[2 * i + 1];
		pOut.push_back(tempv);
		tempv = 0;
	}
}

void gCode_zIdx(std::vector<int> &pIn, std::vector<int> &pOut, int p /*= 7*/)
{
	std::vector<int> tempvec;
	gPre_differential_encoding(pIn, tempvec);
	std::vector<int> tempvec1;
// 	gPre_differential_encoding(tempvec, tempvec1);
// 	std::vector<int> tempvec2;
	gRunlength_coding(tempvec, tempvec1);
	gMerge2int(tempvec1,pOut,p);
}
