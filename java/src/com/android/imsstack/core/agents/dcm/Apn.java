package com.android.imsstack.core.agents.dcm;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.LinkAddress;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.NetworkRequest;
import android.net.TelephonyNetworkSpecifier;
import android.os.Handler;
import android.os.Message;
import android.telephony.AccessNetworkConstants;
import android.telephony.DataFailCause;
import android.telephony.PreciseDataConnectionState;
import android.telephony.TelephonyManager;
import android.telephony.data.ApnSetting;
import android.text.TextUtils;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.agentif.ISubscription;
import com.android.imsstack.core.agents.agentif.MsgProcInterface;
import com.android.imsstack.core.agents.agentif.SubscriptionListener;
import com.android.imsstack.core.agents.dcm.DCFactory;
import com.android.imsstack.core.agents.dcmif.ApnStateListener;
import com.android.imsstack.core.agents.dcmif.DCConstants;
import com.android.imsstack.core.agents.dcmif.EApnReqState;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.EIpVersion;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDCApn;
import com.android.imsstack.core.agents.dcmif.IDCNetWatcher;
import com.android.imsstack.core.agents.dcmif.IDCSettings;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.core.config.FeatureConfig;
import com.android.imsstack.system.ImsEventDef;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;
import com.android.net.module.util.LinkPropertiesUtils;

