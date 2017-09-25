#include "lmCodeInfo.h"
void gPre_differential_encoding(std::vector<int> &pIn, std::vector<int> &pOut)
{
	
	int outV = 0;
	int t2 = 0;
	for (size_t i = 0; i < pIn.size(); i++)
	{
		if (i == 0)
		{
			outV = pIn[i];
		}
		else
		{
			outV = pIn[i] - pIn[i - 1];
			t2 = static_cast<int>(log2(outV));
		}
		pOut.push_back(t2);
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

void gMerge2int(std::vector<int> &pIn, std::vector<int> &pOut)
{
	//合并，低位为个数，高位为值;
	int tempv = 0;
	for (size_t i = 0; i < (pIn.size() >> 1); i++)
	{
		tempv = pIn[2 * i + 1];
		tempv = tempv << diffBit;
		tempv += pIn[2 * i];
		pOut.push_back(tempv);
		tempv = 0;
	}
}

void gCode_zIdx(std::vector<int> &pIn, std::vector<int> &pOut)
{
	std::vector<int> tempvec;
	gPre_differential_encoding(pIn, tempvec);
	std::vector<int> tempvec1;
// 	gPre_differential_encoding(tempvec, tempvec1);
// 	std::vector<int> tempvec2;
	gRunlength_coding(tempvec, tempvec1);
	gMerge2int(tempvec1,pOut);
}

void gdeMerge2int(std::vector<int> &pIn, std::vector<int> &pOut)
{
	int trs = 1;
	for (int i = 0; i < diffBit -1; i++)
	{
		trs = (trs << 1) + 1;
	}
	for (int i = 0; i < pIn.size(); i++)
	{
		pOut.push_back(pIn[i] & trs);
		pOut.push_back(pIn[i] >> diffBit);
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
	std::vector<int> tt;
	for (int i = 0; i < pIn.size(); i++)
		if (i == 0)
			tt.push_back(0);
		else
		tt.push_back(static_cast<int>(pow(2, pIn[i])));
	for (int i = 0; i < pIn.size(); i++)
	{
		if (i == 0)
			pOut.push_back(tt[i]);
		else
			pOut.push_back(pOut[i-1]+ tt[i]);
	}
}

void gdeCode_zIdx(std::vector<int> &pIn, std::vector<int> &pOut)
{
	std::vector<int> tempv;
	gdeMerge2int(pIn, tempv);
	std::vector<int> tempv2;
	gdeRunlength_coding(tempv, tempv2);
	gdePre_differential_encoding(tempv2, pOut);
}
