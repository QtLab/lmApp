#include "lmStructure.h"
#include <memory>
lmStructure::~lmStructure()
{
	delete[] g_auiZscanToRaster;
	delete[] g_auiRasterToZscan;
	delete[] g_auiRasterToPelX;
	delete[] g_auiRasterToPelY;
}

void lmStructure::init()
{
	
	int partSize = mLCUSizeIN_4_4*mLCUSizeIN_4_4;
	g_auiZscanToRaster = new unsigned int[partSize]();
	g_auiRasterToZscan = new unsigned int[partSize]();
	g_auiRasterToPelX = new unsigned int[partSize]();
	g_auiRasterToPelY = new unsigned int[partSize]();
	//memset(g_auiZscanToRaster, 0, sizeof(unsigned int));
	//memset(g_auiRasterToZscan, 0, partSize*sizeof(unsigned int));
	//memset(g_auiRasterToPelX, 0, partSize*sizeof(unsigned int));
	//memset(g_auiRasterToPelY, 0, partSize*sizeof(unsigned int));
	unsigned int *tempData = g_auiZscanToRaster;
	initZscanToRaster(mMaxDepth, 1, 0, tempData);
	initRasterToZscan(mLCUSize, mLCUSize, mMaxDepth);
	initRasterToPelXY(mLCUSize, mLCUSize, mMaxDepth);
	mstate = true;
}

void lmStructure::initZscanToRaster(int iMaxDepth, int iDepth, unsigned int uiStartVal, unsigned int*& rpuiCurrIdx)
{
	int iStride = 1 << (iMaxDepth - 1);

	if (iDepth == iMaxDepth)
	{
		rpuiCurrIdx[0] = uiStartVal;
		rpuiCurrIdx++;
	}
	else
	{
		int iStep = iStride >> iDepth;
		initZscanToRaster(iMaxDepth, iDepth + 1, uiStartVal, rpuiCurrIdx);
		initZscanToRaster(iMaxDepth, iDepth + 1, uiStartVal + iStep, rpuiCurrIdx);
		initZscanToRaster(iMaxDepth, iDepth + 1, uiStartVal + iStep*iStride, rpuiCurrIdx);
		initZscanToRaster(iMaxDepth, iDepth + 1, uiStartVal + iStep*iStride + iStep, rpuiCurrIdx);
	}
}
void lmStructure::initRasterToZscan(unsigned int uiMaxCUWidth, unsigned int uiMaxCUHeight, unsigned int uiMaxDepth)
{
	unsigned int  uiMinCUWidth = uiMaxCUWidth >> (uiMaxDepth - 1);
	unsigned int  uiMinCUHeight = uiMaxCUHeight >> (uiMaxDepth - 1);

	unsigned int  uiNumPartInWidth = (unsigned int)uiMaxCUWidth / uiMinCUWidth;
	unsigned int  uiNumPartInHeight = (unsigned int)uiMaxCUHeight / uiMinCUHeight;

	for (unsigned int i = 0; i < uiNumPartInWidth*uiNumPartInHeight; i++)
	{
		g_auiRasterToZscan[g_auiZscanToRaster[i]] = i;
	}
}
void lmStructure::initRasterToPelXY(unsigned int uiMaxCUWidth, unsigned int uiMaxCUHeight, unsigned int uiMaxDepth)
{
	unsigned int    i;

	unsigned int* uiTempX = &g_auiRasterToPelX[0];
	unsigned int* uiTempY = &g_auiRasterToPelY[0];

	unsigned int  uiMinCUWidth = uiMaxCUWidth >> (uiMaxDepth - 1);
	unsigned int  uiMinCUHeight = uiMaxCUHeight >> (uiMaxDepth - 1);

	unsigned int  uiNumPartInWidth = uiMaxCUWidth / uiMinCUWidth;
	unsigned int  uiNumPartInHeight = uiMaxCUHeight / uiMinCUHeight;

	uiTempX[0] = 0; uiTempX++;
	for (i = 1; i < uiNumPartInWidth; i++)
	{
		uiTempX[0] = uiTempX[-1] + uiMinCUWidth; uiTempX++;
	}
	for (i = 1; i < uiNumPartInHeight; i++)
	{
		memcpy(uiTempX, uiTempX - uiNumPartInWidth, sizeof(unsigned int)*uiNumPartInWidth);
		uiTempX += uiNumPartInWidth;
	}

	for (i = 1; i < uiNumPartInWidth*uiNumPartInHeight; i++)
	{
		uiTempY[i] = (i / uiNumPartInWidth) * uiMinCUWidth;
	}
}