import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.util.Collection;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

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
    protected static final int EVENT_WAITING_LOCAL_ADDRESS_IPV6 = 1003;
    protected static final int EVENT_NOTIFY_DATA_STATE_CHANGED = 1004;
    protected static final int EVENT_PRECISE_DATA_CONNECTION_STATE_CHANGED = 1005;
    protected static final int EVENT_DATA_CONNECTION_FAILED = 1006;

    protected static final int EVENT_AIRPLANE_MODE_CHANGED = 2001;
    protected static final int EVENT_WIFI_STATE_CHANGED = 2002;
    protected static final int EVENT_VOWIFI_SETTING_CHANGED = 2003;
    protected static final int EVENT_VOWIFI_PREF_CHANGED = 2004;
    protected static final int EVENT_VOWIFI_ROAMING_PREF_CHANGED = 2005;

    protected static final int FEATURE_NONE = 0;
    protected static final int FEATURE_IPV6_DELAY = 0x00000001;

    protected static final int PROTOCOL_IP = ApnSetting.PROTOCOL_IP;
    protected static final int PROTOCOL_IPV6 = ApnSetting.PROTOCOL_IPV6;
    protected static final int PROTOCOL_IPV4V6 = ApnSetting.PROTOCOL_IPV4V6;

    // Variables--------------------------------------------------
    protected static final Hashtable<Integer, String> sEventToString;

    protected Context        mContext;
    protected IDCSettings    mDcSettings;
    protected IDCNetWatcher  mDcNetWatcher;
    protected int            nSlotId = 0;

    protected EApnType       eType;
    protected EApnReqState   mAPNState = EApnReqState.APN_REQUEST_IDLE;
    protected int            mDataState = TelephonyManager.DATA_DISCONNECTED;
    protected String         mApnString = null;
    protected int            mApnProtocol = PROTOCOL_IPV4V6;
    protected int            mNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
    protected int            mPreciseDcState = TelephonyManager.DATA_UNKNOWN;

    protected int            mIpcanCategory = IPCAN_CATEGORY_MOBILE;
    protected Hashtable<Integer, MsgProcInterface> mapMsgHandler
            = new Hashtable<Integer, MsgProcInterface>();
    protected ImsNetworkCallback mNetworkCallback = null;
    protected ImsNetworkCallback mNetworkMonitoringCallback = null;

    protected int mApnEmployCount = 0;

    protected boolean mESMCausePermanentFailure = false;

    protected boolean mIsMonitoringCallbackRegistered = false;

    private int mSubId = MSimUtils.INVALID_SUB_ID;
    private ApnSubscriptionListener mSubscriptionListener = null;
    private Set<ApnStateListener> mApnStateListeners
            = new CopyOnWriteArraySet<ApnStateListener>();

    // Interface implementation methods --------------------------
    @Override
    public void cleanup() {
        ImsLog.d(nSlotId, "clean up");

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
        msg.obj = new IDCNetWatcher.NotiObj(eType, EDataState.DATA_STATE_DISCONNECTED);
        handleMessage(msg);
    }

    @Override
    public void handleMessage(Message msg) {
        // pumping to hash table.
        // Apn class provide simple event pumping table through "registerHandler" api
        // for derived classes
        MsgProcInterface msgProcess = mapMsgHandler.get(msg.what);

        if (msgProcess == null) {
            ImsLog.w(nSlotId, "Apn :: no proc - apn=" + eType + ", msg=" + msg.what
                    + ", proc=" + sEventToString.get(msg.what));
            return;
        }

        ImsLog.i(nSlotId, "Apn :: apn=" + eType + ", msg=" + msg.what
                + ", proc=" + sEventToString.get(msg.what));

        msgProcess.procMsg(msg);
    }

    @Override
    public void setSettings(IDCSettings dcSettings) {
        this.mDcSettings = dcSettings;
    }

    @Override
    public IDCSettings getSettings() {
        return mDcSettings;
    }

    @Override
    public void setNetWatcher(IDCNetWatcher dcNetWatcher) {
        this.mDcNetWatcher = dcNetWatcher;
    }

    @Override
    public IDCNetWatcher getNetWatcher() {
        return mDcNetWatcher;
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
    public abstract boolean connect();

    @Override
    public boolean connect(int ipcanType) {
        return connect();
    }

    @Override
    public abstract void disconnect(int nTimeAfterRecover);

    @Override
    public void disconnect(int ipcanType, int timeAfterRecover) {
        disconnect(timeAfterRecover);
    }

    @Override
    public String getApn() {
        if (mApnString != null) {
            return mApnString;
        }
        return eType.getString();
    }

    @Override
    public boolean isConnected() {
        if (mNetworkCallback != null) {
            return mNetworkCallback.isNetworkConnected();
        }

        if (mNetworkMonitoringCallback != null) {
            ImsLog.i(nSlotId, "mNetworkMonitoringCallback not null");
            return mNetworkMonitoringCallback.isNetworkConnected();
        }

        return false;
    }

    @Override
    public int getDataState() {
        return this.mDataState;
    }

    @Override
    public int getIpcanCategory() {
        return mIpcanCategory;
    }

    @Override
    public int getIpVersion() {
        if (mApnProtocol == PROTOCOL_IP) {
            return EIpVersion.IPV4.getInt();
        } else if (mApnProtocol == PROTOCOL_IPV6) {
            return EIpVersion.IPV6.getInt();
        } else {
            int ipVersion = EIpVersion.IPV6V4.getInt();
            if (mDcSettings != null) {
                if (eType.getType() == DCConstants.TYPE_EMERGENCY) {
                    if (mDcSettings.getEmergencyPreferredIpVersion() ==
                            CarrierConfig.Ims.IPV4_PREFERRED) {
                        ipVersion = EIpVersion.IPV4V6.getInt();
                    }
                } else if (eType.getType() == DCConstants.TYPE_IMS) {
                    if (mDcSettings.getPreferredIpVersion() == CarrierConfig.Ims.IPV4_PREFERRED) {
                        ipVersion = EIpVersion.IPV4V6.getInt();
                    }
                }
            }
            return ipVersion;
        }
    }

    @Override
    public int getSlotId() {
        return this.nSlotId;
    }

    @Override
    public void employApn() {
        mApnEmployCount++;
    }

    @Override
    public void dismissApn() {
        if (mApnEmployCount > 0) {
            mApnEmployCount--;
        }
    }

    @Override
    public int getApnEmployCount() {
        return mApnEmployCount;
    }

    @Override
    public boolean isESMCausePermanentFailure() {
        return mESMCausePermanentFailure;
    }

    @Override
    public Network getCachedNetwork() {
        if (mNetworkCallback != null) {
            return mNetworkCallback.getCachedNetwork();
        } else if (mNetworkMonitoringCallback != null) {
            mNetworkMonitoringCallback.getCachedNetwork();
        }
        return null;
    }

    @Override
    public void notifyIPCanChange(int ipcanType) {
        // Child Class Need to Implement
    }

    @Override
    public void setManualDetachedTriggered(boolean value) {
        // Child Class Need to Implement
    }

    @Override
    public int getWlanPreference() {
        // Child Class Need to Implement
        return -1;
    }

    @Override
    public String toString() {
        return ", ApnType= " + eType
                + ", DataState= " + mDataState
                + ", APNState= " + mAPNState;
    }

    // Private/Protected methods ---------------------------------
    //---------------------------------------------------------------------
    protected void registerCallback(int events) {
        ImsLog.i(nSlotId, "type = " + eType.getString());

        ConnectivityManager cm = (mContext == null) ? null :
                mContext.getSystemService(ConnectivityManager.class);

        if ((cm == null) || (events == 0)) {
            return;
        }

        NetworkRequest.Builder nrb = new NetworkRequest.Builder();
        NetworkRequest nr = null;

        nrb.addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR);

        if (MSimUtils.isMultiSimEnabled()) {
            mSubId = MSimUtils.getSubId(nSlotId);
            registerSubscription();

            nrb.setNetworkSpecifier(new TelephonyNetworkSpecifier.Builder()
                    .setSubscriptionId(MSimUtils.getSubId(nSlotId)).build());
        }

        if (eType.getType() == DCConstants.TYPE_IMS) {
            nr = nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_IMS).build();
        } else if (eType.getType() == DCConstants.TYPE_INTERNET) {
            nr = nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET).build();
        } else if (eType.getType() == DCConstants.TYPE_XCAP) {
            nr = nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_XCAP).build();
        } else if (eType.getType() == DCConstants.TYPE_EMERGENCY) {
            nr = nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_EIMS).build();
        }

        if (mNetworkMonitoringCallback == null) {
            mNetworkMonitoringCallback = new ImsNetworkCallback(
                    mContext, eType.getType(), events, this);
            mNetworkMonitoringCallback.setSlotId(nSlotId);
        } else {
            mNetworkMonitoringCallback.setEvents(events);
        }

        if (nr != null) {
            cm.registerNetworkCallback(nr, mNetworkMonitoringCallback);
            mIsMonitoringCallbackRegistered = true;
        }
    }

    protected void unregisterCallback() {
        ImsLog.i(nSlotId, "type = " + eType.getString());

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
        ImsLog.i(nSlotId, "type = " + eType.getString());
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

            if (eType.getType() == DCConstants.TYPE_EMERGENCY) {
                ISubscription isub = (ISubscription)AgentFactory.getAgent(
                        AgentFactory.SUBSCRIPTION);
                int subId = MSimUtils.getSubId(nSlotId);

                if ((isub != null) && isub.isAllSimAbsentOrLocked()
                        && !MSimUtils.isValidSubId(subId)) {
                    setSubId = false;
                }
            }

            if (setSubId) {
                nrb.setNetworkSpecifier(new TelephonyNetworkSpecifier.Builder()
                        .setSubscriptionId(MSimUtils.getSubId(nSlotId)).build());
            }
        }

        if (eType.getType() == DCConstants.TYPE_IMS) {
            nr = nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_IMS).build();
        } else if (eType.getType() == DCConstants.TYPE_INTERNET) {
            nr = nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET).build();
        } else if (eType.getType() == DCConstants.TYPE_XCAP) {
            nr = nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_XCAP).build();
        } else if (eType.getType() == DCConstants.TYPE_EMERGENCY) {
            nr = nrb.addCapability(NetworkCapabilities.NET_CAPABILITY_EIMS).build();
        }

        if (mNetworkCallback == null) {
            mNetworkCallback = new ImsNetworkCallback(mContext, eType.getType(), this);
            mNetworkCallback.setSlotId(nSlotId);
        }

        if (nr != null) {
            cm.requestNetwork(nr, mNetworkCallback);
        }
    }

    protected void releaseNetwork() {
        ImsLog.d(nSlotId, "type = " + eType.getString());

        mESMCausePermanentFailure = false;

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
        mapMsgHandler.put(evt, proc);
    }

    protected void registerEvent() {
        registerHandler(EVENT_NOTIFY_DATA_STATE_CHANGED,
                new Handle_EVENT_NOTIFY_DATA_STATE_CHANGED());
        registerHandler(EVENT_PRECISE_DATA_CONNECTION_STATE_CHANGED,
                new Handle_EVENT_PRECISE_DATA_CONNECTION_STATE_CHANGED());
    }

    protected void setAPNReqState(EApnReqState s) {
        mAPNState = s;
    }

    protected EApnReqState getAPNReqState() {
        return mAPNState;
    }

    protected void setDataState(int newState) {
        if (mDataState != newState) {
            mDataState = newState;
        }
    }

    protected void registerSubscription() {
        if (mSubscriptionListener == null) {
            mSubscriptionListener = new ApnSubscriptionListener();
            ISubscription subs = (ISubscription)AgentFactory.getAgent(AgentFactory.SUBSCRIPTION);
            if (subs != null) {
                subs.addListener(mSubscriptionListener);
            }
        }
    }

    protected void unregisterSubscription() {
        if (mSubscriptionListener != null) {
            ISubscription subs = (ISubscription)AgentFactory.getAgent(AgentFactory.SUBSCRIPTION);
            if (subs != null) {
                subs.removeListener(mSubscriptionListener);
            }
            mSubscriptionListener = null;
        }
    }

    protected Apn(Context context, int slotId) {
        mContext = context;
        nSlotId = slotId;
        mDcSettings = (IDCSettings)DCFactory.getDC(DCFactory.SETTING, nSlotId);
        mDcNetWatcher = (IDCNetWatcher)DCFactory.getDC(DCFactory.NETWORK_WATCHER, nSlotId);

        registerEvent();
    }

    protected boolean isVoWifiSupported() {
        return FeatureConfig.isEnabled(getSlotId(), FeatureConfig.VOWIFI);
    }

    protected int getIpcanCategory(int networkType) {
        return (networkType == TelephonyManager.NETWORK_TYPE_IWLAN) ?
                IPCAN_CATEGORY_WLAN : IPCAN_CATEGORY_MOBILE;
    }

    protected boolean notifyDataConnectionIpcanChanged(int networkType) {
        int category = getIpcanCategory(networkType);

        if (mIpcanCategory == category) {
            return false;
        }

        ImsLog.i(nSlotId, "type = " + eType.getString() + " , old cat = "
                + mIpcanCategory + " , curr cat = " + category);

        mIpcanCategory = category;

        ISystem system = SystemInterface.getInstance().getSystem(nSlotId);

        if (system != null) {
            system.notifyDataConnectionIpcanChanged(eType.getType(), mIpcanCategory);
        }

        return true;
    }

    protected void notifyPdnConnectionFailed(EApnType apnType) {
        ImsLog.i(nSlotId, "apnType : " + apnType);

        //notify to watcher
        if (mDcNetWatcher != null) {
            mDcNetWatcher.notifyPdnConnectionFailed(apnType);
        }
    }

    protected int getDataStateFromCM() {
        ConnectivityManager cm = (mContext == null) ? null :
                mContext.getSystemService(ConnectivityManager.class);

        if (cm == null) {
            return TelephonyManager.DATA_DISCONNECTED;
        }

        NetworkInfo netInfo = null;
        IDCApn dcapn = (IDCApn)DCFactory.getDC(DCFactory.APN, nSlotId);

        if (dcapn != null) {
            netInfo = cm.getNetworkInfo(dcapn.getNetworkByCapability(eType.getType()));
        }

        if (netInfo == null) {
            ImsLog.w(nSlotId, "NetworkInfo is null");
            return TelephonyManager.DATA_DISCONNECTED;
        }

        NetworkInfo.State niState = netInfo.getState();

        int dataState = TelephonyManager.DATA_DISCONNECTED;

        if ((niState == NetworkInfo.State.CONNECTED)
                || (niState == NetworkInfo.State.SUSPENDED)) {
            dataState = TelephonyManager.DATA_CONNECTED;
        }

        return dataState;
    }

    protected boolean hasLocalAddress(int version) {
        IDCApn dcapn = (IDCApn)DCFactory.getDC(DCFactory.APN, nSlotId);

        if (dcapn != null) {
            String ip = dcapn.getLocalAddress(eType.getType(), version);

            if (ip != null) {
                return true;
            }
        }

        return false;
    }

    protected boolean isIPChanged() {
        IDCApn dcapn = (IDCApn)DCFactory.getDC(DCFactory.APN, nSlotId);

        if (dcapn != null) {
            String cachedIP = dcapn.getCachedLocalAddress(eType.getType());
            String ip = dcapn.getLocalAddress(eType.getType(), 0);

            if (cachedIP == null || ip == null) {
                return true;
            }

            return !cachedIP.equals(ip);
        }

        return true;
    }

    protected void updateDataState() {
        int dataState = getDataStateFromCM();

        if (mDataState != dataState) {
            ImsLog.i(nSlotId, "data state :: " + mDataState + " >> " + dataState);

            setDataState(dataState);
            if (mDataState == TelephonyManager.DATA_CONNECTED) {
                notifyDataConnectionIpcanChanged(mNetworkType);
            }
            sendDataStateUpdateMessage(eType, EDataState.convertIntTypeToEnum(
                    (EDataState.convertFromTMtoImsType(mDataState))));
        }
    }

    protected void updateNetworkType() {
        ImsLog.i(nSlotId, "Update connected data network type");

        TelephonyManager tm = AppContext.getTelephonyManager(MSimUtils.getSubId(nSlotId));
        if (tm != null) {
            mNetworkType = tm.getDataNetworkType();
        } else {
            ImsLog.i(nSlotId, "TelephonyManager is null");
        }
    }

    protected boolean isApnEmployed() {
        if (mApnEmployCount > 0) {
            ImsLog.d(nSlotId, "apn is employed (" + mApnEmployCount + ")");
            return true;
        }

        return false;
    }

    /**
     * Send message to oneself(Apn) to clean up call stack.
     * after sometime.. we will invoke JNI api to notify network data status.
     *
     * @see handle_EVENT_NOTIFY_DATA_STATE_CHANGED
     *
     * @param apnType IMS,Emergency...
     * @param state Data state to be updated
     * @return
     */
    protected void sendDataStateUpdateMessage(EApnType apnType, EDataState dataState) {
        ImsLog.i(nSlotId, "apnType : " + apnType + ", state : " + dataState);

        //notify to watcher
        if (mDcNetWatcher != null) {
            mDcNetWatcher.notifyResult(apnType, dataState);
        }

        //notify to apn
        Message msg = Message.obtain();
        msg.what = EVENT_NOTIFY_DATA_STATE_CHANGED;
        msg.obj = new IDCNetWatcher.NotiObj(apnType, dataState);

        sendMessage(msg);
    }

    protected boolean handleIpcanCategory(int networkType) {
        if ((eType.getType() != DCConstants.TYPE_IMS)
                && (eType.getType() != DCConstants.TYPE_EMERGENCY)) {
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
        if (nSlotId != phoneId || subId == mSubId) {
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
     * Notifies the application that IPCAN category is changed.
     */
    protected void notifyIpcanCategoryChanged(int ipcanCategory) {
        ImsLog.i(nSlotId, "notifyIpcanCategoryChanged");
        for (ApnStateListener l : mApnStateListeners) {
            l.onIpcanCategoryChanged(eType.getType(), ipcanCategory);
        }
    }

    /**
     * Notifies the application that data handover information is changed.
     */
    protected void notifyHandoverInfoChanged(int handoverState, int networkType, int failCause) {
        ImsLog.i(nSlotId, "notifyHandoverInfoChanged");
        for (ApnStateListener l : mApnStateListeners) {
            l.onHandoverInfoChanged(handoverState, networkType, failCause);
        }
    }

    /**
    *  Handle_EVENT_NOTIFY_DATA_STATE_CHANGED
    *
    *        common DATA_STATE_NOTIFICATION handling logic from android PhoneState
    *        IMS / Emergency APN nothing different to handle current event.
    *        if we need specific control, child class just need override this.
    * @param
    * @return
    */
    protected class Handle_EVENT_NOTIFY_DATA_STATE_CHANGED implements MsgProcInterface {

        @Override
        public void procMsg(Message msg) {
            // Ignore DataState Change During ShutDown
            if ((mDcNetWatcher != null) && (mDcNetWatcher.isDoingOffRadio() == true)) {
                ImsLog.w(nSlotId, "radio is doing off");
                return;
            }

            IDCNetWatcher.NotiObj res = (IDCNetWatcher.NotiObj)msg.obj;
            if (res == null) {
                return;
            }

            // Do not use dataState vi intent
            EApnType apnType = res.eApnType;
            EDataState dataState = res.eDataState;

            ISystem system = SystemInterface.getInstance().getSystem(nSlotId);
            if (system == null) {
                return;
            }

            if (dataState == EDataState.DATA_STATE_CONNECT_FAILED) {
                ImsLog.w(nSlotId, "Data Connection failed : apnType=" + apnType);
                system.notifyDataConnectionFailed(apnType.getType());
                return;
            }

            ImsLog.i(nSlotId, "apnType=" + apnType
                    + " : " + apnType.getString() + ", dataState=" + dataState);

            system.notifyDataConnectionStateChanged(
                    apnType.getType(), dataState.getState());
        }
    }

    protected class Handle_EVENT_PRECISE_DATA_CONNECTION_STATE_CHANGED
            implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            if (msg.obj == null) {
                ImsLog.w(nSlotId, "msg.obj is null");
                return;
            }
            PreciseDataConnectionState dataConnectionState = (PreciseDataConnectionState)msg.obj;
            ApnSetting apnSetting = dataConnectionState.getApnSetting();
            int dataState = dataConnectionState.getState();
            int networkType = dataConnectionState.getNetworkType();
            int causeCode = dataConnectionState.getLastCauseCode();

            if (networkType == TelephonyManager.NETWORK_TYPE_UNKNOWN) {
                ImsLog.w(nSlotId, "Unknown network type");
                return;
            }

            switch (dataState) {
                case TelephonyManager.DATA_CONNECTED:
                    if (mPreciseDcState == TelephonyManager.DATA_HANDOVER_IN_PROGRESS) {
                        if (mNetworkType == TelephonyManager.NETWORK_TYPE_UNKNOWN) {
                            ImsLog.w(nSlotId, "Not handle handover from unknown network");
                            break;
                        }

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
                        if ((mDcNetWatcher != null) && mDcNetWatcher.isRoaming()){
                            mApnProtocol = apnSetting.getRoamingProtocol();
                        } else {
                            mApnProtocol = apnSetting.getProtocol();
                        }
                    }
                    if (mNetworkType != networkType) {
                        // update network type
                        ImsLog.i(nSlotId, "network type :: "+ mNetworkType +" >> "+ networkType);
                        mNetworkType = networkType;
                    }
                    break;
                case TelephonyManager.DATA_HANDOVER_IN_PROGRESS:
                    handleHandoverStart(networkType);
                    break;
                case TelephonyManager.DATA_DISCONNECTING:
                    // To Be update
                    break;
                case TelephonyManager.DATA_DISCONNECTED:
                    if (mPreciseDcState == TelephonyManager.DATA_CONNECTING &&
                            causeCode != DataFailCause.NONE) {
                        // initial connection failure
                        handleInitialConnectionFailure(causeCode);
                    }
                    mNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
                    break;
                default:
                    // no-op
                    break;
            }

            // update PreciseDataConnectionState
            mPreciseDcState = dataState;
        }

        private void handleHandoverStart(int networkType) {
            ImsLog.i(nSlotId, "handleHandoverStart");
            notifyHandoverInfoChanged(HANDOVER_START, networkType, DataFailCause.NONE);
        }

        private void handleHandoverSuccess(int networkType) {
            ImsLog.i(nSlotId, "handleHandoverSuccess");
            notifyHandoverInfoChanged(HANDOVER_SUCCESS, networkType, DataFailCause.NONE);
            handleIpcanCategory(networkType);
        }

        private void handleHandoverFailure(int networkType, int causeCode) {
            ImsLog.i(nSlotId, "handleHandoverFailure");
            notifyHandoverInfoChanged(HANDOVER_FAILURE, networkType, causeCode);
        }

        private void handleInitialConnectionFailure(int causeCode) {
            ImsLog.i(nSlotId, "handleInitialConnectionFailed");
            if (mDcSettings != null) {
                mESMCausePermanentFailure = mDcSettings.isPermanentFailure(causeCode);
            }
            Message msg = Message.obtain();
            msg.what = EVENT_DATA_CONNECTION_FAILED;
            msg.obj = causeCode;
            sendMessage(msg);
        }

        private boolean isIpcanChanged(int networkToCheck) {
            int currentType = (mNetworkType == TelephonyManager.NETWORK_TYPE_IWLAN) ?
                    IPCAN_CATEGORY_WLAN : IPCAN_CATEGORY_MOBILE;
            int checkType = (networkToCheck == TelephonyManager.NETWORK_TYPE_IWLAN) ?
                    IPCAN_CATEGORY_WLAN : IPCAN_CATEGORY_MOBILE;

            if (currentType == checkType) {
                return false;
            } else {
                return true;
            }
        }
    }

    /* ---------------------------------------------------------------------------------------------
        Listener class - SubscriptionListener
    --------------------------------------------------------------------------------------------- */
    private final class ApnSubscriptionListener extends SubscriptionListener {
        public ApnSubscriptionListener() {
            ImsLog.d("ApnSubscriptionListener");
        }

        @Override
        public void onCarrierConfigChanged(int phoneId, int subId) {
            handleCarrierConfigChanged(phoneId, subId);
        }
    }

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
        // DCConstants.TYPE_XXX
        private final int mType;
        private Network mNetwork = null;
        private Handler mTarget;
        private int mEvents = 0;
        private int mSlotId = 0;
        private LinkProperties mCachedLinkProperties = null;
        private boolean mIsPendingOnAvailable = false;

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

        public void cleanUp() {
            mEvents = 0;
            if (mTarget != null) {
                mTarget.removeCallbacksAndMessages(null);
                mTarget = null;
            }
        }

        // DCConstants.TYPE_XXX
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

            if ((mCachedLinkProperties == null) || !isNetworkConnected()) {
                ImsLog.w(mSlotId, "no LinkProperties");
                mIsPendingOnAvailable = true;
                return;
            }

            if (mTarget != null) {
                Message.obtain(mTarget, EVENT_NETWORK_AVAILABLE, this).sendToTarget();
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
                Message.obtain(mTarget, EVENT_NETWORK_LOSING, maxMsToLive, 0, this)
                        .sendToTarget();
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
                Message.obtain(mTarget, EVENT_NETWORK_LOST, this).sendToTarget();
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
                Message.obtain(mTarget, EVENT_NETWORK_UNAVAILABLE, this).sendToTarget();
            }
        }

        @Override
        public void onCapabilitiesChanged(Network network,
                NetworkCapabilities networkCapabilities) {
            if (mIsPendingOnAvailable) {
                if (isNetworkConnected()) {
                    ImsLog.w(mSlotId, "network is connected");
                    mIsPendingOnAvailable = false;
                    onAvailable(network);
                }
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
                Message.obtain(mTarget, EVENT_NETWORK_CAPABILITIES_CHANGED, this)
                        .sendToTarget();
            }
        }

        @Override
        public void onLinkPropertiesChanged(Network network,
                LinkProperties linkProperties) {
            boolean ipChanged = isIpChanged(linkProperties);
            boolean pcscfChanged = isPcscfChanged(linkProperties);

            if (ipChanged || pcscfChanged) {
                cacheLinkProperties(network);
            }

            if (!isEventSet(EVENT_LINK_PROPERTIES_CHANGED)
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
                Message.obtain(mTarget, EVENT_IP_CHANGED, this).sendToTarget();
            }

            if (isEventSet(EVENT_NET_PCSCF_CHANGED) && pcscfChanged) {
                Message.obtain(mTarget, EVENT_PCSCF_CHANGED, this)
                        .sendToTarget();
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
            boolean bChanged = false;

            bChanged = ((mCachedLinkProperties != null) && (newLinkProperties != null)
                    && !LinkPropertiesUtils.isIdenticalAddresses(
                            mCachedLinkProperties, newLinkProperties));

            if (bChanged) {
                String[] cachedAddress = null;
                String[] newAddress = null;

                if (mCachedLinkProperties != null ) {
                    Collection<LinkAddress> cachedlinkAddresses
                            = mCachedLinkProperties.getLinkAddresses();
                    if (cachedlinkAddresses.isEmpty()) {
                        ImsLog.w(mSlotId, "cached LinkAddresses is empty, ");

                        if (newLinkProperties != null) {
                            Collection<LinkAddress> newAddr = newLinkProperties.getLinkAddresses();
                            if (!newAddr.isEmpty()) {
                                ImsLog.w(mSlotId, "new LinkAddresses is not empty");
                                return true;
                            }
                        }

                        return false;
                    }

                    cachedAddress = getIPAddress(cachedlinkAddresses);
                }

                if (newLinkProperties != null) {
                    Collection<LinkAddress> newlinkAddresses
                            = newLinkProperties.getLinkAddresses();
                    if (newlinkAddresses.isEmpty()) {
                        ImsLog.w(mSlotId, "new LinkAddresses is empty, ");
                        return false;
                    }

                    newAddress = getIPAddress(newlinkAddresses);
                }

                if ((cachedAddress == null) || (newAddress == null)) {
                    return false;
                }

                printAddress("cached ip address", cachedAddress);
                printAddress("new ip address", newAddress);

                int nSize = 0;
                if (cachedAddress.length != newAddress.length) {
                    return true;
                }
                else {
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

            return bChanged;
        }

        protected boolean isPcscfChanged(LinkProperties newLinkProperties) {
            if (getType() != DCConstants.TYPE_IMS) {
                return false;
            }

            if (mCachedLinkProperties == null || newLinkProperties == null) {
                return false;
            }

            String[] cachedAddress = null;
            String[] newAddress = null;

            Collection<InetAddress> cachedInetAddresses = mCachedLinkProperties.getPcscfServers();
            if (cachedInetAddresses.isEmpty()) {
                ImsLog.d(mSlotId, "cached InetAddress is empty");
                return false;
            }

            cachedAddress = getPcscfAddress(cachedInetAddresses);

            Collection<InetAddress> newInetAddresses = newLinkProperties.getPcscfServers();
            if (newInetAddresses.isEmpty()) {
                ImsLog.d(mSlotId, "new InetAddress is empty");
                return false;
            }

            newAddress = getPcscfAddress(newInetAddresses);

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

        private String[] getIPAddress(Collection<LinkAddress> linkAddresses) {
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
                    ImsLog.i(mSlotId, "ip6Address - " + ip6Addr );

                    boolean bSame = false;
                    for (int j = 0; j < i; j++) {
                        if (addr[j].equals(ip6Addr)) {
                            bSame = true;
                        }
                    }

                    if (!bSame) {
                        addr[i] = ip6Addr;
                        ImsLog.i(mSlotId, "saved ip6Address - [" + i + "]" + addr[i] );
                        i++;
                    }
                }

                if (netAddress instanceof Inet4Address) {
                    String ip4Addr = netAddress.getHostAddress();
                    ImsLog.i(mSlotId, "ip4Address - " + ip4Addr );

                    boolean bSame = false;
                    for (int j = 0; j < i; j++) {
                        if (addr[j].equals(ip4Addr)) {
                            bSame = true;
                        }
                    }

                    if (!bSame) {
                        addr[i] = ip4Addr;
                        ImsLog.i(mSlotId, "saved ip4Address - [" + i + "]" + addr[i] );
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

            Iterator<InetAddress> iterator = inetAddresses.iterator();

            if (iterator == null) {
                return null;
            }

            String[] addr = new String[inetAddresses.size()];

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

        private void printAddress(String prifix, String[] addresses) {
            StringBuffer sb = new StringBuffer();
            sb.append(prifix + " : ");

            for (int i = 0; i < addresses.length; i++) {
                sb.append(addresses[i] + " / ");
            }

            ImsLog.d(mSlotId, sb.toString());
        }

        protected boolean isNetworkConnected() {
            ConnectivityManager cm = (mContext == null) ? null :
                    mContext.getSystemService(ConnectivityManager.class);

            if (cm != null) {
                if (mNetwork == null) {
                    return false;
                }

                NetworkInfo netInfo = cm.getNetworkInfo(mNetwork);

                if (netInfo != null) {
                    return netInfo.isConnected()
                            || netInfo.getState() == NetworkInfo.State.SUSPENDED;
                }
            }
            return false;
        }
    }

    static {
        sEventToString = new Hashtable<Integer, String>();

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
        sEventToString.put(EVENT_WAITING_LOCAL_ADDRESS_IPV6,
                "WAITING_LOCAL_ADDRESS_IPV6");
        sEventToString.put(EVENT_NOTIFY_DATA_STATE_CHANGED,
                "NOTIFY_DATA_STATE_CHANGED");
        sEventToString.put(EVENT_PRECISE_DATA_CONNECTION_STATE_CHANGED,
                "PRECISE_DATA_CONNECTION_STATE_CHANGED");
        sEventToString.put(EVENT_DATA_CONNECTION_FAILED,
                "DATA_CONNECTION_FAILED");

        sEventToString.put(EVENT_AIRPLANE_MODE_CHANGED,
                "AIRPLANE_MODE_CHANGED");
        sEventToString.put(EVENT_WIFI_STATE_CHANGED,
                "WIFI_STATE_CHANGED");
        sEventToString.put(EVENT_VOWIFI_SETTING_CHANGED,
                "VOWIFI_SETTING_CHANGED");
        sEventToString.put(EVENT_VOWIFI_PREF_CHANGED,
                "VOWIFI_PREF_CHANGED");
        sEventToString.put(EVENT_VOWIFI_ROAMING_PREF_CHANGED,
                "VOWIFI_ROAMING_PREF_CHANGED");
    }
}
