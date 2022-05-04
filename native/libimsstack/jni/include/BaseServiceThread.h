/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20110309  joonhun.shin@             Created
    20110502  hwangoo.park@             Adapted from RCS
    </table>

    Description

*/

#ifndef _BASE_SERVICE_THREAD_H_
#define _BASE_SERVICE_THREAD_H_

#include "BaseService.h"
#include "BaseThread.h"
#include <binder/Parcel.h>

class BaseServiceThread : public BaseThread
{
public:
    BaseServiceThread();
    virtual ~BaseServiceThread();

public:
    void SetCallback(IN IMS_SINTP nNativeObject, CBServiceNoti pfnNotifier);

protected:
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMSG) override;
    IMS_BOOL SendData2Java(
            IN const android::Parcel& objParcel, IN IMS_BOOL bThreadSwitched = IMS_FALSE);
    virtual IMS_BOOL IsThreadSwitchingRequired(IN IMS_SINT32 nMsg) const;

protected:
    IMS_SINTP nNativeObject;
    CBServiceNoti pfnNotifier;
    static const IMS_UINT32 MESSAGE_THREAD_SWITCHING = 0;
};

#endif  //_BASE_SERVICE_THREAD_H_
