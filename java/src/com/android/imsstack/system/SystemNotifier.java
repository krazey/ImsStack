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
package com.android.imsstack.system;

import java.util.HashSet;
import java.util.Set;

public abstract class SystemNotifier {
    private final Set<Integer> mRegisteredEvents = new HashSet<Integer>();

    public SystemNotifier() {
    }

    /**
     * Notifies the changes of airplane mode in the phone settings.
     *
     * @param airplaneMode the current airplane mode status 0: Airplane mode
     *            OFF, 1: Airplane mode ON
     */
    public abstract void notifyAirplaneModeChanged(final int airplaneMode);

    /**
     * Notifies the expiration of the timer.
     *
     * @param tid The timer id that was specified to start a timer.
     */
    public abstract void notifyTimerExpired(long tid);

    /**
     * Notifies the changes of the battery level.
     *
     * @param level the battery level (integer between 1 and 100)
     */
    public abstract void notifyBatteryLevelChanged(final int level);

    /**
     * Notifies the failure result to connect a data connection of the specified
     * APN type.
     *
     * @param apnType the APN type (1: ims, 2: internet, 3: xcap, 9: emergency,
     *            21: wifi)
     */
    public abstract void notifyDataConnectionFailed(final int apnType);

    /**
     * Notifies the IPCAN category of the attached data connection.
     *
     * @param apnType the APN type (1: ims, 2: internet, 3: xcap, 9: emergency,
     *            21: wifi)
     * @param ipcanCategory the IPCAN category (0: MOBILE, 1: WLAN); Refer to IIPCAN.h
     */
    public abstract void notifyDataConnectionIpcanChanged(final int apnType,
            final int ipcanCategory);

    /**
     * Notifies the state of the specified data connection.
     *
     * @param apnType the APN type (1: ims, 2: internet, 3: xcap, 9: emergency,
     *            21: wifi)
     * @param state the data state (TelephonyManager.DATA_*)
     *            {@link TelephonyManager.DATA_UNKNOWN} (-1)
     *            {@link TelephonyManager.DATA_DISCONNECTED} (0)
     *            {@link TelephonyManager.DATA_CONNECTING} (1)
     *            {@link TelephonyManager.DATA_CONNECTED} (2)
     *            {@link TelephonyManager.DATA_SUSPENDED} (3)
     */
    public abstract void notifyDataConnectionStateChanged(final int apnType,
        final int state);

    /**
     * Notifies the network type which the device is attached.
     *
     * @param networkType the network type (TelephonyManager.NETWORK_TYPE_*)
     *          {@link RAT_NONET} (0)
     *          {@link RAT_EHRPD} (1)
     *          {@link RAT_4G} (2)
     *          {@link RAT_3G} (3)
     *          {@link RAT_1XRTT} (4)
     *          {@link RAT_2G} (5)
     */
    public abstract void notifyNetworkTypeChanged(final int networkType);

    /**
     * Notifies the voice network type which the device is attached.
     *
     * @param networkType defined in DcNetWatcher
     *          {@link RAT_NONET} (0)
     *          {@link RAT_EHRPD} (1)
     *          {@link RAT_4G} (2)
     *          {@link RAT_3G} (3)
     *          {@link RAT_1XRTT} (4)
     *          {@link RAT_2G} (5)
     */
    public abstract void notifyVoiceNetworkTypeChanged(final int networkType);

    /**
     * Notifies the service state related to the attached network.
     *
     * @param serviceState the service state (ServiceState.STATE_*)
     *            {@link ServiceState.STATE_IN_SERVICE} (0)
     *            {@link ServiceState.STATE_OUT_OF_SERVICE} (1)
     *            {@link ServiceState.STATE_EMERGENCY_ONLY} (2)
     *            {@link ServiceState.STATE_POWER_OFF} (3)
     */
    public abstract void notifyServiceStateChanged(final int serviceState);

    /**
     * Notifies the voice call (CS / IMS) state.
     *
     * @param state the call state (TelephonyManager.CALL_STATE_*)
     *            {@link TelephonyManager.CALL_STATE_IDLE} (0)
     *            {@link TelephonyManager.CALL_STATE_RINGING} (1)
     *            {@link TelephonyManager.CALL_STATE_OFFHOOK} (2)
     */
    public abstract void notifyVoiceCallStateChanged(final int state);

    /**
     * Notifies the Wi-Fi setting state.
     *
     * @param state the Wi-Fi setting state.
     *            {@link WifiInterface#STATE_DISABLED}
     *            {@link WifiInterface#STATE_ENABLED}
     */
    public abstract void notifyWifiStateChanged(int state);

    /**
     * Notifies the Wi-Fi connection state.
     *
     * @param state the Wi-Fi connection state.
     *            {@link WifiInterface#CONNECTION_STATE_DISCONNECTED}
     *            {@link WifiInterface#CONNECTION_STATE_CONNECTED}
     */
    public abstract void notifyWifiConnectionStateChanged(int state);

    /**
     * Notifies the changes of the IMS configuration.
     *
     * @param configs the configuration items to be updated
     */
    public abstract void notifyConfigurationChanged(final int configs);

    /**
     * Notifies the events which are registered by the native modules.
     *
     * @param event the current event
     * @param param1 the parameter related to the current event
     * @param param2 the additional parameter related to the current event
     */
    public abstract void notifyEvent(final int event, final int param1, final int param2);

    /**
     * Notifies the ISIM state to the native module.
     *
     * @param event The current event.
     * @param state The current ISIM state.
     */
    public abstract void notifyIsimState(int event, String state);

    /**
     * Notifies the ISIM file attributes response to the native module.
     *
     * @param event The current event.
     * @param fileId The file id to be responded.
     * @param size The size of the specified file id.
     * @param values The content of the specified file id.
     */
    public abstract void notifyIsimFileAttributesResponse(int event,
            int fileId, int size, String[] values);

    /**
     * Notifies the ISIM record response to the native module.
     *
     * @param event The current event.
     * @param fileId The file id to be responded.
     * @param index The index of the specified file id.
     * @param value The content of the specified file id.
     */
    public abstract void notifyIsimRecordResponse(int event,
            int fileId, int index, String value);

    /**
     * Notifies the ISIM authentication response to the native module.
     *
     * @param event The current event.
     * @param response The ISIM authentication response.
     * @param owner The owner of this request.
     */
    public abstract void notifyIsimAuthenticationResponse(int event, String response, long owner);

    /**
     * Notifies the USIM authentication response to the native module.
     *
     * @param event The current event.
     * @param response The USIM authentication response.
     * @param owner The owner of this request.
     */
    public abstract void notifyUsimAuthenticationResponse(int event, String response, long owner);

    /**
     * Checks if the specified event is registered or not.
     *
     * @param event the event to be evaluated
     * @return true if the specified event is registered. Otherwise, false
     */
    public boolean isEventRegistered(final int event) {
        synchronized (mRegisteredEvents) {
            return mRegisteredEvents.contains(event);
        }
    }

    /**
     * Registers the event.
     *
     * @param event the event to be registered
     */
    public void registerEvent(final int event) {
        synchronized (mRegisteredEvents) {
            mRegisteredEvents.add(event);
        }
    }

    /**
     * Unregisters the event.
     *
     * @param event the event to be unregistered
     */
    public void unregisterEvent(final int event) {
        synchronized (mRegisteredEvents) {
            mRegisteredEvents.remove(event);
        }
    }
}
