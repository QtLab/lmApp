#ifndef lmmodel_h__
#define lmmodel_h__
#include "longmanEvt.h"
#include "lmmodelD.h"
//�ײ����,ͨ�����ί��
class lmmodel
{
public:
	explicit lmmodel(lmEvtBus *evtBus);
	/*~lmmodel();*/
	void subscribeEvt(const std::string&, const CallBackFunc&);
	void unsubcribeEvt(const std::string&);
	void unsubcribeEvt(const longmanEvt&);
	void setModelName(const std::string&);
private:
	lmmodelD _modeld;
	std::string _modelName;
};
#endif // lmmodelD_h__
