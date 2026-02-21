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

import android.annotation.NonNull;
import android.os.Parcel;
import android.telephony.ims.ProvisioningManager;
import android.telephony.ims.stub.ImsConfigImplBase;
import android.util.ArraySet;
import android.util.SparseArray;

import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.core.agents.LocationInterface;
import com.android.imsstack.core.agents.WifiInterface;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDcUtils;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.internal.imsservice.MmTelFeatureRegistry;
import com.android.imsstack.jni.JniImsProxy;
import com.android.imsstack.jni.JniObjectId;
import com.android.imsstack.jni.JniSystemListener;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MessageExecutor;
import com.android.internal.annotations.VisibleForTesting;

import java.io.FileDescriptor;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * This class provides the system interfaces to communicate with native and Java layer.
 */
public class SystemInterface implements JniSystemListener {
    private static final Map<Integer, String> METHOD_TO_STRING = Map.ofEntries(
            Map.entry(SystemConstants.SET_TIMER, "SET_TIMER"),
            Map.entry(SystemConstants.KILL_TIMER, "KILL_TIMER"),
            Map.entry(SystemConstants.GET_UUID, "GET_UUID"),
            Map.entry(SystemConstants.GET_BATTERY_LEVEL, "GET_BATTERY_LEVEL"),
            Map.entry(SystemConstants.IS_EMERGENCY_NUMBER, "IS_EMERGENCY_NUMBER"),
            Map.entry(SystemConstants.GET_TTY_MODE, "GET_TTY_MODE"),
            Map.entry(SystemConstants.IS_IMS_VOICE_CALL_SUPPORTED, "IS_IMS_VOICE_CALL_SUPPORTED"),
            Map.entry(SystemConstants.REQUEST_NETWORK, "REQUEST_NETWORK"),
            Map.entry(SystemConstants.RELEASE_NETWORK, "RELEASE_NETWORK"),
            Map.entry(SystemConstants.GET_ACCESS_NETWORK_INFO, "GET_ACCESS_NETWORK_INFO"),
            Map.entry(SystemConstants.GET_APN_NAME, "GET_APN_NAME"),
            Map.entry(SystemConstants.GET_DATA_CONNECTION_STATE, "GET_DATA_CONNECTION_STATE"),
            Map.entry(SystemConstants.GET_HOST_BY_NAME, "GET_HOST_BY_NAME"),
            Map.entry(SystemConstants.GET_IFACE_ID, "GET_IFACE_ID"),
            Map.entry(SystemConstants.GET_IFACE_NAME, "GET_IFACE_NAME"),
            Map.entry(SystemConstants.GET_IPCAN_CATEGORY, "GET_IPCAN_CATEGORY"),
            Map.entry(SystemConstants.GET_LAST_ACCESS_NETWORK_INFO, "GET_LAST_ACCESS_NETWORK_INFO"),
            Map.entry(SystemConstants.GET_LOCAL_ADDRESS, "GET_LOCAL_ADDRESS"),
            Map.entry(SystemConstants.GET_PCSCF_ADDRESSES, "GET_PCSCF_ADDRESSES"),
            Map.entry(SystemConstants.GET_ROAMING_STATE, "GET_ROAMING_STATE"),
            Map.entry(SystemConstants.GET_SERVICE_STATE, "GET_SERVICE_STATE"),
            Map.entry(SystemConstants.IS_EMERGENCY_ONLY, "IS_EMERGENCY_ONLY"),
            Map.entry(SystemConstants.IS_MOBILE_DATA_ENABLED, "IS_MOBILE_DATA_ENABLED"),
            Map.entry(SystemConstants.GET_VOICE_SERVICE_STATE, "GET_VOICE_SERVICE_STATE"),
            Map.entry(SystemConstants.GET_VOICE_ROAMING_TYPE, "GET_VOICE_ROAMING_TYPE"),
            Map.entry(SystemConstants.GET_DATA_ROAMING_TYPE, "GET_DATA_ROAMING_TYPE"),
            Map.entry(SystemConstants.GET_MTU, "GET_MTU"),
            Map.entry(SystemConstants.IS_EMERGENCY_ATTACH_SUPPORTED,
                    "IS_EMERGENCY_ATTACH_SUPPORTED"),
            Map.entry(SystemConstants.BIND_SOCKET, "BIND_SOCKET"),
            Map.entry(SystemConstants.IS_IPV6_PREFERRED, "IS_IPV6_PREFERRED"),
            Map.entry(SystemConstants.GET_NETWORK_REGISTRATION_REJECT_CAUSE,
                    "GET_NETWORK_REGISTRATION_REJECT_CAUSE"),
            Map.entry(SystemConstants.GET_CELLULAR_SERVICE_STATE, "GET_CELLULAR_SERVICE_STATE"),
            Map.entry(SystemConstants.GET_ACCESS_NETWORK_PLMN, "GET_ACCESS_NETWORK_PLMN"),
            Map.entry(SystemConstants.GET_PREFERENCE, "GET_PREFERENCE"),
            Map.entry(SystemConstants.SET_PREFERENCE, "SET_PREFERENCE"),
            Map.entry(SystemConstants.GET_PRIVATE_PROPERTY, "GET_PRIVATE_PROPERTY"),
            Map.entry(SystemConstants.SET_PRIVATE_PROPERTY, "SET_PRIVATE_PROPERTY"),
            Map.entry(SystemConstants.GET_CARRIER_CONFIG, "GET_CARRIER_CONFIG"),
            Map.entry(SystemConstants.SEND_EVENT, "SEND_EVENT"),
            Map.entry(SystemConstants.GET_ISIM_STATE, "GET_ISIM_STATE"),
            Map.entry(SystemConstants.GET_ISIM_RECORD, "GET_ISIM_RECORD"),
            Map.entry(SystemConstants.REQUEST_ISIM_AUTH, "REQUEST_ISIM_AUTH"),
            Map.entry(SystemConstants.REQUEST_USIM_AUTH, "REQUEST_USIM_AUTH"),
            Map.entry(SystemConstants.GET_NETWORK_TYPE, "GET_NETWORK_TYPE"),
            Map.entry(SystemConstants.GET_VOICE_NETWORK_TYPE, "GET_VOICE_NETWORK_TYPE"),
            Map.entry(SystemConstants.GET_CS_CALL_STATE, "GET_CS_CALL_STATE"),
            Map.entry(SystemConstants.GET_DEVICE_ID, "GET_DEVICE_ID"),
            Map.entry(SystemConstants.GET_DEVICE_NAME, "GET_DEVICE_NAME"),
            Map.entry(SystemConstants.GET_DEVICE_SOFTWARE_VERSION, "GET_DEVICE_SOFTWARE_VERSION"),
            Map.entry(SystemConstants.GET_EXTERNAL_STORAGE_PATH, "GET_EXTERNAL_STORAGE_PATH"),
            Map.entry(SystemConstants.GET_PHONE_NUMBER, "GET_PHONE_NUMBER"),
            Map.entry(SystemConstants.GET_SUBSCRIBER_ID, "GET_SUBSCRIBER_ID"),
            Map.entry(SystemConstants.GET_SIM_MCC, "GET_SIM_MCC"),
            Map.entry(SystemConstants.GET_SIM_MNC, "GET_SIM_MNC"),
            Map.entry(SystemConstants.GET_SIM_COUNTRY_ISO, "GET_SIM_COUNTRY_ISO"),
            Map.entry(SystemConstants.GET_NETWORK_COUNTRY_ISO, "GET_NETWORK_COUNTRY_ISO"),
            Map.entry(SystemConstants.GET_NETWORK_OPERATOR, "GET_NETWORK_OPERATOR"),
            Map.entry(SystemConstants.GET_WIFI_STATE, "GET_WIFI_STATE"),
            Map.entry(SystemConstants.GET_WIFI_CONNECTION_STATE, "GET_WIFI_CONNECTION_STATE"),
            Map.entry(SystemConstants.GET_WIFI_BSS_ID, "GET_WIFI_BSS_ID"),
            Map.entry(SystemConstants.GET_WIFI_SSID, "GET_WIFI_SSID"),
            Map.entry(SystemConstants.IS_WFC_ENABLED, "IS_WFC_ENABLED"),
            Map.entry(SystemConstants.GET_WFC_PREFERENCES, "GET_WFC_PREFERENCES"),
            Map.entry(SystemConstants.IS_WFC_PROVISIONED, "IS_WFC_PROVISIONED"),
            Map.entry(SystemConstants.GET_WFC_ADDRESS_ID, "GET_WFC_ADDRESS_ID"),
            Map.entry(SystemConstants.START_LISTENING_FOR_LOCATION, "START_LISTENING_FOR_LOCATION"),
            Map.entry(SystemConstants.STOP_LISTENING_FOR_LOCATION, "STOP_LISTENING_FOR_LOCATION"),
            Map.entry(SystemConstants.GET_LAST_KNOWN_LOCATION, "GET_LAST_KNOWN_LOCATION"),
            Map.entry(SystemConstants.REQUEST_LOCATION_UPDATE, "REQUEST_LOCATION_UPDATE"),
            Map.entry(SystemConstants.CANCEL_LOCATION_UPDATE, "CANCEL_LOCATION_UPDATE"),
            Map.entry(SystemConstants.SET_EVENT, "SET_EVENT"),
            Map.entry(SystemConstants.RESET_EVENT, "RESET_EVENT"),
            Map.entry(SystemConstants.GET_CS_CALL_STATE_IN_OTHER_SLOT,
                    "GET_CS_CALL_STATE_IN_OTHER_SLOT"),
            Map.entry(SystemConstants.ADD_IPSEC_SA_PARAMETER, "ADD_IPSEC_SA_PARAMETER"),
            Map.entry(SystemConstants.REMOVE_IPSEC_SA_PARAMETER, "REMOVE_IPSEC_SA_PARAMETER"),
            Map.entry(SystemConstants.APPLY_IPSEC_SA, "APPLY_IPSEC_SA"),
            Map.entry(SystemConstants.REMOVE_IPSEC_SA, "REMOVE_IPSEC_SA"),
            Map.entry(SystemConstants.START_IMS_TRAFFIC, "START_IMS_TRAFFIC"),
            Map.entry(SystemConstants.STOP_IMS_TRAFFIC, "STOP_IMS_TRAFFIC"),
            Map.entry(SystemConstants.TRIGGER_EPS_FALLBACK, "TRIGGER_EPS_FALLBACK"),
            Map.entry(SystemConstants.SET_TRAFFIC_PRIORITY, "SET_TRAFFIC_PRIORITY"),
            Map.entry(SystemConstants.IS_CROSS_SIM_REDIALING_AVAILABLE,
                    "IS_CROSS_SIM_REDIALING_AVAILABLE"));

