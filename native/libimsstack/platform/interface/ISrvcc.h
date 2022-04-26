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
#ifndef INTERFACE_SRVCC_H_
#define INTERFACE_SRVCC_H_

#include "ImsTypeDef.h"

class ISrvccListener;

class ISrvcc
{
public:
    enum
    {
        SRVCC_EVENT_START = 1,
        SRVCC_EVENT_FAILURE = 2,
        SRVCC_EVENT_SUCCESS = 3,
        SRVCC_EVENT_CANCEL = 4
    };

public:
    virtual void SubscribeSrvccListener(IN ISrvccListener* piListener) = 0;
    virtual void UnsubscribeSrvccListener(IN ISrvccListener* piListener) = 0;
};

class ISrvccListener
{
public:
    /**
     * Notifies the application that SRVCC event is changed.
     *
     * @param nEvent The SRVCC event to be notified\n
     *               #SRVCC_EVENT_START\n
     *               #SRVCC_EVENT_FAILURE\n
     *               #SRVCC_EVENT_SUCCESS\n
     *               #SRVCC_EVENT_CANCEL
     */
    virtual void Srvcc_NotifyEventChanged(IN IMS_SINT32 nEvent) = 0;
};

#endif
