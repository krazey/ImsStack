#ifndef CALL_INFO_H_
#define CALL_INFO_H_

#include "IMSTypeDef.h"
#include "IMtcService.h"
#include "call/IMtcCall.h"

struct JniCallInfo
{
public:
    JniCallInfo() :
            eServiceType(ServiceType::NORMAL),
            eCallType(CallType::VOIP),
            bWifi(IMS_FALSE),
            bEmergency(IMS_FALSE),
            bOffline(IMS_FALSE),
            bUssi(IMS_FALSE),
            bConference(IMS_FALSE),
            bConferenceEnabled(IMS_FALSE),
            bConferenceSubscriptionRequired(IMS_FALSE),
            bRttCapable(IMS_FALSE),
            bVideoCapable(IMS_FALSE)
    {
    }

    JniCallInfo(IN const JniCallInfo& objRhs) :
            eServiceType(objRhs.eServiceType),
            eCallType(objRhs.eCallType),
            bWifi(objRhs.bWifi),
            bEmergency(objRhs.bEmergency),
            bOffline(objRhs.bOffline),
            bUssi(objRhs.bUssi),
            bConference(objRhs.bConference),
            bConferenceEnabled(objRhs.bConferenceEnabled),
            bConferenceSubscriptionRequired(objRhs.bConferenceSubscriptionRequired),
            bRttCapable(objRhs.bRttCapable),
            bVideoCapable(objRhs.bVideoCapable)
    {
    }

    JniCallInfo& operator=(IN const JniCallInfo& objRhs)
    {
        if (this != &objRhs)
        {
            eServiceType = objRhs.eServiceType;
            eCallType = objRhs.eCallType;
            bWifi = objRhs.bWifi;
            bEmergency = objRhs.bEmergency;
            bOffline = objRhs.bOffline;
            bUssi = objRhs.bUssi;
            bConference = objRhs.bConference;
            bConferenceEnabled = objRhs.bConferenceEnabled;
            bConferenceSubscriptionRequired = objRhs.bConferenceSubscriptionRequired;
            bRttCapable = objRhs.bRttCapable;
            bVideoCapable = objRhs.bVideoCapable;
        }

        return *this;
    }

public:
    ServiceType eServiceType;
    CallType eCallType;

    IMS_BOOL bWifi;
    IMS_BOOL bEmergency;
    IMS_BOOL bOffline;
    IMS_BOOL bUssi;
    IMS_BOOL bConference;
    IMS_BOOL bConferenceEnabled;
    IMS_BOOL bConferenceSubscriptionRequired;
    IMS_BOOL bRttCapable;
    IMS_BOOL bVideoCapable;
};

#endif
