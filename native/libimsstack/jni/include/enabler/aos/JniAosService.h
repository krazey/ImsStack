#ifndef _JNI_AOS_SERVICE_H_
#define _JNI_AOS_SERVICE_H_

#include "BaseService.h"

class JniAosServiceThread;
class IAosService;

class JniAosService
    : public BaseService
{
public:
    JniAosService(IN CBServiceNoti pCbServiceNoti, IN IMS_SINT32 nSlotId);
    virtual ~JniAosService();

    virtual int SendData(const android::Parcel& objParcel) override;

    void Initialize(IN CBServiceNoti pCbServiceNoti);
    void SetAosService(IN IAosService* piAosService);

    JniAosServiceThread* GetThread();

private:
    virtual void HandleMessage(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel) override;
    IMS_BOOL Attach();
    void UpdateSipDelegateRegistration(IN const android::Parcel& objParcel);
    void TriggerSipDelegateDeregistration(IN const android::Parcel& objParcel);
    void TriggerFullNetworkRegistration(IN const android::Parcel& objParcel);
    void NotifyCapabilitiesChanged(IN const android::Parcel& objParcel);
    void ControlRegistration(IN const android::Parcel& objParcel);

    void NotifyAirplaneSetting(IN const android::Parcel& objParcel);
    void NotifyDataRoamingSetting(IN const android::Parcel& objParcel);
    void NotifyMobileDataSetting(IN const android::Parcel& objParcel);
    void NotifyRoamingPreferredVoiceNetwork(IN const android::Parcel& objParcel);
    void NotifyServiceSetting(IN const android::Parcel& objParcel);
    void NotifyTtySetting(IN const android::Parcel& objParcel);
    void NotifyVideoSetting(IN const android::Parcel& objParcel);
    void NotifyVolteSetting(IN const android::Parcel& objParcel);
    void NotifyWfcSetting(IN const android::Parcel& objParcel);

    void NotifyAosStart(IN const android::Parcel& objParcel);
    void NotifyIpcanHandoverFailure(IN const android::Parcel& objParcel);
    void NotifyIsimState(IN const android::Parcel& objParcel);
    void NotifyLocationInfo(IN const android::Parcel& objParcel);
    void NotifyMobileDataLimit(IN const android::Parcel& objParcel);
    void NotifyNetworkVideoCapability(IN const android::Parcel& objParcel);
    void NotifyPhoneNumberState(IN const android::Parcel& objParcel);
    void NotifyPlmnChanged(IN const android::Parcel& objParcel);
    void NotifyPowerOff(IN const android::Parcel& objParcel);
    void NotifyPreciseCallState(IN const android::Parcel& objParcel);

    static void ConvertString(IN const android::String16& strSource, OUT AString& strDest);

private:
    IMS_SINT32 m_nSlotId;
    AString m_strThreadName;
    IAosService* m_piAosService;
    JniAosServiceThread* m_pJniAosServiceThread;
};

#endif // _JNI_AOS_SERVICE_H_