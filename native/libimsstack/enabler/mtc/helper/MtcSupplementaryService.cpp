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

PUBLIC
MtcSupplementaryService::MtcSupplementaryService(IN IMtcCallContext& objContext,
        IN MtcConfigurationProxy& objConfigurationProxy,
        IN const ImsMap<SuppType, SuppService*>& objSuppServices) :
        m_objContext(objContext),
        m_objSuppService(objSuppServices),
        m_objConfigurationProxy(objConfigurationProxy)
{
    IMS_TRACE_I("+MtcSupplementaryService", 0, 0, 0);
}

PUBLIC
MtcSupplementaryService::~MtcSupplementaryService()
{
    IMS_TRACE_I("~MtcSupplementaryService", 0, 0, 0);
    DeleteServices();
}

PUBLIC
void MtcSupplementaryService::UpdateOutgoingServices(
        IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_UINT32 nInServiceSize = objSuppServices.GetSize();
    IMS_TRACE_I("MtcSupplementaryService : ServiceNum[%d] InServiceNum[%d]",
            m_objSuppService.GetSize(), nInServiceSize, 0);

    for (IMS_UINT32 i = 0; i < nInServiceSize; i++)
    {
        const SuppType eType = objSuppServices.GetKeyAt(i);
        IMS_SLONG nIndex = m_objSuppService.GetIndexOfKey(eType);

        if (nIndex >= 0)
        {
            delete m_objSuppService.GetValueAt(nIndex);
        }

        m_objSuppService.Add(eType, objSuppServices.GetValueAt(i));
    }
}

