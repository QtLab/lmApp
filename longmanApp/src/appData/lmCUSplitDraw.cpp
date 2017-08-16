#include "lmCUSplitDraw.h"

#include "../../../lmDecInfoData/src/lmStructure.h"
lmCUSplitDraw::~lmCUSplitDraw()
{
}

QPixmap * lmCUSplitDraw::lmDraw(QImage &iDrawMap)
{
	QPixmap *mPixMap = c_lmImageDB->lmDraw(iDrawMap);
	imageWidth = mPixMap->width();
	imageHeight = mPixMap->height();
	QPainter cmPainter(mPixMap);
	lmDrawFuction(cmPainter);
	return mPixMap;
}

void lmCUSplitDraw::lmDrawFuction(QPainter &cPainter)
{
	const std::vector<std::vector<int>> &pf = lmDecInfo::getInstanceForReadonly()->getframeCUSplit(mLayer, mPoc);
	std::vector<std::vector<int>> ctupos{};
	lmStructure trs;
	int rIdx = 0;
	int cuPos[2] = {0,0};
	int ctuNum = pf.size();
	int ctuNUmInImageWid = static_cast<int>(imageWidth/double(trs.getLCUSize())+0.5);
	int ctux = 0;
	int ctuy = 0;
	int cuSize = 0;
	int zd = 0;
	for (size_t i = 0; i < pf.size(); i++)
	{
		ctux = trs.getLCUSize()*(i % ctuNUmInImageWid);
		ctuy = trs.getLCUSize()*(i / ctuNUmInImageWid);
		for (size_t j = 0; j < pf[i].size(); j++)
		{
			if (j == 0 )
				{
					rIdx = trs.zIdx2rIdx(0);
					if (pf[i].size()>1)
						zd = abs(pf[i][j + 1] - 0);
					else
						zd = abs(256 - 0);
				}
			else if (j< pf[i].size() - 1)
				{
					rIdx = trs.zIdx2rIdx(pf[i][j]);
					zd = abs(pf[i][j + 1] - pf[i][j]);
				}
			else
			{
				rIdx = trs.zIdx2rIdx(pf[i][j]);
				zd = abs(256 - pf[i][j]);
			}
			cuPos[0] = trs.rIdx2xPel(rIdx);
			cuPos[1] = trs.rIdx2yPel(rIdx);
			cuSize = sqrt(zd)*SIZE_4_4;
			cPainter.setPen(QColor(0, 0, 255));
			cPainter.drawRect(ctux+cuPos[0], ctuy+cuPos[1], cuSize, cuSize);
			//±ê¼ÇCU;
			if (px>=(ctux + cuPos[0])&& py>=(ctuy + cuPos[1])
				&& px<(ctux + cuPos[0]+ cuSize)&& py<(ctuy + cuPos[1] + cuSize))
			{
				cPainter.fillRect(ctux + cuPos[0], ctuy + cuPos[1], cuSize, cuSize, QBrush(QColor(255,0,0,96)));
			}
		}
		cPainter.setPen(Qt::green);
		cPainter.drawRect(ctux, ctuy, trs.getLCUSize(), trs.getLCUSize());
		
	}
}
