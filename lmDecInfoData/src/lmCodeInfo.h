#ifndef lmCodeInfo_h__
#define lmCodeInfo_h__
#include <vector>
//简单的前向差分;
void gPre_differential_encoding(std::vector<int> &pIn, std::vector<int> &pOut);
//简单的游程编码;
void gRunlength_coding(std::vector<int> &pIn, std::vector<int> &pOut);
//两个数合并为一个Int;
void gMerge2int(std::vector<int> &pIn, std::vector<int> &pOut, int p);
void gCode_zIdx(std::vector<int> &pIn, std::vector<int> &pOut, int p = 7);
//class lmCodeInfo
//{
//public:
//	lmCodeInfo() {};
//	~lmCodeInfo() {};
//};
#endif // lmCodeInfo_h__

