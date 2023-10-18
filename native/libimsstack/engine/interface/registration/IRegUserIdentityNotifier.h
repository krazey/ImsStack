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
#ifndef INTERFACE_REG_USER_IDENTITY_NOTIFIER_H_
#define INTERFACE_REG_USER_IDENTITY_NOTIFIER_H_

#include "AStringArray.h"

/**
 * @brief This class provides an interface to adjust public user identities
 *        which are provisioned from the IMS network.
 */
class IRegUserIdentityNotifier
{
protected:
    virtual ~IRegUserIdentityNotifier() = default;

public:
    /**
     * @brief Notifies the application when the network provisioned user identities are received
     *        (200 OK to REGISTER).
     *
     * Then, the application can reorder the default public user identities according to
     * the operator's policy.
     *
     * @param objUserIds Network provisioned user identities(from P-Associated-URI w/ same order)
     * @param objReorderedUserIds Reordered default public user identities
     * @return If default ordered user identities are changed, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL RegUserIdentity_ReorderUserIdentities(
            IN const AStringArray& objUserIds, OUT AStringArray& objReorderedUserIds) = 0;
};

#endif
