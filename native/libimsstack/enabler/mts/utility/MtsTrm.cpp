#include "ServiceTrace.h"
#include "ServiceTimer.h"
#include "ServicePhoneInfo.h"
#include "utility/MtsTrm.h"
#include "utility/MtsUtils.h"
#include "MtsApp.h"

__IMS_TRACE_TAG_COM_SMS__;

static MtsTrm* s_pMtsTrm[] = {IMS_NULL, IMS_NULL};

PUBLIC
MtsTrm::MtsTrm(IN IMS_SINT32 nSlotId) :
        m_nSlotId(nSlotId),
        m_bIsTrmSet(IMS_FALSE),
        m_piTrm(IMS_NULL),
        m_piMtsTrmTimer(IMS_NULL),
        m_objTrmListeners(IMSList<MtsTrmListener*>())
{
    IMS_TRACE_MEM("MTSTRM_MEM", "Create : [SLOT%d] MtsTrm = %" PFLS_u "/%" PFLS_x, nSlotId,
            sizeof(MtsTrm), this);

    m_piTrm = PhoneInfoService::GetPhoneInfoService()->GetTrm();
    if (m_piTrm != IMS_NULL)
    {
        if (m_piTrm->IsTrmSupported())
        {
            m_piTrm->RegisterObserver(this);
            m_piTrm->Enable(m_nSlotId);
        }
        else
        {
            m_piTrm = IMS_NULL;
        }
    }
}

PUBLIC
MtsTrm::~MtsTrm()
{
    IMS_TRACE_MEM("MTSTRM_MEM", "Delete : [SLOT%d] MtsTrm = %" PFLS_u "/%" PFLS_x, m_nSlotId,
            sizeof(MtsTrm), this);

    if (m_piTrm != IMS_NULL)
    {
        m_piTrm->RemoveObserver(this);
        m_piTrm->Disable(m_nSlotId);
        m_piTrm = IMS_NULL;
    }

    m_objTrmListeners.Clear();
    StopTimer();
}

PUBLIC GLOBAL MtsTrm* MtsTrm::GetInstance(IN IMS_SINT32 nSlotId)
{
    if (nSlotId < 0 || nSlotId > 1)
    {
        IMS_TRACE_E(0, "MtsTrm::GetInstance() Error! Invalid SlotId:%d", nSlotId, 0, 0);
        nSlotId = 0;
    }

    IMS_TRACE_D("MtsTrm::GetInstance(): slot[%d]", nSlotId, 0, 0);
    if (s_pMtsTrm[nSlotId] == IMS_NULL)
    {
        IMS_TRACE_D("MtsTrm::GetInstance: Generate New MtsTrm slot[%d]", nSlotId, 0, 0);
        s_pMtsTrm[nSlotId] = new MtsTrm(nSlotId);
        IMS_TRACE_D("GetInstance: MtsTrm[%d]: %d", nSlotId, s_pMtsTrm[nSlotId], 0);
    }
    return s_pMtsTrm[nSlotId];
}

void MtsTrm::DestroyMtsTrm(IN IMS_SINT32 nSlotId)
{
    if (s_pMtsTrm[nSlotId] != IMS_NULL)
    {
        IMS_TRACE_D("DestroyMtsTrm: MtsTrm[%d]: %d", nSlotId, s_pMtsTrm[nSlotId], 0);
        delete s_pMtsTrm[nSlotId];
        s_pMtsTrm[nSlotId] = IMS_NULL;
    }
}

PUBLIC
void MtsTrm::AddListener(IN MtsTrmListener* piListener)
{
    for (IMS_UINT32 i = 0; i < m_objTrmListeners.GetSize(); ++i)
    {
        MtsTrmListener* piTmpListener = m_objTrmListeners.GetAt(i);

        if (piTmpListener == piListener)
        {
            IMS_TRACE_I("[MTSTRM] AddListener :: (%" PFLS_x ") is already added", piListener, 0, 0);
            return;
        }
    }

    m_objTrmListeners.Append(piListener);

    IMS_TRACE_I("[MTSTRM] AddListener :: (%" PFLS_x ") is added", piListener, 0, 0);
}

