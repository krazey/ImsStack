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
#include "ImsTrace.h"
#include "PlatformContext.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#define __IMS_TRACE_TAG_DEF__(ENUM, NAME, MODULE) {NAME, IMS_TRACE_MODULE_##MODULE},

static const ImsTraceTag TRACE_TAGS[IMS_TRACE_TAG_MAX + 1] = {
        {__IMS_TRACE_DEFAULT_NAME__, IMS_TRACE_MODULE_DEFAULT},

#include "ITraceTagDef.h"

        {__IMS_TRACE_DEFAULT_NAME__, IMS_TRACE_MODULE_DEFAULT}
};

#undef __IMS_TRACE_TAG_DEF__

class TraceServicePrivate
{
public:
    inline TraceServicePrivate() :
            m_pTrace(IMS_NULL)
    {
    }
    inline ~TraceServicePrivate()
    {
        if (m_pTrace != IMS_NULL)
        {
            delete m_pTrace;
        }
    }

    TraceServicePrivate(IN const TraceServicePrivate&) = delete;
    TraceServicePrivate& operator=(IN const TraceServicePrivate&) = delete;

public:
    inline ImsTrace* GetTrace()
    {
        if (m_pTrace == IMS_NULL)
        {
            IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
            m_pTrace = piOsFactory->CreateTrace();
        }

        return m_pTrace;
    }

private:
    ImsTrace* m_pTrace;
};

PRIVATE GLOBAL IMS_SINT32 TraceService::s_nLoggableForDebug = -1;

PRIVATE
TraceService::TraceService() :
        m_pPrivate(new TraceServicePrivate())
{
    for (IMS_SINT32 i = 0; i < IMS_TRACE_TAG_MAX + 1; ++i)
    {
        m_objTraceTag[i] = const_cast<ImsTraceTag*>(&TRACE_TAGS[i]);
    }

    s_nLoggableForDebug = ImsTrace::IsLoggable(ITrace::CAT_D);
}

PRIVATE
TraceService::~TraceService()
{
    if (m_pPrivate != IMS_NULL)
    {
        delete m_pPrivate;
    }
}

PUBLIC
const ImsTraceTag& TraceService::GetTraceTag(IN IMS_SINT32 nTag) const
{
    if ((nTag < 0) || (nTag > IMS_TRACE_TAG_MAX))
    {
        return (*m_objTraceTag[IMS_TRACE_TAG_MAX]);
    }

    return (*m_objTraceTag[nTag]);
}

PUBLIC
IMS_UINT32 TraceService::GetOption() const
{
    const ImsTrace* pTrace = m_pPrivate->GetTrace();

    if (pTrace == IMS_NULL)
    {
        return 0;
    }

    return pTrace->GetOption();
}

PUBLIC
void TraceService::SetOption(IN IMS_UINT32 nOption, IN IMS_UINT32 nModule)
{
    ImsTrace* pTrace = m_pPrivate->GetTrace();

    if (pTrace == IMS_NULL)
    {
        return;
    }

    pTrace->SetOption(nOption, nModule);
}

PUBLIC
void TraceService::PrintPrivacyLog(IN IMS_SINT32 nCategory, IN const IMS_CHAR* pszTag,
        IN IMS_UINT32 nModule, IN const IMS_CHAR* pszFormat, IN const IMS_CHAR* pszFileName,
        IN IMS_UINT32 nLine, IN const IMS_CHAR* pszArg1, IN const IMS_CHAR* pszArg2,
        IN const IMS_CHAR* pszArg3)
{
    ImsTrace* pTrace = m_pPrivate->GetTrace();

    if (pTrace == IMS_NULL)
    {
        return;
    }

    IMS_CHAR _buffer_A1[512 + 7] = {
            0,
    };
    IMS_CHAR _buffer_A2[512 + 7] = {
            0,
    };
    IMS_CHAR _buffer_A3[512 + 7] = {
            0,
    };
    pTrace->OutP(nCategory, pszTag, nModule, pszFormat, pszFileName, nLine,
            ImsTrace::EncryptPrivacyLog(_buffer_A1, pszArg1),
            ImsTrace::EncryptPrivacyLog(_buffer_A2, pszArg2),
            ImsTrace::EncryptPrivacyLog(_buffer_A3, pszArg3));
}

PUBLIC
ITrace* TraceService::GetTrace()
{
    return m_pPrivate->GetTrace();
}

PUBLIC GLOBAL TraceService* TraceService::GetTraceService()
{
    return DYNAMIC_CAST(TraceService*,
            PlatformContext::GetInstance()->GetService(PlatformContext::SERVICE_TRACE));
}

GLOBAL
void TraceService_Assert(
        IN const IMS_CHAR* pszCondition, IN const IMS_CHAR* pszModule, IN IMS_UINT32 nLine)
{
    TraceService::GetTraceService()->GetTrace()->Out(ITrace::CAT_E, "ASSERT",
            IMS_TRACE_MODULE_DEFAULT, pszModule, nLine, "(%s) FAILED", pszCondition);
}

GLOBAL
const ImsTraceTag& TraceService_GetTraceTag(IN IMS_SINT32 nTag)
{
    return TraceService::GetTraceService()->GetTraceTag(nTag);
}