    private static SystemInterface sSystemInterface = null;
    private long mNativeObject = 0;
    private final SparseArray<ImsSystem> mSystems = new SparseArray<>(2);
    private final MessageExecutor mDefaultExecutor;
    private DefaultSystemCallInterface mDefaultSystemCall;

    private SystemInterface() {
        this(new MessageExecutor(SystemInterface.class.getSimpleName()));
    }

    @VisibleForTesting
    protected SystemInterface(@NonNull MessageExecutor executor) {
        mDefaultExecutor = executor;
    }

    /**
     * Returns a singleton System interface.
     */
    public static SystemInterface getInstance() {
        if (sSystemInterface == null) {
            sSystemInterface = new SystemInterface();
        }
        return sSystemInterface;
    }

    /**
     * Sets a System interface for testing.
     *
     * @param systemInterface A new System interface
     */
    @VisibleForTesting
    public static void setSystemInterface(SystemInterface systemInterface) {
        sSystemInterface = systemInterface;
    }

    /**
     * Initializes the system interface.
     */
    public void init() {
        ImsLog.i("ImsSystem#init: nativeObject=" + mNativeObject);

        if (mNativeObject == 0) {
            mNativeObject = JniImsProxy.getInterface(
                    JniObjectId.SYSTEM, MSimUtils.DEFAULT_SLOT_ID);
            JniImsProxy.setSystemListener(mNativeObject, this);
        }
    }

    /**
     * Clears all the resources of the system interface.
     */
    public void cleanup() {
        ImsLog.i("ImsSystem#cleanup: nativeObject=" + mNativeObject);

        if (mNativeObject != 0) {
            JniImsProxy.removeSystemListener(mNativeObject);
            JniImsProxy.releaseInterface(mNativeObject);
            mNativeObject = 0;
        }

        mDefaultExecutor.removeCallbacksAndMessages(null);
    }

    private ImsSystem getSystemForSlot(int slotId) {
        synchronized (mSystems) {
            return mSystems.get(slotId);
        }
    }

    /**
     * Returns the {@link ISystem} object for the specified slot.
     *
     * @param slotId The slot id.
     * @return A {@link ISystem} object or null.
     */
    public ISystem getSystem(int slotId) {
        return getSystemForSlot(slotId);
    }

    /**
     * Starts the {@link ISystem} object for the specified slot.
     *
     * @param slotId The slot id.
     * @param executor The message executor.
     * @param mmTelFeatureRegistry The MmTel feature registry.
     */
    @VisibleForTesting
    protected void start(int slotId, MessageExecutor executor,
            @NonNull MmTelFeatureRegistry mmTelFeatureRegistry) {
        Objects.requireNonNull(mmTelFeatureRegistry, "mmTelFeatureRegistry must not be null");
        ImsLog.d(slotId, "ImsSystem" + slotId + " is started.");
        ImsSystem system = getSystemForSlot(slotId);

        if (system == null) {
            system = new ImsSystem(slotId, executor, mmTelFeatureRegistry);
            synchronized (mSystems) {
                mSystems.put(slotId, system);
            }
        }
    }

    /**
     * Starts the {@link ISystem} object for the specified slot.
     *
     * @param slotId The slot id.
     */
    public void start(int slotId) {
        start(slotId, null, ImsServiceRegistry.getInstance(slotId).getMmTelFeatureRegistry());
    }

    /**
     * Stops the {@link ISystem} object for the specified slot.
     *
     * @param slotId The slot id.
     */
    public void stop(int slotId) {
        ImsLog.d(slotId, "ImsSystem" + slotId + " is stopped.");
        ImsSystem system = getSystemForSlot(slotId);

        if (system != null) {
            system.dispose();
        }
        synchronized (mSystems) {
            mSystems.remove(slotId);
        }
    }

    /**
     * Sets the {@link DefaultSystemCallInterface} instance for accessing the system APIs.
     *
     * @param systemCall The system call interface to be set.
     */
    public void setSystemCallInterface(DefaultSystemCallInterface systemCall) {
        mDefaultSystemCall = systemCall;
    }

    /**
     * Notifies the low battery state for all the systems.
     */
    public void notifyLowBatteryStateChanged() {
        synchronized (mSystems) {
            for (int i = 0; i < mSystems.size(); ++i) {
                ImsSystem system = mSystems.get(i);
                if (system != null) {
                    system.notifyEvent(ImsEventDef.IMS_EVENT_POWER_LOW_BATTERY,
                            ImsEventDef.IMS_POWER_LOW_CHANGED, 0);
                }
            }
        }
    }

    /**
     * Notifies the low battery state for all the systems.
     */
    public void notifyLowBatteryState() {
        synchronized (mSystems) {
            for (int i = 0; i < mSystems.size(); ++i) {
                ImsSystem system = mSystems.get(i);
                if (system != null) {
                    system.notifyEvent(ImsEventDef.IMS_EVENT_POWER_LOW_BATTERY,
                            ImsEventDef.IMS_POWER_LOW_BATTERY, 0);
                }
            }
        }
    }

