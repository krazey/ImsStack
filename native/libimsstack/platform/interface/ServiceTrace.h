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
#ifndef SERVICE_TRACE_H_
#define SERVICE_TRACE_H_

#include "ITrace.h"
#include "PlatformService.h"

class TraceServicePrivate;

class TraceService : public PlatformService
{
public:
    TraceService();
    TraceService(IN const TraceService&) = delete;
    TraceService& operator=(IN const TraceService&) = delete;

protected:
    virtual ~TraceService();

public:
    const IMS_CHAR* GetFileName(IN const IMS_CHAR* pszFileName) const;
    const IMS_CHAR* GetFileName(
            IN_OUT IMS_CHAR* pszOutFileName, IN const IMS_CHAR* pszFileName) const;
    const ImsTraceTag& GetTraceTag(IN IMS_SINT32 nTag) const;
    IMS_UINT32 GetOption() const;
    void SetOption(IN IMS_UINT32 nOption, IN IMS_UINT32 nModule);
    void PrintPrivacyLog(IN IMS_SINT32 nCategory, IN const IMS_CHAR* pszTag, IN IMS_UINT32 nModule,
            IN const IMS_CHAR* pszFormat, IN const IMS_CHAR* pszFileName, IN IMS_UINT32 nLine,
            IN const IMS_CHAR* pszArg1, IN const IMS_CHAR* pszArg2, IN const IMS_CHAR* pszArg3);

    virtual ITrace* GetTrace();

    static TraceService* GetTraceService();

    inline static IMS_BOOL IsLoggableForDebug() { return s_nLoggableForDebug == 1; }

private:
    TraceServicePrivate* m_pPrivate;
    ImsTraceTag* m_objTraceTag[IMS_TRACE_TAG_MAX + 1];

    static IMS_SINT32 s_nLoggableForDebug;
};

////
// MACRO DEFINITION FOR TRACE
////

// Macro based on platform's log configuration
#define _IMS_LOG_DEBUG_      TraceService::IsLoggableForDebug()

// Marco for boolean
#define _TRACE_B_(B)         (((B) == IMS_TRUE) ? "true" : "false")
// Macro for null-terminated string
#define _TRACE_S_(S)         (((S) != IMS_NULL) ? (S) : "__NULL__")

#define IMS_TRACE_OPT_NONE   0x0000
#define IMS_TRACE_OPT_D      0x0001
#define IMS_TRACE_OPT_E      0x0002
#define IMS_TRACE_OPT_I      0x0004
#define IMS_TRACE_OPT_TEXT   0x0008
#define IMS_TRACE_OPT_MEM    0x0010

// Additional options
#define IMS_TRACE_OPT_TIME   0x0100
#define IMS_TRACE_OPT_FILE   0x0200

//// Macro for unknown tag trace statement
#define IMS_TRACE(VA_FORMAT) TraceService::GetTraceService()->GetTrace()->Out VA_FORMAT

#define IMS_TRACE_D(FORMAT, A1, A2, A3)                                                      \
    do                                                                                       \
    {                                                                                        \
        TraceService* pTs = TraceService::GetTraceService();                                 \
        if (pTs->GetTrace()->IsTraceEnabled(ITrace::CAT_D, __IMS_TRACE_MODULE__))            \
        {                                                                                    \
            char acFileNameForTrace__[128] = {                                               \
                    0,                                                                       \
            };                                                                               \
            pTs->GetTrace()->Out(ITrace::CAT_D, __IMS_TRACE_NAME__, __IMS_TRACE_MODULE__,    \
                    "[%s:%d] " FORMAT, pTs->GetFileName(acFileNameForTrace__, __IMS_FILE__), \
                    __IMS_LINE__, A1, A2, A3);                                               \
        }                                                                                    \
    } while (0)

#define A_IMS_TRACE_D(ID, FORMAT, A1, A2, A3)                                                     \
    do                                                                                            \
    {                                                                                             \
        TraceService* pTs = TraceService::GetTraceService();                                      \
        if (pTs->GetTrace()->IsTraceEnabled(ITrace::CAT_D, __IMS_TRACE_MODULE__))                 \
        {                                                                                         \
            char acFileNameForTrace__[128] = {                                                    \
                    0,                                                                            \
            };                                                                                    \
            pTs->GetTrace()->Out(ITrace::CAT_D, __IMS_TRACE_NAME__, __IMS_TRACE_MODULE__,         \
                    "[%s:%d] [%s] " FORMAT, pTs->GetFileName(acFileNameForTrace__, __IMS_FILE__), \
                    __IMS_LINE__, ID, A1, A2, A3);                                                \
        }                                                                                         \
    } while (0)

