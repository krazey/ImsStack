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

#ifndef INTERFACE_MTC_CALL_CONTROLLER_H_
#define INTERFACE_MTC_CALL_CONTROLLER_H_

#include "IMtcService.h"
#include "call/IMtcCall.h"
#include "INativeEnabler.h"

enum class KeyType;
union Key;

class IMtcCallController : public INativeEnabler
{
public:
    virtual ~IMtcCallController() {}

    /**
     * Terminates the calls that matches the given key.
     *
     * @param eKeyType All existing calls are affected if `NONE`.
     * @param nKey Key to find the calls.
     * @param objReason The calls can use this information for terminating.
     */
    virtual void TerminateCalls(
            IN KeyType eKeyType, IN Key nKey, IN const CallReasonInfo& objReason) = 0;

    /**
     * Removes the calls that matches the given key without terminating behavior.
     *
     * @param eKeyType All existing calls are affected if `NONE`.
     * @param nKey Key to find the calls.
     */
    virtual void RemoveCalls(IN KeyType eKeyType, IN Key nKey) = 0;

    /**
     * Creates a new outgoing call.
     *
     * @param eServiceType Service type of the new call.
     * @return The key of the new call.
     */
    virtual CallKey Open(IN ServiceType eServiceType, IN CallInfo& objCallInfo) = 0;

    /**
     * Sets an interface to interact with the Java layer.
     *
     * @param nCallKey Key of the call to be manipulated.
     * @param pJniMtcCallThread Interface to send messages to the Java layer.
     */
    virtual void Attach(IN CallKey nCallKey) = 0;

    /**
     * Creates a call to handle the incoming call.
     *
     * @param pService Service of the incoming call.
     * @param piSession Session of the incoming call.
     */
    virtual void HandleIncoming(IN IMtcService* pService, IN ISession* piSession) = 0;

    /**
     * Sets the call information to start the outgoing call.
     *
     * @param nCallKey Key of the call to be manipulated.
     * @param eCallType Type of the call.
     * @param strTarget Remote target.
     * @param pMediaInfo Media of the call.
     * @param lstSuppServices Supplementary services.
     * @param pDialog TODO:
     */
    virtual void Start(IN CallKey nCallKey, IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo* pMediaInfo, IN const IMSMap<SuppType, SuppService*>& objSuppServices,
            IN IDialogEvent* pDialog) = 0;

    /**
     * Notifies the call that the user is alerted by the incoming call.
     *
     * @param nCallKey Key of the call to be manipulated.
     */
    virtual void HandleUserAlert(IN CallKey nCallKey) = 0;

    // Accepts an incoming call.
    // - nIMSKey: Key of the call to be manipulated.
    virtual void Accept(IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo* pMediaInfo) = 0;

    // Rejects an incoming call.
    // - nIMSKey: Key of the call to be manipulated.
    // - eSessionType: See MtcDef::SESSIONTYPE_*
    // - pMediaInfo: `MediaInfo`
    virtual void Reject(IN CallKey nCallKey, IN const CallReasonInfo& objReason) = 0;

    // Holds a call.
    // - nIMSKey: Key of the call to be manipulated.
    // - pMediaInfo: `MediaInfo`
    virtual void Hold(IN CallKey nCallKey, IN MediaInfo* pMediaInfo) = 0;

    // Resumes a call.
    // - nIMSKey: Key of the call to be manipulated.
    // - pMediaInfo: `MediaInfo`
    virtual void Resume(IN CallKey nCallKey, IN MediaInfo* pMediaInfo) = 0;

    // Accepts the resume request from the remote.
    // - nIMSKey: Key of the call to be manipulated.
    // - eSessionType: See MtcDef::SESSIONTYPE_*
    // - pMediaInfo: `MediaInfo`
    virtual void AcceptResume(
            IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo* pMediaInfo) = 0;

    // Rejects the resume request from the remote.
    // - nIMSKey: Key of the call to be manipulated.
    // - objReason: Rejected reason.
    virtual void RejectResume(IN CallKey nCallKey, IN const CallReasonInfo& objReason) = 0;

    // Terminates a call.
    // - nIMSKey: Key of the call to be manipulated.
    // - objReason: Terminate reason.
    virtual void Terminate(IN CallKey nCallKey, IN const CallReasonInfo& objReason) = 0;

    // Modifies media parameters of a call.
    // - nIMSKey: Key of the call to be manipulated.
    // - eSessionType: See MtcDef::SESSIONTYPE_*
    // - pMediaInfo: `MediaInfo`
    virtual void Update(IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo* pMediaInfo) = 0;

    // Cancels the ongoing modification request.
    // - nIMSKey: Key of the call to be manipulated.
    // - objReason: Canceled reason.
    virtual void CancelUpdate(IN CallKey nCallKey, IN const CallReasonInfo& objReason) = 0;

    // Accepts the modification request from the remote.
    // - nIMSKey: Key of the call to be manipulated.
    // - eSessionType: See MtcDef::SESSIONTYPE_*
    // - pMediaInfo: `MediaInfo`
    virtual void AcceptUpdate(
            IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo* pMediaInfo) = 0;

    // Rejects the modification request from the remote.
    // - nIMSKey: Key of the call to be manipulated.
    // - objReason: Rejected reason.
    virtual void RejectUpdate(IN CallKey nCallKey, IN const CallReasonInfo& objReason) = 0;

    // Sends USSD. Nothing happens if the specified call isn't a USSI session.
    // - nIMSKey: Key of the call to be manipulated.
    // - aStrUSSD: USSD string.
    virtual void SendUssd(IN CallKey nCallKey, IN const AString& strUssd) = 0;

    // Handles conference call related IMS messages.
    /*
    void StartGroupCall(IN CallKey nCallKey, IN IMS_UINT32 nCmd, IN IMSList<ConfUser*>& objUsers,
            IN CallInfo& objCallInfo, IN MediaInfo& objMediaInfo,
            IN IMSMap<SuppType, SuppService*>& objSuppServices);
    */

    virtual void MergeToConference(IN CallKey nCallKey, IN IMSList<ConfUser*>& objUsers) = 0;
    virtual void AddToConference(IN CallKey nCallKey, IN IMSList<ConfUser*>& objUsers) = 0;
    virtual void RemoveFromConference(IN CallKey nCallKey, IN IMSList<ConfUser*>& objUsers) = 0;

    // TODO: Consider ECT, SRVCC
    virtual void Transfer(IN CallKey nCallKey, IN const AString& strTarget) = 0;

    virtual void HandleIpcanChanged() = 0;
};

enum class KeyType
{
    NONE,
    CALL_KEY,
    CALL_TYPE,
    SERVICE_TYPE
};

union Key
{
    CallKey nCallKey;
    CallType eCallType;
    ServiceType eServiceType;
};

#endif
