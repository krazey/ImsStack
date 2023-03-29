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

class IAosRegStateManagerListener;

class IAosRegStateManager
{
public:
    virtual ~IAosRegStateManager(){};

    /**
     * @brief Set the listener for getting the registration state.
     *
     * @param piRegListener Indicate the registration state.
     */
    virtual void SetListener(IN IAosRegStateManagerListener* piRegListener) = 0;

    /**
     * @brief Get the slot id to be retrieved.
     *
     * @return IMS_SINT32 Return slot id to be retrieved.
     */
    virtual IMS_SINT32 GetSlotId() const = 0;

    /**
     * @brief Set the slot id.
     *
     * @param nSlotId Slot id to set up.
     */
    virtual void SetSlotId(IN IMS_SINT32 nSlotId) = 0;

    /**
     * @brief Set the ims registration state.
     *
     * @param nState Ims registration state to set up.
     * @param bLimited Ims registration mode. (@see MODE_XXX enum)
     */
    virtual void SetImsRegState(IN IMS_UINT32 nState, IN IMS_BOOL bLimited) = 0;

    /**
     * @brief Get the ims registration state
     *
     * @return IMS_SINT32 Indicate the registration information.
     */
    virtual IMS_SINT32 GetImsRegState() = 0;

    /**
     * @brief Set the emergency registration state.
     *
     * @param nState Emergency registration state to set up.
     */
    virtual void SetEImsRegState(IN IMS_UINT32 nState) = 0;

    /**
     * @brief Set the registration state for each service.
     *
     * @param nServiceType Service information to set up.
     * @param nState registration state to set up.
     */
    virtual void SetRegState(IN IMS_UINT32 nServiceType, IN IMS_UINT32 nState) = 0;
};
#endif  // INTERFACE_AOS_REG_STATE_MANAGER_H_
