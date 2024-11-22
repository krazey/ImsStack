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
#ifndef INTERFACE_PHONE_INFO_CALL_H_
#define INTERFACE_PHONE_INFO_CALL_H_

#include "AString.h"

class ICallInfo
{
protected:
    virtual ~ICallInfo() = default;

public:
    /// TTY mode
    enum
    {
        TTY_MODE_OFF = 0,
        TTY_MODE_FULL = 1,
        TTY_MODE_HCO = 2,
        TTY_MODE_VCO = 3
    };

    /// RTT mode
    enum
    {
        RTT_MODE_NONE = -1,
        RTT_MODE_CAPABLE_OFF = 0,
        RTT_MODE_VISIBLE_DURING_CALLS = 1,
        RTT_MODE_ALWAYS_VISIBLE = 2
    };

    /// Wi-Fi calling mode
    enum
    {
        WFC_MODE_WFC_ONLY = 0,
        WFC_MODE_CELLULAR_PREFERRED = 1,
        WFC_MODE_WFC_PREFERRED = 2
    };

public:
    /**
     * @brief Checks if a number is an emergency number or not.
     *
     * @param strNumber The number to be checked
     * @return IMS_TRUE if the specified number is an emergency number, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsEmergencyNumber(IN const AString& strNumber) const = 0;

    /**
     * @brief Returns the TTY mode.
     *
     * @return The TTY mode setting.\n
     *         #TTY_MODE_OFF\n
     *         #TTY_MODE_FULL\n
     *         #TTY_MODE_HCO\n
     *         #TTY_MODE_VCO
     */
    virtual IMS_UINT32 GetTtyMode() const = 0;

    /**
     * @brief Returns the RTT mode.
     *
     * @return The RTT mode setting.\n
     *         #RTT_MODE_NONE\n
     *         #RTT_MODE_CAPABLE_OFF\n
     *         #RTT_MODE_VISIBLE_DURING_CALLS\n
     *         #RTT_MODE_ALWAYS_VISIBLE
     */
    virtual IMS_UINT32 GetRttMode() const = 0;

    /**
     * @brief Checks if the Wi-Fi calling is enabled or not.
     *
     * @return IMS_TRUE if the Wi-Fi calling is enabled, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsWifiCallingEnabled() = 0;

    /**
     * @brief Returns the Wi-Fi calling mode.
     *
     * @return The Wi-Fi calling mode.\n
     *         #WFC_MODE_WFC_ONLY\n
     *         #WFC_MODE_CELLULAR_PREFERRED\n
     *         #WFC_MODE_WFC_PREFERRED
     */
    virtual IMS_UINT32 GetWifiCallingPreferences() = 0;

    /**
     * @brief Checks if the Wi-Fi calling is provisioned or not.
     *
     * @return IMS_TRUE if the Wi-Fi calling is provisioned, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsWifiCallingProvisioned() = 0;

    /**
     * @brief Returns the Wi-Fi calling address identifier (emergency address id).
     *
     * @return The Wi-Fi calling address identifier.
     *         If the Wi-Fi calling is provisioned, it returns non-Null string.
     */
    virtual AString GetWifiCallingAddressId() = 0;

    /**
     * @brief Returns the CS call state on the other slot.
     *
     * @return The call state.
     */
    virtual IMS_SINT32 GetCsCallStateInOtherSlot() const = 0;

    /**
     * @brief Excluding the current slot, the SIM state of other slots is checked to determine and
     * return the availability of cross SIM redialing.
     *
     * @return {@code true} if cross SIM redialing is available, {@code false} otherwise.
     */
    virtual IMS_BOOL IsCrossSimRedialingAvailable() const = 0;
};

#endif
