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
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.hardware.TriggerEventListener;
import android.location.LastLocationRequest;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationRequest;
import android.net.ConnectivityManager;
import android.net.ConnectivityManager.NetworkCallback;
import android.net.IpSecManager;
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
import android.telephony.CarrierConfigManager;
import android.telephony.CarrierConfigManager.CarrierConfigChangeListener;
import android.telephony.SmsManager;
import android.telephony.SubscriptionManager;
import android.telephony.SubscriptionManager.PhoneNumberSource;
import android.telephony.ims.ImsException;
import android.telephony.ims.ImsManager;
import android.telephony.ims.ImsMmTelManager;
import android.telephony.ims.ImsMmTelManager.WiFiCallingMode;
import android.telephony.ims.ProvisioningManager;
import android.telephony.ims.ProvisioningManager.FeatureProvisioningCallback;
import android.telephony.ims.feature.MmTelFeature;
import android.telephony.ims.stub.ImsConfigImplBase;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.util.ArrayMap;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.imsstack.util.Log;

import java.io.FileDescriptor;
import java.io.IOException;
import java.net.InetAddress;
import java.util.Arrays;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.Executor;
import java.util.function.Consumer;

/**
 * An implementation class to access the APIs of the various system managers.
 */
public class SystemServiceProxyImpl implements SystemServiceProxy {
    private final Context mContext;
    private final Map<Class<?>, Object> mManagerProxies = new ArrayMap<>();

    SystemServiceProxyImpl(@NonNull Context context) {
        mContext = context;

        mManagerProxies.put(TelephonyManagerProxy.class, new TelephonyManagerProxyImpl(mContext));
        mManagerProxies.put(CarrierConfigManagerProxy.class, new CarrierConfigManagerProxyImpl());
        mManagerProxies.put(SubscriptionManagerProxy.class, new SubscriptionManagerProxyImpl());
        mManagerProxies.put(ConnectivityManagerProxy.class, new ConnectivityManagerProxyImpl());
        mManagerProxies.put(IpSecManagerProxy.class, new IpSecManagerProxyImpl());
        mManagerProxies.put(LocationManagerProxy.class, new LocationManagerProxyImpl());
        mManagerProxies.put(SensorManagerProxy.class, new SensorManagerProxyImpl());
        mManagerProxies.put(SmsManagerProxy.class, new SmsManagerProxyImpl());
        mManagerProxies.put(ImsManagerProxy.class, new ImsManagerProxyImpl());
        mManagerProxies.put(TelecomManagerProxy.class, new TelecomManagerProxyImpl());
    }

    /**
     * Returns a specific system service corresponding to the given class.
     *
     * @param clazz A requested class name.
     * @return A system service object corresponding to the given class.
     */
    @Override
    @SuppressWarnings("unchecked")
    public <T> T getSystemService(Class<T> clazz) {
        Object proxy = mManagerProxies.get(clazz);
        if (proxy == null) {
            throw new IllegalArgumentException("Unknown system proxy: " + clazz);
        }
        return (T) proxy;
    }

    class CarrierConfigManagerProxyImpl implements CarrierConfigManagerProxy {
        @Override
        public boolean isConfigForIdentifiedCarrier(PersistableBundle bundle) {
            return CarrierConfigManager.isConfigForIdentifiedCarrier(bundle);
        }

        @Override
        public @NonNull PersistableBundle getDefaultConfig() {
            return CarrierConfigManager.getDefaultConfig();
        }

        @Override
        public @NonNull PersistableBundle getConfigForSubId(int subId, @NonNull String... keys) {
            CarrierConfigManager ccm = mContext.getSystemService(CarrierConfigManager.class);
            PersistableBundle b;
            try {
                b = ccm != null ? ccm.getConfigForSubId(subId, keys) : new PersistableBundle();
            } catch (Exception e) {
                Log.w(this, "getConfigForSubId: " + e + " for " + Arrays.toString(keys));
                b = new PersistableBundle();
            }
            return b;
        }

        @Override
        public void registerCarrierConfigChangeListener(
                @NonNull @CallbackExecutor Executor executor,
                @NonNull CarrierConfigChangeListener listener) {
            CarrierConfigManager ccm = mContext.getSystemService(CarrierConfigManager.class);
            if (ccm != null) {
                ccm.registerCarrierConfigChangeListener(executor, listener);
            }
        }

        @Override
        public void unregisterCarrierConfigChangeListener(
                @NonNull CarrierConfigChangeListener listener) {
            CarrierConfigManager ccm = mContext.getSystemService(CarrierConfigManager.class);
            if (ccm != null) {
                ccm.unregisterCarrierConfigChangeListener(listener);
            }
        }
    }

