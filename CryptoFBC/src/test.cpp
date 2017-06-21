#include <iostream>
#include <windows.h>

#include "../src/blowfish.h"
#include "../src/baseroutine.h"
#include "../src/md5.h"
#include "../src/des.h"
#include "../src/tea.h"
#include "../src/gf256.h"
#include "../src/twofish.h"
#include "../src/idea.h"
#include "../src/rc4.h"
#include "../src/ModifiedBlowfish.h"
#include "../src/aes.h"
#include "../src/base64.h"
#include "../src/sha1.h"
#include "../src/crc32.h"
#include "../src/base32.h"
#include "../src/rc5.h"
#include "../src/sha256.h"
#include "../src/sha3.h"

using namespace std;
using namespace CryptoFBC;

#if _MSC_VER == 1400

#ifdef _DEBUG
#pragma comment(lib, "Debug/cryptofbcvc8D.lib")
#else
#pragma comment(lib, "Release/cryptofbcvc8.lib")
#endif

#elif _MSC_VER == 1500

#ifdef _DEBUG
#pragma comment(lib, "Debug/cryptofbcvc9D.lib")
#else
#pragma comment(lib, "Release/cryptofbcvc9.lib")
#endif

#endif

void Test_SHA256(fbyte* message, int nSize)
{
	FBC_SHA256 sha256Inst;
	//char* szMessage = "abc";
	//char* szMessage2 = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
	//FBC_Byte szHash[32] = { 0 };
    fbyte pHash[32];

	sha256Inst.Update( message, nSize );
	sha256Inst.Final(pHash);

    /*char szValue[65] = { 0 };

    for ( int i = 0; i < 32; i++ )
    {
        sprintf_s( szValue + i * 2, 3, "%02x", pHash[i] );
    }

    cout << szValue << endl;*/
}

void Test_SHA3_256()
{
    char* pMessage = "The quick brown fox jumps over the lazy dog.";
    fbyte pHashVal[32];
    char* pMessage2 = "724627916C50338643E6996F07877EAFD96BDF01DA7E991D4155B9BE1295EA7D21C9391F4C4A";
    char* pMessage3 = "41C75F77E5D27389253393725F1427F57914B273AB862B9E31DABCE506E558720520D33352D119F699E784F9E548FF91BC35CA147042128709820D69A8287EA3257857615EB0321270E94B84F446942765CE882B191FAEE7E1C87E0F0BD4E0CD8A927703524B559B769CA4ECE1F6DBF313FDCF67C572EC4185C1A88E86EC11B6454B371980020F19633B6B95BD280E4FBCB0161E1A82470320CEC6ECFA25AC73D09F1536F286D3F9DACAFB2CD1D0CE72D64D197F5C7520B3CCB2FD74EB72664BA93853EF41EABF52F015DD591500D018DD162815CC993595B195";
    char* pMessage4 = "724627916C50338643E6996F07877EAFD96BDF01DA7E991D4155B9BE1295EA7D21C9391F4C4A41C75F77E5D27389253393725F1427F57914B273AB862B9E31DABCE506E558720520D33352D119F699E784F9E548FF91BC35CA147042128709820D69A8287EA3257857615EB0321270E94B84F446942765CE882B191FAEE7E1C87E0F0BD4E0CD8A927703524B559B769CA4ECE1F6DBF313FDCF67C572EC4185C1A88E86EC11B6454B371980020F19633B6B95BD280E4FBCB0161E1A82470320CEC6ECFA25AC73D09F1536F286D3F9DACAFB2CD1D0CE72D64D197F5C7520B3CCB2FD74EB72664BA93853EF41EABF52F015DD591500D018DD162815CC993595B195";
    FBC_SHA3 sha3Inst(256);

    //sha3Inst.Update( (fbyte*)pMessage2, strlen(pMessage2) * 8);
    //sha3Inst.Update( (fbyte*)pMessage3, strlen(pMessage3) * 8);
    //sha3Inst.Update( (fbyte*)pMessage4, strlen(pMessage4) * 8);
    sha3Inst.Update( (fbyte*)pMessage4, 2048);
    sha3Inst.Final(pHashVal);

    char szHash[65] = { 0 };

    for ( int i = 0; i < 32; i++ )
    {
        sprintf_s(szHash + i * 2, 3, "%02x", pHashVal[i]);
    }

    //cout << szHash << endl;
}

void Test_SHA3_File(fbyte* p, int nSize)
{
    fbyte pHashVal[32];
    FBC_SHA3 sha3Inst(256);

    sha3Inst.Update(p, nSize);
    sha3Inst.Final( pHashVal );

    char szHash[65] = { 0 };

    for ( int i = 0; i < 32; i++ )
    {
        sprintf_s(szHash + i * 2, 3, "%02x", pHashVal[i]);
    }

    cout << szHash << endl;
}

void main()
{
	
}