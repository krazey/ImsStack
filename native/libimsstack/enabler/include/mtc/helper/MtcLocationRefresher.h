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

#ifndef MTC_LOCATION_REFRESHER_H_
#define MTC_LOCATION_REFRESHER_H_

#include "IPhoneInfoLocation.h"
#include "ImsList.h"
#include "ImsTypeDef.h"

/*
 * This class manages location refresh requests through the {@link ILocationInfo} and notifies the
 * registered listeners.
 *
 * It can be used when the module that requests location update and the module that receiving the
 * result are different.
 */
class MtcLocationRefresher final : public ILocationUpdateListener
{
public:
    enum class LocationRefreshState
    {
        IDLE,
        REFRESHING,
    };

    explicit MtcLocationRefresher(IN ILocationInfo& objLocationInfo);
    ~MtcLocationRefresher();
    MtcLocationRefresher(IN const MtcLocationRefresher&) = delete;
    MtcLocationRefresher& operator=(IN const MtcLocationRefresher&) = delete;

    /**
     * @brief Starts location refresh. Listeners will be notified if the wait timer is expired or
     *        update is completed.
     *
     * If it's waiting for the refresh already, it doesn't trigger another refresh request.
     * All listeners will receive notification simultaneously, regardless of when they are added.
     *
     * @param nWaitTimeInMillis The maximum wait time in milliseconds.
     */
    void RequestUpdate(IN IMS_SINT32 nWaitTimeInMillis);

    void AddListener(IN ILocationUpdateListener& objListener);
    void RemoveListener(IN ILocationUpdateListener& objListener);

    /**
     * @brief Gets the current location update state.
     *
     * @return {@code REFRESHING} if there's ongoing request and wait for it.
     *         Otherwise {@code IDLE}.
     */
    inline LocationRefreshState GetState() const { return m_eState; }

    void LocationUpdate_OnCompleted() override;

private:
    ILocationInfo& m_objLocationInfo;
    LocationRefreshState m_eState;
    ImsList<ILocationUpdateListener*> m_lstListeners;

    void NotifyListeners();
};

#endif