    class SubscriptionManagerProxyImpl implements SubscriptionManagerProxy {
        @Override
        public boolean isUsableSubscriptionId(int subId) {
            return SubscriptionManager.isUsableSubscriptionId(subId);
        }

        @Override
        public boolean isValidSubscriptionId(int subId) {
            return SubscriptionManager.isValidSubscriptionId(subId);
        }

        @Override
        public int getDefaultDataSubscriptionId() {
            return SubscriptionManager.getDefaultDataSubscriptionId();
        }

        @Override
        public int getSlotIndex(int subId) {
            return SubscriptionManager.getSlotIndex(subId);
        }

        @Override
        public int getSubscriptionId(int slotIndex) {
            return SubscriptionManager.getSubscriptionId(slotIndex);
        }

        @Override
        public @NonNull String getPhoneNumber(int subId, @PhoneNumberSource int source) {
            SubscriptionManager sm = mContext.getSystemService(SubscriptionManager.class);
            if (sm != null) {
                return sm.getPhoneNumber(subId, source);
            } else {
                throw new IllegalStateException("SubscriptionManager unavailable.");
            }
        }
    }

    class ConnectivityManagerProxyImpl implements ConnectivityManagerProxy {
        @Override
        public void registerQosCallback(@NonNull final QosSocketInfo socketInfo,
                @CallbackExecutor @NonNull final Executor executor,
                @NonNull final QosCallback callback) {
            ConnectivityManager cm = mContext.getSystemService(ConnectivityManager.class);
            if (cm != null) {
                cm.registerQosCallback(socketInfo, executor, callback);
            } else {
                throw new IllegalStateException("ConnectivityManager unavailable.");
            }
        }

        @Override
        public void unregisterQosCallback(@NonNull final QosCallback callback) {
            ConnectivityManager cm = mContext.getSystemService(ConnectivityManager.class);
            if (cm != null) {
                cm.unregisterQosCallback(callback);
            }
        }

        @Override
        public void registerNetworkCallback(@NonNull NetworkRequest request,
                @NonNull NetworkCallback networkCallback, @NonNull Handler handler) {
            ConnectivityManager cm = mContext.getSystemService(ConnectivityManager.class);
            if (cm != null) {
                cm.registerNetworkCallback(request, networkCallback, handler);
            } else {
                throw new IllegalStateException("ConnectivityManager unavailable.");
            }
        }

        @Override
        public void unregisterNetworkCallback(@NonNull NetworkCallback networkCallback) {
            ConnectivityManager cm = mContext.getSystemService(ConnectivityManager.class);
            if (cm != null) {
                cm.unregisterNetworkCallback(networkCallback);
            }
        }

        @Override
        public @Nullable LinkProperties getLinkProperties(@Nullable Network network) {
            ConnectivityManager cm = mContext.getSystemService(ConnectivityManager.class);
            return cm != null ? cm.getLinkProperties(network) : null;
        }

        @Override
        public void requestNetwork(@NonNull NetworkRequest request,
                @NonNull NetworkCallback networkCallback, @NonNull Handler handler) {
            ConnectivityManager cm = mContext.getSystemService(ConnectivityManager.class);
            if (cm != null) {
                cm.requestNetwork(request, networkCallback, handler);
            } else {
                throw new IllegalStateException("ConnectivityManager unavailable.");
            }
        }

        @Override
        public void registerSystemDefaultNetworkCallback(
                @NonNull NetworkCallback networkCallback, @NonNull Handler handler) {
            ConnectivityManager cm = mContext.getSystemService(ConnectivityManager.class);
            if (cm != null) {
                cm.registerSystemDefaultNetworkCallback(networkCallback, handler);
            } else {
                throw new IllegalStateException("ConnectivityManager unavailable.");
            }
        }
    }

    class IpSecManagerProxyImpl implements IpSecManagerProxy {
        @Override
        @NonNull
        public SecurityParameterIndex allocateSecurityParameterIndex(
                @NonNull InetAddress destinationAddress, int requestedSpi)
                throws SpiUnavailableException, ResourceUnavailableException {
            IpSecManager ism = mContext.getSystemService(IpSecManager.class);
            if (ism != null) {
                return ism.allocateSecurityParameterIndex(destinationAddress, requestedSpi);
            } else {
                throw new IllegalStateException("IpSecManager unavailable.");
            }
        }

        @Override
        public void applyTransportModeTransform(@NonNull FileDescriptor socket,
                @PolicyDirection int direction,
                @NonNull IpSecTransform transform) throws IOException {
            IpSecManager ism = mContext.getSystemService(IpSecManager.class);
            if (ism != null) {
                ism.applyTransportModeTransform(socket, direction, transform);
            } else {
                throw new IllegalStateException("IpSecManager unavailable.");
            }
        }

