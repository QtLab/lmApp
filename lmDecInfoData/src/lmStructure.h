#ifndef lmStructure_h__
#define lmStructure_h__
#include <stdexcept>
#define  SIZE_4_4 4
class lmStructure
{
public:
	lmStructure(int pdepth, int pLCUSzie) :mMaxDepth(pdepth), mLCUSize(pLCUSzie), mLCUSizeIN_4_4(pLCUSzie/ SIZE_4_4){ init(); };
	lmStructure() :mMaxDepth(5), mLCUSize(64), mLCUSizeIN_4_4(64/SIZE_4_4) { init(); };
	~lmStructure();
	unsigned int zIdx2rIdx(unsigned int zI) { if (!mstate) throw std::runtime_error("NULL");	 return g_auiZscanToRaster[zI]; };
	unsigned int rIdx2zIdx(unsigned int zI) { if (!mstate) throw std::runtime_error("NULL");	 return g_auiRasterToZscan[zI]; };
	unsigned int rIdx2xPel(unsigned int zI) { if (!mstate) throw std::runtime_error("NULL");	 return g_auiRasterToPelX[zI]; };
	unsigned int rIdx2yPel(unsigned int zI) { if (!mstate) throw std::runtime_error("NULL");	 return g_auiRasterToPelY[zI]; };
	int getLCUSize() { return mLCUSize; };
private:
	unsigned int mMaxDepth;
	unsigned int mLCUSize;
	unsigned int mLCUSizeIN_4_4;
	bool mstate = false;
	unsigned int *g_auiZscanToRaster/*[SIZE_IN_4_4*SIZE_IN_4_4] = { 0, }*/;
	unsigned int *g_auiRasterToZscan/*[SIZE_IN_4_4*SIZE_IN_4_4] = { 0, }*/;
	unsigned int *g_auiRasterToPelX/*[SIZE_IN_4_4*SIZE_IN_4_4] = { 0, }*/;
	unsigned int *g_auiRasterToPelY/*[SIZE_IN_4_4*SIZE_IN_4_4] = { 0, }*/;
	void init();
	void initZscanToRaster(int iMaxDepth, int iDepth, unsigned int uiStartVal, unsigned int*& rpuiCurrIdx);
	void initRasterToZscan(unsigned int uiMaxCUWidth, unsigned int uiMaxCUHeight, unsigned int uiMaxDepth);
	void initRasterToPelXY(unsigned int uiMaxCUWidth, unsigned int uiMaxCUHeight, unsigned int uiMaxDepth);
};
#endif // lmStructure_h__

