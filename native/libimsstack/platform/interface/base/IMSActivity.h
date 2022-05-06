/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100324  joonhun.shin@             Created
    </table>

    Description

*/

#ifndef _IMS_ACTIVITY_H_
#define _IMS_ACTIVITY_H_

#include "IThread.h"
#include "IIMSActivityControl.h"

class IMSActivity
{
public:
    // When giving the activity name, the name MUST not contain the dot ('.').
    IMSActivity(IN CONST AString& strName = AString::ConstNull());
    virtual ~IMSActivity();

    inline const AString& GetName() const { return strName; }
    inline IMS_SINT32 GetSlotId() const
    {
        return (pIThread == IMS_NULL) ? IMS_SLOT_ANY : pIThread->GetSlotId();
    }

    IMS_BOOL PostMessage(IN IMSMSG& objMSG);
    IMS_BOOL PostMessage(IN IMS_UINT32 nMSG, IN IMS_UINTP nWparam, IN IMS_UINTP nLparam);

    virtual IIMSActivityControl* GetController() = 0;

protected:
    virtual IMS_BOOL DispatchMessage(IN IMSMSG& objMSG) = 0;

private:
    static AString GetOwnerThreadName(IN CONST AString& strTargetName);

private:
    friend class IMSActivityMngr;

    IThread* pIThread;
    AString strName;
};

#endif  // _IMS_ACTIVITY_H_
