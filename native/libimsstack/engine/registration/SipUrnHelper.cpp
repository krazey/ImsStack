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
#include "ServiceMemory.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "IMSMD5.h"
#include "IMSSHA1.h"
#include "IMSUUID.h"

#include "SipUrnHelper.h"

__IMS_TRACE_TAG_REG__;

// 3GPP 23.003 13.8 clause / draft-allen-dispatch-imei-urn-as-instanceid
#define __IMS_SIP_IMEI_URN_AS_INSTANCE_ID__

#if defined(__IMS_SIP_IMEI_URN_AS_INSTANCE_ID__)
PRIVATE GLOBAL const IMS_CHAR SipUrnHelper::IMEI[] = "urn:gsma:imei:00000000-000000-0";
PRIVATE GLOBAL const IMS_CHAR SipUrnHelper::IMEI_SV[] = "urn:gsma:imei:00000000-000000-0";
#else
PRIVATE GLOBAL const IMS_CHAR SipUrnHelper::IMEI[] = "urn:gsma:imei:00000000-000000-0;vers=0";
PRIVATE GLOBAL const IMS_CHAR SipUrnHelper::IMEI_SV[] = "urn:gsma:imei:00000000-000000-0;svn=00";
#endif

LOCAL
IMS_BOOL sipUrnHelper_IsUuidFallbackRequiredWhenNoImei(IN IMS_SINT32 /*nSlotId*/)
{
    return IMS_FALSE;
}

PUBLIC GLOBAL AString SipUrnHelper::GetUrn(
        IN IMS_SINT32 nSlotId, IN IMS_SINT32 nType, IN IMS_BOOL bSv /*= IMS_TRUE*/)
{
    AStringBuffer objUrn(64);
    IDeviceInfo* piDeviceInfo = PhoneInfoService::GetPhoneInfoService()->GetDeviceInfo();

    if (piDeviceInfo == IMS_NULL)
    {
        if (bSv)
        {
            return IMEI_SV;
        }
        else
        {
            return IMEI;
        }
    }

    AString strImei;

    piDeviceInfo->GetDeviceId(nSlotId, strImei);

    if (strImei.GetLength() == 0)
    {
        // 3GPP - If no IMEI is available, so take the form of a string representation
        // of a UUID as a URN in RFC 4122.
        // 4 Name selection should be changed lator
        strImei = "00000000000000";

        if (sipUrnHelper_IsUuidFallbackRequiredWhenNoImei(nSlotId) &&
                ((nType == GSMA_IMEI) || (nType == UUID_IMEI_MD5) || (nType == UUID_IMEI_SHA1)))
        {
            nType = UUID_IMEI_NAMED_V5;
        }
    }

    switch (nType)
    {
        case GSMA_IMEI:
        {
            objUrn.Append("urn:gsma:imei:");

            // IMEI = tac (8) "-" snr (6) "-" spare (1)
            if (strImei.GetLength() < 14)
            {
                for (IMS_SINT32 i = strImei.GetLength(); i < 14; ++i)
                {
                    strImei.Append("0");
                }
            }

            objUrn.Append(strImei.GetSubStr(0, 8));
            objUrn.Append('-');
            objUrn.Append(strImei.GetSubStr(8, 6));
            objUrn.Append('-');
            // spare field is always set to zero
            objUrn.Append('0');
#if !defined(__IMS_SIP_IMEI_URN_AS_INSTANCE_ID__)
            objUrn.Append(';');

            if (bSv)
            {
                AString strSv;

                piDeviceInfo->GetDeviceSoftwareVersion(strSv);

                objUrn.Append("svn=");

                if (strSv.GetLength() == 0)
                {
                    objUrn.Append("00");
                }
                else if (strSv.GetLength() == 1)
                {
                    objUrn.Append("0");
                    objUrn.Append(strSv);
                }
                else
                {
                    objUrn.Append(strSv);
                }
            }
            else
            {
                objUrn.Append("vers=0");
            }
#endif
            break;
        }
        case UUID_IMEI_MD5:
        {
            MD5Context stMd5;
            IMS_UCHAR uacHashedImei[16];

            // tac (8) + snr (6)
            strImei = strImei.GetSubStr(0, 14);

            IMSMD5_Initialize(&stMd5);
            IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(strImei.GetStr()), strImei.GetLength(),
                    &stMd5);
            IMSMD5_Finalize(&stMd5, uacHashedImei);

            AString strHex;

            objUrn.Append("urn:uuid:");

            for (IMS_SINT32 i = 0; i < 16; ++i)
            {
                strHex.Sprintf("%02x", uacHashedImei[i]);
                objUrn.Append(strHex);

                if ((i == 3) || (i == 5) || (i == 7) || (i == 9))
                {
                    objUrn.Append('-');
                }
            }
            break;
        }
        case UUID_IMEI_SHA1:
        {
            SHA1Context stSha1;
            IMS_UCHAR uacHashedImei[20];

            // tac (8) + snr (6)
            strImei = strImei.GetSubStr(0, 14);

            IMSSHA1_Initialize(&stSha1);
            IMSSHA1_Update(reinterpret_cast<const IMS_UCHAR*>(strImei.GetStr()),
                    strImei.GetLength(), &stSha1);
            IMSSHA1_Finalize(&stSha1, uacHashedImei);

            AString strHex;

            objUrn.Append("urn:uuid:");

            for (IMS_SINT32 i = 0; i < 16; ++i)
            {
                strHex.Sprintf("%02x", uacHashedImei[i]);
                objUrn.Append(strHex);

                if ((i == 3) || (i == 5) || (i == 7) || (i == 9))
                {
                    objUrn.Append('-');
                }
            }
            break;
        }
        case UUID_IMEI_NAMED_V3:
        {
            // tac (8) + snr (6)
            strImei = strImei.GetSubStr(0, 14);

            objUrn.Append("urn:uuid:");
            objUrn.Append(IMSUUID::GetUUID(IMSUUID::VERSION_3, strImei));
            break;
        }
        case UUID_IMEI_NAMED_V5:
        {
            // tac (8) + snr (6)
            strImei = strImei.GetSubStr(0, 14);

            objUrn.Append("urn:uuid:");
            objUrn.Append(IMSUUID::GetUUID(IMSUUID::VERSION_5, strImei));
            break;
        }
        case UUID_IMEI_V4:
        {
            // tac (8) + snr (6)
            strImei = strImei.GetSubStr(0, 14);

            objUrn.Append("urn:uuid:");
            objUrn.Append(IMSUUID::GetUUID(IMSUUID::VERSION_4, strImei));
            break;
        }
        default:
            break;
    }

    return static_cast<const AStringBuffer&>(objUrn).GetString();
}

PUBLIC GLOBAL AString SipUrnHelper::GetUrn(IN IMS_SINT32 nVersion, IN const AString& strName)
{
    AStringBuffer objUrn(64);

    objUrn.Append("urn:uuid:");
    objUrn.Append(IMSUUID::GetUUID(nVersion, strName));

    return static_cast<const AStringBuffer&>(objUrn).GetString();
}
