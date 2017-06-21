
#include "adler32.h"

NAMESPACE_BEGIN(CryptoFBC)

#define defFBC_ADLER32_MODULUS      65521

bool FBC_ADLER32::CalculateAdler32(__in const char* p, 
                                   __in const fdword dwLen, 
                                   __in const fdword dwInitAdler, 
                                   __in __out fdword* lpdwAdlerValue)
{
    bool bRet = false;
    fdword dwA = dwInitAdler;
    fdword dwB = 0;

    if ( !p || !dwLen || !lpdwAdlerValue )
    {
        goto Exit0;
    }

    for ( fdword i = 0; i < dwLen; i++ )
    {
        dwA = ( dwA + p[i] ) % defFBC_ADLER32_MODULUS;
        dwB = ( dwB + dwA ) % defFBC_ADLER32_MODULUS;
    }

    *lpdwAdlerValue = (dwB << 16) | dwA;

    bRet = true;
Exit0:
    return bRet;
}

NAMESPACE_END