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
#include "ServiceTrace.h"
#include "ImsStrLib.h"
#include "utility/MtsSmUtils.h"
#include "MtsApp.h"

__IMS_TRACE_TAG_COM_SMS__;

PUBLIC
MtsSmUtils::MtsSmUtils()
{
    IMS_TRACE_I("+MtsSmUtils", 0, 0, 0);
}

PUBLIC
MtsSmUtils::~MtsSmUtils()
{
    IMS_TRACE_I("~MtsSmUtils", 0, 0, 0);
}

PUBLIC GLOBAL MtsSmUtils* MtsSmUtils::GetInstance(IN IMS_SINT32 nSlotId)
{
    static MtsSmUtils* m_pMtsSmUtil = IMS_NULL;

    IMS_TRACE_I("+GetInstance : Slot [%d]", nSlotId, 0, 0);

    if (m_pMtsSmUtil == IMS_NULL)
    {
        m_pMtsSmUtil = new MtsSmUtils();
    }

    return m_pMtsSmUtil;
}

PUBLIC
IMS_SINT32 MtsSmUtils::GetRpMr(IN const IMS_BYTE* pbySmsData)
{
    if (pbySmsData == IMS_NULL)
    {
        return (-1);
    }

    return static_cast<IMS_SINT32>(pbySmsData[1]);
}

PUBLIC
IMS_SINT32 MtsSmUtils::GetRpMr(IN const ByteArray& objSmsData)
{
    if (objSmsData.GetLength() < 2)
    {
        return (-1);
    }

    return GetRpMr(objSmsData.GetData());
}

PUBLIC
IMS_SINT32 MtsSmUtils::GetMti(IN const IMS_UINT32 nSmsType, IN const IMS_BYTE* pbySmsData)
{
    if (pbySmsData != IMS_NULL)
    {
        if (nSmsType == MtsApp::SMSFORMAT_3GPP)
        {
            return static_cast<IMS_SINT32>(pbySmsData[0] & 0x07);
        }
        else if (nSmsType == MtsApp::SMSFORMAT_3GPP2)
        {
            return static_cast<IMS_SINT32>(pbySmsData[0]);
        }
    }

    return (-1);
}

PUBLIC
IMS_SINT32 MtsSmUtils::GetMti(IN const IMS_UINT32 nSmsType, IN const ByteArray& objSmsData)
{
    if (objSmsData.GetLength() == 0)
    {
        return (-1);
    }

    return GetMti(nSmsType, objSmsData.GetData());
}

PUBLIC
void MtsSmUtils::PrintSmsDataBurst(IN const ByteArray& objSmsData)
{
    if (objSmsData.IsNULL())
    {
        return;
    }

    IMS_CHAR szTemp[3] = {
            0,
    };
    AString strSmsMsg = AString::ConstNull();

    for (IMS_SINT32 i = 0; i < objSmsData.GetLength(); i++)
    {
        IMS_Sprintf(szTemp, 2, "%2X", objSmsData[i]);

        strSmsMsg += szTemp;
    }

    IMS_TRACE_D("<< Received SMS data burst >> (%d) >>  %s", objSmsData.GetLength(),
            strSmsMsg.GetStr(), 0);
}

PUBLIC GLOBAL const IMS_CHAR* MtsSmUtils::GetMtiStringFrom3gpp(IN const IMS_SINT32 nMti)
{
    switch (nMti)
    {
        case MtsSmUtils::MTS_3GPP_MTI_RP_DATA_From_MS:
            return "MTS_3GPP_MTI_RP_DATA_From_MS";
        case MtsSmUtils::MTS_3GPP_MTI_RP_DATA_From_N:
            return "MTS_3GPP_MTI_RP_DATA_From_N";
        case MtsSmUtils::MTS_3GPP_MTI_RP_ACK_From_MS:
            return "MTS_3GPP_MTI_RP_ACK_From_MS";
        case MtsSmUtils::MTS_3GPP_MTI_RP_ACK_From_N:
            return "MTS_3GPP_MTI_RP_ACK_From_N";
        case MtsSmUtils::MTS_3GPP_MTI_RP_ERROR_From_MS:
            return "MTS_3GPP_MTI_RP_ERROR_From_MS";
        case MtsSmUtils::MTS_3GPP_MTI_RP_ERROR_From_N:
            return "MTS_3GPP_MTI_RP_ERROR_From_N";
        case MtsSmUtils::MTS_3GPP_MTI_RP_SMMA:
            return "MTS_3GPP_MTI_RP_SMMA";
        default:
            return "SMS 3GPP MTI INFO INVALID";
    }
}

PUBLIC GLOBAL const IMS_CHAR* MtsSmUtils::GetMtiStringFrom3gpp2(IN const IMS_SINT32 nMti)
{
    switch (nMti)
    {
        case MtsSmUtils::MTS_3GPP2_MTI_SMS_POINT_TO_POINT:
            return "MTS_3GPP2_MTI_SMS_POINT_TO_POINT";
        case MtsSmUtils::MTS_3GPP2_MTI_SMS_BROADCAST:
            return "MTS_3GPP2_MTI_SMS_BROADCAST";
        case MtsSmUtils::MTS_3GPP2_MTI_SMS_ACKNOWLEDGE:
            return "MTS_3GPP2_MTI_SMS_ACKNOWLEDGE";
        default:
            return "SMS 3GPP2 MTI INFO INVALID";
    }
}
