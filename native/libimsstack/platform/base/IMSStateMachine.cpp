/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100331  joonhun.shin@             create (hwangoo.park@ base code)
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "IMSStateMachine.h"

#if 0  // public
#endif

PUBLIC
IMSStateMachine::IMSStateMachine(IN IMS_UINT32 nState_) :
        nState(nState_),
        nOldState(IMS_INVALID_STATE)
{
}

PUBLIC VIRTUAL IMSStateMachine::~IMSStateMachine()
{
    // remove all of primitives
}

IMS_BOOL IMSStateMachine::OnStateMessage(IN IMSMSG& objMSG)
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

#if 0  // protected
#endif

EMPTY_STATE_MAP(IMSStateMachine)

PROTECTED
IMS_BOOL IMSStateMachine::SetState(IN IMS_UINT32 nState)
{
    nOldState = this->nState;
    this->nState = nState;

    return IMS_TRUE;
}

PROTECTED
IMS_UINT32 IMSStateMachine::GetState()
{
    return nState;
}

PROTECTED
IMS_UINT32 IMSStateMachine::GetOldState()
{
    return nOldState;
}
