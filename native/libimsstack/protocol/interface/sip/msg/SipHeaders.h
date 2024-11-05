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
#ifndef __SIP_HEADERS_H__
#define __SIP_HEADERS_H__

#include "SipConfiguration.h"
#include "SipDatatypes.h"
#include "SipMap.h"
#include "SipRefBase.h"
#include "msg/SipAuthBase.h"
#include "msg/SipAuthInfoHeader.h"
#include "msg/SipContentTypeHeader.h"
#include "msg/SipCSeqHeader.h"
#include "msg/SipDateHeader.h"
#include "msg/SipEventHeader.h"
#include "msg/SipGeolocationRoutingHeader.h"
#include "msg/SipHeaderBase.h"
#include "msg/SipHeaderList.h"
#include "msg/SipIdentityHeader.h"
#include "msg/SipInfoBase.h"
#include "msg/SipIntegerHeader.h"
#include "msg/SipMsgBody.h"
#include "msg/SipPChargingVectorHeader.h"
#include "msg/SipPPreferredServiceHeader.h"
#include "msg/SipPrivacyHeader.h"
#include "msg/SipPVisitedNetworkIdHeader.h"
#include "msg/SipRAcKHeader.h"
#include "msg/SipReferSubHeader.h"
#include "msg/SipRequestDispositionHeader.h"
#include "msg/SipRequestLine.h"
#include "msg/SipResourcePriorityHeader.h"
#include "msg/SipRetryAfterHeader.h"
#include "msg/SipStatusLine.h"
#include "msg/SipTimeStampHeader.h"
#include "msg/SipTriggerConsentHeader.h"
#include "msg/SipUnknownHeader.h"
#include "msg/SipUserAgentHeader.h"
#include "msg/SipViaHeader.h"
#include "msg/SipWarningHeader.h"

class SipHeaders
{
private:
    SipMap<SIP_INT32, SipHeaderBase*> m_objHeaders;

public:
    SipHeaders();
    virtual ~SipHeaders();
    SIP_BOOL CopyHdrs(SipHeaders* pHdrs);
    SIP_BOOL Encode(SIP_CHAR** ppCurrPos, SIP_UINT32 nMsgOptions);

    SIP_BOOL Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen, SIP_CHAR** ppHdrName,
            SIP_CHAR** ppHdrBody);

    SipHeaderBase* GetHdrObj(SIP_INT32 eHdrType);

    SipHeaderBase* GetHdrObj(SIP_INT32 eHdrType, SIP_UINT16 eIndex);

    SIP_BOOL SetHdr(SipHeaderBase* pHeader);
    SIP_BOOL AppendHdr(SipHeaderBase* pHdr);
    SIP_BOOL InsertHdr(SipHeaderBase* pHdr, SIP_UINT32 nIndex);
    SIP_VOID OverWriteHdrObj(IN SipHeaders* pSrcHdrs, IN SIP_BOOL bIgnoreUnknownHeader);
    SIP_BOOL RemoveHdr(SIP_INT32 eHdrType);
    static SipHeaderBase* CreateCoreHdrObj(SIP_INT32 eHdrType);
    static SipHeaderBase* CloneHdrObj(SipHeaderBase* pHdr);
    static SIP_BOOL IsListHdr(SIP_INT32 eHdrType);
    static SIP_BOOL SipEncodeHdrName(
            SIP_INT32 eHdrType, SIP_CHAR** ppMsgBuffCurrPos, SIP_UINT32 nMsgOptions);

    static SIP_BOOL SipEncodeShortHdrName(SIP_INT32 eHdrType, SIP_CHAR** ppMsgBuffCurrPos);

private:
    SipHeaderBase* GetHeader(SIP_INT32 eHdrType);
    SIP_VOID SetHeader(SIP_INT32 eHdrType, SipHeaderBase* pHeader);

    SipHeaderBase* GetNewHdrObj(SIP_INT32 eHdrType, SipHeaderBase* pHeader = SIP_NULL);

    SIP_BOOL EncodeMandatoryHdrs(SIP_CHAR** ppCurrPos, SIP_UINT32 nMsgOptions);

    SIP_BOOL EncodeContentHdrs(SIP_CHAR** ppCurrPos, SIP_UINT32 nMsgOptions);
};

#endif  //__SIP_HEADERS_H__
