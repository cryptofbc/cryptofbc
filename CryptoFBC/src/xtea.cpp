#include "xtea.h"

NAMESPACE_BEGIN(CryptoFBC)

#define defFBC_XTEA_DELTA       0x9e3779b9
#define defFBC_XTEA_ROUNDS      32

void FBC_XTEA::ECB_Encryption(const fdword inBlock[2], fdword outBlock[2], const fdword dwKey[4])
{
    fdword y = inBlock[0];
    fdword z = inBlock[1];
    fdword dwLimit = defFBC_XTEA_DELTA;    
    fdword dwSum = 0;
    
    dwLimit *= defFBC_XTEA_ROUNDS;

    while ( dwSum != dwLimit )
    {
        y += (z << 4 ^ z >> 5) + z ^ dwSum + dwKey[ dwSum & 3 ];
        dwSum += defFBC_XTEA_DELTA;
        z += (y << 4 ^ y >> 5) + y ^ dwSum + dwKey[ dwSum >> 11 & 3 ];
    }

    outBlock[0] = y;
    outBlock[1] = z;
}

void CryptoFBC::FBC_XTEA::ECB_Decryption(const fdword inBlock[2], fdword outBlock[2], const fdword dwKey[4])
{
    fdword y = inBlock[0];
    fdword z = inBlock[1];
    fdword dwSum = defFBC_XTEA_DELTA;
    dwSum *= defFBC_XTEA_ROUNDS;

    while ( dwSum )
    {
        z -= (y << 4 ^ y >> 5) + y ^ dwSum + dwKey[ dwSum >> 11 & 3 ];        
        dwSum -= defFBC_XTEA_DELTA;
        y -= (z << 4 ^ z >> 5) + z ^ dwSum + dwKey[ dwSum & 3 ];
    }

    outBlock[0] = y;
    outBlock[1] = z;
}

NAMESPACE_END