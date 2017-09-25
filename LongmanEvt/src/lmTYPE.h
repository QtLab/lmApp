#ifndef lmTYPE_h__
#define lmTYPE_h__

#include <memory>
#include <functional>
#include <map>
#include "longmanEvt.h"
typedef std::shared_ptr<longmanEvt> Evt_smartptr;
typedef std::list<longmanEvt*> EvtQue;
#endif // lmTYPE_h__