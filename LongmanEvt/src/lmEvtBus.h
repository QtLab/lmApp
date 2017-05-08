#ifndef lmEvtBus_h__
#define lmEvtBus_h__

#include <QObject>
#include"src\lmTYPE.h"
class lmmodelD;

class lmEvtBus : public QObject
{
	Q_OBJECT


public:
	~lmEvtBus();
	static lmEvtBus* createCon();
public:
	static lmEvtBus* createSingleCon();
	void registerModel(lmmodelD*);
public slots:
	void post(const longmanEvt&);
signals:
	void evtTriggered(Evt_smartptr);
private:
	lmEvtBus();
	static lmEvtBus* m_SingleCon;
};
#endif // lmEvtBus_h__
