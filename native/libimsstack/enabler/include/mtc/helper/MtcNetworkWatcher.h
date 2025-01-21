
/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef MTC_NETWORK_WATCHER_H_
#define MTC_NETWORK_WATCHER_H_

#include "IMtcService.h"

class MtcNetworkWatcher : public INetworkWatcherListener
{
public:
    explicit MtcNetworkWatcher(IN IMtcService& objService, IN IMS_SINT32 nSlotId);
    MtcNetworkWatcher(const MtcNetworkWatcher&) = delete;
    MtcNetworkWatcher& operator=(const MtcNetworkWatcher&) = delete;
    virtual ~MtcNetworkWatcher();

    virtual void AddListener(IN IMtcNetworkWatcherListener& objListener);
    virtual void RemoveListener(IN IN IMtcNetworkWatcherListener& objListener);
    virtual IMS_SINT32 GetRatType() const;
    virtual IMS_SINT32 GetMobileRatType() const;
    virtual void OnServiceConnected(IN IMS_UINT32 eIpcan);
    // This API must be used test purpose only.
    virtual void SetTestRatChanged(IN IMS_SINT32 eRatType);

    void NetworkWatcher_NotifyStatus(IN INetworkWatcher* piNetWatcherInfo) override;

private:
    void Notify();
    static IMS_SINT32 ConvertCellularRatType(NETRADIO_ENTYPE eRatType);
    IMS_SINT32 GetCurrentRat() const;

    ServiceType m_eServiceType;
    IMS_SINT32 m_nSlotId;
    INetworkWatcher* m_piNetWatcher;
    IMS_UINT32 m_eIpcanType;
    IMS_SINT32 m_eMobileRatType;

    ImsList<IMtcNetworkWatcherListener*> m_objListeners;
};

#endif
