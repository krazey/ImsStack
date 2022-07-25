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
#ifndef RCS_MESSAGE_TRACKER_H_
#define RCS_MESSAGE_TRACKER_H_

#include "ImsStateMachine.h"
#include "ISipClientConnectionListener.h"
#include "ISipConnectionFactory.h"
#include "IURcsMessageService.h"
class ICoreService;

class RcsMessageTracker : public ImsStateMachine, public ISipClientConnectionListener
{
    DECLARE_STATE_MAP()

    DECLARE_STATE_MSG_MAP(INITIATED)
    DECLARE_STATE_MSG_MAP(SENDING)
    DECLARE_STATE_MSG_MAP(TERMINATED)

public:
    enum
    {
        INITIATED,  //(MO) initial state before sending message
        SENDING,    //(MO) msg is sending (not delivered yet)
        TERMINATED  //(MO/MT) message is sent/sent failed or received (so all transaction is
                    // completed)
    };

public:
    RcsMessageTracker(IN ISipConnectionFactory* _piscf, IN IMS_SINT32 nSimSlot = 0);
    virtual ~RcsMessageTracker();

    void SetSessionId(IN IMS_UINTP _nSessionId);
    IMS_UINTP GetSessionId();
    IMS_BOOL HandleMessage(IN IMSMSG& objMSG);
    void Abort(IN IMS_SINT32 nReason, IN const IMS_BOOL bNeedAnswer = IMS_TRUE);
    void SetListenerThread(IN const AString& strThread);

protected:
    void PostNotification(IN IMS_SINT32 nMSG, IN IMS_UINTP npParam);

private:
    IMS_BOOL StateINITIATED_SendMessage(IN IMSMSG& objMSG);
    IMS_BOOL StateINITIATED_NotifyReceiveError(IN IMSMSG& objMSG);
    IMS_BOOL StateSENDING_Sent(IN IMSMSG& objMSG);
    IMS_BOOL StateSENDING_SendFailed(IN IMSMSG& objMSG);

    IMS_RESULT SendMessage(IN IUSncMessageParam* pParam);
    IMS_RESULT NotifyReceiveError(IN IUSncNotifyErrorCmdParam* pParam);

public:
    virtual void ClientConnection_NotifyResponse(
            IN ISipClientConnection* piScc, IN ISipClientConnection* piForkedScc = IMS_NULL);

private:
    ISipConnectionFactory* m_piscf;
    ISipClientConnection* m_piscc;
    IMS_SINT32 m_nSimSlot;
    IMS_UINTP m_nSessionId;
    AString m_strListenerThread;
};
#endif  // RCS_MESSAGE_TRACKER_H_