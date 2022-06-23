/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100727  hwangoo.park@             Created (RFC 2104)
    </table>

    Description
     This file implements HMAC, a mechanism for message authentication using cryptographic hash
    functions. HMAC can be used with any iterative cryptographic hash function, e.g., MD5, SHA-1,
    in combination with a secret shared key.
    The cryptographic strength of HMAC depends on the properties of the underlying hash function.
    HMAC can be used in combination with any iterated cryptographic hash function.
    MD5 and SHA-1 are examples of such hash functions. HMAC also uses a secret key for calculation
    and verification of the message authentication values.
*/

#include "ServiceMemory.h"
#include "ImsMd5.h"
#include "ImsSha1.h"
#include "ImsHmac.h"

// The byte-length of data blocks
#define HMAC_B      64
// The byte-length of hash outputs for MD5
#define HMAC_L_MD5  16
// The byte-length of hash outputs for SHA-1
#define HMAC_L_SHA1 20

/*
This function generates HMAC-MD5.

Remarks

Parameters
<table>
parameter               description
----------              ----------
pucText                 Pointer to the input data stream
nTextLen                Length of the input data stream
pucKey                  Pointer to the authentication key
nKeyLen                 Length of the authentication key
aucDigest               Digest value to be filled in
</table>

Returns
<table>
return                  description
----------              ----------
</table>
*/
GLOBAL void IMSHMAC_MD5(IN CONST IMS_UCHAR* pucText, IN IMS_SINT32 nTextLen,
        IN CONST IMS_UCHAR* pucKey, IN IMS_SINT32 nKeyLen, OUT IMS_UCHAR aucDigest[16])
{
    MD5Context stContext;
    // Inner padding - key XORd with ipad
    IMS_UCHAR aucKey_IPad[HMAC_B + 1];
    // Outer padding - key XORd with opad
    IMS_UCHAR aucKey_OPad[HMAC_B + 1];
    IMS_UCHAR aucKey_Temp[HMAC_L_MD5];

    // If key is longer than 64 bytes, reset it to key = MD5(key)
    if (nKeyLen > HMAC_B)
    {
        MD5Context stContext;

        IMSMD5_Initialize(&stContext);
        IMSMD5_Update(pucKey, nKeyLen, &stContext);
        IMSMD5_Finalize(&stContext, aucKey_Temp);

        pucKey = aucKey_Temp;
        nKeyLen = HMAC_L_MD5;
    }

    // The HMAC_MD5 transform looks like:
    //    MD5 (K XOR opad, MD5 (K XOR ipad, text))
    //
    //    where K is an n byte key
    //    ipad is the byte 0x36 repeated 64 times
    //    opad is the byte 0x5c repeated 64 times
    //    and text is the data being protected

    // Start out by storing key in pads
    IMS_MEM_Memset(aucKey_IPad, 0, sizeof(aucKey_IPad));
    IMS_MEM_Memset(aucKey_OPad, 0, sizeof(aucKey_OPad));

    IMS_MEM_Memcpy(aucKey_IPad, pucKey, nKeyLen);
    IMS_MEM_Memcpy(aucKey_OPad, pucKey, nKeyLen);

    // XOR key with ipad and opad values
    for (IMS_SINT32 i = 0; i < HMAC_B; ++i)
    {
        aucKey_IPad[i] ^= 0x36;
        aucKey_OPad[i] ^= 0x5c;
    }

    //// Performs inner MD5

    // Init context for 1st pass
    IMSMD5_Initialize(&stContext);
    // Starts with inner pad
    IMSMD5_Update(aucKey_IPad, HMAC_B, &stContext);
    // then text of datagram
    IMSMD5_Update(pucText, nTextLen, &stContext);
    // Finish up 1st pass
    IMSMD5_Finalize(&stContext, aucDigest);

    //// Performs outer MD5

    // Init context for 2nd pass
    IMSMD5_Initialize(&stContext);
    // Starts with outer pad
    IMSMD5_Update(aucKey_OPad, HMAC_B, &stContext);
    // then results of 1st hash
    IMSMD5_Update(aucDigest, HMAC_L_MD5, &stContext);
    // Finish up 2nd pass
    IMSMD5_Finalize(&stContext, aucDigest);
}

/*
This function generates HMAC-SHA-1.

Remarks

Parameters
<table>
parameter               description
----------              ----------
pucText                 Pointer to the input data stream
nTextLen                Length of the input data stream
pucKey                  Pointer to the authentication key
nKeyLen                 Length of the authentication key
aucHash                 Hash value to be filled in
</table>

Returns
<table>
return                  description
----------              ----------
</table>
*/
GLOBAL void IMSHMAC_SHA1(IN CONST IMS_UCHAR* pucText, IN IMS_SINT32 nTextLen,
        IN CONST IMS_UCHAR* pucKey, IN IMS_SINT32 nKeyLen, OUT IMS_UCHAR aucHash[20])
{
    SHA1Context stContext;
    // Inner padding - key XORd with ipad
    IMS_UCHAR aucKey_IPad[HMAC_B + 1];
    // Outer padding - key XORd with opad
    IMS_UCHAR aucKey_OPad[HMAC_B + 1];
    IMS_UCHAR aucKey_Temp[HMAC_L_SHA1];

    // If key is longer than 64 bytes, reset it to key = SHA-1(key)
    if (nKeyLen > HMAC_B)
    {
        SHA1Context stContext;

        IMSSHA1_Initialize(&stContext);
        IMSSHA1_Update(pucKey, nKeyLen, &stContext);
        IMSSHA1_Finalize(&stContext, aucKey_Temp);

        pucKey = aucKey_Temp;
        nKeyLen = HMAC_L_SHA1;
    }

    // The HMAC-SHA-1 transform looks like:
    //    SHA-1 (K XOR opad, SHA-1 (K XOR ipad, text))
    //
    //    where K is an n byte key
    //    ipad is the byte 0x36 repeated 64 times
    //    opad is the byte 0x5c repeated 64 times
    //    and text is the data being protected

    // Start out by storing key in pads
    IMS_MEM_Memset(aucKey_IPad, 0, sizeof(aucKey_IPad));
    IMS_MEM_Memset(aucKey_OPad, 0, sizeof(aucKey_OPad));

    IMS_MEM_Memcpy(aucKey_IPad, pucKey, nKeyLen);
    IMS_MEM_Memcpy(aucKey_OPad, pucKey, nKeyLen);

    // XOR key with ipad and opad values
    for (IMS_SINT32 i = 0; i < HMAC_B; ++i)
    {
        aucKey_IPad[i] ^= 0x36;
        aucKey_OPad[i] ^= 0x5c;
    }

    //// Performs inner SHA-1

    // Init context for 1st pass
    IMSSHA1_Initialize(&stContext);
    // Starts with inner pad
    IMSSHA1_Update(aucKey_IPad, HMAC_B, &stContext);
    // then text of datagram
    IMSSHA1_Update(pucText, nTextLen, &stContext);
    // Finish up 1st pass
    IMSSHA1_Finalize(&stContext, aucHash);

    //// Performs outer SHA-1

    // Init context for 2nd pass
    IMSSHA1_Initialize(&stContext);
    // Starts with outer pad
    IMSSHA1_Update(aucKey_OPad, HMAC_B, &stContext);
    // then results of 1st hash
    IMSSHA1_Update(aucHash, HMAC_L_SHA1, &stContext);
    // Finish up 2nd pass
    IMSSHA1_Finalize(&stContext, aucHash);
}
