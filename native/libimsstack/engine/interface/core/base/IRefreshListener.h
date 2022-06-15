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
#ifndef INTERFACE_REFRESH_LISTENER_H_
#define INTERFACE_REFRESH_LISTENER_H_

class ISipClientConnection;

/**
 * @brief This class defines a refresh listener for refreshable SIP methods.
 */
class IRefreshListener
{
public:
    /**
     * @brief Notifies the application that the refresh operation is completed.
     *
     * @param piScc SIP client connection for the refresh transaction
     * @see ISipClientConnection
     */
    virtual void Refresh_NotifyCompleted(IN ISipClientConnection* piScc) = 0;

    /**
     * @brief Notifies the application that the refresh is terminated.
     */
    virtual void Refresh_NotifyTerminated() = 0;

    /**
     * @brief Notifies the application that the refresh timer is expired.
     *
     * @param bDoImplicitRefresh Flag to indicate if the refresh will be executed or not
     */
    virtual void Refresh_NotifyTimerExpired(OUT IMS_BOOL& bDoImplicitRefresh) = 0;
};

#endif
