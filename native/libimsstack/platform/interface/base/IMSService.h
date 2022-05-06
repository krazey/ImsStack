/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100324  joonhun.shin@             Created
    </table>

    Description

*/

#ifndef _IMS_SERVICE_H_
#define _IMS_SERVICE_H_

#include "IMSActivity.h"
#include "IMSStateMap.h"
#include "IMSStateObject.h"

class IMSService : public IMSActivity, public IMSStateObject
{
    DECLARE_STATE_MAP()

public:
    IMSService(IN CONST AString& strName);
    virtual ~IMSService();

protected:
    virtual IMS_BOOL OnPreprocess(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnPostprocess(IN IMSMSG& objMSG);
    virtual IIMSActivityControl* GetController();

    IMS_BOOL SetState(IN IMS_UINT32 nState);
    IMS_UINT32 GetState();
    IMS_UINT32 GetOldState();

private:
    virtual IMS_BOOL DispatchMessage(IN IMSMSG& objMSG);
    IMS_BOOL OnStateMsgProcess(IN IMSMSG& objMSG);

private:
    IMS_UINT32 nState;
    IMS_UINT32 nOldState;
};

#endif  // _IMS_SERVICE_H_
