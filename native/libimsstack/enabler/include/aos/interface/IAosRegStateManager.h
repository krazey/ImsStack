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
#ifndef INTERFACE_AOS_REG_STATE_MANAGER_H_
#define INTERFACE_AOS_REG_STATE_MANAGER_H_

#define IMS_REG_OFF                                (0)
#define IMS_REG_ON                                 (1)

#define IMS_REGISTRATION_INVALID                   (-1)
#define IMS_REGISTRATION_OFFLINE                   (0)
#define IMS_REGISTRATION_REGISTERING               (1)
#define IMS_REGISTRATION_REGISTERED                (2)
#define IMS_REGISTRATION_REREGISTERING             (3)
#define IMS_REGISTRATION_DEREGISTERING             (4)
#define IMS_REGISTRATION_STOP                      (5)

#define IMS_REGISTRATION_SERVICE_NONE              (0)

#define IMS_REGISTRATION_REASON_BLOCK_NOTIFICATION (0x100)

class IAosRegStateManager
{
public:
    virtual ~IAosRegStateManager(){};

    virtual IMS_SINT32 GetSlotId() const = 0;
    virtual void SetSlotId(IN IMS_SINT32 nSlotId) = 0;

    virtual void SetImsRegState(IN IMS_UINT32 nState, IN IMS_BOOL bLimited) = 0;
    virtual IMS_SINT32 GetImsRegState() = 0;
    virtual void SetEImsRegState(IN IMS_UINT32 nState) = 0;
    virtual void SetRegState(IN IMS_UINT32 nServiceType, IN IMS_UINT32 nState) = 0;
};
#endif  // INTERFACE_AOS_REG_STATE_MANAGER_H_
