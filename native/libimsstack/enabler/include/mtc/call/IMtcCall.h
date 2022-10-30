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

#ifndef INTERFACE_MTC_CALL_H_
#define INTERFACE_MTC_CALL_H_

#include "AString.h"
#include "IMtcService.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "ImsTypeDef.h"
#include "MtcDef.h"

class CallContext;
class ISession;
class MediaInfo;
class SuppService;
class IMtcCallContext;
enum class UpdateType;
struct CallReasonInfo;
struct ConfUser;

using CallKey = IMS_ULONG;

enum class CallType
{
    UNKNOWN = 0,
    VOIP = 1,
    VT = 2,
    RTT = 3,
    VIDEO_RTT = 4,
};

enum class PeerType
{
    MO,
    MT,
};

class IMtcCall
{
public:
    enum class State
    {
        IDLE,
        OUTGOING,
        INCOMING,
        ALERTING,
        ESTABLISHED,
        UPDATING,
        TERMINATING,
    };

    static const CallKey CALL_KEY_INVALID = 0;

    virtual ~IMtcCall(){};

    // Sets thread to interact with the Java layer. Nothing happens if the thread is null.
    virtual void Attach() = 0;

    // Starts an outgoing call.
    virtual void Start(IN CallType eCallType, IN const AString& strTarget, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    virtual void StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo* pMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN const ImsList<ConfUser*>& objUsers) = 0;

    virtual void StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN const ImsList<ConfUser*>& objUsers) = 0;

    // Handles an incoming call.
    virtual void HandleIncoming(IN ISession* piSession) = 0;

    // Notifies that the user alerting for this call is started.
    virtual void HandleUserAlert() = 0;

    // Accepts the incoming call.
    virtual void Accept(IN CallType eCallType, IN MediaInfo* pMediaInfo) = 0;

    // Rejects the incoming call.
    virtual void Reject(IN const CallReasonInfo& objReason) = 0;

    // Holds the call.
    virtual void Hold(IN MediaInfo* pMediaInfo) = 0;

    // Resumes the call.
    virtual void Resume(IN MediaInfo* pMediaInfo) = 0;

    // Accepts the resume request from the remote.
    virtual void AcceptResume(IN CallType eCallType, IN MediaInfo* pMediaInfo) = 0;

    // Rejects the resume request from the remote.
    virtual void RejectResume(IN const CallReasonInfo& objReason) = 0;

    // Requests call updating(converting) to the remote.
    virtual void Update(IN CallType eCallType, IN MediaInfo* pMediaInfo) = 0;

    // Accepts the call updating(converting) request from the remote.
    virtual void AcceptUpdate(IN CallType eCallType, IN MediaInfo* pMediaInfo) = 0;

    // Rejects the call updating(converting) request from the remote.
    virtual void RejectUpdate(IN const CallReasonInfo& objReason) = 0;

    // Cancels the ongoing call updating(converting) request.
    virtual void CancelUpdate(IN const CallReasonInfo& objReason) = 0;

    // Terminates the call.
    virtual void Terminate(IN const CallReasonInfo& objReason) = 0;

    // Sends DTMF to the remote.
    virtual void SendDtmf(IN const AString& strSignal, IN IMS_SINT32 nDuration) = 0;

    // Sends USSD. Does nothing if the call isn't a USSI call.
    virtual void SendUssd(IN const AString& strUssd) = 0;

    // Returns a key to uniquely identify this call.
    virtual CallKey GetKey() const = 0;

    /**
     * Returns the current call type.
     *
     * @return Current call type.
     */
    virtual CallType GetCallType() const = 0;

    /**
     * Returns the current call state.
     *
     * @return Current state.
     */
    virtual State GetState() const = 0;

    virtual IMtcCallContext& GetCallContext() const = 0;
};

struct CallInfo
{
public:
    explicit CallInfo() :
            ePeerType(PeerType::MO),
            eInitialCallType(CallType::VOIP),
            bEmergency(IMS_FALSE),
            bOffline(IMS_FALSE),
            bUssi(IMS_FALSE),
            bConference(IMS_FALSE)
    {
    }

    explicit CallInfo(IN const CallInfo& objRhs) :
            ePeerType(objRhs.ePeerType),
            eInitialCallType(objRhs.eInitialCallType),
            bEmergency(objRhs.bEmergency),
            bOffline(objRhs.bOffline),
            bUssi(objRhs.bUssi),
            bConference(objRhs.bConference)
    {
    }

    CallInfo& operator=(IN const CallInfo& objRhs)
    {
        if (this != &objRhs)
        {
            ePeerType = objRhs.ePeerType;
            eInitialCallType = objRhs.eInitialCallType;
            bEmergency = objRhs.bEmergency;
            bOffline = objRhs.bOffline;
            bUssi = objRhs.bUssi;
            bConference = objRhs.bConference;
        }

        return *this;
    }

    IMS_BOOL operator==(const CallInfo& objRhs) const
    {
        if (this == &objRhs)
        {
            return IMS_TRUE;
        }

        return ePeerType == objRhs.ePeerType && eInitialCallType == objRhs.eInitialCallType &&
                bEmergency == objRhs.bEmergency && bOffline == objRhs.bOffline &&
                bUssi == objRhs.bUssi && bConference == objRhs.bConference;
    }

    IMS_BOOL operator!=(const CallInfo& objRhs) const { return !(*this == objRhs); }

public:
    PeerType ePeerType;
    CallType eInitialCallType;
    IMS_BOOL bEmergency;
    IMS_BOOL bOffline;
    IMS_BOOL bUssi;
    IMS_BOOL bConference;
};

#endif
