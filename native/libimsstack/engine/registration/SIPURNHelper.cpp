/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20120214  hwangoo.park@             Created
    </table>

    Description
*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServicePhoneInfo.h"
#include "IMSMD5.h"
#include "IMSSHA1.h"
#include "IMSUUID.h"
#include "SIPURNHelper.h"

__IMS_TRACE_TAG_REG__;

// 3GPP 23.003 13.8 clause / draft-allen-dispatch-imei-urn-as-instanceid
#define __IMS_SIP_IMEI_URN_AS_INSTANCE_ID__

#if defined(__IMS_SIP_IMEI_URN_AS_INSTANCE_ID__)
PRIVATE GLOBAL const IMS_CHAR SIPURNHelper::IMEI[] = "urn:gsma:imei:00000000-000000-0";
PRIVATE GLOBAL const IMS_CHAR SIPURNHelper::IMEI_SV[] = "urn:gsma:imei:00000000-000000-0";
#else
PRIVATE GLOBAL const IMS_CHAR SIPURNHelper::IMEI[] = "urn:gsma:imei:00000000-000000-0;vers=0";
PRIVATE GLOBAL const IMS_CHAR SIPURNHelper::IMEI_SV[] = "urn:gsma:imei:00000000-000000-0;svn=00";
#endif

LOCAL
IMS_BOOL sipURNHelper_IsUuidFallbackRequiredWhenNoImei(IN IMS_SINT32 /* nSlotId*/)
{
    return IMS_FALSE;
}

/*

Remarks

*/
PUBLIC GLOBAL AString SIPURNHelper::GetURN(
        IN IMS_SINT32 nSlotId, IN IMS_SINT32 nType, IN IMS_BOOL bSV /* = IMS_TRUE */)
{
    AStringBuffer objURN(64);
    IDeviceInfo* piDeviceInfo = PhoneInfoService::GetPhoneInfoService()->GetDeviceInfo();

    //---------------------------------------------------------------------------------------------

    if (piDeviceInfo == IMS_NULL)
    {
        if (bSV)
        {
            return IMEI_SV;
        }
        else
        {
            return IMEI;
        }
    }

    AString strIMEI;

    piDeviceInfo->GetDeviceId(nSlotId, strIMEI);

    if (strIMEI.GetLength() == 0)
    {
        // 3GPP - If no IMEI is available, so take the form of a string representation
        // of a UUID as a URN in RFC 4122.
        // 4 Name selection should be changed lator
        strIMEI = "00000000000000";

        if (sipURNHelper_IsUuidFallbackRequiredWhenNoImei(nSlotId) &&
                ((nType == GSMA_IMEI) || (nType == UUID_IMEI_MD5) || (nType == UUID_IMEI_SHA1)))
        {
            nType = UUID_IMEI_NAMED_V5;
        }
    }

    switch (nType)
    {
        case GSMA_IMEI:
        {
            objURN.Append("urn:gsma:imei:");

            // IMEI = tac (8) "-" snr (6) "-" spare (1)
            if (strIMEI.GetLength() < 14)
            {
                for (IMS_SINT32 i = strIMEI.GetLength(); i < 14; ++i)
                {
                    strIMEI.Append("0");
                }
            }

            objURN.Append(strIMEI.GetSubStr(0, 8));
            objURN.Append('-');
            objURN.Append(strIMEI.GetSubStr(8, 6));
            objURN.Append('-');
            // spare field is always set to zero
            objURN.Append('0');
#if !defined(__IMS_SIP_IMEI_URN_AS_INSTANCE_ID__)
            objURN.Append(';');

            if (bSV)
            {
                AString strSV;

                piDeviceInfo->GetDeviceSoftwareVersion(strSV);

                objURN.Append("svn=");

                if (strSV.GetLength() == 0)
                {
                    objURN.Append("00");
                }
                else if (strSV.GetLength() == 1)
                {
                    objURN.Append("0");
                    objURN.Append(strSV);
                }
                else
                {
                    objURN.Append(strSV);
                }
            }
            else
            {
                objURN.Append("vers=0");
            }
#endif  // #if !defined(__IMS_SIP_IMEI_URN_AS_INSTANCE_ID__)
        }
        break;

        case UUID_IMEI_MD5:
        {
            MD5Context stMD5;
            IMS_UCHAR uacHashedIMEI[16];

            // tac (8) + snr (6)
            strIMEI = strIMEI.GetSubStr(0, 14);

            IMSMD5_Initialize(&stMD5);
            IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(strIMEI.GetStr()), strIMEI.GetLength(),
                    &stMD5);
            IMSMD5_Finalize(&stMD5, uacHashedIMEI);

            AString strHex;

            objURN.Append("urn:uuid:");

            for (IMS_SINT32 i = 0; i < 16; ++i)
            {
                strHex.Sprintf("%02x", uacHashedIMEI[i]);
                objURN.Append(strHex);

                if ((i == 3) || (i == 5) || (i == 7) || (i == 9))
                {
                    objURN.Append('-');
                }
            }
        }
        break;

        case UUID_IMEI_SHA1:
        {
            SHA1Context stSHA1;
            IMS_UCHAR uacHashedIMEI[20];

            // tac (8) + snr (6)
            strIMEI = strIMEI.GetSubStr(0, 14);

            IMSSHA1_Initialize(&stSHA1);
            IMSSHA1_Update(reinterpret_cast<const IMS_UCHAR*>(strIMEI.GetStr()),
                    strIMEI.GetLength(), &stSHA1);
            IMSSHA1_Finalize(&stSHA1, uacHashedIMEI);

            AString strHex;

            objURN.Append("urn:uuid:");

            for (IMS_SINT32 i = 0; i < 16; ++i)
            {
                strHex.Sprintf("%02x", uacHashedIMEI[i]);
                objURN.Append(strHex);

                if ((i == 3) || (i == 5) || (i == 7) || (i == 9))
                {
                    objURN.Append('-');
                }
            }
        }
        break;

        case UUID_IMEI_NAMED_V3:
        {
            // tac (8) + snr (6)
            strIMEI = strIMEI.GetSubStr(0, 14);

            objURN.Append("urn:uuid:");
            objURN.Append(IMSUUID::GetUUID(IMSUUID::VERSION_3, strIMEI));
        }
        break;

        case UUID_IMEI_NAMED_V5:
        {
            // tac (8) + snr (6)
            strIMEI = strIMEI.GetSubStr(0, 14);

            objURN.Append("urn:uuid:");
            objURN.Append(IMSUUID::GetUUID(IMSUUID::VERSION_5, strIMEI));
        }
        break;

        case UUID_IMEI_V4:
        {
            // tac (8) + snr (6)
            strIMEI = strIMEI.GetSubStr(0, 14);

            objURN.Append("urn:uuid:");
            objURN.Append(IMSUUID::GetUUID(IMSUUID::VERSION_4, strIMEI));
        }
        break;

        default:
            break;
    }

    return static_cast<const AStringBuffer&>(objURN).GetString();
}

/*

Remarks

*/
PUBLIC GLOBAL AString SIPURNHelper::GetURN(IN IMS_SINT32 nVersion, IN CONST AString& strName)
{
    AStringBuffer objURN(64);

    //---------------------------------------------------------------------------------------------

    objURN.Append("urn:uuid:");
    objURN.Append(IMSUUID::GetUUID(nVersion, strName));

    return static_cast<const AStringBuffer&>(objURN).GetString();
}
