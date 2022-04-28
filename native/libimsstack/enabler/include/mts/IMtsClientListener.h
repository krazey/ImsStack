#ifndef IMTS_CLIENT_LISTENER_H_
#define IMTS_CLIENT_LISTENER_H_

#include "ImsWmsLiteTypeDef.h"

class IMtsClientListener
{
public:
    virtual void Client_SendMo(IN IMSWMS_UINTP nWparam_,
        IN IWMSSmsSendRequestParam* nLparam_) = 0;
};

#endif
