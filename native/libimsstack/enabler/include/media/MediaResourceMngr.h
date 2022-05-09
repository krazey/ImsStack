/**
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

#ifndef _IMS_MEDIA_RESOURCE_MNGR_H_
#define _IMS_MEDIA_RESOURCE_MNGR_H_

#include "MediaDef.h"
#include "config/MediaConfiguration.h"
#include "config/AudioConfiguration.h"
#include "IMediaConnectionWatcherListener.h"

class INetworkConnection;
class IMediaConnectionWatcher;

class MediaResourceMngr : public IMediaConnectionWatcherListener
{
    // == Inner class  ========================================
    class PDNResource
    {
    public:
        IMS_SINT32 nIsIPv6;  // -1 : No Information, 0 : IPv4, 1 : IPv6
        AString strIPAddrOfIPv6;
        AString strApnName;
        IMS_BOOL m_bModemIPUpdated;

    public:
        PDNResource() :
                nIsIPv6(MEDIA_IP_NONE),
                strIPAddrOfIPv6(AString::ConstNull()),
                strApnName(AString::ConstNull()),
                m_bModemIPUpdated(IMS_FALSE){};
    };

    // == Constructor, Destructor, Operator Overloading ========================================
public:
    MediaResourceMngr(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    virtual ~MediaResourceMngr();

protected:
    // == PUBLIC METHOD ==============================================================
public:
    virtual IMS_BOOL UpdatePdnResource(IN IMS_SINT32 nPDNType, IN IMS_BOOL nIsIPv6);
    virtual void ResetPdnResource(IN IMS_SINT32 nPDNType = PDN_IMS);
    virtual void UpdateAPN(IN MEDIA_SERVICE_TYPE eMediaServiceType, PDNResource* pLocalPdn);
    virtual AString GetApnName(IN MEDIA_SERVICE_TYPE eServiceType);
    virtual IMS_UINT32 AcquireRtpPort(IN MediaConfiguration* pConfig);
    virtual IMS_UINT32 AcquireRtpPort(IN IMS_UINT32 nRangeStart, IN IMS_UINT32 nRangeEnd);
    virtual void ReleaseRtpPort(IN IMS_UINT32 nPort);
    /* IMediaConnectionWatcherListener Interface Impl */
    virtual void NotifyMediaConnection(IN INetworkConnection* piNetConnection,
            IN IMS_SINT32 nMediaConnectionType, IN IMS_UINT32 nNetworkInterfaceId);
    virtual void NotifyIPChanged(IMS_BOOL bIsIPv6);
    virtual void NotifyWifiEarlyRouteSetup(IN IMS_UINT32 nNetworkInferfaceID);
    virtual IMediaConnectionWatcher* GetMediaConnectionWatcher();
    virtual IMS_BOOL GetMediaConnectionWatcherInfo(IN IPAddress& objIpAddress,
            OUT IMS_BOOL& bWIFICondition, OUT IMS_UINT32& nNetworkInterfaceId);
    virtual IMS_UINT32 GetRtpFragmentSize(IN IPAddress& objIpAddress);
    // == PRIVATE METHOD ============================================================
    virtual INetworkConnection* GetNetConnection(IN MEDIA_SERVICE_TYPE eServiceType);
    virtual MEDIA_NETWORK_TYPE ConvertMediaNetworkType(IN IMS_SINT32 eRadioType);
    virtual IMS_SINT32 GetSupportedNetworkTypeFlag();
    IMS_BOOL SetMediaConnectionWatcherListener();
    IMS_BOOL UnsetMediaConnectionWatcherListener();
    virtual void SetSlotId(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    virtual IMS_SINT32 GetSlotId();

    // == PRIVATE VARIABLE ============================================================
protected:
    IMS_SINT32 m_nSlotId;
    PDNResource m_objIMSPDN;
    PDNResource m_objEmergencyPDN;
    IMSList<IMS_UINT32> m_lstUsedRtpPort;
    IMS_SINT32 m_nSupportedNetworkTypeFlag;

public:
    enum
    {
        PDN_INTERNET,
        PDN_IMS,
        PDN_EMERGENCY,
    };  // PDN Type
};
#endif /* _IMS_MEDIA_RESOURCE_MNGR_H_ */