    /**
     * Notifies the low battery state for the specified slot.
     *
     * @param slotId The slot id.
     */
    public void notifyLowBatteryState(int slotId) {
        ImsSystem system = getSystemForSlot(slotId);
        if (system != null) {
            system.notifyEvent(ImsEventDef.IMS_EVENT_POWER_LOW_BATTERY,
                    ImsEventDef.IMS_POWER_LOW_BATTERY, 0);
        }
    }

    /**
     * Notifies the expiration of the timer.
     *
     * @param tid The timer id that was specified to start a timer.
     */
    public void notifyTimerExpired(long tid) {
        mDefaultExecutor.execute(() -> {
            Parcel parcel = Parcel.obtain();
            try {
                parcel.writeInt(MSimUtils.DEFAULT_SLOT_ID);
                parcel.writeInt(SystemConstants.NOTIFY_TIMER_EXPIRED);
                parcel.writeLong(tid);
                sendSystemEvent(parcel);
            } finally {
                parcel.recycle();
            }
        });
    }

    /**
     * Notifies the changes of the battery level.
     *
     * @param level the battery level (integer between 1 and 100)
     */
    public void notifyBatteryLevelChanged(int level) {
        mDefaultExecutor.execute(() -> {
            Parcel parcel = Parcel.obtain();
            try {
                parcel.writeInt(MSimUtils.DEFAULT_SLOT_ID);
                parcel.writeInt(SystemConstants.NOTIFY_BATTERY_LEVEL_CHANGED);
                parcel.writeInt(level);
                sendSystemEvent(parcel);
            } finally  {
                parcel.recycle();
            }
        });
    }

    /**
     * Notifies the state of the WiFi module.
     *
     * @param state the state of the WiFi module
     *              {@link WifiInterface#STATE_DISABLED}
     *              {@link WifiInterface#STATE_ENABLED}
     */
    public void notifyWifiStateChanged(int state) {
        mDefaultExecutor.execute(() -> {
            Parcel parcel = Parcel.obtain();
            try {
                parcel.writeInt(MSimUtils.DEFAULT_SLOT_ID);
                parcel.writeInt(SystemConstants.NOTIFY_WIFI_STATE_CHANGED);
                parcel.writeInt(state);
                sendSystemEvent(parcel);
            } finally {
                parcel.recycle();
            }
        });
    }

    /**
     * Notifies the Wi-Fi connection state.
     *
     * @param state the Wi-Fi connection state.
     *              {@link WifiInterface#CONNECTION_STATE_DISCONNECTED}
     *              {@link WifiInterface#CONNECTION_STATE_CONNECTED}
     */
    public void notifyWifiConnectionStateChanged(int state) {
        mDefaultExecutor.execute(() -> {
            Parcel parcel = Parcel.obtain();
            try {
                parcel.writeInt(MSimUtils.DEFAULT_SLOT_ID);
                parcel.writeInt(SystemConstants.NOTIFY_WIFI_CONNECTION_STATE_CHANGED);
                parcel.writeInt(state);
                sendSystemEvent(parcel);
            } finally {
                parcel.recycle();
            }
        });
    }

    @Override
    public byte[] onMessage(Parcel parcel, FileDescriptor fd) {
        Parcel out = Parcel.obtain();
        try {
            handleMessage(parcel, fd, out);
            if (out.dataSize() != 0) {
                byte[] result = out.marshall();
                return result;
            }
        } catch (Throwable t) {
            ImsLog.i("ImsSystem#onMessage: " + t.toString());
            t.printStackTrace();
        } finally {
            out.recycle();
        }
        return JniImsProxy.RESULT_FAILURE;
    }

    private void handleMessage(Parcel in, FileDescriptor fd, Parcel out) {
        int slotId = in.readInt();
        int method = in.readInt();

        if (ImsLog.DBG) {
            ImsLog.d(slotId, "ImsSystem: " + METHOD_TO_STRING.get(method));
        }

        if (!handleDefaultSystemCall(method, in, out)) {
            // Initializes the data position and consumes the slot and method parameter
            // when the Parcel is modified by the handling of the default system call.
            in.setDataPosition(0);
            slotId = in.readInt();
            method = in.readInt();

            ImsSystem system = getSystemForSlot(slotId);

            if (system != null) {
                system.handleSystemCall(method, in, fd, out);
            }
        }

        if (out.dataSize() == 0) {
            ImsLog.d(slotId, "ImsSystem: "
                    + (ImsLog.DBG ? METHOD_TO_STRING.get(method) : Integer.toString(method))
                    + " is not handled.");
        }
    }

    private boolean handleDefaultSystemCall(int method, Parcel in, Parcel out) {
        if (mDefaultSystemCall == null) {
            return false;
        }

        switch (method) {
            case SystemConstants.SET_TIMER: // fall through
            case SystemConstants.KILL_TIMER:
                handleSystemCallForTimer(method, in, out);
                break;
            case SystemConstants.GET_PREFERENCE: // fall through
            case SystemConstants.SET_PREFERENCE:
                handleSystemCallForPreference(method, in, out);
                break;
            case SystemConstants.GET_WIFI_STATE: // fall through
            case SystemConstants.GET_WIFI_CONNECTION_STATE: // fall through
            case SystemConstants.GET_WIFI_BSS_ID: // fall through
            case SystemConstants.GET_WIFI_SSID:
                handleSystemCallForWifi(method, out);
                break;
            case SystemConstants.GET_UUID: // fall through
            case SystemConstants.GET_BATTERY_LEVEL:
                handleSystemCallForCommon(method, in, out);
                break;
            case SystemConstants.GET_DEVICE_NAME: // fall through
            case SystemConstants.GET_EXTERNAL_STORAGE_PATH:
                handleSystemCallForDevice(method, out);
                break;
            case SystemConstants.GET_PRIVATE_PROPERTY: // fall through
            case SystemConstants.SET_PRIVATE_PROPERTY:
                handleSystemCallForConfiguration(method, in, out);
                break;
            case SystemConstants.SEND_EVENT:
                return handleSystemCallForEvent(method, in, out);
            case SystemConstants.SET_TRAFFIC_PRIORITY:
                handleSystemCallForRadio(method, in, out);
                break;
            case SystemConstants.IS_CROSS_SIM_REDIALING_AVAILABLE:
                handleSystemCallForCrossSimRedialing(method, in, out);
                break;
            default:
                return false;
        }

        return true;
    }

    private void handleSystemCallForTimer(int method, Parcel in, Parcel out) {
        if (method == SystemConstants.SET_TIMER) {
            long duration = in.readLong();
            long tid = in.readLong();
            out.writeInt(mDefaultSystemCall.startTimer(tid, duration) ? 1 : 0);
        } else if (method == SystemConstants.KILL_TIMER) {
            long tid = in.readLong();
            mDefaultSystemCall.stopTimer(tid);
            out.writeInt(1);
        }
    }

    private void handleSystemCallForPreference(int method, Parcel in, Parcel out) {
        if (method == SystemConstants.GET_PREFERENCE) {
            String fileName = in.readString();
            String key = in.readString();
            int slotId = in.readInt();
            out.writeString(mDefaultSystemCall.getPreference(fileName, key, slotId));
        } else if (method == SystemConstants.SET_PREFERENCE) {
            String fileName = in.readString();
            String key = in.readString();
            String value = in.readString();
            int slotId = in.readInt();
            out.writeInt(mDefaultSystemCall.setPreference(fileName, key, value, slotId) ? 1 : 0);
        }
    }

