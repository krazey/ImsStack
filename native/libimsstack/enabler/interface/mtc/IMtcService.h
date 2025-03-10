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

#ifndef INTERFACE_MTC_SERVICE_H_
#define INTERFACE_MTC_SERVICE_H_

#include "INativeEnabler.h"
#include "ImsTypeDef.h"

class AString;
class ICoreService;
class IJniMtcServiceThread;
class IMtcAosConnector;
class IMtcAosStateListener;
class IMtcNetworkWatcherListener;
class ISrvccStateListener;
class ISsacTimerHandler;
enum class ServiceStatus;
enum class ServiceType;
enum class SrvccState;
enum class SuppStatus;

class IMtcService : public INativeEnabler
{
public:
    virtual ~IMtcService() {}

    /**
     * @brief Gets the service type.
     *
     * @return The service type.
     */
    virtual ServiceType GetServiceType() const = 0;

    /**
     * @brief Adds a listener for AOS state changes.
     *
     * @param piListener The listener to add.
     */
    virtual void AddAosStateListener(IN IMtcAosStateListener* piListener) = 0;

    /**
     * @brief Removes a listener for AOS state changes.
     *
     * @param piListener The listener to remove.
     */
    virtual void RemoveAosStateListener(IN IMtcAosStateListener* piListener) = 0;

    /**
     * @brief Removes a listener for SRVCC state changes.
     *
     * @param piListener The listener to remove.
     */
    virtual void AddSrvccStateListener(IN ISrvccStateListener* piListener) = 0;

    /**
     * @brief Removes a listener for SRVCC state changes.
     *
     * @param piListener The listener to remove.
     */
    virtual void RemoveSrvccStateListener(IN ISrvccStateListener* piListener) = 0;

    /**
     * @brief Adds a listener for network watcher events.
     *
     * @param piListener The listener to add.
     */
    virtual void AddNetworkWatcherListener(IN IMtcNetworkWatcherListener* piListener) = 0;

    /**
     * @brief Removes a listener for network watcher events.
     *
     * @param piListener The listener to remove.
     */
    virtual void RemoveNetworkWatcherListener(IN IMtcNetworkWatcherListener* piListener) = 0;

    /**
     * @brief Gets the current RAT type.
     *
     * @return The current RAT type.
     */
    virtual IMS_SINT32 GetRatType() const = 0;

    /**
     * @brief Gets the current mobile RAT type.
     *
     * @return The current mobile RAT type.
     */
    virtual IMS_SINT32 GetMobileRatType() const = 0;

    /**
     * @brief Gets the last connected RAT type.
     *
     * @return The last connected RAT type.
     */
    virtual IMS_SINT32 GetLastConnectedRatType() const = 0;

    /**
     * @brief Checks if the service is active.
     *
     * @return True if the service is active.
     */
    virtual IMS_BOOL IsActive() const = 0;

    /**
     * @brief Checks if the service is in emergency mode.
     *
     * @return True if the service is in emergency mode.
     */
    virtual IMS_BOOL IsEmergency() const = 0;

    /**
     * @brief Checks if the UE is currently in NR.
     *
     * It reads the device's current radio state. So it could be different from the RAT which this
     * service is in.
     *
     * @return True if it's in NR.
     */
    virtual IMS_BOOL IsNr() const = 0;

    /**
     * @brief Checks if the UE is currently in LTE and has EPS only attach type.
     *
     * @return True if it's in LTE and has combined attach type.
     */
    virtual IMS_BOOL IsEpsOnlyAttach() const = 0;

    /**
     * @brief Checks if the UE is currently in LTE and has combined attach type.
     *
     * @return True if it's in LTE and has combined attach type.
     */
    virtual IMS_BOOL IsEpsCombinedAttach() const = 0;

    /**
     * @brief Checks if the UE is currently in a roaming network.
     *
     * @return True if it's in a roaming network.
     */
    virtual IMS_BOOL IsRoaming() const = 0;

    /**
     * @brief Checks if the UE is currently registered through an ePDG.
     *
     * @return True if it's in ePDG.
     */
    virtual IMS_BOOL IsWlanIpCanType() const = 0;