PUBLIC
void MtcSupplementaryService::UpdateTip(IN IMessage* piMessage)
{
    AString strPrivacy = m_objContext.GetMessageUtils().GetHeader(piMessage, ISipHeader::PRIVACY);
    IMS_BOOL bHasPAssertedIdentity = m_objContext.GetMessageUtils().IsHeaderPresent(
            piMessage, ISipHeader::P_ASSERTED_IDENTITY);

    IMS_SINT32 tipType;
    AString tipStr;
    if (!bHasPAssertedIdentity && strPrivacy.EqualsIgnoreCase("id"))
    {
        tipType = TIP_TYPE_RESTRICTED;
    }
    else if (!bHasPAssertedIdentity && !(strPrivacy.EqualsIgnoreCase("id")))
    {
        tipType = TIP_TYPE_NONE;
    }
    else
    {
        tipType = TIP_TYPE_IDENTITY;
        AString strNumber = m_objContext.GetMessageUtils().GetUserPart(
                piMessage, ISipHeader::P_ASSERTED_IDENTITY);
        AString strName = m_objContext.GetMessageUtils().GetDisplayName(
                piMessage, ISipHeader::P_ASSERTED_IDENTITY);
        tipStr.Append(strNumber);
        tipStr.Append(',');
        tipStr.Append(strName);
    }
    Add(SuppType::TIP, tipType);
    Add(SuppType::TIP, tipStr);
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateIncomingServices(IN IMessage* piMessage)
{
    IMS_BOOL bUpdate = IMS_FALSE;

    bUpdate |= UpdateCallerId(piMessage);
    bUpdate |= UpdateCnap(piMessage);
    bUpdate |= UpdateCnapEx(piMessage);
    bUpdate |= UpdateMmc(piMessage);
    bUpdate |= UpdateGtt(piMessage);
    bUpdate |= UpdateCdivCause(piMessage);
    bUpdate |= UpdateCdivHistory(piMessage);
    bUpdate |= UpdateCw(piMessage);
    bUpdate |= UpdateVm(piMessage);
    bUpdate |= UpdateAnswerHold(piMessage);
    bUpdate |= UpdateMcid(piMessage);
    bUpdate |= UpdateDualNumber(piMessage);
    bUpdate |= UpdateCallingNumVerification(piMessage);
    bUpdate |= UpdateCallComposerElements(piMessage);

    IMS_TRACE_I("UpdateService : [%s]", _TRACE_B_(bUpdate), 0, 0);
    return bUpdate;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateCallerId(IN IMessage* piMessage)
{
    AString strPrivacy = m_objContext.GetMessageUtils().GetHeader(piMessage, ISipHeader::PRIVACY);
    if (strPrivacy.EqualsIgnoreCase("id"))
    {
        // 3GPP 24.607 requires to check absence of PAID as well in this case.
        Add(SuppType::CALLER_ID, static_cast<IMS_SINT32>(OipType::RESTRICTED));
        IMS_TRACE_I("UpdateCallerId Privacy header value is id", 0, 0, 0);
        return IMS_TRUE;
    }

    IMS_BOOL bPolicyFallBack =
            m_objConfigurationProxy.Is(Feature::ENABLE_OIP_HEADER_POLICY_FALLBACK);
    IMS_BOOL bOipSourceFromHeader = m_objConfigurationProxy.Is(Feature::OIP_SOURCE_FROM_HEADER);
    OipType eOipType = GetOipTypeByHeader(piMessage, bOipSourceFromHeader, bPolicyFallBack);
    if (eOipType == OipType::INVALID)
    {
        eOipType = OipType::NONE;
    }

    IMS_TRACE_I("UpdateCallerId FromHeader[%s] OIP-Type[%d]", _TRACE_B_(bOipSourceFromHeader),
            eOipType, 0);
    Add(SuppType::CALLER_ID, static_cast<IMS_SINT32>(eOipType));
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateCnap(IN IMessage* piMessage)
{
    AString strCnap;
    IMS_BOOL bPolicyFallBack =
            m_objConfigurationProxy.Is(Feature::ENABLE_OIP_HEADER_POLICY_FALLBACK);
    IMS_BOOL bOipSourceFromHeader = m_objConfigurationProxy.Is(Feature::OIP_SOURCE_FROM_HEADER);

    GetCnapByHeader(piMessage, bOipSourceFromHeader, strCnap, bPolicyFallBack);

    if (strCnap.GetLength() <= 0)
    {
        return IMS_FALSE;
    }

    Add(SuppType::CNAP, strCnap);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateCnapEx(IN IMessage* /*piMessage*/)
{
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateMmc(IN IMessage* /*piMessage*/)
{
    IMS_BOOL bUseMMC = m_objConfigurationProxy.Is(Feature::USE_MMC_SUPPLEMENTARY_SERVICE);

    if (bUseMMC)
    {
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateGtt(IN IMessage* /*piMessage*/)
{
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateCdivCause(IN IMessage* piMessage)
{
    ISipHeader* piHeader = GetHistoryInfoHeader(piMessage);

    if (piHeader == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nCause;

    if (GetCdivCause(piHeader->GetSipAddress(), nCause))
    {
        Add(SuppType::CDIV_CAUSE, ConvertCdivCause(nCause));
    }

    piHeader->Destroy();

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateCdivHistory(IN IMessage* piMessage)
{
    ISipHeader* piHeader = GetHistoryInfoHeader(piMessage);

    if (piHeader == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AString strTarget;

    if (GetCdivTarget(piHeader->GetSipAddress(), strTarget))
    {
        Add(SuppType::CDIV_HISTORY, strTarget);
    }

    piHeader->Destroy();

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateCw(IN IMessage* piMessage)
{
    if (m_objContext.GetMessageUtils().IsHeaderPresent(piMessage, ISipHeader::ALERT_INFO) ==
            IMS_FALSE)
    {
        return IMS_FALSE;
    }

    Add(SuppType::CW, IMS_TRUE);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateVm(IN IMessage* /*piMessage*/)
{
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateAnswerHold(IN IMessage* /*piMessage*/)
{
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateMcid(IN IMessage* /*piMessage*/)
{
    IMS_BOOL bUseMCID = m_objConfigurationProxy.Is(Feature::USE_MCID_SUPPLEMENTARY_SERVICE);

    if (bUseMCID)
    {
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateDualNumber(IN IMessage* /*piMessage*/)
{
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateCallingNumVerification(IN IMessage* piMessage)
{
    AString strValue = GetCnvParameterValue(piMessage);
    if (strValue.GetLength() <= 0)
    {
        return IMS_FALSE;
    }

    Add(SuppType::CALLING_NUM_VERIFICATION, GetCallingNumVerificationResult(strValue));
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateCallComposerElements(IN IMessage* piMessage)
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

    return IMS_TRUE;
}

PUBLIC
void MtcSupplementaryService::Delete(IN SuppType eType)
{
    IMS_SLONG nIndex = m_objSuppService.GetIndexOfKey(eType);

    if (nIndex >= 0)
    {
        SuppService* pSuppService = m_objSuppService.GetValueAt(nIndex);
        delete pSuppService;
        m_objSuppService.RemoveAt(nIndex);
        IMS_TRACE_I("Delete : size[%d] Type[%d]", m_objSuppService.GetSize(), eType, 0);
        return;
    }

    IMS_TRACE_I("Delete : NoT Matched Size[%d]", m_objSuppService.GetSize(), 0, 0);
}

PUBLIC
void MtcSupplementaryService::DeleteServices()
{
    IMS_UINT32 nSize = m_objSuppService.GetSize();

    IMS_TRACE_I("DeleteAll : Size[%d]", nSize, 0, 0);

    for (IMS_UINT32 index = 0; index < nSize; index++)
    {
        SuppService* pService = m_objSuppService.GetValueAt(index);
        delete pService;
    }

    m_objSuppService.Clear();
}

PUBLIC
const SuppService* MtcSupplementaryService::Get(IN SuppType eType)
{
    IMS_SLONG nIndex = m_objSuppService.GetIndexOfKey(eType);

    if (nIndex >= 0)
    {
        SuppService* pSuppService = m_objSuppService.GetValueAt(nIndex);
        return pSuppService;
    }

    IMS_TRACE_I("Get : NoT Matched, Size[%d]", m_objSuppService.GetSize(), 0, 0);
    return IMS_NULL;
}

PUBLIC
const ImsMap<SuppType, SuppService*>& MtcSupplementaryService::GetServices() const
{
    return m_objSuppService;
}

PUBLIC
void MtcSupplementaryService::Add(IN SuppType eSuppType, IN const AString& strValue)
{
    if (IsExist(eSuppType) == IMS_TRUE)
    {
        SuppService* pExistService = m_objSuppService.GetValue(eSuppType);
        pExistService->strValue = strValue;
    }
    else
    {
        SuppService* pUpdateService = new SuppService();
        pUpdateService->strValue = strValue;
        m_objSuppService.Add(eSuppType, pUpdateService);
    }
}

PUBLIC
void MtcSupplementaryService::Add(IN SuppType eSuppType, IN IMS_SINT32 nValue)
{
    if (IsExist(eSuppType) == IMS_TRUE)
    {
        SuppService* pExistService = m_objSuppService.GetValue(eSuppType);
        pExistService->nValue = nValue;
    }
    else
    {
        SuppService* pUpdateService = new SuppService();
        pUpdateService->nValue = nValue;
        m_objSuppService.Add(eSuppType, pUpdateService);
    }
}

PUBLIC
void MtcSupplementaryService::Add(IN SuppType eSuppType, IN IMS_BOOL bValue)
{
    if (IsExist(eSuppType) == IMS_TRUE)
    {
        SuppService* pExistService = m_objSuppService.GetValue(eSuppType);
        pExistService->bValue = bValue;
    }
    else
    {
        SuppService* pUpdateService = new SuppService();
        pUpdateService->bValue = bValue;
        m_objSuppService.Add(eSuppType, pUpdateService);
    }
}

PRIVATE
ISipHeader* MtcSupplementaryService::GetHistoryInfoHeader(IN IMessage* piMessage)
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
IMS_BOOL MtcSupplementaryService::GetCdivCause(
        IN const SipAddress* pAddress, OUT IMS_SINT32& nCause)
{
    if (pAddress != IMS_NULL)
    {
        const SipParameter* pSIPParameter = pAddress->GetParameter("cause");

        if (pSIPParameter != IMS_NULL)
        {
            const AString strCause = pSIPParameter->GetValue();

            if (strCause.GetLength() > 0)
            {
                IMS_TRACE_D("GetCDIVCause : Cause=%s", strCause.GetStr(), 0, 0);

                nCause = strCause.ToInt32();
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL MtcSupplementaryService::GetCdivTarget(
        IN const SipAddress* pAddress, OUT AString& strTarget)
{
    if (pAddress == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetCDIVTarget : pAddress is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    if (pAddress->IsSchemeSip() || pAddress->IsSchemeSips())
    {
        strTarget = pAddress->GetUser();
    }
    else if (pAddress->IsSchemeTel())
    {
        strTarget = pAddress->GetHost();
    }
    else
    {
        IMS_TRACE_I("GetCDIVTarget : Getting the target failed", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_D("GetCDIVTarget : Target=%s", strTarget.GetStr(), 0, 0);
    return IMS_TRUE;
}

PRIVATE
IMS_SINT32 MtcSupplementaryService::ConvertCdivCause(IN IMS_SINT32 nCause)
{
    IMS_SINT32 nCDIVCause;

    switch (nCause)
    {
        case 302:
            nCDIVCause = static_cast<IMS_SINT32>(CdivCause::UNCONDITION);
            break;
        case 404:
            nCDIVCause = static_cast<IMS_SINT32>(CdivCause::NOT_LOGGED_IN);
            break;
        case 408:
            nCDIVCause = static_cast<IMS_SINT32>(CdivCause::NO_REPLY);
            break;
        case 480:
            nCDIVCause = static_cast<IMS_SINT32>(CdivCause::DEFLECTION);
            break;
        case 486:
            nCDIVCause = static_cast<IMS_SINT32>(CdivCause::BUSY);
            break;
        case 487:
            nCDIVCause = static_cast<IMS_SINT32>(CdivCause::DEFLECTION_ALERTING);
            break;
        case 503:
            nCDIVCause = static_cast<IMS_SINT32>(CdivCause::NOT_REACHABLE);
            break;

        default:
            nCDIVCause = static_cast<IMS_SINT32>(CdivCause::NONE);
            break;
    }

    return nCDIVCause;
}

PRIVATE
IMS_SINT32 MtcSupplementaryService::GetCallingNumVerificationResult(IN const AString& strValue)
{
    IMS_SINT32 nVerstatResult = CALLING_NUM_VERSTAT_NONE;

    if (strValue.GetLength() > 0)
    {
        if (strValue.EqualsIgnoreCase(STR_VERSTAT_TN_VALIDATION_PASSED))
        {
            nVerstatResult = CALLING_NUM_VERSTAT_VERIFIED;
        }
        else if (strValue.EqualsIgnoreCase(STR_VERSTAT_TN_VALIDATION_FAILED))
        {
            nVerstatResult = CALLING_NUM_VERSTAT_NOT_VERIFIED;
        }
    }

    IMS_TRACE_D("GetCallingNumVerificationResult : result is [%d]", nVerstatResult, 0, 0);
    return nVerstatResult;
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
        IN IMessage* piMessage, IN IMS_BOOL bFromHeader, IN IMS_BOOL bDoFallBack)
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
        if (objAddr.GetDisplayName().EqualsIgnoreCase(MessageUtil::STR_ANONYMOUS) ||
                objAddr.GetUser().EqualsIgnoreCase(MessageUtil::STR_ANONYMOUS))
        {
            eOipType = OipType::RESTRICTED;
            break;
        }
        else if (objAddr.GetDisplayName().EqualsIgnoreCase(MessageUtil::STR_UNAVAILABLE) ||
                objAddr.GetUser().EqualsIgnoreCase(MessageUtil::STR_UNAVAILABLE))
        {
            // TODO: CarrierConfig.h : 0 - NONE, 1 - RESTRICTED
            if (m_objConfigurationProxy.GetInt(Feature::OIP_TYPE_FOR_UNAVAILABLE) == 0)
            {
                eOipType = OipType::NONE;
            }
            else
            {
                eOipType = OipType::RESTRICTED;
            }
            break;
        }

        eOipType = OipType::IDENTITY;
    }

    if (eOipType == OipType::INVALID && bDoFallBack)
    {
        return GetOipTypeByHeader(piMessage, !bFromHeader, IMS_FALSE);
    }

    return eOipType;
}

PRIVATE
void MtcSupplementaryService::GetCnapByHeader(IN IMessage* piMessage, IN IMS_BOOL bFromHeader,
        OUT AString& strCnap, IN IMS_BOOL bDoFallBack)
{
    IMS_SINT32 nDeterminationPolicyHeader =
            bFromHeader ? ISipHeader::FROM : ISipHeader::P_ASSERTED_IDENTITY;
    strCnap = m_objContext.GetMessageUtils().GetDisplayName(piMessage, nDeterminationPolicyHeader);

    if (strCnap.GetLength() <= 0 && bDoFallBack)
    {
        return GetCnapByHeader(piMessage, !bFromHeader, strCnap, IMS_FALSE);
    }
}

PRIVATE
IMS_BOOL MtcSupplementaryService::IsExist(IN SuppType suppType)
{
    IMS_SLONG nIndex = m_objSuppService.GetIndexOfKey(suppType);

    if (nIndex >= 0)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}
