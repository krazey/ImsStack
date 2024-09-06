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
#ifndef INTERFACE_TRACE_TAG_H_
#define INTERFACE_TRACE_TAG_H_

#include "ImsTypeDef.h"

#define IMS_LOG_TAG                "ImsStackN"

// DEFAULT TRACE TAG
#define __IMS_TRACE_DEFAULT_NAME__ "XXX"

// MAIN macro definition for TRACE module
#define __IMS_TRACE_TAG__(TAG) \
    static const ImsTraceTag& s_objTag = TraceService_GetTraceTag(IMS_TRACE_TAG_##TAG);

#define __IMS_TRACE_NAME__   (s_objTag.pszName)
#define __IMS_TRACE_MODULE__ (s_objTag.nModule)

// User-defined tag
#define __IMS_TRACE_TAG_USER_DECL__(TAG) \
    static const ImsTraceTag s_objTag = {TAG, IMS_TRACE_MODULE_DEFAULT};

////
// TRACE TAG ENUMERATION DEFINITION FOR IMS CLIENT PLATFORM
////
#define __IMS_TRACE_TAG_DEF__(ENUM, NAME, MODULE) IMS_TRACE_TAG_##ENUM,

typedef enum _IMS_TRACE_TAG_ENTYPE_
{
    IMS_TRACE_TAG_DEFAULT = 0,

#include "ITraceTagDef.h"

    IMS_TRACE_TAG_MAX
} IMS_TRACE_TAG_ENTYPE;

#undef __IMS_TRACE_TAG_DEF__

////
// TRACE MODULE ENUMERATION DEFINITION FOR IMS CLIENT PLATFORM
////
typedef enum _IMS_TRACE_MODULE_ENTYPE_
{
    IMS_TRACE_MODULE_NONE = 0,  // no modules : no trace enabled

    IMS_TRACE_MODULE_IMS = 0x00000001,   // IMS
    IMS_TRACE_MODULE_REG = 0x00000002,   // REG
    IMS_TRACE_MODULE_CORE = 0x00000004,  // CORE
    IMS_TRACE_MODULE_BASE = 0x00000008,  // BASE, ADAPT, CONF
    IMS_TRACE_MODULE_SIP = 0x00000010,   // SIP, SIGCOMP
    IMS_TRACE_MODULE_XML = 0x00000020,   // XML
    IMS_TRACE_MODULE_SDP = 0x00000040,   // SDP
    IMS_TRACE_MODULE_FWK = 0x00000100,   // FWK
    IMS_TRACE_MODULE_AOS = 0x00000200,   // AOS

    IMS_TRACE_MODULE_ENABLER_MTC = 0x00010000,           // ENABLER: MTC
    IMS_TRACE_MODULE_ENABLER_MTS = 0x00020000,           // ENABLER: MTS
    IMS_TRACE_MODULE_ENABLER_UCE = 0x00040000,           // ENABLER: UCE
    IMS_TRACE_MODULE_ENABLER_SIP_DELEGATE = 0x00080000,  // ENABLER: SIP_DELEGATE
    IMS_TRACE_MODULE_ENABLER_MEDIA = 0x00100000,         // ENABLER: MEDIA

    IMS_TRACE_MODULE_DEFAULT = 0x80000000,  // Undefined module
    IMS_TRACE_MODULE_ALL = 0xFFFFFFFF       // All modules : all trace enabled
} IMS_TRACE_MODULE_ENTYPE;

////
// STRUCTURE FOR TRACE TAG & FILTER
////
typedef struct _ImsTraceTag_
{
    const IMS_CHAR* pszName;
    IMS_UINT32 nModule;
} ImsTraceTag;

// Function prototype to get the trace tag
extern const ImsTraceTag& TraceService_GetTraceTag(IN IMS_SINT32 nTag);

#include "ITraceTagDecl.h"

#endif
