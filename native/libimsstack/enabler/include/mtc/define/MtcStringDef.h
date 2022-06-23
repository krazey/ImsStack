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

#ifndef MTC_STRING_DEF_H_
#define MTC_STRING_DEF_H_

#include "CallReasonInfo.h"
#include "MtcDef.h"
#include "ImsStrLib.h"

class MtcDefPs
{
public:
    inline static const IMS_CHAR* PS_BOOL(IN IMS_BOOL /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_ServiceType(IN IMS_UINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_SessionType(IN IMS_SINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_SideType(IN IMS_SINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_PeerType(IN IMS_SINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_OAType(IN IMS_SINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_MSGType(IN IMS_SINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_UpdateType(IN IMS_SINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_HRType(IN IMS_SINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_CCType(IN IMS_SINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_DTMFType(IN IMS_SINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_FeatureType(IN IMS_SINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_PEMType(IN IMS_SINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_MediaType(IN IMS_SINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_MediaInfoType(IN IMS_SINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_NegoState(IN IMS_SINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_FormResult(IN IMS_UINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_NegoResult(IN IMS_UINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_Direction(IN IMS_SINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_VQuality(IN IMS_UINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_AQuality(IN IMS_UINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_GTTMode(IN IMS_SINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_SuppType(IN IMS_UINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_InfoType(IN IMS_UINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_CWType(IN IMS_UINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_CNAPScheme(IN IMS_UINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_CallReason(IN IMS_SINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_RejectReason(IN IMS_SINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_BlockFeature(IN IMS_UINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_SRVCCStatus(IN IMS_UINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_VoLTECallState(IN IMS_UINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_SessionTimer(IN IMS_UINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_QoSType(IN IMS_SINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_QoSStatus(IN IMS_SINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_QoSDir(IN IMS_UINT32 /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_FR(IN CallReasonInfo /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_RR(IN CallReasonInfo /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_LFR(IN CallReasonInfo /*bValue*/) { return ""; }
    inline static const IMS_CHAR* PS_MT(IN MediaThreshold /*bValue*/) { return ""; }
};

#ifndef PS_BOOL
#define PS_BOOL(A) MtcDefPs::PS_BOOL(A)
#endif

#ifndef PS_ServiceType
#define PS_ServiceType(A) "SomeServiceType" /* TODO: MTC BUILD MtcDefPs::PS_ServiceType(A) */
#endif

#ifndef PS_SessionType
#define PS_SessionType(A) MtcDefPs::PS_SessionType(A)
#endif

#ifndef PS_SideType
#define PS_SideType(A) MtcDefPs::PS_SideType(A)
#endif

#ifndef PS_PeerType
#define PS_PeerType(A) MtcDefPs::PS_PeerType(A)
#endif

#ifndef PS_OAType
#define PS_OAType(A) MtcDefPs::PS_OAType(A)
#endif

#ifndef PS_MSGType
#define PS_MSGType(A) MtcDefPs::PS_MSGType(A)
#endif

#ifndef PS_UpdateType
#define PS_UpdateType(A) MtcDefPs::PS_UpdateType(A)
#endif

#ifndef PS_HRType
#define PS_HRType(A) MtcDefPs::PS_HRType(A)
#endif

#ifndef PS_CCType
#define PS_CCType(A) MtcDefPs::PS_CCType(A)
#endif

#ifndef PS_DTMFType
#define PS_DTMFType(A) MtcDefPs::PS_DTMFType(A)
#endif

#ifndef PS_FeatureType
#define PS_FeatureType(A) MtcDefPs::PS_FeatureType(A)
#endif

#ifndef PS_PEMType
#define PS_PEMType(A) MtcDefPs::PS_PEMType(A)
#endif

#ifndef PS_MediaType
#define PS_MediaType(A) MtcDefPs::PS_MediaType(A)
#endif

#ifndef PS_MediaInfoType
#define PS_MediaInfoType(A) MtcDefPs::PS_MediaInfoType(A)
#endif

#ifndef PS_NegoState
#define PS_NegoState(A) MtcDefPs::PS_NegoState(A)
#endif

#ifndef PS_FormResult
#define PS_FormResult(A) MtcDefPs::PS_FormResult(A)
#endif

#ifndef PS_NegoResult
#define PS_NegoResult(A) MtcDefPs::PS_NegoResult(A)
#endif

#ifndef PS_Direction
#define PS_Direction(A) MtcDefPs::PS_Direction(A)
#endif

#ifndef PS_VQuality
#define PS_VQuality(A) MtcDefPs::PS_VQuality(A)
#endif

#ifndef PS_AQuality
#define PS_AQuality(A) MtcDefPs::PS_AQuality(A)
#endif

#ifndef PS_GTTMode
#define PS_GTTMode(A) MtcDefPs::PS_GTTMode(A)
#endif

#ifndef PS_SuppType
#define PS_SuppType(A) MtcDefPs::PS_SuppType(A)
#endif

#ifndef PS_InfoType
#define PS_InfoType(A) MtcDefPs::PS_InfoType(A)
#endif

#ifndef PS_CWType
#define PS_CWType(A) MtcDefPs::PS_CWType(A)
#endif
#ifndef PS_CNAPScheme
#define PS_CNAPScheme(A) MtcDefPs::PS_CNAPScheme(A)
#endif

#ifndef PS_CallReason
#define PS_CallReason(A) MtcDefPs::PS_CallReason(A)
#endif

#ifndef PS_RejectReason
#define PS_RejectReason(A) MtcDefPs::PS_RejectReason(A)
#endif

#ifndef PS_BlockFeature
#define PS_BlockFeature(A) MtcDefPs::PS_BlockFeature(A)
#endif

#ifndef PS_SRVCCStatus
#define PS_SRVCCStatus(A) MtcDefPs::PS_SRVCCStatus(A)
#endif

#ifndef PS_VoLTECallState
#define PS_VoLTECallState(A) MtcDefPs::PS_VoLTECallState(A)
#endif

#ifndef PS_SessionTimer
#define PS_SessionTimer(A) MtcDefPs::PS_SessionTimer(A)
#endif

#ifndef PS_FR
#define PS_FR(A) MtcDefPs::PS_FR(A)
#endif

#ifndef PS_RR
#define PS_RR(A) MtcDefPs::PS_RR(A)
#endif

#ifndef PS_LFR
#define PS_LFR(A) MtcDefPs::PS_LFR(A)
#endif

#ifndef PS_MT
#define PS_MT(A) MtcDefPs::PS_MT(A)
#endif

#ifndef PS_QoSType
#define PS_QoSType(A) MtcDefPs::PS_QoSType(A)
#endif

#ifndef PS_QoSStatus
#define PS_QoSStatus(A) MtcDefPs::PS_QoSStatus(A)
#endif

#ifndef PS_QoSDir
#define PS_QoSDir(A) MtcDefPs::PS_QoSDir(A)
#endif

#endif