#define U_IMS_TRACE_D(ID, FORMAT, A1, A2, A3)                                                     \
    do                                                                                            \
    {                                                                                             \
        TraceService* pTs = TraceService::GetTraceService();                                      \
        if (pTs->GetTrace()->IsTraceEnabled(ITrace::CAT_D, __IMS_TRACE_MODULE__))                 \
        {                                                                                         \
            char acFileNameForTrace__[128] = {                                                    \
                    0,                                                                            \
            };                                                                                    \
            pTs->GetTrace()->Out(ITrace::CAT_D, __IMS_TRACE_NAME__, __IMS_TRACE_MODULE__,         \
                    "[%s:%d] [%s] " FORMAT, pTs->GetFileName(acFileNameForTrace__, __IMS_FILE__), \
                    __IMS_LINE__, ID, A1, A2, A3);                                                \
        }                                                                                         \
    } while (0)

#define IMS_TRACE_DV(VA_FORMAT) TraceService::GetTraceService()->GetTrace()->Out VA_FORMAT

#define IMS_TRACE_I(FORMAT, A1, A2, A3)                                                      \
    do                                                                                       \
    {                                                                                        \
        TraceService* pTs = TraceService::GetTraceService();                                 \
        if (pTs->GetTrace()->IsTraceEnabled(ITrace::CAT_I, __IMS_TRACE_MODULE__))            \
        {                                                                                    \
            char acFileNameForTrace__[128] = {                                               \
                    0,                                                                       \
            };                                                                               \
            pTs->GetTrace()->Out(ITrace::CAT_I, __IMS_TRACE_NAME__, __IMS_TRACE_MODULE__,    \
                    "[%s:%d] " FORMAT, pTs->GetFileName(acFileNameForTrace__, __IMS_FILE__), \
                    __IMS_LINE__, A1, A2, A3);                                               \
        }                                                                                    \
    } while (0)

#define A_IMS_TRACE_I(ID, FORMAT, A1, A2, A3)                                                     \
    do                                                                                            \
    {                                                                                             \
        TraceService* pTs = TraceService::GetTraceService();                                      \
        if (pTs->GetTrace()->IsTraceEnabled(ITrace::CAT_I, __IMS_TRACE_MODULE__))                 \
        {                                                                                         \
            char acFileNameForTrace__[128] = {                                                    \
                    0,                                                                            \
            };                                                                                    \
            pTs->GetTrace()->Out(ITrace::CAT_I, __IMS_TRACE_NAME__, __IMS_TRACE_MODULE__,         \
                    "[%s:%d] [%s] " FORMAT, pTs->GetFileName(acFileNameForTrace__, __IMS_FILE__), \
                    __IMS_LINE__, ID, A1, A2, A3);                                                \
        }                                                                                         \
    } while (0)

#define U_IMS_TRACE_I(ID, FORMAT, A1, A2, A3)                                                     \
    do                                                                                            \
    {                                                                                             \
        TraceService* pTs = TraceService::GetTraceService();                                      \
        if (pTs->GetTrace()->IsTraceEnabled(ITrace::CAT_I, __IMS_TRACE_MODULE__))                 \
        {                                                                                         \
            char acFileNameForTrace__[128] = {                                                    \
                    0,                                                                            \
            };                                                                                    \
            pTs->GetTrace()->Out(ITrace::CAT_I, __IMS_TRACE_NAME__, __IMS_TRACE_MODULE__,         \
                    "[%s:%d] [%s] " FORMAT, pTs->GetFileName(acFileNameForTrace__, __IMS_FILE__), \
                    __IMS_LINE__, ID, A1, A2, A3);                                                \
        }                                                                                         \
    } while (0)

#define IMS_TRACE_IV(VA_FORMAT) TraceService::GetTraceService()->GetTrace()->Out VA_FORMAT

#define IMS_TRACE_E(ECODE, FORMAT, A1, A2, A3)                                                  \
    do                                                                                          \
    {                                                                                           \
        TraceService* pTs = TraceService::GetTraceService();                                    \
        if (pTs->GetTrace()->IsTraceEnabled(ITrace::CAT_E, __IMS_TRACE_MODULE__))               \
        {                                                                                       \
            char acFileNameForTrace__[128] = {                                                  \
                    0,                                                                          \
            };                                                                                  \
            pTs->GetTrace()->OutE(ECODE, __IMS_FUNC__, __IMS_LINE__, __IMS_TRACE_NAME__,        \
                    __IMS_TRACE_MODULE__, "[%s:%d] " FORMAT,                                    \
                    pTs->GetFileName(acFileNameForTrace__, __IMS_FILE__), __IMS_LINE__, A1, A2, \
                    A3);                                                                        \
        }                                                                                       \
    } while (0)

