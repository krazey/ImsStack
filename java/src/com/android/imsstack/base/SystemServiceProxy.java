/*
 * Copyright (C) 2023 The Android Open Source Project
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
package com.android.imsstack.base;

import android.annotation.CallbackExecutor;
import android.hardware.Sensor;
import android.hardware.TriggerEventListener;
import android.location.LastLocationRequest;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationRequest;
import android.net.ConnectivityManager.NetworkCallback;
import android.net.IpSecManager.ResourceUnavailableException;
import android.net.IpSecManager.SecurityParameterIndex;
import android.net.IpSecManager.SpiUnavailableException;
import android.net.IpSecTransform;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkRequest;
import android.net.QosCallback;
import android.net.QosSocketInfo;
import android.net.Uri;
import android.net.annotations.PolicyDirection;
import android.os.CancellationSignal;
import android.os.Handler;
import android.os.PersistableBundle;
import android.telecom.TelecomManager;
import android.telephony.CarrierConfigManager.CarrierConfigChangeListener;
import android.telephony.SubscriptionManager.PhoneNumberSource;
import android.telephony.ims.ImsMmTelManager.WiFiCallingMode;
import android.telephony.ims.ProvisioningManager.FeatureProvisioningCallback;
import android.telephony.ims.feature.MmTelFeature;
import android.telephony.ims.stub.ImsConfigImplBase;
import android.telephony.ims.stub.ImsRegistrationImplBase;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.io.FileDescriptor;
import java.io.IOException;
import java.net.InetAddress;
import java.util.concurrent.Executor;
import java.util.function.Consumer;

/**
 * A proxy interface to access the APIs of the various system managers.
 * The following managers are handled by this interface:
 *     {@link android.telephony.CarrierConfigManager}
 *     {@link android.telephony.SubscriptionManager}
 *     {@link android.net.ConnectivityManager}
 *     {@link android.net.IpSecManager}
 *     {@link android.location.LocationManager}
 *     {@link android.hardware.SensorManager}
 *     {@link android.telecom.TelecomManager}
 *     {@link android.telephony.SmsManager}
 *     {@link android.telephony.ims.ImsManager}
 *     {@link android.telephony.ims.ImsMmTelManager}
 *     {@link android.telephony.ims.ProvisioningManager}
 */
public interface SystemServiceProxy {
    /**
     * Returns a specific system service corresponding to the given class.
     *
     * @param clazz A requested class name.
     * @return A system service object corresponding to the given class.
     */
    <T> T getSystemService(Class<T> clazz);

    /**
     * A proxy interface for accessing the {@link CarrierConfigManager} class.
     */
    interface CarrierConfigManagerProxy {
        /**
         * Determines whether a configuration {@link PersistableBundle} obtained from
         * {@link #getConfigForSubId(int, String...)} corresponds to an identified carrier.
         *
         * @param bundle The configuration bundle to be checked.
         * @return {@code true} if any carrier specific configuration bundle has been applied,
         *         {@code false} otherwise or the bundle is null.
         */
        boolean isConfigForIdentifiedCarrier(PersistableBundle bundle);

        /**
         * Returns a new bundle with the default value for every supported configuration variable.
         */
        @NonNull PersistableBundle getDefaultConfig();

        /**
         * Gets the configuration values of the specified keys for a particular subscription.
         *
         * If an invalid subId is used, the returned configuration will contain default values for
         * the specified keys. If the value for the key can't be found, the returned configuration
         * will filter the key out.
         *
         * After using this method to get the configuration bundle,
         * {@link #isConfigForIdentifiedCarrier(PersistableBundle)} should be called to confirm
         * whether any carrier specific configuration has been applied.
         *
         * @param subId The subscription ID on which the carrier config should be retrieved.
         * @param keys The carrier config keys to retrieve values.
         * @return A {@link PersistableBundle} with key/value mapping for the specified
         *         configuration on success, or an empty (but never null) bundle on failure
         *         (for example, when the calling app has no permission).
         */
        @NonNull PersistableBundle getConfigForSubId(int subId, @NonNull String... keys);

