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

#ifndef MTC_PRECONDITION_MANAGER_H_
#define MTC_PRECONDITION_MANAGER_H_

#include "ImsMap.h"
#include "ImsTypeDef.h"
#include "MediaDef.h"
#include "call/IMtcCall.h"
#include "media/IMediaQosEventListener.h"
#include "precondition/IMtcPreconditionManager.h"
#include "precondition/IQosTimerListener.h"
#include "precondition/QosDef.h"
#include "precondition/QosStatusTable.h"
#include "precondition/QosTimer.h"
#include "precondition/SdpPreconditionHelper.h"

class IMediaDescriptor;
class IMediaSession;
class IMtcCallContext;
class IMtcPreconditionListener;

class QosInfo
{
public:
    inline explicit QosInfo(IN IQosTimerListener* pListener) :
            eAudioStatus(QosStatus::IDLE),
            eVideoStatus(QosStatus::IDLE),
            eTextStatus(QosStatus::IDLE),
            objTimer(QosTimer(pListener)),
            objStatusTable(QosStatusTable()),
            bSupportPrecondition(IMS_FALSE),
            bWaitAudioDedicatedBearerTimerStarted(IMS_FALSE)
    {
    }
    inline virtual ~QosInfo() {}
    QosInfo(IN const QosInfo&) = delete;
    QosInfo& operator=(IN const QosInfo&) = delete;

public:
    inline virtual QosStatus GetAudioStatus() const { return eAudioStatus; }
    inline virtual QosStatus GetVideoStatus() const { return eVideoStatus; }
    inline virtual QosStatus GetTextStatus() const { return eTextStatus; }
    inline virtual QosTimer& GetTimer() { return objTimer; }
    inline virtual QosStatusTable& GetStatusTable() { return objStatusTable; }
    inline virtual IMS_BOOL IsPreconditionSupported() const { return bSupportPrecondition; }
    inline virtual IMS_BOOL IsWaitAudioDedicatedBearerTimerStarted() const
    {
        return bWaitAudioDedicatedBearerTimerStarted;
    }

    inline virtual void SetAudioStatus(IN QosStatus eStatus) { eAudioStatus = eStatus; }
    inline virtual void SetVideoStatus(IN QosStatus eStatus) { eVideoStatus = eStatus; }
    inline virtual void SetTextStatus(IN QosStatus eStatus) { eTextStatus = eStatus; }
    inline virtual void SetSupportingPrecondition(IN IMS_BOOL bSupported)
    {
        bSupportPrecondition = bSupported;
    }
    inline virtual void SetWaitAudioDedicatedBearerTimerStarted()
    {
        bWaitAudioDedicatedBearerTimerStarted = IMS_TRUE;
    }

private:
    QosStatus eAudioStatus;
    QosStatus eVideoStatus;
    QosStatus eTextStatus;
    QosTimer objTimer;
    QosStatusTable objStatusTable;
    IMS_BOOL bSupportPrecondition;
    IMS_BOOL bWaitAudioDedicatedBearerTimerStarted;
};

