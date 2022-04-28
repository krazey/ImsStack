#ifndef INTERFACE_MTS_MESSAGE_CONTROLLER_LISTENER_H_
#define INTERFACE_MTS_MESSAGE_CONTROLLER_LISTENER_H_

class IMtsMessageControllerListener
{
public:
    virtual void MtsMessageController_NoTransaction() = 0;
};

#endif
