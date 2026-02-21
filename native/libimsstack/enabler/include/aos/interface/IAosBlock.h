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
#ifndef INTERFACE_AOS_BLOCK_H_
#define INTERFACE_AOS_BLOCK_H_

#include "ImsTypeDef.h"

class IAosBlockListener;
class IAosBlockSilentListener;

/// Block reason of IMS Service
typedef enum
{
    /// Start common Block reason
    BLOCK_START = 0,
    /// auto configuration hasn't been done yet.
    BLOCK_AC_INCOMPLETED = BLOCK_START,
    /// authentication is failed
    BLOCK_AUTHENTICATION_FAILED,
    /// USIM authentication is failed
    BLOCK_USIM_AUTHENTICATION_FAILED,
    /// aos boot-up hasn't been done yet.
    BLOCK_AOS_INCOMPLETED,
    /// cs call is active. ims may need to be de-registered while cs calling.
    BLOCK_CSCALL_STARTED,
    /// Permanent data failed
    BLOCK_PERMANENT_DATA_FAILED,
    /// no enablers are attached.
    BLOCK_ENABLER_DETACHED,
    /// ims is disabled.
    BLOCK_IMS_DISABLED,
    /// A permanent error (i.e. 403 forbidden) is received.
    BLOCK_PERMANENT_REG_FAILED,
    /// power off
    BLOCK_POWER_OFF,
    /// The service is not yet connected.
    BLOCK_SERVICE_CONNECTING,
    /// subscriber information is not available. i.e.) IMS credentials in ISIM are not ready.
    BLOCK_SUBSCRIBER_INCOMPLETED,
    /// TTY mode is turned on and a TTY device is connected.
    BLOCK_TTY_MODE_ON,
    /// PDN is block temporarily with timer
    BLOCK_TEMPORARY_DATA_DEACTIVATED,
    /// IMS service is disabled
    BLOCK_IMS_SERVICE_DISABLED,
    /// EPS fallback is started. registration is holding until LTE is attached.
    BLOCK_EPS_FALLBACK_STARTED,
    /// Can not obtain valid P-CSCF address.
    BLOCK_INVALID_CONNECTION,
    /// End common Block reason
    BLOCK_END = BLOCK_INVALID_CONNECTION,

    /// Start cellular Block reason
    BLOCK_CELLULAR_START,
    /// Airplane mode is enabled.
    BLOCK_CELLULAR_AIRPLANE_MODE_ON = BLOCK_CELLULAR_START,
    /// network is not appropriate for ims service.
    BLOCK_CELLULAR_NO_NETWORK,
    /// Current data network is not valid for ims services.
    BLOCK_CELLULAR_OUT_OF_SERVICE,
    /// The current RAT was blocked because registration failed for all P-CSCFs.
    BLOCK_CELLULAR_RAT_BLOCK,
    /// a roming network. volte may be not allowed in roaming area.
    BLOCK_CELLULAR_ROAMING,
    /// End cellular Block reason
    BLOCK_CELLULAR_END = BLOCK_CELLULAR_ROAMING,

    /// Start wifi Block reason
    BLOCK_WIFI_START,
    /// data connection of wifi ap is unavailable.
    BLOCK_WIFI_BAD_CONNECTION = BLOCK_WIFI_START,
    /// conuntry code unavailable.
    BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE,
    /// Airplane mode is enabled.
    BLOCK_WIFI_AIRPLANE_MODE_ON,
    /// wifi is not connected.
    BLOCK_WIFI_NO_WIFI,
    /// The registration is blocked when registration failed repeatedly in wifi.
    BLOCK_WIFI_REG_FORBIDDEN,
    /// wifi service is blocked temporarily
    BLOCK_WIFI_TEMPORARILY_BLOCKED,
    /// End wifi Block reason
    BLOCK_WIFI_END = BLOCK_WIFI_TEMPORARILY_BLOCKED,

    /// Max of block reason
    BLOCK_MAX
} BLOCK_REASON;

/// Types for service
typedef enum
{
    /// Type for cellular service
    SERVICE_CELLULAR,
    /// Type for wifi service
    SERVICE_WIFI,
    /// Type for cellular and wifi service
    SERVICE_WHOLE
} SERVICE_TYPE;

/**
 * @class IAosBlock
 * @brief Interface for managing and querying IMS service block states.
 */
class IAosBlock
{
public:
    virtual ~IAosBlock(){};

    /**
     * @brief Registers a listener for block state change notifications.
     *
     * @param piListener Pointer to the listener implementation.
     */
    virtual void SetListener(IN IAosBlockListener* piListener) = 0;