class MtcPreconditionManager :
        public IMtcPreconditionManager,
        public IMediaQosEventListener,
        public IQosTimerListener
{
public:
    explicit MtcPreconditionManager(IN IMtcCallContext& objContext);
    virtual ~MtcPreconditionManager() override;
    MtcPreconditionManager(IN const MtcPreconditionManager& objRHS) = delete;
    MtcPreconditionManager& operator=(IN const MtcPreconditionManager& objRHS) = delete;

public:
    virtual void CreateQos(IN ISession* piSession) override;
    virtual void DestroyQos(IN ISession* piSession) override;
    virtual void SetListener(IN IMtcPreconditionListener* pListener) override;
    virtual void InitializeMobileRatInformation() override;
    virtual IMS_BOOL IsPreconditionSupportedInLocal() const override;
    virtual IMS_BOOL IsDedicatedBearerAllocated(
            IN ISession* piSession, IN IMS_UINT32 eMediaType) const override;
    virtual IMS_BOOL IsCheckingResourcesRequiredToAlertUser(IN ISession* piSession) const override;
    virtual IMS_BOOL IsAvailableToAlertUser(IN ISession* piSession) const override;
    virtual IMS_BOOL IsLocalResourceConfirmationRequired(IN ISession* piSession) const override;
    virtual IMS_BOOL IsAvailableToSendLocalResourceConfirmation(
            IN ISession* piSession) const override;
    inline IMS_BOOL IsPreconditionIncludedInSdp(IN ISession* piSession) const override
    {
        return m_pSdpPreconditionHelper->IsPreconditionIncludedInSdp(piSession);
    }
    virtual void FormPreconditionSdp(IN ISession* piSession, IN IMS_BOOL bFailure) override;
    virtual void OnSdpReceived(IN ISession* piSession) override;
    virtual void OnSdpSent(IN ISession* piSession, IN IMS_BOOL bInitialInvite = IMS_FALSE) override;
    virtual void OnMessageReceived(IN ISession* piSession, IN IMessage* piMessage) override;
    virtual void OnCallEstablished(IN ISession* piSession) override;
    virtual void OnCallModified(IN ISession* piSession) override;
    virtual void OnRatChanged(IN IMS_SINT32 eRatType) override;
    virtual void UpdateQosIfAvailable(IN ISession* piSession, IN IMS_UINTP nNegoId,
            IN MEDIA_CONTENT_TYPE eNegotiatedMediaType, IN IMediaSession* piMediaSession) override;
    virtual IMS_BOOL IsAudioQosEverAvailable() const override;

public:
    virtual void OnQosStatusChanged(IN ISession* piSession, IN QosStatus eStatus,
            IN IMS_UINT32 eMediaType, IN IMS_BOOL bNeedToNotify = IMS_TRUE) override;
    virtual void OnTimerExpired(IN QosTimer* pTimer, IN QosTimerType eType) override;

protected:
    virtual QosInfo* GetQosInfo(IN ISession* piSession) const;
    IMS_BOOL IsEpsFallback() const;

private:
    void DestroyAllQosInfo();
    IMS_RESULT SetQosStatus(IN ISession* piSession, QosStatus eStatus, IN IMS_UINT32 eMediaType);
    QosStatus GetQosStatus(IN ISession* piSession, IN IMS_UINT32 eMediaType) const;
    QosTimer* GetQosTimer(IN ISession* piSession) const;
    QosStatusTable* GetQosStatusTable(IN ISession* piSession) const;
    void StartQosTimer(IN ISession* piSession, IN QosTimerType eType) const;
    void StopQosTimer(IN ISession* piSession, IN QosTimerType eType) const;
    void StopAllQosTimer(IN ISession* piSession) const;
    void OnWaitAudioDedicatedBearerTimerExpired(IN QosTimer* pTimer);
    void OnWaitAvailableAfterW2LHandoverTimerExpired(IN QosTimer* pTimer);
    void OnWaitVideoTextAvailableTimerExpired(IN const QosTimer* pTimer);
    void OnForceAvailableTimerExpired(IN const QosTimer* pTimer);
    void HandleReservationFailureByTimerExpiration(IN const QosTimer* pTimer);
    void InitializeStatusForUnusedLostQos(IN ISession* piSession);
    void CreateStatusRecordsWithActiveMediaTypes(IN ISession* piSession);
    void CreateStatusRecords(IN ISession* piSession, IN IMS_UINT32 eMediaType);
    void HandleQosTimer(IN ISession* piSession, IN QosStatus eCurrentStatus,
            IN QosStatus eNewStatus, IN IMS_UINT32 eMediaType) const;
    void NotifyQosStatusToListener(
            IN ISession* piSession, IN IMS_BOOL bReserved, IN IMS_UINT32 eMediaTypes);
    void SetOnWlan(IN IMS_BOOL bOnWlan);
    void UpdateMobileRatType(IN IMS_SINT32 eRatType);
    void SetRemoteResourceAvailable(IN ISession* piSession) const;
    void UpdateSupportingPrecondition(IN ISession* piSession, IN IMS_BOOL bRemoteSupported) const;
    void UpdateQosAttributesFromRemoteSdp(IN ISession* piSession);
    IMS_UINT32 GetReservedMediaTypes(IN ISession* piSession, IN CallType eCallType) const;
    static IMS_BOOL IsNeedToUpdateQosStatus(IN QosStatus eCurrentStatus, IN QosStatus eNewStatus);
    IMS_BOOL IsDefaultBearerAllowed(IN IMS_UINT32 eMediaType) const;
    IMS_BOOL IsRemoteResourceReserved(IN ISession* piSession) const;
    IMS_BOOL IsLocalResourceReserved(IN ISession* piSession, IN IMS_BOOL bAtLeastOneReserved) const;
    IMS_BOOL IsLocalResourceReservedByMediaType(
            IN ISession* piSession, IN IMS_UINT32 eMediaType) const;
    IMS_BOOL IsLocalResourceReservedForVideoOrText(IN ISession* piSession);
    IMS_BOOL IsPreconditionSupported(IN ISession* piSession) const;
    IMS_BOOL IsPreconditionSupportedInLocal(IN IMS_UINT32 eMediaType) const;
    static IMS_BOOL IsConfirmedDialog(IN const ISession* piSession);
    IMS_BOOL IsNeedToStartWaitAudioDedicatedBearerTimer(
            IN ISession* piSession, IN IMS_BOOL bSendingInitialInvite) const;
    IMS_UINT32 SetLocalResourceAvailable(IN ISession* piSession);
    IMS_SINT32 GetQosTime(IN QosTimerType eType) const;
    static IMS_SINT32 GetSdpMediaType(IN IMS_UINT32 eMediaType);
    ISession* GetISessionWithTimer(IN const QosTimer* pTimer) const;
    static IMediaDescriptor* GetMediaDescriptor(IN const IMedia* piMedia);
    static const SdpMedia* GetSdpMedia(IN const IMedia* piMedia, IN IMS_BOOL bRemote);
    QosLossPolicy GetQosLossPolicy(IN IMS_UINT32 eMediaType) const;
    QosLossPolicy GetActionForQosLoss(IN ISession* piSession) const;
    IMS_BOOL IsConfirmationRequired(IN const ISession& objISession) const;
    IMS_BOOL IsAudioDedicatedBearerWaitTimerRequiredByRatCondition() const;
    IMS_BOOL IsVideoOrTextIncluded(IN CallType eCallType) const;

protected:
    ImsMap<ISession*, QosInfo*> m_objQosInfos;
    IMtcPreconditionListener* m_pListener;
    IMtcCallContext& m_objContext;
    SdpPreconditionHelper* m_pSdpPreconditionHelper;
    IMS_BOOL m_bLocalResourceConfirmedInitially;
    IMS_BOOL m_bOnWlan;
    IMS_SINT32 m_ePreviousRatType;
    IMS_SINT32 m_eCurrentRatType;
    IMS_BOOL m_bAudioQosEverAvailable;
};

#endif