        /**
         * Registers a {@link CarrierConfigManager#CarrierConfigChangeListener} to get
         * a notification when carrier configurations have changed.
         *
         * @param executor The executor on which the listener will be called.
         * @param listener The CarrierConfigChangeListener called when carrier configs has changed.
         */
        void registerCarrierConfigChangeListener(
                @NonNull @CallbackExecutor Executor executor,
                @NonNull CarrierConfigChangeListener listener);

        /**
         * Unregisters the {@link CarrierConfigManager#CarrierConfigChangeListener} to stop
         * notification on carrier configurations change.
         *
         * @param listener The {@link CarrierConfigManager#CarrierConfigChangeListener} which was
         *                 previously registered.
         */
        void unregisterCarrierConfigChangeListener(@NonNull CarrierConfigChangeListener listener);
    }

    /**
     * A proxy interface for accessing the {@link SubscriptionManager} class.
     */
    interface SubscriptionManagerProxy {
        /**
         * Check if the supplied subscription id is usable.
         *
         * @param subId the subscription id.
         * @return {@code true} if the subscription id is usable; {@code false} otherwise.
         */
        boolean isUsableSubscriptionId(int subId);

        /**
         * Check if the supplied subscription id is valid.
         *
         * @param subId The subscription id.
         * @return {@code true} if the subscription id is valid; {@code false} otherwise.
         */
        boolean isValidSubscriptionId(int subId);

        /**
         * Returns the system's default data subscription id.
         *
         * On a voice only device or on error, will return INVALID_SUBSCRIPTION_ID.
         *
         * @return The default data subscription id.
         */
        int getDefaultDataSubscriptionId();

        /**
         * Returns the slot index associated with the subscription.
         *
         * @param subId The subscription id.
         * @return The slot index as a positive integer or
         *         {@link SubscriptionManager#INVALID_SIM_SLOT_INDEX} if the supplied
         *         subscription id doesn't have an associated slot index.
         */
        int getSlotIndex(int subId);

        /**
         * Returns the subscription id for specified logical SIM slot index.
         *
         * @param slotIndex The logical SIM slot index.
         * @return The subscription id. Returns
         *         {@link SubscriptionManager#INVALID_SUBSCRIPTION_ID} if SIM is absent.
         */
        int getSubscriptionId(int slotIndex);

        /**
         * Returns the phone number for the given {@code subId}, or an empty string if
         * not available.
         *
         * @param subId The subscription id.
         * @param source The source of the phone number, one of the PHONE_NUMBER_SOURCE_* constants.
         * @return The phone number, or an empty string if not available.
         */
        @NonNull String getPhoneNumber(int subId, @PhoneNumberSource int source);
    }

    /**
     * A proxy interface for accessing the {@link ConnectivityManager} class.
     */
    interface ConnectivityManagerProxy {
        /**
         * Registers a {@link QosSocketInfo} with an associated {@link QosCallback}.
         * The callback will receive available QoS events related to the {@link Network} and
         * local ip + port specified within socketInfo.
         *
         * Exceptions after the time of registration is passed through
         * {@link QosCallback#onError(QosCallbackException)}.  see: {@link QosCallbackException}.
         *
         * @param socketInfo The socket information used to match QoS events.
         * @param executor The executor on which the callback will be invoked. The provided
         *                 {@link Executor} must run callback sequentially, otherwise the order of
         *                 callbacks cannot be guaranteed.onQosCallbackRegistered.
         * @param callback The callback receives qos events that satisfy socketInfo.
         */
        void registerQosCallback(@NonNull QosSocketInfo socketInfo,
                @CallbackExecutor @NonNull Executor executor, @NonNull QosCallback callback);

        /**
         * Unregisters the given {@link QosCallback}. The {@link QosCallback} will no longer receive
         * events once unregistered and can be registered a second time.
         *
         * @param callback The callback being unregistered.
         */
        void unregisterQosCallback(@NonNull QosCallback callback);

