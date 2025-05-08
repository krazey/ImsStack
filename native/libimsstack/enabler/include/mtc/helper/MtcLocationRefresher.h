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
#include "ImsTypeDef.h"

/*
 * This class stores the state of location refresh request through the {@link ILocationInfo} and
 * notifies the registered listener when it's finished.
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
        REFRESHED,
    };

    explicit MtcLocationRefresher(IN ILocationInfo& objLocationInfo);
    ~MtcLocationRefresher();
    MtcLocationRefresher(IN const MtcLocationRefresher&) = delete;
    MtcLocationRefresher& operator=(IN const MtcLocationRefresher&) = delete;

    /**
     * @brief Starts location refresh. The registered listener will be notified if the wait timer is
     *        expired or update is completed.
     *
     * If it's waiting for the refresh already, it doesn't trigger another refresh request.
     *
     * @param nWaitTimeInMillis The maximum wait time in milliseconds.
     */
    void RequestUpdate(IN IMS_SINT32 nWaitTimeInMillis);

    void SetListener(IN ILocationUpdateListener& objListener);
    void ResetListener();

    /**
     * @brief Gets the current location update state.
     *
     * @return {@code IDLE} if no request have been made.
     *         {@code REFRESHING} if there's ongoing request and wait for it.
     *         {@code REFRESHED} Location have been updated.
     */
    inline LocationRefreshState GetState() const { return m_eState; }

    void LocationUpdate_OnCompleted() override;

private:
    ILocationInfo& m_objLocationInfo;
    LocationRefreshState m_eState;
    ILocationUpdateListener* m_pListener;
};

#endif
