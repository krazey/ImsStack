/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ImsDigest.h"
#include "ImsMd5.h"
#include "ImsStrLib.h"

#define HASH_SIZE 16

typedef IMS_CHAR HASH[HASH_SIZE];

static const IMS_CHAR MD5_SESS[] = "md5-sess";
static const IMS_CHAR QOP_AUTH_INT[] = "auth-int";

static void imsDigest_ConvertBinToHex(IN const HASH hBin, OUT HASHHEX hHex);

GLOBAL void ImsDigest_CalculateA1(IN const AString& strAlgorithm, IN const AString& strUsername,
        IN const AString& strRealm, IN const AString& strPassword, IN const AString& strNonce,
        IN const AString& strCNonce, OUT HASHHEX hA1)
{
    const IMS_UCHAR COLON[2] = {':', '\0'};
    ImsMd5Context objMd5Ctx;
    HASH hA1Local;

    ImsMd5_Initialize(&objMd5Ctx);
    ImsMd5_Update(reinterpret_cast<const IMS_UCHAR*>(strUsername.GetStr()), strUsername.GetLength(),
            &objMd5Ctx);
    ImsMd5_Update(COLON, 1, &objMd5Ctx);
    ImsMd5_Update(reinterpret_cast<const IMS_UCHAR*>(strRealm.GetStr()), strRealm.GetLength(),
            &objMd5Ctx);
    ImsMd5_Update(COLON, 1, &objMd5Ctx);
    ImsMd5_Update(reinterpret_cast<const IMS_UCHAR*>(strPassword.GetStr()), strPassword.GetLength(),
            &objMd5Ctx);
    ImsMd5_Finalize(&objMd5Ctx, reinterpret_cast<IMS_UCHAR*>(hA1Local));

    if (strAlgorithm.EqualsIgnoreCase(MD5_SESS))
    {
        ImsMd5_Initialize(&objMd5Ctx);
        ImsMd5_Update(reinterpret_cast<const IMS_UCHAR*>(hA1Local), HASH_SIZE, &objMd5Ctx);

        if (strNonce.GetLength() > 0)
        {
            ImsMd5_Update(COLON, 1, &objMd5Ctx);
            ImsMd5_Update(reinterpret_cast<const IMS_UCHAR*>(strNonce.GetStr()),
                    strNonce.GetLength(), &objMd5Ctx);
        }

        if (strCNonce.GetLength() > 0)
        {
            ImsMd5_Update(COLON, 1, &objMd5Ctx);
            ImsMd5_Update(reinterpret_cast<const IMS_UCHAR*>(strCNonce.GetStr()),
                    strCNonce.GetLength(), &objMd5Ctx);
        }

        ImsMd5_Finalize(&objMd5Ctx, reinterpret_cast<IMS_UCHAR*>(hA1Local));
    }

    imsDigest_ConvertBinToHex(hA1Local, hA1);
}

GLOBAL void ImsDigest_CalculateEntity(IN const AString& strEntityBody, OUT HASHHEX hEntity)
{
    ImsMd5Context objMd5Ctx;
    HASH hEntityLocal;

    ImsMd5_Initialize(&objMd5Ctx);
    ImsMd5_Update(reinterpret_cast<const IMS_UCHAR*>(strEntityBody.GetStr()),
            strEntityBody.GetLength(), &objMd5Ctx);
    ImsMd5_Finalize(&objMd5Ctx, reinterpret_cast<IMS_UCHAR*>(hEntityLocal));

    imsDigest_ConvertBinToHex(hEntityLocal, hEntity);
}

