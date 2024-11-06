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
#include "ServiceTrace.h"
#include "ServiceTimer.h"
#include "ServiceSystemTime.h"
#include "ServiceUtil.h"
#include "ServicePhoneInfo.h"
#include "Engine.h"
#include "IConfigurable.h"
#include "IConfiguration.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "IRegistration.h"
#include "ISipRtConfigHelper.h"
#include "SipFactory.h"
#include "Ims3gpp.h"
#include "TextParser.h"
#include "Sip.h"
#include "SipParameter.h"
#include "SipParsingHelper.h"
#include "provider/AosUtil.h"
#include "provider/AosString.h"

__IMS_TRACE_TAG_AOS__;

PUBLIC
AosUtil::AosUtil() :
        m_piSipConfigV(IMS_NULL),
        m_bIsWifiTest(IMS_FALSE)
{
    m_bIsWifiTest = (UtilService::GetUtilService()->GetPrivateProperty()->GetPersistentInt(
                             ImsPrivateProperties::Persistent::KEY_WIFI_TEST, 0) == 1);

    IMS_TRACE_D("AosUtil :: wifi(%d)", m_bIsWifiTest, 0, 0);
}

PUBLIC VIRTUAL AosUtil::~AosUtil()
{
    IMS_TRACE_D("~AosUtil()", 0, 0, 0);
}

PUBLIC GLOBAL AosUtil* AosUtil::GetInstance()
{
    static AosUtil* s_pUtil = IMS_NULL;

    if (s_pUtil == IMS_NULL)
    {
        s_pUtil = new AosUtil();
    }

    return s_pUtil;
}

PUBLIC
IMS_SINT32 AosUtil::GetResponseCode(IN const ISipMessage* piSipMsg)
{
    if (piSipMsg == IMS_NULL)
    {
        return -1;
    }

    return piSipMsg->GetStatusCode();
}

PUBLIC
IMS_UINT32 AosUtil::GetRetryAfterValue(IN const IRegistration* piRegistration)
{
    if (piRegistration == IMS_NULL)
    {
        return 0;
    }

    IMS_SINT32 nRetryAfter = GetRetryAfterValue(piRegistration->GetPreviousResponse());

    if (nRetryAfter > 0)
    {
        return static_cast<IMS_UINT32>(nRetryAfter);
    }

    return 0;
}

PUBLIC
IMS_SINT32 AosUtil::GetRetryAfterValue(IN const ISipMessage* piSipMsg)
{
    if (piSipMsg == IMS_NULL)
    {
        return -1;
    }

    AString strHeader = piSipMsg->GetHeader(ISipHeader::RETRY_AFTER_SEC);

    if (strHeader.GetLength() == 0)
    {
        return -1;
    }

    ISipHeader* piHeader = SipParsingHelper::CreateHeader(ISipHeader::RETRY_AFTER_SEC, strHeader);

    if (piHeader == IMS_NULL)
    {
        return -1;
    }

    IMS_SINT32 nValue = piHeader->GetValueInt();

    piHeader->Destroy();

    IMS_TRACE_I("GetRetryAfterValue :: (%d)", nValue, 0, 0);

    return nValue;
}

PUBLIC
IMS_SINT32 AosUtil::GetMinExpiresValue(IN const ISipMessage* piSipMsg)
{
    if (piSipMsg == IMS_NULL)
    {
        return -1;
    }

    AString strHeader = piSipMsg->GetHeader(ISipHeader::MIN_EXPIRES);

    if (strHeader.GetLength() == 0)
    {
        return -1;
    }

    ISipHeader* piHeader = SipParsingHelper::CreateHeader(ISipHeader::MIN_EXPIRES, strHeader);

    if (piHeader == IMS_NULL)
    {
        return -1;
    }

    IMS_SINT32 nValue = piHeader->GetValueInt();

    piHeader->Destroy();

    IMS_TRACE_I("GetMinExpiresValue :: (%d)", nValue, 0, 0);

    return nValue;
}

