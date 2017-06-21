#ifndef CryptoFBC_SUPERFAST_H
#define CryptoFBC_SUPERFAST_H

#include "cfbconfig.h"

NAMESPACE_BEGIN(CryptoFBC)

class FBC_SUPERFAST
{
public:
    static bool CalculateSuperFastHash(
        __in const char* p, 
        __in const fdword dwLen, 
        __in const fdword dwInitHash,
        __in __out fdword* lpdwHashValue
        );
};
NAMESPACE_END

#endif