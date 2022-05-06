/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090302  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ByteArray.h"
#include "IMSBase64.h"
#include "IMSHMAC.h"
#include "Credential.h"

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
        nTypeOfMD5(TYPE_MD5),
        strUsername(AString::ConstNull()),
        strPassword(AString::ConstNull()),
        strRealm(AString::ConstNull())
{
}

PUBLIC
Credential::Credential(IN CONST AString& strUsername_, IN CONST AString& strPassword_,
        IN CONST AString& strRealm_) :
        nTypeOfMD5(TYPE_MD5),
        strUsername(strUsername_),
        strPassword(strPassword_),
        strRealm(strRealm_)
{
}

PUBLIC
Credential::Credential(IN CONST Credential& objRHS) :
        nTypeOfMD5(objRHS.nTypeOfMD5),
        strUsername(objRHS.strUsername),
        strPassword(objRHS.strPassword),
        strRealm(objRHS.strRealm),
        stAKAParam(objRHS.stAKAParam)
{
}

PUBLIC
Credential::~Credential() {}

/*

Remarks

*/
PUBLIC
Credential& Credential::operator=(IN CONST Credential& objRHS)
{
    if (this != &objRHS)
    {
        nTypeOfMD5 = objRHS.nTypeOfMD5;
        strUsername = objRHS.strUsername;
        strPassword = objRHS.strPassword;
        strRealm = objRHS.strRealm;

        stAKAParam = objRHS.stAKAParam;
    }

    return (*this);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Credential::IsSameRealm(IN CONST AString& strRealm) const
{
    if (this->strRealm.Equals(strRealm))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PUBLIC
const AString& Credential::GetPassword() const
{
    return strPassword;
}

/*

Remarks

*/
PUBLIC
const AString& Credential::GetRealm() const
{
    return strRealm;
}

/*

Remarks

*/
PUBLIC
const AString& Credential::GetUsername() const
{
    return strUsername;
}

/*

Remarks

*/
PUBLIC
void Credential::SetCredentials(
        IN CONST AString& strUsername, IN CONST AString& strPassword, IN CONST AString& strRealm)
{
    this->strUsername = strUsername;
    this->strPassword = strPassword;
    this->strRealm = strRealm;
}

/*

Remarks

*/
PUBLIC
void Credential::SetPassword(IN CONST AString& strPassword)
{
    this->strPassword = strPassword;
}

/*

Remarks

*/
PUBLIC
void Credential::SetRealm(IN CONST AString& strRealm)
{
    this->strRealm = strRealm;
}

/*

Remarks

*/
PUBLIC
void Credential::SetUsername(IN CONST AString& strUsername)
{
    this->strUsername = strUsername;
}

/*

Remarks

*/
PUBLIC
const IMS_AKA& Credential::GetAKAResponse() const
{
    return stAKAParam;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 Credential::GetType() const
{
    return nTypeOfMD5;
}

/*

Remarks

*/
PUBLIC
void Credential::SetAKAResponse(IN IMS_SINT32 nStatus, IN CONST ByteArray& objRES,
        IN CONST ByteArray& objAUTS /* = ByteArray::ConstNull() */)
{
    stAKAParam.nStatus = nStatus;

    // Update the password
    if (nStatus == IMS_AKA::RESULT_OK)
    {
        strPassword = objRES.ToString();
    }
    else
    {
        if (nStatus == IMS_AKA::RESULT_NOK_SQN_SYNC_FAILED)
        {
            stAKAParam.strAUTS = objAUTS.ToString().ToBase64();
        }

        strPassword = AString::ConstEmpty();
    }
}

/*

Remarks

*/
PUBLIC
void Credential::SetAKAResponse(IN IMS_SINT32 nStatus, IN CONST ByteArray& objRES,
        IN CONST ByteArray& objIK, IN CONST ByteArray& objCK,
        IN CONST ByteArray& objAUTS /* = ByteArray::ConstNull() */)
{
    stAKAParam.nStatus = nStatus;

    // Update the password
    if (nStatus == IMS_AKA::RESULT_OK)
    {
        ByteArray objPRF = objRES;
        IMS_UCHAR aucDigest[16];
        IMS_CHAR acPassword[128] = {
                0,
        };

        objPRF.Append(objIK);
        objPRF.Append(objCK);

        // Do HMAC_MD5
        IMSHMAC_MD5(STR_DIGEST_KEY_PASSWORD, DIGEST_KEY_PASSWORD, objPRF.GetData(),
                objPRF.GetLength(), aucDigest);

        // Encodes base64 format
        IMSBase64_Encode(aucDigest, 16, acPassword, sizeof(acPassword), IMS_FALSE);

        strPassword = acPassword;
    }
    else
    {
        if (nStatus == IMS_AKA::RESULT_NOK_SQN_SYNC_FAILED)
        {
            stAKAParam.strAUTS = objAUTS.ToString().ToBase64();
        }

        strPassword = AString::ConstEmpty();
    }
}

/*

Remarks

*/
PUBLIC
void Credential::SetType(IN IMS_SINT32 nType)
{
    nTypeOfMD5 = nType;
}

/*

Remarks

*/
PUBLIC GLOBAL ByteArray Credential::DeriveCKForAKAv2(IN CONST ByteArray& objCK)
{
    if (objCK.GetLength() == 0)
    {
        return ByteArray::ConstNull();
    }

    IMS_UCHAR aucDigest[16];

    // Do HMAC_MD5
    IMSHMAC_MD5(STR_DIGEST_KEY_CK, DIGEST_KEY_CK, objCK.GetData(), objCK.GetLength(), aucDigest);

    return ByteArray(reinterpret_cast<const IMS_BYTE*>(aucDigest), 16);
}

/*

Remarks

*/
PUBLIC GLOBAL ByteArray Credential::DeriveIKForAKAv2(IN CONST ByteArray& objIK)
{
    if (objIK.GetLength() == 0)
    {
        return ByteArray::ConstNull();
    }

    IMS_UCHAR aucDigest[16];

    // Do HMAC_MD5
    IMSHMAC_MD5(STR_DIGEST_KEY_IK, DIGEST_KEY_IK, objIK.GetData(), objIK.GetLength(), aucDigest);

    return ByteArray(reinterpret_cast<const IMS_BYTE*>(aucDigest), 16);
}

/*

Remarks

*/
PUBLIC GLOBAL IMS_SINT32 Credential::TranslateAlgorithm(IN CONST AString& strAlgorithm)
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
