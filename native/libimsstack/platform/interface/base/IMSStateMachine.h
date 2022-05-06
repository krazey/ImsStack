/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100331  joonhun.shin@             create (hwangoo.park@ base code)
    </table>

    Description

*/

#ifndef _IMS_STATEMACHINE_H_
#define _IMS_STATEMACHINE_H_

#include "IMSStateMap.h"
#include "IMSActivity.h"
#include "IMSStateObject.h"

class IMSStateMachine : public IMSStateObject
{
    DECLARE_STATE_MAP()

public:
    IMSStateMachine(IN IMS_UINT32 nState = IMS_INVALID_STATE);
    virtual ~IMSStateMachine();
    IMS_BOOL OnStateMessage(IN ImsMessage& objMSG);

protected:
    IMS_BOOL SetState(IN IMS_UINT32 nState);
    IMS_UINT32 GetState();
    IMS_UINT32 GetOldState();

private:
    IMS_UINT32 nState;
    IMS_UINT32 nOldState;
};

#endif  // _IMS_STATEMACHINE_H_
