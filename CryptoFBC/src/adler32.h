#ifndef CryptoFBC_ADLER32_H
#define CryptoFBC_ADLER32_H

#include "cfbconfig.h"

NAMESPACE_BEGIN(CryptoFBC)

class FBC_ADLER32
{
public:
	static bool CalculateAdler32(
		__in const char* p, 
		__in const fdword dwLen, 
		__in const fdword dwInitAdler,
		__in __out fdword* lpdwAdlerValue
		);
};
NAMESPACE_END

#endif