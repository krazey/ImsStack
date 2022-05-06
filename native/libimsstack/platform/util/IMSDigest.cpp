/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090303  toastops@                 Porting (From Aricent SIP stack)
    </table>

    Description
     This file implements the base64 encoding. It is designed to represent arbitrary sequences of
    octets in a form that allows the use of both upper- and lowercase letters but that need not be
    human readable.
    A 65-character subset of US-ASCII is used, enabling 6 bits to be represented per printable
    character. (The extra 65th character, "=" is used to signify a special processing function.)
*/

#include "IMSStrLib.h"
#include "IMSMD5.h"
#include "IMSDigest.h"

#define HASH_SIZE 16

typedef IMS_CHAR HASH[HASH_SIZE];

LOCAL const IMS_CHAR MD5_SESS[] = "md5-sess";
LOCAL const IMS_CHAR QOP_AUTH_INT[] = "auth-int";

LOCAL void imsDigest_ConvertBinToHex(IN CONST HASH HBin, OUT HASHHEX HHex);

/*
This function encodes an arbitrary source data as base64 encoded data.

Remarks

Parameters
<table>
parameter               description
----------              ----------
pSrcData                Pointer to source data to be encoded
nSrcLen                 Length in bytes of source data to be encoded
pszDest                 Caller-allocated buffer to receive the encoded data
nDestLen                Length in characters of pszDest
bAddCRLF                Flag to indicate if the CRLF is inserted or not
</table>

Returns
<table>
return                  description
----------              ----------
Nonzero                 Length in characters of the encoded data
Zero                    Encoding failed
</table>
*/
GLOBAL void IMSDigest_CalculateHA1(IN CONST IMS_CHAR* pszAlgorithm, IN CONST IMS_CHAR* pszUsername,
        IN CONST IMS_CHAR* pszRealm, IN CONST IMS_CHAR* pszPassword, IN CONST IMS_CHAR* pszNonce,
        IN CONST IMS_CHAR* pszCNonce, OUT HASHHEX HA1)
{
    const IMS_UCHAR COLON[2] = {':', '\0'};
    MD5Context stMD5Ctx;
    HASH HA1Local;

    IMSMD5_Initialize(&stMD5Ctx);
    IMSMD5_Update(
            reinterpret_cast<const IMS_UCHAR*>(pszUsername), IMS_StrLen(pszUsername), &stMD5Ctx);
    IMSMD5_Update(COLON, 1, &stMD5Ctx);
    IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(pszRealm), IMS_StrLen(pszRealm), &stMD5Ctx);
    IMSMD5_Update(COLON, 1, &stMD5Ctx);
    IMSMD5_Update(
            reinterpret_cast<const IMS_UCHAR*>(pszPassword), IMS_StrLen(pszPassword), &stMD5Ctx);
    IMSMD5_Finalize(&stMD5Ctx, reinterpret_cast<IMS_UCHAR*>(HA1Local));

    if (IMS_StrICmp(pszAlgorithm, MD5_SESS) == 0)
    {
        IMSMD5_Initialize(&stMD5Ctx);
        IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(HA1Local), HASH_SIZE, &stMD5Ctx);

        if (pszNonce != IMS_NULL)
        {
            IMSMD5_Update(COLON, 1, &stMD5Ctx);
            IMSMD5_Update(
                    reinterpret_cast<const IMS_UCHAR*>(pszNonce), IMS_StrLen(pszNonce), &stMD5Ctx);
        }

        if (pszCNonce != IMS_NULL)
        {
            IMSMD5_Update(COLON, 1, &stMD5Ctx);
            IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(pszCNonce), IMS_StrLen(pszCNonce),
                    &stMD5Ctx);
        }

        IMSMD5_Finalize(&stMD5Ctx, reinterpret_cast<IMS_UCHAR*>(HA1Local));
    }

    imsDigest_ConvertBinToHex(HA1Local, HA1);
}

GLOBAL void IMSDigest_CalculateHEntity(
        IN CONST IMS_CHAR* pszEntityBody, IN IMS_SINT32 nLength, OUT HASHHEX HEntity)
{
    MD5Context stMD5Ctx;
    HASH HEntityLocal;

    IMSMD5_Initialize(&stMD5Ctx);
    IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(pszEntityBody), nLength, &stMD5Ctx);
    IMSMD5_Finalize(&stMD5Ctx, reinterpret_cast<IMS_UCHAR*>(HEntityLocal));

    imsDigest_ConvertBinToHex(HEntityLocal, HEntity);
}

