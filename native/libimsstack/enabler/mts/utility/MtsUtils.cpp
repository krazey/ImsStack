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

#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "ServiceTimer.h"
#include "ServiceUtil.h"
#include "IMSStrLib.h"
#include "utility/MtsUtils.h"


__IMS_TRACE_TAG_COM_SMS__;

PUBLIC GLOBAL const IMS_CHAR MtsUtils::PROPERTY_RIL_ECCLIST[] = "ril.ecclist";

PUBLIC GLOBAL const IMS_CHAR MtsUtils::PROPERTY_FEATURESET_SMSTO911[] =
        "persist.product.xxx.sms.smsto911.enable";

const IMS_CHAR MtsUtils::PROPERTY_SCBM_MODE[] = "persist.product.xxx.sms.smsto911.scbm.state";

PUBLIC
MtsUtils::MtsUtils() :
        m_bIsScbm(IMS_FALSE)
{
    IMS_TRACE_I("+MtsUtils", 0, 0, 0);
}

PUBLIC
MtsUtils::~MtsUtils()
{
    IMS_TRACE_I("~MtsUtils", 0, 0, 0);
}

PUBLIC GLOBAL MtsUtils* MtsUtils::GetInstance()
{
    static MtsUtils* pMtsUtils = IMS_NULL;

    if (pMtsUtils == IMS_NULL)
    {
        pMtsUtils = new MtsUtils();
    }

    return pMtsUtils;
}

PUBLIC GLOBAL const IMS_CHAR* MtsUtils::MtsTimerToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case TIMER_MTS_CALLBACK_MODE:
            return "TIMER_MTS_CALLBACK_MODE";
        default:
            return "__INVALID__";
    }
}

PUBLIC
ITimer* MtsUtils::StartTimer(IN IMS_UINT32 nDuration, IN ITimerListener* piListener,
        IN AString strLog /* = AString("") */)
{
    ITimer* piTimer = TimerService::GetTimerService()->CreateTimer();
    IMS_UINTP nID = piTimer->SetTimer(nDuration, piListener);

    IMS_TRACE_I(
            "StartTimer :: id (%p) , type (%s) , duration (%d)", nID, strLog.GetStr(), nDuration);

    return piTimer;
}

PUBLIC
void MtsUtils::StopTimer(IN ITimer*& piTimer, IN AString strLog /* = AString("") */)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(piTimer);
    piTimer = IMS_NULL;

    IMS_TRACE_I("StopTimer :: type (%s)", strLog.GetStr(), 0, 0);
}

PUBLIC
IMS_BOOL MtsUtils::IsEccNumber(IN const IMS_CHAR* strDstAddr, IMS_SINT32 nSlotId)
{
    IMS_BOOL bIsEccNumber = IMS_FALSE;
    AString strDestAddr(strDstAddr);

    if (PhoneInfoService::GetPhoneInfoService()->GetCallInfo(nSlotId)
            ->IsEmergencyNumber(strDestAddr))
    {
        IMS_TRACE_I("IsEccNumber:This Number( %s ) is a ECC Number from PhoneInfoService",
                strDestAddr.GetStr(), 0, 0);
        return IMS_TRUE;
    }

    AString strEccListStr = "";
    IMSList<AString> eccList = strEccListStr.Split(',');

    for (IMS_UINT32 i = 0; i < eccList.GetSize(); i++)
    {
        if (eccList.GetAt(i).Equals(strDestAddr))
        {
            bIsEccNumber = IMS_TRUE;
            break;
        }
    }
    return bIsEccNumber;
}

PUBLIC
IMS_BOOL MtsUtils::IsEpdgConnected(IN MtsService* pMtsService)
{
    if (pMtsService != IMS_NULL)
    {
        return pMtsService->IsEpdgConnected();
    }
    else
    {
        return IMS_FALSE;
    }
}

PUBLIC
IMS_BOOL MtsUtils::IsSupportFeature(IN const IMS_CHAR* pszProperty)
{
    IMS_BOOL bIsSupportProperty = AString("true").Equals(
            UtilService::GetUtilService()->GetSystemProperty()->Get(AString(pszProperty)));
    IMS_TRACE_I("IsSupportFeature: bIsSupportProperty = %d", bIsSupportProperty, 0, 0);
    return bIsSupportProperty;
}

PUBLIC
void MtsUtils::SetScbm(IN IMS_BOOL bIsScbmTimerStatus)
{
    m_bIsScbm = bIsScbmTimerStatus;
    IMS_TRACE_I("SetScbm: %d", bIsScbmTimerStatus, 0, 0);

    // FIXME: SCBM
}

PUBLIC
IMS_BOOL MtsUtils::GetScbm()
{
    return m_bIsScbm;
}
