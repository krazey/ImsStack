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
import android.telephony.DataFailCause;
import android.telephony.TelephonyManager;

import com.android.imsstack.core.agents.MsgProcInterface;
import com.android.imsstack.core.agents.dcmif.EApnReqState;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.IDcUtils;
import com.android.imsstack.enabler.mtc.Call;
import com.android.imsstack.internal.enabler.MtcCallRegistry;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

/**
 * this is data connection class for emergency
 */
public final class ApnEmergency extends Apn {
    static final int DISCONNECT_DELAY_TIME = 1000;
    @VisibleForTesting
    boolean mIsWaitForTransportChange = false;
    private IDcUtils mDcUtils;
    final MtcCallRegistryListener mMtcCallRegistryListener = new MtcCallRegistryListener();

    // Public methods --------------------------------------------
    public ApnEmergency(Context context, int slotId) {
        super(context, slotId, EApnType.EMERGENCY);

        mDcUtils = DcFactory.getDcAgent(IDcUtils.class, slotId);

        registerHandler(EVENT_NETWORK_AVAILABLE, new HandleNetworkAvailable());
        registerHandler(EVENT_NETWORK_LOST, new HandleNetworkLost());
        registerHandler(EVENT_IP_CHANGED, new HandleIpChanged());
        registerHandler(EVENT_DATA_CONNECTION_FAILED, new HandleDataConnectionFailed());
        registerHandler(EVENT_DELAYED_DISCONNECT, new HandleDelayedDisconnect());
        registerHandler(EVENT_CALL_CREATED, new HandleCallCreated());
        registerHandler(EVENT_CALL_DESTROYED, new HandleCallDestroyed());

        MtcCallRegistry mtcCallRegistry = MtcCallRegistry.getInstance(mSlotId);
        mtcCallRegistry.addListener(mMtcCallRegistryListener);
    }

    // Interface implementation methods --------------------------
    @Override
    public void cleanup() {
        MtcCallRegistry mtcCallRegistry = MtcCallRegistry.getInstance(mSlotId);
        mtcCallRegistry.removeListener(mMtcCallRegistryListener);
        mIsWaitForTransportChange = false;

        super.cleanup();
    }

    @Override
    public boolean connect() {
        mIsWaitForTransportChange = false;
        removeMessages(EVENT_DELAYED_DISCONNECT);

        if (mAPNState == EApnReqState.APN_REQUEST_DONE) {
            ImsLog.w(mSlotId, "request is already done");
            return true;
        }

        if (mDcNetWatcher != null) {
            mDcNetWatcher.clearNetworkRegistrationRejectCause();
        }

        setApnReqState(EApnReqState.APN_REQUEST_DONE);

        requestNetwork();

        setDataState(TelephonyManager.DATA_CONNECTING);

        return true;
    }

    @Override
    public boolean disconnect() {
        if (mAPNState != EApnReqState.APN_REQUEST_DONE) {
            ImsLog.w(mSlotId, "request is not done");
            return false;
        }

        if (mIsWaitForTransportChange) {
            ImsLog.w(mSlotId, "wait for transport change before disconnect");
            if (!hasMessages(EVENT_DELAYED_DISCONNECT)) {
                sendEmptyMessageDelayed(EVENT_DELAYED_DISCONNECT, DISCONNECT_DELAY_TIME);
            }
            return true;
        }

        releaseNetwork();
        setDataState(TelephonyManager.DATA_DISCONNECTED);
        setApnReqState(EApnReqState.APN_REQUEST_IDLE);

        if (isConnected()) {
            sendDataStateUpdateMessage(mType, EDataState.DATA_STATE_DISCONNECTED);
        }

        return true;
    }

    @Override
    public String getApn() {
        if (mApnString != null) {
            return mApnString;
        }

        return super.getApn();
    }