GLOBAL void IMSDigest_CalculateResponse(IN CONST HASHHEX HA1,  // H(A1)
        IN CONST IMS_CHAR* pszNonce,                           // nonce from the server
        IN CONST IMS_CHAR* pszNonceCount,                      // 8 HEX digits
        IN CONST IMS_CHAR* pszCNonce,                          // Client nonce
        IN CONST IMS_CHAR* pszQoP,                             // qop-value : "", "auth", "auth-int"
        IN CONST IMS_CHAR* pszMethod,                          // Method from the Request-Line
        IN CONST IMS_CHAR* pszDigestURI,                       // Requested URL
        IN CONST HASHHEX HEntity,                              // H(entity-body) if qop = "auth-int"
        OUT HASHHEX HResponse)                                 // Request-digest or response-digest
{
    const IMS_UCHAR COLON[2] = {':', '\0'};
    MD5Context stMD5Ctx;
    HASH HA2;
    HASH HResponseLocal;
    HASHHEX HA2Hex;

    // Calculate H(A2)
    IMSMD5_Initialize(&stMD5Ctx);
    IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(pszMethod), IMS_StrLen(pszMethod), &stMD5Ctx);
    IMSMD5_Update(COLON, 1, &stMD5Ctx);
    IMSMD5_Update(
            reinterpret_cast<const IMS_UCHAR*>(pszDigestURI), IMS_StrLen(pszDigestURI), &stMD5Ctx);

    if (pszQoP != IMS_NULL)
    {
        if (IMS_StrICmp(pszQoP, QOP_AUTH_INT) == 0)
        {
            IMSMD5_Update(COLON, 1, &stMD5Ctx);
            IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(HEntity), HASHHEX_SIZE, &stMD5Ctx);
        }
    }

    IMSMD5_Finalize(&stMD5Ctx, reinterpret_cast<IMS_UCHAR*>(HA2));
    imsDigest_ConvertBinToHex(HA2, HA2Hex);

    // Calculate response
    IMSMD5_Initialize(&stMD5Ctx);
    IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(HA1), HASHHEX_SIZE, &stMD5Ctx);
    IMSMD5_Update(COLON, 1, &stMD5Ctx);

    if (pszNonce != IMS_NULL)
    {
        IMSMD5_Update(
                reinterpret_cast<const IMS_UCHAR*>(pszNonce), IMS_StrLen(pszNonce), &stMD5Ctx);
        IMSMD5_Update(COLON, 1, &stMD5Ctx);
    }

    if ((pszQoP != IMS_NULL) && (IMS_StrCmp(pszQoP, "") != 0))
    {
        IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(pszNonceCount), IMS_StrLen(pszNonceCount),
                &stMD5Ctx);
        IMSMD5_Update(COLON, 1, &stMD5Ctx);
        IMSMD5_Update(
                reinterpret_cast<const IMS_UCHAR*>(pszCNonce), IMS_StrLen(pszCNonce), &stMD5Ctx);
        IMSMD5_Update(COLON, 1, &stMD5Ctx);
        IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(pszQoP), IMS_StrLen(pszQoP), &stMD5Ctx);
        IMSMD5_Update(COLON, 1, &stMD5Ctx);
    }

    IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(HA2Hex), HASHHEX_SIZE, &stMD5Ctx);
    IMSMD5_Finalize(&stMD5Ctx, reinterpret_cast<IMS_UCHAR*>(HResponseLocal));

    imsDigest_ConvertBinToHex(HResponseLocal, HResponse);
}

GLOBAL void IMSDigestEx_CalculateHA1(IN CONST AString& strAlgorithm, IN CONST AString& strUsername,
        IN CONST AString& strRealm, IN CONST AString& strPassword, IN CONST AString& strNonce,
        IN CONST AString& strCNonce, OUT HASHHEX HA1)
{
    const IMS_UCHAR COLON[2] = {':', '\0'};
    MD5Context stMD5Ctx;
    HASH HA1Local;

    IMSMD5_Initialize(&stMD5Ctx);
    IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(strUsername.GetStr()), strUsername.GetLength(),
            &stMD5Ctx);
    IMSMD5_Update(COLON, 1, &stMD5Ctx);
    IMSMD5_Update(
            reinterpret_cast<const IMS_UCHAR*>(strRealm.GetStr()), strRealm.GetLength(), &stMD5Ctx);
    IMSMD5_Update(COLON, 1, &stMD5Ctx);
    IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(strPassword.GetStr()), strPassword.GetLength(),
            &stMD5Ctx);
    IMSMD5_Finalize(&stMD5Ctx, reinterpret_cast<IMS_UCHAR*>(HA1Local));

    if (strAlgorithm.EqualsIgnoreCase(MD5_SESS))
    {
        IMSMD5_Initialize(&stMD5Ctx);
        IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(HA1Local), HASH_SIZE, &stMD5Ctx);

        if (strNonce.GetLength() > 0)
        {
            IMSMD5_Update(COLON, 1, &stMD5Ctx);
            IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(strNonce.GetStr()),
                    strNonce.GetLength(), &stMD5Ctx);
        }

        if (strCNonce.GetLength() > 0)
        {
            IMSMD5_Update(COLON, 1, &stMD5Ctx);
            IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(strCNonce.GetStr()),
                    strCNonce.GetLength(), &stMD5Ctx);
        }

        IMSMD5_Finalize(&stMD5Ctx, reinterpret_cast<IMS_UCHAR*>(HA1Local));
    }

    imsDigest_ConvertBinToHex(HA1Local, HA1);
}