        /**
         * Registers to receive notifications about all networks which satisfy the given
         * {@link NetworkRequest}.  The callbacks will continue to be called until
         * either the application exits or {@link #unregisterNetworkCallback(NetworkCallback)} is
         * called.
         *
         * @param request {@link NetworkRequest} describing this request.
         * @param networkCallback The {@link NetworkCallback} that the system will call as suitable
         *                        networks change state.
         * @param handler {@link Handler} to specify the thread upon which the callback will be
         *                invoked.
         */
        void registerNetworkCallback(@NonNull NetworkRequest request,
                @NonNull NetworkCallback networkCallback, @NonNull Handler handler);

        /**
         * Unregisters a {@code NetworkCallback} and possibly releases networks originating from
         * {@link #requestNetwork(NetworkRequest, NetworkCallback)} and
         * {@link #registerNetworkCallback(NetworkRequest, NetworkCallback, Handler)} calls.
         * If the given {@code NetworkCallback} had previously been used with
         * {@code #requestNetwork}, any networks that the device brought up only to satisfy
         * that request will be disconnected.
         *
         * Notifications that would have triggered that {@code NetworkCallback} will immediately
         * stop triggering it as soon as this call returns.
         *
         * @param networkCallback The {@link NetworkCallback} used when making the request.
         */
        void unregisterNetworkCallback(@NonNull NetworkCallback networkCallback);

        /**
         * Returns the {@link LinkProperties} for the given {@link Network}.  This
         * will return {@code null} if the network is unknown.
         *
         * @param network The {@link Network} object identifying the network in question.
         * @return The {@link LinkProperties} for the network, or {@code null}.
         */
        @Nullable LinkProperties getLinkProperties(@Nullable Network network);

        /**
         * Request a network to satisfy a set of {@link android.net.NetworkCapabilities}.
         *
         * @param request {@link NetworkRequest} describing this request.
         * @param networkCallback The {@link NetworkCallback} to be utilized for this request.
         *                        Note the callback must not be shared - it uniquely specifies this
         *                        request.
         * @param handler {@link Handler} to specify the thread upon which the callback will be
         *                invoked.
         */
        void requestNetwork(@NonNull NetworkRequest request,
                @NonNull NetworkCallback networkCallback, @NonNull Handler handler);

        /**
         * Registers to receive notifications about changes in the application's default network.
         * This may be a physical network or a virtual network, such as a VPN that applies to the
         * application. The callbacks will continue to be called until either the application exits
         * or {@link #unregisterNetworkCallback(NetworkCallback)} is called.
         *
         * @param networkCallback The {@link NetworkCallback} that the system will call as the
         *     application's default network changes.
         * @param handler {@link Handler} to specify the thread upon which the callback will be
         *     invoked.
         */
        void registerSystemDefaultNetworkCallback(
                @NonNull NetworkCallback networkCallback, @NonNull Handler handler);
    }

    /**
     * A proxy interface for accessing the {@link IpSecManager} class.
     */
    interface IpSecManagerProxy {
        /**
         * Reserve the requested SPI for traffic bound to or from the specified destination address.
         *
         * If successful, this SPI is guaranteed available until released by a call to {@link
         * SecurityParameterIndex#close()}.
         *
         * @param destinationAddress The destination address for traffic bearing the requested SPI.
         *                           For inbound traffic, the destination should be an address
         *                           currently assigned on-device.
         * @param requestedSpi The requested SPI. The range 1-255 is reserved and may not be used.
         *                     See RFC 4303 Section 2.1.
         * @return The reserved {@link SecurityParameterIndex} object.
         * @throws ResourceUnavailableException indicating that too many SPIs are
         *     currently allocated for this user
         * @throws SpiUnavailableException indicating that the requested SPI could not be
         *     reserved
         */
        @NonNull SecurityParameterIndex allocateSecurityParameterIndex(
                @NonNull InetAddress destinationAddress, int requestedSpi)
                throws SpiUnavailableException, ResourceUnavailableException;

        /**
         * Applies an IPsec transform to a socket.
         *
         * @param socket A socket file descriptor.
         * @param direction The direction in which the transform should be applied.
         * @param transform A transport mode {@code IpSecTransform}.
         * @throws IOException Indicating that the transform could not be applied
         */
        void applyTransportModeTransform(@NonNull FileDescriptor socket,
                @PolicyDirection int direction,
                @NonNull IpSecTransform transform) throws IOException;