#define A_IMS_TRACE_E(ECODE, ID, FORMAT, A1, A2, A3)                                            \
    do                                                                                          \
    {                                                                                           \
        TraceService* pTs = TraceService::GetTraceService();                                    \
        if (pTs->GetTrace()->IsTraceEnabled(ITrace::CAT_E, __IMS_TRACE_MODULE__))               \
        {                                                                                       \
            char acFileNameForTrace__[128] = {                                                  \
                    0,                                                                          \
            };                                                                                  \
            pTs->GetTrace()->OutE(ECODE, __IMS_FUNC__, __IMS_LINE__, __IMS_TRACE_NAME__,        \
                    __IMS_TRACE_MODULE__, "[%s:%4d] [%s] " FORMAT,                              \
                    pTs->GetFileName(acFileNameForTrace__, __IMS_FILE__), __IMS_LINE__, ID, A1, \
                    A2, A3);                                                                    \
        }                                                                                       \
    } while (0)

#define U_IMS_TRACE_E(ECODE, ID, FORMAT, A1, A2, A3)                                            \
    do                                                                                          \
    {                                                                                           \
        TraceService* pTs = TraceService::GetTraceService();                                    \
        if (pTs->GetTrace()->IsTraceEnabled(ITrace::CAT_E, __IMS_TRACE_MODULE__))               \
        {                                                                                       \
            char acFileNameForTrace__[128] = {                                                  \
                    0,                                                                          \
            };                                                                                  \
            pTs->GetTrace()->OutE(ECODE, __IMS_FUNC__, __IMS_LINE__, __IMS_TRACE_NAME__,        \
                    __IMS_TRACE_MODULE__, "[%s:%4d] [%s] " FORMAT,                              \
                    pTs->GetFileName(acFileNameForTrace__, __IMS_FILE__), __IMS_LINE__, ID, A1, \
                    A2, A3);                                                                    \
        }                                                                                       \
    } while (0)

#if ((__IMS_TRACE_MEM__ & IMS_TRACE_OPT_MEM) == IMS_TRACE_OPT_MEM)

#define IMS_TRACE_MEM(TAG, FORMAT, A1, A2, A3)                                                  \
    do                                                                                          \
    {                                                                                           \
        TraceService* pTs = TraceService::GetTraceService();                                    \
        if (pTs->GetTrace()->IsTraceEnabled(ITrace::CAT_D, IMS_TRACE_MODULE_IMS))               \
        {                                                                                       \
            char acFileNameForTrace__[128] = {                                                  \
                    0,                                                                          \
            };                                                                                  \
            pTs->GetTrace()->Out(ITrace::CAT_D, TAG, IMS_TRACE_MODULE_IMS, "[%s:%d] " FORMAT,   \
                    pTs->GetFileName(acFileNameForTrace__, __IMS_FILE__), __IMS_LINE__, A1, A2, \
                    A3);                                                                        \
        }                                                                                       \
    } while (0)

#else

#define IMS_TRACE_MEM(TAG, FORMAT, A1, A2, A3)

#endif

#define IMS_TRACE_EV(VA_FORMAT) TraceService::GetTraceService()->GetTrace()->OutE VA_FORMAT

#define IMS_TRACE_SIP(DESC, TEXT, SIZE, BODY)                                                    \
    TraceService::GetTraceService()->GetTrace()->OutText(IMS_TRACE_MODULE_SIP, ITrace::TEXT_SIP, \
            DESC, reinterpret_cast<const IMS_CHAR*>(TEXT), SIZE, BODY)

#define IMS_TRACE_SDP(DESC, TEXT, SIZE)                                                          \
    TraceService::GetTraceService()->GetTrace()->OutText(IMS_TRACE_MODULE_SDP, ITrace::TEXT_SDP, \
            DESC, reinterpret_cast<const IMS_CHAR*>(TEXT), SIZE)

#define IMS_TRACE_XML(DESC, TEXT, SIZE)                                            \
    TraceService::GetTraceService()->GetTrace()->OutText(IMS_TRACE_MODULE_DEFAULT, \
            ITrace::TEXT_XML, DESC, reinterpret_cast<const IMS_CHAR*>(TEXT), SIZE)

#define IMS_TRACE_TEXT(DESC, TEXT, SIZE)                                           \
    TraceService::GetTraceService()->GetTrace()->OutText(IMS_TRACE_MODULE_DEFAULT, \
            ITrace::TEXT_ANY, DESC, reinterpret_cast<const IMS_CHAR*>(TEXT), SIZE)

//// END OF MACRO DEFINITION FOR TRACE
#endif