PUBLIC
IMS_BOOL AosUtil::IsInitialRegistrationRequired(IN const ISipMessage* piSipMsg)
{
    IMS_BOOL bInitialRegistration = IMS_FALSE;

    if (piSipMsg != IMS_NULL)
    {
        ImsList<ISipMessageBodyPart*> objBodyParts = piSipMsg->GetBodyParts();
        if (!objBodyParts.IsEmpty())
        {
            const ISipMessageBodyPart* piBodyPart = objBodyParts.GetAt(0);
            if (piBodyPart != IMS_NULL)
            {
                AString strContentTypeHdr =
                        piBodyPart->GetHeader(ISipMessageBodyPart::CONTENT_TYPE);

                AString strType, strSubType;
                TextParser::ParseMediaType(strContentTypeHdr, strType, strSubType);

                if (strType.EqualsIgnoreCase(AosString::STR_APPLICATION) &&
                        strSubType.EqualsIgnoreCase(AosString::STR_3GPP_IMS_XML))
                {
                    Ims3gpp* pIms3gpp = new Ims3gpp();
                    if (pIms3gpp->Parse(piBodyPart->GetContent().ToString()))
                    {
                        IMS_SINT32 nAction = pIms3gpp->GetAlternativeService().GetAction();
                        if (nAction == Ims3gpp::AlternativeService::ACTION_INITIAL_REGISTRATION)
                        {
                            bInitialRegistration = IMS_TRUE;
                        }
                    }
                    delete pIms3gpp;
                }
            }
        }
    }

    return bInitialRegistration;
}

