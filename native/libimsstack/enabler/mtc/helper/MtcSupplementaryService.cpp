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

#include "CarrierConfig.h"
#include "IMessage.h"
#include "ISipHeader.h"
#include "IuMtcService.h"
#include "MtcDef.h"
#include "ServiceThread.h"
#include "ServiceTrace.h"
#include "SipAddress.h"
#include "SipHeaderName.h"
#include "SipParameter.h"
#include "SipParsingHelper.h"
#include "call/IMtcCallContext.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MtcSupplementaryService.h"
#include "ussi/UssiController.h"
#include "utility/CallComposerUtil.h"
#include "utility/IMessageUtils.h"
#include "utility/MessageUtil.h"
#include <utility>

__IMS_TRACE_TAG_COM_MTC__;

LOCAL const IMS_CHAR STR_VERSTAT[] = "verstat";
LOCAL const IMS_CHAR STR_VERSTAT_TN_VALIDATION_PASSED[] = "TN-Validation-Passed";
LOCAL const IMS_CHAR STR_VERSTAT_TN_VALIDATION_FAILED[] = "TN-Validation-Failed";
LOCAL const IMS_CHAR STR_VERSTAT_POTENTIAL_SPAM[] = "Potential Spam";
LOCAL const IMS_CHAR STR_COIN_LINE_OR_PAYPHONE[] = "Coin line/payphone";
LOCAL const IMS_CHAR STR_INTERACTION_WITH_OTHER_SERVICE[] = "Interaction with other service";
LOCAL const IMS_CHAR STR_UNAVAILABLE[] = "Unavailable";

PUBLIC
MtcSupplementaryService::MtcSupplementaryService(
        IN IMtcCallContext& objContext, IN MtcConfigurationProxy& objConfigurationProxy) :
        m_objContext(objContext),
        m_objConfigurationProxy(objConfigurationProxy),
        m_objSuppServices(ImsList<SuppService*>())
{
    IMS_TRACE_I("+MtcSupplementaryService", 0, 0, 0);
}

PUBLIC
MtcSupplementaryService::~MtcSupplementaryService()
{
    IMS_TRACE_I("~MtcSupplementaryService", 0, 0, 0);
    SuppServiceUtils::DeleteServices(m_objSuppServices);
}

PUBLIC
void MtcSupplementaryService::UpdateServices(IN const ImsList<SuppService*>& objSuppServices)
{
    IMS_UINT32 nInServiceSize = objSuppServices.GetSize();
    IMS_TRACE_I("UpdateServices : ServiceNum[%d] InServiceNum[%d]", m_objSuppServices.GetSize(),
            nInServiceSize, 0);

    for (IMS_UINT32 i = 0; i < nInServiceSize; i++)
    {
        if (SuppServiceUtils::Get(m_objSuppServices, objSuppServices.GetAt(i)->nType) != IMS_NULL)
        {
            Delete(static_cast<SuppType>(objSuppServices.GetAt(i)->nType));
        }

        IMS_TRACE_I("UpdateServices : Append[%d]", objSuppServices.GetAt(i)->nType, 0, 0);
        m_objSuppServices.Append(objSuppServices.GetAt(i));
    }
}

PUBLIC
void MtcSupplementaryService::UpdateServices(IN IMessage* piMessage)
{
    IMS_TRACE_I("UpdateServices : update services using a received SIP message", 0, 0, 0);

    UpdateCallerId(piMessage);
    UpdateCnap(piMessage);
    UpdateCdiv(piMessage);
    UpdateCw(piMessage);
    UpdateCallingNumberVerification(piMessage);
    UpdateCallComposerElements(piMessage);
    UpdateSessionId(piMessage);
}

PUBLIC
void MtcSupplementaryService::UpdateTip(IN IMessage* piMessage)
{
    const AString strPrivacy =
            m_objContext.GetMessageUtils().GetHeader(piMessage, ISipHeader::PRIVACY);
    IMS_BOOL bHasPAssertedIdentity = m_objContext.GetMessageUtils().IsHeaderPresent(
            piMessage, ISipHeader::P_ASSERTED_IDENTITY);

    IMS_SINT32 eTipType;
    AString strTipNumberAndName;
    if (!bHasPAssertedIdentity && strPrivacy.EqualsIgnoreCase("id"))
    {
        eTipType = TIP_TYPE_RESTRICTED;
    }
    else if (!bHasPAssertedIdentity && !(strPrivacy.EqualsIgnoreCase("id")))
    {
        eTipType = TIP_TYPE_NONE;
    }
    else
    {
        eTipType = TIP_TYPE_IDENTITY;
        AString strNumber = m_objContext.GetMessageUtils().GetUserPart(
                piMessage, ISipHeader::P_ASSERTED_IDENTITY);
        AString strName = m_objContext.GetMessageUtils().GetDisplayName(
                piMessage, ISipHeader::P_ASSERTED_IDENTITY);
        strTipNumberAndName.Append(strNumber);
        strTipNumberAndName.Append(',');
        strTipNumberAndName.Append(strName);
    }
    Add(SuppType::TIP, eTipType);
    Add(SuppType::TIP, strTipNumberAndName);
}

