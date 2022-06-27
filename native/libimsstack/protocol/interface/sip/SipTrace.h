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
#ifndef __SIP_TRACE_H__
#define __SIP_TRACE_H__

#include "sip_pf_datatypes.h"

#define SIP_TRACE_MAX_SIZE 2000

#ifdef SIP_TRACE_ENABLE
#define SIP_TRACE_MESSAGE(a, b, c, d) \
    SIP_TRACE_LOG(CAT_I, (SIP_CHAR*)__FILE__, __LINE__, (SIP_CHAR*)(b), (c), (d))
#define SIP_TRACE_NORMAL(a, b, c, d) \
    SIP_TRACE_LOG(CAT_D, (SIP_CHAR*)__FILE__, __LINE__, (SIP_CHAR*)(b), (c), (d))
#else
#define SIP_TRACE_MESSAGE(a, b, c, d)
#define SIP_TRACE_NORMAL(a, b, c, d)

#endif

typedef enum _SipEn_TraceTypes
{
    /*this enum when set tracks all messages     encoded and decoded*/
    ESIPTRACE_TYPEMESSAGE = (1 << 0),
    /*when set tracks data flow.eg:list of headers     parsed,message length etc*/
    ESIPTRACE_TYPENORMAL = (1 << 1),
    ESIPTRACE_TYPEALL = 0x0F, /* Set to print all the trace types */
    ESIPTRACE_TYPEEND,
    ESIPTRACE_TYPENVALID = SIP_INVALID
} SipEn_TraceTypes;

typedef enum _SipEn_TraceModules
{
    ESIPTRACE_MODFWK = 0,
    ESIPTRACE_MODTXN,
    ESIPTRACE_MODTRANSP,
    ESIPTRACE_MODENCODER,
    ESIPTRACE_MODDECODER,
    ESIPTRACE_MODACCESSOR,
    ESIPTRACE_MODABNF,
    ESIPTRACE_MODTIMER,
    ESIPTRACE_MODHASH,
    ESIPTRACE_MODLIST,
    ESIPTRACE_MODMEMORY,
    ESIPTRACE_MODSTRING,
    ESIPTRACE_MODALL,
    ESIPTRACE_MODEND,
    ESIPTRACE_MODINVALID = SIP_INVALID
} SipEn_TraceModules;

/******************************************************************************
 * Function name    : SIP_TRACE_LOG
 * Description    : This function logs the trace in case module is set for tracing.
 *                :
 *
 * Return type    : SIP_BOOL

 SIP_TRUE if logging of trace is successful
 *
 * Argument      :
 *    [IN]        : eModule[IN] : Module traced
 eTraceType[IN] - Trace type
 *    [IN]        : pszFilename[IN] - File traced.
 iLine[IN]    - Line number in file traced
 pcFormat[IN] - Format String
 *
 * Side Effects    :
 * NOTE             :
 ******************************************************************************/
void SIP_TRACE_LOG(
        SIP_UINT32 nCategory, SIP_CHAR* pszFilename, SIP_INT32 nLine, SIP_CHAR* pszFormat, ...);

class SipTrace
{
public:
    SipTrace();
    virtual ~SipTrace();

    /* Enable Trace type for particular module */
    SIP_BOOL EnableTrace(SipEn_TraceModules eModule, SIP_UINT32 nTraceType);

    /* Enable Trace type for all module */
    SIP_BOOL EnableTrace(SIP_UINT32 nTraceType);

    /* disable Trace type for particular module */
    SIP_BOOL DisableTrace(SipEn_TraceModules eModule, SIP_UINT32 nTraceType);

    /* disable Trace type for all module */
    SIP_BOOL DisableTrace(SIP_UINT32 nTraceType);

    SIP_BOOL IsTraceEnable(SipEn_TraceModules eModule, SipEn_TraceTypes eTraceType);

private:
    SIP_INT16 m_ausModTrace[ESIPTRACE_MODEND];
};

void SipTrace_Construct();
void SipTrace_Destruct();
SipTrace* SipTrace_GetInstance();

#endif  //__SIP_TRACE_H__
