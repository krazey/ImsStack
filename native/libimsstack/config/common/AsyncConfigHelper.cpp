/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20101022  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "IAsyncConfig.h"
#include "AsyncConfigHelper.h"

__IMS_TRACE_TAG_CONF__;

class AsyncAction
{
public:
    inline AsyncAction(IN IAsyncConfig* piConfig_, IN IMS_SINT32 nMSG_, IN IMS_SINTP nParam1_,
            IN IMS_SINTP nParam2_) :
            piConfig(piConfig_),
            nMSG(nMSG_),
            nParam1(nParam1_),
            nParam2(nParam2_)
    {
    }

    inline ~AsyncAction() {}

public:
    IAsyncConfig* piConfig;
    IMS_SINT32 nMSG;
    IMS_SINTP nParam1;
    IMS_SINTP nParam2;
};

PUBLIC
AsyncConfigHelper::AsyncConfigHelper() :
        IMSActivityEx(),
        objAsyncConfigs(IMSList<IAsyncConfig*>())
{
}

PUBLIC VIRTUAL AsyncConfigHelper::~AsyncConfigHelper()
{
    objAsyncConfigs.Clear();
}

PUBLIC
void AsyncConfigHelper::Register(IN IAsyncConfig* piConfig)
{
    if (IsRegisteredConfig(piConfig))
    {
        return;
    }

    objAsyncConfigs.Append(piConfig);
}

PUBLIC
IMS_BOOL AsyncConfigHelper::SendTo(
        IN IAsyncConfig* piConfig, IN IMS_SINT32 nMSG, IN IMS_SINTP nParam1, IN IMS_SINTP nParam2)
{
    if (!IsRegisteredConfig(piConfig))
    {
        IMS_TRACE_D("AsyncConfig is not registered ...", 0, 0, 0);
        return IMS_FALSE;
    }

    AsyncAction* pAction = new AsyncAction(piConfig, nMSG, nParam1, nParam2);

    if (pAction == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!PostMessage(AMSG_SEND_TO, 0, reinterpret_cast<IMS_UINTP>(pAction)))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
void AsyncConfigHelper::Unregister(IN IAsyncConfig* piConfig)
{
    for (IMS_UINT32 i = 0; i < objAsyncConfigs.GetSize(); ++i)
    {
        IAsyncConfig* piTmpConfig = objAsyncConfigs.GetAt(i);

        if (piTmpConfig == piConfig)
        {
            objAsyncConfigs.RemoveAt(i);
            break;
        }
    }
}

PRIVATE VIRTUAL IMS_BOOL AsyncConfigHelper::OnMessage(IN IMSMSG& objMSG)
{
    switch (objMSG.GetName())
    {
        case AMSG_SEND_TO:
        {
            AsyncAction* pAction = reinterpret_cast<AsyncAction*>(objMSG.nLparam);

            if (pAction == IMS_NULL)
            {
                IMS_TRACE_D("No action in the message", 0, 0, 0);
                break;
            }

            if (!IsRegisteredConfig(pAction->piConfig))
            {
                IMS_TRACE_D("AsyncConfig is not registered, so message(%d) is dropped",
                        pAction->nMSG, 0, 0);

                delete pAction;
                break;
            }

            pAction->piConfig->HandleMessage(pAction->nMSG, pAction->nParam1, pAction->nParam2);

            delete pAction;
            break;
        }

        default:
            // no-op
            break;
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL AsyncConfigHelper::IsRegisteredConfig(IN IAsyncConfig* piConfig)
{
    for (IMS_UINT32 i = 0; i < objAsyncConfigs.GetSize(); ++i)
    {
        IAsyncConfig* piTmpConfig = objAsyncConfigs.GetAt(i);

        if (piTmpConfig == piConfig)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}