PUBLIC
void MtsTrm::RemoveListener(IN MtsTrmListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objTrmListeners.GetSize(); ++i)
    {
        MtsTrmListener* piTmpListener = m_objTrmListeners.GetAt(i);

        if (piTmpListener == piListener)
        {
            m_objTrmListeners.RemoveAt(i);

            IMS_TRACE_I("[MTSTRM] RemoveListener :: (%" PFLS_x ") is removed", piListener, 0, 0);
            return;
        }
    }
}

PUBLIC
IMS_BOOL MtsTrm::IsReady()
{
    if (m_piTrm == IMS_NULL)
    {
        return IMS_TRUE;
    }

    if (m_bIsTrmSet)
    {
        return IMS_TRUE;
    }

    return m_piTrm->IsServiceAvailable(m_nSlotId, ITrm::SERVICE_SMS);
}

PUBLIC
IMS_BOOL MtsTrm::IsTRMSupported()
{
    if (m_piTrm == IMS_NULL)
    {
        return IMS_FALSE;
    }
    return m_piTrm->IsTrmSupported();
}

PUBLIC
void MtsTrm::Set(IN IMS_BOOL bStart)
{
    IMS_TRACE_I("[MTSTRM] Set ::  start (%d) , slotid(%d) ", bStart, m_nSlotId, 0);

    if (m_piTrm == IMS_NULL)
    {
        return;
    }

    if (bStart)
    {
        if (m_bIsTrmSet)
        {
            StopTimer();
        }
        else
        {
            if (m_piTrm->SetService(m_nSlotId, ITrm::SERVICE_SMS, ITrm::MODE_START))
            {
                m_bIsTrmSet = IMS_TRUE;
            }
        }
    }
    else
    {
        if (m_bIsTrmSet)
        {
            StartTimer(MTS_TRM_TIME_STOP_DELAY);
        }
    }
}

PRIVATE
void MtsTrm::StartTimer(IN IMS_UINT32 nDuration)
{
    if (m_piMtsTrmTimer != IMS_NULL)
    {
        StopTimer();
    }

    m_piMtsTrmTimer =
            MtsUtils::GetInstance()->StartTimer(nDuration, this, "TIMER_MTS_TRM_TIME_STOP_DELAY");
}

PRIVATE
void MtsTrm::StopTimer()
{
    if (m_piMtsTrmTimer == IMS_NULL)
    {
        return;
    }

    MtsUtils::GetInstance()->StopTimer(m_piMtsTrmTimer, "TIMER_MTS_TRM_TIME_STOP_DELAY");
}

PRIVATE
void MtsTrm::ProcessTimerExpired()
{
    if (m_bIsTrmSet)
    {
        IMS_TRACE_I("ProcessTimerExpired :: set end to trm", 0, 0, 0);
        if (m_piTrm->SetService(m_nSlotId, ITrm::SERVICE_SMS, ITrm::MODE_END))
        {
            m_bIsTrmSet = IMS_FALSE;
        }
    }
}

PRIVATE
void MtsTrm::Trm_NotifyServicePriorityChanged()
{
    if (m_piTrm == IMS_NULL)
    {
        IMS_TRACE_E(0, "NotifyServicePriorityChanged : m_piTrm is NULL", 0, 0, 0);
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objTrmListeners.GetSize(); ++i)
    {
        MtsTrmListener* piListener = m_objTrmListeners.GetAt(i);

        if (piListener == IMS_NULL)
        {
            continue;
        }

        piListener->Trm_PriorityChanged();
    }
}

PRIVATE
void MtsTrm::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (piTimer != m_piMtsTrmTimer)
    {
        return;
    }

    StopTimer();

    ProcessTimerExpired();
}
