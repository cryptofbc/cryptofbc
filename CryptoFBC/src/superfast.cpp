
/* By Paul Hsieh (C) 2004, 2005.  Covered under the Paul Hsieh derivative 
   license. See: 
   http://www.azillionmonkeys.com/qed/weblicense.html for license details.

   http://www.azillionmonkeys.com/qed/hash.html */

#include "superfast.h"

NAMESPACE_BEGIN(CryptoFBC)

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
    || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const unsigned short *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
    +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

bool FBC_SUPERFAST::CalculateSuperFastHash(__in const char* p, 
                                           __in const fdword dwLen, 
                                           __in const fdword dwInitHash, 
                                           __in __out fdword* lpdwHashValue)
{
    bool bRet = false;
    fdword dwRem = 0;
    fdword dwHash = 0;
    fdword dwTemp = 0;
    fdword dwSize = dwLen;

    if ( !p || !dwLen || !lpdwHashValue )
    {
        goto Exit0;
    }

    dwRem = dwLen & 3;
    dwSize >>= 2;

    while (dwSize > 0)
    {
        dwHash += get16bits (p);
        dwTemp = (get16bits (p + 2) << 11) ^ dwHash;
        dwHash = (dwHash << 16) ^ dwTemp;
        p += 4;
        dwHash += dwHash >> 11;
        dwSize--;
    }

    /* Handle end cases */
    switch (dwRem) {
        case 3:	
            dwHash += get16bits (p);
            dwHash ^= dwHash << 16;
            dwHash ^= p[2] << 18;
            dwHash += dwHash >> 11;
            break;
        case 2:	
            dwHash += get16bits (p);
            dwHash ^= dwHash << 11;
            dwHash += dwHash >> 17;
            break;
        case 1: 
            dwHash += *p;
            dwHash ^= dwHash << 10;
            dwHash += dwHash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    dwHash ^= dwHash << 3;
    dwHash += dwHash >> 5;
    dwHash ^= dwHash << 4;
    dwHash += dwHash >> 17;
    dwHash ^= dwHash << 25;
    dwHash += dwHash >> 6;

    *lpdwHashValue = dwHash;

    bRet = true;
Exit0:
    return bRet;
}

NAMESPACE_END