        @Override
        public void removeTransportModeTransforms(
                @NonNull FileDescriptor socket) throws IOException {
            IpSecManager ism = mContext.getSystemService(IpSecManager.class);
            if (ism != null) {
                ism.removeTransportModeTransforms(socket);
            }
        }
    }

    class LocationManagerProxyImpl implements LocationManagerProxy {
        @Override
        public boolean isProviderEnabled(@NonNull String provider) {
            LocationManager lm = mContext.getSystemService(LocationManager.class);
            return lm != null ? lm.isProviderEnabled(provider) : false;
        }

        @Override
        public void getCurrentLocation(@NonNull String provider,
                @NonNull LocationRequest locationRequest,
                @Nullable CancellationSignal cancellationSignal,
                @NonNull @CallbackExecutor Executor executor,
                @NonNull Consumer<Location> consumer) {
            LocationManager lm = mContext.getSystemService(LocationManager.class);
            if (lm != null) {
                try {
                    lm.getCurrentLocation(provider,
                            locationRequest, cancellationSignal, executor, consumer);
                } catch (SecurityException | IllegalArgumentException e) {
                    Log.e(this, "getCurrentLocation: " + e);
                    executor.execute(() -> consumer.accept(null));
                } catch (Exception e) {
                    Log.e(this, "getCurrentLocation: unknown exception=" + e);
                    executor.execute(() -> consumer.accept(null));
                }
            } else {
                executor.execute(() -> consumer.accept(null));
            }
        }

        @Override
        @Nullable
        public Location getLastKnownLocation(@NonNull String provider,
                @NonNull LastLocationRequest lastLocationRequest) {
            LocationManager lm = mContext.getSystemService(LocationManager.class);
            return lm != null ? lm.getLastKnownLocation(provider, lastLocationRequest) : null;
        }

        @Override
        public void requestLocationUpdates(@NonNull String provider,
                @NonNull LocationRequest locationRequest,
                @NonNull @CallbackExecutor Executor executor,
                @NonNull LocationListener listener) {
            LocationManager lm = mContext.getSystemService(LocationManager.class);
            if (lm != null) {
                lm.requestLocationUpdates(provider, locationRequest, executor, listener);
            }
        }

        @Override
        public void removeUpdates(@NonNull LocationListener listener) {
            LocationManager lm = mContext.getSystemService(LocationManager.class);
            if (lm != null) {
                lm.removeUpdates(listener);
            }
        }
    }

    class SensorManagerProxyImpl implements SensorManagerProxy {
        @Override
        public @Nullable Sensor getDefaultSensor(int type) {
            SensorManager sm = mContext.getSystemService(SensorManager.class);
            return sm != null ? sm.getDefaultSensor(type) : null;
        }

        @Override
        public boolean requestTriggerSensor(TriggerEventListener listener, Sensor sensor) {
            SensorManager sm = mContext.getSystemService(SensorManager.class);
            return sm != null ? sm.requestTriggerSensor(listener, sensor) : false;
        }

        @Override
        public boolean cancelTriggerSensor(TriggerEventListener listener, Sensor sensor) {
            SensorManager sm = mContext.getSystemService(SensorManager.class);
            return sm != null ? sm.cancelTriggerSensor(listener, sensor) : false;
        }
    }

    class SmsManagerProxyImpl implements SmsManagerProxy {
        private final SmsManager mSmsManager;

        SmsManagerProxyImpl() {
            mSmsManager = mContext.getSystemService(SmsManager.class);
        }

        SmsManagerProxyImpl(int subId) {
            SmsManager sm = mContext.getSystemService(SmsManager.class);
            if (sm != null) {
                mSmsManager = sm.createForSubscriptionId(subId);
            } else {
                mSmsManager = null;
                throw new IllegalStateException("SmsManager unavailable.");
            }
        }

        @Override
        public @NonNull SmsManagerProxy createForSubscriptionId(int subId) {
            return new SmsManagerProxyImpl(subId);
        }

        @Override
        public @Nullable String getSmscAddress() {
            return mSmsManager != null ? mSmsManager.getSmscAddress() : null;
        }

        @Override
        public @NonNull Uri getSmscIdentity() {
            return mSmsManager != null ? mSmsManager.getSmscIdentity() : Uri.EMPTY;
        }
    }

    class ImsManagerProxyImpl implements ImsManagerProxy {
        @Override
        public @Nullable ImsMmTelManagerProxy getImsMmTelManagerProxy(int subId) {
            try {
                return new ImsMmTelManagerProxyImpl(subId);
            } catch (IllegalArgumentException | IllegalStateException e) {
                // The subscription id is invalid or ImsManager unavailable.
                return null;
            }
        }

