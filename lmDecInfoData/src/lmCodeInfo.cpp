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

void gdeMerge2int(std::vector<int> &pIn, std::vector<int> &pOut, int p)
{
	int trs = 1;
	for (int i = 0; i < p-1; i++)
	{
		trs = (trs << 1) + 1;
	}
	for (int i = 0; i < pIn.size(); i++)
	{
		pOut.push_back((pIn[i] >> p) & trs);
		pOut.push_back(pIn[i] & trs);
	}
}

void gdeRunlength_coding(std::vector<int> &pIn, std::vector<int> &pOut)
{
	int num = pIn.size();
	for (int i = 0; i < num; i += 2)
	{
		for (size_t j = 0; j < pIn[i+1]; j++)
		{
			pOut.push_back(pIn[i]);
		}
	}
}

void gdePre_differential_encoding(std::vector<int> &pIn, std::vector<int> &pOut)
{
	for (int i = 0; i < pIn.size(); i++)
	{
		if (i == 0)
			pOut.push_back(pIn[i]);
		else
			pOut.push_back(pOut[i-1]+ pIn[i]);
	}
}

void gdeCode_zIdx(std::vector<int> &pIn, std::vector<int> &pOut)
{
	std::vector<int> tempv;
	gdeMerge2int(pIn, tempv, 7);
	std::vector<int> tempv2;
	gdeRunlength_coding(tempv, tempv2);
	gdePre_differential_encoding(tempv2, pOut);
}
