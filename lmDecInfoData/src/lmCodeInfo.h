#ifndef lmCodeInfo_h__
#define lmCodeInfo_h__
#include <vector>
//�򵥵�ǰ����;
void gPre_differential_encoding(std::vector<int> &pIn, std::vector<int> &pOut);
//�򵥵��γ̱���;
void gRunlength_coding(std::vector<int> &pIn, std::vector<int> &pOut);
//�������ϲ�Ϊһ��Int;
void gMerge2int(std::vector<int> &pIn, std::vector<int> &pOut, int p);
void gCode_zIdx(std::vector<int> &pIn, std::vector<int> &pOut, int p = 7);
//class lmCodeInfo
//{
//public:
//	lmCodeInfo() {};
//	~lmCodeInfo() {};
//};
#endif // lmCodeInfo_h__

