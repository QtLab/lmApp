#ifndef lmTypeDef_h__
#define lmTypeDef_h__
//
#ifndef __COMMONDEF__
#define __COMMONDEF__
template <typename T> inline T Clip3(const T minVal, const T maxVal, const T a) { return std::min<T>(std::max<T>(minVal, a), maxVal); }  ///< general min/max clip
typedef       int             Pel;
enum ChromaFormat
{
	CHROMA_400 = 0,
	CHROMA_420 = 1,
	CHROMA_422 = 2,
	CHROMA_444 = 3,
	NUM_CHROMA_FORMAT = 4
};
enum ComponentID
{
	COMPONENT_Y = 0,
	COMPONENT_Cb = 1,
	COMPONENT_Cr = 2,
	MAX_NUM_COMPONENT = 3
};
#endif
#endif // lmTypeDef_h__
