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
#ifndef SIP_MESSAGE_H_
#define SIP_MESSAGE_H_

#include "ISipMessage.h"
#include "SipMessageBodyPart.h"
#include "SipStatusCode.h"
#include "msg/SipMessage.h"

namespace sipcore
{

class SipMessage : public ISipMessage
{
public:
    explicit SipMessage(IN IMS_SINT32 nType = ISipMessage::TYPE_REQUEST);
    explicit SipMessage(IN ::SipMessage* pSipMsg);
    explicit SipMessage(IN ::SipMessage* pSipMsg, IN IMS_BOOL bMessageClone);
    virtual ~SipMessage();

    SipMessage(IN const SipMessage&) = delete;

public:
    SipMessage& operator=(IN const SipMessage& other);

public:
    // ISipObject interface
    void Destroy() override;
    // ISipMessage interface
    ISipMessage* Clone() const override;
    IMS_RESULT AddHeader(IN IMS_SINT32 nType, IN const AString& strValue,
            IN const AString& strName = AString::ConstNull()) override;
    IMS_UINT32 GetCSeqNumber() const override;
    AString GetHeader(IN IMS_SINT32 nType, IN IMS_SINT32 nIndex = 0,
            IN const AString& strName = AString::ConstNull()) const override;
    IMS_SINT32 GetHeaderCount(
            IN IMS_SINT32 nType, IN const AString& strName = AString::ConstNull()) const override;
    IMSList<AString> GetHeaders(
            IN IMS_SINT32 nType, IN const AString& strName = AString::ConstNull()) const override;
    inline const SipMethod& GetMethod() const override { return m_objMethod; }
    inline const AString& GetReasonPhrase() const override
    {
        return m_objStatusCode.GetReasonPhrase();
    }
    inline const AString& GetRequestUri() const override
    {
        return (m_nType == ISipMessage::TYPE_RESPONSE) ? AString::ConstNull() : m_strRequestUri;
    }
    inline IMS_SINT32 GetStatusCode() const override { return m_objStatusCode.ToInt(); }
    inline IMS_SINT32 GetType() const override { return m_nType; }
    IMS_RESULT PrependHeader(IN IMS_SINT32 nType, IN const AString& strValue,
            IN const AString& strName = AString::ConstNull()) override;
    void RemoveHeader(
            IN IMS_SINT32 nType, IN const AString& strName = AString::ConstNull()) override;
    IMS_RESULT SetHeader(IN IMS_SINT32 nType, IN const AString& strValue,
            IN const AString& strName = AString::ConstNull()) override;

    ISipMessageBodyPart* CreateBodyPart() override;
    ISipMessageBodyPart* CreateSdpBodyPart() override;
    IMSList<ISipMessageBodyPart*> GetBodyParts() const override;
    ISipMessageBodyPart* GetSdpBodyPart() const override;
    IMSList<ISipMessageBodyPart*> GetSdpBodyParts() const override;

    IMS_RESULT CopyHeadersAndBodyParts(IN const ISipMessage* piSipMsg) override;
    IMS_BOOL IsHeaderPresent(
            IN IMS_SINT32 nType, IN const AString& strName = AString::ConstNull()) const override;
    IMS_BOOL IsMessageRpr() const override;
    IMS_BOOL IsOptionRequired(IN const AString& strOption) const override;
    IMS_BOOL IsOptionSupported(IN const AString& strOption) const override;
    void RemoveBodyParts() override;
    ByteArray ToByteArray(IN IMS_SINT32 nOptions = OPT_ALL) const override;

    // ISipConnection interface
    SipMessageBodyPart* GetBodyPart() const;

    // ISipClientConnection interface
    IMS_RESULT SetRequestUri(IN const AString& strUri);
    void UpdateRequestUri();

    // ISipServerConnection interface
    inline void SetStatusCode(IN IMS_SINT32 nStatusCode) { m_objStatusCode = nStatusCode; }
    inline void SetReasonPhrase(IN const AString& strPhrase) { m_objStatusCode = strPhrase; }

    // General-purpose methods
    IMS_BOOL CreateBodyParts();
    IMS_BOOL FormMessage();
    IMS_BOOL FormMessageOnChallenge();
    IMS_BOOL FormMessageOnRetransmission();
    inline ::SipMessage* GetMessage() const { return m_pSipMsg; }
    inline void SetMethod(IN const SipMethod& objMethod) { m_objMethod = objMethod; }

    static SipMessage* CreateMessage(IN const ByteArray& objMessage);

private:
    void Init(IN IMS_BOOL bMessageClone);
    IMS_BOOL ExtractBodyParts();
    IMS_BOOL ExtractProperties();
    IMS_BOOL ExtractUnknownHeaders();

private:
    IMS_SINT32 m_nType;
    SipMethod m_objMethod;
    AString m_strRequestUri;
    SipStatusCode m_objStatusCode;
    SipUnknownHeaders m_objUnknownHeaders;
    IMS_BOOL m_bBodyPartParsed;
    IMSList<SipMessageBodyPart*> m_objBodyParts;
    ::SipMessage* m_pSipMsg;
};

}  // namespace sipcore

#endif