        /**
         * Removes an IPsec transform from a socket.
         *
         * @param socket A socket that previously had a transform applied to it.
         * @throws IOException indicating that the transform could not be removed from the socket.
         */
        void removeTransportModeTransforms(@NonNull FileDescriptor socket) throws IOException;
    }

    /**
     * A proxy interface for accessing the {@link LocationManager} class.
     */
    interface LocationManagerProxy {
        /**
         * Returns the current enabled/disabled status of the given provider.
         * To listen for changes, see {@link LocationManager#PROVIDERS_CHANGED_ACTION}.
         *
         * @param provider The provider listed by {@link LocationManager#getAllProviders()}.
         * @return {@code true} if the provider exists and is enabled, {@code false} otherwise.
         */
        boolean isProviderEnabled(@NonNull String provider);

        /**
         * Asynchronously returns a single current location fix from the given provider based on
         * the given {@link LocationRequest}. This may activate sensors in order to compute
         * a new location, unlike {@link #getLastKnownLocation(String)}, which will only return
         * a cached fix if available. The given callback will be invoked once and only once,
         * either with a valid location or with a null location if the provider was unable to
         * generate a valid location.
         *
         * @param provider A provider listed by {@link LocationManager#getAllProviders()}.
         * @param locationRequest The location request containing location parameters.
         * @param cancellationSignal An optional signal that allows for cancelling this call
         * @param executor The executor handling listener callbacks.
         * @param consumer The callback invoked with either a {@link Location} or null
         */
        void getCurrentLocation(@NonNull String provider,
                @NonNull LocationRequest locationRequest,
                @Nullable CancellationSignal cancellationSignal,
                @NonNull @CallbackExecutor Executor executor,
                @NonNull Consumer<Location> consumer);

        /**
         * Gets the last known location from the given provider, or null if there is no last known
         * location. The returned location may be quite old in some circumstances, so the age of
         * the location should always be checked.
         *
         * @param provider The provider listed by {@link LocationManager#getAllProviders()}.
         * @param lastLocationRequest The last location request containing location parameters.
         * @return The last known location for the given provider, or null if not available
         */
        @Nullable Location getLastKnownLocation(@NonNull String provider,
                @NonNull LastLocationRequest lastLocationRequest);

        /**
         * Registers for location updates from the specified provider, using a
         * {@link LocationRequest}, and a callback on the specified {@link Executor}.
         *
         * @param provider A provider listed by {@link LocationManager#getAllProviders()}.
         * @param locationRequest The location request containing location parameters.
         * @param executor The executor handling listener callbacks.
         * @param listener The listener to receive location updates.
         */
        void requestLocationUpdates(@NonNull String provider,
                @NonNull LocationRequest locationRequest,
                @NonNull @CallbackExecutor Executor executor,
                @NonNull LocationListener listener);

        /**
         * Removes all location updates for the specified {@link LocationListener}.
         * The given listener is guaranteed not to receive any invocations that
         * <b>happens-after</b> this method is invoked.
         *
         * @param listener The listener that no longer needs location updates.
         */
        void removeUpdates(@NonNull LocationListener listener);
    }

    /**
     * A proxy interface for accessing the {@link SensorManager} class.
     */
    interface SensorManagerProxy {
        /**
         * Uses this method to get the default sensor for a given type. Note that the
         * returned sensor could be a composite sensor, and its data could be
         * averaged or filtered. If you need to access the raw sensors use
         * {@link SensorManager#getSensorList(int) getSensorList}.
         *
         * @param type The sensor type to request.
         * @return The default sensor matching the requested type if one exists and the application
         *         has the necessary permissions, or null otherwise.
         */
        @Nullable Sensor getDefaultSensor(int type);

        /**
         * Requests receiving trigger events for a trigger sensor.
         *
         * @param listener The listener on which the
         *                 {@link TriggerEventListener#onTrigger(TriggerEvent)} will be delivered.
         * @param sensor The sensor to be enabled.
         * @return {@code true} if the sensor was successfully enabled, {@code false} otherwise.
         */
        boolean requestTriggerSensor(TriggerEventListener listener, Sensor sensor);

