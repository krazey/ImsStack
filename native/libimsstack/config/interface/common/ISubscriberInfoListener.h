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
#ifndef INTERFACE_SUBSCRIBER_INFO_LISTENER_H_
#define INTERFACE_SUBSCRIBER_INFO_LISTENER_H_

#include "AString.h"

class ISubscriberInfoListener
{
public:
    /**
     * @brief Notifies the application that the subscriber info. (IMPU) has been updated.
     *
     * @param nSlotId The slot-id
     * @param strId The identifier of subscriber configuration that contains the subscriber info.
     * @param strOld The IMPU that is previously set
     * @param strNew The IMPU that will be updated
     */
    virtual void SubscriberInfo_UpdateImpu(IN IMS_SINT32 nSlotId, IN const AString& strId,
            IN const AString& strOld, IN const AString& strNew) = 0;
};

#endif
