/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100324  joonhun.shin@             Created
    </table>

    Description

*/

#ifndef _IMS_ACTIVITYMNGR_H_
#define _IMS_ACTIVITYMNGR_H_

#include "IMSActivity.h"
#include "IIMSActivityControl.h"

class IMSActivityMngr
{
public:
    IMSActivityMngr();
    ~IMSActivityMngr();

    IMS_BOOL Attach(IN IMSActivity* pIMSActivity);
    void Detach(IN IMSActivity* pIMSActivity);
    IMSActivity* Get(IN CONST AString& strActivityName);
    AString GenerateName(IN CONST AString& strThreadName, IN CONST AString& strName);
    IMS_BOOL HandleMessage(IN IMSMSG& objMSG);
    IIMSActivityControl* GetController(IN CONST AString& strControllerName);

private:
    AString GetActivityNameFromMsg(IN CONST AString& strTargetName);

private:
    IMS_UINT32 nMajorId;
    IMS_UINT32 nMinorId;
    IMSList<IMSActivity*> objIMSActivities;
};

#endif  // _IMS_ACTIVITYMNGR_H_
