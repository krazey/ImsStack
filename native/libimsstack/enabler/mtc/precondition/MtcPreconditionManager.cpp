#include "media/IMedia.h"
#include "precondition/MtcPreconditionManager.h"
#include "precondition/SdpPreconditionHelper.h"
#include "media/IMtcMediaManager.h"
#include "call/MtcSession.h"
#include "configuration/MtcConfigurationProxy.h"
#include "Configuration.h"
#include "ISubscriberConfig.h"
#include "ServiceTrace.h"
#include "precondition/QosStringDef.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcPreconditionManager::MtcPreconditionManager(IN IMtcCallContext& objContext) :
        m_objQosDatas(IMSMap<ISession*, QosData*>()),
        m_objQosTimers(IMSMap<ISession*, QosTimer*>()),
        m_objStatusTables(IMSMap<ISession*, QosStatusTable*>()),
        m_objCapabilities(IMSMap<ISession*, IMS_BOOL>()),
        m_pListener(IMS_NULL),
        m_objContext(objContext)
{
    IMS_TRACE_D("+MtcPreconditionManager Callkey[%" PFLS_x "]", m_objContext.GetCallKey(), 0, 0);
    m_objContext.GetMediaManager().SetQosListener(this);
}

PUBLIC VIRTUAL MtcPreconditionManager::~MtcPreconditionManager()
{
    IMS_TRACE_D("~MtcPreconditionManager Callkey[%" PFLS_x "]", m_objContext.GetCallKey(), 0, 0);
    m_objContext.GetMediaManager().SetQosListener(IMS_NULL);
    DestroyAll();
}

PUBLIC VIRTUAL void MtcPreconditionManager::CreateQos(IN ISession* piSession)
{
    IMS_TRACE_D("CreateQos", 0, 0, 0);
    CreateQosData(piSession);
    CreateQosTimer(piSession);
    CreateStatusTable(piSession);

    InitializeCapability(piSession);
}

PUBLIC VIRTUAL void MtcPreconditionManager::DestroyQos(IN ISession* piSession)
{
    IMS_TRACE_D("DestroyQos", 0, 0, 0);
    RemoveQosData(piSession);
    RemoveQosTimer(piSession);
    RemoveStatusTable(piSession);

    m_objCapabilities.Remove(piSession);
}

PUBLIC VIRTUAL void MtcPreconditionManager::SetListener(IN IMtcPreconditionListener* pListener)
{
    m_pListener = pListener;
}

