/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _INTERFACE_AOS_HANDLE_H_
#define _INTERFACE_AOS_HANDLE_H_

#include "ImsTypeDef.h"
#include "AString.h"

class IImsAosMonitor;
class AosFeatureTagList;

class IAosHandle
{
public:
    virtual ~IAosHandle(){};

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

    virtual IMS_BOOL IsRegFeatureTagRequired() = 0;
    virtual IMS_BOOL IsRegToNextPcscfRequested() = 0;

    virtual AosFeatureTagList& GetFeatureTagList() = 0;
    virtual AosFeatureTagList& GetBindedFeatureTagList() = 0;

    virtual void ProcessFeatureTagChange() = 0;

    // Request : nType
    enum
    {
        // Notify to Handle
        TYPE_LIMITED_MODE = 100,

        // Notify to Monitor
        TYPE_HANDOVER = 111,
    };

    // Request : nState
    enum
    {
        STATE_ADD = 0,
        STATE_REMOVE
    };

    virtual void Request(IN IMS_UINT32 nType, IN IMS_UINT32 nState = 0) = 0;

    // AoSApp to AoSHandle
    virtual IMS_BOOL App_StateChanged(IN IMS_UINT32 nState, IN IMS_UINT32 nReason = 0) = 0;
    virtual IMS_BOOL App_Notify() = 0;

    // AoSHandle to AoSHandle
    virtual void Handle_Notify(IN IMS_UINT32 nType, IN IMS_BOOL bBlocked) = 0;

protected:
    friend class AosBuildDirector;
    friend class AosAppContext;
    virtual void Init() = 0;
    virtual void CleanUp() = 0;
};
#endif  // _INTERFACE_AOS_HANDLE_H_
