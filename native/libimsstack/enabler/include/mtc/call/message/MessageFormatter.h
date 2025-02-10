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

#ifndef MESSAGE_FORMATTER_H_
#define MESSAGE_FORMATTER_H_

#include "IReasonHeaderSetter.h"
#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "configuration/ConfigDef.h"

class AString;
class ICoreService;
class IFeatureCaps;
class IMessage;
class IMtcCallContext;
class ISession;
class ISipMessage;
class MtcSupplementaryService;
struct CallReasonInfo;

class MessageFormatter : public IReasonHeaderSetter
{
public:
    MessageFormatter(IN IMtcCallContext& objContext, IN ISession& objSession);
    virtual ~MessageFormatter();
    MessageFormatter(IN const MessageFormatter&) = delete;
    MessageFormatter& operator=(IN const MessageFormatter&) = delete;

private:
    enum class FormType
    {
        NONE,
        START,
        PROVISIONAL_RESPONSE,
        PRACK,
        PRACK_RESPONSE,
        EARLY_UPDATE,
        EARLY_UPDATE_RESPONSE,
        ACCEPT,
        REJECT,
        ACK,
        UPDATE,
        ACCEPT_UPDATE,
        CANCEL_UPDATE,
        TERMINATE,
    };

public:
    void ReasonHeaderSetter_SetHeader(
            IN ISipMessage* piSipMsg, IN IMS_SINT32 nTerminationReason) override;
    void ReasonHeaderSetter_SetPrivateHeader(
            IN ISipMessage* piOldSipMsg, IN ISipMessage* piNewSipMsg) override;

    virtual IMS_RESULT FormStartMessage(IN CallType eCallType);
    virtual IMS_RESULT FormProvisionalResponseMessage(IN IMS_BOOL bIncludeAlertInfo);
    virtual IMS_RESULT FormPrackMessage();
    virtual IMS_RESULT FormPrackResponseMessage();
    virtual IMS_RESULT FormEarlyUpdateMessage(IN UpdateType eUpdateType);
    virtual IMS_RESULT FormEarlyUpdateResponseMessage();
    virtual IMS_RESULT FormAcceptMessage();
    virtual IMS_RESULT FormRejectMessage(IN const CallReasonInfo& objReason,
            OUT IMS_SINT32& eStatusCode, OUT AString& strPhrase);
    virtual IMS_RESULT FormAckMessage();
    virtual IMS_RESULT FormUpdateMessage(IN UpdateType eUpdateType, IN IMS_BOOL bIncludeAlertInfo);
    virtual IMS_RESULT FormAcceptUpdateMessage();
    virtual IMS_RESULT FormCancelUpdateMessage(IN const CallReasonInfo& objReason);
    virtual IMS_RESULT FormTerminateMessage(IN const CallReasonInfo& objReason);

protected:
    virtual void SetAcceptHeader();
    virtual void SetLocation();

    ICoreService* GetICoreService();
    IFeatureCaps* GetIFeatureCaps();

private:
    void SetPPreferredServiceHeader();
    void SetAcceptContactHeader(IN CallType eCallType);
    void AddSrvccFeature();
    void SetSrvccContactParameter();
    void SetCallerIdHeader();
    // void SetTipHeader();
    void SetPEarlyMediaHeader();
    void SetAlertInfoHeader(IN IMS_BOOL bIncludeAlertInfo);
    void SetReasonHeader(IN const AString& strReason);
    void SetCarrierSpecificHeaders();
    void SetCallComposerElements();
    void SetReplacesHeader();
    void SetHeadersForReject(IN const CallReasonInfo& objReason);

    IMS_SINT32 GetRejectStatusCode(IN const CallReasonInfo& objReason);
    void GetRejectPhrase(IN const CallReasonInfo& objReason, OUT AString& strPhrase);
    static void GetUpdateReason(IN UpdateType eUpdateType, OUT AString& strReason);
    void GetTerminateReason(IN const CallReasonInfo& objReason, OUT AString& strReason);
    AString GetTerminateReason(IN TerminateType eType);
    AString GetRejectPhrase(IN RejectType eType);
    AString GetRejectPhraseForLocalCallBusy(IN IMS_SINT32 nExtraCode);

    IMS_RESULT InitVariables(IN FormType eFormType);
    IMS_RESULT SetNextMessage();

protected:
    IMtcCallContext& m_objContext;
    ISession& m_objSession;
    IMessage* m_piNextMessage;

private:
    FormType m_eFormType;
};

#endif
