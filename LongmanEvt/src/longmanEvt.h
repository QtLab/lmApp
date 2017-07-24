#ifndef longmanEvt_h__
#define longmanEvt_h__
#define  EvtTYPE1 "_VIEW_UPDATE_EVENT_TYPE_"
#define  EvtTYPE2 "_EXE_COMMAND_EVENT_TYPE_"
//����Event��Ĺ���;
//�¼����ͣ�(Ŀǰ)����EvtTYPE1��EvtTYPE2,��Ϊ���캯������;�ײ���lmmodel��EvtTYPE1����Event;
//���͸���������lmView��,��EvtTYPE2���͵�Event���͸�������lmController;
//Event�еĲ����б���ΪParamlist,���б��һ�Բ�����Ϊ��ʶ��,һ��㶨Ϊ;
//("CommandName","command"),����,"command"Ϊ�Զ����ʶ��,lmView��(��lmController��);
//����"command"�ص����ǵ�������(��lmmodel����Щ����������)����;
//���ڵ���:ʹ�ú���dispatch()����
//���ڴ���:�����ṩ��cloneģʽ��ʹ��C++�Դ�����ָ��shared_ptrģ�壬����;
//�¼��Ĵ���;
#include "EvtParamList.h"
#include <list>
class lmEvtBus;
class longmanEvt
{
public:
	//longmanEvt();
	longmanEvt(const std::string &);
	virtual ~longmanEvt();
	virtual longmanEvt* clone()const { return new longmanEvt(*this); }
private:
	EvtParamList m_paramList;
	std::string m_eventTYPE;

public:
	bool setParam(const std::string&, const lmVariant&);
	longmanEvt &setParam(const std::string&, const lmVariant&,bool ret);
	//ret=true,get success;ret=false,not exist;
	lmVariant getParam(const std::string&);
	bool getParam(const std::string&, lmVariant&);
	void dispatch(lmEvtBus *pcEventBus=nullptr) const;
	bool getParamIter(paramlist::const_iterator&,paramlist::const_iterator&) const;
	std::string getEvtTYPE() const { return m_eventTYPE; }
};

#endif // longmanEvt_h__
