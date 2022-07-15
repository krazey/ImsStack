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

#ifndef MTC_MESSAGE_FORMATTER_H_
#define MTC_MESSAGE_FORMATTER_H_

#include "MtcDef.h"
#include "configuration/ConfigDef.h"

class ICoreService;
class IFeatureCaps;
class IMessage;
class IMtcSessionContext;
class ISession;
class AString;
struct CallReasonInfo;

class MessageFormatter
{
public:
    MessageFormatter(IN IMtcSessionContext& objContext);
    virtual ~MessageFormatter();
    MessageFormatter(IN CONST MessageFormatter&) = delete;
    MessageFormatter& operator=(IN CONST MessageFormatter&) = delete;

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
    virtual IMS_RESULT FormStartMessage();
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
    virtual void SetLocation();

    ICoreService* GetICoreService();
    IFeatureCaps* GetIFeatureCaps();

private:
    void SetPPreferredServiceHeader();
    void SetAcceptContactHeader();
    void SetAcceptHeader();
    void AddSrvccFeature();
    void SetSrvccContactParameter();
    // void SetKeepAliveProfile();
    void SetCallerIdHeader();
    // void SetTipHeader();
    void SetSupportedHeader();
    void SetPreconditionHeader();
    void SetPEarlyMediaHeader();
    void SetAlertInfoHeader(IN IMS_BOOL bIncludeAlertInfo);
    void SetReasonHeader(IN CONST AString& strReason);
    void SetCarrierSpecificHeaders();

    IMS_SINT32 GetRejectStatusCode(IN const CallReasonInfo& objReason);
    void GetRejectPhrase(IN const CallReasonInfo& objReason, OUT AString& strPhrase);
    void GetUpdateReason(IN UpdateType eUpdateType, OUT AString& strReason);
    void GetTerminateReason(IN const CallReasonInfo& objReason, OUT AString& strReason);
    AString GetTerminateReason(IN TerminateType eType);
    AString GetRejectPhrase(IN RejectType eType);

    IMS_RESULT InitVariables(IN FormType eFormType);
    IMS_RESULT SetNextMessage();

protected:
    IMtcSessionContext& m_objContext;
    ISession& m_objSession;
    IMessage* m_piNextMessage;

private:
    FormType m_eFormType;
};

#endif
