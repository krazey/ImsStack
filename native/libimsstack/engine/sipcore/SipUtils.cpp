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
#include "ImsHmac.h"
#include "ImsMd5.h"
#include "ImsStrLib.h"
#include "ServiceMemory.h"
#include "ServicePhoneInfo.h"
#include "ServiceSystemTime.h"
#include "SystemConfig.h"

#include "SipFeatures.h"
#include "SipPrivate.h"
#include "SipUtils.h"

// HEADER_REQ_SESSION-ID
PRIVATE GLOBAL AString* SipUtils::s_pFixedKeyForSessionId = IMS_NULL;

PUBLIC GLOBAL AString SipUtils::GenerateBoundary()
{
    // boundary := 0*69<bchars> bcharsnospace
    AString strBoundary;

    strBoundary.Sprintf("b_%08x-%08x", IMS_SYS_GetRandom0(), IMS_SYS_GetTimeInMicroSeconds());

    return strBoundary;
}

PUBLIC GLOBAL AString SipUtils::GenerateCallId(IN const AString& strHost)
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
PUBLIC GLOBAL AString SipUtils::GenerateSessionId(
        IN IMS_SINT32 nSlotId, IN const AString& strCallId)
{
    // FIXME: add a runtime feature check routine in here (FEATURE)

    AString strSecretKey;
    IMS_UINT32 nRandom = IMS_SYS_GetRandom0();

    strSecretKey.Sprintf("%s%02x", s_pFixedKeyForSessionId[nSlotId].GetStr(), (nRandom % 0x100));

    // HMAC-SHA-1-128 (Call-ID, secret-key):
    // 128-bit result encoded using lowercase alphanumeric hex representation
    IMS_UCHAR uacSessionId[20];

    ImsHmac_Sha1(reinterpret_cast<const IMS_UCHAR*>(strCallId.GetStr()), strCallId.GetLength(),
            reinterpret_cast<const IMS_UCHAR*>(strSecretKey.GetStr()), strSecretKey.GetLength(),
            uacSessionId);

    AString strSessionId;
    strSessionId.Attach(reinterpret_cast<const IMS_CHAR*>(uacSessionId), 16);

    return strSessionId.ToHexString();
}

PUBLIC GLOBAL AString SipUtils::GenerateTag(IN const AString& strMagicCookie)
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

PUBLIC GLOBAL AString SipUtils::GenerateViaBranch(IN const IMS_CHAR* pszToTag,
        IN const IMS_CHAR* pszFromTag, IN const IMS_CHAR* pszCallId,
        IN const IMS_CHAR* pszRequestUri, IN const IMS_CHAR* pszTopmostVia, IN IMS_SINT32 nCSeqNum,
        IN const AString& strExtensionToken /* = AString::ConstNull() */)
{
    const IMS_UCHAR COLON[2] = {':', '\0'};
    ImsMd5Context objMd5Ctx;
    IMS_UCHAR ucTemp;
    IMS_UCHAR aucBranch[16];
    IMS_CHAR acViaBranch[32 + 1] = {
            0,
    };

    // Calculate H(branch)
    ImsMd5_Initialize(&objMd5Ctx);
    ImsMd5_Update(
            reinterpret_cast<const IMS_UCHAR*>(pszFromTag), IMS_StrLen(pszFromTag), &objMd5Ctx);
    ImsMd5_Update(COLON, 1, &objMd5Ctx);
    ImsMd5_Update(reinterpret_cast<const IMS_UCHAR*>(pszCallId), IMS_StrLen(pszCallId), &objMd5Ctx);

    if (pszToTag != IMS_NULL)
    {
        ImsMd5_Update(COLON, 1, &objMd5Ctx);
        ImsMd5_Update(
                reinterpret_cast<const IMS_UCHAR*>(pszToTag), IMS_StrLen(pszToTag), &objMd5Ctx);
    }

    ImsMd5_Update(COLON, 1, &objMd5Ctx);
    ImsMd5_Update(reinterpret_cast<const IMS_UCHAR*>(pszRequestUri), IMS_StrLen(pszRequestUri),
            &objMd5Ctx);

    AString strCSeq;
    strCSeq.SetNumber(nCSeqNum);
    ImsMd5_Update(COLON, 1, &objMd5Ctx);
    ImsMd5_Update(
            reinterpret_cast<const IMS_UCHAR*>(strCSeq.GetStr()), strCSeq.GetLength(), &objMd5Ctx);

    if (pszTopmostVia != IMS_NULL)
    {
        ImsMd5_Update(COLON, 1, &objMd5Ctx);
        ImsMd5_Update(reinterpret_cast<const IMS_UCHAR*>(pszTopmostVia), IMS_StrLen(pszTopmostVia),
                &objMd5Ctx);
    }

    ImsMd5_Finalize(&objMd5Ctx, aucBranch);

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

PUBLIC GLOBAL AString SipUtils::GenerateViaBranch(
        IN const AString& strExtensionToken /*= AString::ConstNull()*/)
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

PUBLIC GLOBAL void SipUtils::Init(IN IMS_SINT32 nSlotId)
{
    (void)nSlotId;

    // HEADER_REQ_SESSION-ID
    if (SipFeatures::IsHeaderSessionIdRequired(nSlotId))
    {
        if (s_pFixedKeyForSessionId == IMS_NULL)
        {
            s_pFixedKeyForSessionId = new AString[SystemConfig::GetMaxSimSlot()];
        }

        if ((nSlotId >= IMS_SLOT_0) && (nSlotId < SystemConfig::GetMaxSimSlot()))
        {
            // Generates pseudo-random 128-bit system secret key
            IDeviceInfo* piDeviceInfo = PhoneInfoService::GetPhoneInfoService()->GetDeviceInfo();
            AString strImei;

            piDeviceInfo->GetDeviceId(nSlotId, strImei);

            // If the IMEI is less than 14,
            // we consider it to the invalid value and generate the random secret key
            if (strImei.GetLength() < 14)
            {
                IMS_UINT32 nR0 = IMS_SYS_GetRandom0();

                s_pFixedKeyForSessionId[nSlotId].Sprintf(
                        "%08x%05x%x", nR0, IMS_SYS_GetTimeInMicroSeconds(), (nR0 % 0x10));
            }
            else
            {
                s_pFixedKeyForSessionId[nSlotId] = strImei.GetSubStr(0, 14);
            }
        }
    }
}
