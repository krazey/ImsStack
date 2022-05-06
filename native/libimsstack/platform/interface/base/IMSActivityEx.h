/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100331  joonhun.shin@             Created
    </table>

    Description

*/

#ifndef _IMS_ACTIVITY_EX_H_
#define _IMS_ACTIVITY_EX_H_

#include "IMSActivity.h"

class IMSActivityEx : public IMSActivity
{
public:
    // When giving the activity name, the name MUST not contain the dot ('.').
    IMSActivityEx(IN CONST AString& strName = AString::ConstNull());
    virtual ~IMSActivityEx();

protected:
    inline virtual IIMSActivityControl* GetController() { return IMS_NULL; }

    virtual IMS_BOOL OnMessage(IN IMSMSG& objMSG);

private:
    virtual IMS_BOOL DispatchMessage(IN IMSMSG& objMSG);
};

#endif  // _IMS_ACTIVITY_EX_H_
