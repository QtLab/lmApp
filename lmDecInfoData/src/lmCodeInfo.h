#ifndef lmCodeInfo_h__
#define lmCodeInfo_h__
//���ڴ洢���z������2���ݵ�λ��,���ڲ��z�������Ϊ64��λ��=3��
#define diffBit 3
#include <vector>
//�򵥵�ǰ����;
void gPre_differential_encoding(std::vector<int> &pIn, std::vector<int> &pOut);
//�򵥵��γ̱���;
void gRunlength_coding(std::vector<int> &pIn, std::vector<int> &pOut);
//�������ϲ�Ϊһ��Int;
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

