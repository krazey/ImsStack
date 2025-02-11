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

package com.android.imsstack.core.agents.dcm;

import android.annotation.NonNull;
import android.content.Context;
import android.net.ConnectivityManager;
import android.net.LinkAddress;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.net.TelephonyNetworkSpecifier;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.telephony.Annotation.NetworkType;
import android.telephony.DataFailCause;
import android.telephony.PreciseDataConnectionState;
import android.telephony.TelephonyManager;
import android.telephony.data.ApnSetting;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.SystemServiceProxy.ConnectivityManagerProxy;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.MsgProcInterface;
import com.android.imsstack.core.agents.dcmif.DcConstants;
import com.android.imsstack.core.agents.dcmif.EApnReqState;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.EIpVersion;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.agents.dcmif.IDcSettings;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

/**
 * This class is for providing the data connection interface
 */
public abstract class Apn extends Handler implements IApn {

    // Constants--------------------------------------------------
    protected static final int EVENT_NETWORK_AVAILABLE = 101;
    protected static final int EVENT_NETWORK_BLOCKED_STATUS_CHANGED = 102;
    protected static final int EVENT_NETWORK_CAPABILITIES_CHANGED = 103;
    protected static final int EVENT_NETWORK_LINK_PROPERTIES_CHANGED = 104;
    protected static final int EVENT_NETWORK_LOSING = 105;
    protected static final int EVENT_NETWORK_UNAVAILABLE = 106;
    protected static final int EVENT_NETWORK_LOST = 107;

    protected static final int EVENT_IP_CHANGED = 1001;
    protected static final int EVENT_PCSCF_CHANGED = 1002;
    protected static final int EVENT_WAITING_IPV6_ADDRESS = 1003;
    protected static final int EVENT_NOTIFY_DATA_STATE_CHANGED = 1004;
    protected static final int EVENT_PRECISE_DATA_CONNECTION_STATE_CHANGED = 1005;
    protected static final int EVENT_DATA_CONNECTION_FAILED = 1006;
    protected static final int EVENT_DEFAULT_NETWORK_STATUS_CHANGED = 1007;

    protected static final int EVENT_AIRPLANE_MODE_CHANGED = 2001;

    protected static final int FEATURE_NONE = 0;
    protected static final int FEATURE_IPV6_DELAY = 0x00000001;

    // Variables--------------------------------------------------
    protected static final LinkedHashMap<Integer, String> sEventToString;

    static {
        sEventToString = new LinkedHashMap<Integer, String>();

        // Network callbacks
        sEventToString.put(EVENT_NETWORK_AVAILABLE,
                "NETWORK_AVAILABLE");
        sEventToString.put(EVENT_NETWORK_BLOCKED_STATUS_CHANGED,
                "NETWORK_BLOCKED_STATUS_CHANGED");
        sEventToString.put(EVENT_NETWORK_CAPABILITIES_CHANGED,
                "NETWORK_CAPABILITIES_CHANGED");
        sEventToString.put(EVENT_NETWORK_LINK_PROPERTIES_CHANGED,
                "NETWORK_LINK_PROPERTIES_CHANGED");
        sEventToString.put(EVENT_NETWORK_LOSING,
                "NETWORK_LOSING");
        sEventToString.put(EVENT_NETWORK_UNAVAILABLE,
                "NETWORK_UNAVAILABLE");
        sEventToString.put(EVENT_NETWORK_LOST,
                "NETWORK_LOST");

        sEventToString.put(EVENT_IP_CHANGED,
                "IP_CHANGED");
        sEventToString.put(EVENT_PCSCF_CHANGED,
                "PCSCF_CHANGED");
        sEventToString.put(EVENT_WAITING_IPV6_ADDRESS,
                "WAITING_IPV6_ADDRESS");
        sEventToString.put(EVENT_NOTIFY_DATA_STATE_CHANGED,
                "NOTIFY_DATA_STATE_CHANGED");
        sEventToString.put(EVENT_PRECISE_DATA_CONNECTION_STATE_CHANGED,
                "PRECISE_DATA_CONNECTION_STATE_CHANGED");
        sEventToString.put(EVENT_DATA_CONNECTION_FAILED,
                "DATA_CONNECTION_FAILED");
        sEventToString.put(EVENT_DEFAULT_NETWORK_STATUS_CHANGED,
                "DEFAULT_NETWORK_STATUS_CHANGED");

        sEventToString.put(EVENT_AIRPLANE_MODE_CHANGED,
                "AIRPLANE_MODE_CHANGED");
    }