    /**
     * @brief Checks if CSFB is available based on the UE's network status and carrier
     *        configuration.
     *
     * This method evaluates the UE's network conditions, including NR, EPS Attach Type, Roaming,
     * and WiFi status, against the CSFB block conditions specified in the carrier configuration
     * {@link ConfigVoice#KEY_CSFB_BLOCK_CONDITION_INT_ARRAY}.
     *
     * @return True if CSFB is available based on the network status and carrier configuration;
     *         False otherwise.
     */
    virtual IMS_BOOL IsCsfbAvailable() const = 0;

    /**
     * @brief Checks if the service is connected using CST(Cross-SIM calling Tech).
     *
     * @return True if CST.
     */
    virtual IMS_BOOL IsCrossSimConnected() const = 0;

    /**
     * @brief Gets the old service status.
     *
     * @return The old service status.
     */
    virtual ServiceStatus GetOldStatus() const = 0;

    /**
     * @brief Gets the current service status.
     *
     * @return The current service status.
     */
    virtual ServiceStatus GetStatus() const = 0;

    /**
     * @brief Gets the ICoreService object.
     *
     * @return The ICoreService object.
     */
    virtual ICoreService* GetICoreService() const = 0;

    /**
     * @brief Gets the IMtcAosConnector object.
     *
     * @return The IMtcAosConnector object.
     */
    virtual IMtcAosConnector* GetAosConnector() const = 0;

    /**
     * @brief Gets the IJniMtcServiceThread object.
     *
     * @return The IJniMtcServiceThread object.
     */
    virtual IJniMtcServiceThread* GetJniServiceThread() const = 0;

    /**
     * @brief Gets the current SRVCC state.
     *
     * @return The current SRVCC state.
     */
    virtual SrvccState GetSrvccState() const = 0;

    /**
     * @brief Updates the SRVCC state.
     *
     * @param eState The new SRVCC state.
     */
    virtual void UpdateSrvccState(IN SrvccState eState) = 0;

    /**
     * @brief Opens an emergency service.
     *
     * @param eServiceType The type of emergency service to open.
     */
    virtual void OpenEmergencyService(IN ServiceType eServiceType) = 0;

    /**
     * @brief Stops an emergency service.
     */
    virtual void StopEmergencyService() = 0;

    /**
     * @brief Processes a test command.
     *
     * @param nCommand The command to process.
     * @param nWParam The first parameter for the command.
     * @param nLParam The second parameter for the command.
     */
    virtual void ProcessTestCommand(
            IN IMS_SINT32 nCommand, IN IMS_SINT32 nWParam, IN IMS_SINT32 nLParam) = 0;

    /**
     * @brief Sets the terminal-based call waiting (TBCW) status.
     *
     * @param bEnabled True to enable TBCW, false to disable it.
     */
    virtual void SetTerminalBasedCallWaiting(IN IMS_BOOL bEnabled) = 0;

    /**
     * @brief Gets the terminal-based call waiting (TBCW) status.
     *
     * @return The TBCW status.
     */
    virtual SuppStatus GetTbcwStatus() const = 0;

    /**
     * @brief Sets a value for the Terminal based TIR.
     */
    virtual void SetTerminalBasedTir(IN IMS_BOOL bEnabled) = 0;

    /**
     * @brief Gets a value for the Terminal based TIR.
     *
     * Currently, this value is always SuppStatus::UNPROVISIONED.
     * It will be modified when the Terminal-based TIR is actually implemented.
     * Please refer to a comment in ag/30608488.
     */
    virtual SuppStatus GetTirStatus() const = 0;

    /**
     * @brief Gets the ISsacTimerHandler object.
     *
     * @return The ISsacTimerHandler object.
     */
    virtual ISsacTimerHandler& GetSsacTimerHandler() = 0;
};

enum class ServiceStatus
{
    SERVICE_IDLE,
    SERVICE_ACTIVE,
    SERVICE_SUSPENDED,
};

enum class ServiceType
{
    UNKNOWN = 0,
    NORMAL = 1 << 0,
    EMERGENCY = 1 << 1,
};

enum class SuppStatus
{
    UNPROVISIONED,
    PROVISIONED_ENABLED,
    PROVISIONED_DISABLED,
};

#endif
