#include "ServiceTrace.h"
#include "ISession.h"
#include "IReference.h"
#include "helper/sipinterfaceholder/ReferenceInterfaceHolder.h"
#include "helper/sipinterfaceholder/IInterfaceHolderListener.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
ReferenceInterfaceHolder::ReferenceInterfaceHolder(IN IInterfaceHolderListener& objListener) :
        m_objListener(objListener)
{
    IMS_TRACE_D("+ReferenceInterfaceHolder", 0, 0, 0);
}

PUBLIC
ReferenceInterfaceHolder::~ReferenceInterfaceHolder()
{
    IMS_TRACE_D("~ReferenceInterfaceHolder", 0, 0, 0);

    ClearIReferences();

    for (IMS_SINT32 i = m_objReferenceTerminatedGuardTimers.GetSize() - 1; i >= 0; i--)
    {
        StopTimer(m_objReferenceTerminatedGuardTimers.GetKeyAt(i));
    }
    m_objReferenceTerminatedGuardTimers.Clear();
}

PUBLIC VIRTUAL void ReferenceInterfaceHolder::ReferenceTerminated(IN IReference* piReference)
{
    IMS_TRACE_D("ReferenceTerminated", 0, 0, 0);

    // TODO: delete piReference / timer???
    ReleaseIReference(piReference, IMS_TRUE);
}

PUBLIC VIRTUAL void ReferenceInterfaceHolder::Timer_TimerExpired(IN ITimer* piTimer)
{
    IMS_TRACE_D("Timer_TimerExpired", 0, 0, 0);

    IMS_SLONG nIndex = m_objReferenceTerminatedGuardTimers.GetIndexOfKey(piTimer);
    if (nIndex < 0)
    {
        return;
    }

    IReference* piReference = m_objReferenceTerminatedGuardTimers.GetValueAt(nIndex);

    if (piReference != IMS_NULL)
    {
        StopTimer(piTimer);
        ReleaseIReference(piReference, IMS_TRUE);
    }
}

PUBLIC
IReference* ReferenceInterfaceHolder::GetIReference(
        IN ISession* piSession, IN const AString& strReferTo, IN const AString& strMethod)
{
    IMS_TRACE_D("GetIReference", 0, 0, 0);

    IReference* piReference = piSession->CreateReference(strReferTo, strMethod);
    m_objIReferences.Append(piReference);
    return piReference;
}

PUBLIC
void ReferenceInterfaceHolder::ReleaseIReference(
        IN IReference* piReference, IN IMS_BOOL bTerminated /* = IMS_FALSE*/)
{
    IMS_TRACE_D("ReleaseIReference", 0, 0, 0);

    if (piReference == IMS_NULL)
    {
        return;
    }

    piReference->SetListener(this);
    if (bTerminated || IsReadyToDestroy(piReference))
    {
        for (IMS_UINT32 i = 0; i < m_objIReferences.GetSize(); i++)
        {
            if (m_objIReferences.GetAt(i) == piReference)
            {
                ITimer* piTempTimer = GetTimer(piReference);
                StopTimer(piTempTimer);

                piReference->Destroy();
                m_objIReferences.RemoveAt(i);
                IMS_TRACE_D("ReleaseIReference remove index=[%d]", i, 0, 0);

                if (m_objIReferences.GetSize() == 0)
                {
                    m_objListener.OnReferenceInterfaceCleared();
                }
                break;
            }
        }
    }
    else
    {
        StartTimer(piReference, TIME_TRANSACTION_TERMINATED_GUARD);
    }
}

#ifdef _PRIVATE_METHOD_
#endif

PRIVATE
IMS_BOOL ReferenceInterfaceHolder::IsReadyToDestroy(IN IReference* piReference)
{
    IMS_TRACE_D("IsReadyToDestroy [%d]", piReference->GetState(), 0, 0);

    if (piReference->GetState() == IReference::STATE_TERMINATED)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
void ReferenceInterfaceHolder::ClearIReferences()
{
    for (IMS_UINT32 i = 0; i < m_objIReferences.GetSize(); i++)
    {
        IReference* piReference = m_objIReferences.GetAt(i);
        if (piReference != IMS_NULL)
        {
            piReference->Destroy();
        }
    }

    m_objIReferences.Clear();
}

PRIVATE
IMS_RESULT ReferenceInterfaceHolder::StartTimer(IN IReference* piReference, IN IMS_SINT32 nDuration)
{
    IMS_TRACE_D("StartTimer reference duration[%d]", nDuration, 0, 0);

    if (nDuration <= 0)
    {
        return IMS_FAILURE;
    }

    ITimer* piTempTimer = GetTimer(piReference);
    if (piTempTimer != IMS_NULL)
    {
        return IMS_FAILURE;
    }

    piTempTimer = TimerService::GetTimerService()->CreateTimer();
    m_objReferenceTerminatedGuardTimers.Add(piTempTimer, piReference);
    piTempTimer->SetTimer(nDuration, this);

    return IMS_SUCCESS;
}

PRIVATE
void ReferenceInterfaceHolder::StopTimer(IN ITimer* piTimer)
{
    IMS_TRACE_D("StopTimer", 0, 0, 0);

    if (piTimer == IMS_NULL)
    {
        return;
    }

    m_objReferenceTerminatedGuardTimers.Remove(piTimer);

    piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(piTimer);
}

PRIVATE
ITimer* ReferenceInterfaceHolder::GetTimer(IN IReference* piReference)
{
    IMS_UINT32 nSize = m_objReferenceTerminatedGuardTimers.GetSize();
    IMS_TRACE_D("GetTimer reference size = [%d]", nSize, 0, 0);

    for (IMS_UINT32 i = 0; i < nSize; i++)
    {
        if (piReference == m_objReferenceTerminatedGuardTimers.GetValueAt(i))
        {
            IMS_TRACE_D("GetTimer reference exists.", 0, 0, 0);
            return m_objReferenceTerminatedGuardTimers.GetKeyAt(i);
        }
    }

    return IMS_NULL;
}
