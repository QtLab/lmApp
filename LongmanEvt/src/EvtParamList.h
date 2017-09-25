#ifndef EvtParamList_h__
#define EvtParamList_h__
#include <string>
#include <map>
//#define list_vaslue_Type_usingUnion


union myVariant
{
	int _int;
	std::string *_str=nullptr;

	double _doub;
};
#ifndef list_vaslue_Type_usingUnion
#include <QVariant>
typedef QVariant lmVariant;
#else
typedef myVariant lmVariant;
#endif
typedef std::map<std::string, lmVariant> paramlist;
class EvtParamList
{
public:
	EvtParamList();
	~EvtParamList();
	//ret=true,insert param,ret=false,overwrite param;
	bool setParam(const std::string&, const lmVariant&);
	//ret=true,get success;ret=false,not exist;
	lmVariant getParam(const std::string&);
	bool getParam(const std::string&, lmVariant&);
	bool getParamIter(paramlist::const_iterator&, paramlist::const_iterator&) const;
private:
	paramlist m_paramlist;
};

#endif // EvtParamList_h__
