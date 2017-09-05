#include "lmBitDraw.h"
#include "../../../lmDecInfoData/src/lmStructure.h"
lmBitDraw::~lmBitDraw()
{
}

QPixmap * lmBitDraw::lmDraw(QImage &iDrawMap)
{
	QPixmap *mPixMap = c_lmImageDB->lmDraw(iDrawMap);
	imageWidth = mPixMap->width();
	imageHeight = mPixMap->height();
	QPainter cmPainter(mPixMap);
	lmDrawFuction(cmPainter);
	return mPixMap;
}

void lmBitDraw::lmDrawFuction(QPainter &cPainter)
{
	const std::vector<int> &pBit = lmDecInfo::getInstanceForReadonly()->getframeBit(mLayer, mPoc);
	QFont displayFont;
	QFontMetrics fontMetrics(displayFont);
	lmStructure trs;
	int ctux = 0;
	int ctuy = 0;
	int bit = 0;
	int ctuNum = pBit.size();
	int ctuSize = trs.getLCUSize();
	int ctuNUmInImageWid = static_cast<int>(imageWidth / double(ctuSize) + 0.5);
	int maxBit = 0;
	int b = 0;
	for (size_t i = 1; i < ctuNum; i++)
		if (pBit[i]>maxBit)
			maxBit = pBit[i];
	for (size_t i = 0; i < ctuNum - 1; i++)
	{
		ctux = ctuSize*(i % ctuNUmInImageWid);
		ctuy = ctuSize*(i / ctuNUmInImageWid);
		bit = pBit[i + 1];
		b = (16 + 255 * (bit >> 1) / maxBit);
		cPainter.fillRect(ctux, ctuy, ctuSize, ctuSize, QBrush(QColor(0, 255, 0, b)));

		if (px >= ctux && py >= ctuy
			&& px < (ctux + ctuSize) && py < (ctuy + ctuSize))
		{
			QString strrBit = QString::fromStdString(std::to_string(bit));
			cPainter.setPen(Qt::red);
			cPainter.drawText(ctux + (ctuSize >> 2)-fontMetrics.width(strrBit) / 2, 
				ctuy + (ctuSize >> 2) +fontMetrics.ascent(),
				strrBit);
		}
	}
}
