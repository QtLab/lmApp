#include "EvtParamList.h"
EvtParamList::EvtParamList()
{

}


EvtParamList::~EvtParamList()
{
}

bool EvtParamList::setParam(const std::string& pParamName, const lmVariant& pParamValue)
{
	paramlist::iterator iter;
	iter = m_paramlist.find(pParamName);
	if (iter == m_paramlist.end())
	{
		m_paramlist.insert(paramlist::value_type(pParamName, pParamValue));
		return true;
	}
	else
	{
		iter->second = pParamValue;
		return false;
	}
}


lmVariant EvtParamList::getParam(const std::string& pParamName)
{
#ifndef list_vaslue_Type_usingUnion
	lmVariant ret=0;
#else
	lmVariant ret._int=0;
#endif
	paramlist::iterator iter;
	iter = m_paramlist.find(pParamName);
	if (iter == m_paramlist.end())
		return ret;//param no exist
	else
		return iter->second;
}
bool EvtParamList::getParam(const std::string& pParamName, lmVariant& pParamValue)
{
#ifndef list_vaslue_Type_usingUnion
	lmVariant ret=0;
#else
	lmVariant ret._int = 0;
#endif
	paramlist::iterator iter;
	iter = m_paramlist.find(pParamName);
	if (iter == m_paramlist.end())
		return false;
	else
		{
			pParamValue = iter->second;
			return true;
		}
}

bool EvtParamList::getParamIter(paramlist::const_iterator &begIt,paramlist::const_iterator &endIt) const
{
	if (m_paramlist.empty())
		return false;
	else
	{
		begIt = m_paramlist.begin();
		endIt = m_paramlist.end();
		return true;
	}
}
