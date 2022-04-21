#ifndef _INTERFACE_AOS_REGISTRATION_CONTROL_LISTENER_H_
#define _INTERFACE_AOS_REGISTRATION_CONTROL_LISTENER_H_

#include "IMSMap.h"
#include "IMSTypeDef.h"
#include "AString.h"

class IAosRegistrationControlListener
{
public:
    virtual void RegistrationControl_UpdateSipDelegateRegistration() = 0;
    virtual void RegistrationControl_TriggerSipDelegateDeregistration() = 0;
    virtual void RegistrationControl_TriggerFullNetworkRegistration(IN IMS_SINT32 nSipCode,
            IN const AString& strTarget) = 0;
    virtual void RegistrationControl_NotifyCapabilitiesChanged(
            IN const IMSMap<IMS_UINT32, IMS_UINT32>& objCapabilities) = 0;
    virtual void RegistrationControl_ControlRegistration(IN IMS_SINT32 nRequestType,
            IN IMS_SINT32 nPcscfOrder) = 0;
};

class AosRegistrationControlListener
    : public IAosRegistrationControlListener
{
public:
    inline void RegistrationControl_UpdateSipDelegateRegistration() override {};
    inline void RegistrationControl_TriggerSipDelegateDeregistration() override {};
    inline void RegistrationControl_TriggerFullNetworkRegistration(
            IN IMS_SINT32 /*nSipCode*/, IN const AString& /*strTarget*/) override {};
    inline void RegistrationControl_NotifyCapabilitiesChanged(
            IN const IMSMap<IMS_UINT32, IMS_UINT32>& /*objCapabilities*/) override {};
    inline void RegistrationControl_ControlRegistration(IN IMS_SINT32 /*nRequestType*/,
            IN IMS_SINT32 /*nPcscfOrder*/) override {};
};

#endif // _INTERFACE_AOS_REGISTRATION_CONTROL_LISTENER_H_