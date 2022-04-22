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

#ifndef _CODEC_CONFIG_H_
#define _CODEC_CONFIG_H_

#include "AString.h"
#include "ImsSlot.h"

class IConfigBuffer;

class CodecConfig :
        public IMSSlot
{
private:
    static const IMS_CHAR KEY_PAYLOAD_TYPE[];
    static const IMS_CHAR KEY_NETWORK_TYPE[];
    static const IMS_CHAR KEY_PRIORITY[];
    static const IMS_CHAR KEY_BANDWIDTH[];

public:
    CodecConfig(IN IMS_SINT32 nCodecType_, IN CONST AString &strCodecName_,
            IN CONST IMS_SINT32 nProfileNum_ = 0);
    virtual ~CodecConfig();

private:
    CodecConfig();
    CodecConfig(IN CONST CodecConfig &objRHS);
    CodecConfig& operator=(IN CONST CodecConfig &objRHS);

public:
    virtual IMS_BOOL Create(IN IConfigBuffer *piBuffer);
    virtual AString GetFmtp() const;
    virtual AString GetRtpMap() const;
    virtual void ToDebugString() const;

    IMS_SINT32 GetCodecType() const;
    const AString& GetCodecName() const;
    IMS_SINT32 GetProfileNum() const;
    IMS_SINT32 GetPayloadType() const;
    IMS_UINT32 GetAvailableNetworkType() const;
    IMS_UINT32 GetPriority() const;

    IMS_BOOL SetPriority(IMS_UINT32 priority);

protected:
    void GetSection(OUT AString &strSection) const;

protected:
    AString strSectionName;
    IMS_SINT32 nProfileNum;
    IMS_SINT32 nPayloadType;
    IMS_UINT32 nNetworkType;
    IMS_SINT32 nCodecType;
    AString strCodecName;
    IMS_UINT32 nPriority;
};
#endif                                              // _CODEC_CONFIG_H_
