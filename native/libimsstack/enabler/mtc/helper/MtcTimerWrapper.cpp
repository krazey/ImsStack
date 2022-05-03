#include "ServiceTimer.h"
#include "ServiceTrace.h"

#include "helper/IMtcTimerListener.h"
#include "helper/MtcTimerWrapper.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcTimerWrapper::MtcTimerWrapper() :
        m_lstTimers(IMSList<MtcTimer*>()),
        m_piListener(IMS_NULL)
{
}

PUBLIC VIRTUAL MtcTimerWrapper::~MtcTimerWrapper()
{
    StopAll();
}

PUBLIC VIRTUAL void MtcTimerWrapper::Timer_TimerExpired(IN ITimer* piTimer)
{
    for (IMS_UINT32 i = 0; i < m_lstTimers.GetSize(); i++)
    {
        MtcTimer* pTimer = m_lstTimers.GetAt(i);
        if (pTimer->piTimer == piTimer)
        {
            m_piListener->OnTimerExpired(pTimer->eType);
            break;
        }
    }
}

PUBLIC
void MtcTimerWrapper::SetListener(IN IMtcTimerListener* piListener)
{
    m_piListener = piListener;
}

PUBLIC
void MtcTimerWrapper::Start(IN IMS_UINT32 eType, IN IMS_SINT32 nDuration)
{
    if (IsActive(eType))
    {
        IMS_TRACE_E(0, "[type:%d] timer is already running", eType, 0, 0);
        return;
    }

    MtcTimer* pTimer = new MtcTimer();

    pTimer->eType = eType;
    pTimer->nDuration = nDuration;
    pTimer->piTimer = TimerService::GetTimerService()->CreateTimer();
    pTimer->nId = pTimer->piTimer->SetTimer(nDuration, this);
    m_lstTimers.Append(pTimer);

    IMS_TRACE_I("Start : ID[%" PFLS_u "] Type[%d] Duration[%d]", pTimer->nId, eType, nDuration);
}

PUBLIC
void MtcTimerWrapper::Stop(IN IMS_UINT32 eType)
{
    for (IMS_UINT32 i = 0; i < m_lstTimers.GetSize(); i++)
    {
        MtcTimer* pTimer = m_lstTimers.GetAt(i);

        if (eType != pTimer->eType)
        {
            continue;
        }

        IMS_TRACE_I("Stop : Type[%d]", eType, 0, 0);

        pTimer->piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(pTimer->piTimer);
        delete pTimer;
        m_lstTimers.RemoveAt(i);

        return;
    }

    IMS_TRACE_I("Stop : Not Found Type[%d]", eType, 0, 0);
}

PUBLIC
void MtcTimerWrapper::StopAll()
{
    IMS_UINT32 nSize = m_lstTimers.GetSize();

    IMS_TRACE_I("AllStop : Size[%d]", nSize, 0, 0);

    for (IMS_UINT32 i = 0; i < nSize; i++)
    {
        MtcTimer* pTimer = m_lstTimers.GetAt(i);
        pTimer->piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(pTimer->piTimer);
        pTimer->piTimer = IMS_NULL;
        pTimer->nId = 0;
        delete pTimer;
    }

    m_lstTimers.Clear();
}

PUBLIC
IMS_BOOL MtcTimerWrapper::IsActive(IN IMS_UINT32 eType)
{
    for (IMS_UINT32 i = 0; i < m_lstTimers.GetSize(); i++)
    {
        if (eType == m_lstTimers.GetAt(i)->eType)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}
