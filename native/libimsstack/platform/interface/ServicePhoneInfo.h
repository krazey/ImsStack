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
#include "INetWatcher.h"
#include "IWifiWatcher.h"
#include "IISIM.h"
#include "IUSIM.h"
#include "ISRVCC.h"
#include "ITRM.h"

class PhoneInfoServicePrivate;

class PhoneInfoService
{
private:
    PhoneInfoService();
    ~PhoneInfoService();

    PhoneInfoService(IN const PhoneInfoService& objRHS);
    PhoneInfoService& operator=(IN const PhoneInfoService& objRHS);

public:
    void DispatchServiceMessage(IN IMSMSG &objMSG);

    IDeviceInfo* GetDeviceInfo();
    ISubscriberInfo* GetSubscriberInfo(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IPhoneInfoCall* GetPhoneInfoCall(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IISIM* GetISIM(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IPowerInfo* GetPowerInfo();
    INetWatcherInfo* GetNetWatcherInfo(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IWifiWatcher* GetWifiWatcher();
    ILocationInfo* GetLocationInfo(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    ISRVCC* GetSRVCC(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IUSIM* GetUSIM(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    ITRM* GetTRM();

    static PhoneInfoService* GetPhoneInfoService();

private:
    PhoneInfoServicePrivate *pPrivate;
};

#endif // _SERVICE_IMS_PHONE_INFO_H_
