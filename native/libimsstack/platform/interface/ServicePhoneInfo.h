/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090819  YR@                       Created
    </table>

    Description

*/

#ifndef _SERVICE_IMS_PHONE_INFO_H_
#define _SERVICE_IMS_PHONE_INFO_H_

#include "IPhoneInfoCall.h"
#include "IPhoneInfoDevice.h"
#include "IPhoneInfoPower.h"
#include "IPhoneInfoSubscriber.h"
#include "IPhoneInfoLocation.h"
#include "INetworkWatcher.h"
#include "IWifiWatcher.h"
#include "IIsim.h"
#include "IUsim.h"
#include "ISrvcc.h"
#include "ITrm.h"

class PhoneInfoServicePrivate;

class PhoneInfoService
{
private:
    PhoneInfoService();
    ~PhoneInfoService();

    PhoneInfoService(IN const PhoneInfoService& objRHS);
    PhoneInfoService& operator=(IN const PhoneInfoService& objRHS);

public:
    void DispatchServiceMessage(IN ImsMessage &objMSG);

    IDeviceInfo* GetDeviceInfo();
    ISubscriberInfo* GetSubscriberInfo(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    ICallInfo* GetCallInfo(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IIsim* GetIsim(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IPowerInfo* GetPowerInfo();
    INetworkWatcher* GetNetworkWatcher(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IWifiWatcher* GetWifiWatcher();
    ILocationInfo* GetLocationInfo(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    ISrvcc* GetSrvcc(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IUsim* GetUsim(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    ITrm* GetTrm();

    static PhoneInfoService* GetPhoneInfoService();

private:
    PhoneInfoServicePrivate *pPrivate;
};

#endif // _SERVICE_IMS_PHONE_INFO_H_
