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

import android.os.Parcel;
import android.util.ArraySet;
import android.util.SparseArray;

import com.android.imsstack.core.agents.WifiInterface;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDcUtils;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.internal.imsservice.MmTelFeatureRegistry;
import com.android.imsstack.jni.JniImsProxy;
import com.android.imsstack.jni.JniSystemListener;
import com.android.imsstack.system.SystemConstants;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsPrivateProperties;
import com.android.imsstack.util.MSimUtils;
import com.android.imsstack.util.ThreadMessageExecutor;
import com.android.internal.annotations.VisibleForTesting;

import java.io.FileDescriptor;

/**
 * This class provides the system interfaces to communicate with native and Java layer.
 */
public class SystemInterface implements JniSystemListener {
    private static SystemInterface sSystemInterface = null;
    private static final SparseArray<String> sMethodToString = new SparseArray<>();

    private long mNativeObject = 0;

    private final SparseArray<ISystem> mSystems =
            new SparseArray<>(MSimUtils.getSupportedSimCount());

    private ThreadMessageExecutor mDefaultExecutor =
            new ThreadMessageExecutor(SystemInterface.class.getSimpleName() + "_DEFAULT");

    private DefaultSystemCallInterface mDefaultSystemCall;
    private ISystemAPIBattery mISystemAPIBattery;

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

    public void init() {
        if (mNativeObject == 0) {

            mNativeObject = JniImsProxy.getInterface(
                    SystemConstants.SYSTEM_INTERFACE, MSimUtils.DEFAULT_SLOT_ID);

            JniImsProxy.setSystemListener(mNativeObject, this);
            mDefaultExecutor.start();

            if (ImsLog.DBG) {
                initForDebugLog();
            }
        }
        else {
            ImsLog.w("aready registered listener or parameter is null");
        }
    }

    public void cleanup() {
        if (mNativeObject != 0) {
            mDefaultExecutor = null;
            JniImsProxy.removeSystemListener(mNativeObject);
            JniImsProxy.releaseInterface(mNativeObject);
            mNativeObject = 0;
        }
    }

    public synchronized ISystem getSystem(int slotId) {
        return mSystems.get(slotId);
    }

    public synchronized void start(int slotId) {
        ImsLog.d(slotId, "");
        ImsSystem system = new ImsSystem(slotId);
        system.init();
        mSystems.put(slotId, system);
    }

    public synchronized void stop(int slotId) {
        ImsLog.d(slotId, "");
        ImsSystem system = (ImsSystem) mSystems.get(slotId);

        if (system == null) {
            return;
        }

        system.cleanup();
        mSystems.remove(slotId);
    }

    public void setSystemCallInterface(DefaultSystemCallInterface systemCall) {
        mDefaultSystemCall = systemCall;
    }

    public void setISystemAPIBattery(ISystemAPIBattery api) {
        mISystemAPIBattery = api;
    }

    /**
     * Notifies the expiration of the timer.
     *
     * @param tid The timer id that was specified to start a timer.
     */
    public void notifyTimerExpired(long tid) {
        mDefaultExecutor.execute(new Runnable() {
            @Override
            public void run() {
                Parcel parcel = Parcel.obtain();

                if (parcel == null) {
                    ImsLog.d("Parcel is null");
                    return;
                }

                parcel.writeInt(MSimUtils.DEFAULT_SLOT_ID);
                parcel.writeInt(SystemConstants.NOTIFY_TIMER_EXPIRED);
                parcel.writeLong(tid);

                sendData2Native(mNativeObject, parcel);
            }
        });
    }

