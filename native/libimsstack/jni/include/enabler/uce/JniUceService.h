/*
    Author
    <table>
    date              author                    description
    --------      --------------            ----------
    20111206      hyunho.shin@           Created
    </table>

    Description

*/

#ifndef _IMS_UCE_SERVICE_H_
#define _IMS_UCE_SERVICE_H_

#include "BaseService.h"

class JniUceServiceThread;

using namespace android;

class JniUceService : public BaseService
{
public:
    JniUceService(IN IMS_UINT32 nSimSlot = 0);
    JniUceService(Jni_SendDataToJava pfnSendDataToJava, IN IMS_UINT32 nSimSlot = 0);
    virtual ~JniUceService();

    virtual int SendData(const Parcel& pParcel);

private:
    void HandleMessage(int nMsg, const Parcel& pParcel);

private:
    JniUceServiceThread* m_pJniUceServiceThread;
    IMS_UINT32 m_nSimSlot;
    AString m_strTarget;
};

#endif  //_IMS_PEOPLE_SERVICE_H_
