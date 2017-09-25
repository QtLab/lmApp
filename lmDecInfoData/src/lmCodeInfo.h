#ifndef lmCodeInfo_h__
#define lmCodeInfo_h__
//用于存储差分z索引的2次幂的位数,由于差分z索引最大为64，位数=3；
#define diffBit 3
#include <vector>
//简单的前向差分;
void gPre_differential_encoding(std::vector<int> &pIn, std::vector<int> &pOut);
//简单的游程编码;
void gRunlength_coding(std::vector<int> &pIn, std::vector<int> &pOut);
//两个数合并为一个Int;
void gMerge2int(std::vector<int> &pIn, std::vector<int> &pOut);
void gCode_zIdx(std::vector<int> &pIn, std::vector<int> &pOut);
void gdeCode_zIdx(std::vector<int> &pIn, std::vector<int> &pOut);
void gdeMerge2int(std::vector<int> &pIn, std::vector<int> &pOut);
void gdeRunlength_coding(std::vector<int> &pIn, std::vector<int> &pOut);
void gdePre_differential_encoding(std::vector<int> &pIn, std::vector<int> &pOut);
//class lmCodeInfo
//{
//public:
//	lmCodeInfo() {};
//	~lmCodeInfo() {};
//};
#endif // lmCodeInfo_h__