PUBLIC
IMS_BOOL AosUtil::IsParameterIncluded(
        IN const ISipMessage* piSipMsg, IN IMS_SINT32 nHeaderType, IN const AString& strParameter)
{
    if (piSipMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ImsList<AString> objHeaders = piSipMsg->GetHeaders(nHeaderType);

    if (objHeaders.IsEmpty())
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 nAt = 0; nAt < objHeaders.GetSize(); ++nAt)
    {
        AString strHeader = objHeaders.GetAt(nAt);

        if (strHeader.Contains(strParameter))
        {
            IMS_TRACE_I("Parameter (%s) is in header [%d]", strParameter.GetStr(), nHeaderType, 0);
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL AosUtil::IsParameterIncluded(IN const ISipMessage* piSipMsg, IN IMS_SINT32 nHeaderType,
        IN const AString& strName, IN const AString& strParameter)
{
    if (piSipMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ImsList<AString> objHeaders = piSipMsg->GetHeaders(nHeaderType, strName);

    if (objHeaders.IsEmpty())
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 nAt = 0; nAt < objHeaders.GetSize(); ++nAt)
    {
        AString strHeader = objHeaders.GetAt(nAt);

        if (strHeader.Contains(strParameter))
        {
            IMS_TRACE_I(
                    "Parameter (%s) is in header [%s]", strParameter.GetStr(), strName.GetStr(), 0);
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_SINT32 AosUtil::GetLocalPort(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    const ISipConfig* piConfig = Engine::GetConfiguration()->GetSipConfig(nSlotId);
    IMS_SINT32 nPort = -1;

    if (piConfig != IMS_NULL)
    {
        nPort = piConfig->GetPort();
    }

    return nPort;
}

PUBLIC
void AosUtil::AddFeature(IN IMS_UINT32 nAdd, IN_OUT IMS_UINT32& nFeatures)
{
    nFeatures |= nAdd;
}

PUBLIC
void AosUtil::RemoveFeature(IN IMS_UINT32 nRemove, IN_OUT IMS_UINT32& nFeatures)
{
    nFeatures &= ~(nRemove);
}

PUBLIC
IMS_BOOL AosUtil::IsFeatureOn(IN IMS_UINT32 nFeature, IN IMS_UINT32 nFeatures)
{
    return (nFeatures & nFeature);
}

PUBLIC
void AosUtil::ClearFeature(IN_OUT IMS_UINT32& nFeatures)
{
    nFeatures = 0;
}

PUBLIC
ITimer* AosUtil::StartTimer(IN IMS_UINT32 nDuration, IN ITimerListener* piListener,
        IN AString strLog /* = AString("") */)
{
    ITimer* piTimer = TimerService::GetTimerService()->CreateTimer();
    IMS_UINTP nId = piTimer->SetTimer(nDuration, piListener);

    IMS_TRACE_I(
            "StartTimer :: id (%p) , type (%s) , duration (%d)", nId, strLog.GetStr(), nDuration);

    return piTimer;
}

PUBLIC
void AosUtil::StopTimer(IN ITimer*& piTimer, IN AString strLog /* = AString("") */)
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
void AosUtil::AddElementToList(IN IMS_UINT32 nElement, IN ImsList<IMS_UINT32>& objTarget)
{
    for (IMS_UINT32 nAt = 0; nAt < objTarget.GetSize(); nAt++)
    {
        IMS_UINT32 nCurrElement = objTarget.GetAt(nAt);
        if (nElement == nCurrElement)
        {
            return;
        }
    }

    objTarget.Append(nElement);
}

PUBLIC
IMS_BOOL AosUtil::IsListEqual(IN const AStringArray& objLeft, IN const AStringArray& objRight,
        IN IMS_BOOL bIsIpAddress /* = IMS_FALSE */)
{
    if (objLeft.GetCount() != objRight.GetCount())
    {
        return IMS_FALSE;
    }

    for (IMS_SINT32 nAt = 0; nAt < objLeft.GetCount(); ++nAt)
    {
        const AString& strL = objLeft.GetElementAt(nAt);
        IMS_BOOL bEqual = IMS_FALSE;

        for (IMS_SINT32 nRightAt = 0; nRightAt < objRight.GetCount(); ++nRightAt)
        {
            const AString& strR = objRight.GetElementAt(nRightAt);

            if (bIsIpAddress == IMS_TRUE)
            {
                IpAddress objIpAddrL(strL);
                IpAddress objIpAddrR(strR);

                if (objIpAddrL.Equals(objIpAddrR))
                {
                    bEqual = IMS_TRUE;
                    break;
                }
            }
            else
            {
                if (strL.Equals(strR))
                {
                    bEqual = IMS_TRUE;
                    break;
                }
            }
        }

        if (!bEqual)
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AosUtil::IsStrExistInList(IN const AString& strValue, IN const AStringArray& objList,
        IN IMS_BOOL bIsIpAddress /* = IMS_FALSE */)
{
    for (IMS_SINT32 nAt = 0; nAt < objList.GetCount(); ++nAt)
    {
        const AString& strCurr = objList.GetElementAt(nAt);

        if (bIsIpAddress == IMS_TRUE)
        {
            IpAddress objIpAddrCurr(strCurr);
            IpAddress objIpAddrValue(strValue);

            if (objIpAddrCurr.Equals(objIpAddrValue))
            {
                return IMS_TRUE;
            }
        }
        else
        {
            if (strCurr.Equals(strValue))
            {
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL AosUtil::IsListEqual(IN const ImsList<IMS_UINT32>& objLeft,
        IN const ImsList<IMS_UINT32>& objRight, IN IMS_BOOL bOrderChecked)
{
    if (objLeft.GetSize() != objRight.GetSize())
    {
        return IMS_FALSE;
    }

    if (bOrderChecked)
    {
        for (IMS_UINT32 nAt = 0; nAt < objLeft.GetSize(); nAt++)
        {
            if (objLeft.GetAt(nAt) != objRight.GetAt(nAt))
            {
                return IMS_FALSE;
            }
        }
    }
    else
    {
        for (IMS_UINT32 nAt = 0; nAt < objLeft.GetSize(); nAt++)
        {
            IMS_UINT32 nLeftValue = objLeft.GetAt(nAt);
            IMS_BOOL bEqual = IMS_FALSE;

            for (IMS_UINT32 nRightAt = 0; nRightAt < objRight.GetSize(); nRightAt++)
            {
                if (nLeftValue == objRight.GetAt(nRightAt))
                {
                    bEqual = IMS_TRUE;
                    break;
                }
            }

            if (!bEqual)
            {
                return IMS_FALSE;
            }
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AosUtil::IsElementExistInList(
        IN const ImsList<IMS_UINT32>& objElements, IN const ImsList<IMS_UINT32>& objTarget)
{
    for (IMS_UINT32 nAt = 0; nAt < objElements.GetSize(); nAt++)
    {
        IMS_UINT32 nElement = objElements.GetAt(nAt);

        for (IMS_UINT32 nListAt = 0; nListAt < objTarget.GetSize(); nListAt++)
        {
            IMS_UINT32 nTarget = objTarget.GetAt(nListAt);
            if (nElement == nTarget)
            {
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_UINT32 AosUtil::Pow(IN IMS_UINT32 nArg1, IN IMS_UINT32 nArg2)
{
    IMS_UINT32 nResult = 1;

    if (nArg2 == 0)
    {
        return nResult;
    }

    for (IMS_UINT32 i = 0; i < nArg2; i++)
    {
        nResult = nResult * nArg1;
    }

    IMS_TRACE_I("Pow :: (%d) ^ (%d) = (%d)", nArg1, nArg2, nResult);

    return nResult;
}

PUBLIC
IMS_UINT32 AosUtil::CalculateUpperBoundTime(
        IN IMS_UINT32 nBaseTime, IN IMS_UINT32 nMaxTime, IN IMS_UINT32 nConsecutiveFailCount)
{
    if (nConsecutiveFailCount > REASONABLE_MAX_FAILURE_COUNT)
    {
        IMS_TRACE_I("Consecutive Fail Count reach REASONABLE_MAX_FAILURE_COUNT", 0, 0, 0);
        return nMaxTime;
    }

    IMS_UINT32 nUpperBoundWaitTime = nBaseTime * (Pow(2, nConsecutiveFailCount));

    nUpperBoundWaitTime = (nUpperBoundWaitTime > nMaxTime) ? nMaxTime : nUpperBoundWaitTime;

    return nUpperBoundWaitTime;
}

PUBLIC
IMS_UINT32 AosUtil::WaitTimeForFlowRecovery(
        IN IMS_UINT32 nBaseTime, IN IMS_UINT32 nMaxTime, IN IMS_UINT32 nConsecutiveFailCount)
{
    IMS_UINT32 nUpperBoundWaitTime =
            CalculateUpperBoundTime(nBaseTime, nMaxTime, nConsecutiveFailCount);

    IMS_UINT32 nWaitTime = (nUpperBoundWaitTime / 2) + IMS_SYS_GetSRandom(nUpperBoundWaitTime / 2);

    IMS_TRACE_I("WaitTimeForFlowRecovery(), WaitTime(%d), BaseTime(%d), UpperBoundTime(%d)",
            nWaitTime, nBaseTime, nUpperBoundWaitTime);

    return nWaitTime;
}

PUBLIC
void AosUtil::SetSocketOption(
        IN IMS_UINT32 nOption, IN IMS_UINT32 nValue, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    IMS_TRACE_I("SocketOption: %d, value=%d", nOption, nValue, 0);

    ISipRtConfigHelper* piRunTimeConfigHelper = SipFactory::GetRtConfigHelper(nSlotId);
    SipRtConfig::SocketOption objSO;

    objSO.nValue = nValue;
    piRunTimeConfigHelper->SetConfig(nOption, &objSO);
}

PUBLIC
void AosUtil::SetSocketOptionLinger(IN IMS_UINT32 nOption, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    ISipRtConfigHelper* piRunTimeConfigHelper = SipFactory::GetRtConfigHelper(nSlotId);
    SipRtConfig::SocketOption objSO;

    objSO.nValue = nOption;

    piRunTimeConfigHelper->SetConfig(SipRtConfig::CONFIG_I_LINGER, &objSO);
}

PUBLIC
void AosUtil::SetSocketOptionShutDown(
        IN IMS_UINT32 nOption, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    ISipRtConfigHelper* piRunTimeConfigHelper = SipFactory::GetRtConfigHelper(nSlotId);
    SipRtConfig::SocketOption objSO;

    // 0 (RX) / 1 (TX) / 2 (RX & TX) / 3 (No Shutdown)
    objSO.nValue = nOption;

    piRunTimeConfigHelper->SetConfig(SipRtConfig::CONFIG_I_SHUTDOWN, &objSO);
}

PUBLIC
IMS_BOOL AosUtil::UpdateFeatureTagOptions(IN IMS_UINT32 nUpdatedFeatureTags,
        IN IMS_BOOL bIsSupported, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    const ISipConfigV* piSipConfigV;

    if (m_piSipConfigV != IMS_NULL)
    {
        piSipConfigV = m_piSipConfigV;
    }
    else
    {
        piSipConfigV = Engine::GetConfiguration()->GetSipConfig(nSlotId)->GetSipConfigV();
    }

    if (piSipConfigV != IMS_NULL)
    {
        IConfigurable* piConfigurable = piSipConfigV->GetConfigurable();

        if (piConfigurable != IMS_NULL)
        {
            IMS_UINT32 nOldFeatureTags = piSipConfigV->GetFeatureTagOptions();
            IMS_UINT32 nNewFeatureTags = 0;

            if (bIsSupported == IMS_TRUE)
            {
                nNewFeatureTags = nOldFeatureTags | nUpdatedFeatureTags;
            }
            else
            {
                nNewFeatureTags = nOldFeatureTags & (~nUpdatedFeatureTags);
            }

            if (nNewFeatureTags == nOldFeatureTags)
            {
                IMS_TRACE_I("No change in FeatureTags", 0, 0, 0);
                return IMS_FALSE;
            }

            AString strSipFeatures;
            strSipFeatures.Sprintf("0x%08x", nNewFeatureTags);

            if (!piConfigurable->Update(IConfigurable::CP_I_FEATURE_TAG_OPTIONS, strSipFeatures))
            {
                IMS_TRACE_D("Updating CP_I_SIP_FEATURES failed", 0, 0, 0);
                return IMS_FALSE;
            }

            IMS_TRACE_I("UpdateFeatureTagOptions (%s)", strSipFeatures.GetStr(), 0, 0);
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL AosUtil::IsSupportedNetworkType(IN IMS_UINT32 nType) const
{
    switch (nType)
    {
        case NW_REPORT_RADIO_LTE:  // FALL-THROUGH
        case NW_REPORT_RADIO_NR:   // FALL-THROUGH
        case NW_REPORT_RADIO_WLAN:
            return IMS_TRUE;

        default:
            return IMS_FALSE;
    }
}

PUBLIC
IMS_BOOL AosUtil::IsSupportedNetworkTypeForCellular(IN IMS_UINT32 nType) const
{
    return (nType == NW_REPORT_RADIO_LTE || nType == NW_REPORT_RADIO_NR);
}

PUBLIC
IMS_BOOL AosUtil::IsWifiTest() const
{
    return m_bIsWifiTest;
}

PUBLIC
IMS_BOOL AosUtil::IsDifferentCountry(IN AString strSimCountry, IN IMS_SINT32 nSlotId) const
{
    const ILocationProperties* piLocation =
            PhoneInfoService::GetPhoneInfoService()
                    ->GetLocationInfo(nSlotId)
                    ->GetLocationProperties(ILocationInfo::LOCATION_POSITION_N_COUNTRY);
    if (piLocation == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AString strCountry = piLocation->GetCountry();
    IMS_TRACE_I("IsDifferentCountry :: country (%s), sim country (%s)", strCountry.GetStr(),
            strSimCountry.GetStr(), 0);

    return !strCountry.Equals(strSimCountry);
}

PUBLIC
void AosUtil::SetISipConfigV(IN ISipConfigV* piSipConfigV)
{
    m_piSipConfigV = piSipConfigV;
}

PUBLIC
void AosUtil::SetWifiTest(IN IMS_BOOL bEnabled)
{
    m_bIsWifiTest = bEnabled;
}