    /**
     * @brief Unregisters a block state change listener.
     *
     * @param piListener Pointer to the listener to be removed.
     */
    virtual void RemoveListener(IN IAosBlockListener* piListener) = 0;

    /**
     * @brief Sets an IAosBlockSilentListener to be notified of silent block reason updates.
     *
     * This method registers a listener that will be invoked when the block reason is changed
     * with the option to suppress block reason change notifications.
     *
     * @param piListener The listener to be added.
     */
    virtual void SetSilentListener(IN IAosBlockSilentListener* piListener) = 0;

    /**
     * @brief Removes an IAosBlockSilentListener.
     *
     * This method unregisters a previously registered listener, so it will no longer receive
     * notifications about silent block reason updates.
     *
     * @param piListener The listener to be removed.
     */
    virtual void RemoveSilentListener(IN IAosBlockSilentListener* piListener) = 0;

    /**
     * @brief Activates a specific block reason.
     *
     * Adds the reason to the internal block maps based on its category (Common,
     * Cellular, or WiFi). If bNotify is true, standard listeners are notified;
     * otherwise, silent listeners are notified.
     *
     * @param eReason The block reason to set.
     * @param bNotify If IMS_TRUE, notifies standard listeners. If IMS_FALSE, notifies silent
     *                listeners.
     * @return IMS_TRUE if the block state was changed; IMS_FALSE if the reason was already blocked.
     */
    virtual IMS_BOOL SetBlockReason(IN BLOCK_REASON eReason, IN IMS_BOOL bNotify = IMS_TRUE) = 0;

    /**
     * @brief Deactivates a specific block reason.
     *
     * Removes the reason from internal block maps. Notification logic follows the bNotify flag.
     *
     * @param eReason The block reason to reset.
     * @param bNotify If IMS_TRUE, notifies standard listeners. If IMS_FALSE, notifies silent
     *                listeners.
     * @return IMS_TRUE if the block state was changed; IMS_FALSE if the reason was not active.
     */
    virtual IMS_BOOL ResetBlockReason(IN BLOCK_REASON eReason, IN IMS_BOOL bNotify = IMS_TRUE) = 0;

    /**
     * @brief Clears all active block reasons for all service types.
     *
     * Resets all internal maps (Common, Cellular, and WiFi) and sends a notification.
     */
    virtual void ClearAllBlockReasons() = 0;

    /**
     * @brief Retrieves a formatted string listing all currently active block reasons.
     *
     * This method iterates through all block categories (Common, Cellular, and WiFi)
     * and appends the string representation of every enabled block reason to the output.
     * This is primarily used by the diagnostic reporter to capture the current blocking state.
     *
     * @param strOutLog [OUT] The string buffer to populate with the formatted block info.
     * Format Example: "Common(1):[AOS_INCOMPLETED] Cellular(0): WiFi(0):"
     */
    virtual void GetBlockReasonsString(OUT AString& strOutLog) = 0;

    /**
     * @brief Prints the currently active block reasons to the native trace log.
     *
     * @return IMS_TRUE if successful.
     */
    virtual IMS_BOOL PrintBlockReasons() = 0;

    /**
     * @brief Gets a list of currently active block reasons for a specific service type.
     *
     * @param objReasons [OUT] List to be populated with active block reason IDs.
     * @param eType Filter for the service type (CELLULAR, WIFI, or WHOLE).
     */
    virtual void GetBlockReasons(
            OUT ImsList<IMS_UINT32>& objReasons, IN SERVICE_TYPE eType = SERVICE_WHOLE) = 0;

    /**
     * @brief Checks if a specific reason is currently blocking the service.
     *
     * @param eReason The block reason to check.
     * @param bOnlyEnabled If IMS_TRUE, returns true only if the specified reason is the *only*
     *                     reason active for that service.
     * @param eType The service type context to check against.
     * @return IMS_TRUE if the reason is blocking; otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsReasonBlocked(IN BLOCK_REASON eReason, IN IMS_BOOL bOnlyEnabled = IMS_FALSE,
            IN SERVICE_TYPE eType = SERVICE_CELLULAR) = 0;

    /**
     * @brief Checks if the specified service is free of any block reasons.
     *
     * @param eType The service type to check (CELLULAR, WIFI, or WHOLE).
     * @return IMS_TRUE if no block reasons are active for the service; otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsCleared(IN SERVICE_TYPE eType = SERVICE_CELLULAR) = 0;
};
#endif  // INTERFACE_AOS_BLOCK_H_