    private void handleSystemCallForWifi(int method, Parcel out) {
        WifiInterface wifi = mDefaultSystemCall.getWifiInterface();
        if (wifi != null) {
            if (method == SystemConstants.GET_WIFI_STATE) {
                out.writeInt(wifi.isWifiEnabled()
                        ? WifiInterface.STATE_ENABLED
                        : WifiInterface.STATE_DISABLED);
            } else if (method == SystemConstants.GET_WIFI_CONNECTION_STATE) {
                out.writeInt(wifi.isWifiConnected()
                        ? WifiInterface.CONNECTION_STATE_CONNECTED
                        : WifiInterface.CONNECTION_STATE_DISCONNECTED);
            } else if (method == SystemConstants.GET_WIFI_BSS_ID) {
                out.writeString(wifi.getBssId());
            } else if (method == SystemConstants.GET_WIFI_SSID) {
                out.writeString(wifi.getSsId());
            }
        } else {
            if (method == SystemConstants.GET_WIFI_STATE) {
                out.writeInt(WifiInterface.STATE_DISABLED);
            } else if (method == SystemConstants.GET_WIFI_CONNECTION_STATE) {
                out.writeInt(WifiInterface.CONNECTION_STATE_DISCONNECTED);
            } else if (method == SystemConstants.GET_WIFI_BSS_ID) {
                out.writeString("");
            } else if (method == SystemConstants.GET_WIFI_SSID) {
                out.writeString("");
            }
        }
    }

    private void handleSystemCallForCommon(int method, Parcel in, Parcel out) {
        if (method == SystemConstants.GET_UUID) {
            int version = in.readInt();
            String name = (version == 3) ? in.readString() : null;
            out.writeString(mDefaultSystemCall.getUuid(version, name));
        } else if (method == SystemConstants.GET_BATTERY_LEVEL) {
            out.writeInt(mDefaultSystemCall.getBatteryLevel());
        }
    }

    private void handleSystemCallForDevice(int method, Parcel out) {
        if (method == SystemConstants.GET_DEVICE_NAME) {
            out.writeString(mDefaultSystemCall.getDeviceName());
        } else if (method == SystemConstants.GET_EXTERNAL_STORAGE_PATH) {
            out.writeString(mDefaultSystemCall.getExternalStoragePath());
        }
    }

    private void handleSystemCallForConfiguration(int method, Parcel in, Parcel out) {
        if (method == SystemConstants.GET_PRIVATE_PROPERTY) {
            int persistent = in.readInt();
            String key = in.readString();
            int slotId = in.readInt();

            if (persistent == 1) {
                out.writeString(ImsPrivateProperties.Persistent.get(key, "", slotId));
            } else {
                out.writeString(ImsPrivateProperties.Ephemeral.get(key, "", slotId));
            }
        } else if (method == SystemConstants.SET_PRIVATE_PROPERTY) {
            int persistent = in.readInt();
            String key = in.readString();
            String value = in.readString();
            int slotId = in.readInt();

            if (persistent == 1) {
                ImsPrivateProperties.Persistent.set(key, value, slotId);
            } else {
                ImsPrivateProperties.Ephemeral.set(key, value, slotId);
            }
            out.writeInt(1);
        }
    }

    private boolean handleSystemCallForEvent(int method, Parcel in, Parcel out) {
        if (method == SystemConstants.SEND_EVENT) {
            int event = in.readInt();
            int param1 = in.readInt();
            int param2 = in.readInt();

            if (event == ImsEventDef.IMS_EVENT_WAKE_LOCK) {
                mDefaultSystemCall.acquireWakeLock(param2);
                out.writeInt(1);
                return true;
            } else if (event == ImsEventDef.IMS_EVENT_WIFI_SERVICE) {
                WifiInterface wifi = mDefaultSystemCall.getWifiInterface();
                if (wifi != null) {
                    wifi.requestWifiService(param1 == ImsEventDef.IMS_WIFI_ON);
                    out.writeInt(1);
                } else {
                    out.writeInt(0);
                }
                return true;
            }
        }

        return false;
    }

    private void handleSystemCallForRadio(int method, Parcel in, Parcel out) {
        if (method == SystemConstants.SET_TRAFFIC_PRIORITY) {
            int priorityType = in.readInt();
            int slotId = in.readInt();
            mDefaultSystemCall.setTrafficPriority(priorityType, slotId);
            out.writeInt(1);
        }
    }

    private void handleSystemCallForCrossSimRedialing(int method, Parcel in, Parcel out) {
        if (method == SystemConstants.IS_CROSS_SIM_REDIALING_AVAILABLE) {
            int slotId = in.readInt();
            out.writeInt(mDefaultSystemCall.isCrossSimRedialingAvailable(slotId) ? 1 : 0);
        }
    }

    @VisibleForTesting
    protected void sendSystemEvent(Parcel parcel) {
        byte[] data = parcel.marshall();

        try {
            // The result will be ignored at this moment.
            JniImsProxy.sendDataForSystem(mNativeObject, data);
        } catch (RuntimeException e) {
            ImsLog.e("ImsSystem#sendSystemEvent: " + e);
        }
    }

    private class ImsSystem implements ISystem, MmTelFeatureRegistry.Listener {
        private final MessageExecutor mExecutor;
        private final ArraySet<Integer> mRegisteredEvents = new ArraySet<>();
        private final int mSlotId;
        private final MmTelFeatureRegistry mMmTelFeatureRegistry;
        private SystemCallInterface mSystemCall;

        ImsSystem(int slotId, MessageExecutor executor, MmTelFeatureRegistry mmTelFeatureRegistry) {
            mSlotId = slotId;
            mExecutor = (executor != null) ? executor : new MessageExecutor("ImsSystem" + mSlotId);
            mMmTelFeatureRegistry = mmTelFeatureRegistry;
            mMmTelFeatureRegistry.addListener(this);
        }

        public void dispose() {
            mMmTelFeatureRegistry.removeListener(this);
            mExecutor.removeCallbacksAndMessages(null);
        }

        @Override
        public void setSystemCallInterface(SystemCallInterface systemCall) {
            mSystemCall = systemCall;
        }

        /**
         * Notifies the changes of airplane mode in the phone settings.
         *
         * @param airplaneMode the current airplane mode status
         *          0: Airplane mode OFF, 1: Airplane mode ON
         */
        @Override
        public void notifyAirplaneModeChanged(int airplaneMode) {
            mExecutor.execute(() -> {
                Parcel parcel = Parcel.obtain();
                try {
                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_AIRPLANE_MODE_CHANGED);
                    parcel.writeInt(airplaneMode);
                    sendSystemEvent(parcel);
                } finally {
                    parcel.recycle();
                }
            });
        }

