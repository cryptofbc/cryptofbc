#ifndef CryptoFBC_XTEA_H
#define CryptoFBC_XTEA_H

#include "cfbconfig.h"
#include "cipherbase.h"

NAMESPACE_BEGIN(CryptoFBC)

class FBC_XTEA
{

public:

    void ECB_Encryption(const fdword inBlock[2], fdword outBlock[2], const fdword dwKey[4]);

    void ECB_Decryption(const fdword inBlock[2], fdword outBlock[2], const fdword dwKey[4]);
};

NAMESPACE_END
#endif