    /**
     * Notifies the changes of the battery level.
     *
     * @param level the battery level (integer between 1 and 100)
     */
    public void notifyBatteryLevelChanged(final int level) {
        mDefaultExecutor.execute(new Runnable() {
            @Override
            public void run() {
                Parcel parcel = Parcel.obtain();

                if (parcel == null) {
                    ImsLog.d("Parcel is null");
                    return;
                }

                parcel.writeInt(MSimUtils.DEFAULT_SLOT_ID);
                parcel.writeInt(SystemConstants.NOTIFY_BATTERY_LEVEL_CHANGED);
                parcel.writeInt(level);

                sendData2Native(mNativeObject, parcel);
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
        mDefaultExecutor.execute(new Runnable() {
            @Override
            public void run() {
                Parcel parcel = Parcel.obtain();

                if (parcel == null) {
                    ImsLog.d("Parcel is null");
                    return;
                }

                parcel.writeInt(MSimUtils.DEFAULT_SLOT_ID);
                parcel.writeInt(SystemConstants.NOTIFY_WIFI_STATE_CHANGED);
                parcel.writeInt(state);

                sendData2Native(mNativeObject, parcel);
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
        mDefaultExecutor.execute(new Runnable() {
            @Override
            public void run() {
                Parcel parcel = Parcel.obtain();

                if (parcel == null) {
                    ImsLog.d("Parcel is null");
                    return;
                }

                parcel.writeInt(MSimUtils.DEFAULT_SLOT_ID);
                parcel.writeInt(SystemConstants.NOTIFY_WIFI_CONNECTION_STATE_CHANGED);
                parcel.writeInt(state);

                sendData2Native(mNativeObject, parcel);
            }
        });
    }

    private byte[] onDefaultMessage(int method, Parcel parcel) {
        Parcel result = null;

        switch (method) {
            case SystemConstants.SET_TIMER: //FALL-THROUGH
            case SystemConstants.KILL_TIMER:
                result = handleSystemCallForTimer(method, parcel);
                break;
            case SystemConstants.GET_PREFERENCE: //FALL-THROUGH
            case SystemConstants.SET_PREFERENCE:
                result = handleSystemCallForPreference(method, parcel);
                break;
            case SystemConstants.GET_WIFI_STATE: //FALL-THROUGH
            case SystemConstants.GET_WIFI_CONNECTION_STATE: //FALL-THROUGH
            case SystemConstants.GET_WIFI_BSS_ID: //FALL-THROUGH
            case SystemConstants.GET_WIFI_SSID:
                result = handleSystemCallForWifi(method, parcel);
                break;
            case SystemConstants.GET_BATTERY_LEVEL:
                result = handleSystemAPIBattery(method);
                break;
            case SystemConstants.GET_DEVICE_NAME: //FALL-THROUGH
            case SystemConstants.GET_EXTERNAL_STORAGE_PATH:
                result = handleSystemCallForDevice(method);
                break;
            case SystemConstants.GET_PRIVATE_PROPERTY: // FALL-THROUGH
            case SystemConstants.SET_PRIVATE_PROPERTY:
                result = handleSystemAPIConfiguration(method, parcel);
                break;
            default:
                break;
        }

        if (result == null) {
            if (ImsLog.DBG) {
                ImsLog.d("method = " + sMethodToString.get(method) + " is not handled");
            } else {
                ImsLog.i("method = " + method + " is not handled");
            }
            return new byte[] {(byte)0};
        }

        byte[] buffer = result.marshall();
        result.recycle();
        result = null;

        return buffer;
    }

    private byte[] onDefaultMessageForEvent(int event, int wParam, int lParam) {
        int returnValue = 1;

        switch (event) {
            case ImsEventDef.IMS_EVENT_WAKE_LOCK:
                if (mDefaultSystemCall != null) {
                    mDefaultSystemCall.acquireWakeLock(lParam);
                }
                break;

            case ImsEventDef.IMS_EVENT_WIFI_SERVICE: {
                WifiInterface wifi = (mDefaultSystemCall != null)
                        ? mDefaultSystemCall.getWifiInterface() : null;
                if (wifi != null) {
                    wifi.requestWifiService((wParam == ImsEventDef.IMS_WIFI_ON) ? true : false);
                }
                break;
            }
            default:
                returnValue = 0;
                break;
        }

        Parcel result = Parcel.obtain();
        result.writeInt(returnValue);

        byte[] buffer = result.marshall();
        result.recycle();
        result = null;

        return buffer;
    }

    @Override
    public byte[] onMessage(Parcel parcel, FileDescriptor fd) {
        try {
            return handleMessage(parcel, fd);
        } catch (Throwable t) {
            ImsLog.i("onMessage :: exception=" + t.toString());
            t.printStackTrace();
            return new byte[] {(byte)0};
        }
    }

    private byte[] handleMessage(Parcel parcel, FileDescriptor fd) {
        int slotId = parcel.readInt();
        int method = parcel.readInt();
        int event = 0;
        int wParam = 0;
        int lParam = 0;

        if (ImsLog.DBG) {
            ImsLog.d(slotId, "method = " + sMethodToString.get(method));
        }

        if (isSendEvent(method)) {
            event = parcel.readInt();
            wParam = parcel.readInt();
            lParam = parcel.readInt();
        }

        if (isDefaultRequired(method, event)) {
            if (isSendEvent(method)) {
                return onDefaultMessageForEvent(event, wParam, lParam);
            } else {
                return onDefaultMessage(method, parcel);
            }
        }

        ImsSystem system;
        if (isActiveSlotIdRequired(method)) {
            system = getActiveSystem();
        } else {
            system = (ImsSystem) getSystem(slotId);
        }

        if (system == null) {
            return new byte[] {(byte)0};
        }

        Parcel result = null;

        switch (method) {
            case SystemConstants.SEND_EVENT:
                result = system.handleSystemCallForEvent(event, wParam, lParam);
                break;
            case SystemConstants.SET_EVENT: {
                int setEvent = parcel.readInt();
                ImsLog.d(slotId, "set event = " + Integer.toString(setEvent, 16));
                system.registerEvent(setEvent);
                result = Parcel.obtain();
                result.writeInt(1);
                break;
            }
            case SystemConstants.RESET_EVENT: {
                int resetEvent = parcel.readInt();
                ImsLog.d(slotId, "reset event = " + Integer.toString(resetEvent, 16));
                system.unregisterEvent(resetEvent);
                result = Parcel.obtain();
                result.writeInt(1);
                break;
            }
            default:
                result = system.handleSystemAPI(method, parcel, fd);
                break;
        }

        if (result == null) {
            if (ImsLog.DBG) {
                ImsLog.d(slotId, "method = " + sMethodToString.get(method) + " is not handled");
            } else {
                ImsLog.i(slotId, "method = " + method + " is not handled");
            }
            return new byte[] {(byte)0};
        }

        byte[] buffer = result.marshall();
        result.recycle();
        result = null;

        return buffer;
    }


    private synchronized ImsSystem getActiveSystem() {
        int activeSimCount = MSimUtils.getActiveSimCount();

        for (int i = 0; i < activeSimCount; i++) {
            ImsSystem system = (ImsSystem) getSystem(i);
            if (system != null) {
                return system;
            }
        }

        return null;
    }

    private Parcel handleSystemCallForTimer(int method, Parcel parcel) {
        if (mDefaultSystemCall == null) {
            return null;
        }

        Parcel result = Parcel.obtain();

        switch (method) {
            case SystemConstants.SET_TIMER: {
                int duration = parcel.readInt();
                long tid = parcel.readLong();
                result.writeInt(mDefaultSystemCall.startTimer(tid, duration) ? 1 : 0);
                break;
            }
            case SystemConstants.KILL_TIMER: {
                long tid = parcel.readLong();
                mDefaultSystemCall.stopTimer(tid);
                result.writeInt(1);
                break;
            }
            default:
                result.recycle();
                return null;
        }

        return result;
    }

    private Parcel handleSystemAPIBattery(int method) {
        if (mISystemAPIBattery == null) {
            return null;
        }

        Parcel result = Parcel.obtain();

        switch (method) {
        case SystemConstants.GET_BATTERY_LEVEL:
            result.writeInt(mISystemAPIBattery.getBatteryLevel4Sys());
            break;
        default:
            result.recycle();
            return null;
        }

        return result;
    }

    private Parcel handleSystemCallForDevice(int method) {
        if (mDefaultSystemCall == null) {
            return null;
        }

        Parcel result = Parcel.obtain();

        switch (method) {
            case SystemConstants.GET_DEVICE_NAME:
                result.writeString(mDefaultSystemCall.getDeviceName());
                break;
            case SystemConstants.GET_EXTERNAL_STORAGE_PATH:
                result.writeString(mDefaultSystemCall.getExternalStoragePath());
                break;
            default:
                result.recycle();
                return null;
        }

        return result;
    }

    public Parcel handleSystemAPIConfiguration(int method, Parcel parcel) {
        Parcel result = Parcel.obtain();

        switch (method) {
        case SystemConstants.GET_PRIVATE_PROPERTY: {
            int persistent = parcel.readInt();
            String key = parcel.readString();
            int slotId = parcel.readInt();

            if (persistent == 1) {
                result.writeString(ImsPrivateProperties.Persistent.get(key, "", slotId));
            } else {
                result.writeString(ImsPrivateProperties.Ephemeral.get(key, "", slotId));
            }
            break;
        }
        case SystemConstants.SET_PRIVATE_PROPERTY: {
            int persistent = parcel.readInt();
            String key = parcel.readString();
            String value = parcel.readString();
            int slotId = parcel.readInt();

            if (persistent == 1) {
                ImsPrivateProperties.Persistent.set(key, value, slotId);
            } else {
                ImsPrivateProperties.Ephemeral.set(key, value, slotId);
            }
            result.writeInt(1);
            break;
        }
        default:
            result.recycle();
            return null;
        }

        return result;
    }

    private Parcel handleSystemCallForPreference(int method, Parcel parcel) {
        if (mDefaultSystemCall == null) {
            return null;
        }

        Parcel result = Parcel.obtain();
        switch (method) {
            case SystemConstants.GET_PREFERENCE: {
                String fileName = parcel.readString();
                String key = parcel.readString();
                int slotId = parcel.readInt();
                result.writeString(mDefaultSystemCall.getPreference(fileName, key, slotId));
                break;
            }
            case SystemConstants.SET_PREFERENCE: {
                String fileName = parcel.readString();
                String key = parcel.readString();
                String value = parcel.readString();
                int slotId = parcel.readInt();
                result.writeInt(
                        mDefaultSystemCall.setPreference(fileName, key, value, slotId) ? 1 : 0);
                break;
            }
            default:
                result.recycle();
                return null;
        }

        return result;
    }

    private Parcel handleSystemCallForWifi(int method, Parcel parcel) {
        WifiInterface wifi = (mDefaultSystemCall != null)
                ? mDefaultSystemCall.getWifiInterface() : null;
        if (wifi == null) {
            return null;
        }

        Parcel result = Parcel.obtain();

        switch (method) {
            case SystemConstants.GET_WIFI_STATE:
                result.writeInt(wifi.isWifiEnabled()
                        ? WifiInterface.STATE_ENABLED
                        : WifiInterface.STATE_DISABLED);
                break;
            case SystemConstants.GET_WIFI_CONNECTION_STATE:
                result.writeInt(wifi.isWifiConnected()
                        ? WifiInterface.CONNECTION_STATE_CONNECTED
                        : WifiInterface.CONNECTION_STATE_DISCONNECTED);
                break;
            case SystemConstants.GET_WIFI_BSS_ID:
                result.writeString(wifi.getBssId());
                break;
            case SystemConstants.GET_WIFI_SSID:
                result.writeString(wifi.getSsId());
                break;
            default:
                result.recycle();
                return null;
        }

        return result;
    }

    private boolean isActiveSlotIdRequired(int method) {
        switch (method) {
        case SystemConstants.GET_DIGEST_SHA1:
            return true;
        default:
            break;
        }

        return false;
    }

    private boolean isSendEvent(int method) {
        return (method == SystemConstants.SEND_EVENT);
    }

    private boolean isDefaultRequired(int method, int event) {
        if (method == SystemConstants.SEND_EVENT) {
            if (event == ImsEventDef.IMS_EVENT_WAKE_LOCK
                || event == ImsEventDef.IMS_EVENT_WIFI_SERVICE) {
                return true;
            } else {
                return false;
            }
        }

        switch (method) {
            case SystemConstants.SET_TIMER: //FALL-THROUGH
            case SystemConstants.KILL_TIMER: //FALL-THROUGH
            case SystemConstants.GET_PREFERENCE: //FALL-THROUGH
            case SystemConstants.SET_PREFERENCE: //FALL-THROUGH
            case SystemConstants.GET_WIFI_STATE: //FALL-THROUGH
            case SystemConstants.GET_WIFI_CONNECTION_STATE: //FALL-THROUGH
            case SystemConstants.GET_WIFI_BSS_ID: //FALL-THROUGH
            case SystemConstants.GET_WIFI_SSID: //FALL-THROUGH
            case SystemConstants.GET_BATTERY_LEVEL: //FALL-THROUGH
            case SystemConstants.GET_EXTERNAL_STORAGE_PATH: //FALL-THROUGH
            case SystemConstants.GET_DEVICE_NAME: //FALL-THROUGH
            case SystemConstants.GET_PRIVATE_PROPERTY: // FALL-THROUGH
            case SystemConstants.SET_PRIVATE_PROPERTY:
                return true;
            default:
                break;
        }

        return false;
    }

    private void initForDebugLog() {
        sMethodToString.put(SystemConstants.SET_TIMER,
                "SET_TIMER");
        sMethodToString.put(SystemConstants.KILL_TIMER,
                "KILL_TIMER");
        sMethodToString.put(SystemConstants.GET_BATTERY_LEVEL,
                "GET_BATTERY_LEVEL");
        sMethodToString.put(SystemConstants.IS_EMERGENCY_NUMBER,
                "IS_EMERGENCY_NUMBER");
        sMethodToString.put(SystemConstants.GET_TTY_MODE,
                "GET_TTY_MODE");
        sMethodToString.put(SystemConstants.IS_IMS_VOICE_CALL_SUPPORTED,
                "IS_IMS_VOICE_CALL_SUPPORTED");
        sMethodToString.put(SystemConstants.ACTIVATE_DATA_CONNECTION,
                "ACTIVATE_DATA_CONNECTION");
        sMethodToString.put(SystemConstants.DEACTIVATE_DATA_CONNECTION,
                "DEACTIVATE_DATA_CONNECTION");
        sMethodToString.put(SystemConstants.GET_ACCESS_NETWORK_INFO,
                "GET_ACCESS_NETWORK_INFO");
        sMethodToString.put(SystemConstants.GET_APN_NAME,
                "GET_APN_NAME");
        sMethodToString.put(SystemConstants.GET_DATA_CONNECTION_STATE,
                "GET_DATA_CONNECTION_STATE");
        sMethodToString.put(SystemConstants.GET_HOST_BY_NAME,
                "GET_HOST_BY_NAME");
        sMethodToString.put(SystemConstants.GET_IFACE_ID,
                "GET_IFACE_ID");
        sMethodToString.put(SystemConstants.GET_IFACE_NAME,
                "GET_IFACE_NAME");
        sMethodToString.put(SystemConstants.GET_IPCAN_CATEGORY,
                "GET_IPCAN_CATEGORY");
        sMethodToString.put(SystemConstants.GET_LAST_ACCESS_NETWORK_INFO,
                "GET_LAST_ACCESS_NETWORK_INFO");
        sMethodToString.put(SystemConstants.GET_LOCAL_ADDRESS,
                "GET_LOCAL_ADDRESS");
        sMethodToString.put(SystemConstants.GET_PCSCF_ADDRESSES,
                "GET_PCSCF_ADDRESSES");
        sMethodToString.put(SystemConstants.GET_ROAMING_STATE,
                "GET_ROAMING_STATE");
        sMethodToString.put(SystemConstants.GET_SERVICE_STATE,
                "GET_SERVICE_STATE");
        sMethodToString.put(SystemConstants.IS_LTE_EMERGENCY_ONLY,
                "IS_LTE_EMERGENCY_ONLY");
        sMethodToString.put(SystemConstants.IS_MOBILE_DATA_ENABLED,
                "IS_MOBILE_DATA_ENABLED");
        sMethodToString.put(SystemConstants.GET_MOCN_PLMN_INFO,
                "GET_MOCN_PLMN_INFO");
        sMethodToString.put(SystemConstants.GET_VOICE_SERVICE_STATE,
                "GET_VOICE_SERVICE_STATE");
        sMethodToString.put(SystemConstants.GET_MTU,
                "GET_MTU");
        sMethodToString.put(SystemConstants.IS_EMERGENCY_ATTACH_SUPPORTED,
                "IS_EMERGENCY_ATTACH_SUPPORTED");
        sMethodToString.put(SystemConstants.BIND_SOCKET,
                "BIND_SOCKET");
        sMethodToString.put(SystemConstants.GET_PREFERENCE,
                "GET_PREFERENCE");
        sMethodToString.put(SystemConstants.SET_PREFERENCE,
                "SET_PREFERENCE");
        sMethodToString.put(SystemConstants.GET_PRIVATE_PROPERTY,
                "GET_PRIVATE_PROPERTY");
        sMethodToString.put(SystemConstants.SET_PRIVATE_PROPERTY,
                "SET_PRIVATE_PROPERTY");
        sMethodToString.put(SystemConstants.GET_CARRIER_CONFIG,
                "GET_CARRIER_CONFIG");
        sMethodToString.put(SystemConstants.GET_DIGEST_SHA1,
                "GET_DIGEST_SHA1");
        sMethodToString.put(SystemConstants.SEND_EVENT,
                "SEND_EVENT");
        sMethodToString.put(SystemConstants.GET_ISIM_STATE,
                "GET_ISIM_STATE");
        sMethodToString.put(SystemConstants.READ_ISIM_FILE_ATTR,
                "READ_ISIM_FILE_ATTR");
        sMethodToString.put(SystemConstants.READ_ISIM_RECORD,
                "READ_ISIM_RECORD");
        sMethodToString.put(SystemConstants.REQUEST_ISIM_AUTH,
                "REQUEST_ISIM_AUTH");
        sMethodToString.put(SystemConstants.REQUEST_USIM_AUTH,
                "REQUEST_USIM_AUTH");
        sMethodToString.put(SystemConstants.GET_NETWORK_TYPE,
                "GET_NETWORK_TYPE");
        sMethodToString.put(SystemConstants.GET_VOICE_NETWORK_TYPE,
                "GET_VOICE_NETWORK_TYPE");
        sMethodToString.put(SystemConstants.GET_CS_CALL_STATE,
                "GET_CS_CALL_STATE");
        sMethodToString.put(SystemConstants.GET_DEVICE_ID,
                "GET_DEVICE_ID");
        sMethodToString.put(SystemConstants.GET_DEVICE_NAME,
                "GET_DEVICE_NAME");
        sMethodToString.put(SystemConstants.GET_DEVICE_SOFTWARE_VERSION,
                "GET_DEVICE_SOFTWARE_VERSION");
        sMethodToString.put(SystemConstants.GET_EXTERNAL_STORAGE_PATH,
                "GET_EXTERNAL_STORAGE_PATH");
        sMethodToString.put(SystemConstants.GET_PHONE_NUMBER,
                "GET_PHONE_NUMBER");
        sMethodToString.put(SystemConstants.GET_SUBSCRIBER_ID,
                "GET_SUBSCRIBER_ID");
        sMethodToString.put(SystemConstants.GET_SIM_MCC,
                "GET_SIM_MCC");
        sMethodToString.put(SystemConstants.GET_SIM_MNC,
                "GET_SIM_MNC");
        sMethodToString.put(SystemConstants.GET_SIM_COUNTRY_ISO,
                "GET_SIM_COUNTRY_ISO");
        sMethodToString.put(SystemConstants.GET_NETWORK_COUNTRY_ISO,
                "GET_NETWORK_COUNTRY_ISO");
        sMethodToString.put(SystemConstants.GET_WIFI_STATE,
                "GET_WIFI_STATE");
        sMethodToString.put(SystemConstants.GET_WIFI_CONNECTION_STATE,
                "GET_WIFI_CONNECTION_STATE");
        sMethodToString.put(SystemConstants.GET_WIFI_BSS_ID,
                "GET_WIFI_BSS_ID");
        sMethodToString.put(SystemConstants.GET_WIFI_SSID,
                "GET_WIFI_SSID");
        sMethodToString.put(SystemConstants.IS_WFC_ENABLED,
                "IS_WFC_ENABLED");
        sMethodToString.put(SystemConstants.GET_WFC_PREFERENCES,
                "GET_WFC_PREFERENCES");
        sMethodToString.put(SystemConstants.IS_WFC_PROVISIONED,
                "IS_WFC_PROVISIONED");
        sMethodToString.put(SystemConstants.GET_WFC_ADDRESS_ID,
                "GET_WFC_ADDRESS_ID");
        sMethodToString.put(SystemConstants.START_LISTENING_FOR_LOCATION,
                "START_LISTENING_FOR_LOCATION");
        sMethodToString.put(SystemConstants.STOP_LISTENING_FOR_LOCATION,
                "STOP_LISTENING_FOR_LOCATION");
        sMethodToString.put(SystemConstants.GET_LAST_KNOWN_LOCATION,
                "GET_LAST_KNOWN_LOCATION");
        sMethodToString.put(SystemConstants.START_INSTANT_LOCATION_UPDATE,
                "START_INSTANT_LOCATION_UPDATE");
        sMethodToString.put(SystemConstants.SET_EVENT,
                "SET_EVENT");
        sMethodToString.put(SystemConstants.RESET_EVENT,
                "RESET_EVENT");
        sMethodToString.put(SystemConstants.GET_CS_CALL_STATE_IN_OTHER_SLOT,
                "GET_CS_CALL_STATE_IN_OTHER_SLOT");
        sMethodToString.put(SystemConstants.ADD_IPSEC_SA_PARAMETER,
                "ADD_IPSEC_SA_PARAMETER");
        sMethodToString.put(SystemConstants.REMOVE_IPSEC_SA_PARAMETER,
                "REMOVE_IPSEC_SA_PARAMETER");
        sMethodToString.put(SystemConstants.APPLY_IPSEC_SA,
                "APPLY_IPSEC_SA");
        sMethodToString.put(SystemConstants.REMOVE_IPSEC_SA,
                "REMOVE_IPSEC_SA");
        sMethodToString.put(SystemConstants.START_IMS_TRAFFIC,
                "START_IMS_TRAFFIC");
        sMethodToString.put(SystemConstants.STOP_IMS_TRAFFIC,
                "STOP_IMS_TRAFFIC");
        sMethodToString.put(SystemConstants.TRIGGER_EPS_FALLBACK,
                "TRIGGER_EPS_FALLBACK");
    }

    private void sendData2Native(long cmd, Parcel parcel) {
        byte[] result;
        byte[] data = parcel.marshall();

        try {
            result = JniImsProxy.sendDataForSystem(mNativeObject, data);
        } catch (RuntimeException e) {
            ImsLog.e("sendDataForSystem: " + e.toString() + "; cmd=" + cmd);
            return;
        }

        Parcel parcelOut = Parcel.obtain();
        parcelOut.unmarshall(result, 0, result.length);
        parcelOut.setDataPosition(0);

        // if need the return data, use parcelOut at here
        // start
        // end

        parcelOut.recycle();
        parcelOut = null;
    }

    private class ImsSystem implements ISystem, MmTelFeatureRegistry.Listener {
        private ISystemAPICallInfo mISystemAPICallInfo;
        private ISystemAPINetwork mISystemAPINetwork;

        private ThreadMessageExecutor mExecutor =
                new ThreadMessageExecutor(SystemInterface.class.getSimpleName());

        private final Object mLock = new Object();
        private final ArraySet<Integer> mRegisteredEvents = new ArraySet<>();
        private final int mSlotId;
        private SystemCallInterface mSystemCall = null;
        private SystemRadioInterface mSystemRadio = null;

        ImsSystem(int slotId) {
            mSlotId = slotId;
        }

        public void init() {
            mExecutor.start();
            MmTelFeatureRegistry mtfr =
                    ImsServiceRegistry.getInstance(mSlotId).getMmTelFeatureRegistry();
            mtfr.addListener(this);
        }

        public void cleanup() {
            mExecutor = null;
            MmTelFeatureRegistry mtfr =
                    ImsServiceRegistry.getInstance(mSlotId).getMmTelFeatureRegistry();
            mtfr.removeListener(this);
        }

        @Override
        public int getSlotId() {
            return mSlotId;
        }

        @Override
        public void setSystemCallInterface(SystemCallInterface systemCall) {
            mSystemCall = systemCall;
        }

        @Override
        public void setSystemRadioInterface(SystemRadioInterface systemRadio) {
            mSystemRadio = systemRadio;
        }

        @Override
        public void setISystemAPICallInfo(ISystemAPICallInfo api) {
            synchronized (mLock) {
                mISystemAPICallInfo = api;
            }
        }

        @Override
        public void setISystemAPINetwork(ISystemAPINetwork api) {
            synchronized (mLock) {
                mISystemAPINetwork = api;
            }
        }

        /**
         * Notifies the changes of airplane mode in the phone settings.
         *
         * @param airplaneMode the current airplane mode status
         *          0: Airplane mode OFF, 1: Airplane mode ON
         */
        @Override
        public void notifyAirplaneModeChanged(final int airplaneMode) {
            mExecutor.execute(new Runnable() {
                @Override
                public void run() {
                    Parcel parcel = Parcel.obtain();

                    if (parcel == null) {
                        ImsLog.d("Parcel is null");
                        return;
                    }

                    // Write the event name
                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_AIRPLANE_MODE_CHANGED);
                    parcel.writeInt(airplaneMode);

                    sendData2Native(mNativeObject, parcel);
                }
            });
        }

        /**
         * Notifies the failure result to connect a data connection of the specified APN type.
         *
         * @param apnType the APN type (1: ims, 2: internet, 3: xcap, 9: emergency, 21: wifi)
         */
        @Override
        public void notifyDataConnectionFailed(final int apnType) {
            mExecutor.execute(new Runnable() {
                @Override
                public void run() {
                    Parcel parcel = Parcel.obtain();

                    if (parcel == null) {
                        ImsLog.d("Parcel is null");
                        return;
                    }

                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_DATA_CONNECTION_FAILED);
                    parcel.writeInt(apnType);

                    sendData2Native(mNativeObject, parcel);
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
        public void notifyDataConnectionIpcanChanged(final int apnType,
                final int ipcanCategory) {
            mExecutor.execute(new Runnable() {
                @Override
                public void run() {
                    Parcel parcel = Parcel.obtain();

                    if (parcel == null) {
                        ImsLog.d("Parcel is null");
                        return;
                    }

                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_DATA_CONNECTION_IPCAN_CHANGED);
                    parcel.writeInt(apnType);
                    parcel.writeInt(ipcanCategory);

                    sendData2Native(mNativeObject, parcel);
                }
            });
        }

        /**
         * Notifies the state of the specified data connection.
         *
         * @param apnType the APN type (1: ims, 2: internet, 3: xcap, 9: emergency, 21: wifi)
         * @param state the data state (TelephonyManager.DATA_*)
         *          {@link TelephonyManager.DATA_UNKNOWN} (-1)
         *          {@link TelephonyManager.DATA_DISCONNECTED} (0)
         *          {@link TelephonyManager.DATA_CONNECTING} (1)
         *          {@link TelephonyManager.DATA_CONNECTED} (2)
         *          {@link TelephonyManager.DATA_SUSPENDED} (3)
         */
        @Override
        public void notifyDataConnectionStateChanged(final int apnType, final int state) {
            mExecutor.execute(new Runnable() {
                @Override
                public void run() {
                    Parcel parcel = Parcel.obtain();

                    if (parcel == null) {
                        ImsLog.d("Parcel is null");
                        return;
                    }

                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_DATA_CONNECTION_STATE_CHANGED);
                    parcel.writeInt(apnType);
                    parcel.writeInt(state);

                    sendData2Native(mNativeObject, parcel);
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
        public void notifyNetworkTypeChanged(final int networkType) {
            mExecutor.execute(new Runnable() {
                @Override
                public void run() {
                    Parcel parcel = Parcel.obtain();

                    if (parcel == null) {
                        ImsLog.d("Parcel is null");
                        return;
                    }

                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_NETWORK_TYPE_CHANGED);
                    parcel.writeInt(networkType);

                    sendData2Native(mNativeObject, parcel);
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
        public void notifyVoiceNetworkTypeChanged(final int networkType) {
            mExecutor.execute(new Runnable() {
                @Override
                public void run() {
                    Parcel parcel = Parcel.obtain();

                    if (parcel == null) {
                        ImsLog.d("Parcel is null");
                        return;
                    }

                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_VOICE_NETWORK_TYPE_CHANGED);
                    parcel.writeInt(networkType);

                    sendData2Native(mNativeObject, parcel);
                }
            });
        }

        /**
         * Notifies the service state related to the attached network.
         *
         * @param serviceState the service state (ServiceState.STATE_*)
         *          {@link ServiceState.STATE_IN_SERVICE} (0)
         *          {@link ServiceState.STATE_OUT_OF_SERVICE} (1)
         *          {@link ServiceState.STATE_EMERGENCY_ONLY} (2)
         *          {@link ServiceState.STATE_POWER_OFF} (3)
         */
        @Override
        public void notifyServiceStateChanged(final int serviceState) {
            mExecutor.execute(new Runnable() {
                @Override
                public void run() {
                    Parcel parcel = Parcel.obtain();

                    if (parcel == null) {
                        ImsLog.d("Parcel is null");
                        return;
                    }

                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_SERVICE_STATE_CHANGED);
                    parcel.writeInt(serviceState);

                    sendData2Native(mNativeObject, parcel);
                }
            });
        }

        /**
         * Notifies the voice call (CS / IMS) state.
         *
         * @param state the call state (TelephonyManager.CALL_STATE_*)
         *          {@link TelephonyManager.CALL_STATE_IDLE} (0)
         *          {@link TelephonyManager.CALL_STATE_RINGING} (1)
         *          {@link TelephonyManager.CALL_STATE_OFFHOOK} (2)
         */
        @Override
        public void notifyVoiceCallStateChanged(final int state) {
            mExecutor.execute(new Runnable() {
                @Override
                public void run() {
                    Parcel parcel = Parcel.obtain();

                    if (parcel == null) {
                        ImsLog.d("Parcel is null");
                        return;
                    }

                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_VOICE_CALL_STATE_CHANGED);
                    parcel.writeInt(state);

                    sendData2Native(mNativeObject, parcel);
                }
            });
        }

        /**
         * Notifies the changes of the IMS configuration.
         *
         * @param configs the configuration items to be updated
         */
        @Override
        public void notifyConfigurationChanged(final int configs) {
            mExecutor.execute(new Runnable() {
                @Override
                public void run() {
                    Parcel parcel = Parcel.obtain();

                    if (parcel == null) {
                        ImsLog.d("Parcel is null");
                        return;
                    }

                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_CONFIGURATION_CHANGED);
                    parcel.writeInt(configs);

                    sendData2Native(mNativeObject, parcel);
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
        public void notifyEvent(final int event, final int param1, final int param2) {
            mExecutor.execute(new Runnable() {
                @Override
                public void run() {
                    if (!isEventRegistered(event)) {
                        ImsLog.w("Event (" + Integer.toString(event, 16) + ") is not registered");
                        return;
                    }

                    Parcel parcel = Parcel.obtain();

                    if (parcel == null) {
                        ImsLog.d("Parcel is null");
                        return;
                    }

                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_EVENT);
                    parcel.writeInt(event);
                    parcel.writeInt(param1);
                    parcel.writeInt(param2);

                    sendData2Native(mNativeObject, parcel);
                }
            });
        }

        @Override
         public void notifyIsimState(int event, String state) {
            mExecutor.execute(new Runnable() {
                @Override
                public void run() {
                    Parcel parcel = Parcel.obtain();

                    if (parcel == null) {
                        ImsLog.d("Parcel is null");
                        return;
                    }

                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_ISIM_EVENT);
                    parcel.writeInt(event);
                    parcel.writeString(state);

                    sendData2Native(mNativeObject, parcel);
                }
            });
        }

        @Override
        public void notifyIsimFileAttributesResponse(int event,
                int fileId, int size, String[] values) {
            mExecutor.execute(new Runnable() {
                @Override
                public void run() {
                    Parcel parcel = Parcel.obtain();

                    if (parcel == null) {
                        ImsLog.d("Parcel is null");
                        return;
                    }

                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_ISIM_EVENT);
                    parcel.writeInt(event);
                    parcel.writeInt(fileId);
                    parcel.writeInt(size);
                    for (int i = 0;  i < size; i++)
                    {
                        parcel.writeString(values[i]);
                    }

                    sendData2Native(mNativeObject, parcel);
                }
            });
        }

        @Override
        public void notifyIsimRecordResponse(int event,
                int fileId, int index, String value) {
            mExecutor.execute(new Runnable() {
                @Override
                public void run() {
                    Parcel parcel = Parcel.obtain();

                    if (parcel == null) {
                        ImsLog.d("Parcel is null");
                        return;
                    }

                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_ISIM_EVENT);
                    parcel.writeInt(event);
                    parcel.writeInt(fileId);
                    parcel.writeInt(index);
                    parcel.writeString(value);

                    sendData2Native(mNativeObject, parcel);
                }
            });
        }

        @Override
        public void notifyIsimAuthenticationResponse(int event, String response, long owner) {
            mExecutor.execute(new Runnable() {
                @Override
                public void run() {
                    Parcel parcel = Parcel.obtain();

                    if (parcel == null) {
                        ImsLog.d("Parcel is null");
                        return;
                    }

                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_ISIM_EVENT);
                    parcel.writeInt(event);
                    parcel.writeString(response);
                    parcel.writeLong(owner);

                    sendData2Native(mNativeObject, parcel);
                }
            });
        }

        @Override
        public void notifyUsimAuthenticationResponse(int event, String response, long owner) {
            mExecutor.execute(new Runnable() {
                @Override
                public void run() {
                    Parcel parcel = Parcel.obtain();

                    if (parcel == null) {
                        ImsLog.d("Parcel is null");
                        return;
                    }

                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_USIM_EVENT);
                    parcel.writeInt(event);
                    parcel.writeString(response);
                    parcel.writeLong(owner);

                    sendData2Native(mNativeObject, parcel);
                }
            });
        }

        @Override
        public void notifyRadioConnectionFailed(int event, int id, int failureReason, int causeCode,
                int waitTimeMillis) {
            mExecutor.execute(new Runnable() {
                @Override
                public void run() {
                    Parcel parcel = Parcel.obtain();

                    if (parcel == null) {
                        ImsLog.d("Parcel is null");
                        return;
                    }

                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_RADIO_EVENT);
                    parcel.writeInt(event);
                    parcel.writeInt(id);
                    parcel.writeInt(failureReason);
                    parcel.writeInt(causeCode);
                    parcel.writeInt(waitTimeMillis);

                    sendData2Native(mNativeObject, parcel);
                }
            });
        }

        @Override
        public void notifyRadioConnectionSetupPrepared(int event, int id) {
            mExecutor.execute(new Runnable() {
                @Override
                public void run() {
                    Parcel parcel = Parcel.obtain();

                    if (parcel == null) {
                        ImsLog.d("Parcel is null");
                        return;
                    }

                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_RADIO_EVENT);
                    parcel.writeInt(event);
                    parcel.writeInt(id);

                    sendData2Native(mNativeObject, parcel);
                }
            });
        }

        @Override
        public void notifySsacInfo(int event, int voiceFactor, int voiceTimeSec, int videoFactor,
                int videoTimeSec) {
            mExecutor.execute(new Runnable() {
                @Override
                public void run() {
                    Parcel parcel = Parcel.obtain();

                    if (parcel == null) {
                        ImsLog.d("Parcel is null");
                        return;
                    }

                    parcel.writeInt(mSlotId);
                    parcel.writeInt(SystemConstants.NOTIFY_RADIO_EVENT);
                    parcel.writeInt(event);
                    parcel.writeInt(voiceFactor);
                    parcel.writeInt(voiceTimeSec);
                    parcel.writeInt(videoFactor);
                    parcel.writeInt(videoTimeSec);

                    sendData2Native(mNativeObject, parcel);
                }
            });
        }

        @Override
        public void onAdvancedCallingSettingChanged() {
            MmTelFeatureRegistry mtfr =
                    ImsServiceRegistry.getInstance(mSlotId).getMmTelFeatureRegistry();
            notifyEvent(ImsEventDef.IMS_EVENT_VOLTE_SETTING,
                    mtfr.isAdvancedCallingSettingEnabled()
                    ? ImsEventDef.IMS_VOLTE_SETTING_ON
                    : ImsEventDef.IMS_VOLTE_SETTING_OFF, 0);
        }

        @Override
        public void onVoWiFiSettingChanged() {
            MmTelFeatureRegistry mtfr =
                    ImsServiceRegistry.getInstance(mSlotId).getMmTelFeatureRegistry();
            notifyEvent(ImsEventDef.IMS_EVENT_WFC_SETTING_CHANGED,
                    mtfr.isVoWiFiSettingEnabled()
                    ? ImsEventDef.IMS_WFC_ON
                    : ImsEventDef.IMS_WFC_OFF, mtfr.getVoWiFiModeSetting());
        }

        @Override
        public void onRttModeChanged() {
            MmTelFeatureRegistry mtfr =
                    ImsServiceRegistry.getInstance(mSlotId).getMmTelFeatureRegistry();
            notifyEvent(ImsEventDef.IMS_EVENT_RTT_SETTING, mtfr.getRttMode(), 0);
        }

        // Private/Protected methods ---------------------------------
        public boolean isEventRegistered(final int event) {
            synchronized (mLock) {
                return (mRegisteredEvents.contains(event));
            }
        }

        public void registerEvent(final int event) {
            synchronized (mLock) {
                mRegisteredEvents.add(event);
            }
        }

        public void unregisterEvent(final int event) {
            synchronized (mLock) {
                mRegisteredEvents.remove(event);
            }
        }

        public Parcel handleSystemAPI(int method, Parcel parcel, FileDescriptor fd) {
            if (method == SystemConstants.GET_HOST_BY_NAME) {
                final ISystemAPINetwork network = getSystemAPINetwork();
                return handleSystemAPINetwork(network, method, parcel);
            }

            Parcel result = null;

            synchronized (mLock) {
                switch (method) {
                    case SystemConstants.GET_TTY_MODE: //FALL-THROUGH
                    case SystemConstants.GET_RTT_MODE:
                        result = handleSystemCallForCallSettings(method);
                        break;
                    //mISystemAPINetwork 1 ~ 10
                    case SystemConstants.ACTIVATE_DATA_CONNECTION: //FALL-THROUGH
                    case SystemConstants.DEACTIVATE_DATA_CONNECTION: //FALL-THROUGH
                    case SystemConstants.GET_ACCESS_NETWORK_INFO: //FALL-THROUGH
                    case SystemConstants.GET_APN_NAME: //FALL-THROUGH
                    case SystemConstants.GET_DATA_CONNECTION_STATE: //FALL-THROUGH
                    case SystemConstants.GET_HOST_BY_NAME: //FALL-THROUGH
                    case SystemConstants.GET_IFACE_ID: //FALL-THROUGH
                    case SystemConstants.GET_IFACE_NAME: //FALL-THROUGH
                    case SystemConstants.GET_LAST_ACCESS_NETWORK_INFO: //FALL-THROUGH
                    case SystemConstants.GET_LOCAL_ADDRESS:
                        result = handleSystemAPINetwork1(method, parcel);
                        break;
                    //mISystemAPINetwork 11 ~ 20
                    case SystemConstants.GET_IPCAN_CATEGORY: //FALL-THROUGH
                    case SystemConstants.GET_PCSCF_ADDRESSES: //FALL-THROUGH
                    case SystemConstants.GET_ROAMING_STATE: //FALL-THROUGH
                    case SystemConstants.GET_SERVICE_STATE: //FALL-THROUGH
                    case SystemConstants.IS_LTE_EMERGENCY_ONLY: //FALL-THROUGH
                    case SystemConstants.IS_MOBILE_DATA_ENABLED://FALL-THROUGH
                    case SystemConstants.GET_MOCN_PLMN_INFO: //FALL-THROUGH
                    case SystemConstants.GET_VOICE_SERVICE_STATE: //FALL-THROUGH
                    case SystemConstants.GET_MTU: //FALL-THROUGH
                    case SystemConstants.IS_EMERGENCY_ATTACH_SUPPORTED: // FALL-THROUGH
                    case SystemConstants.BIND_SOCKET:
                        result = handleSystemAPINetwork2(method, parcel, fd);
                        break;
                    case SystemConstants.GET_ISIM_STATE: //FALL-THROUGH
                    case SystemConstants.READ_ISIM_FILE_ATTR: //FALL-THROUGH
                    case SystemConstants.READ_ISIM_RECORD: //FALL-THROUGH
                    case SystemConstants.REQUEST_ISIM_AUTH: //FALL-THROUGH
                    case SystemConstants.REQUEST_USIM_AUTH:
                        result = handleSystemCallForSim(method, parcel);
                        break;
                    case SystemConstants.GET_DEVICE_ID: //FALL-THROUGH
                    case SystemConstants.GET_DEVICE_SOFTWARE_VERSION: //FALL-THROUGH
                    case SystemConstants.GET_PHONE_NUMBER: //FALL-THROUGH
                    case SystemConstants.GET_SUBSCRIBER_ID: //FALL-THROUGH
                    case SystemConstants.GET_SIM_MCC: //FALL-THROUGH
                    case SystemConstants.GET_SIM_MNC: //FALL-THROUGH
                    case SystemConstants.GET_SIM_COUNTRY_ISO: //FALL-THROUGH
                    case SystemConstants.GET_NETWORK_COUNTRY_ISO: //FALL-THROUGH
                    case SystemConstants.GET_NETWORK_TYPE: //FALL-THROUGH
                    case SystemConstants.GET_VOICE_NETWORK_TYPE: //FALL-THROUGH
                    case SystemConstants.GET_CS_CALL_STATE: //FALL-THROUGH
                    case SystemConstants.GET_CS_CALL_STATE_IN_OTHER_SLOT: //FALL-THROUGH
                    case SystemConstants.IS_EMERGENCY_NUMBER:
                        result = handleSystemCallForTelephony(method, parcel);
                        break;
                    case SystemConstants.IS_WFC_ENABLED: //FALL-THROUGH
                    case SystemConstants.GET_WFC_PREFERENCES: //FALL-THROUGH
                    case SystemConstants.IS_WFC_PROVISIONED: //FALL-THROUGH
                    case SystemConstants.GET_WFC_ADDRESS_ID:
                        result = handleSystemCallForWifiCalling(method);
                        break;
                    case SystemConstants.START_LISTENING_FOR_LOCATION: //FALL-THROUGH
                    case SystemConstants.STOP_LISTENING_FOR_LOCATION: //FALL-THROUGH
                    case SystemConstants.GET_LAST_KNOWN_LOCATION: //FALL-THROUGH
                    case SystemConstants.START_INSTANT_LOCATION_UPDATE:
                        result = handleSystemCallForLocation(method, parcel);
                        break;
                    case SystemConstants.START_IMS_TRAFFIC: //FALL-THROUGH
                    case SystemConstants.STOP_IMS_TRAFFIC: //FALL-THROUGH
                    case SystemConstants.TRIGGER_EPS_FALLBACK:
                        result = handleSystemCallForRadio(method, parcel);
                        break;
                    case SystemConstants.ADD_IPSEC_SA_PARAMETER: // FALL-THROUGH
                    case SystemConstants.REMOVE_IPSEC_SA_PARAMETER: // FALL-THROUGH
                    case SystemConstants.APPLY_IPSEC_SA: // FALL-THROUGH
                    case SystemConstants.REMOVE_IPSEC_SA:
                        result = handleSystemCallForIpSec(method, parcel, fd);
                        break;
                    default:
                        result = handleSystemCall(method);
                        break;
                }
            }

            return result;
        }

        public Parcel handleSystemCallForEvent(int event, int wParam, int lParam) {
            if (mSystemCall == null) {
                return null;
            }

            Parcel result = Parcel.obtain();

            switch (event) {
                case ImsEventDef.IMS_EVENT_NATIVE_BOOT_COMPLETED:
                    result.writeInt(mSystemCall.updateNativeServiceReady(true));
                    break;
                default:
                    result.recycle();
                    return null;
            }

            return result;
        }

        private Parcel handleSystemCallForCallSettings(int method) {
            Parcel result = Parcel.obtain();

            switch (method) {
                case SystemConstants.GET_TTY_MODE: {
                    MmTelFeatureRegistry mtfr =
                            ImsServiceRegistry.getInstance(mSlotId).getMmTelFeatureRegistry();
                    result.writeInt(mtfr.getTtyMode());
                    break;
                }
                case SystemConstants.GET_RTT_MODE: {
                    MmTelFeatureRegistry mtfr =
                            ImsServiceRegistry.getInstance(mSlotId).getMmTelFeatureRegistry();
                    result.writeInt(mtfr.getRttMode());
                    break;
                }
                default:
                    result.recycle();
                    return null;
            }

            return result;

        }

        private Parcel handleSystemAPINetwork(ISystemAPINetwork network,
                int method, Parcel parcel) {
            if (network == null) {
                return null;
            }

            Parcel result = Parcel.obtain();
            switch (method) {
            case SystemConstants.GET_HOST_BY_NAME:
                String[] ipAddrs = network.getHostByName4Sys(
                            parcel.readInt(), parcel.readInt(), parcel.readString());

                if (ipAddrs == null) {
                    result.writeInt(0);
                    break;
                }

                result.writeInt(ipAddrs.length);

                for (int i = 0; i < ipAddrs.length; ++i) {
                    result.writeString(ipAddrs[i]);
                }
                break;
            default:
                result.recycle();
                return null;
            }

            return result;
        }

        private Parcel handleSystemAPINetwork1(int method, Parcel parcel) {
            if (mISystemAPINetwork == null) {
                return null;
            }

            Parcel result = Parcel.obtain();
            switch (method) {
                case SystemConstants.ACTIVATE_DATA_CONNECTION:
                    result.writeInt(mISystemAPINetwork.activateDataConnection4Sys(
                            parcel.readInt()));
                    break;
                case SystemConstants.DEACTIVATE_DATA_CONNECTION:
                    result.writeInt(mISystemAPINetwork.deactivateDataConnection4Sys(
                            parcel.readInt()));
                    break;
                case SystemConstants.GET_ACCESS_NETWORK_INFO:
                    int defaultNetworkType = parcel.readInt();
                    IDcUtils.AccessNetworkInfo ani =
                            mISystemAPINetwork.getAccessNetworkInfo4Sys(defaultNetworkType);

                    if (ani == null) {
                        result.writeInt(defaultNetworkType);
                        result.writeInt(0);
                        break;
                    }

                    result.writeInt(ani.mNetworkType);

                    if (ani.mAni == null) {
                        result.writeInt(0);
                    } else {
                        result.writeInt(ani.mAni.length);

                        for (int i = 0; i < ani.mAni.length; ++i) {
                            result.writeString(ani.mAni[i]);
                        }
                    }
                    break;
                case SystemConstants.GET_APN_NAME:
                    result.writeString(mISystemAPINetwork.getApnName4Sys(parcel.readInt()));
                    break;
                case SystemConstants.GET_DATA_CONNECTION_STATE:
                    result.writeInt(
                            mISystemAPINetwork.getDataConnectionState4Sys(parcel.readInt()));
                    break;
                case SystemConstants.GET_HOST_BY_NAME: {
                    int apnType = parcel.readInt();
                    int ipVersion = parcel.readInt();
                    String host = parcel.readString();
                    String[] ipAddrs = null;

                    if (apnType == EApnType.WIFI.getType()) {
                        WifiInterface wifi = (mDefaultSystemCall != null)
                                ? mDefaultSystemCall.getWifiInterface() : null;
                        if (wifi != null) {
                            ipAddrs = wifi.getHostByName(ipVersion, host);
                        }
                    } else {
                        ipAddrs = mISystemAPINetwork.getHostByName4Sys(apnType, ipVersion, host);
                    }

                    if (ipAddrs == null) {
                        result.writeInt(0);
                        break;
                    }

                    result.writeInt(ipAddrs.length);

                    for (int i = 0; i < ipAddrs.length; ++i) {
                        result.writeString(ipAddrs[i]);
                    }
                    break;
                }
                case SystemConstants.GET_IFACE_ID: {
                    int apnType = parcel.readInt();
                    int ifaceId = -1;

                    if (apnType == EApnType.WIFI.getType()) {
                        WifiInterface wifi = (mDefaultSystemCall != null)
                                ? mDefaultSystemCall.getWifiInterface() : null;
                        if (wifi != null) {
                            ifaceId = wifi.getIfaceId();
                        }
                    } else {
                        ifaceId = mISystemAPINetwork.getIfaceId4Sys(apnType);
                    }
                    result.writeInt(ifaceId);
                    break;
                }
                case SystemConstants.GET_IFACE_NAME: {
                    int apnType = parcel.readInt();
                    String ifaceName = null;

                    if (apnType == EApnType.WIFI.getType()) {
                        WifiInterface wifi = (mDefaultSystemCall != null)
                                ? mDefaultSystemCall.getWifiInterface() : null;
                        if (wifi != null) {
                            ifaceName = wifi.getIfaceName();
                        }
                    } else {
                        ifaceName = mISystemAPINetwork.getIfaceName4Sys(apnType);
                    }
                    result.writeString(ifaceName);
                    break;
                }
                case SystemConstants.GET_LAST_ACCESS_NETWORK_INFO:
                    String[] lastANInfo = mISystemAPINetwork.getLastAccessNetworkInfo4Sys(
                            parcel.readInt());

                    if (lastANInfo == null) {
                        result.writeInt(0);
                        break;
                    }

                    result.writeInt(lastANInfo.length);

                    for (int i = 0; i < lastANInfo.length; ++i) {
                        result.writeString(lastANInfo[i]);
                    }
                    break;
                case SystemConstants.GET_LOCAL_ADDRESS: {
                    int apnType = parcel.readInt();
                    int ipVersion = parcel.readInt();
                    String localAddress = null;

                    if (apnType == EApnType.WIFI.getType()) {
                        WifiInterface wifi = (mDefaultSystemCall != null)
                                ? mDefaultSystemCall.getWifiInterface() : null;
                        if (wifi != null) {
                            localAddress = wifi.getLocalAddress(ipVersion);
                        }
                    } else {
                        localAddress = mISystemAPINetwork.getLocalAddress4Sys(apnType, ipVersion);
                    }
                    result.writeString(localAddress);
                    break;
                }
                default:
                    result.recycle();
                    return null;
            }

            return result;
        }

        private Parcel handleSystemAPINetwork2(int method, Parcel parcel, FileDescriptor fd) {
            if (mISystemAPINetwork == null) {
                return null;
            }

            Parcel result = Parcel.obtain();
            switch (method) {
                case SystemConstants.GET_IPCAN_CATEGORY: {
                    int apnType = parcel.readInt();

                    if (apnType == EApnType.WIFI.getType()) {
                        result.writeInt(IApn.IPCAN_CATEGORY_WLAN);
                    } else {
                        result.writeInt(mISystemAPINetwork.getIpcanCategory4Sys(apnType));
                    }
                    break;
                }
                case SystemConstants.GET_PCSCF_ADDRESSES:
                    String[] pcscfs = mISystemAPINetwork.getPcscfAddresses4Sys(parcel.readInt(),
                        parcel.readInt());

                    if (pcscfs == null) {
                        result.writeInt(0);
                        break;
                    }

                    result.writeInt(pcscfs.length);

                    for (int i = 0; i < pcscfs.length; ++i) {
                        result.writeString(pcscfs[i]);
                    }
                    break;
                case SystemConstants.GET_ROAMING_STATE:
                    result.writeInt(mISystemAPINetwork.getRoamingState4Sys());
                    break;
                case SystemConstants.GET_SERVICE_STATE:
                    result.writeInt(mISystemAPINetwork.getServiceState4Sys());
                    break;
                case SystemConstants.IS_LTE_EMERGENCY_ONLY:
                    result.writeInt(mISystemAPINetwork.isLteEmergencyOnly4Sys());
                    break;
                case SystemConstants.IS_MOBILE_DATA_ENABLED:
                    result.writeInt(mISystemAPINetwork.isMobileDataEnabled() ? 1 : 0);
                    break;
                case SystemConstants.GET_MOCN_PLMN_INFO:
                    result.writeInt(mISystemAPINetwork.getMocnPlmnInfo4Sys());
                    break;
                case SystemConstants.GET_VOICE_SERVICE_STATE:
                    result.writeInt(mISystemAPINetwork.getVoiceServiceState4Sys());
                    break;
                case SystemConstants.GET_VOICE_ROAMING_TYPE:
                    result.writeInt(mISystemAPINetwork.getVoiceRoamingType4Sys());
                    break;
                case SystemConstants.GET_DATA_ROAMING_TYPE:
                    result.writeInt(mISystemAPINetwork.getDataRoamingType4Sys());
                    break;
                case SystemConstants.GET_MTU: {
                    int apnType = parcel.readInt();
                    int mtu = 0; // Use a default MTU size (1500)

                    if (apnType == EApnType.WIFI.getType()) {
                        WifiInterface wifi = (mDefaultSystemCall != null)
                                ? mDefaultSystemCall.getWifiInterface() : null;
                        if (wifi != null) {
                            mtu = wifi.getMtu();
                        }
                    } else {
                        mtu = mISystemAPINetwork.getMtu4Sys(apnType);
                    }
                    result.writeInt(mtu);
                    break;
                }
                case SystemConstants.IS_EMERGENCY_ATTACH_SUPPORTED:
                    result.writeInt(mISystemAPINetwork.isEmergencyAttachSupported4Sys());
                    break;
                case SystemConstants.BIND_SOCKET: {
                    int apnType = parcel.readInt();
                    int bindResult = 0;

                    if (apnType == EApnType.WIFI.getType()) {
                        WifiInterface wifi = (mDefaultSystemCall != null)
                                ? mDefaultSystemCall.getWifiInterface() : null;
                        if (wifi != null) {
                            bindResult = wifi.bindSocket(fd);
                        }
                    } else {
                        bindResult = mISystemAPINetwork.bindSocket(apnType, fd);
                    }
                    result.writeInt(bindResult);
                    break;
                }
                default:
                    result.recycle();
                    return null;
            }

            return result;
        }

        private Parcel handleSystemCallForSim(int method, Parcel parcel) {
            if (mSystemCall == null) {
                return null;
            }

            Parcel result = Parcel.obtain();

            switch (method) {
                case SystemConstants.GET_ISIM_STATE:
                    result.writeString(mSystemCall.getIsimState());
                    break;
                case SystemConstants.READ_ISIM_FILE_ATTR: {
                    int fileId = parcel.readInt();
                    result.writeInt(mSystemCall.readIsimFileAttributes(fileId));
                    break;
                }
                case SystemConstants.READ_ISIM_RECORD: {
                    int fileId = parcel.readInt();
                    int index = parcel.readInt();
                    result.writeInt(mSystemCall.readIsimRecord(fileId, index));
                    break;
                }
                case SystemConstants.REQUEST_ISIM_AUTH: {
                    String nonce = parcel.readString();
                    long owner = parcel.readLong();
                    result.writeInt(mSystemCall.requestIsimAuthentication(nonce, owner));
                    break;
                }
                case SystemConstants.REQUEST_USIM_AUTH: {
                    String nonce = parcel.readString();
                    long owner = parcel.readLong();
                    result.writeInt(mSystemCall.requestUsimAuthentication(nonce, owner));
                    break;
                }
                default:
                    result.recycle();
                    return null;
            }

            return result;
        }

        private Parcel handleSystemCallForTelephony(int method, Parcel parcel) {
            if (mSystemCall == null) {
                return null;
            }

            Parcel result = Parcel.obtain();
            switch (method) {
                case SystemConstants.GET_DEVICE_ID:
                    result.writeString(mSystemCall.getImei());
                    break;
                case SystemConstants.GET_DEVICE_SOFTWARE_VERSION:
                    result.writeString(mSystemCall.getDeviceSoftwareVersion());
                    break;
                case SystemConstants.GET_PHONE_NUMBER:
                    result.writeString(mSystemCall.getPhoneNumber());
                    break;
                case SystemConstants.GET_SUBSCRIBER_ID:
                    result.writeString(mSystemCall.getSubscriberId());
                    break;
                case SystemConstants.GET_SIM_MCC:
                    result.writeString(mSystemCall.getSimMcc());
                    break;
                case SystemConstants.GET_SIM_MNC:
                    result.writeString(mSystemCall.getSimMnc());
                    break;
                case SystemConstants.GET_SIM_COUNTRY_ISO:
                    result.writeString(mSystemCall.getSimCountryIso());
                    break;
                case SystemConstants.GET_NETWORK_COUNTRY_ISO:
                    result.writeString(mSystemCall.getNetworkCountryIso());
                    break;
                case SystemConstants.GET_NETWORK_TYPE:
                    result.writeInt(mSystemCall.getNetworkType());
                    break;
                case SystemConstants.GET_VOICE_NETWORK_TYPE:
                    result.writeInt(mSystemCall.getVoiceNetworkType());
                    break;
                case SystemConstants.GET_CS_CALL_STATE:
                    result.writeInt(mSystemCall.getCsCallState());
                    break;
                case SystemConstants.GET_CS_CALL_STATE_IN_OTHER_SLOT:
                    result.writeInt(mSystemCall.getCsCallStateInOtherSlot());
                    break;
                case SystemConstants.IS_EMERGENCY_NUMBER: {
                    boolean isEmergencyNumber = mSystemCall.isEmergencyNumber(parcel.readString());
                    result.writeInt(isEmergencyNumber ? 1 : 0);
                    break;
                }
                default:
                    result.recycle();
                    return null;
            }

            return result;
        }

        private Parcel handleSystemCallForWifiCalling(int method) {
            Parcel result = Parcel.obtain();
            switch (method) {
                case SystemConstants.IS_WFC_ENABLED: {
                    MmTelFeatureRegistry mtfr =
                            ImsServiceRegistry.getInstance(mSlotId).getMmTelFeatureRegistry();
                    result.writeInt(mtfr.isVoWiFiSettingEnabled() ? 1 : 0);
                    break;
                }
                case SystemConstants.GET_WFC_PREFERENCES: {
                    MmTelFeatureRegistry mtfr =
                            ImsServiceRegistry.getInstance(mSlotId).getMmTelFeatureRegistry();
                    result.writeInt(mtfr.getVoWiFiModeSetting());
                    break;
                }
                case SystemConstants.IS_WFC_PROVISIONED:
                    // Need to be checked whether this is necessary or not.
                    result.writeInt(1);
                    break;
                case SystemConstants.GET_WFC_ADDRESS_ID: {
                    result.writeString(ImsPrivateProperties.Persistent.get(
                            ImsPrivateProperties.Persistent.KEY_VOWIFI_ENTITLEMENT_ID,
                            "", mSlotId));
                    break;
                }
                default:
                    result.recycle();
                    return null;
            }

            return result;
        }

        private Parcel handleSystemCallForLocation(int method, Parcel parcel) {
            if (mSystemCall == null) {
                return null;
            }

            Parcel result = Parcel.obtain();
            switch (method) {
                case SystemConstants.START_LISTENING_FOR_LOCATION:
                    mSystemCall.startListeningForLocation(parcel.readInt());
                    result.writeInt(1);
                    break;
                case SystemConstants.STOP_LISTENING_FOR_LOCATION:
                    mSystemCall.stopListeningForLocation();
                    result.writeInt(1);
                    break;
                case SystemConstants.GET_LAST_KNOWN_LOCATION:
                    String[] locationParam = mSystemCall.getLastKnownLocation(parcel.readInt());

                    if (locationParam == null) {
                        result.writeInt(0);
                        break;
                    }

                    result.writeInt(locationParam.length);

                    for (int i = 0; i < locationParam.length; ++i) {
                        result.writeString(locationParam[i]);
                    }
                    break;
                case SystemConstants.START_INSTANT_LOCATION_UPDATE:
                    mSystemCall.startInstantLocationUpdate();
                    result.writeInt(1);
                    break;
                default:
                    result.recycle();
                    return null;
            }

            return result;
        }

        private Parcel handleSystemCallForRadio(int method, Parcel parcel) {
            if (mSystemRadio == null) {
                return null;
            }

            Parcel result = Parcel.obtain();

            switch (method) {
                case SystemConstants.START_IMS_TRAFFIC: {
                    int id = parcel.readInt();
                    int trafficType = parcel.readInt();
                    int accessNetworkType = parcel.readInt();
                    int direction = parcel.readInt();

                    result.writeInt(mSystemRadio.startImsTraffic(id, trafficType,
                            accessNetworkType, direction));
                    break;
                }
                case SystemConstants.STOP_IMS_TRAFFIC: {
                    int id = parcel.readInt();

                    mSystemRadio.stopImsTraffic(id);
                    break;
                }
                case SystemConstants.TRIGGER_EPS_FALLBACK: {
                    int reason = parcel.readInt();

                    result.writeInt(mSystemRadio.triggerEpsFallback(reason));
                    break;
                }
                default:
                    result.recycle();
                    return null;
            }

            return result;
        }

        // IpSec
        private Parcel handleSystemCallForIpSec(int method, Parcel parcel, FileDescriptor fd) {
            if (mSystemCall == null) {
                return null;
            }

            Parcel result = Parcel.obtain();
            switch (method) {
                case SystemConstants.ADD_IPSEC_SA_PARAMETER: {
                    IpSecSaParameter param = IpSecSaParameter.CREATOR.createFromParcel(parcel);
                    result.writeInt(mSystemCall.addIpSecSaParameter(param));
                    break;
                }
                case SystemConstants.REMOVE_IPSEC_SA_PARAMETER:
                    mSystemCall.removeIpSecSaParameter(parcel.readInt());
                    break;
                case SystemConstants.APPLY_IPSEC_SA: {
                    int ipsecId = parcel.readInt();
                    int spi = parcel.readInt();
                    int intFd = parcel.readInt();
                    result.writeInt(mSystemCall.applyIpSecSa(ipsecId, spi, intFd, fd));
                    break;
                }
                case SystemConstants.REMOVE_IPSEC_SA: {
                    int ipsecId = parcel.readInt();
                    int spi = parcel.readInt();
                    int intFd = parcel.readInt();
                    mSystemCall.removeIpSecSa(ipsecId, spi, intFd, fd);
                    break;
                }
                default:
                    result.recycle();
                    return null;
            }

            return result;
        }

        private ISystemAPINetwork getSystemAPINetwork() {
            synchronized (mLock) {
                return mISystemAPINetwork;
            }
        }

        private Parcel handleSystemCall(int method) {
            if (mSystemCall == null) {
                return null;
            }

            Parcel result = null;

            switch (method) {
                case SystemConstants.GET_CARRIER_CONFIG: {
                    CarrierConfig cc = mSystemCall.getCarrierConfig();

                    if (cc != null) {
                        result = Parcel.obtain();
                        result.writeInt(1);
                        cc.writeToParcel(result);
                    }
                    break;
                }
                case SystemConstants.IS_IMS_VOICE_CALL_SUPPORTED: {
                    result = Parcel.obtain();
                    result.writeInt(mSystemCall.isImsVoiceCallSupported() ? 1 : 0);
                    break;
                }
                default: {
                    break;
                }
            }

            return result;
        }
    }
}