    protected Context mContext;
    protected IDcApn mDcApn;
    protected IDcSettings mDcSettings;
    protected IDcNetWatcher mDcNetWatcher;
    protected ISystem mSystem;
    protected final int mSlotId;
    protected EApnType mType;
    protected EApnReqState mAPNState = EApnReqState.APN_REQUEST_IDLE;
    protected int mDataState = TelephonyManager.DATA_DISCONNECTED;
    protected String mApnString = null;
    protected int mApnProtocol = ApnSetting.PROTOCOL_IPV4V6;
    protected int mNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
    protected int mPreciseDcState = TelephonyManager.DATA_UNKNOWN;
    protected int mIpcanCategory = IPCAN_CATEGORY_MOBILE;
    protected final LinkedHashMap<Integer, MsgProcInterface> mMapMsgHandler =
            new LinkedHashMap<Integer, MsgProcInterface>();
    protected ImsNetworkCallback mNetworkCallback = null;
    protected ImsNetworkCallback mNetworkMonitoringCallback = null;
    protected boolean mIsMonitoringCallbackRegistered = false;
    protected int mSubId = MSimUtils.INVALID_SUB_ID;
    protected ConfigInterface.Listener mConfigListener;
    protected Set<Listener> mListeners = new CopyOnWriteArraySet<>();

    protected Apn(Context context, int slotId, EApnType type) {
        super(Looper.myLooper());

        mContext = context;
        mSlotId = slotId;
        mType = type;
        mDcApn = DcFactory.getDcAgent(IDcApn.class, mSlotId);
        mDcSettings = DcFactory.getDcAgent(IDcSettings.class, mSlotId);
        mDcNetWatcher = DcFactory.getDcAgent(IDcNetWatcher.class, mSlotId);
        mSystem = SystemInterface.getInstance().getSystem(mSlotId);

        registerHandler(EVENT_NOTIFY_DATA_STATE_CHANGED, new HandleDataStateChanged());
        registerHandler(EVENT_PRECISE_DATA_CONNECTION_STATE_CHANGED,
                new HandlePreciseDataConnectionStateChanged());
    }

    // Interface implementation methods --------------------------
    @Override
    public void cleanup() {
        ImsLog.d(mSlotId, "clean up");

        unregisterConfigListener();

        unregisterCallback();

        if (mAPNState == EApnReqState.APN_REQUEST_DONE) {
            releaseNetwork();
            mAPNState = EApnReqState.APN_REQUEST_IDLE;
        }

        if (mNetworkCallback != null) {
            mNetworkCallback.cleanUp();
            mNetworkCallback = null;
        }

        if (mNetworkMonitoringCallback != null) {
            mNetworkMonitoringCallback.cleanUp();
            mNetworkMonitoringCallback = null;
        }

        Message msg = Message.obtain();
        msg.what = EVENT_NOTIFY_DATA_STATE_CHANGED;
        msg.arg1 = mType.getType();
        msg.arg2 = EDataState.DATA_STATE_DISCONNECTED.getState();
        handleMessage(msg);

        removeCallbacksAndMessages(null);
    }

    @Override
    public void handleMessage(Message msg) {
        // pumping to hash table.
        // Apn class provide simple event pumping table through "registerHandler" api
        // for derived classes
        MsgProcInterface msgProcess = mMapMsgHandler.get(msg.what);
        if (msgProcess != null) {
            ImsLog.i(mSlotId, "Apn :: apn=" + mType + ", msg=" + msg.what
                    + ", proc=" + sEventToString.get(msg.what));

            msgProcess.procMsg(msg);
        }
    }

    @Override
    public Context getContext() {
        return mContext;
    }

    @Override
    public void addListener(Listener listener) {
        mListeners.add(listener);
    }

    @Override
    public void removeListener(Listener listener) {
        mListeners.remove(listener);
    }

    @Override
    public boolean connect() {
        // Child Class Need to Implement
        return false;
    }

    @Override
    public boolean disconnect() {
        // Child Class Need to Implement
        return false;
    }

    @Override
    public String getApn() {
        if (mApnString != null) {
            return mApnString;
        }
        return mType.getString();
    }

    @Override
    public boolean isConnected() {
        return (mPreciseDcState != TelephonyManager.DATA_UNKNOWN
                && mPreciseDcState != TelephonyManager.DATA_DISCONNECTED
                && mPreciseDcState != TelephonyManager.DATA_CONNECTING);
    }

    @Override
    public int getDataState() {
        return this.mDataState;
    }

    protected void setDataState(int newState) {
        if (mDataState != newState) {
            mDataState = newState;
        }
    }

    @Override
    public int getIpcanCategory() {
        return mIpcanCategory;
    }

    @Override
    public int getIpVersion() {
        if (isConnected()) {
            if (mApnProtocol == ApnSetting.PROTOCOL_IP) {
                return EIpVersion.IPV4V6.getInt();
            }
            if (mApnProtocol == ApnSetting.PROTOCOL_IPV6) {
                return EIpVersion.IPV6V4.getInt();
            }
        }
        if (mDcSettings != null) {
            if (mType.getType() == DcConstants.TYPE_EMERGENCY
                    && mDcSettings.getEmergencyPreferredIpVersion()
                            == CarrierConfig.Ims.IPV4_PREFERRED) {
                return EIpVersion.IPV4V6.getInt();
            }

            if (mType.getType() == DcConstants.TYPE_IMS
                    && mDcSettings.getPreferredIpVersion()
                            == CarrierConfig.Ims.IPV4_PREFERRED) {
                return EIpVersion.IPV4V6.getInt();
            }
        }
        return EIpVersion.IPV6V4.getInt();
    }

    @Override
    public int getSlotId() {
        return this.mSlotId;
    }