        @Override
        public @Nullable ProvisioningManagerProxy getProvisioningManagerProxy(int subId) {
            try {
                return new ProvisioningManagerProxyImpl(subId);
            } catch (IllegalArgumentException | IllegalStateException e) {
                // The subscription id is invalid or ImsManager unavailable.
                return null;
            }
        }
    }

    class ImsMmTelManagerProxyImpl implements ImsMmTelManagerProxy {
        private final ImsMmTelManager mMmTelManager;

        ImsMmTelManagerProxyImpl(int subId) {
            ImsManager im = mContext.getSystemService(ImsManager.class);
            if (im != null) {
                mMmTelManager = im.getImsMmTelManager(subId);
            } else {
                mMmTelManager = null;
                throw new IllegalStateException("ImsManager unavailable.");
            }
        }

        @Override
        public boolean isAdvancedCallingSettingEnabled() {
            return mMmTelManager.isAdvancedCallingSettingEnabled();
        }

        @Override
        public boolean isVtSettingEnabled() {
            return mMmTelManager.isVtSettingEnabled();
        }

        @Override
        public boolean isVoWiFiSettingEnabled() {
            return mMmTelManager.isVoWiFiSettingEnabled();
        }

        @Override
        public @WiFiCallingMode int getVoWiFiModeSetting() {
            return mMmTelManager.getVoWiFiModeSetting();
        }

        @Override
        public boolean isVoWiFiRoamingSettingEnabled() {
            return mMmTelManager.isVoWiFiRoamingSettingEnabled();
        }

        @Override
        public @WiFiCallingMode int getVoWiFiRoamingModeSetting() {
            return mMmTelManager.getVoWiFiRoamingModeSetting();
        }

        @Override
        public boolean isCrossSimCallingEnabled() {
            try {
                return mMmTelManager.isCrossSimCallingEnabled();
            } catch (ImsException e) {
                Log.d(this, "isCrossSimCallingEnabled: " + e);
                return false;
            }
        }

        @Override
        public void setCrossSimCallingEnabled(boolean isEnabled) {
            try {
                mMmTelManager.setCrossSimCallingEnabled(isEnabled);
            } catch (ImsException e) {
                Log.d(this, "setCrossSimCallingEnabled: " + e);
            }
        }
    }

    class ProvisioningManagerProxyImpl implements ProvisioningManagerProxy {
        private final ProvisioningManager mProvisioningManager;

        ProvisioningManagerProxyImpl(int subId) {
            ImsManager im = mContext.getSystemService(ImsManager.class);
            if (im != null) {
                mProvisioningManager = im.getProvisioningManager(subId);
            } else {
                mProvisioningManager = null;
                throw new IllegalStateException("ImsManager unavailable.");
            }
        }

        @Override
        public void registerFeatureProvisioningChangedCallback(
                @NonNull @CallbackExecutor Executor executor,
                @NonNull FeatureProvisioningCallback callback) {
            Objects.requireNonNull(executor, "executor must not be null");
            Objects.requireNonNull(callback, "callback must not be null");

            try {
                mProvisioningManager.registerFeatureProvisioningChangedCallback(
                        executor, callback);
            } catch (ImsException e) {
                Log.d(this, "registerFeatureProvisioningChangedCallback: " + e);
            }
        }

        @Override
        public void unregisterFeatureProvisioningChangedCallback(
                @NonNull FeatureProvisioningCallback callback) {
            Objects.requireNonNull(callback, "callback must not be null");

            try {
                mProvisioningManager.unregisterFeatureProvisioningChangedCallback(callback);
            } catch (IllegalArgumentException e) {
                Log.d(this, "unregisterFeatureProvisioningChangedCallback: " + e);
            }
        }

        @Override
        public @ImsConfigImplBase.SetConfigResult int setProvisioningIntValue(int key, int value) {
            return mProvisioningManager.setProvisioningIntValue(key, value);
        }

        @Override
        public boolean getProvisioningStatusForCapability(
                @MmTelFeature.MmTelCapabilities.MmTelCapability int capability,
                @ImsRegistrationImplBase.ImsRegistrationTech int tech) {
            return mProvisioningManager.getProvisioningStatusForCapability(capability, tech);
        }
    }

    class TelecomManagerProxyImpl implements TelecomManagerProxy {
        @Override
        public boolean isInEmergencyCall() {
            TelecomManager tm = mContext.getSystemService(TelecomManager.class);
            return tm != null ? tm.isInEmergencyCall() : false;
        }
    }
}
