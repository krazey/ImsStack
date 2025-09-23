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
#include "ImsMd5.h"
#include "ImsSha1.h"
#include "ImsUuid.h"

#include "SipUrnHelper.h"

__IMS_TRACE_TAG_REG__;

// 3GPP 23.003 13.8 clause (initial: draft-allen-dispatch-imei-urn-as-instanceid)
// When an IMEI is available, the instance-id shall take the form of a IMEI URN (see RFC 7254).
// The format of the instance-id shall take the form "urn:gsma:imei:" where by the imeival shall
// contain the IMEI encoded as defined in RFC 7254.
// The optional <sw-version-param> and <imei-version-param> parameters shall not be included
// in the instance-id.
// e.g. IMEI: "urn:gsma:imei:00000000-000000-0", IMEISV: urn:gsma:imei:00000000-000000-0;svn=00"

LOCAL
IMS_BOOL sipUrnHelper_IsUuidFallbackRequiredWhenNoImei(IN IMS_SINT32 /*nSlotId*/)
{
    // NOTE: will be integrated in the future.
    return IMS_FALSE;
}

PUBLIC GLOBAL AString SipUrnHelper::GetUrn(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nType)
{
    AStringBuffer objUrn(64);
    const IDeviceInfo* piDeviceInfo = PhoneInfoService::GetPhoneInfoService()->GetDeviceInfo();
    AString strImei;

    if (piDeviceInfo != IMS_NULL)
    {
        piDeviceInfo->GetDeviceId(nSlotId, strImei);
    }

    if (strImei.GetLength() == 0)
    {
        strImei = "00000000000000";

        // 3GPP - If no IMEI is available, so take the form of a string representation
        // of a UUID as a URN in RFC 4122.
        // (Name selection should be changed later)
        if (sipUrnHelper_IsUuidFallbackRequiredWhenNoImei(nSlotId) &&
                ((nType == GSMA_IMEI) || (nType == GSMA_IMEISV) || (nType == UUID_IMEI_MD5) ||
                        (nType == UUID_IMEI_SHA1)))
        {
            nType = UUID_IMEI_NAMED_V3;
        }
    }

    switch (nType)
    {
        case GSMA_IMEI:
        case GSMA_IMEISV:
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

            if (nType == GSMA_IMEISV)
            {
                AString strSv(2, '0');
                piDeviceInfo->GetDeviceSoftwareVersion(nSlotId, strSv);

                for (IMS_SINT32 i = strSv.GetLength(); i < 2; ++i)
                {
                    strSv.Prepend("0");
                }

                if (strSv.GetLength() > 2)
                {
                    strSv = strSv.GetSubStr(strSv.GetLength() - 2);
                }

                objUrn.Append(";svn=");
                objUrn.Append(strSv);
            }
            break;
        }
        case UUID_IMEI_MD5:
        {
            ImsMd5Context objMd5Ctx;
            IMS_UCHAR uacHashedImei[16];

            // tac (8) + snr (6)
            strImei = strImei.GetSubStr(0, 14);

            ImsMd5_Initialize(&objMd5Ctx);
            ImsMd5_Update(reinterpret_cast<const IMS_UCHAR*>(strImei.GetStr()), strImei.GetLength(),
                    &objMd5Ctx);
            ImsMd5_Finalize(&objMd5Ctx, uacHashedImei);

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
            ImsSha1Context objSha1Ctx;
            IMS_UCHAR uacHashedImei[20];

            // tac (8) + snr (6)
            strImei = strImei.GetSubStr(0, 14);

            ImsSha1_Initialize(&objSha1Ctx);
            ImsSha1_Update(reinterpret_cast<const IMS_UCHAR*>(strImei.GetStr()),
                    strImei.GetLength(), &objSha1Ctx);
            ImsSha1_Finalize(&objSha1Ctx, uacHashedImei);

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
            objUrn.Append(ImsUuid::GetUuid(ImsUuid::VERSION_3, strImei));
            break;
        }
        default:
            break;
    }

    return static_cast<const AStringBuffer&>(objUrn).GetString();
}

PUBLIC GLOBAL AString SipUrnHelper::GetUuidUrn(
        IN IMS_SINT32 nVersion, IN const AString& strName /* = AString::ConstNull()*/)
{
    AString strUuid = ImsUuid::GetUuid(nVersion, strName);

    if (strUuid.GetLength() == 0)
    {
        return AString::ConstNull();
    }

    AStringBuffer objUrn(64);

    objUrn.Append("urn:uuid:");
    objUrn.Append(strUuid);

    return static_cast<const AStringBuffer&>(objUrn).GetString();
}