    @VisibleForTesting
    protected class MtcCallRegistryListener implements MtcCallRegistry.Listener {
        @Override
        public void onCallCreated(Call call) {
            if (!call.getCallExtraBoolean(Call.EXTRA_E_CALL, false)) {
                return;
            }

            if (getDataState() != TelephonyManager.DATA_CONNECTED) {
                return;
            }

            sendMessage(obtainMessage(EVENT_CALL_CREATED,
                    call.getCallExtraBoolean(Call.EXTRA_WIFI_E_CALL, false)));
        }

        @Override
        public void onCallDestroyed(Call call) {
            if (!call.getCallExtraBoolean(Call.EXTRA_E_CALL, false)) {
                return;
            }

            sendMessage(obtainMessage(EVENT_CALL_DESTROYED));
        }
    }

    private final class HandleNetworkAvailable implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {

            int curDataState = TelephonyManager.DATA_CONNECTED;

            if (mAPNState != EApnReqState.APN_REQUEST_DONE) {
                ImsLog.w(mSlotId, "apn is not requested, ignore event");
                return;
            }

            mIsWaitForTransportChange = false;
            removeMessages(EVENT_DELAYED_DISCONNECT);

            if (mDataState != curDataState) {
                if (mDcUtils != null) {
                    mDcUtils.updateAllCellInfoForcinglyOnLimitedServiceState();
                }
            }

            if (mDataState != curDataState) {
                ImsLog.i(mSlotId, "data state :: " + mDataState + " >> " + curDataState
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
                return;
            }

            if (mDataState != curDataState) {
                ImsLog.i(mSlotId, "data state :: " + mDataState + " >> " + curDataState);

                setDataState(curDataState);
                if (mDcSettings != null && !mDcSettings.isEmergencyCallbackModeSupported()) {
                    ImsLog.d(mSlotId, "network release due to not support ECBM");
                    disconnect();
                }
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

    private final class HandleDataConnectionFailed implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            if (msg == null || msg.obj == null) {
                return;
            }

            if (mAPNState != EApnReqState.APN_REQUEST_DONE) {
                ImsLog.w("apn is not requested, ignore event");
                return;
            }

            if (mDcSettings != null) {
                int causeCode = (int) msg.obj;
                if (mDcSettings.isCrossStackRedialCause(mType, causeCode)) {
                    sendDataStateUpdateMessage(mType, EDataState.DATA_STATE_CONNECT_FAILED);
                    return;
                }

                if (causeCode == DataFailCause.ERROR_UNSPECIFIED && mDcNetWatcher != null) {
                    // If E-PDN connection is failed by EMM rejection, it needs to check the reject
                    // cause in the network registration info.
                    int nwRejectCause = mDcNetWatcher.getNetworkRegistrationRejectCause();
                    if (mDcSettings.isCrossStackRedialCause(mType, nwRejectCause)) {
                        ImsLog.d(mSlotId, "nwRejectCause : " + nwRejectCause);
                        sendDataStateUpdateMessage(mType, EDataState.DATA_STATE_CONNECT_FAILED);
                        return;
                    }
                }
            }

            sendDataStateUpdateMessage(mType, EDataState.DATA_STATE_DISCONNECTED);
        }
    }

    private final class HandleDelayedDisconnect implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            ImsLog.i(mSlotId, "handle delayed disconnect");
            mIsWaitForTransportChange = false;
            disconnect();
        }
    }

    private final class HandleCallCreated implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            if (msg == null || msg.obj == null) {
                return;
            }

            boolean isWifiCall = (boolean) msg.obj;
            int eCallTransportType = (isWifiCall) ? IPCAN_CATEGORY_WLAN : IPCAN_CATEGORY_MOBILE;
            mIsWaitForTransportChange = (eCallTransportType != mIpcanCategory);
            ImsLog.i(mSlotId, "Telephony selected: " + (isWifiCall ? "WLAN" : "WWAN")
                    + ", Wait for change: " + mIsWaitForTransportChange);
        }
    }

    private final class HandleCallDestroyed implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            mIsWaitForTransportChange = false;
            if (hasMessages(EVENT_DELAYED_DISCONNECT)) {
                removeMessages(EVENT_DELAYED_DISCONNECT);
                disconnect();
            }
        }
    }
}