GLOBAL void IMSDigestEx_CalculateHEntity(IN CONST AString& strEntityBody, OUT HASHHEX HEntity)
{
    MD5Context stMD5Ctx;
    HASH HEntityLocal;

    IMSMD5_Initialize(&stMD5Ctx);
    IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(strEntityBody.GetStr()),
            strEntityBody.GetLength(), &stMD5Ctx);
    IMSMD5_Finalize(&stMD5Ctx, reinterpret_cast<IMS_UCHAR*>(HEntityLocal));

    imsDigest_ConvertBinToHex(HEntityLocal, HEntity);
}

GLOBAL void IMSDigestEx_CalculateResponse(IN CONST HASHHEX HA1,  // H(A1)
        IN CONST AString& strNonce,                              // nonce from the server
        IN CONST AString& strNonceCount,                         // 8 HEX digits
        IN CONST AString& strCNonce,                             // Client nonce
        IN CONST AString& strQoP,        // qop-value : "", "auth", "auth-int"
        IN CONST AString& strMethod,     // Method from the Request-Line
        IN CONST AString& strDigestURI,  // Requested URL
        IN CONST HASHHEX HEntity,        // H(entity-body) if qop = "auth-int"
        OUT HASHHEX HResponse)           // Request-digest or response-digest
{
    const IMS_UCHAR COLON[2] = {':', '\0'};
    MD5Context stMD5Ctx;
    HASH HA2;
    HASH HResponseLocal;
    HASHHEX HA2Hex;

    // Calculate H(A2)
    IMSMD5_Initialize(&stMD5Ctx);
    IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(strMethod.GetStr()), strMethod.GetLength(),
            &stMD5Ctx);
    IMSMD5_Update(COLON, 1, &stMD5Ctx);
    IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(strDigestURI.GetStr()),
            strDigestURI.GetLength(), &stMD5Ctx);

    if (strQoP.EqualsIgnoreCase(QOP_AUTH_INT))
    {
        IMSMD5_Update(COLON, 1, &stMD5Ctx);
        IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(HEntity), HASHHEX_SIZE, &stMD5Ctx);
    }

    IMSMD5_Finalize(&stMD5Ctx, reinterpret_cast<IMS_UCHAR*>(HA2));
    imsDigest_ConvertBinToHex(HA2, HA2Hex);

    // Calculate response
    IMSMD5_Initialize(&stMD5Ctx);
    IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(HA1), HASHHEX_SIZE, &stMD5Ctx);
    IMSMD5_Update(COLON, 1, &stMD5Ctx);

    if (strNonce.GetLength() > 0)
    {
        IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(strNonce.GetStr()), strNonce.GetLength(),
                &stMD5Ctx);
        IMSMD5_Update(COLON, 1, &stMD5Ctx);
    }

    if (strQoP.GetLength() > 0)
    {
        IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(strNonceCount.GetStr()),
                strNonceCount.GetLength(), &stMD5Ctx);
        IMSMD5_Update(COLON, 1, &stMD5Ctx);
        IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(strCNonce.GetStr()), strCNonce.GetLength(),
                &stMD5Ctx);
        IMSMD5_Update(COLON, 1, &stMD5Ctx);
        IMSMD5_Update(
                reinterpret_cast<const IMS_UCHAR*>(strQoP.GetStr()), strQoP.GetLength(), &stMD5Ctx);
        IMSMD5_Update(COLON, 1, &stMD5Ctx);
    }

    IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(HA2Hex), HASHHEX_SIZE, &stMD5Ctx);
    IMSMD5_Finalize(&stMD5Ctx, reinterpret_cast<IMS_UCHAR*>(HResponseLocal));

    imsDigest_ConvertBinToHex(HResponseLocal, HResponse);
}

LOCAL void imsDigest_ConvertBinToHex(IN CONST HASH HBin, OUT HASHHEX HHex)
{
    IMS_UCHAR ucTemp;

    for (IMS_UINT32 i = 0; i < HASH_SIZE; i++)
    {
        ucTemp = (HBin[i] >> 4) & 0x0F;

        if (ucTemp <= 9)
        {
            HHex[i * 2] = (ucTemp + '0');
        }
        else
        {
            HHex[i * 2] = (ucTemp + 'a' - 10);
        }

        ucTemp = HBin[i] & 0x0F;

        if (ucTemp <= 9)
        {
            HHex[i * 2 + 1] = (ucTemp + '0');
        }
        else
        {
            HHex[i * 2 + 1] = (ucTemp + 'a' - 10);
        }
    }

    HHex[HASHHEX_SIZE] = '\0';
}
