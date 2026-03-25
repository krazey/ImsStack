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

import android.content.Context;
import android.os.Message;
import android.telephony.Annotation.DataState;
import android.telephony.Annotation.NetworkType;
import android.telephony.TelephonyManager;

import androidx.annotation.VisibleForTesting;

import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.base.SystemServiceProxy.ConnectivityManagerProxy;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.MsgProcInterface;
import com.android.imsstack.core.agents.SubsInfoInterface;
import com.android.imsstack.core.agents.dcmif.EApnReqState;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.util.ImsLog;

/**
 * this is data connection class for ims
 */
public final class ApnIms extends Apn {
    @VisibleForTesting
    boolean mIsCellularDefaultNetwork = false;
    @VisibleForTesting
    boolean mIsConnectedOverCrossSim = false;
    private DefaultNetworkCallback mDefaultNetworkCallback = null;

    // Public methods --------------------------------------------
    public ApnIms(Context context, int slotId) {
        super(context, slotId, EApnType.IMS);

        registerHandler(EVENT_NETWORK_AVAILABLE, new HandleNetworkAvailable());
        registerHandler(EVENT_NETWORK_LOST, new HandleNetworkLost());
        registerHandler(EVENT_IP_CHANGED, new HandleIpChanged());
        registerHandler(EVENT_PCSCF_CHANGED, new HandlePcscfChanged());
        registerHandler(EVENT_DATA_CONNECTION_FAILED, new HandleDataConnectionFailed());
        registerHandler(EVENT_DEFAULT_NETWORK_STATUS_CHANGED,
                new HandleDefaultNetworkStatusChanged());

        registerConfigListener();
        registerSystemDefaultNetworkCallback();
    }

    // Interface implementation methods --------------------------
    @Override
    public void cleanup() {
        ImsLog.d(mSlotId, "clean up");
        unregisterSystemDefaultNetworkCallback();

        super.cleanup();
    }

    @Override
    public boolean connect() {
        synchronized (this) {
            SubsInfoInterface subsInfo = AgentFactory.getInstance().getAgent(
                    SubsInfoInterface.class, mSlotId);
            if (subsInfo != null) {
                if (!subsInfo.isImsEnabled()) {
                    ImsLog.w(mSlotId, "ims is off");
                    return false;
                }
            }

            if (mAPNState == EApnReqState.APN_REQUEST_DONE) {
                ImsLog.i(mSlotId, "apn request is already done");
                return true;
            }

            setApnReqState(EApnReqState.APN_REQUEST_DONE);
            requestNetwork();

            setDataState(TelephonyManager.DATA_CONNECTING);

            return true;
        }
    }

    @Override
    public boolean disconnect() {
        synchronized (this) {
            if (mAPNState != EApnReqState.APN_REQUEST_DONE) {
                ImsLog.w(mSlotId, "apn request is not done");
                return false;
            }

            releaseNetwork();
            setApnReqState(EApnReqState.APN_REQUEST_IDLE);

            int dataState = TelephonyManager.DATA_DISCONNECTED;

            if (mDataState != dataState) {
                ImsLog.w(mSlotId, "data state :: " + mDataState + " >> " + dataState);

                setDataState(dataState);
            }

            sendDataStateUpdateMessage(mType, EDataState.DATA_STATE_DISCONNECTED);

            return true;
        }
    }

    @Override
    public String getApn() {
        if (mApnString != null) {
            return mApnString;
        }

        return super.getApn();
    }

    /**
     * Called when carrier config changes
     * ApnIms register or unregister DefaultNetworkCallback according to the configuration
     */
    @Override
    protected void handleCarrierConfigChanged(int phoneId, int subId) {
        super.handleCarrierConfigChanged(phoneId, subId);

        if (mSlotId != phoneId) {
            return;
        }
        unregisterSystemDefaultNetworkCallback();
        registerSystemDefaultNetworkCallback();
    }

    /**
     * If the access network status is changed, update CrossSim connection status and notify it.
     * @param dataState The current precise data connection state
     * @param networkType The type of access network that carries this data connection
     */
    @Override
    protected void updateCrossSimStatus(@DataState int dataState, @NetworkType int networkType) {
        boolean isCrossSimActive = mDcNetWatcher == null
                || mDcNetWatcher.isCrossSimCapabilityEnabled() || mIsConnectedOverCrossSim;
        if (!isCrossSimActive) {
            ImsLog.d(mSlotId, "IMS service capabilities are disabled for CrossSim");
            return;
        }

        boolean isNetworkTypeIwlan = (networkType == TelephonyManager.NETWORK_TYPE_IWLAN);
        boolean isConnected = (dataState == TelephonyManager.DATA_CONNECTED);
        boolean isCrossSimUsed = (mIsCellularDefaultNetwork && isNetworkTypeIwlan && isConnected);
        if (mIsConnectedOverCrossSim != isCrossSimUsed) {
            ImsLog.i(mSlotId, "Update CrossSim status from " + mIsConnectedOverCrossSim
                    + " to " + isCrossSimUsed);
            mIsConnectedOverCrossSim = isCrossSimUsed;
            for (Listener l : mListeners) {
                l.onCrossSimStatusChanged(mIsConnectedOverCrossSim, isNetworkTypeIwlan);
            }
        }
    }