        /**
         * Cancels receiving trigger events for a trigger sensor.
         *
         * @param listener The listener on which the
         *                 {@link TriggerEventListener#onTrigger(TriggerEvent)} is delivered.
         *                 It should be the same as the one used in
         *                 {@link #requestTriggerSensor(TriggerEventListener, Sensor)}.
         * @param sensor The sensor for which the trigger request should be canceled.
         *               If null, it cancels receiving trigger for all sensors associated
         *               with the listener.
         * @return {@code true} if successfully canceled, {@code false} otherwise.
         */
        boolean cancelTriggerSensor(TriggerEventListener listener, Sensor sensor);
    }

    /**
     * A proxy interface for accessing the {@link SmsManager} class.
     */
    interface SmsManagerProxy {
        /**
         * Returns the instance of the SmsManager associated with a particular subscription id.
         *
         * @param subId The subscription id.
         * @return The instance of the SmsManager associated with subscription.
         */
        @NonNull SmsManagerProxy createForSubscriptionId(int subId);

        /**
         * Returns the SMSC address from (U)SIM.
         *
         * @return The SMSC address string, null if failed.
         */
        @Nullable String getSmscAddress();

        /**
         * Fetches the EF_PSISMSC value from the UICC that contains the Public Service Identity of
         * the SM-SC (either a SIP URI or tel URI). The EF_PSISMSC of ISIM and USIM can be found in
         * DF_TELECOM.
         * The EF_PSISMSC value is used by the ME to submit SMS over IP as defined in 24.341 [55].
         *
         * @return Uri : Public Service Identity of SM-SC from the ISIM or USIM if the ISIM is not
         *         available.
         */
        @NonNull Uri getSmscIdentity();
    }

    /**
     * A proxy interface for accessing the {@link ImsManager} class.
     */
    interface ImsManagerProxy {
        /**
         * Creates an instance of {@link ImsMmTel} for the subscription id specified.
         *
         * @param subId The subscription id.
         * @return A {@link ImsMmTel} instance with the specific subscription id.
         */
        @Nullable ImsMmTelManagerProxy getImsMmTelManagerProxy(int subId);

        /**
         * Creates an instance of {@link Provisioning} for the subscription id specified.
         *
         * @param subId The subscription id.
         * @return A {@link Provisioning} instance with the specific subscription id.
         */
        @Nullable ProvisioningManagerProxy getProvisioningManagerProxy(int subId);
    }

    /**
     * A proxy interface for accessing the {@link ImsMmTelManager} class.
     */
    interface ImsMmTelManagerProxy {
        /**
         * Queries the user’s setting for “Advanced Calling” or "Enhanced 4G LTE", which is used to
         * enable MmTel IMS features, depending on the carrier configuration for the current
         * subscription. If this setting is enabled, IMS voice and video telephony over IWLAN/LTE
         * will be enabled as long as the carrier has provisioned these services for the specified
         * subscription. Other IMS services (SMS/UT) are not affected by this user setting and
         * depend on carrier requirements.
         *
         * Note: If the carrier configuration for advanced calling is not editable or hidden, this
         * method will always return the default value.
         *
         * @return {@code true} if the user's setting for advanced calling is enabled,
         *         {@code false} otherwise.
         */
        boolean isAdvancedCallingSettingEnabled();

        /**
         * The user's setting for whether or not they have enabled the "Video Calling" setting.
         *
         * Note: If the carrier configuration for advanced calling is not editable or hidden, this
         * method will always return the default value.
         *
         * @return {@code true} if the user’s “Video Calling” setting is currently enabled,
         *         {@code false} otherwise.
         */
        boolean isVtSettingEnabled();

        /**
         * Checks whether the user's setting for Voice over WiFi is enabled or not.
         *
         * @return {@code true} if the user's setting for Voice over WiFi is enabled,
         *         {@code false} otherwise.
         */
        boolean isVoWiFiSettingEnabled();

        /**
         * Returns the user's voice over WiFi Roaming mode setting associated with the device.
         *
         * @return The Voice over WiFi Mode preference set by the user, which can be one of the
         *         following:
         * - {@link ImsMmTelManager#WIFI_MODE_WIFI_ONLY}
         * - {@link ImsMmTelManager#WIFI_MODE_CELLULAR_PREFERRED}
         * - {@link ImsMmTelManager#WIFI_MODE_WIFI_PREFERRED}
         */
        @WiFiCallingMode int getVoWiFiModeSetting();

