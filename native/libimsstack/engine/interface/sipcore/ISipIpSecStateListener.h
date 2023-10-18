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
#ifndef INTERFACE_SIP_IPSEC_STATE_LISTENER_H_
#define INTERFACE_SIP_IPSEC_STATE_LISTENER_H_

#include "ImsTypeDef.h"

/**
 * @brief This class provides an interface to monitor IpSec SA(Security Association) state.
 */
class ISipIpSecStateListener
{
protected:
    virtual ~ISipIpSecStateListener() = default;

public:
    /**
     * @brief Notifies the application that IpSec security association's state is changed.
     *
     * @param nSaType SA type to be notified\n
     *                #ISipIpSecState#SA_NEW\n
     *                #ISipIpSecState#SA_OLD
     */
    virtual void IpSecState_StateChanged(IN IMS_SINT32 nSaType) = 0;
};

#endif
