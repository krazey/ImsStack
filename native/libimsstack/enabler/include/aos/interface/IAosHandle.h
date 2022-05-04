#ifndef _INTERFACE_AOS_HANDLE_H_
#define _INTERFACE_AOS_HANDLE_H_

#include "IMSTypeDef.h"
#include "AString.h"

class IImsAosMonitor;
class AosFeatureTagList;

class IAosHandle
{
public:
    virtual AString& GetAppId() = 0;
    virtual AString& GetServiceId() = 0;
    virtual IMS_UINT32 GetServiceType() = 0;
    virtual IImsAosMonitor* GetMonitor() = 0;

    // nReqType is set from AoSHandle
    enum
    {
        DETACH = 0,  // This service will be removed in Registration
        ATTACH       // This service will be added in Registration
    };
    virtual IMS_SINT32 GetRequestType() = 0;
    virtual void SetRequestType(IN IMS_SINT32 nReqType) = 0;

    // bBind is set from AoSRegistration
    /*
        if bBind is true, this service is added in Registration
        if bBind is false, this service is removed in Registration
    */
    virtual IMS_BOOL IsRegBinded() = 0;
    virtual void SetRegBinded(IN IMS_BOOL bBind) = 0;

    // bNetworkBind is set from AoSRegistration
    /*
        if bNetworkBind is true, this service is kept in Registration
        if bNetworkBind is false, this service is removed in Registration
    */
    virtual IMS_BOOL IsNetworkRegBinded() = 0;
    virtual void SetNetworkRegBinded(IN IMS_BOOL bNetworkBind) = 0;

    virtual AosFeatureTagList& GetFeatureTagList() = 0;
    virtual AosFeatureTagList& GetBindedFeatureTagList() = 0;

    virtual void ProcessFeatureTagChange() = 0;

    // Request : nType
    enum
    {
        // Action
        TYPE_LIMITED_MODE = 100,
        TYPE_FAKE_MODE,

        // Notify to Monitor
        TYPE_SERVICE_BLOCKED = 110,
        TYPE_HANDOVER
    };

    // Request : nState
    enum
    {
        STATE_ADD = 0,
        STATE_REMOVE
    };
    virtual void Request(IN IMS_UINT32 nType, IN IMS_UINT32 nState = 0) = 0;

    // AoSApp to AoSHandle
    virtual void App_StateChanged(IN IMS_UINT32 nState, IN IMS_UINT32 nReason = 0) = 0;
    virtual void App_Notify() = 0;

    // AoSHandle to AoSHandle
    virtual void Handle_Notify(IN IMS_UINT32 nType, IN IMS_BOOL bBlocked) = 0;

protected:
    friend class AosBuildDirector;
    friend class AosAppContext;
    virtual void Init() = 0;
    virtual void CleanUp() = 0;
};
#endif  // _INTERFACE_AOS_HANDLE_H_