    @VisibleForTesting
    void registerSystemDefaultNetworkCallback() {
        if (!DeviceConfig.isMultiSimEnabled()) {
            ImsLog.i(mSlotId, "MultiSim is not enabled");
            return;
        }
        if (mDcSettings == null || !mDcSettings.isCrossSimEnabledByPlatform()) {
            ImsLog.i(mSlotId, "CrossSim is not enabled");
            return;
        }
        if (mDefaultNetworkCallback != null) {
            ImsLog.i(mSlotId, "Default network callback already has been registered");
            return;
        }

        ImsLog.i(mSlotId, "registerSystemDefaultNetworkCallback");
        ConnectivityManagerProxy cmp = getConnectivityManagerProxy();

        mDefaultNetworkCallback = new DefaultNetworkCallback(mSlotId, this);
        try {
            cmp.registerSystemDefaultNetworkCallback(mDefaultNetworkCallback, this);
        } catch (RuntimeException e) {
            ImsLog.e(mSlotId, "registerSystemDefaultNetworkCallback: " + e.getMessage());
            mDefaultNetworkCallback = null;
        }
    }

    @VisibleForTesting
    void unregisterSystemDefaultNetworkCallback() {
        if (mDefaultNetworkCallback == null) {
            ImsLog.i(mSlotId, "Default network callback has been not registered");
            return;
        }

        ImsLog.i(mSlotId, "unregisterSystemDefaultNetworkCallback");
        ConnectivityManagerProxy cmp = getConnectivityManagerProxy();
        try {
            cmp.unregisterNetworkCallback(mDefaultNetworkCallback);
        } catch (RuntimeException e) {
            ImsLog.e(mSlotId, "unregisterSystemDefaultNetworkCallback: " + e.getMessage());
        }
        mDefaultNetworkCallback = null;
    }

    private boolean shouldDisableN1Mode() {
        return mDcSettings.disableN1ModeOnImsPduEstablishFailure()
                && (mDcNetWatcher.getNetworkType() == TelephonyManager.NETWORK_TYPE_NR);
    }

    private final class HandleNetworkAvailable implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            int curDataState = TelephonyManager.DATA_CONNECTED;

            if (mAPNState != EApnReqState.APN_REQUEST_DONE) {
                ImsLog.w(mSlotId, "apn is not requested, ignore event");
                return;
            }

            if (mDataState != curDataState) {
                ImsLog.w(mSlotId, "data state :: " + mDataState + " >> " + curDataState
                        + ", apn string = " + mApnString + ", network type = " + mNetworkType);

                setDataState(curDataState);
                sendDataStateUpdateMessage(mType, EDataState.DATA_STATE_CONNECTED);
            }
        }
    }

    private final class HandleNetworkLost implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            int curDataState = TelephonyManager.DATA_DISCONNECTED;

            if (mAPNState != EApnReqState.APN_REQUEST_DONE) {
                ImsLog.w(mSlotId, "apn is not requested");

                if (mDataState != TelephonyManager.DATA_CONNECTED) {
                    ImsLog.w(mSlotId, "data is not connected");
                    return;
                }
            }

            if (mDataState != curDataState) {
                ImsLog.i(mSlotId, "data state :: " + mDataState + " >> " + curDataState);

                setDataState(curDataState);

                sendDataStateUpdateMessage(mType, EDataState.DATA_STATE_DISCONNECTED);
            }
        }
    }

    private final class HandleIpChanged implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            ImsLog.i(mSlotId, "ip is changed");

            if (getDataState() != TelephonyManager.DATA_CONNECTED) {
                return;
            }

            if (!isIpChanged()) {
                ImsLog.i(mSlotId, "ip is changed but ip address is same");
                return;
            }

            sendDataStateUpdateMessage(mType, EDataState.DATA_STATE_IP_CHANGED);
        }
    }

    private final class HandlePcscfChanged implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            ImsLog.i(mSlotId, "PCSCF address is changed");

            sendDataStateUpdateMessage(mType, EDataState.DATA_STATE_PCSCF_CHANGED);
        }
    }

    private final class HandleDataConnectionFailed implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            ImsLog.d(mSlotId, "");

            if (msg == null || msg.obj == null) {
                return;
            }

            if (mAPNState != EApnReqState.APN_REQUEST_DONE) {
                ImsLog.w("apn is not requested, ignore event");
                return;
            }

            int causeCode = (int) msg.obj;
            if (mDcSettings != null) {
                if (mDcSettings.isPermanentFailure(mType, causeCode) || shouldDisableN1Mode()) {
                    sendDataStateUpdateMessage(mType, EDataState.DATA_STATE_CONNECT_FAILED);
                }
            }
        }
    }

    /**
     * This handle EVENT_DEFAULT_NETWORK_STATUS_CHANGED event
     */
    private final class HandleDefaultNetworkStatusChanged implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            ImsLog.d(mSlotId, "");
            if (msg == null || msg.obj == null) {
                return;
            }

            boolean isCellularAvailable = (boolean) msg.obj;
            if (mIsCellularDefaultNetwork != isCellularAvailable) {
                mIsCellularDefaultNetwork = isCellularAvailable;
                updateCrossSimStatus(mPreciseDcState, mNetworkType);
            }
        }
    }
}
