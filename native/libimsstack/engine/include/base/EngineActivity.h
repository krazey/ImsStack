/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100427  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _ENGINE_ACTIVITY_H_
#define _ENGINE_ACTIVITY_H_

#include "ImsMessageDef.h"
#include "IMSActivity.h"

class EngineActivity : public IMSActivity
{
public:
    explicit EngineActivity(IN CONST AString& strName_ = AString::ConstNull());
    virtual ~EngineActivity();

protected:
    // IMSActivity class
    virtual IIMSActivityControl* GetController();
    virtual IMS_BOOL DispatchMessage(IN IMSMSG& objMSG);

    virtual void OnDestroy();

protected:
    enum
    {
        // Connection Message - System messages: resource release, ...
        // IMS_MSG_USER ~ (IMS_MSG_USER + 100)
        AMSG_DESTROY = IMS_MSG_USER,

        // Connection Message - User messages: start message of user messages for derived classes
        AMSG_USER = (IMS_MSG_USER + 101)
    };
};

#endif  // _ENGINE_ACTIVITY_H_
