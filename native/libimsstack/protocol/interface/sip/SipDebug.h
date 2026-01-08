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
#ifndef __SIP_DEBUG_H__
#define __SIP_DEBUG_H__

#include "SipDatatypes.h"

#define SIP_DEBUG_ENABLE
#define DEBUG_MSG_MAX_SIZE 500
#define SIP_TRACE_MAX_SIZE 2000

#ifdef SIP_DEBUG_ENABLE
#define SIP_DEBUG_WARNING(a, b, c, d)  SIP_DEBUG_LOG(CAT_D, __FILE__, __LINE__, b, c, d)
#define SIP_DEBUG_EXTRLBUG(a, b, c, d) SIP_DEBUG_LOG(CAT_D, __FILE__, __LINE__, b, c, d)
#define SIP_DEBUG_STACKBUG(a, b, c, d) SIP_DEBUG_LOG(CAT_D, __FILE__, __LINE__, b, c, d)
// Added New INFO log capturing macro function
#define SIP_TRACE_I(a, b, c)           SIP_DEBUG_LOG(CAT_I, __FILE__, __LINE__, a, b, c)
// Added New ERROR log capturing macro function
#define SIP_TRACE_E(a, b, c)           SIP_DEBUG_LOG(CAT_E, __FILE__, __LINE__, a, b, c)
// Added New DEBUG og capturing macro function
#define SIP_TRACE_D(a, b, c)           SIP_DEBUG_LOG(CAT_D, __FILE__, __LINE__, a, b, c)

#if defined(__arm)

#if defined(__clang__)
#define __IMS_FILE__ __FILE__
#else
#define __IMS_FILE__ __MODULE__
#endif
#define __IMS_LINE__ __LINE__
#define __IMS_FUNC__ __func__

#elif defined(__LINUX__)

#define __IMS_FILE__ __FILE__
#define __IMS_LINE__ __LINE__
#define __IMS_FUNC__ __func__

#else

#define __IMS_FILE__ "N/A"
#define __IMS_LINE__ 0
#define __IMS_FUNC__ "N/A"

#endif

#define SIP_ASSERT(CONDITION)                                       \
    do                                                              \
    {                                                               \
        if (!(CONDITION))                                           \
            SIP_ASSERT_LOG(#CONDITION, __IMS_FUNC__, __IMS_LINE__); \
    } while (0)
#else
#define SIP_DEBUG_WARNING(a, b, c, d)
#define SIP_DEBUG_EXTRLBUG(a, b, c, d)
#define SIP_DEBUG_STACKBUG(a, b, c, d)
#define SIP_TRACE_I(a, b, c)
#define SIP_TRACE_E(a, b, c)
#define SIP_TRACE_D(a, b, c)
#define SIP_ASSERT(CONDITION)
#endif

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

typedef enum _SipEn_DebugTypes
{
    ESIPDEBUG_WARNING = SIP_ZERO, /*when set logs expected error
                  or stack limitations
                  eg:invalid header input*/
    ESIPDEBUG_EXTERNALRSRC,       /*logs memory,transport,network,timer,
                         os related     errors*/
    ESIPDEBUG_STACKBUG,           /*logs bugs in stack    */
    ESIPDEBUG_END,
    ESIPDEBUG_INVALID = SIP_INVALID
} SipEn_DebugTypes;

SIP_VOID SIP_DEBUG_LOG(SIP_UINT32 nCategory, const SIP_CHAR* pszFilename, SIP_INT32 nLine,
        const SIP_CHAR* pszFormat, ...);
SIP_VOID SIP_ASSERT_LOG(const SIP_CHAR* pszCondition, const SIP_CHAR* pszModule, SIP_UINT16 nLine);

#endif  //__SIP_DEBUG_H__