PUBLIC
void MtcSupplementaryService::UpdateSessionId(IN const IMessage* piMessage)
{
    AString strSessionId =
            m_objContext.GetMessageUtils().GetHeaderValue(piMessage, ISipHeader::SESSION_ID);
    if (strSessionId.GetLength() > 0)
    {
        Add(SuppType::SESSION_ID, strSessionId);
    }
}

PUBLIC
void MtcSupplementaryService::UpdateCallerId(IN IMessage* piMessage)
{
    IMS_BOOL bOipSourceFromHeader =
            m_objConfigurationProxy.GetBoolean(ConfigVoice::KEY_OIP_SOURCE_FROM_HEADER_BOOL);
    IMS_BOOL bPolicyFallBack = m_objConfigurationProxy.GetBoolean(
            ConfigVoice::KEY_ENABLE_OIP_HEADER_POLICY_FALLBACK_BOOL);
    OipType eOipType = GetOipTypeByHeader(piMessage, bOipSourceFromHeader, bPolicyFallBack);
    if (eOipType == OipType::INVALID || eOipType == OipType::IDENTITY)
    {
        AString strPrivacy =
                m_objContext.GetMessageUtils().GetHeader(piMessage, ISipHeader::PRIVACY);
        if (strPrivacy.EqualsIgnoreCase("id"))
        {
            eOipType = OipType::RESTRICTED;
            IMS_TRACE_I("UpdateCallerId Privacy header value is id", 0, 0, 0);
        }
        else if (eOipType == OipType::INVALID)
        {
            eOipType = OipType::NONE;
        }
    }

    IMS_TRACE_I("UpdateCallerId FromHeader[%s] OIP-Type[%d]", _TRACE_B_(bOipSourceFromHeader),
            eOipType, 0);
    Add(SuppType::CALLER_ID, static_cast<IMS_SINT32>(eOipType));
}

PUBLIC
void MtcSupplementaryService::UpdateCnap(IN IMessage* piMessage)
{
    IMS_BOOL bOipSourceFromHeader =
            m_objConfigurationProxy.GetBoolean(ConfigVoice::KEY_OIP_SOURCE_FROM_HEADER_BOOL);
    IMS_BOOL bPolicyFallBack = m_objConfigurationProxy.GetBoolean(
            ConfigVoice::KEY_ENABLE_OIP_HEADER_POLICY_FALLBACK_BOOL);
    AString strCnap = GetCnapByHeader(piMessage, bOipSourceFromHeader, bPolicyFallBack);
    if (strCnap.GetLength() <= 0)
    {
        return;
    }

    Add(SuppType::CNAP, strCnap);
}