    @Override
    public Network getCachedNetwork() {
        if (mNetworkCallback != null) {
            return mNetworkCallback.getCachedNetwork();
        } else if (mNetworkMonitoringCallback != null) {
            return mNetworkMonitoringCallback.getCachedNetwork();
        }
        return null;
    }

    @Override
    public String toString() {
        return ", ApnType= " + mType
                + ", DataState= " + mDataState
                + ", APNState= " + mAPNState;
    }

    protected void registerCallback(int events) {
        ImsLog.i(mSlotId, "Apn=" + mType.getString() + ", events=" + events);

        if (events == 0) {
            return;
        }

        ConnectivityManagerProxy cmp = getConnectivityManagerProxy();
        NetworkRequest.Builder nrb = new NetworkRequest.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);

        if (DeviceConfig.isMultiSimEnabled()) {
            mSubId = MSimUtils.getSubId(mSlotId);
            registerConfigListener();

            nrb.setNetworkSpecifier(new TelephonyNetworkSpecifier.Builder()
                    .setSubscriptionId(mSubId).build());
        }

        NetworkRequest nr = null;
        if (mType.getType() == DcConstants.TYPE_IMS) {
            nr = nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_IMS).build();
        } else if (mType.getType() == DcConstants.TYPE_INTERNET) {
            nr = nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET).build();
        } else if (mType.getType() == DcConstants.TYPE_XCAP) {
            nr = nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_XCAP).build();
        } else if (mType.getType() == DcConstants.TYPE_EMERGENCY) {
            nr = nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_EIMS).build();
        }

        if (mNetworkMonitoringCallback == null) {
            mNetworkMonitoringCallback = new ImsNetworkCallback(mType.getType(), events, this);
            mNetworkMonitoringCallback.setSlotId(mSlotId);
        } else {
            mNetworkMonitoringCallback.setEvents(events);
        }

        if (nr != null) {
            cmp.registerNetworkCallback(nr, mNetworkMonitoringCallback, this);
            mIsMonitoringCallbackRegistered = true;
        }
    }

    protected void unregisterCallback() {
        ImsLog.i(mSlotId, "Apn=" + mType.getString());

        if (mNetworkMonitoringCallback != null) {
            mIsMonitoringCallbackRegistered = false;

            ConnectivityManagerProxy cmp = getConnectivityManagerProxy();
            try {
                cmp.unregisterNetworkCallback(mNetworkMonitoringCallback);
            } catch (IllegalArgumentException e) {
                ImsLog.e("" + e);
            }
        }
    }

    protected void requestNetwork() {
        ImsLog.i(mSlotId, "Apn=" + mType.getString());
        ConnectivityManagerProxy cmp = getConnectivityManagerProxy();
        NetworkRequest.Builder nrb = new NetworkRequest.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);

        if (DeviceConfig.isMultiSimEnabled()) {
            boolean needToSetSubId = true;
            int subId = MSimUtils.getSubId(mSlotId);

            // This condition is aligned with A-TEL.
            if (mType.getType() == DcConstants.TYPE_EMERGENCY
                    && !MSimUtils.isValidSubId(subId)) {
                needToSetSubId = false;
            }

            if (needToSetSubId) {
                nrb.setNetworkSpecifier(new TelephonyNetworkSpecifier.Builder()
                        .setSubscriptionId(subId).build());
            }
        }

        NetworkRequest nr = null;
        if (mType.getType() == DcConstants.TYPE_IMS) {
            if (!mDcSettings.isImsPdnRequestWithoutMmtelRequired()) {
                nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_MMTEL);
            }
            nr = nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_IMS).build();
        } else if (mType.getType() == DcConstants.TYPE_INTERNET) {
            nr = nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET).build();
        } else if (mType.getType() == DcConstants.TYPE_XCAP) {
            nr = nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_XCAP).build();
        } else if (mType.getType() == DcConstants.TYPE_EMERGENCY) {
            nr = nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_EIMS).build();
        }

        if (mNetworkCallback == null) {
            mNetworkCallback = new ImsNetworkCallback(mType.getType(), this);
            mNetworkCallback.setSlotId(mSlotId);
        }

        if (nr != null) {
            cmp.requestNetwork(nr, mNetworkCallback, this);
        }
    }

    protected void releaseNetwork() {
        ImsLog.d(mSlotId, "Apn=" + mType.getString());

        if (mNetworkCallback != null) {
            ConnectivityManagerProxy cmp = getConnectivityManagerProxy();
            try {
                cmp.unregisterNetworkCallback(mNetworkCallback);
            } catch (IllegalArgumentException e) {
                ImsLog.e("" + e);
            }
            mNetworkCallback.cleanUp();
            mNetworkCallback = null;
        }
    }

    protected final void registerHandler(int evt, MsgProcInterface proc) {
        mMapMsgHandler.put(evt, proc);
    }

    protected EApnReqState getApnReqState() {
        return mAPNState;
    }

    protected void setApnReqState(EApnReqState s) {
        mAPNState = s;
    }

    protected void registerConfigListener() {
        if (mConfigListener == null) {
            mConfigListener = new ConfigInterface.Listener() {
                @Override
                public void onCarrierConfigChanged(int slotId, int subId) {
                    handleCarrierConfigChanged(slotId, subId);
                }
            };

            ConfigInterface config = AgentFactory.getInstance().getAgent(
                    ConfigInterface.class, mSlotId);
            if (config != null) {
                config.addListener(mConfigListener);
            }
        }
    }

    protected void unregisterConfigListener() {
        if (mConfigListener != null) {
            ConfigInterface config = AgentFactory.getInstance().getAgent(
                    ConfigInterface.class, mSlotId);
            if (config != null) {
                config.removeListener(mConfigListener);
            }
            mConfigListener = null;
        }
    }

    protected int getIpcanCategory(int networkType) {
        return (networkType == TelephonyManager.NETWORK_TYPE_IWLAN)
                ? IPCAN_CATEGORY_WLAN : IPCAN_CATEGORY_MOBILE;
    }

    protected boolean notifyDataConnectionIpcanChanged(int networkType) {
        int category = getIpcanCategory(networkType);

        if (mIpcanCategory == category) {
            return false;
        }

        ImsLog.i(mSlotId, "type = " + mType.getString() + " , old cat = "
                + mIpcanCategory + " , curr cat = " + category);

        mIpcanCategory = category;

        if (mSystem != null) {
            mSystem.notifyDataConnectionIpcanChanged(mType.getType(), mIpcanCategory);
        }

        return true;
    }

    protected void notifyPdnConnectionFailed(EApnType apnType, int smCause) {
        ImsLog.i(mSlotId, "apnType : " + apnType + ", smCause : " + smCause);

        //notify to watcher
        if (mDcNetWatcher != null) {
            mDcNetWatcher.notifyPdnConnectionFailed(apnType, smCause);
        }
    }

    protected boolean hasLocalAddress(int version) {
        if (mDcApn != null) {
            String ip = mDcApn.getLocalAddress(mType.getType(), version);

            if (ip != null) {
                return true;
            }
        }

        return false;
    }

    protected boolean isIpChanged() {
        if (mDcApn != null) {
            String cachedIP = mDcApn.getCachedLocalAddress(mType.getType());
            String ip = mDcApn.getLocalAddress(mType.getType(), 0);

            if (cachedIP == null || ip == null) {
                return true;
            }

            return !cachedIP.equals(ip);
        }

        return true;
    }

    protected void updateDataState() {
        int dataState = isConnected() ? TelephonyManager.DATA_CONNECTED
                : TelephonyManager.DATA_DISCONNECTED;

        if (mDataState != dataState) {
            ImsLog.i(mSlotId, "data state :: " + mDataState + " >> " + dataState);

            setDataState(dataState);
            if (mDataState == TelephonyManager.DATA_CONNECTED) {
                notifyDataConnectionIpcanChanged(mNetworkType);
            }
            sendDataStateUpdateMessage(mType, EDataState.convertIntTypeToEnum(
                    (EDataState.convertFromTMtoImsType(mDataState))));
        }
    }

    /**
     * Send message to oneself(Apn) to clean up call stack.
     * after sometime.. we will invoke JNI api to notify network data status.
     *
     * @see HandleDataStateChanged
     *
     * @param apnType IMS,Emergency...
     * @param dataState Data state to be updated
     */
    protected void sendDataStateUpdateMessage(EApnType apnType, EDataState dataState) {
        ImsLog.i(mSlotId, "apnType : " + apnType + ", state : " + dataState);

        //notify to watcher
        if (mDcNetWatcher != null) {
            mDcNetWatcher.notifyDataConnectionState(apnType, dataState);
        }

        //notify to apn
        Message msg = Message.obtain();
        msg.what = EVENT_NOTIFY_DATA_STATE_CHANGED;
        msg.arg1 = apnType.getType();
        msg.arg2 = dataState.getState();

        sendMessage(msg);
    }

    protected boolean handleIpcanCategory(int networkType) {
        if ((mType.getType() != DcConstants.TYPE_IMS)
                && (mType.getType() != DcConstants.TYPE_EMERGENCY)
                && (mType.getType() != DcConstants.TYPE_XCAP)) {
            return false;
        }

        if (!notifyDataConnectionIpcanChanged(networkType)) {
            return false;
        }

        // Notifies the change of IPCAN category
        notifyIpcanCategoryChanged(mIpcanCategory);

        return true;
    }

    protected void handleCarrierConfigChanged(int slotId, int subId) {
        if (subId == mSubId) {
            return;
        }

        ImsLog.d("onCarrierConfigChanged :: subId=" + mSubId + "->" + subId
                + ", slotId=" + slotId);

        if (mIsMonitoringCallbackRegistered) {
            unregisterCallback();
            registerCallback(mNetworkMonitoringCallback.getEvents());
        }
    }

    /**
     * Notifies the change of CrossSim connection status.
     * @param networkType The type of access network that is carry this data connection
     */
    protected void updateCrossSimStatus(@NetworkType int networkType) {
        // ApnIms need to implement because CrossSim feature is only available for IMS type
    }

    /**
     * Notifies that IPCAN(IP Connectivity Access Network) category is changed.
     */
    protected void notifyIpcanCategoryChanged(int ipcanCategory) {
        ImsLog.i(mSlotId, "notifyIpcanCategoryChanged");
        for (Listener l : mListeners) {
            l.onIpcanCategoryChanged(mType.getType(), ipcanCategory);
        }
    }

    /**
     * Notifies that state of handover between WWAN and WLAN is changed.
     */
    protected void notifyHandoverStateChanged(int handoverState, int networkType, int failCause) {
        ImsLog.i(mSlotId, "notifyHandoverStateChanged");
        for (Listener l : mListeners) {
            l.onHandoverStateChanged(handoverState, networkType, failCause);
        }
    }

    /**
     * Notifies that data connection state is changed.
     */
    protected void notifyConnectionStateChanged(int state, int failCause, int networkType) {
        for (Listener l : mListeners) {
            l.onPreciseDataConnectionStateChanged(mType.getType(), state, failCause, networkType);
        }
    }

    protected static ConnectivityManagerProxy getConnectivityManagerProxy() {
        return AppContext.getInstance().getSystemServiceProxy(ConnectivityManagerProxy.class);
    }

    /**
     * This class is for network callback interface
     */
    public static class ImsNetworkCallback extends ConnectivityManager.NetworkCallback {
        public static final int EVENT_AVAILABLE = 0x00000001;
        public static final int EVENT_LOSING = 0x00000002;
        public static final int EVENT_LOST = 0x00000004;
        public static final int EVENT_UNAVAILABLE = 0x00000008;
        public static final int EVENT_CAPABILITIES_CHANGED = 0x00000010;
        public static final int EVENT_LINK_PROPERTIES_CHANGED = 0x00000020;
        public static final int EVENT_LOCAL_IP_CHANGED = 0x00000040;
        public static final int EVENT_NET_PCSCF_CHANGED = 0x00000080;
        public static final int EVENT_ALL = 0x0000FFFF;

        // DcConstants.TYPE_XXX
        private final int mType;
        private Handler mTarget;
        private int mEvents = 0;
        private int mSlotId = 0;

        @VisibleForTesting
        protected Network mNetwork = null;
        @VisibleForTesting
        protected LinkProperties mCachedLinkProperties = null;
        @VisibleForTesting
        protected boolean mIsPendingOnAvailable = false;

        ImsNetworkCallback(int type, Handler target) {
            this(type, EVENT_ALL, target);
        }

        ImsNetworkCallback(int type, int events, Handler target) {
            mType = type;
            mEvents = events;
            mTarget = target;
        }

        /**
         * clear up this class
         */
        public void cleanUp() {
            mEvents = 0;
            if (mTarget != null) {
                mTarget.removeCallbacksAndMessages(null);
                mTarget = null;
            }
        }

        // DcConstants.TYPE_XXX
        public int getType() {
            return mType;
        }

        public Network getCachedNetwork() {
            return mNetwork;
        }

        public int getEvents() {
            return mEvents;
        }

        public void setEvents(int events) {
            mEvents = events;
        }

        public void setSlotId(int slotId) {
            mSlotId = slotId;
        }

        @Override
        public void onAvailable(Network network) {
            if (mNetwork != null && !mNetwork.equals(network)) {
                onLost(mNetwork);
            }

            cacheLinkProperties(network);

            mNetwork = network;

            if (!isEventSet(EVENT_AVAILABLE)) {
                // no-op
                return;
            }

            ImsLog.i(mSlotId, "network=" + network + ", type=" + mType);

            if (mCachedLinkProperties == null) {
                ImsLog.w(mSlotId, "no LinkProperties");
                mIsPendingOnAvailable = true;
                return;
            }

            if (mTarget != null) {
                Message.obtain(mTarget, EVENT_NETWORK_AVAILABLE).sendToTarget();
            }
        }

        @Override
        public void onLosing(Network network, int maxMsToLive) {
            if (!isEventSet(EVENT_LOSING)) {
                // no-op
                return;
            }

            ImsLog.i(mSlotId, "network=" + network + ", maxMsToLive=" + maxMsToLive);

            if (mTarget != null) {
                Message.obtain(mTarget, EVENT_NETWORK_LOSING, maxMsToLive, 0).sendToTarget();
            }
        }

        @Override
        public void onLost(Network network) {
            clearLinkProperties();

            mNetwork = null;

            if (!isEventSet(EVENT_LOST)) {
                // no-op
                return;
            }

            ImsLog.i(mSlotId, "ImsNetworkCallback :: onLost=" + network + ", Type=" + mType);

            if (mTarget != null) {
                Message.obtain(mTarget, EVENT_NETWORK_LOST).sendToTarget();
            }
        }

        @Override
        public void onUnavailable() {
            clearLinkProperties();

            if (!isEventSet(EVENT_UNAVAILABLE)) {
                // no-op
                return;
            }

            ImsLog.i(mSlotId, "network is unavailable");

            if (mTarget != null) {
                Message.obtain(mTarget, EVENT_NETWORK_UNAVAILABLE).sendToTarget();
            }
        }

        @Override
        public void onCapabilitiesChanged(Network network,
                NetworkCapabilities networkCapabilities) {
            if (mIsPendingOnAvailable) {
                ImsLog.w(mSlotId, "network is connected");
                mIsPendingOnAvailable = false;
                onAvailable(network);
            }

            if (!isEventSet(EVENT_CAPABILITIES_CHANGED)) {
                // no-op
                return;
            }

            if (ImsLog.DBG) {
                ImsLog.d(mSlotId, "network=" + network + ", capabilities=" + networkCapabilities);
            } else {
                ImsLog.i(mSlotId, "network=" + network);
            }

            if (mTarget != null) {
                Message.obtain(mTarget, EVENT_NETWORK_CAPABILITIES_CHANGED).sendToTarget();
            }
        }

        @Override
        public void onLinkPropertiesChanged(Network network, LinkProperties linkProperties) {
            boolean ipChanged = isIpChanged(linkProperties);
            boolean pcscfChanged = isPcscfChanged(linkProperties);

            if (ipChanged || pcscfChanged) {
                cacheLinkProperties(network);
            }

            if (!isEventSet(EVENT_NET_PCSCF_CHANGED)
                    && !isEventSet(EVENT_LOCAL_IP_CHANGED)) {
                // no-op
                return;
            }

            if (ImsLog.DBG) {
                ImsLog.d(mSlotId, "network=" + network + ", linkProperties=" + linkProperties
                        + ", ipChanged=" + ipChanged);
            } else {
                ImsLog.i(mSlotId, "netwokr=" + network + ", ipChanged=" + ipChanged);
            }

            if (mTarget == null) {
                return;
            }

            if (isEventSet(EVENT_LOCAL_IP_CHANGED) && ipChanged) {
                Message.obtain(mTarget, EVENT_IP_CHANGED).sendToTarget();
            }

            if (isEventSet(EVENT_NET_PCSCF_CHANGED) && pcscfChanged) {
                Message.obtain(mTarget, EVENT_PCSCF_CHANGED).sendToTarget();
            }
        }

        protected void cacheLinkProperties(Network network) {
            ConnectivityManagerProxy cmp = getConnectivityManagerProxy();
            mCachedLinkProperties = cmp.getLinkProperties(network);
        }

        protected void clearLinkProperties() {
            mCachedLinkProperties = null;
        }

        protected boolean isEventSet(int event) {
            return ((mEvents & event) != 0);
        }

        protected boolean isIpChanged(LinkProperties newLinkProperties) {
            if (mCachedLinkProperties == null || newLinkProperties == null) {
                return false;
            }

            if (isIdenticalAddresses(mCachedLinkProperties, newLinkProperties)) {
                return false;
            }

            String[] cachedAddress = null;

            if (mCachedLinkProperties != null) {
                Collection<LinkAddress> cachedlinkAddresses =
                        mCachedLinkProperties.getLinkAddresses();
                if (cachedlinkAddresses.isEmpty()) {
                    ImsLog.w(mSlotId, "cached LinkAddresses is empty, ");

                    Collection<LinkAddress> newAddr = newLinkProperties.getLinkAddresses();
                    if (!newAddr.isEmpty()) {
                        ImsLog.w(mSlotId, "new LinkAddresses is not empty");
                        return true;
                    }

                    return false;
                }

                cachedAddress = getIpAddress(cachedlinkAddresses);
            }

            Collection<LinkAddress> newlinkAddresses = newLinkProperties.getLinkAddresses();
            if (newlinkAddresses.isEmpty()) {
                ImsLog.w(mSlotId, "new LinkAddresses is empty, ");
                return false;
            }

            String[] newAddress = getIpAddress(newlinkAddresses);

            if ((cachedAddress == null) || (newAddress == null)) {
                return false;
            }

            printAddress("cached ip address", cachedAddress);
            printAddress("new ip address", newAddress);

            int nSize = 0;
            if (cachedAddress.length != newAddress.length) {
                return true;
            } else {
                nSize = newAddress.length;
            }

            for (int i = 0; i < nSize; i++) {
                boolean bIsSame = false;
                for (int ii = 0; ii < nSize; ii++) {
                    if (cachedAddress[i].equals(newAddress[ii])) {
                        bIsSame = true;
                    }
                }

                if (!bIsSame) {
                    return true;
                }
            }

            return false;
        }

        protected boolean isPcscfChanged(LinkProperties newLinkProperties) {
            if (getType() != DcConstants.TYPE_IMS) {
                return false;
            }

            if (mCachedLinkProperties == null || newLinkProperties == null) {
                return false;
            }


            Collection<InetAddress> cachedInetAddresses = mCachedLinkProperties.getPcscfServers();
            if (cachedInetAddresses.isEmpty()) {
                ImsLog.d(mSlotId, "cached Pcscf Server is empty");

                Collection<InetAddress> newAddr = newLinkProperties.getPcscfServers();
                if (!newAddr.isEmpty()) {
                    ImsLog.d(mSlotId, "new Pcscf Server is not empty");
                    return true;
                }

                return false;
            }

            Collection<InetAddress> newInetAddresses = newLinkProperties.getPcscfServers();
            if (newInetAddresses.isEmpty()) {
                ImsLog.d(mSlotId, "new Pcscf Server is empty");
                return false;
            }

            String[] cachedAddress = getPcscfAddress(cachedInetAddresses);
            String[] newAddress = getPcscfAddress(newInetAddresses);

            if (cachedAddress == null || newAddress == null) {
                return false;
            }

            printAddress("cached pcscf", cachedAddress);
            printAddress("new pcscf", newAddress);

            int nSize = 0;
            if (cachedAddress.length != newAddress.length) {
                return true;
            } else {
                nSize = newAddress.length;
            }

            for (int i = 0; i < nSize; i++) {
                if (!cachedAddress[i].equals(newAddress[i])) {
                    return true;
                }
            }

            return false;
        }

        private String[] getIpAddress(Collection<LinkAddress> linkAddresses) {
            if (linkAddresses == null) {
                return null;
            }

            Iterator<LinkAddress> iterator = linkAddresses.iterator();

            if (iterator == null) {
                return null;
            }

            String[] addr = new String[linkAddresses.size()];

            int i = 0;
            while (iterator.hasNext()) {
                LinkAddress linkAddress = iterator.next();

                if (linkAddress == null) {
                    ImsLog.w(mSlotId, "linkAddress is null");
                    continue;
                }

                InetAddress netAddress = linkAddress.getAddress();

                if (netAddress == null) {
                    ImsLog.w(mSlotId, "Resolving InetAddress failed - " + linkAddress.toString());
                    continue;
                }

                if (netAddress.isAnyLocalAddress()
                        || netAddress.isLinkLocalAddress()
                        || netAddress.isLoopbackAddress()) {
                    ImsLog.w(mSlotId, "Invalid InetAddress - " + linkAddress.toString());
                    continue;
                }

                if (netAddress instanceof Inet6Address) {

                    String ip6Addr = netAddress.getHostAddress();
                    ImsLog.i(mSlotId, "ip6Address - " + ip6Addr);

                    boolean bSame = false;
                    for (int j = 0; j < i; j++) {
                        if (addr[j].equals(ip6Addr)) {
                            bSame = true;
                        }
                    }

                    if (!bSame) {
                        addr[i] = ip6Addr;
                        ImsLog.i(mSlotId, "saved ip6Address - [" + i + "]" + addr[i]);
                        i++;
                    }
                }

                if (netAddress instanceof Inet4Address) {
                    String ip4Addr = netAddress.getHostAddress();
                    ImsLog.i(mSlotId, "ip4Address - " + ip4Addr);

                    boolean bSame = false;
                    for (int j = 0; j < i; j++) {
                        if (addr[j].equals(ip4Addr)) {
                            bSame = true;
                        }
                    }

                    if (!bSame) {
                        addr[i] = ip4Addr;
                        ImsLog.i(mSlotId, "saved ip4Address - [" + i + "]" + addr[i]);
                        i++;
                    }
                }
            }

            if (linkAddresses.size() != i) {
                String[] addrToDeliver = new String[i];
                for (int k = 0; k < i; k++) {
                    addrToDeliver[k] = addr[k];
                }

                return addrToDeliver;
            }

            return addr;
        }

        private String[] getPcscfAddress(Collection<InetAddress> inetAddresses) {
            if (inetAddresses == null) {
                return null;
            }

            String[] addr = new String[inetAddresses.size()];
            Iterator<InetAddress> iterator = inetAddresses.iterator();

            int i = 0;
            while (iterator.hasNext()) {
                InetAddress inetAddress = iterator.next();

                if (inetAddress == null) {
                    continue;
                }

                if (inetAddress.isAnyLocalAddress()
                        || inetAddress.isLinkLocalAddress()
                        || inetAddress.isLoopbackAddress()) {
                    continue;
                }

                if ((inetAddress instanceof Inet6Address)
                        || (inetAddress instanceof Inet4Address)) {
                    String ipAddress = inetAddress.getHostAddress();
                    boolean bSame = false;

                    for (int j = 0; j < i; j++) {
                        if (addr[j].equals(ipAddress)) {
                            bSame = true;
                        }
                    }

                    if (!bSame) {
                        addr[i] = ipAddress;
                        i++;
                    }
                }
            }

            if (inetAddresses.size() != i) {
                String[] addrToDeliver = new String[i];
                for (int k = 0; k < i; k++) {
                    addrToDeliver[k] = addr[k];
                }

                return addrToDeliver;
            }

            return addr;
        }

        private boolean isIdenticalAddresses(@NonNull LinkProperties left,
                @NonNull LinkProperties right) {
            final Collection<InetAddress> leftAddresses = left.getAddresses();
            final Collection<InetAddress> rightAddresses = right.getAddresses();
            return leftAddresses.size() == rightAddresses.size()
                    && leftAddresses.containsAll(rightAddresses);
        }

        private void printAddress(String prifix, String[] addresses) {
            StringBuilder sb = new StringBuilder();
            sb.append(prifix).append(" : ");

            for (String address : addresses) {
                sb.append(address).append(" / ");
            }

            ImsLog.d(mSlotId, sb.toString());
        }
    }

    /**
     * This is common handler of each APN type to handle EVENT_NOTIFY_DATA_STATE_CHANGED event
     * It notify the change of data connection state to IMS Native
     */
    protected class HandleDataStateChanged implements MsgProcInterface {

        @Override
        public void procMsg(Message msg) {
            if (mSystem == null) {
                return;
            }

            int apnType = msg.arg1;
            int dataState = msg.arg2;
            if (dataState == EDataState.DATA_STATE_CONNECT_FAILED.getState()) {
                ImsLog.w(mSlotId, "Data Connection failed : apnType=" + apnType);
                mSystem.notifyDataConnectionFailed(apnType);
                return;
            }

            ImsLog.i(mSlotId, "apnType=" + apnType + ", dataState=" + dataState);
            mSystem.notifyDataConnectionStateChanged(apnType, dataState);
        }
    }

    protected class HandlePreciseDataConnectionStateChanged implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            if (msg.obj == null) {
                ImsLog.w(mSlotId, "msg.obj is null");
                return;
            }
            PreciseDataConnectionState dataConnectionState = (PreciseDataConnectionState) msg.obj;
            ApnSetting apnSetting = dataConnectionState.getApnSetting();
            int dataState = dataConnectionState.getState();
            int networkType = dataConnectionState.getNetworkType();
            int causeCode = dataConnectionState.getLastCauseCode();

            switch (dataState) {
                case TelephonyManager.DATA_CONNECTED:
                    if (networkType == TelephonyManager.NETWORK_TYPE_UNKNOWN) {
                        ImsLog.w(mSlotId, "Unknown network type");
                        return;
                    }

                    if (mPreciseDcState == TelephonyManager.DATA_HANDOVER_IN_PROGRESS) {
                        if (isIpcanChanged(networkType)) {
                            // Notify handover is successful
                            handleHandoverSuccess(networkType);
                        } else {
                            // Notify handover failure
                            handleHandoverFailure(networkType, causeCode);
                        }
                    } else {
                        // update APN string
                        String strApn = apnSetting.getApnName();
                        if (strApn != null && !strApn.equals("(null)")) {
                            mApnString = strApn;
                        }
                        // update APN protocol
                        if (mDcNetWatcher != null) {
                            mApnProtocol = (mDcNetWatcher.isDataNetworkRoaming())
                                    ? apnSetting.getRoamingProtocol() : apnSetting.getProtocol();
                        }
                    }
                    if (mNetworkType != networkType) {
                        updateCrossSimStatus(networkType);
                        if (mNetworkType == TelephonyManager.NETWORK_TYPE_UNKNOWN
                                || isIpcanChanged(networkType)) {
                            handleIpcanCategory(networkType);
                        }

                        // update network type
                        ImsLog.i(mSlotId, "network type :: " + mNetworkType + " >> " + networkType);
                        mNetworkType = networkType;
                    }
                    break;
                case TelephonyManager.DATA_HANDOVER_IN_PROGRESS:
                    handleHandoverStart(networkType);
                    break;
                case TelephonyManager.DATA_DISCONNECTED:
                    if (mPreciseDcState == TelephonyManager.DATA_CONNECTING
                                && causeCode != DataFailCause.NONE) {
                        // initial connection failure
                        handleInitialConnectionFailure(causeCode);
                    }
                    mNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
                    updateCrossSimStatus(mNetworkType);
                    break;
                default:
                    // no-op
                    break;
            }

            // update PreciseDataConnectionState
            if (mPreciseDcState != dataState) {
                mPreciseDcState = dataState;
                ImsLog.i(mSlotId, "notifyConnectionStateChanged : apnType=" + mType.getString()
                        + ", dataState=" + dataState);
                notifyConnectionStateChanged(dataState, causeCode, networkType);
            }
        }

        private void handleHandoverStart(int networkType) {
            ImsLog.i(mSlotId, "handleHandoverStart");
            notifyHandoverStateChanged(HANDOVER_START, networkType, DataFailCause.NONE);
        }

        private void handleHandoverSuccess(int networkType) {
            ImsLog.i(mSlotId, "handleHandoverSuccess");
            notifyHandoverStateChanged(HANDOVER_SUCCESS, networkType, DataFailCause.NONE);
        }

        private void handleHandoverFailure(int networkType, int causeCode) {
            ImsLog.i(mSlotId, "handleHandoverFailure");
            notifyHandoverStateChanged(HANDOVER_FAILURE, networkType, causeCode);
        }

        private void handleInitialConnectionFailure(int causeCode) {
            ImsLog.i(mSlotId, "handleInitialConnectionFailed");

            Message msg = Message.obtain();
            msg.what = EVENT_DATA_CONNECTION_FAILED;
            msg.obj = causeCode;
            sendMessage(msg);
        }

        private boolean isIpcanChanged(int networkToCheck) {
            int currentType = (mNetworkType == TelephonyManager.NETWORK_TYPE_IWLAN)
                    ? IPCAN_CATEGORY_WLAN : IPCAN_CATEGORY_MOBILE;
            int checkType = (networkToCheck == TelephonyManager.NETWORK_TYPE_IWLAN)
                    ? IPCAN_CATEGORY_WLAN : IPCAN_CATEGORY_MOBILE;

            return (currentType != checkType);
        }
    }
}