GLOBAL void ImsDigest_CalculateResponse(IN const HASHHEX hA1,  // H(A1)
        IN const AString& strNonce,                            // nonce from the server
        IN const AString& strNonceCount,                       // 8 HEX digits
        IN const AString& strCNonce,                           // Client nonce
        IN const AString& strQop,                              // qop-value : "", "auth", "auth-int"
        IN const AString& strMethod,                           // Method from the Request-Line
        IN const AString& strDigestUri,                        // Requested URI
        IN const HASHHEX hEntity,                              // H(entity-body) if qop = "auth-int"
        OUT HASHHEX hResponse)                                 // Request-digest or response-digest
{
    const IMS_UCHAR COLON[2] = {':', '\0'};
    ImsMd5Context objMd5Ctx;
    HASH hA2;
    HASH hResponseLocal;
    HASHHEX hA2Hex;

    // Calculate H(A2)
    ImsMd5_Initialize(&objMd5Ctx);
    ImsMd5_Update(reinterpret_cast<const IMS_UCHAR*>(strMethod.GetStr()), strMethod.GetLength(),
            &objMd5Ctx);
    ImsMd5_Update(COLON, 1, &objMd5Ctx);
    ImsMd5_Update(reinterpret_cast<const IMS_UCHAR*>(strDigestUri.GetStr()),
            strDigestUri.GetLength(), &objMd5Ctx);

    if (strQop.EqualsIgnoreCase(QOP_AUTH_INT))
    {
        ImsMd5_Update(COLON, 1, &objMd5Ctx);
        ImsMd5_Update(reinterpret_cast<const IMS_UCHAR*>(hEntity), HASHHEX_SIZE, &objMd5Ctx);
    }

    ImsMd5_Finalize(&objMd5Ctx, reinterpret_cast<IMS_UCHAR*>(hA2));
    imsDigest_ConvertBinToHex(hA2, hA2Hex);

    // Calculate response
    ImsMd5_Initialize(&objMd5Ctx);
    ImsMd5_Update(reinterpret_cast<const IMS_UCHAR*>(hA1), HASHHEX_SIZE, &objMd5Ctx);
    ImsMd5_Update(COLON, 1, &objMd5Ctx);

    if (strNonce.GetLength() > 0)
    {
        ImsMd5_Update(reinterpret_cast<const IMS_UCHAR*>(strNonce.GetStr()), strNonce.GetLength(),
                &objMd5Ctx);
        ImsMd5_Update(COLON, 1, &objMd5Ctx);
    }

    if (strQop.GetLength() > 0)
    {
        ImsMd5_Update(reinterpret_cast<const IMS_UCHAR*>(strNonceCount.GetStr()),
                strNonceCount.GetLength(), &objMd5Ctx);
        ImsMd5_Update(COLON, 1, &objMd5Ctx);
        ImsMd5_Update(reinterpret_cast<const IMS_UCHAR*>(strCNonce.GetStr()), strCNonce.GetLength(),
                &objMd5Ctx);
        ImsMd5_Update(COLON, 1, &objMd5Ctx);
        ImsMd5_Update(reinterpret_cast<const IMS_UCHAR*>(strQop.GetStr()), strQop.GetLength(),
                &objMd5Ctx);
        ImsMd5_Update(COLON, 1, &objMd5Ctx);
    }

    ImsMd5_Update(reinterpret_cast<const IMS_UCHAR*>(hA2Hex), HASHHEX_SIZE, &objMd5Ctx);
    ImsMd5_Finalize(&objMd5Ctx, reinterpret_cast<IMS_UCHAR*>(hResponseLocal));

    imsDigest_ConvertBinToHex(hResponseLocal, hResponse);
}

static void imsDigest_ConvertBinToHex(IN const HASH hBin, OUT HASHHEX hHex)
{
    for (IMS_UINT32 i = 0; i < HASH_SIZE; i++)
    {
        IMS_UCHAR ucTemp = (hBin[i] >> 4) & 0x0F;

        if (ucTemp <= 9)
        {
            hHex[i * 2] = (ucTemp + '0');
        }
        else
        {
            hHex[i * 2] = (ucTemp + 'a' - 10);
        }

        ucTemp = hBin[i] & 0x0F;

        if (ucTemp <= 9)
        {
            hHex[i * 2 + 1] = (ucTemp + '0');
        }
        else
        {
            hHex[i * 2 + 1] = (ucTemp + 'a' - 10);
        }
    }

    hHex[HASHHEX_SIZE] = '\0';
}
