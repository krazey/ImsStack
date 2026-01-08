/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef MTS_NETWORK_TRACKER_H_
#define MTS_NETWORK_TRACKER_H_

#include "IEventListener.h"
#include "IMtsNetworkTracker.h"
#include "ImsTypeDef.h"

class IMtsContext;
class INetworkWatcher;

class MtsNetworkTracker : public IEventListener, public IMtsNetworkTracker
{
public:
    explicit MtsNetworkTracker(IN IMtsContext& objContext);
    virtual ~MtsNetworkTracker() override;
    MtsNetworkTracker(IN const MtsNetworkTracker&) = delete;
    MtsNetworkTracker& operator=(IN const MtsNetworkTracker&) = delete;

    // IEventListener
    void Event_NotifyEvent(
            IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam) override;

    // IMtsNetworkTracker
    IMS_SINT32 GetCellularServiceState() const override;
    inline IMS_UINT32 GetLteAttachState() const override { return m_nLteAttachState; }
    IMS_SINT32 GetNetworkType() const override;
    inline IMS_BOOL IsInRoamingState() const override { return m_bDataRoaming; }

protected:
    INetworkWatcher* m_piNetWatcherInfo;

private:
    IMtsContext& m_objContext;
    IMS_BOOL m_bDataRoaming;
    IMS_UINT32 m_nLteAttachState;
};

#endif
