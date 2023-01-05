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
#ifndef SIP_MESSAGE_BODY_PART_H_
#define SIP_MESSAGE_BODY_PART_H_

#include "ISipMessageBodyPart.h"
#include "SipUnknownHeaders.h"
#include "msg/SipMsgBody.h"

class SipMessageBodyPart : public ISipMessageBodyPart
{
public:
    explicit SipMessageBodyPart(IN IMS_BOOL bSdpBody = IMS_FALSE);
    explicit SipMessageBodyPart(IN SipMsgBody* pMsgBody, IN IMS_BOOL bSdpBody = IMS_FALSE);
    virtual ~SipMessageBodyPart();

    SipMessageBodyPart(IN const SipMessageBodyPart&) = delete;

public:
    SipMessageBodyPart& operator=(IN const SipMessageBodyPart& other);

public:
    // ISipObject interface
    void Destroy() override;
    // ISipMessageBodyPart interface
    ISipMessageBodyPart* Clone() const override;
    void CopyFrom(IN const ISipMessageBodyPart* piBodyPart) override;
    AString GetHeader(
            IN IMS_SINT32 nType, IN const AString& strName = AString::ConstNull()) const override;
    void SetHeader(IN IMS_SINT32 nType, IN const AString& strValue,
            IN const AString& strName = AString::ConstNull()) override;
    inline const ByteArray& GetContent() const override { return m_objContent; }
    void SetContent(IN const ByteArray& objContent) override;

    IMS_BOOL FormMessageBody();
    void SetHeader(
            IN SipHeaderBase* pSipHdr, IN IMS_SINT32 nType = ISipMessageBodyPart::CONTENT_UNKNOWN);
    inline SipMsgBody* GetMessageBody() const { return m_pMsgBody; }
    inline IMS_BOOL IsSdpBodyPart() const { return m_bSdpBody; }

private:
    IMS_BOOL ExtractProperties();

private:
    IMS_BOOL m_bSdpBody;
    SipUnknownHeaders m_objOtherMimeHeaders;
    ByteArray m_objContent;
    SipMsgBody* m_pMsgBody;
};

#endif
