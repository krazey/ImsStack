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
#include "MtcDef.h"
#include "call/IMtcCallContext.h"
#include "media/IMediaDescriptor.h"
#include "media/IMediaQosEventListener.h"
#include "precondition/IMtcPreconditionListener.h"
#include "precondition/IMtcPreconditionManager.h"
#include "precondition/QosDef.h"
#include "precondition/QosStatusTable.h"
#include "precondition/QosTimer.h"

class QosData
{
public:
    inline QosData() :
            eAudioStatus(QosStatus::IDLE),
            eVideoStatus(QosStatus::IDLE),
            eTextStatus(QosStatus::IDLE)
    {
    }

    inline virtual ~QosData() {}

public:
    inline QosStatus GetAudioStatus() { return eAudioStatus; }
    inline QosStatus GetVideoStatus() { return eVideoStatus; }
    inline QosStatus GetTextStatus() { return eTextStatus; }

    inline void SetAudioStatus(IN QosStatus eStatus) { eAudioStatus = eStatus; }
    inline void SetVideoStatus(IN QosStatus eStatus) { eVideoStatus = eStatus; }
    inline void SetTextStatus(IN QosStatus eStatus) { eTextStatus = eStatus; }

private:
    QosData& operator=(IN const QosData& objRHS);

private:
    QosStatus eAudioStatus;
    QosStatus eVideoStatus;
    QosStatus eTextStatus;
};

class MtcPreconditionManager :
        public IMtcPreconditionManager,
        public IMediaQosEventListener,
        public IQosTimerListener
{
public:
    MtcPreconditionManager(IN IMtcCallContext& objContext);
    virtual ~MtcPreconditionManager();

private:
    MtcPreconditionManager(IN const MtcPreconditionManager& objRHS);
    MtcPreconditionManager& operator=(IN const MtcPreconditionManager& objRHS);

public:
    virtual void CreateQos(IN ISession* piSession) override;
    virtual void DestroyQos(IN ISession* piSession) override;
    virtual void SetListener(IN IMtcPreconditionListener* pListener) override;
    virtual IMS_BOOL IsResourceReserved(IN ISession* piSession, IN QosCheckType eType) override;

    virtual void StartQosTimer(
            IN ISession* piSession, IN QosTimerType eType = QosTimerType::WAIT_AVAILABLE) override;
    virtual void StopQosTimer(
            IN ISession* piSession, IN QosTimerType eType = QosTimerType::WAIT_AVAILABLE) override;
    virtual void StopAllQosTimer(IN ISession* piSession) override;

    virtual void UpdatePreconditionCapability(
            IN ISession* piSession, IN IMS_BOOL bCapability) override;
    virtual IMS_BOOL HasPreconditionCapability(IN ISession* piSession) override;
    virtual IMS_BOOL IsPreconditionSupportedInLocal() override;
    virtual void UpdateQosAttributesFromSdp(IN ISession* piSession) override;
    virtual void FormPreconditionSdp(IN ISession* piSession, IN IMS_BOOL bFailure) override;
    virtual void RemovePreconditionSdp(IN ISession* piSession) override;
    virtual IMS_UINT32 SetLocalResourceAvailable(IN ISession* piSession) override;
    virtual void SetRemoteResourceAvailable(IN ISession* piSession) override;
    virtual void HandleQosOnIpcanChanged() override;

public:
    virtual void OnQosStatusChanged(
            IN ISession* piSession, IN QosStatus eStatus, IN IMS_UINT32 eMediaType) override;

    virtual void OnWaitTimerExpired(IN QosTimer* pTimer) override;
    virtual void OnGuardInactiveTimerExpired(IN QosTimer* pTimer) override;
    virtual void OnForceAvailableTimerExpired(IN QosTimer* pTimer) override;
    virtual void OnWaitTimerAfterHandOverExpired(IN QosTimer* pTimer) override;

private:
    void CreateQosTimer(IN ISession* piSession);
    void RemoveQosTimer(IN ISession* piSession);

    void CreateStatusTable(IN ISession* piSession);
    void RemoveStatusTable(IN ISession* piSession);

    void CreateQosData(IN ISession* piSession);
    void RemoveQosData(IN ISession* piSession);

    void DestroyAll();

    QosData* GetQosData(IN ISession* piSession);
    QosTimer* GetQosTimer(IN ISession* piSession);
    QosStatusTable* GetQosStatusTable(IN ISession* piSession);

    void HandleReservationFailureByTimerExpiration(IN QosTimer* pTimer);
    QosLossPolicy GetActionForQosLoss(IN ISession* piSession);
    void InitializeStatusForLostQos(IN ISession* piSession);
    void UpdateStatusRecords(IN ISession* piSession);

    void HandleQosTimer(IN ISession* piSession, IN QosStatus eCurrStatus, IN QosStatus eNewStatus);
    void NotifyQosStatusToListener(
            IN ISession* piSession, IN IMS_BOOL bReserved, IN IMS_UINT32 eMediaTypes);

    IMS_BOOL IsStatusAvailable(IN QosStatus eStatus);
    IMediaDescriptor* GetMediaDescriptor(IN IMedia* piMedia);
    const SdpMedia* GetSdpMedia(IN IMedia* piMedia, IN IMS_BOOL bRemote);

    IMS_SINT32 GetSdpMediaType(IN IMS_UINT32 eMediaType);
    IMS_UINT32 GetMediaTypesFromCallType();
    IMS_BOOL IsDefaultBearerUsed(IN IMS_UINT32 eMediaType);
    void InitializeCapability(IN ISession* piSession);
    QosLossPolicy GetQosLossPolicy(IN IMS_UINT32 eMediaType);
    QosStatus GetQosStatus(IN ISession* piSession, IN IMS_UINT32 eMediaType);
    void SetQosStatus(IN ISession* piSession, QosStatus eStatus, IN IMS_UINT32 eMediaType);

    IMS_BOOL IsNeedToUpdateQosStatus(IN QosStatus eCurrStatus, IN QosStatus eNewStatus);
    IMS_BOOL IsPreconditionSupportedInLocal(IN IMS_UINT32 eMediaType);
    IMS_BOOL IsConfirmedDialog(IN const ISession* piSession);
    void SetOnWlan(IN IMS_BOOL bOnWlan);

private:
    ImsMap<ISession*, QosData*> m_objQosDatas;
    ImsMap<ISession*, QosTimer*> m_objQosTimers;
    ImsMap<ISession*, QosStatusTable*> m_objStatusTables;
    ImsMap<ISession*, IMS_BOOL> m_objCapabilities;
    IMtcPreconditionListener* m_pListener;
    IMtcCallContext& m_objContext;
    IMS_BOOL m_bOnWlan;
};

#endif
