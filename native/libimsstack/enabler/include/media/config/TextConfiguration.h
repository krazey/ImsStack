/**
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

#ifndef _TEXT_CONFIGURATION_H_
#define _TEXT_CONFIGURATION_H_

#include "config/MediaConfiguration.h"

class ICarrierConfig;

class TextConfiguration : public MediaConfiguration
{
public:
    TextConfiguration(MEDIA_CONTENT_TYPE _eSessionType = MEDIA_TYPE_TEXT);
    virtual ~TextConfiguration();

public:
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc);
    virtual IMS_BOOL Update(IN ICarrierConfig* piCc);

protected:
    virtual IMS_BOOL CreateCodecConfigs(IN ICarrierConfig* piCc);
    virtual void ToDebugString() const;

public:
    IMS_SINT32 GetT140PayloadType() const;
    IMS_SINT32 GetRedPayloadType() const;
    IMS_BOOL IsTextCodecEmptyRedundantEnabled() const;

    static const IMS_SINT32 NEED_TO_CHECK_I = 0;

    static const IMS_SINT32 DEFAULT_RTP_PORT = 50010;
    static const IMS_SINT32 DEFAULT_RTP_PORT_END = 50060;
    static const IMS_SINT32 DEFAULT_RTCP_PORT = 50011;
    static const IMS_SINT32 DEFAULT_RTCP_INVERVAL_LIVE = 5;
    static const IMS_SINT32 DEFAULT_RTCP_INVERVAL = 5;
    static const IMS_SINT32 DEFAULT_AS = 4;
    static const IMS_SINT32 DEFAULT_RS = 300;
    static const IMS_SINT32 DEFAULT_RR = 100;
    static const IMS_SINT32 DEFAULT_RTP_INACTIVITY = 20000;
    static const IMS_SINT32 DEFAULT_RTCP_INACTIVITY = 200000;

    static const IMS_SINT32 DEFAULT_PAYLOAD_T140 = NEED_TO_CHECK_I;
    static const IMS_SINT32 DEFAULT_PAYLOAD_RED = NEED_TO_CHECK_I;
    static const IMS_BOOL DEFAULT_EMPTY_REDUNDANT = IMS_FALSE;

private:
    IMS_SINT32 nT140PayloadType;
    IMS_SINT32 nRedPayloadType;
    IMS_BOOL bTextCodecEmptyRedundantEnabled;
};
#endif  // _TEXT_CONFIGURATION_H_
