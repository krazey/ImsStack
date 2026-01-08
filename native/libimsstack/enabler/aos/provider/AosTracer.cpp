/*
 * Copyright (C) 2025 The Android Open Source Project
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
#include "provider/AosTracer.h"
#include "provider/AosProvider.h"
#include "provider/AosLog.h"
#include "IAosService.h"

#include "interface/IAosBlock.h"
#include "interface/IAosApplication.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosNetTracker.h"
#include "interface/IAosRegistration.h"

__IMS_TRACE_TAG_AOS__;

#define AOSTAG m_strTag.GetStr()

PUBLIC
AosTracer::AosTracer(IN IMS_SINT32 nSlotId) :
        m_nSlotId(nSlotId)
{
    m_strTag.Sprintf("%d", m_nSlotId);
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [SLOT%d] AosTracer = %" PFLS_u "/%" PFLS_x, m_nSlotId,
            sizeof(AosTracer), this);
}

PUBLIC
AosTracer::~AosTracer()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [SLOT%d] AosTracer = %" PFLS_u "/%" PFLS_x, m_nSlotId,
            sizeof(AosTracer), this);
}

PUBLIC
void AosTracer::TraceStatus(IN IAosAppContext* piContext, IN const AosStatusInfo& objStatus)
{
    AosRegistrationType eRegType;
    if (!IsTraceTarget(piContext, eRegType))
    {
        return;
    }

    AString strCriticalLog = FormatCriticalKey(objStatus);

    if (strCriticalLog.Equals(m_strLastCriticalLog))
    {
        return;
    }
    m_strLastCriticalLog = strCriticalLog;

    AString strLog;
    strLog.Sprintf("[APP][I](%s) Reg{%s %s} Conn{%s %s} Off=%s DatFail=%d",
            AosLog::AppStateToString(objStatus.m_nAppState),
            AosLog::RegResultToString(objStatus.m_nRegResult),
            AosLog::RegReasonForResultToString(objStatus.m_nRegReason),
            AosLog::AppConnectionStateToString(objStatus.m_nConnState),
            AosLog::ConnectorReasonToString(objStatus.m_nConnReason),
            AosLog::AosReasonToString(objStatus.m_nOffReason), objStatus.m_nDataFailureReason);

    IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
    if (piService != IMS_NULL)
    {
        piService->NotifyTrace(eRegType, strLog);
    }
}

PUBLIC
void AosTracer::TraceEvent(IN IAosAppContext* piContext, IN AosDomain eDomain,
        IN AosSeverity eSeverity, IN const IMS_CHAR* pszEventName, IN const IMS_CHAR* pszEventValue)
{
    AosRegistrationType eRegType;
    if (!IsTraceTarget(piContext, eRegType))
    {
        return;
    }

    AString strLog;
    strLog.Sprintf(
            "[%s][%s] %s", DomainToString(eDomain), SeverityToString(eSeverity), pszEventName);

    if (pszEventValue != IMS_NULL)
    {
        strLog.Append(" : ");
        strLog.Append(pszEventValue);
    }

    IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
    if (piService != IMS_NULL)
    {
        piService->NotifyTrace(eRegType, strLog);
    }
}

PUBLIC
void AosTracer::TraceEvent(IN IAosAppContext* piContext, IN AosDomain eDomain,
        IN AosSeverity eSeverity, IN const IMS_CHAR* pszPrefix, IN const IMS_CHAR* pszEventName,
        IN const IMS_CHAR* pszEventValue)
{
    AString strCompositeName;

    if (pszPrefix != IMS_NULL && pszEventName != IMS_NULL)
    {
        strCompositeName.Sprintf("%s%s", pszPrefix, pszEventName);
    }
    else if (pszPrefix != IMS_NULL)
    {
        strCompositeName = pszPrefix;
    }
    else if (pszEventName != IMS_NULL)
    {
        strCompositeName = pszEventName;
    }
    else
    {
        strCompositeName = "UnknownEvent";
    }

    TraceEvent(piContext, eDomain, eSeverity, strCompositeName.GetStr(), pszEventValue);
}

PROTECTED
IMS_BOOL AosTracer::IsTraceTarget(
        IN IAosAppContext* piContext, OUT AosRegistrationType& eRegType) const
{
    if (piContext == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IAosRegistration* piRegistration = piContext->GetRegistration();
    if (piRegistration == IMS_NULL)
    {
        return IMS_FALSE;
    }

    eRegType = piRegistration->GetRegType();
    if (eRegType == AosRegistrationType::FAKE || eRegType == AosRegistrationType::RCS)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED
AString AosTracer::FormatCriticalKey(IN const AosStatusInfo& objStatus) const
{
    AString strKey;
    strKey.Sprintf("%d:%d:%d:%d:%d:%d:%d", objStatus.m_nAppState, objStatus.m_nRegResult,
            objStatus.m_nRegReason, objStatus.m_nConnState, objStatus.m_nConnReason,
            objStatus.m_nOffReason, objStatus.m_nDataFailureReason);
    return strKey;
}

PROTECTED const IMS_CHAR* AosTracer::DomainToString(IN AosDomain eDomain)
{
    switch (eDomain)
    {
        case AosDomain::APP:
            return "APP";
        case AosDomain::COND:
            return "COND";
        case AosDomain::CONN:
            return "CONN";
        case AosDomain::HDL:
            return "HDL";
        case AosDomain::REG:
            return "REG";
        case AosDomain::RSUB:
            return "RSUB";
        case AosDomain::SUBS:
            return "SUBS";
        case AosDomain::MISC:
            return "MISC";
        default:
            return "INVALID";
    }
}

PROTECTED const IMS_CHAR* AosTracer::SeverityToString(IN AosSeverity eSeverity)
{
    switch (eSeverity)
    {
        case AosSeverity::I:
            return "I";
        case AosSeverity::W:
            return "W";
        case AosSeverity::E:
            return "E";
        default:
            return "U";
    }
}
