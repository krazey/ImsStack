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
#ifndef INTERFACE_NOTIFICATION_LISTENER_H_
#define INTERFACE_NOTIFICATION_LISTENER_H_

#include "ImsTypeDef.h"

class IServiceMethod;

/**
 * @brief This class provides a listener interface which is used to notify the application about
 *        the delivery result of notification (as notifier for REFER-NOTIFY, SUBSCRIBE-NOTIFY).
 *
 * @see IReference
 */
class INotificationListener
{
public:
    /**
     * @brief Notifies the application that the notification was successfully delivered.
     *
     * This can be notified by ISubscription or IReference.
     *
     * @param piMethod Service method to be notified
     */
    virtual void NotificationDelivered(IN IServiceMethod* piMethod) = 0;

    /**
     * @brief Notifies the application that the notification was not successfully delivered.
     *
     * This can be notified by ISubscription or IReference.
     *
     * @param piMethod Service method to be notified
     * @param nStatusCode SIP status code\n
     *                    0(zero) indicates that SIP transaction timeout is occurred
     */
    virtual void NotificationDeliveryFailed(
            IN IServiceMethod* piMethod, IN IMS_SINT32 nStatusCode) = 0;
};

#endif