PUBLIC VIRTUAL IMS_BOOL MtcPreconditionManager::IsResourceReserved(
        IN ISession* piSession, IN QosCheckType eType)
{
    if (!IsPreconditionSupportedInLocal())
    {
        IMS_TRACE_D("IsResourceReserved : don't use precondition mechanism.", 0, 0, 0);
        return IMS_TRUE;
    }

    IMS_UINT32 eMediaTypes = SdpPreconditionHelper::GetMediaTypesBySdp(piSession);
    if (eMediaTypes == MEDIATYPE_NONE)
    {
        IMS_TRACE_D("IsResourceReserved : can't get media types from SDP.", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bAudioEnabled = IMS_TRUE;
    IMS_BOOL bVideoEnabled = IMS_TRUE;
    IMS_BOOL bTextEnabled = IMS_TRUE;

    if (eType != QosCheckType::LOCAL_STATUS)
    {
        QosStatusTable* pStatusTable = GetQosStatusTable(piSession);
        if (pStatusTable == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (eMediaTypes & MEDIATYPE_AUDIO)
        {
            bAudioEnabled = pStatusTable->IsCurrentStatusEnabled(
                    SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_REMOTE);
        }

        if (eMediaTypes & MEDIATYPE_VIDEO)
        {
            bVideoEnabled = pStatusTable->IsCurrentStatusEnabled(
                    SdpMedia::TYPE_VIDEO, SdpPrecondition::STATUS_REMOTE);
        }

        if (eMediaTypes & MEDIATYPE_TEXT)
        {
            bTextEnabled = pStatusTable->IsCurrentStatusEnabled(
                    SdpMedia::TYPE_TEXT, SdpPrecondition::STATUS_REMOTE);
        }
    }

    if (eType != QosCheckType::REMOTE_STATUS)
    {
        if (eMediaTypes & MEDIATYPE_AUDIO)
        {
            bAudioEnabled =
                    bAudioEnabled && IsStatusAvailable(GetQosStatus(piSession, MEDIATYPE_AUDIO));
        }

        if (eMediaTypes & MEDIATYPE_VIDEO)
        {
            bVideoEnabled =
                    bVideoEnabled && IsStatusAvailable(GetQosStatus(piSession, MEDIATYPE_VIDEO));
        }

        if (eMediaTypes & MEDIATYPE_TEXT)
        {
            bTextEnabled =
                    bTextEnabled && IsStatusAvailable(GetQosStatus(piSession, MEDIATYPE_TEXT));
        }
    }

    IMS_BOOL bResult = bAudioEnabled && bVideoEnabled && bTextEnabled;
    IMS_TRACE_D("IsResourceReserved : CheckType[%s] Result[%s]", PS_QosCheckType(eType),
            _TRACE_B_(bResult), 0);

    return bResult;
}

PUBLIC VIRTUAL void MtcPreconditionManager::StartQosTimer(
        IN ISession* piSession, IN QosTimerType eType /*= QosTimerType::WAIT_AVAILABLE*/)
{
    if (!IsPreconditionSupportedInLocal())
    {
        IMS_TRACE_D("StartQosTimer : don't use precondition mechanism.", 0, 0, 0);
        return;
    }

    QosTimer* pTimer = GetQosTimer(piSession);
    if (pTimer == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nDuration = 0;

    if (eType == QosTimerType::WAIT_AVAILABLE)
    {
        nDuration =
                m_objContext.GetConfigurationProxy().GetInt(Feature::DEDICATED_BEARER_WAIT_TIMER);

        // TODO: restore when QoS event from media module is activated.
        // if (nDuration < 0)
        {
            IMS_TRACE_D("StartQosTimer : start the timer to enable QoS by force.", 0, 0, 0);
            eType = QosTimerType::FORCE_AVAILABLE;
            nDuration = 500;
        }
    }
    else if (eType == QosTimerType::GUARD_INACTIVE)
    {
        nDuration = 1000;
    }

    IMS_TRACE_D("StartQosTimer : Type[%s] Duration[%d]", PS_QosTimerType(eType), nDuration, 0);
    pTimer->StartQosTimer(eType, nDuration);
}

PUBLIC VIRTUAL void MtcPreconditionManager::StopQosTimer(
        IN ISession* piSession, IN QosTimerType eType /*= QosTimerType::WAIT_AVAILABLE*/)
{
    QosTimer* pTimer = GetQosTimer(piSession);
    if (pTimer == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("StopQosTimer : Type[%s]", PS_QosTimerType(eType), 0, 0);
    pTimer->StopQosTimer(eType);
}

PUBLIC VIRTUAL void MtcPreconditionManager::StopAllQosTimer(IN ISession* piSession)
{
    IMS_TRACE_D("StopAllQosTimer", 0, 0, 0);
    StopQosTimer(piSession, QosTimerType::WAIT_AVAILABLE);
    StopQosTimer(piSession, QosTimerType::GUARD_INACTIVE);
    StopQosTimer(piSession, QosTimerType::FORCE_AVAILABLE);
}

PUBLIC VIRTUAL void MtcPreconditionManager::UpdatePreconditionCapability(
        IN ISession* piSession, IN IMS_BOOL bCapability)
{
    IMS_BOOL bLocalCapa = IsPreconditionSupportedInLocal();
    IMS_TRACE_D("UpdatePreconditionCapability : Local[%s] Remote[%s]", _TRACE_B_(bLocalCapa),
            _TRACE_B_(bCapability), 0);

    m_objCapabilities.Add(piSession, (bLocalCapa && bCapability));
}

PUBLIC VIRTUAL IMS_BOOL MtcPreconditionManager::HasPreconditionCapability(IN ISession* piSession)
{
    IMS_SLONG nIndex = m_objCapabilities.GetIndexOfKey(piSession);
    if (nIndex < 0)
    {
        IMS_TRACE_D("HasPreconditionCapability : no capability.", 0, 0, 0);
        return IMS_FALSE;
    }

    return m_objCapabilities.GetValueAt(nIndex);
}

PUBLIC VIRTUAL IMS_BOOL MtcPreconditionManager::IsPreconditionSupportedInLocal()
{
    IMS_BOOL bSupport = IMS_FALSE;

    CallType eCallType = m_objContext.GetCallInfo().eCallType;
    switch (eCallType)
    {
        case CallType::VOIP:
            bSupport = IsPreconditionSupportedInLocal(MEDIATYPE_AUDIO);
            break;
        case CallType::VT:
            bSupport = IsPreconditionSupportedInLocal(MEDIATYPE_AUDIO) ||
                    IsPreconditionSupportedInLocal(MEDIATYPE_VIDEO);
            break;
        case CallType::RTT:
            bSupport = IsPreconditionSupportedInLocal(MEDIATYPE_AUDIO) ||
                    IsPreconditionSupportedInLocal(MEDIATYPE_TEXT);
            break;
        case CallType::VIDEO_RTT:
            bSupport = IsPreconditionSupportedInLocal(MEDIATYPE_AUDIO) ||
                    IsPreconditionSupportedInLocal(MEDIATYPE_VIDEO) ||
                    IsPreconditionSupportedInLocal(MEDIATYPE_TEXT);
            break;
        default:
            break;
    }

    IMS_TRACE_D("IsPreconditionSupportedInLocal : CallType[%d] Result[%s]", eCallType,
            _TRACE_B_(bSupport), 0);
    return bSupport;
}

PUBLIC VIRTUAL void MtcPreconditionManager::UpdateQosAttributesFromSdp(IN ISession* piSession)
{
    if (piSession == IMS_NULL)
    {
        return;
    }

    QosStatusTable* pStatusTable = GetQosStatusTable(piSession);
    if (pStatusTable == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("UpdateQosAttributesFromSdp", 0, 0, 0);

    UpdateStatusRecords(piSession);

    IMSList<IMedia*> lstMedias = piSession->GetMedia();
    for (IMS_UINT32 index = 0; index < lstMedias.GetSize(); index++)
    {
        IMedia* piMedia = lstMedias.GetAt(index);
        if (piMedia == IMS_NULL)
        {
            continue;
        }

        if (SdpPreconditionHelper::IsPreconditionIncludedInSdp(piSession))
        {
            pStatusTable->UpdateStatusTableWithRemoteSdp(piMedia);
        }
    }
}

PUBLIC VIRTUAL void MtcPreconditionManager::FormPreconditionSdp(
        IN ISession* piSession, IN IMS_BOOL bFailure)
{
    if (!HasPreconditionCapability(piSession))
    {
        IMS_TRACE_D("FormPreconditionSdp : UE don't have precondition capability.", 0, 0, 0);
        RemovePreconditionSdp(piSession);
        return;
    }

    IMS_TRACE_D("FormPreconditionSdp", 0, 0, 0);

    if (bFailure)
    {
        SdpPreconditionHelper::FormFailurePreconditionSdp(piSession);
        return;
    }

    QosStatusTable* pStatusTable = GetQosStatusTable(piSession);
    if (pStatusTable == IMS_NULL)
    {
        return;
    }

    IMSList<IMedia*> lstMedias = piSession->GetMedia();
    for (IMS_UINT32 index = 0; index < lstMedias.GetSize(); index++)
    {
        IMedia* piMedia = lstMedias.GetAt(index);
        const SdpMedia* pLocalSdp = GetSdpMedia(piMedia, IMS_FALSE);
        if (pLocalSdp == IMS_NULL)
        {
            continue;
        }

        IMS_SINT32 eSdpMediaType = pLocalSdp->GetType();

        if ((pLocalSdp->GetPort() <= 0) || !pStatusTable->IsStatusRecordsListEmpty(eSdpMediaType))
        {
            continue;
        }

        pStatusTable->CreateStatusRecords(eSdpMediaType);

        IMS_UINT32 eMediaType = SdpPreconditionHelper::GetMediaType(pLocalSdp, piMedia->GetState());

        IMS_BOOL bCheckQosWhileCallUpgrade =
                (m_objContext.GetConfigurationProxy().GetInt(
                         Feature::POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING) == 2);

        if (!bCheckQosWhileCallUpgrade && IsConfirmedDialog(piSession))
        {
            SetQosStatus(piSession, QosStatus::AVAILABLE, eMediaType);
        }

        pStatusTable->UpdateLocalCurrentStatus(
                eSdpMediaType, IsStatusAvailable(GetQosStatus(piSession, eMediaType)));
    }

    IMS_BOOL bUseConf = (m_objContext.GetCallInfo().ePeerType == PeerType::MT);
    if (IsConfirmedDialog(piSession))
    {
        // TODO: check if the confirmation status should be added when UE uses qos check
        // for upgrading the call.
        bUseConf = IMS_FALSE;
    }

    SdpPreconditionHelper::FormPreconditionSdp(piSession, pStatusTable, bUseConf);
}

PUBLIC VIRTUAL void MtcPreconditionManager::RemovePreconditionSdp(IN ISession* piSession)
{
    IMS_TRACE_D("RemovePreconditionSdp", 0, 0, 0);
    SdpPreconditionHelper::RemovePreconditionSdp(piSession);
}

PUBLIC VIRTUAL IMS_UINT32 MtcPreconditionManager::SetLocalResourceAvailable(IN ISession* piSession)
{
    IMS_TRACE_D("SetLocalResourceAvailable", 0, 0, 0);

    IMS_UINT32 eEnabledMediaTypes = MEDIATYPE_NONE;
    QosStatusTable* pStatusTable = GetQosStatusTable(piSession);
    if (pStatusTable == IMS_NULL)
    {
        return eEnabledMediaTypes;
    }

    IMS_UINT32 eMediaTypes = SdpPreconditionHelper::GetMediaTypesBySdp(piSession);
    if (eMediaTypes == MEDIATYPE_NONE)
    {
        eMediaTypes = GetMediaTypesFromCallType();
    }

    if ((eMediaTypes & MEDIATYPE_AUDIO) &&
            GetQosStatus(piSession, MEDIATYPE_AUDIO) != QosStatus::AVAILABLE)
    {
        SetQosStatus(piSession, QosStatus::AVAILABLE, MEDIATYPE_AUDIO);
        pStatusTable->UpdateLocalCurrentStatus(GetSdpMediaType(MEDIATYPE_AUDIO), IMS_TRUE);
        eEnabledMediaTypes |= MEDIATYPE_AUDIO;
    }

    if ((eMediaTypes & MEDIATYPE_VIDEO) &&
            GetQosStatus(piSession, MEDIATYPE_VIDEO) != QosStatus::AVAILABLE)
    {
        SetQosStatus(piSession, QosStatus::AVAILABLE, MEDIATYPE_VIDEO);
        pStatusTable->UpdateLocalCurrentStatus(GetSdpMediaType(MEDIATYPE_VIDEO), IMS_TRUE);
        eEnabledMediaTypes |= MEDIATYPE_VIDEO;
    }

    if ((eMediaTypes & MEDIATYPE_TEXT) &&
            GetQosStatus(piSession, MEDIATYPE_TEXT) != QosStatus::AVAILABLE)
    {
        SetQosStatus(piSession, QosStatus::AVAILABLE, MEDIATYPE_TEXT);
        pStatusTable->UpdateLocalCurrentStatus(GetSdpMediaType(MEDIATYPE_TEXT), IMS_TRUE);
        eEnabledMediaTypes |= MEDIATYPE_TEXT;
    }

    IMS_TRACE_D("SetLocalResourceAvailable : Enabled Media Types[%d]", eEnabledMediaTypes, 0, 0);
    return eEnabledMediaTypes;
}

PUBLIC VIRTUAL void MtcPreconditionManager::SetRemoteResourceAvailable(IN ISession* piSession)
{
    QosStatusTable* pStatusTable = GetQosStatusTable(piSession);
    if (pStatusTable == IMS_NULL)
    {
        IMS_TRACE_D("SetRemoteResourceAvailable : can't get StatusTable", 0, 0, 0);
        return;
    }

    IMSList<IMedia*> lstMedias = piSession->GetMedia();
    IMS_UINT32 nSize = lstMedias.GetSize();

    IMS_TRACE_D("SetRemoteResourceAvailable : media size[%d]", nSize, 0, 0);

    for (IMS_UINT32 index = 0; index < nSize; index++)
    {
        IMedia* piMedia = lstMedias.GetAt(index);
        if (piMedia == IMS_NULL)
        {
            continue;
        }

        const SdpMedia* pRemoteSdp = GetSdpMedia(piMedia, IMS_TRUE);
        if (pRemoteSdp == IMS_NULL)
        {
            continue;
        }

        pStatusTable->EnableRemoteCurrentStatus(pRemoteSdp->GetType());
    }
}

PUBLIC VIRTUAL void MtcPreconditionManager::OnQosStatusChanged(
        IN ISession* piSession, IN QosStatus eStatus, IN IMS_UINT32 eMediaType)
{
    if (piSession == IMS_NULL)
    {
        IMS_TRACE_D("OnQosStatusChanged : ISession is null", 0, 0, 0);
        return;
    }

    // 1. Update QosData and check QosStatus if it needs to be updated or not.
    QosStatus eCurrStatus = GetQosStatus(piSession, eMediaType);
    eStatus = IsDefaultBearerUsed(eMediaType) ? QosStatus::AVAILABLE : eStatus;

    if (eCurrStatus == eStatus)
    {
        IMS_TRACE_D("OnQosStatusChanged : there's no update for status.", 0, 0, 0);
        return;
    }

    IMS_TRACE_D("OnQosStatusChanged", 0, 0, 0);

    SetQosStatus(piSession, eStatus, eMediaType);

    // 2. Update QosStatusTable
    QosStatusTable* pStatusTable = GetQosStatusTable(piSession);
    if (pStatusTable == IMS_NULL)
    {
        return;
    }

    pStatusTable->UpdateLocalCurrentStatus(GetSdpMediaType(eMediaType), IsStatusAvailable(eStatus));

    // 3. Handle Qos Timer
    HandleQosTimer(piSession, eCurrStatus, eStatus);

    if (eCurrStatus == QosStatus::IDLE && eStatus == QosStatus::AVAILABLE)
    {
        NotifyQosStatusToListener(piSession, IsStatusAvailable(eStatus), eMediaType);
    }
}

PUBLIC VIRTUAL void MtcPreconditionManager::OnWaitTimerExpired(IN QosTimer* pTimer)
{
    IMS_TRACE_D("OnWaitTimerExpired", 0, 0, 0);
    HandleReservationFailureByTimerExpiration(pTimer);
}

PUBLIC VIRTUAL void MtcPreconditionManager::OnGuardInactiveTimerExpired(IN QosTimer* pTimer)
{
    IMS_TRACE_D("OnGuardInactiveTimerExpired", 0, 0, 0);
    HandleReservationFailureByTimerExpiration(pTimer);
}

PUBLIC VIRTUAL void MtcPreconditionManager::OnForceAvailableTimerExpired(IN QosTimer* pTimer)
{
    IMS_TRACE_D("OnForceAvailableTimerExpired", 0, 0, 0);

    ISession* piSession = IMS_NULL;
    for (IMS_UINT32 index = 0; index < m_objQosTimers.GetSize(); index++)
    {
        QosTimer* pTempTimer = m_objQosTimers.GetValueAt(index);
        if (pTempTimer == pTimer)
        {
            piSession = m_objQosTimers.GetKeyAt(index);
            break;
        }
    }

    if (piSession == IMS_NULL)
    {
        return;
    }

    IMS_UINT32 eEnabledMediaTypes = SetLocalResourceAvailable(piSession);

    if (eEnabledMediaTypes & MEDIATYPE_AUDIO)
    {
        m_pListener->QosReserved(piSession, MEDIATYPE_AUDIO);
    }

    if (eEnabledMediaTypes & MEDIATYPE_VIDEO)
    {
        m_pListener->QosReserved(piSession, MEDIATYPE_VIDEO);
    }

    if (eEnabledMediaTypes & MEDIATYPE_TEXT)
    {
        m_pListener->QosReserved(piSession, MEDIATYPE_TEXT);
    }
}

PRIVATE
void MtcPreconditionManager::CreateQosTimer(IN ISession* piSession)
{
    QosTimer* pTimer = GetQosTimer(piSession);
    if (pTimer != IMS_NULL)
    {
        IMS_TRACE_D("CreateQosTimer : Timer has been already created.", 0, 0, 0);
        return;
    }

    IMS_TRACE_D("CreateQosTimer", 0, 0, 0);

    pTimer = new QosTimer(this);
    m_objQosTimers.Add(piSession, pTimer);
}

PRIVATE
void MtcPreconditionManager::RemoveQosTimer(IN ISession* piSession)
{
    QosTimer* pTimer = GetQosTimer(piSession);
    if (pTimer != IMS_NULL)
    {
        delete pTimer;
        m_objQosTimers.Remove(piSession);
    }

    IMS_TRACE_D("RemoveQosTimer : size[%d]", m_objQosTimers.GetSize(), 0, 0);
}

PRIVATE
void MtcPreconditionManager::CreateStatusTable(IN ISession* piSession)
{
    QosStatusTable* pStatusTable = GetQosStatusTable(piSession);
    if (pStatusTable != IMS_NULL)
    {
        IMS_TRACE_D("CreateStatusTable : already have status table.", 0, 0, 0);
        return;
    }

    IMS_TRACE_D("CreateStatusTable", 0, 0, 0);

    pStatusTable = new QosStatusTable();
    m_objStatusTables.Add(piSession, pStatusTable);
}

PRIVATE
void MtcPreconditionManager::RemoveStatusTable(IN ISession* piSession)
{
    QosStatusTable* pStatusTable = GetQosStatusTable(piSession);
    if (pStatusTable != IMS_NULL)
    {
        delete pStatusTable;
        m_objStatusTables.Remove(piSession);
    }

    IMS_TRACE_D("RemoveStatusTable : size[%d]", m_objStatusTables.GetSize(), 0, 0);
}

PRIVATE
void MtcPreconditionManager::CreateQosData(IN ISession* piSession)
{
    QosData* pData = GetQosData(piSession);
    if (pData != IMS_NULL)
    {
        IMS_TRACE_D("CreateQosData : already have QosData.", 0, 0, 0);
        return;
    }

    IMS_TRACE_D("CreateQosData", 0, 0, 0);

    pData = new QosData();
    m_objQosDatas.Add(piSession, pData);

    QosStatus eAudioStatus =
            IsDefaultBearerUsed(MEDIATYPE_AUDIO) ? QosStatus::AVAILABLE : QosStatus::IDLE;
    QosStatus eVideoStatus =
            IsDefaultBearerUsed(MEDIATYPE_VIDEO) ? QosStatus::AVAILABLE : QosStatus::IDLE;
    QosStatus eTextStatus =
            IsDefaultBearerUsed(MEDIATYPE_TEXT) ? QosStatus::AVAILABLE : QosStatus::IDLE;

    if (m_objContext.GetService().IsWlanIpCanType() &&
            !(m_objContext.GetConfigurationProxy().Is(Feature::ENABLE_FAKE_QOS_CALL_FLOW_ON_WIFI)))
    {
        IMS_TRACE_D("CreateQosData : set all statuses as available because of wifi call.", 0, 0, 0);
        eAudioStatus = QosStatus::AVAILABLE;
        eVideoStatus = QosStatus::AVAILABLE;
        eTextStatus = QosStatus::AVAILABLE;
    }

    IMS_TRACE_D("CreateQosData : audio[%s] video[%s] text[%s]",
            _TRACE_B_(IsStatusAvailable(eAudioStatus)), _TRACE_B_(IsStatusAvailable(eVideoStatus)),
            _TRACE_B_(IsStatusAvailable(eTextStatus)));

    pData->SetAudioStatus(eAudioStatus);
    pData->SetVideoStatus(eVideoStatus);
    pData->SetTextStatus(eTextStatus);
}

PRIVATE
void MtcPreconditionManager::RemoveQosData(IN ISession* piSession)
{
    QosData* pData = GetQosData(piSession);
    if (pData != IMS_NULL)
    {
        delete pData;
        m_objQosDatas.Remove(piSession);
    }

    IMS_TRACE_D("RemoveQosData : size[%d]", m_objQosDatas.GetSize(), 0, 0);
}

PRIVATE
void MtcPreconditionManager::DestroyAll()
{
    IMS_TRACE_D("DestroyAll", 0, 0, 0);

    for (IMS_UINT32 index = 0; index < m_objQosDatas.GetSize(); index++)
    {
        delete m_objQosDatas.GetValueAt(index);
    }

    for (IMS_UINT32 index = 0; index < m_objQosTimers.GetSize(); index++)
    {
        delete m_objQosTimers.GetValueAt(index);
    }

    for (IMS_UINT32 index = 0; index < m_objStatusTables.GetSize(); index++)
    {
        delete m_objStatusTables.GetValueAt(index);
    }
}

PRIVATE
QosData* MtcPreconditionManager::GetQosData(IN ISession* piSession)
{
    IMS_SLONG nIndex = m_objQosDatas.GetIndexOfKey(piSession);
    if (nIndex < 0)
    {
        IMS_TRACE_D("GetQosData : fail to get QosData.", 0, 0, 0);
        return IMS_NULL;
    }

    return m_objQosDatas.GetValueAt(nIndex);
}

PRIVATE
QosTimer* MtcPreconditionManager::GetQosTimer(IN ISession* piSession)
{
    IMS_SLONG nIndex = m_objQosTimers.GetIndexOfKey(piSession);
    if (nIndex < 0)
    {
        IMS_TRACE_D("GetQosTimer : fail to get QosTimer.", 0, 0, 0);
        return IMS_NULL;
    }

    return m_objQosTimers.GetValueAt(nIndex);
}

PRIVATE
QosStatusTable* MtcPreconditionManager::GetQosStatusTable(IN ISession* piSession)
{
    IMS_SLONG nIndex = m_objStatusTables.GetIndexOfKey(piSession);
    if (nIndex < 0)
    {
        IMS_TRACE_D("GetQosStatusTable : fail to get QosStatusTable.", 0, 0, 0);
        return IMS_NULL;
    }

    return m_objStatusTables.GetValueAt(nIndex);
}

PRIVATE
void MtcPreconditionManager::HandleReservationFailureByTimerExpiration(IN QosTimer* pTimer)
{
    ISession* piSession = IMS_NULL;
    for (IMS_UINT32 index = 0; index < m_objQosTimers.GetSize(); index++)
    {
        QosTimer* pTempTimer = m_objQosTimers.GetValueAt(index);
        if (pTempTimer == pTimer)
        {
            piSession = m_objQosTimers.GetKeyAt(index);
            break;
        }
    }

    if (piSession == IMS_NULL)
    {
        IMS_TRACE_D("HandleReservationFailureByTimerExpiration : can't find ISession", 0, 0, 0);
        return;
    }

    NotifyQosStatusToListener(piSession, IMS_FALSE, MEDIATYPE_NONE);
    InitializeStatusForLostQos(piSession);
}

PRIVATE
QosLossPolicy MtcPreconditionManager::GetActionForQosLoss(IN ISession* piSession)
{
    QosLossPolicy eAction = QosLossPolicy::MAINTAIN;
    if (piSession == IMS_NULL)
    {
        return eAction;
    }

    IMS_UINT32 eMediaTypes = SdpPreconditionHelper::GetMediaTypesBySdp(piSession);

    if ((eMediaTypes & MEDIATYPE_AUDIO) &&
            !IsStatusAvailable(GetQosStatus(piSession, MEDIATYPE_AUDIO)))
    {
        QosLossPolicy ePartialAction = GetQosLossPolicy(MEDIATYPE_AUDIO);

        if (eAction < ePartialAction)
        {
            eAction = ePartialAction;
        }
    }

    if ((eMediaTypes & MEDIATYPE_VIDEO) &&
            !IsStatusAvailable(GetQosStatus(piSession, MEDIATYPE_VIDEO)))
    {
        QosLossPolicy ePartialAction = GetQosLossPolicy(MEDIATYPE_VIDEO);

        if (eAction < ePartialAction)
        {
            eAction = ePartialAction;
        }
    }

    if ((eMediaTypes & MEDIATYPE_TEXT) &&
            !IsStatusAvailable(GetQosStatus(piSession, MEDIATYPE_TEXT)))
    {
        QosLossPolicy ePartialAction = GetQosLossPolicy(MEDIATYPE_TEXT);

        if (eAction < ePartialAction)
        {
            eAction = ePartialAction;
        }
    }

    IMS_TRACE_D("GetActionForQosLoss : The next Action is %s", PS_QosLossPolicy(eAction), 0, 0);
    return eAction;
}

PRIVATE
void MtcPreconditionManager::InitializeStatusForLostQos(IN ISession* piSession)
{
    IMS_UINT32 eMediaTypes = SdpPreconditionHelper::GetMediaTypesBySdp(piSession);

    IMS_TRACE_D("InitializeStatusForLostQos : MediaTypes[%d]", eMediaTypes, 0, 0);

    if ((eMediaTypes & MEDIATYPE_AUDIO) &&
            GetQosStatus(piSession, MEDIATYPE_AUDIO) == QosStatus::LOST)
    {
        SetQosStatus(piSession, QosStatus::IDLE, MEDIATYPE_AUDIO);
    }

    if ((eMediaTypes & MEDIATYPE_VIDEO) &&
            GetQosStatus(piSession, MEDIATYPE_VIDEO) == QosStatus::LOST)
    {
        SetQosStatus(piSession, QosStatus::IDLE, MEDIATYPE_VIDEO);
    }

    if ((eMediaTypes & MEDIATYPE_TEXT) &&
            GetQosStatus(piSession, MEDIATYPE_TEXT) == QosStatus::LOST)
    {
        SetQosStatus(piSession, QosStatus::IDLE, MEDIATYPE_TEXT);
    }
}

PRIVATE
void MtcPreconditionManager::UpdateStatusRecords(IN ISession* piSession)
{
    IMS_UINT32 eMediaTypes = SdpPreconditionHelper::GetMediaTypesBySdp(piSession);
    if (eMediaTypes == MEDIATYPE_NONE)
    {
        IMS_TRACE_D("UpdateStatusRecords : no media line to update.", 0, 0, 0);
        return;
    }

    QosStatusTable* pStatusTable = GetQosStatusTable(piSession);
    if (pStatusTable == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("UpdateStatusRecords : MediaTypes[%d]", eMediaTypes, 0, 0);

    if (eMediaTypes & MEDIATYPE_AUDIO)
    {
        IMS_SINT32 eSdpMediaType = GetSdpMediaType(MEDIATYPE_AUDIO);

        if (pStatusTable->IsStatusRecordsListEmpty(eSdpMediaType))
        {
            pStatusTable->CreateStatusRecords(eSdpMediaType);
            pStatusTable->UpdateLocalCurrentStatus(
                    eSdpMediaType, IsStatusAvailable(GetQosStatus(piSession, MEDIATYPE_AUDIO)));
        }
    }

    if (eMediaTypes & MEDIATYPE_VIDEO)
    {
        IMS_SINT32 eSdpMediaType = GetSdpMediaType(MEDIATYPE_VIDEO);

        if (pStatusTable->IsStatusRecordsListEmpty(eSdpMediaType))
        {
            pStatusTable->CreateStatusRecords(eSdpMediaType);
            pStatusTable->UpdateLocalCurrentStatus(
                    eSdpMediaType, IsStatusAvailable(GetQosStatus(piSession, MEDIATYPE_VIDEO)));
        }
    }

    if (eMediaTypes & MEDIATYPE_TEXT)
    {
        IMS_SINT32 eSdpMediaType = GetSdpMediaType(MEDIATYPE_TEXT);

        if (pStatusTable->IsStatusRecordsListEmpty(eSdpMediaType))
        {
            pStatusTable->CreateStatusRecords(eSdpMediaType);
            pStatusTable->UpdateLocalCurrentStatus(
                    eSdpMediaType, IsStatusAvailable(GetQosStatus(piSession, MEDIATYPE_TEXT)));
        }
    }

    pStatusTable->RemoveUnusedStatusRecords(eMediaTypes);
}

PRIVATE
void MtcPreconditionManager::HandleQosTimer(
        IN ISession* piSession, IN QosStatus eCurrStatus, IN QosStatus eNewStatus)
{
    IMS_TRACE_D("HandleQosTimer", 0, 0, 0);

    if (eCurrStatus == QosStatus::IDLE && eNewStatus == QosStatus::AVAILABLE)
    {
        StopQosTimer(piSession);
        StopQosTimer(piSession, QosTimerType::FORCE_AVAILABLE);
    }
    else if (eCurrStatus == QosStatus::AVAILABLE && eNewStatus == QosStatus::LOST)
    {
        StartQosTimer(piSession, QosTimerType::GUARD_INACTIVE);
    }
    else if (eCurrStatus == QosStatus::LOST && eNewStatus == QosStatus::AVAILABLE)
    {
        StopQosTimer(piSession, QosTimerType::GUARD_INACTIVE);
    }
}

PRIVATE
void MtcPreconditionManager::NotifyQosStatusToListener(
        IN ISession* piSession, IN IMS_BOOL bReserved, IN IMS_UINT32 eMediaTypes)
{
    if (bReserved)
    {
        m_pListener->QosReserved(piSession, eMediaTypes);
    }
    else
    {
        QosLossPolicy eAction = GetActionForQosLoss(piSession);

        if (eAction != QosLossPolicy::MAINTAIN)
        {
            m_pListener->QosReserveFailed(piSession, eAction);
        }
    }
}

PRIVATE
IMS_BOOL MtcPreconditionManager::IsStatusAvailable(IN QosStatus eStatus)
{
    return (eStatus == QosStatus::AVAILABLE) ? IMS_TRUE : IMS_FALSE;
}

PRIVATE
IMediaDescriptor* MtcPreconditionManager::GetMediaDescriptor(IN IMedia* piMedia)
{
    if (piMedia == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (piMedia->GetUpdateState() == IMedia::UPDATE_MODIFIED)
    {
        return piMedia->GetProposal()->GetMediaDescriptor();
    }
    else
    {
        return piMedia->GetMediaDescriptor();
    }
}

PRIVATE
const SdpMedia* MtcPreconditionManager::GetSdpMedia(IN IMedia* piMedia, IN IMS_BOOL bRemote)
{
    IMediaDescriptor* piMediaDescriptor = GetMediaDescriptor(piMedia);
    if (piMediaDescriptor == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (!bRemote)
    {
        if (piMedia->GetState() == IMedia::STATE_DELETED)
        {
            return IMS_NULL;
        }
        else
        {
            return piMediaDescriptor->GetMediaDescriptionExAsLocal();
        }
    }

    return piMediaDescriptor->GetMediaDescriptionEx();
}

PRIVATE
IMS_SINT32 MtcPreconditionManager::GetSdpMediaType(IN IMS_UINT32 eMediaType)
{
    IMS_SINT32 eSdpMediaType = SdpMedia::TYPE_INVALID;

    switch (eMediaType)
    {
        case MEDIATYPE_AUDIO:
            eSdpMediaType = SdpMedia::TYPE_AUDIO;
            break;
        case MEDIATYPE_VIDEO:
            eSdpMediaType = SdpMedia::TYPE_VIDEO;
            break;
        case MEDIATYPE_TEXT:
            eSdpMediaType = SdpMedia::TYPE_TEXT;
            break;
        default:
            break;
    }

    return eSdpMediaType;
}

PRIVATE
IMS_UINT32 MtcPreconditionManager::GetMediaTypesFromCallType()
{
    IMS_UINT32 eMediaTypes = MEDIATYPE_NONE;
    CallType eCallType = m_objContext.GetCallInfo().eCallType;

    if (eCallType == CallType::UNKNOWN)
    {
        IMS_TRACE_D("GetMediaTypesFromCallType : call type is unknown", 0, 0, 0);
        return eMediaTypes;
    }

    eMediaTypes |= MEDIATYPE_AUDIO;

    if (eCallType == CallType::VT)
    {
        eMediaTypes |= MEDIATYPE_VIDEO;
    }
    else if (eCallType == CallType::RTT)
    {
        eMediaTypes |= MEDIATYPE_TEXT;
    }
    else if (eCallType == CallType::VIDEO_RTT)
    {
        eMediaTypes |= MEDIATYPE_VIDEO;
        eMediaTypes |= MEDIATYPE_TEXT;
    }

    IMS_TRACE_D("GetMediaTypesFromCallType : %d", eMediaTypes, 0, 0);

    return eMediaTypes;
}

PRIVATE
IMS_BOOL MtcPreconditionManager::IsDefaultBearerUsed(IN IMS_UINT32 eMediaType)
{
    IMS_BOOL bUseDefaultBearer = IMS_FALSE;
    if (eMediaType == MEDIATYPE_AUDIO)
    {
        bUseDefaultBearer =
                !m_objContext.GetConfigurationProxy().Is(
                        Feature::DEFAULT_EPS_BEARER_CONTEXT_USAGE_RESTRICTION_ON_CELLULAR) ||
                m_objContext.GetConfigurationProxy().Is(Feature::VOICE_ON_DEFAULT_BEARER_SUPPORTED);
    }
    else if (eMediaType == MEDIATYPE_VIDEO)
    {
        bUseDefaultBearer =
                m_objContext.GetConfigurationProxy().Is(Feature::VIDEO_ON_DEFAULT_BEARER_SUPPORTED);
    }
    else if (eMediaType == MEDIATYPE_TEXT)
    {
        bUseDefaultBearer =
                m_objContext.GetConfigurationProxy().Is(Feature::TEXT_ON_DEFAULT_BEARER_SUPPORTED);
    }

    return bUseDefaultBearer;
}

PRIVATE
void MtcPreconditionManager::InitializeCapability(IN ISession* piSession)
{
    if (m_objCapabilities.GetIndexOfKey(piSession) >= 0)
    {
        return;
    }

    IMS_BOOL bLocalCapability = IsPreconditionSupportedInLocal();
    IMS_TRACE_D("InitializeCapability : %s", _TRACE_B_(bLocalCapability), 0, 0);

    m_objCapabilities.Add(piSession, bLocalCapability);
}

PRIVATE
QosLossPolicy MtcPreconditionManager::GetQosLossPolicy(IN IMS_UINT32 eMediaType)
{
    IMS_SINT32 nPolicy = -1;
    if (eMediaType == MEDIATYPE_AUDIO)
    {
        nPolicy = m_objContext.GetConfigurationProxy().GetInt(
                Feature::POLICY_ON_AUDIO_QOS_DEACTIVATION);
    }
    else if (eMediaType == MEDIATYPE_VIDEO)
    {
        nPolicy = m_objContext.GetConfigurationProxy().GetInt(
                Feature::POLICY_ON_VIDEO_QOS_DEACTIVATION);
    }
    else if (eMediaType == MEDIATYPE_TEXT)
    {
        nPolicy = m_objContext.GetConfigurationProxy().GetInt(
                Feature::POLICY_ON_TEXT_QOS_DEACTIVATION);
    }

    switch (nPolicy)
    {
        case 0:  // POLICY_TERMINATE_CALL
            return QosLossPolicy::RELEASE;
        case 1:  // POLICY_MAINTAIN_CALL
            return QosLossPolicy::MAINTAIN;
        case 2:  // POLICY_MODIFY_CALL
            return QosLossPolicy::MODIFY;
        default:
            break;
    }

    return QosLossPolicy::MAINTAIN;
}

PRIVATE
QosStatus MtcPreconditionManager::GetQosStatus(IN ISession* piSession, IN IMS_UINT32 eMediaType)
{
    QosData* pData = GetQosData(piSession);
    QosStatus eStatus = QosStatus::IDLE;

    if (pData == IMS_NULL)
    {
        return eStatus;
    }

    if (eMediaType == MEDIATYPE_AUDIO)
    {
        eStatus = pData->GetAudioStatus();
    }
    else if (eMediaType == MEDIATYPE_VIDEO)
    {
        eStatus = pData->GetVideoStatus();
    }
    else if (eMediaType == MEDIATYPE_TEXT)
    {
        eStatus = pData->GetTextStatus();
    }

    return eStatus;
}

PRIVATE
void MtcPreconditionManager::SetQosStatus(
        IN ISession* piSession, QosStatus eStatus, IN IMS_UINT32 eMediaType)
{
    QosData* pData = GetQosData(piSession);
    if (pData == IMS_NULL)
    {
        return;
    }

    if (!IsNeedToUpdateQosStatus(GetQosStatus(piSession, eMediaType), eStatus))
    {
        IMS_TRACE_D("SetQosStatus : nothing to update", 0, 0, 0);
        return;
    }

    if (eMediaType == MEDIATYPE_AUDIO)
    {
        pData->SetAudioStatus(eStatus);
    }
    else if (eMediaType == MEDIATYPE_VIDEO)
    {
        pData->SetVideoStatus(eStatus);
    }
    else if (eMediaType == MEDIATYPE_TEXT)
    {
        pData->SetTextStatus(eStatus);
    }
}

PRIVATE
IMS_BOOL MtcPreconditionManager::IsNeedToUpdateQosStatus(
        IN QosStatus eCurrStatus, IN QosStatus eNewStatus)
{
    if ((eCurrStatus == QosStatus::IDLE && eNewStatus == QosStatus::AVAILABLE) ||
            (eCurrStatus == QosStatus::AVAILABLE && eNewStatus == QosStatus::LOST) ||
            (eCurrStatus == QosStatus::LOST && eNewStatus != QosStatus::LOST))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL MtcPreconditionManager::IsPreconditionSupportedInLocal(IN IMS_UINT32 eMediaType)
{
    IMS_BOOL bSupport = IMS_FALSE;

    if (eMediaType == MEDIATYPE_AUDIO)
    {
        bSupport =
                m_objContext.GetConfigurationProxy().Is(Feature::VOICE_QOS_PRECONDITION_SUPPORTED);
    }
    else if (eMediaType == MEDIATYPE_VIDEO)
    {
        bSupport =
                m_objContext.GetConfigurationProxy().Is(Feature::VIDEO_QOS_PRECONDITION_SUPPORTED);
    }
    else if (eMediaType == MEDIATYPE_TEXT)
    {
        bSupport =
                m_objContext.GetConfigurationProxy().Is(Feature::TEXT_QOS_PRECONDITION_SUPPORTED);
    }

    IMS_TRACE_D("IsPreconditionSupportedInLocal : MediaType[%d] Result[%s]", eMediaType,
            _TRACE_B_(bSupport), 0);

    return bSupport;
}

PRIVATE
IMS_BOOL MtcPreconditionManager::IsConfirmedDialog(IN const ISession* piSession)
{
    if (piSession && piSession->GetState() >= ISession::STATE_ESTABLISHED)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}
