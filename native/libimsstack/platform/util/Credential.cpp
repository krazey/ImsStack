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
#include "Credential.h"
#include "ImsBase64.h"
#include "ImsHmac.h"
#include "ServiceMemory.h"

PUBLIC GLOBAL const IMS_CHAR Credential::STR_MD5[] = "MD5";
PUBLIC GLOBAL const IMS_CHAR Credential::STR_AKAv1_MD5[] = "AKAv1-MD5";

// Digest AKAv2
PUBLIC GLOBAL const IMS_CHAR Credential::STR_AKAv2_MD5[] = "AKAv2-MD5";
PUBLIC GLOBAL const IMS_UINT32 Credential::DIGEST_KEY_CK = 27;
PUBLIC GLOBAL const IMS_UINT32 Credential::DIGEST_KEY_IK = 30;
PUBLIC GLOBAL const IMS_UINT32 Credential::DIGEST_KEY_PASSWORD = 26;
PUBLIC GLOBAL const IMS_UCHAR Credential::STR_DIGEST_KEY_CK[] = "http-digest-akav2-cipherkey";
PUBLIC GLOBAL const IMS_UCHAR Credential::STR_DIGEST_KEY_IK[] = "http-digest-akav2-integritykey";
PUBLIC GLOBAL const IMS_UCHAR Credential::STR_DIGEST_KEY_PASSWORD[] = "http-digest-akav2-password";

PRIVATE GLOBAL Credential Credential::s_objSharedNull = Credential();

PUBLIC
Credential::Credential() :
        m_nTypeOfMd5(TYPE_MD5),
        m_strUsername(AString::ConstNull()),
        m_strPassword(AString::ConstNull()),
        m_strRealm(AString::ConstNull())
{
}

PUBLIC
Credential::Credential(
        IN const AString& strUsername, IN const AString& strPassword, IN const AString& strRealm) :
        m_nTypeOfMd5(TYPE_MD5),
        m_strUsername(strUsername),
        m_strPassword(strPassword),
        m_strRealm(strRealm)
{
}

PUBLIC
Credential::Credential(IN const Credential& other) :
        m_nTypeOfMd5(other.m_nTypeOfMd5),
        m_strUsername(other.m_strUsername),
        m_strPassword(other.m_strPassword),
        m_strRealm(other.m_strRealm),
        m_objAkaParam(other.m_objAkaParam)
{
}

PUBLIC
Credential& Credential::operator=(IN const Credential& other)
{
    if (this != &other)
    {
        m_nTypeOfMd5 = other.m_nTypeOfMd5;
        m_strUsername = other.m_strUsername;
        m_strPassword = other.m_strPassword;
        m_strRealm = other.m_strRealm;
        m_objAkaParam = other.m_objAkaParam;
    }

    return (*this);
}

PUBLIC
void Credential::SetCredentials(
        IN const AString& strUsername, IN const AString& strPassword, IN const AString& strRealm)
{
    m_strUsername = strUsername;
    m_strPassword = strPassword;
    m_strRealm = strRealm;
}

PUBLIC
void Credential::SetAkaResponse(IN IMS_SINT32 nStatus, IN const ByteArray& objRes,
        IN const ByteArray& objAuts /*= ByteArray::ConstNull()*/)
{
    m_objAkaParam.m_nStatus = nStatus;

    // Update the password
    if (nStatus == ImsAkaParam::RESULT_OK)
    {
        m_strPassword = objRes.ToString();
    }
    else
    {
        if (nStatus == ImsAkaParam::RESULT_NOK_SQN_SYNC_FAILED)
        {
            m_objAkaParam.m_strAuts = objAuts.ToString().ToBase64();
        }

        m_strPassword = AString::ConstEmpty();
    }
}

PUBLIC
void Credential::SetAkaResponse(IN IMS_SINT32 nStatus, IN const ByteArray& objRes,
        IN const ByteArray& objIk, IN const ByteArray& objCk,
        IN const ByteArray& objAuts /*= ByteArray::ConstNull()*/)
{
    m_objAkaParam.m_nStatus = nStatus;

    // Update the password
    if (nStatus == ImsAkaParam::RESULT_OK)
    {
        // Pseudo-random function for password generation
        ByteArray objPrf = objRes;
        IMS_UCHAR aucDigest[IMS_HMAC_MD5_SIZE];
        IMS_CHAR acPassword[MAX_AUTH_RESP] = {
                0,
        };

        objPrf.Append(objIk);
        objPrf.Append(objCk);

        // Do HMAC_MD5
        ImsHmac_Md5(STR_DIGEST_KEY_PASSWORD, DIGEST_KEY_PASSWORD, objPrf.GetData(),
                objPrf.GetLength(), aucDigest);

        // Encodes base64 format
        ImsBase64_Encode(aucDigest, IMS_HMAC_MD5_SIZE, acPassword, sizeof(acPassword), IMS_FALSE);

        m_strPassword = acPassword;
    }
    else
    {
        if (nStatus == ImsAkaParam::RESULT_NOK_SQN_SYNC_FAILED)
        {
            m_objAkaParam.m_strAuts = objAuts.ToString().ToBase64();
        }

        m_strPassword = AString::ConstEmpty();
    }
}

PUBLIC GLOBAL ByteArray Credential::DeriveCkForAkav2(IN const ByteArray& objCk)
{
    if (objCk.GetLength() == 0)
    {
        return ByteArray::ConstNull();
    }

    IMS_UCHAR aucDigest[IMS_HMAC_MD5_SIZE];

    // Do HMAC_MD5
    ImsHmac_Md5(STR_DIGEST_KEY_CK, DIGEST_KEY_CK, objCk.GetData(), objCk.GetLength(), aucDigest);

    return ByteArray(reinterpret_cast<const IMS_BYTE*>(aucDigest), IMS_HMAC_MD5_SIZE);
}

PUBLIC GLOBAL ByteArray Credential::DeriveIkForAkav2(IN const ByteArray& objIk)
{
    if (objIk.GetLength() == 0)
    {
        return ByteArray::ConstNull();
    }

    IMS_UCHAR aucDigest[IMS_HMAC_MD5_SIZE];

    // Do HMAC_MD5
    ImsHmac_Md5(STR_DIGEST_KEY_IK, DIGEST_KEY_IK, objIk.GetData(), objIk.GetLength(), aucDigest);

    return ByteArray(reinterpret_cast<const IMS_BYTE*>(aucDigest), IMS_HMAC_MD5_SIZE);
}

PUBLIC GLOBAL IMS_SINT32 Credential::TranslateAlgorithm(IN const AString& strAlgorithm)
{
    if (strAlgorithm.EqualsIgnoreCase(STR_AKAv1_MD5))
    {
        return TYPE_AKAv1_MD5;
    }
    else if (strAlgorithm.EqualsIgnoreCase(STR_AKAv2_MD5))
    {
        return TYPE_AKAv2_MD5;
    }
    else
    {
        return TYPE_MD5;
    }
}