        /**
         * Notifies the failure result to connect a data connection of the specified APN type.
         *
         * @param apnType the APN type (1: ims, 2: internet, 3: xcap, 9: emergency, 21: wifi)
         */
        @Override
        public void notifyDataConnectionFailed(int apnType) {
            mExecutor.execute(() -> {
                Parcel parcel = Parcel.obtain();
                try {
                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_DATA_CONNECTION_FAILED);
                    parcel.writeInt(apnType);
                    sendSystemEvent(parcel);
                } finally {
                    parcel.recycle();
                }
            });
        }

        /**
         * Notifies the IPCAN category of the attached data connection.
         *
         * @param apnType the APN type (1: ims, 2: internet, 3: xcap, 9: emergency,
         *            21: wifi)
         * @param ipcanCategory the IPCAN category (0: MOBILE, 1: WLAN); Refer to IIPCAN.h
         */
        @Override
        public void notifyDataConnectionIpcanChanged(int apnType, int ipcanCategory) {
            mExecutor.execute(() -> {
                Parcel parcel = Parcel.obtain();
                try {
                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_DATA_CONNECTION_IPCAN_CHANGED);
                    parcel.writeInt(apnType);
                    parcel.writeInt(ipcanCategory);
                    sendSystemEvent(parcel);
                } finally {
                    parcel.recycle();
                }
            });
        }

        /**
         * Notifies the state of the specified data connection.
         *
         * @param apnType the APN type (1: ims, 2: internet, 3: xcap, 9: emergency, 21: wifi)
         * @param state the data state (TelephonyManager.DATA_*)
         *          {@link TelephonyManager#DATA_UNKNOWN} (-1)
         *          {@link TelephonyManager#DATA_DISCONNECTED} (0)
         *          {@link TelephonyManager#DATA_CONNECTING} (1)
         *          {@link TelephonyManager#DATA_CONNECTED} (2)
         *          {@link TelephonyManager#DATA_SUSPENDED} (3)
         */
        @Override
        public void notifyDataConnectionStateChanged(int apnType, int state) {
            mExecutor.execute(() -> {
                Parcel parcel = Parcel.obtain();
                try {
                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_DATA_CONNECTION_STATE_CHANGED);
                    parcel.writeInt(apnType);
                    parcel.writeInt(state);
                    sendSystemEvent(parcel);
                } finally {
                    parcel.recycle();
                }
            });
        }

        /**
         * Notifies the network type which the device is attached.
         *
         * @param networkType The network type defined in DcNetWatcher
         *          {@link RAT_NONET} (0)
         *          {@link RAT_EHRPD} (1)
         *          {@link RAT_4G} (2)
         *          {@link RAT_3G} (3)
         *          {@link RAT_1XRTT} (4)
         *          {@link RAT_2G} (5)
         */
        @Override
        public void notifyNetworkTypeChanged(int networkType) {
            mExecutor.execute(() -> {
                Parcel parcel = Parcel.obtain();
                try {
                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_NETWORK_TYPE_CHANGED);
                    parcel.writeInt(networkType);
                    sendSystemEvent(parcel);
                } finally {
                    parcel.recycle();
                }
            });
        }

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
        @Override
        public void notifyVoiceNetworkTypeChanged(int networkType) {
            mExecutor.execute(() -> {
                Parcel parcel = Parcel.obtain();
                try {
                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_VOICE_NETWORK_TYPE_CHANGED);
                    parcel.writeInt(networkType);
                    sendSystemEvent(parcel);
                } finally {
                    parcel.recycle();
                }
            });
        }

        /**
         * Notifies the service state related to the attached network.
         *
         * @param serviceState the service state (ServiceState.STATE_*)
         *          {@link ServiceState#STATE_IN_SERVICE} (0)
         *          {@link ServiceState#STATE_OUT_OF_SERVICE} (1)
         *          {@link ServiceState#STATE_EMERGENCY_ONLY} (2)
         *          {@link ServiceState#STATE_POWER_OFF} (3)
         */
        @Override
        public void notifyServiceStateChanged(int serviceState) {
            mExecutor.execute(() -> {
                Parcel parcel = Parcel.obtain();
                try {
                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_SERVICE_STATE_CHANGED);
                    parcel.writeInt(serviceState);
                    sendSystemEvent(parcel);
                } finally {
                    parcel.recycle();
                }
            });
        }

        /**
         * Notifies the changes of the IMS configuration.
         *
         * @param configs the configuration items to be updated
         */
        @Override
        public void notifyConfigurationChanged(int configs) {
            mExecutor.execute(() -> {
                Parcel parcel = Parcel.obtain();
                try {
                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_CONFIGURATION_CHANGED);
                    parcel.writeInt(configs);
                    sendSystemEvent(parcel);
                } finally {
                    parcel.recycle();
                }
            });
        }

        /**
         * Notifies the events which are registered by the native modules.
         *
         * @param event the current event
         * @param param1 the parameter related to the current event
         * @param param2 the additional parameter related to the current event
         */
        @Override
        public void notifyEvent(int event, int param1, int param2) {
            mExecutor.execute(() -> {
                if (!isEventRegistered(event)) {
                    ImsLog.w("Event(0x" + Integer.toHexString(event) + ") is not registered");
                    return;
                }

                Parcel parcel = Parcel.obtain();
                try {
                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_EVENT);
                    parcel.writeInt(event);
                    parcel.writeInt(param1);
                    parcel.writeInt(param2);
                    sendSystemEvent(parcel);
                } finally {
                    parcel.recycle();
                }
            });
        }

        @Override
         public void notifyIsimState(int event, String state) {
            mExecutor.execute(() -> {
                Parcel parcel = Parcel.obtain();
                try {
                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_ISIM_EVENT);
                    parcel.writeInt(event);
                    parcel.writeString(state);
                    sendSystemEvent(parcel);
                } finally {
                    parcel.recycle();
                }
            });
        }

        @Override
        public void notifyIsimAuthenticationResponse(int event, String response, long owner) {
            mExecutor.execute(() -> {
                Parcel parcel = Parcel.obtain();
                try {
                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_ISIM_EVENT);
                    parcel.writeInt(event);
                    parcel.writeString(response);
                    parcel.writeLong(owner);
                    sendSystemEvent(parcel);
                } finally {
                    parcel.recycle();
                }
            });
        }

        @Override
        public void notifyUsimAuthenticationResponse(int event, String response, long owner) {
            mExecutor.execute(() -> {
                Parcel parcel = Parcel.obtain();
                try {
                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_USIM_EVENT);
                    parcel.writeInt(event);
                    parcel.writeString(response);
                    parcel.writeLong(owner);
                    sendSystemEvent(parcel);
                } finally {
                    parcel.recycle();
                }
            });
        }

        @Override
        public void notifyRadioConnectionFailed(int event, int id, int failureReason, int causeCode,
                int waitTimeMillis) {
            mExecutor.execute(() -> {
                Parcel parcel = Parcel.obtain();
                try {
                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_RADIO_EVENT);
                    parcel.writeInt(event);
                    parcel.writeInt(id);
                    parcel.writeInt(failureReason);
                    parcel.writeInt(causeCode);
                    parcel.writeInt(waitTimeMillis);
                    sendSystemEvent(parcel);
                } finally {
                    parcel.recycle();
                }
            });
        }

        @Override
        public void notifyRadioConnectionSetupPrepared(int event, int id) {
            mExecutor.execute(() -> {
                Parcel parcel = Parcel.obtain();
                try {
                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_RADIO_EVENT);
                    parcel.writeInt(event);
                    parcel.writeInt(id);
                    sendSystemEvent(parcel);
                } finally {
                    parcel.recycle();
                }
            });
        }

        @Override
        public void notifySsacInfo(int event, int voiceFactor, int voiceTimeSec, int videoFactor,
                int videoTimeSec) {
            mExecutor.execute(() -> {
                Parcel parcel = Parcel.obtain();
                try {
                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_RADIO_EVENT);
                    parcel.writeInt(event);
                    parcel.writeInt(voiceFactor);
                    parcel.writeInt(voiceTimeSec);
                    parcel.writeInt(videoFactor);
                    parcel.writeInt(videoTimeSec);
                    sendSystemEvent(parcel);
                } finally {
                    parcel.recycle();
                }
            });
        }

        @Override
        public void notifySimultaneousCallingSupportChanged(int event, boolean supported) {
            mExecutor.execute(() -> {
                Parcel parcel = Parcel.obtain();
                try {
                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_RADIO_EVENT);
                    parcel.writeInt(event);
                    parcel.writeInt(supported ? 1 : 0);
                    sendSystemEvent(parcel);
                } finally {
                    parcel.recycle();
                }
            });
        }

        @Override
        public void notifyLocationUpdateCompleted(int requestId) {
            mExecutor.execute(() -> {
                Parcel parcel = Parcel.obtain();
                try {
                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_LOCATION_EVENT);
                    parcel.writeInt(LocationInterface.EVENT_LOCATION_UPDATE_COMPLETED);
                    parcel.writeInt(requestId);
                    sendSystemEvent(parcel);
                } finally {
                    parcel.recycle();
                }
            });
        }

        @Override
        public void onAdvancedCallingSettingChanged() {
            notifyEvent(ImsEventDef.IMS_EVENT_VOLTE_SETTING,
                    mMmTelFeatureRegistry.isAdvancedCallingSettingEnabled()
                    ? ImsEventDef.IMS_VOLTE_SETTING_ON
                    : ImsEventDef.IMS_VOLTE_SETTING_OFF, 0);
        }

        @Override
        public void onVoWiFiSettingChanged() {
            notifyEvent(ImsEventDef.IMS_EVENT_WFC_SETTING_CHANGED,
                    mMmTelFeatureRegistry.isVoWiFiSettingEnabled()
                    ? ImsEventDef.IMS_WFC_ON
                    : ImsEventDef.IMS_WFC_OFF, mMmTelFeatureRegistry.getVoWiFiModeSetting());
        }

        @Override
        public void onRttModeChanged() {
            notifyEvent(ImsEventDef.IMS_EVENT_RTT_SETTING, mMmTelFeatureRegistry.getRttMode(), 0);
        }

        public boolean isEventRegistered(int event) {
            synchronized (mRegisteredEvents) {
                return mRegisteredEvents.contains(event);
            }
        }

        public void registerEvent(int event) {
            ImsLog.d(mSlotId, "registerEvent: 0x" + Integer.toHexString(event));
            synchronized (mRegisteredEvents) {
                mRegisteredEvents.add(event);
            }
        }

        public void unregisterEvent(int event) {
            ImsLog.d(mSlotId, "unregisterEvent: 0x" + Integer.toHexString(event));
            synchronized (mRegisteredEvents) {
                mRegisteredEvents.remove(event);
            }
        }

        public void handleSystemCall(int method, Parcel in, FileDescriptor fd, Parcel out) {
            if (mSystemCall == null) {
                return;
            }

            switch (method) {
                case SystemConstants.SEND_EVENT: {
                    int event = in.readInt();
                    if (event == ImsEventDef.IMS_EVENT_NATIVE_BOOT_COMPLETED) {
                        out.writeInt(mSystemCall.updateNativeServiceReady(true));
                    }
                    break;
                }
                case SystemConstants.SET_EVENT: {
                    int event = in.readInt();
                    registerEvent(event);
                    out.writeInt(1);
                    break;
                }
                case SystemConstants.RESET_EVENT: {
                    int event = in.readInt();
                    unregisterEvent(event);
                    out.writeInt(1);
                    break;
                }
                case SystemConstants.GET_TTY_MODE: // fall through
                case SystemConstants.GET_RTT_MODE:
                    handleSystemCallForCallSettings(method, out);
                    break;
                case SystemConstants.IS_WFC_ENABLED: // fall through
                case SystemConstants.GET_WFC_PREFERENCES: // fall through
                case SystemConstants.IS_WFC_PROVISIONED: // fall through
                case SystemConstants.GET_WFC_ADDRESS_ID:
                    handleSystemCallForWifiCalling(method, out);
                    break;
                case SystemConstants.REQUEST_NETWORK: // fall through
                case SystemConstants.RELEASE_NETWORK: // fall through
                case SystemConstants.GET_ACCESS_NETWORK_INFO: // fall through
                case SystemConstants.GET_APN_NAME: // fall through
                case SystemConstants.GET_DATA_CONNECTION_STATE: // fall through
                case SystemConstants.GET_HOST_BY_NAME: // fall through
                case SystemConstants.GET_IFACE_ID: // fall through
                case SystemConstants.GET_IFACE_NAME: // fall through
                case SystemConstants.GET_LAST_ACCESS_NETWORK_INFO: // fall through
                case SystemConstants.GET_LOCAL_ADDRESS: // fall through
                case SystemConstants.GET_IPCAN_CATEGORY: // fall through
                case SystemConstants.GET_PCSCF_ADDRESSES: // fall through
                case SystemConstants.GET_ROAMING_STATE: // fall through
                case SystemConstants.GET_SERVICE_STATE: // fall through
                case SystemConstants.IS_EMERGENCY_ONLY: // fall through
                case SystemConstants.IS_MOBILE_DATA_ENABLED:// fall through
                case SystemConstants.GET_VOICE_SERVICE_STATE: // fall through
                case SystemConstants.GET_VOICE_ROAMING_TYPE: // fall through
                case SystemConstants.GET_DATA_ROAMING_TYPE: // fall through
                case SystemConstants.GET_MTU: // fall through
                case SystemConstants.IS_EMERGENCY_ATTACH_SUPPORTED: // fall through
                case SystemConstants.BIND_SOCKET: // fall through
                case SystemConstants.IS_IPV6_PREFERRED: // fall through
                case SystemConstants.GET_NETWORK_REGISTRATION_REJECT_CAUSE: // fall through
                case SystemConstants.GET_CELLULAR_SERVICE_STATE:  // fall through
                case SystemConstants.GET_ACCESS_NETWORK_PLMN:
                    handleSystemCallForNetwork(method, in, fd, out);
                    break;
                case SystemConstants.GET_ISIM_STATE: // fall through
                case SystemConstants.GET_ISIM_RECORD: // fall through
                case SystemConstants.REQUEST_ISIM_AUTH: // fall through
                case SystemConstants.REQUEST_USIM_AUTH:
                    handleSystemCallForSim(method, in, out);
                    break;
                case SystemConstants.GET_DEVICE_ID: // fall through
                case SystemConstants.GET_DEVICE_SOFTWARE_VERSION: // fall through
                case SystemConstants.GET_PHONE_NUMBER: // fall through
                case SystemConstants.GET_SUBSCRIBER_ID: // fall through
                case SystemConstants.GET_SIM_MCC: // fall through
                case SystemConstants.GET_SIM_MNC: // fall through
                case SystemConstants.GET_SIM_COUNTRY_ISO: // fall through
                case SystemConstants.GET_NETWORK_COUNTRY_ISO: // fall through
                case SystemConstants.GET_NETWORK_OPERATOR: // fall through
                case SystemConstants.GET_NETWORK_TYPE: // fall through
                case SystemConstants.GET_VOICE_NETWORK_TYPE: // fall through
                case SystemConstants.GET_CS_CALL_STATE: // fall through
                case SystemConstants.GET_CS_CALL_STATE_IN_OTHER_SLOT: // fall through
                case SystemConstants.IS_EMERGENCY_NUMBER:
                    handleSystemCallForTelephony(method, in, out);
                    break;
                case SystemConstants.START_LISTENING_FOR_LOCATION: // fall through
                case SystemConstants.STOP_LISTENING_FOR_LOCATION: // fall through
                case SystemConstants.GET_LAST_KNOWN_LOCATION: // fall through
                case SystemConstants.REQUEST_LOCATION_UPDATE: // fall through
                case SystemConstants.CANCEL_LOCATION_UPDATE:
                    handleSystemCallForLocation(method, in, out);
                    break;
                case SystemConstants.START_IMS_TRAFFIC: // fall through
                case SystemConstants.STOP_IMS_TRAFFIC: // fall through
                case SystemConstants.TRIGGER_EPS_FALLBACK:
                    handleSystemCallForRadio(method, in, out);
                    break;
                case SystemConstants.ADD_IPSEC_SA_PARAMETER: // fall through
                case SystemConstants.REMOVE_IPSEC_SA_PARAMETER: // fall through
                case SystemConstants.APPLY_IPSEC_SA: // fall through
                case SystemConstants.REMOVE_IPSEC_SA:
                    handleSystemCallForIpSec(method, in, fd, out);
                    break;
                case SystemConstants.LOG_SIP_MESSAGE: {
                    int direction = in.readInt();
                    byte[] bytes = in.createByteArray();
                    if (bytes != null) {
                        String sipMessage = new String(bytes, StandardCharsets.UTF_8);
                        mSystemCall.logSipMessage(sipMessage, mSlotId, direction);
                    }
                    out.writeInt(1);
                    break;
                }
                default:
                    handleSystemCallForOthers(method, out);
                    break;
            }
        }

        private void handleSystemCallForCallSettings(int method, Parcel out) {
            if (method == SystemConstants.GET_TTY_MODE) {
                out.writeInt(mMmTelFeatureRegistry.getTtyMode());
            } else if (method == SystemConstants.GET_RTT_MODE) {
                out.writeInt(mMmTelFeatureRegistry.getRttMode());
            }
        }

        private void handleSystemCallForNetwork(int method, Parcel in, FileDescriptor fd,
                Parcel out) {
            switch (method) {
                case SystemConstants.REQUEST_NETWORK: {
                    int apnType = in.readInt();
                    out.writeInt(mSystemCall.requestNetwork(apnType) ? 1 : 0);
                    break;
                }
                case SystemConstants.RELEASE_NETWORK: {
                    int apnType = in.readInt();
                    out.writeInt(mSystemCall.releaseNetwork(apnType) ? 1 : 0);
                    break;
                }
                case SystemConstants.GET_ACCESS_NETWORK_INFO: {
                    int defaultNetworkType = in.readInt();
                    IDcUtils.AccessNetworkInfo ani =
                            mSystemCall.getAccessNetworkInfo(defaultNetworkType);
                    out.writeInt(ani.mNetworkType);
                    int length = (ani.mAni != null) ? ani.mAni.length : 0;
                    out.writeInt(length);
                    for (int i = 0; i < length; ++i) {
                        out.writeString(ani.mAni[i]);
                    }
                    break;
                }
                case SystemConstants.GET_APN_NAME: {
                    int apnType = in.readInt();
                    out.writeString(mSystemCall.getApnName(apnType));
                    break;
                }
                case SystemConstants.GET_DATA_CONNECTION_STATE: {
                    int apnType = in.readInt();
                    out.writeInt(mSystemCall.getDataConnectionState(apnType));
                    break;
                }
                case SystemConstants.GET_HOST_BY_NAME: {
                    int apnType = in.readInt();
                    int ipVersion = in.readInt();
                    String host = in.readString();
                    String[] ipAddrs = null;

                    if (apnType == EApnType.WIFI.getType()) {
                        WifiInterface wifi = (mDefaultSystemCall != null)
                                ? mDefaultSystemCall.getWifiInterface() : null;
                        if (wifi != null) {
                            ipAddrs = wifi.getHostByName(ipVersion, host);
                        }
                    } else {
                        ipAddrs = mSystemCall.getHostByName(apnType, ipVersion, host);
                    }

                    int length = (ipAddrs != null) ? ipAddrs.length : 0;
                    out.writeInt(length);
                    for (int i = 0; i < length; ++i) {
                        out.writeString(ipAddrs[i]);
                    }
                    break;
                }
                case SystemConstants.GET_IFACE_ID: {
                    int apnType = in.readInt();
                    int ifaceId = SystemCallInterface.RESULT_ERROR;

                    if (apnType == EApnType.WIFI.getType()) {
                        WifiInterface wifi = (mDefaultSystemCall != null)
                                ? mDefaultSystemCall.getWifiInterface() : null;
                        if (wifi != null) {
                            ifaceId = wifi.getIfaceId();
                        }
                    } else {
                        ifaceId = mSystemCall.getIfaceId(apnType);
                    }
                    out.writeInt(ifaceId);
                    break;
                }
                case SystemConstants.GET_IFACE_NAME: {
                    int apnType = in.readInt();
                    String ifaceName = null;

                    if (apnType == EApnType.WIFI.getType()) {
                        WifiInterface wifi = (mDefaultSystemCall != null)
                                ? mDefaultSystemCall.getWifiInterface() : null;
                        if (wifi != null) {
                            ifaceName = wifi.getIfaceName();
                        }
                    } else {
                        ifaceName = mSystemCall.getIfaceName(apnType);
                    }
                    out.writeString(ifaceName);
                    break;
                }
                case SystemConstants.GET_LAST_ACCESS_NETWORK_INFO: {
                    int networkType = in.readInt();
                    String[] lastAni = mSystemCall.getLastAccessNetworkInfo(networkType);

                    out.writeInt(lastAni.length);
                    for (int i = 0; i < lastAni.length; ++i) {
                        out.writeString(lastAni[i]);
                    }
                    break;
                }
                case SystemConstants.GET_LOCAL_ADDRESS: {
                    int apnType = in.readInt();
                    int ipVersion = in.readInt();
                    String localAddress = null;

                    if (apnType == EApnType.WIFI.getType()) {
                        WifiInterface wifi = (mDefaultSystemCall != null)
                                ? mDefaultSystemCall.getWifiInterface() : null;
                        if (wifi != null) {
                            localAddress = wifi.getLocalAddress(ipVersion);
                        }
                    } else {
                        localAddress = mSystemCall.getLocalAddress(apnType, ipVersion);
                    }
                    out.writeString(localAddress);
                    break;
                }
                case SystemConstants.GET_IPCAN_CATEGORY: {
                    int apnType = in.readInt();

                    if (apnType == EApnType.WIFI.getType()) {
                        out.writeInt(IApn.IPCAN_CATEGORY_WLAN);
                    } else {
                        out.writeInt(mSystemCall.getIpcanCategory(apnType));
                    }
                    break;
                }
                case SystemConstants.GET_PCSCF_ADDRESSES: {
                    int apnType = in.readInt();
                    int ipVersion = in.readInt();
                    String[] pcscfs = mSystemCall.getPcscfAddresses(apnType, ipVersion);

                    out.writeInt(pcscfs.length);
                    for (int i = 0; i < pcscfs.length; ++i) {
                        out.writeString(pcscfs[i]);
                    }
                    break;
                }
                case SystemConstants.GET_ROAMING_STATE:
                    out.writeInt(mSystemCall.isNetworkRoaming() ? 1 : 0);
                    break;
                case SystemConstants.GET_SERVICE_STATE:
                    out.writeInt(mSystemCall.getDataServiceState());
                    break;
                case SystemConstants.IS_EMERGENCY_ONLY:
                    out.writeInt(mSystemCall.isEmergencyOnly() ? 1 : 0);
                    break;
                case SystemConstants.IS_MOBILE_DATA_ENABLED:
                    out.writeInt(mSystemCall.isMobileDataEnabled() ? 1 : 0);
                    break;
                case SystemConstants.GET_VOICE_SERVICE_STATE:
                    out.writeInt(mSystemCall.getVoiceServiceState());
                    break;
                case SystemConstants.GET_VOICE_ROAMING_TYPE:
                    out.writeInt(mSystemCall.getVoiceRoamingType());
                    break;
                case SystemConstants.GET_DATA_ROAMING_TYPE:
                    out.writeInt(mSystemCall.getDataRoamingType());
                    break;
                case SystemConstants.GET_MTU: {
                    int apnType = in.readInt();
                    int mtu = 0; // Use a default MTU size (1500)

                    if (apnType == EApnType.WIFI.getType()) {
                        WifiInterface wifi = (mDefaultSystemCall != null)
                                ? mDefaultSystemCall.getWifiInterface() : null;
                        if (wifi != null) {
                            mtu = wifi.getMtu();
                        }
                    } else {
                        mtu = mSystemCall.getMtu(apnType);
                    }
                    out.writeInt(mtu);
                    break;
                }
                case SystemConstants.IS_EMERGENCY_ATTACH_SUPPORTED:
                    out.writeInt(mSystemCall.isEmergencyAttachSupported() ? 1 : 0);
                    break;
                case SystemConstants.BIND_SOCKET: {
                    int apnType = in.readInt();
                    boolean bindResult = false;

                    if (apnType == EApnType.WIFI.getType()) {
                        WifiInterface wifi = (mDefaultSystemCall != null)
                                ? mDefaultSystemCall.getWifiInterface() : null;
                        if (wifi != null) {
                            bindResult = wifi.bindSocket(fd);
                        }
                    } else {
                        bindResult = mSystemCall.bindSocket(apnType, fd);
                    }
                    out.writeInt(bindResult ? 1 : 0);
                    break;
                }
                case SystemConstants.IS_IPV6_PREFERRED: {
                    int apnType = in.readInt();
                    out.writeInt(mSystemCall.isIpv6Preferred(apnType) ? 1 : 0);
                    break;
                }
                case SystemConstants.GET_NETWORK_REGISTRATION_REJECT_CAUSE: {
                    out.writeInt(mSystemCall.getNetworkRegistrationRejectCause());
                    break;
                }
                case SystemConstants.GET_CELLULAR_SERVICE_STATE: {
                    out.writeInt(mSystemCall.getCellularDataServiceState());
                    break;
                }
                case SystemConstants.GET_ACCESS_NETWORK_PLMN: {
                    out.writeString(mSystemCall.getAccessNetworkPlmn());
                    break;
                }
            }
        }

        private void handleSystemCallForSim(int method, Parcel in, Parcel out) {
            switch (method) {
                case SystemConstants.GET_ISIM_STATE:
                    out.writeString(mSystemCall.getIsimState());
                    break;
                case SystemConstants.GET_ISIM_RECORD: {
                    int fileId = in.readInt();
                    List<String> record = mSystemCall.getIsimRecord(fileId);
                    out.writeInt(record.size());

                    for (String s : record) {
                        out.writeString(s);
                    }
                    break;
                }
                case SystemConstants.REQUEST_ISIM_AUTH: {
                    String nonce = in.readString();
                    long owner = in.readLong();
                    out.writeInt(mSystemCall.requestIsimAuthentication(nonce, owner));
                    break;
                }
                case SystemConstants.REQUEST_USIM_AUTH: {
                    String nonce = in.readString();
                    long owner = in.readLong();
                    out.writeInt(mSystemCall.requestUsimAuthentication(nonce, owner));
                    break;
                }
            }
        }

        private void handleSystemCallForTelephony(int method, Parcel in, Parcel out) {
            switch (method) {
                case SystemConstants.GET_DEVICE_ID:
                    out.writeString(mSystemCall.getImei());
                    break;
                case SystemConstants.GET_DEVICE_SOFTWARE_VERSION:
                    out.writeString(mSystemCall.getDeviceSoftwareVersion());
                    break;
                case SystemConstants.GET_PHONE_NUMBER:
                    out.writeString(mSystemCall.getPhoneNumber());
                    break;
                case SystemConstants.GET_SUBSCRIBER_ID:
                    out.writeString(mSystemCall.getSubscriberId());
                    break;
                case SystemConstants.GET_SIM_MCC:
                    out.writeString(mSystemCall.getSimMcc());
                    break;
                case SystemConstants.GET_SIM_MNC:
                    out.writeString(mSystemCall.getSimMnc());
                    break;
                case SystemConstants.GET_SIM_COUNTRY_ISO:
                    out.writeString(mSystemCall.getSimCountryIso());
                    break;
                case SystemConstants.GET_NETWORK_COUNTRY_ISO:
                    out.writeString(mSystemCall.getNetworkCountryIso());
                    break;
                case SystemConstants.GET_NETWORK_OPERATOR:
                    out.writeString(mSystemCall.getNetworkOperator());
                    break;
                case SystemConstants.GET_NETWORK_TYPE:
                    out.writeInt(mSystemCall.getNetworkType());
                    break;
                case SystemConstants.GET_VOICE_NETWORK_TYPE:
                    out.writeInt(mSystemCall.getVoiceNetworkType());
                    break;
                case SystemConstants.GET_CS_CALL_STATE:
                    out.writeInt(mSystemCall.getCsCallState());
                    break;
                case SystemConstants.GET_CS_CALL_STATE_IN_OTHER_SLOT:
                    out.writeInt(mSystemCall.getCsCallStateInOtherSlot());
                    break;
                case SystemConstants.IS_EMERGENCY_NUMBER: {
                    boolean isEmergencyNumber = mSystemCall.isEmergencyNumber(in.readString());
                    out.writeInt(isEmergencyNumber ? 1 : 0);
                    break;
                }
            }
        }

        private void handleSystemCallForWifiCalling(int method, Parcel out) {
            if (method == SystemConstants.IS_WFC_ENABLED) {
                out.writeInt(mMmTelFeatureRegistry.isVoWiFiSettingEnabled() ? 1 : 0);
            } else if (method == SystemConstants.GET_WFC_PREFERENCES) {
                out.writeInt(mMmTelFeatureRegistry.getVoWiFiModeSetting());
            } else if (method == SystemConstants.IS_WFC_PROVISIONED) {
                // Need to be checked whether this is necessary or not.
                out.writeInt(1);
            } else if (method == SystemConstants.GET_WFC_ADDRESS_ID) {
                ImsConfigImplBase config = ImsServiceRegistry.getInstance(mSlotId).getImsConfig();
                out.writeString(config != null ? config.getConfigString(
                        ProvisioningManager.KEY_VOICE_OVER_WIFI_ENTITLEMENT_ID) : "");
            }
        }

        private void handleSystemCallForLocation(int method, Parcel in, Parcel out) {
            if (method == SystemConstants.START_LISTENING_FOR_LOCATION) {
                mSystemCall.startListeningForLocation(in.readInt());
                out.writeInt(1);
            } else if (method == SystemConstants.STOP_LISTENING_FOR_LOCATION) {
                mSystemCall.stopListeningForLocation();
                out.writeInt(1);
            } else if (method == SystemConstants.GET_LAST_KNOWN_LOCATION) {
                String[] locationParam = mSystemCall.getLastKnownLocation(in.readInt());
                out.writeInt(locationParam.length);

                for (int i = 0; i < locationParam.length; ++i) {
                    out.writeString(locationParam[i]);
                }
            } else if (method == SystemConstants.REQUEST_LOCATION_UPDATE) {
                out.writeInt(mSystemCall.requestLocationUpdate(in.readInt()));
            } else if (method == SystemConstants.CANCEL_LOCATION_UPDATE) {
                mSystemCall.cancelLocationUpdate(in.readInt());
                out.writeInt(1);
            }
        }

        private void handleSystemCallForRadio(int method, Parcel in, Parcel out) {
            if (method == SystemConstants.START_IMS_TRAFFIC) {
                int id = in.readInt();
                int trafficType = in.readInt();
                int accessNetworkType = in.readInt();
                int direction = in.readInt();
                mSystemCall.startImsTraffic(id, trafficType, accessNetworkType, direction);
                out.writeInt(1);
            } else if (method == SystemConstants.STOP_IMS_TRAFFIC) {
                int id = in.readInt();
                mSystemCall.stopImsTraffic(id);
                out.writeInt(1);
            } else if (method == SystemConstants.TRIGGER_EPS_FALLBACK) {
                int reason = in.readInt();
                out.writeInt(mSystemCall.triggerEpsFallback(reason) ? 1 : 0);
            }
        }

        private void handleSystemCallForIpSec(int method, Parcel in, FileDescriptor fd,
                Parcel out) {
            if (method == SystemConstants.ADD_IPSEC_SA_PARAMETER) {
                IpSecSaParameter param = IpSecSaParameter.CREATOR.createFromParcel(in);
                out.writeInt(mSystemCall.addIpSecSaParameter(param));
            } else if (method == SystemConstants.REMOVE_IPSEC_SA_PARAMETER) {
                mSystemCall.removeIpSecSaParameter(in.readInt());
                out.writeInt(1);
            } else if (method == SystemConstants.APPLY_IPSEC_SA) {
                int ipsecId = in.readInt();
                int spi = in.readInt();
                int intFd = in.readInt();
                out.writeInt(mSystemCall.applyIpSecSa(ipsecId, spi, intFd, fd));
            } else if (method == SystemConstants.REMOVE_IPSEC_SA) {
                int ipsecId = in.readInt();
                int spi = in.readInt();
                int intFd = in.readInt();
                mSystemCall.removeIpSecSa(ipsecId, spi, intFd, fd);
                out.writeInt(1);
            }
        }

        private void handleSystemCallForOthers(int method, Parcel out) {
            if (method == SystemConstants.GET_CARRIER_CONFIG) {
                CarrierConfig cc = mSystemCall.getCarrierConfig();

                if (cc != null) {
                    out.writeInt(1);
                    cc.writeToParcel(out);
                } else {
                    out.writeInt(0);
                }
            } else if (method == SystemConstants.IS_IMS_VOICE_CALL_SUPPORTED) {
                out.writeInt(mSystemCall.isImsVoiceCallSupported() ? 1 : 0);
            }
        }
    }
}