        /**
         * Returns the user's voice over WiFi roaming setting associated with the current
         * subscription.
         *
         * @return {@code true} if the user's setting for Voice over WiFi while roaming is enabled,
         *         {@code false} otherwise.
         */
        boolean isVoWiFiRoamingSettingEnabled();

        /**
         * Returns the user's preference for Voice over WiFi calling mode while the device is
         * roaming on another network.
         *
         * @return The user's preference for the technology to register for IMS over when roaming
         *         on another network, can be one of the following:
         *         - {@link ImsMmTelManager#WIFI_MODE_WIFI_ONLY}
         *         - {@link ImsMmTelManager#WIFI_MODE_CELLULAR_PREFERRED}
         *         - {@link ImsMmTelManager#WIFI_MODE_WIFI_PREFERRED}
         */
        @WiFiCallingMode int getVoWiFiRoamingModeSetting();

        /**
         * Checks whether the cross SIM calling is enabled or not.
         *
         * @return {@code true} if the user's setting for Voice over Cross SIM is enabled
         *         {@code false} otherwise.
         */
        boolean isCrossSimCallingEnabled();

        /**
         * Sets the user's setting for whether or not Voice over Cross SIM is enabled.
         *
         * NOTE: This is used for test purpose to verify the cross SIM calling.
         *
         * @param isEnabled true if the user's setting for Voice over Cross SIM is enabled,
         *                  false otherwise
         */
        void setCrossSimCallingEnabled(boolean isEnabled);
    }

    /**
     * A proxy interface for accessing the {@link ProvisioningManager} class.
     */
    interface ProvisioningManagerProxy {
        /**
         * Register a new {@link FeatureProvisioningCallback}, which is used to listen for
         * IMS feature provisioning updates.
         *
         * @param executor The executor that the callback methods will be called on.
         * @param callback The callback instance being registered.
         * @throws NullPointerException if the executor or the callback is null.
         */
        void registerFeatureProvisioningChangedCallback(
                @NonNull @CallbackExecutor Executor executor,
                @NonNull FeatureProvisioningCallback callback);

        /**
         * Unregisters a previously registered {@link FeatureProvisioningCallback}
         * instance. When the subscription associated with this
         * callback is removed (SIM removed, ESIM swap, etc...), this callback will
         * automatically be removed. If this method is called for an inactive
         * subscription, it will result in a no-op.
         *
         * @param callback The existing {@link FeatureProvisioningCallback} to be removed.
         * @throws NullPointerException if the callback is null.
         */
        void unregisterFeatureProvisioningChangedCallback(
                @NonNull FeatureProvisioningCallback callback);

        /**
         * Sets the integer value associated with the provided key.
         *
         * This operation is blocking and should not be performed on the UI thread.
         *
         * @param key An integer that represents the provisioning key, which is defined by the OEM.
         * @param value An integer value for the provided key.
         * @return The result of setting the configuration value.
         */
        @ImsConfigImplBase.SetConfigResult int setProvisioningIntValue(int key, int value);

        /**
         * Get the provisioning status for the IMS MmTel capability specified.
         *
         * @param capability The MMTEL capability that provisioning is requested for.
         * @param tech The IMS registration technology associated with the MMTEL capability that
         *             provisioning status is requested for.
         */
        boolean getProvisioningStatusForCapability(
                @MmTelFeature.MmTelCapabilities.MmTelCapability int capability,
                @ImsRegistrationImplBase.ImsRegistrationTech int tech);
    }

    /**
     * A proxy interface for accessing the {@link TelecomManager} class.
     */
    interface TelecomManagerProxy {
        /**
         * Determines if there is an ongoing emergency call.
         * This can be either an outgoing emergency call, as identified by the dialed number, or
         * because a call was identified by the network as an emergency call.
         *
         * @return {@code true} if there is an ongoing emergency call, {@code false} otherwise.
         */
        boolean isInEmergencyCall();
    }
}
