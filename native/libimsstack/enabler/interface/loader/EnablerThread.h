/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20170320  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _ENABLER_THREAD_H_
#define _ENABLER_THREAD_H_

#include "ImsMessageDef.h"
#include "IMSAppThread.h"

class EnablerFactory;

class EnablerThread
    : public IMSAppThread
{
public:
    EnablerThread(IN EnablerFactory *pEnablerFactory_, IN IMS_SINT32 nSlotId_);
    virtual ~EnablerThread();

public:
    IMS_SINT32 GetSlotId() const;
    void ControlEnablers(IN IMS_SINT32 nCtrlFlags);

protected:
    // IMSAppThread class
    virtual IMS_BOOL Initialize();

    virtual IMS_BOOL OnStart(IN IMSMSG &objMSG);
    virtual IMS_BOOL OnTerminate(IN IMSMSG &objMSG);
    virtual IMS_BOOL OnMessage(IN IMSMSG &objMSG);

    inline IMS_BOOL IsControlSet(IN IMS_SINT32 nCtrlFlags, IN IMS_SINT32 nCtrlFlag) const
    { return (nCtrlFlags & nCtrlFlag) == nCtrlFlag; }
    inline IMS_SINT32 GetState() const
    { return nState; }

    void InitializeGlobals();
    void UninitializeGlobals();

    void ControlEnablersInternal(IN IMS_SINT32 nFlags);
    void NotifyEnablerStartCompleted();
    void SetState(IN IMS_SINT32 nState);
    IMS_BOOL StartEnablers();
    void StopEnablers();

public:
    // Bitmask for operations
    enum
    {
        CONTROL_NONE  = 0x0000,

        CONTROL_CREATE = 0x0001,
        CONTROL_DESTROY = 0x0002,

        CONTROL_START = 0x0010,
        CONTROL_STOP = 0x0020,

        CONTROL_ALL = (CONTROL_CREATE | CONTROL_DESTROY | CONTROL_START | CONTROL_STOP)
    };

private:
    // Enabler's state
    enum
    {
        STATE_INACTIVE = 0,
        STATE_ACTIVE = 1
    };

    enum
    {
        TMSG_CONTROL_ENABLERS = IMS_MSG_THREAD_INTERNAL_BASE
    };

    EnablerFactory *pEnablerFactory;
    IMS_SINT32 nSlotId;
    IMS_SINT32 nState;
};

#endif // _ENABLER_THREAD_H_
