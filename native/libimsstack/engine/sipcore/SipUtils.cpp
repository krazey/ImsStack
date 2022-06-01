/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceSystemTime.h"
// HEADER_REQ_SESSION-ID
#include "ServicePhoneInfo.h"
#include "SystemConfig.h"
#include "IMSHMAC.h"
#include "IMSMD5.h"
#include "IMSStrLib.h"
#include "SipPrivate.h"
#include "SipFeatures.h"
#include "SipUtils.h"

// HEADER_REQ_SESSION-ID
PRIVATE GLOBAL AString* SIPUtil::pFixedKeyForSessionId = IMS_NULL;

PUBLIC GLOBAL AString SIPUtil::GenerateBoundary()
{
    // boundary := 0*69<bchars> bcharsnospace
    AString strBoundary;

    strBoundary.Sprintf("b_%08x-%08x", IMS_SYS_GetRandom0(), IMS_SYS_GetTimeInMicroSeconds());

    return strBoundary;
}

PUBLIC GLOBAL AString SIPUtil::GenerateCallId(IN const AString& strHost)
{
    AString strCallId;
    ImsTime stTime = IMS_SYS_GetLocalTime();

    if (strHost.GetLength() == 0)
    {
        strCallId.Sprintf("%02d%02d%05x-%08x", stTime.nMinute, stTime.nSecond,
                IMS_SYS_GetTimeInMicroSeconds(), IMS_SYS_GetRandom0());
    }
    else
    {
        strCallId.Sprintf("%02d%02d%05x-%08x@%s", stTime.nMinute, stTime.nSecond,
                IMS_SYS_GetTimeInMicroSeconds(), IMS_SYS_GetRandom0(), strHost.GetStr());
    }

    return strCallId;
}

// HEADER_REQ_SESSION-ID
PUBLIC GLOBAL AString SIPUtil::GenerateSessionId(IN IMS_SINT32 nSlotId, IN const AString& strCallId)
{
    // FIXME: add a runtime feature check routine in here (FEATURE)

    AString strSecretKey;
    IMS_UINT32 nRandom = IMS_SYS_GetRandom0();

    strSecretKey.Sprintf("%s%02x", pFixedKeyForSessionId[nSlotId].GetStr(), (nRandom % 0x100));

    // HMAC-SHA-1-128 (Call-ID, secret-key):
    // 128-bit result encoded using lowercase alphanumeric hex representation
    IMS_UCHAR uacSessionId[20];

    IMSHMAC_SHA1(reinterpret_cast<const IMS_UCHAR*>(strCallId.GetStr()), strCallId.GetLength(),
            reinterpret_cast<const IMS_UCHAR*>(strSecretKey.GetStr()), strSecretKey.GetLength(),
            uacSessionId);

    AString strSessionId;
    strSessionId.Attach(reinterpret_cast<const IMS_CHAR*>(uacSessionId), 16);

    return strSessionId.ToHexString();
}

PUBLIC GLOBAL AString SIPUtil::GenerateTag(IN const AString& strMagicCookie)
{
    AString strTagValue;
    ImsTime stTime = IMS_SYS_GetLocalTime();

    if (strMagicCookie.GetLength() == 0)
    {
        strTagValue.Sprintf("%02d%02d%05x-%08x", stTime.nMinute, stTime.nSecond,
                IMS_SYS_GetTimeInMicroSeconds(), IMS_SYS_GetRandom0());
    }
    else
    {
        strTagValue.Sprintf("%s%02d%02d%05x-%08x", strMagicCookie.GetStr(), stTime.nMinute,
                stTime.nSecond, IMS_SYS_GetTimeInMicroSeconds(), IMS_SYS_GetRandom0());
    }

    return strTagValue;
}

