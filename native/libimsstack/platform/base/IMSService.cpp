/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100324  joonhun.shin@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "IMSService.h"

#if 0  // public
#endif

PUBLIC
IMSService::IMSService(IN CONST AString& strName) :
        IMSActivity(strName),
        nState(IMS_INVALID_STATE),
        nOldState(IMS_INVALID_STATE)
{
}

PUBLIC VIRTUAL IMSService::~IMSService() {}

#if 0  // pretected
#endif

EMPTY_STATE_MAP(IMSService)

PROTECTED VIRTUAL IMS_BOOL IMSService::OnPreprocess(IN IMSMSG& /* objMSG */)
{
    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL IMSService::OnMessage(IN IMSMSG& /* objMSG */)
{
    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL IMSService::OnPostprocess(IN IMSMSG& /* objMSG */)
{
    return IMS_FALSE;
}

PROTECTED VIRTUAL IIMSActivityControl* IMSService::GetController()
{
    return IMS_NULL;
}

PROTECTED
IMS_BOOL IMSService::SetState(IN IMS_UINT32 nState)
{
    nOldState = this->nState;
    this->nState = nState;

    return IMS_TRUE;
}

PROTECTED
IMS_UINT32 IMSService::GetState()
{
    return nState;
}

PROTECTED
IMS_UINT32 IMSService::GetOldState()
{
    return nOldState;
}

#if 0  // private
#endif

PRIVATE VIRTUAL IMS_BOOL IMSService::DispatchMessage(IN IMSMSG& objMSG)
{
    IMS_BOOL bRetValue = IMS_FALSE;

    (void)OnPreprocess(objMSG);

    if (!OnStateMsgProcess(objMSG))
    {
        bRetValue = OnMessage(objMSG);
    }

    (void)OnPostprocess(objMSG);

    return bRetValue;
}

PRIVATE
IMS_BOOL IMSService::OnStateMsgProcess(IN IMSMSG& objMSG)
{
    const StateMap* pstStateMap = GetStateMap();
    IMS_UINT32 nStateIndex = 0;
    IMS_UINT32 nMsgIndex = 0;
    IMS_BOOL bStateFound = IMS_FALSE;

    while (pstStateMap[nStateIndex].nState != IMS_INVALID_STATE)
    {
        if (pstStateMap[nStateIndex].nState == nState)
        {
            bStateFound = IMS_TRUE;
            break;
        }
        nStateIndex++;
    }

    if (bStateFound == IMS_TRUE)
    {
        const StateMsgMap* pstStateMsgMap = (pstStateMap[nStateIndex].pfnGetStateMsgMap)();
        if (pstStateMsgMap != IMS_NULL)
        {
            while (pstStateMsgMap[nMsgIndex].nMsg != IMS_INVALID_MSG)
            {
                if (pstStateMsgMap[nMsgIndex].nMsg == static_cast<IMS_UINT32>(objMSG.GetName()))
                {
                    if (pstStateMsgMap[nMsgIndex].pfnStateMsgHandler != IMS_NULL)
                    {
                        return (this->*(pstStateMsgMap[nMsgIndex].pfnStateMsgHandler))(objMSG);
                    }
                }
                nMsgIndex++;
            }
        }
    }

    return IMS_FALSE;
}
