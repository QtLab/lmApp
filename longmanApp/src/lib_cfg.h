#ifndef lib_cfg_h__
#define lib_cfg_h__
#ifdef _DEBUG
#pragma comment(lib,"Evtd.lib")
#pragma comment(lib,"longmanMVCd.lib")
#else
#pragma comment(lib,"Evtr.lib")
#pragma comment(lib,"longmanMVCr.lib")
#endif
#endif // lib_cfg_h__