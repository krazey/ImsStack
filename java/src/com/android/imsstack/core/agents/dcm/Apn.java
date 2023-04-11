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

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ISubscription;
import com.android.imsstack.core.agents.MsgProcInterface;
import com.android.imsstack.core.agents.SubscriptionListener;
import com.android.imsstack.core.agents.dcmif.ApnStateListener;
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
import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.enabler.aos.IAosRegistration;
import com.android.imsstack.enabler.aos.IAosRegistrationListener;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;
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
    protected ISubscription mSubscription;
    protected IAosRegistration mAosReg;
    protected int mSlotId = 0;
    protected EApnType mType;
    protected EApnReqState mAPNState = EApnReqState.APN_REQUEST_IDLE;
    protected int mDataState = TelephonyManager.DATA_DISCONNECTED;
    protected String mApnString = null;
    protected int mApnProtocol = ApnSetting.PROTOCOL_IPV4V6;
    protected int mNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
    protected int mPreciseDcState = TelephonyManager.DATA_UNKNOWN;
    protected int mIpcanCategory = IPCAN_CATEGORY_MOBILE;
    protected LinkedHashMap<Integer, MsgProcInterface> mMapMsgHandler =
            new LinkedHashMap<Integer, MsgProcInterface>();
    protected ImsNetworkCallback mNetworkCallback = null;
    protected ImsNetworkCallback mNetworkMonitoringCallback = null;
    protected boolean mIsMonitoringCallbackRegistered = false;
    protected int mSubId = MSimUtils.INVALID_SUB_ID;
    protected ApnSubscriptionListener mSubscriptionListener = null;
    protected Set<ApnStateListener> mApnStateListeners =
            new CopyOnWriteArraySet<ApnStateListener>();

    protected Apn(Context context, int slotId) {
        super(Looper.myLooper());

        mContext = context;
        mSlotId = slotId;
        mDcApn = (IDcApn) DcFactory.getDc(DcFactory.APN, mSlotId);
        mDcSettings = (IDcSettings) DcFactory.getDc(DcFactory.SETTING, mSlotId);
        mDcNetWatcher = (IDcNetWatcher) DcFactory.getDc(DcFactory.NETWORK_WATCHER, mSlotId);
        mSystem = SystemInterface.getInstance().getSystem(mSlotId);
        mSubscription = (ISubscription) AgentFactory.getAgent(AgentFactory.SUBSCRIPTION);
        mAosReg = AosFactory.getInstance().getAosRegistration(mSlotId);

        registerEvent();
    }

    // Interface implementation methods --------------------------
    @Override
    public void cleanup() {
        ImsLog.d(mSlotId, "clean up");

        unregisterSubscription();

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
        msg.obj = new IDcNetWatcher.NotiObj(mType, EDataState.DATA_STATE_DISCONNECTED, -1);
        handleMessage(msg);

        removeCallbacksAndMessages(null);
    }

    @Override
    public void handleMessage(Message msg) {
        // pumping to hash table.
        // Apn class provide simple event pumping table through "registerHandler" api
        // for derived classes
        MsgProcInterface msgProcess = mMapMsgHandler.get(msg.what);

        if (msgProcess == null) {
            ImsLog.w(mSlotId, "Apn :: no proc - apn=" + mType + ", msg=" + msg.what
                    + ", proc=" + sEventToString.get(msg.what));
            return;
        }

        ImsLog.i(mSlotId, "Apn :: apn=" + mType + ", msg=" + msg.what
                + ", proc=" + sEventToString.get(msg.what));

        msgProcess.procMsg(msg);
    }

    @Override
    public Context getContext() {
        return mContext;
    }

    @Override
    public void addListener(ApnStateListener listener) {
        mApnStateListeners.add(listener);
    }

    @Override
    public void removeListener(ApnStateListener listener) {
        mApnStateListeners.remove(listener);
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
        int ipVersion = EIpVersion.IPV6V4.getInt();
        if (mDcSettings != null) {
            if (mType.getType() == DcConstants.TYPE_EMERGENCY) {
                if (mDcSettings.getEmergencyPreferredIpVersion()
                        == CarrierConfig.Assets.IPV4_PREFERRED) {
                    ipVersion = EIpVersion.IPV4V6.getInt();
                }
            } else if (mType.getType() == DcConstants.TYPE_IMS) {
                if (mDcSettings.getPreferredIpVersion() == CarrierConfig.Assets.IPV4_PREFERRED) {
                    ipVersion = EIpVersion.IPV4V6.getInt();
                }
            }
        }
        return ipVersion;
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

    // Private/Protected methods ---------------------------------
    //---------------------------------------------------------------------
    protected void registerCallback(int events) {
        ImsLog.i(mSlotId, "type = " + mType.getString());

        ConnectivityManager cm = (mContext == null) ? null :
                mContext.getSystemService(ConnectivityManager.class);

        if ((cm == null) || (events == 0)) {
            return;
        }

        NetworkRequest.Builder nrb = new NetworkRequest.Builder();
        NetworkRequest nr = null;

        nrb.addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);

        if (MSimUtils.isMultiSimEnabled()) {
            mSubId = MSimUtils.getSubId(mSlotId);
            registerSubscription();

            nrb.setNetworkSpecifier(new TelephonyNetworkSpecifier.Builder()
                    .setSubscriptionId(MSimUtils.getSubId(mSlotId)).build());
        }

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
            mNetworkMonitoringCallback = new ImsNetworkCallback(
                    mContext, mType.getType(), events, this);
            mNetworkMonitoringCallback.setSlotId(mSlotId);
        } else {
            mNetworkMonitoringCallback.setEvents(events);
        }

        if (nr != null) {
            cm.registerNetworkCallback(nr, mNetworkMonitoringCallback);
            mIsMonitoringCallbackRegistered = true;
        }
    }

    protected void unregisterCallback() {
        ImsLog.i(mSlotId, "type = " + mType.getString());

        ConnectivityManager cm = (mContext == null) ? null :
                mContext.getSystemService(ConnectivityManager.class);

        if (cm == null) {
            return;
        }

        if (mNetworkMonitoringCallback != null) {
            mIsMonitoringCallbackRegistered = false;

            try {
                cm.unregisterNetworkCallback(mNetworkMonitoringCallback);
            } catch (IllegalArgumentException e) {
                ImsLog.e("" + e);
            }
        }
    }

    protected void requestNetwork() {
        ImsLog.i(mSlotId, "type = " + mType.getString());
        ConnectivityManager cm = (mContext == null) ? null :
                mContext.getSystemService(ConnectivityManager.class);

        if (cm == null) {
            return;
        }

        NetworkRequest.Builder nrb = new NetworkRequest.Builder();
        NetworkRequest nr = null;

        nrb.addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);

        if (MSimUtils.isMultiSimEnabled()) {
            boolean setSubId = true;

            if (mType.getType() == DcConstants.TYPE_EMERGENCY) {
                int subId = MSimUtils.getSubId(mSlotId);

                if ((mSubscription != null) && mSubscription.isAllSimAbsentOrLocked()
                        && !MSimUtils.isValidSubId(subId)) {
                    setSubId = false;
                }
            }

            if (setSubId) {
                nrb.setNetworkSpecifier(new TelephonyNetworkSpecifier.Builder()
                        .setSubscriptionId(MSimUtils.getSubId(mSlotId)).build());
            }
        }

        if (mType.getType() == DcConstants.TYPE_IMS) {
            if (mDcSettings != null) {
                if (mDcSettings.isVopsRequiredForPdn()) {
                    nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_MMTEL);
                }
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
            mNetworkCallback = new ImsNetworkCallback(mContext, mType.getType(), this);
            mNetworkCallback.setSlotId(mSlotId);
        }

        if (nr != null) {
            cm.requestNetwork(nr, mNetworkCallback);
        }
    }

    protected void releaseNetwork() {
        ImsLog.d(mSlotId, "type = " + mType.getString());

        ConnectivityManager cm = (mContext == null) ? null :
                mContext.getSystemService(ConnectivityManager.class);

        if (cm == null) {
            return;
        }

        if (mNetworkCallback != null) {
            try {
                cm.unregisterNetworkCallback(mNetworkCallback);
            } catch (IllegalArgumentException e) {
                ImsLog.e("" + e);
            }
            mNetworkCallback.cleanUp();
            mNetworkCallback = null;
        }
    }

    protected void registerHandler(int evt, MsgProcInterface proc) {
        mMapMsgHandler.put(evt, proc);
    }

    protected void registerEvent() {
        registerHandler(EVENT_NOTIFY_DATA_STATE_CHANGED, new HandleDataStateChanged());
        registerHandler(EVENT_PRECISE_DATA_CONNECTION_STATE_CHANGED,
                new HandlePreciseDataConnectionStateChanged());
    }

    protected EApnReqState getApnReqState() {
        return mAPNState;
    }

    protected void setApnReqState(EApnReqState s) {
        mAPNState = s;
    }

    protected void registerSubscription() {
        if (mSubscriptionListener == null) {
            mSubscriptionListener = new ApnSubscriptionListener();
            if (mSubscription != null) {
                mSubscription.addListener(mSubscriptionListener);
            }
        }
    }

    protected void unregisterSubscription() {
        if (mSubscriptionListener != null) {
            if (mSubscription != null) {
                mSubscription.removeListener(mSubscriptionListener);
            }
            mSubscriptionListener = null;
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
            mDcNetWatcher.notifyResult(apnType, dataState);
        }

        //notify to apn
        Message msg = Message.obtain();
        msg.what = EVENT_NOTIFY_DATA_STATE_CHANGED;
        msg.obj = new IDcNetWatcher.NotiObj(apnType, dataState, -1);

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

    protected void handleCarrierConfigChanged(int phoneId, int subId) {
        if (mSlotId != phoneId || subId == mSubId) {
            return;
        }

        ImsLog.d("onCarrierConfigChanged :: subId=" + mSubId + "->" + subId
                + ", phoneId=" + phoneId);

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
     * Notifies the application that IPCAN category is changed.
     */
    protected void notifyIpcanCategoryChanged(int ipcanCategory) {
        ImsLog.i(mSlotId, "notifyIpcanCategoryChanged");
        for (ApnStateListener l : mApnStateListeners) {
            l.onIpcanCategoryChanged(mType.getType(), ipcanCategory);
        }
    }

    /**
     * Notifies the application that data handover information is changed.
     */
    protected void notifyHandoverInfoChanged(int handoverState, int networkType, int failCause) {
        ImsLog.i(mSlotId, "notifyHandoverInfoChanged");
        for (ApnStateListener l : mApnStateListeners) {
            l.onHandoverInfoChanged(handoverState, networkType, failCause);
        }
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

        private final Context mContext;
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

        ImsNetworkCallback(Context context, int type, Handler target) {
            mContext = context;
            mType = type;
            mEvents = EVENT_ALL;
            mTarget = target;
        }

        ImsNetworkCallback(Context context, int type, int events, Handler target) {
            mContext = context;
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
            ConnectivityManager cm = (mContext == null) ? null :
                    mContext.getSystemService(ConnectivityManager.class);

            if (cm != null) {
                mCachedLinkProperties = cm.getLinkProperties(network);
            }
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
            // Ignore DataState Change During ShutDown
            if (mDcNetWatcher != null && mDcNetWatcher.isDoingOffRadio()) {
                ImsLog.w(mSlotId, "radio is doing off");
                return;
            }

            IDcNetWatcher.NotiObj res = (IDcNetWatcher.NotiObj) msg.obj;
            if (res == null) {
                return;
            }

            // Do not use dataState vi intent
            EApnType apnType = res.eApnType;
            EDataState dataState = res.eDataState;

            if (mSystem == null) {
                return;
            }

            if (dataState == EDataState.DATA_STATE_CONNECT_FAILED) {
                ImsLog.w(mSlotId, "Data Connection failed : apnType=" + apnType);
                mSystem.notifyDataConnectionFailed(apnType.getType());
                return;
            }

            ImsLog.i(mSlotId, "apnType=" + apnType
                    + " : " + apnType.getString() + ", dataState=" + dataState);

            mSystem.notifyDataConnectionStateChanged(
                    apnType.getType(), dataState.getState());
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
                        if ((mDcNetWatcher != null) && mDcNetWatcher.isRoaming()) {
                            mApnProtocol = apnSetting.getRoamingProtocol();
                        } else {
                            mApnProtocol = apnSetting.getProtocol();
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
                case TelephonyManager.DATA_CONNECTING:
                    if (mType.getType() == DcConstants.TYPE_IMS
                            && mPreciseDcState != TelephonyManager.DATA_CONNECTING) {
                        if (mDcSettings != null && mDcSettings.isCdmalessFeatureTagRequired()) {
                            if (mAosReg != null) {
                                mAosReg.controlRegistration(
                                        IAosRegistration.RequestType.START_IMS_EST_TIMER,
                                        IAosRegistration.Pcscf.CURRENT,
                                        IAosRegistration.Cause.DATA_CONNECTING);
                            }
                        }
                    }
                    break;
                case TelephonyManager.DATA_HANDOVER_IN_PROGRESS:
                    handleHandoverStart(networkType);
                    break;
                case TelephonyManager.DATA_DISCONNECTING:
                    if (mType.getType() == DcConstants.TYPE_IMS) {
                        if (mAosReg != null) {
                            if (mAosReg.getRegisteredNetworkType()
                                    != IAosRegistrationListener.NetworkType.NONE) {
                                mAosReg.controlRegistration(IAosRegistration.RequestType.STOP,
                                        IAosRegistration.Pcscf.CURRENT,
                                        IAosRegistration.Cause.DATA);
                            }
                        }
                    }
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
            mPreciseDcState = dataState;
        }

        private void handleHandoverStart(int networkType) {
            ImsLog.i(mSlotId, "handleHandoverStart");
            notifyHandoverInfoChanged(HANDOVER_START, networkType, DataFailCause.NONE);
        }

        private void handleHandoverSuccess(int networkType) {
            ImsLog.i(mSlotId, "handleHandoverSuccess");
            notifyHandoverInfoChanged(HANDOVER_SUCCESS, networkType, DataFailCause.NONE);
        }

        private void handleHandoverFailure(int networkType, int causeCode) {
            ImsLog.i(mSlotId, "handleHandoverFailure");
            notifyHandoverInfoChanged(HANDOVER_FAILURE, networkType, causeCode);
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

    /* ---------------------------------------------------------------------------------------------
        Listener class - SubscriptionListener
    --------------------------------------------------------------------------------------------- */
    @VisibleForTesting
    protected final class ApnSubscriptionListener extends SubscriptionListener {
        ApnSubscriptionListener() {
            ImsLog.d("ApnSubscriptionListener");
        }

        @Override
        public void onCarrierConfigChanged(int phoneId, int subId) {
            handleCarrierConfigChanged(phoneId, subId);
        }
    }
}