PUBLIC
void MtcSupplementaryService::UpdateCdiv(IN const IMessage* piMessage)
{
    ISipHeader* piHeader = GetHistoryInfoHeader(piMessage);

    if (piHeader == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nCause = GetCdivCause(piHeader->GetSipAddress());
    if (nCause >= 0)
    {
        Add(SuppType::CDIV_CAUSE, ConvertCdivCause(nCause));
    }

    AString strTarget = GetCdivTarget(piHeader->GetSipAddress());
    if (strTarget.GetLength() > 0)
    {
        Add(SuppType::CDIV_HISTORY, strTarget);
    }

    piHeader->Destroy();
}

PUBLIC
void MtcSupplementaryService::UpdateCw(IN const IMessage* piMessage)
{
    if (m_objContext.GetMessageUtils()
                    .GetHeaderValue(piMessage, ISipHeader::ALERT_INFO)
                    .Equals(MessageUtil::STR_ALERT_URN_CALL_WAITING))
    {
        IMS_TRACE_I("UpdateCw", 0, 0, 0);
        Add(SuppType::CW, IMS_TRUE);
    }
}

PUBLIC
void MtcSupplementaryService::UpdateCallingNumberVerification(IN IMessage* piMessage)
{
    AString strVerstatParameter = GetCnvParameterValue(piMessage);
    if (strVerstatParameter.GetLength() <= 0)
    {
        return;
    }

    AString strDisplayName = m_objContext.GetMessageUtils().GetDisplayName(
            piMessage, ISipHeader::P_ASSERTED_IDENTITY);

    IMS_SINT32 nResult = GetCallingNumberVerificationResult(strVerstatParameter, strDisplayName);
    IMS_TRACE_D("UpdateCallingNumberVerification : result[%d]", nResult, 0, 0);

    Add(SuppType::CALLING_NUM_VERIFICATION, nResult);
}

PUBLIC
void MtcSupplementaryService::UpdateCallComposerElements(IN const IMessage* piMessage)
{
    IMS_SINT32 nPriority = CallComposerUtil::GetPriority(*piMessage);
    if (nPriority >= 0)
    {
        Add(SuppType::CALL_COMPOSER_PRIORITY, nPriority);
    }

    AString strSubject = CallComposerUtil::GetSubject(*piMessage);
    if (strSubject.GetLength() > 0)
    {
        Add(SuppType::CALL_COMPOSER_SUBJECT, strSubject);
    }

    AString strPicture = CallComposerUtil::GetPicture(*piMessage);
    if (strPicture.GetLength() > 0)
    {
        Add(SuppType::CALL_COMPOSER_PICTURE_URL, strPicture);
    }

    std::pair<AString, AString> objLocation = CallComposerUtil::GetLocation(*piMessage);
    if (objLocation.first.GetLength() > 0 && objLocation.second.GetLength() > 0)
    {
        Add(SuppType::CALL_COMPOSER_LOCATION_LAT, objLocation.first);
        Add(SuppType::CALL_COMPOSER_LOCATION_LONG, objLocation.second);
    }

    if (CallComposerUtil::IsBusiness(*piMessage))
    {
        Add(SuppType::CALL_COMPOSER_IS_BUSINESS, IMS_TRUE);
    }
}

PUBLIC GLOBAL void MtcSupplementaryService::ConvertGlobalNumberToLocalNumber(
        IN const MtcConfigurationProxy& objConfigurationProxy, IN_OUT AString& strNumber)
{
    AString strSet =
            objConfigurationProxy.GetString(ConfigVoice::KEY_LOCAL_NUMBER_PRESENTATION_SET_STRING);
    if (strSet.GetLength() == 0 || strNumber.GetLength() == 0)
    {
        return;
    }

    AString strInternationalNumberPrefix;
    AString strLocalNumberPrefix;
    strSet.SplitF(TextParser::CHAR_COLON, strInternationalNumberPrefix, strLocalNumberPrefix);
    strNumber.Replace(strInternationalNumberPrefix, strLocalNumberPrefix);
}

PRIVATE
ISipHeader* MtcSupplementaryService::GetHistoryInfoHeader(IN const IMessage* piMessage) const
{
    ImsList<AString> lstHistoryInfos =
            m_objContext.GetMessageUtils().GetHeaders(piMessage, ISipHeader::HISTORY_INFO);
    if (lstHistoryInfos.IsEmpty())
    {
        return IMS_NULL;
    }

    const AString& strHistoryInfo = lstHistoryInfos.GetAt(lstHistoryInfos.GetSize() - 1);
    if (strHistoryInfo.GetLength() == 0)
    {
        return IMS_NULL;
    }

    return SipParsingHelper::CreateHeader(SipHeaderName::HISTORY_INFO, strHistoryInfo);
}

PRIVATE
AString MtcSupplementaryService::GetCnvParameterValue(IN IMessage* piMessage) const
{
    AString strValue = m_objContext.GetMessageUtils().GetParameterValueFromUri(
            piMessage, STR_VERSTAT, ISipHeader::P_ASSERTED_IDENTITY);
    if (strValue.GetLength() > 0)
    {
        return strValue;
    }

    return m_objContext.GetMessageUtils().GetParameterValueFromUri(
            piMessage, STR_VERSTAT, ISipHeader::FROM);
}

PRIVATE
OipType MtcSupplementaryService::GetOipTypeByHeader(
        IN IMessage* piMessage, IN IMS_BOOL bFromHeader, IN IMS_BOOL bDoFallBack) const
{
    OipType eOipType = OipType::INVALID;
    IMS_SINT32 nDeterminationPolicyHeader =
            bFromHeader ? ISipHeader::FROM : ISipHeader::P_ASSERTED_IDENTITY;
    ImsList<AString> objHeaders =
            m_objContext.GetMessageUtils().GetHeaders(piMessage, nDeterminationPolicyHeader);
    for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); i++)
    {
        // only PAID can be multiple.
        SipAddress objAddr(objHeaders.GetAt(i));
        const AString& strDisplayName = objAddr.GetDisplayName();
        if (strDisplayName.EqualsIgnoreCase(STR_COIN_LINE_OR_PAYPHONE))
        {
            eOipType = OipType::PAYPHONE;
            break;
        }

        if (strDisplayName.EqualsIgnoreCase(STR_INTERACTION_WITH_OTHER_SERVICE) ||
                strDisplayName.EqualsIgnoreCase(STR_UNAVAILABLE))
        {
            eOipType = static_cast<OipType>(
                    m_objConfigurationProxy.GetInt(ConfigVoice::KEY_OIP_TYPE_FOR_UNAVAILABLE_INT));
            break;
        }

        if (strDisplayName.EqualsIgnoreCase(MessageUtil::STR_ANONYMOUS) ||
                objAddr.GetUser().EqualsIgnoreCase(MessageUtil::STR_ANONYMOUS))
        {
            eOipType = OipType::RESTRICTED;
            break;
        }

        eOipType = OipType::IDENTITY;
    }

    if (eOipType == OipType::INVALID && bDoFallBack)
    {
        return GetOipTypeByHeader(piMessage, !bFromHeader, IMS_FALSE);
    }

    IMS_TRACE_D("GetOipTypeByHeader OIP-Type[%d]", eOipType, 0, 0);
    return eOipType;
}