PUBLIC GLOBAL AString SIPUtil::GenerateViaBranch(IN const IMS_CHAR* pszToTag,
        IN const IMS_CHAR* pszFromTag, IN const IMS_CHAR* pszCallID,
        IN const IMS_CHAR* pszRequestURI, IN const IMS_CHAR* pszTopmostVia, IN IMS_SINT32 nCSeqNum,
        IN const AString& strExtensionToken /* = AString::ConstNull() */)
{
    const IMS_UCHAR COLON[2] = {':', '\0'};
    MD5Context stMD5Ctx;
    IMS_UCHAR ucTemp;
    IMS_UCHAR aucBranch[16];
    IMS_CHAR acViaBranch[32 + 1] = {
            0,
    };

    // Calculate H(branch)
    IMSMD5_Initialize(&stMD5Ctx);
    IMSMD5_Update(
            reinterpret_cast<const IMS_UCHAR*>(pszFromTag), IMS_StrLen(pszFromTag), &stMD5Ctx);
    IMSMD5_Update(COLON, 1, &stMD5Ctx);
    IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(pszCallID), IMS_StrLen(pszCallID), &stMD5Ctx);

    if (pszToTag != IMS_NULL)
    {
        IMSMD5_Update(COLON, 1, &stMD5Ctx);
        IMSMD5_Update(
                reinterpret_cast<const IMS_UCHAR*>(pszToTag), IMS_StrLen(pszToTag), &stMD5Ctx);
    }

    IMSMD5_Update(COLON, 1, &stMD5Ctx);
    IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(pszRequestURI), IMS_StrLen(pszRequestURI),
            &stMD5Ctx);

    AString strCSeq;
    strCSeq.SetNumber(nCSeqNum);
    IMSMD5_Update(COLON, 1, &stMD5Ctx);
    IMSMD5_Update(
            reinterpret_cast<const IMS_UCHAR*>(strCSeq.GetStr()), strCSeq.GetLength(), &stMD5Ctx);

    if (pszTopmostVia != IMS_NULL)
    {
        IMSMD5_Update(COLON, 1, &stMD5Ctx);
        IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(pszTopmostVia), IMS_StrLen(pszTopmostVia),
                &stMD5Ctx);
    }

    IMSMD5_Finalize(&stMD5Ctx, aucBranch);

    for (IMS_UINT32 i = 0; i < 16; i++)
    {
        ucTemp = (aucBranch[i] >> 4) & 0x0F;

        if (ucTemp <= 9)
        {
            acViaBranch[i * 2] = (ucTemp + '0');
        }
        else
        {
            acViaBranch[i * 2] = (ucTemp + 'a' - 10);
        }

        ucTemp = aucBranch[i] & 0x0F;

        if (ucTemp <= 9)
        {
            acViaBranch[i * 2 + 1] = (ucTemp + '0');
        }
        else
        {
            acViaBranch[i * 2 + 1] = (ucTemp + 'a' - 10);
        }
    }

    acViaBranch[32] = '\0';

    AString strViaBranch(Sip::STR_BRANCH_MAGIC_COOKIE);

    return strViaBranch.Append(acViaBranch).Append(strExtensionToken);
}

PUBLIC GLOBAL AString SIPUtil::GenerateViaBranch(
        IN const AString& strExtensionToken /* = AString::ConstNull() */)
{
    AString strViaBranch;
    ImsTime stTime = IMS_SYS_GetLocalTime();

    if (strExtensionToken.GetLength() > 0)
    {
        strViaBranch.Sprintf("%s%02d%02d%05x-%08x_%s", Sip::STR_BRANCH_MAGIC_COOKIE, stTime.nMinute,
                stTime.nSecond, IMS_SYS_GetTimeInMicroSeconds(), IMS_SYS_GetRandom0(),
                strExtensionToken.GetStr());
    }
    else
    {
        strViaBranch.Sprintf("%s%02d%02d%05x-%08x", Sip::STR_BRANCH_MAGIC_COOKIE, stTime.nMinute,
                stTime.nSecond, IMS_SYS_GetTimeInMicroSeconds(), IMS_SYS_GetRandom0());
    }

    return strViaBranch;
}

PUBLIC GLOBAL void SIPUtil::Init(IN IMS_SINT32 nSlotId)
{
    (void)nSlotId;

    // HEADER_REQ_SESSION-ID
    if (SipFeatures::IsHeaderSessionIdRequired(nSlotId))
    {
        if (pFixedKeyForSessionId == IMS_NULL)
        {
            pFixedKeyForSessionId = new AString[SystemConfig::GetMaxSimSlot()];
        }

        if ((nSlotId >= IMS_SLOT_0) && (nSlotId < SystemConfig::GetMaxSimSlot()))
        {
            // Generates pseudo-random 128-bit system secret key
            IDeviceInfo* piDeviceInfo = PhoneInfoService::GetPhoneInfoService()->GetDeviceInfo();
            AString strIMEI;

            piDeviceInfo->GetDeviceId(nSlotId, strIMEI);

            // If the IMEI is less than 14,
            // we consider it to the invalid value and generate the random secret key
            if (strIMEI.GetLength() < 14)
            {
                IMS_UINT32 nR0 = IMS_SYS_GetRandom0();

                pFixedKeyForSessionId[nSlotId].Sprintf(
                        "%08x%05x%x", nR0, IMS_SYS_GetTimeInMicroSeconds(), (nR0 % 0x10));
            }
            else
            {
                pFixedKeyForSessionId[nSlotId] = strIMEI.GetSubStr(0, 14);
            }
        }
    }
}
