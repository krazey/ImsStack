/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20170801  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceThread.h"
#include "ImsMessageDef.h"
#include "BaseThread.h"

__IMS_TRACE_TAG_BASE__;

PUBLIC
BaseThread::BaseThread() :
        strName(AString::ConstNull()),
        piThread(IMS_NULL)
{
}

PUBLIC VIRTUAL BaseThread::~BaseThread()
{
    Terminate();
}

PUBLIC
IMS_BOOL BaseThread::Start(IN const AString& strName, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/)
{
    if (piThread != IMS_NULL)
    {
        IMS_TRACE_I("Start :: Thread(%s) is already running ...", strName.GetStr(), 0, 0);
        return IMS_TRUE;
    }

    this->strName = strName;

    piThread = ThreadService::GetThreadService()->Create(strName, nSlotId);

    if (piThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "Start :: Creating a thread(%s) failed ...", strName.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    piThread->SetRunnable(this);

    if (!piThread->Activate())
    {
        IMS_TRACE_E(0, "Start :: Starting a thread(%s) failed ...", strName.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
void BaseThread::Terminate()
{
    if (piThread == IMS_NULL)
    {
        return;
    }

    // Method call will not be returned util the current thread is exited in platform layer.
    piThread->Deactivate();

    ThreadService::GetThreadService()->Destroy(piThread);
    piThread = IMS_NULL;
}

PROTECTED VIRTUAL IMS_BOOL BaseThread::Runnable_Run(IN IMSMSG& objMSG)
{
    switch (objMSG.GetName())
    {
        case IMS_MSG_START:
            if (!Initialize())
            {
                return IMS_FALSE;
            }

            return OnStart(objMSG);

        case IMS_MSG_TERMINATE:
            OnTerminate(objMSG);
            Uninitialize();
            return IMS_TRUE;

        default:
            break;
    }

    if (!IsThreadMessage(objMSG))
    {
        IMS_TRACE_D("Message(%d) is not for thread(%s)", objMSG.GetName(), GetName().GetStr(), 0);
        return IMS_FALSE;
    }

    return OnMessage(objMSG);
}

PROTECTED VIRTUAL IMS_BOOL BaseThread::Initialize()
{
    // no-op
    return IMS_TRUE;
}

PROTECTED VIRTUAL void BaseThread::Uninitialize()
{
    // no-op
}

PROTECTED VIRTUAL IMS_BOOL BaseThread::OnStart(IN IMSMSG& /* objMSG */)
{
    // no-op
    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL BaseThread::OnTerminate(IN IMSMSG& /* objMSG */)
{
    // no-op
    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL BaseThread::OnMessage(IN IMSMSG& /* objMSG */)
{
    // no-op
    return IMS_FALSE;
}

PROTECTED
IMS_BOOL BaseThread::IsThreadMessage(IN IMSMSG& objMSG) const
{
    const IMS_CHAR* pszTargetName = objMSG.GetTargetName();

    if ((pszTargetName != IMS_NULL) && !strName.Equals(pszTargetName))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}
