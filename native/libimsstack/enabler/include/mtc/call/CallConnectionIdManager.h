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

#ifndef CALL_CONNECTION_ID_MANAGER_H_
#define CALL_CONNECTION_ID_MANAGER_H_

#include "IMtcCallStateListener.h"
#include "ImsList.h"
#include "ImsTypeDef.h"

class IMtcContext;
class IConferenceController;

class CallConnectionIdManager : public IMtcCallStateListener
{
public:
    explicit CallConnectionIdManager(IN IMtcContext& objContext);
    virtual ~CallConnectionIdManager();
    CallConnectionIdManager(IN const CallConnectionIdManager&) = delete;
    CallConnectionIdManager& operator=(IN const CallConnectionIdManager&) = delete;

    // IMtcCallStateListener interface implementation
    void OnCallStateChanged(IN CallKey nCallKey, IN State eState, IN Type eType,
            IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason) override;
    void OnTotalCallStateChanged(IN State eState) override;
    inline IMS_BOOL IsSynchronousCallRequired() override { return IMS_FALSE; }

    void OnConferenceCallStarted(IN IConferenceController* piController, IN IMS_BOOL bStarted);
    virtual void OnConferenceParticipantDisconnected(IN IMS_UINT32 nConnectionId);

    IMS_SINT32 GetIndex(IN CallKey nKey) const;
    virtual CallKey GetCallKey(IN IMS_UINT32 nConnectionId) const;

private:
    // TODO: GetNewConnectionId?
    IMS_UINT32 GetNewIndex();
    IMS_SINT32 GetListIndexByCallKey(IN CallKey nCallKey);
    IMS_SINT32 GetListIndexByConnectionId(IN IMS_UINT32 nConnectionId);
    void AddKeyConnectionId(IN CallKey nCallKey);
    void RemoveKeyConnectionId(IN IMS_SINT32 nIndex);
    IMS_BOOL IsConferenceParticipant(IN CallKey nCallKey);
    IMS_BOOL IsConferenceHost(IN CallKey nCallKey, OUT IMS_UINT32& nControllerIndex);
    AString GetIds();
    void ClearConnectionIdsInConference(IN IMS_UINT32 nControllerIndex);

private:
    struct CallKeyConnection
    {
    public:
        inline explicit CallKeyConnection(IN CallKey nKey_, IN IMS_UINT32 nConnectionId_)
        {
            nKey = nKey_;
            nConnectionId = nConnectionId_;
        }
        inline ~CallKeyConnection() {}
        CallKeyConnection(IN const CallKeyConnection&) = delete;
        CallKeyConnection& operator=(IN const CallKeyConnection&) = delete;

    public:
        CallKey nKey;
        IMS_UINT32 nConnectionId;
    };

    IMtcContext& m_objContext;
    ImsList<CallKeyConnection*> m_objCallKeyConnections;
    ImsList<IConferenceController*> m_objControllers;
};

#endif