PRIVATE
AString MtcSupplementaryService::GetCnapByHeader(
        IN IMessage* piMessage, IN IMS_BOOL bFromHeader, IN IMS_BOOL bDoFallBack) const
{
    IMS_SINT32 nDeterminationPolicyHeader =
            bFromHeader ? ISipHeader::FROM : ISipHeader::P_ASSERTED_IDENTITY;
    AString strCnap =
            m_objContext.GetMessageUtils().GetDisplayName(piMessage, nDeterminationPolicyHeader);

    if (strCnap.GetLength() <= 0 && bDoFallBack)
    {
        return GetCnapByHeader(piMessage, !bFromHeader, IMS_FALSE);
    }
    return strCnap;
}

PRIVATE GLOBAL IMS_SINT32 MtcSupplementaryService::GetCdivCause(IN const SipAddress* pAddress)
{
    if (pAddress == IMS_NULL)
    {
        return -1;
    }

    const SipParameter* pSIPParameter = pAddress->GetParameter("cause");
    if (pSIPParameter != IMS_NULL)
    {
        const AString strCause = pSIPParameter->GetValue();

        if (strCause.GetLength() > 0)
        {
            IMS_TRACE_D("GetCDIVCause : Cause=%s", strCause.GetStr(), 0, 0);

            return strCause.ToInt32();
        }
    }

    return -1;
}

PRIVATE GLOBAL AString MtcSupplementaryService::GetCdivTarget(IN const SipAddress* pAddress)
{
    if (pAddress == IMS_NULL)
    {
        return AString::ConstNull();
    }

    if (pAddress->IsSchemeSip() || pAddress->IsSchemeSips())
    {
        return pAddress->GetUser();
    }
    else if (pAddress->IsSchemeTel())
    {
        return pAddress->GetHost();
    }

    IMS_TRACE_I("GetCdivTarget : Getting the target failed", 0, 0, 0);
    return AString::ConstNull();
}

PRIVATE GLOBAL IMS_SINT32 MtcSupplementaryService::ConvertCdivCause(IN IMS_SINT32 nCause)
{
    switch (nCause)
    {
        case 302:
            return static_cast<IMS_SINT32>(CdivCause::UNCONDITION);
        case 404:
            return static_cast<IMS_SINT32>(CdivCause::NOT_LOGGED_IN);
        case 408:
            return static_cast<IMS_SINT32>(CdivCause::NO_REPLY);
        case 480:
            return static_cast<IMS_SINT32>(CdivCause::DEFLECTION);
        case 486:
            return static_cast<IMS_SINT32>(CdivCause::BUSY);
        case 487:
            return static_cast<IMS_SINT32>(CdivCause::DEFLECTION_ALERTING);
        case 503:
            return static_cast<IMS_SINT32>(CdivCause::NOT_REACHABLE);
        default:
            return static_cast<IMS_SINT32>(CdivCause::NONE);
    }
}

PRIVATE GLOBAL IMS_SINT32 MtcSupplementaryService::GetCallingNumberVerificationResult(
        IN const AString& strVerstatParameter, IN const AString& strDisplayName)
{
    if (strVerstatParameter.EqualsIgnoreCase(STR_VERSTAT_TN_VALIDATION_PASSED))
    {
        if (strDisplayName.EqualsIgnoreCase(STR_VERSTAT_POTENTIAL_SPAM))
        {
            return CALLING_NUM_VERSTAT_NOT_VERIFIED;
        }
        return CALLING_NUM_VERSTAT_VERIFIED;
    }

    if (strVerstatParameter.EqualsIgnoreCase(STR_VERSTAT_TN_VALIDATION_FAILED))
    {
        return CALLING_NUM_VERSTAT_NOT_VERIFIED;
    }

    return CALLING_NUM_VERSTAT_NONE;
}
