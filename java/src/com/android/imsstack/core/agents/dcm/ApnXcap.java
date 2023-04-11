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
import android.os.AsyncResult;
import android.os.Message;
import android.telephony.TelephonyManager;
import android.telephony.data.ApnSetting;

import com.android.imsstack.core.agents.MsgProcInterface;
import com.android.imsstack.core.agents.dcmif.EApnReqState;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.EIpVersion;
import com.android.imsstack.util.ImsLog;

/**
 * this is data connection class for xcap
 */
public class ApnXcap extends Apn {

    // Constants--------------------------------------------------
    protected static final int OBTAIN_IPV6_ADDRESS_DELAY_INTERVAL = 2000;

    // Variables--------------------------------------------------

    // Public methods --------------------------------------------
    public ApnXcap(Context context, int slotId) {
        super(context, slotId);

        initializeApn();
    }

    // Interface implementation methods --------------------------
    @Override
    public void cleanup() {
        if (mDcNetWatcher != null) {
            mDcNetWatcher.unregisterForAirplaneModeChanged(this);
        }
        super.cleanup();
    }

    @Override
    public boolean connect() {
        if (mAPNState == EApnReqState.APN_REQUEST_DONE) {
            ImsLog.i(mSlotId, "apn request is already done");
            return true;
        }

        requestNetwork();
        setApnReqState(EApnReqState.APN_REQUEST_DONE);
        setDataState(TelephonyManager.DATA_CONNECTING);

        return true;
    }

    @Override
    public boolean disconnect() {
        if (mAPNState != EApnReqState.APN_REQUEST_DONE) {
            ImsLog.w(mSlotId, "apn request is not done");
            return false;
        }

        if (hasMessages(EVENT_WAITING_IPV6_ADDRESS)) {
            removeMessages(EVENT_WAITING_IPV6_ADDRESS);
        }

        releaseNetwork();
        setApnReqState(EApnReqState.APN_REQUEST_IDLE);

        int dataState = TelephonyManager.DATA_DISCONNECTED;
        if (mDataState != dataState) {
            setDataState(dataState);
        }

        sendDataStateUpdateMessage(mType, EDataState.DATA_STATE_DISCONNECTED);

        return true;
    }

    // Private/Protected methods ---------------------------------
    protected void initializeApn() {
        mType = EApnType.XCAP;

        registerHandler(EVENT_NETWORK_AVAILABLE, new HandleNetworkAvailable());
        registerHandler(EVENT_NETWORK_LOST, new HandleNetworkLost());
        registerHandler(EVENT_IP_CHANGED, new HandleIpChanged());
        registerHandler(EVENT_WAITING_IPV6_ADDRESS, new HandleWaitingIpv6Address());
        registerHandler(EVENT_AIRPLANE_MODE_CHANGED, new HandleAirplanemodeChanged());
        registerHandler(EVENT_DATA_CONNECTION_FAILED, new HandleDataConnectionFailed());

        //register message handler to DcNetWatcher
        if (mDcNetWatcher != null) {
            mDcNetWatcher.registerForAirplaneModeChanged(this, EVENT_AIRPLANE_MODE_CHANGED, null);
        }
    }

    private boolean procWaitingLocalAddressForIpv6() {
        if (!hasLocalAddress(EIpVersion.IPV6.getInt())) {
            if (hasMessages(EVENT_WAITING_IPV6_ADDRESS)) {
                ImsLog.i("ipv6 is waiting");
                return true;
            }

            sendEmptyMessageDelayed(EVENT_WAITING_IPV6_ADDRESS,
                    OBTAIN_IPV6_ADDRESS_DELAY_INTERVAL);
            return true;
        }

        return false;
    }

    private class HandleNetworkAvailable implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            int curDataState = TelephonyManager.DATA_CONNECTED;

            if (mAPNState != EApnReqState.APN_REQUEST_DONE) {
                ImsLog.w(mSlotId, "apn is not requested, ignore event");
                return;
            }

            ImsLog.i(mSlotId, "IP Connectivity Access Network : " + mIpcanCategory);

            // Check to get Ipv6
            if (mApnProtocol != ApnSetting.PROTOCOL_IP) {
                if (procWaitingLocalAddressForIpv6()) {
                    ImsLog.i("can't obtain IPv6, wait "
                            + OBTAIN_IPV6_ADDRESS_DELAY_INTERVAL + "ms");
                    return;
                }
            }

            if (mDataState != curDataState) {
                ImsLog.i(mSlotId, "data state :: " + mDataState + " >> " + curDataState);

                setDataState(curDataState);
                sendDataStateUpdateMessage(mType, EDataState.DATA_STATE_CONNECTED);
            }
        }
    }

    private class HandleNetworkLost implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            int curDataState = TelephonyManager.DATA_DISCONNECTED;

            if (hasMessages(EVENT_WAITING_IPV6_ADDRESS)) {
                removeMessages(EVENT_WAITING_IPV6_ADDRESS);
            }

            if (mDataState != curDataState) {
                ImsLog.i(mSlotId, "data state :: " + mDataState + " >> " + curDataState);

                setDataState(curDataState);

                sendDataStateUpdateMessage(mType, EDataState.DATA_STATE_DISCONNECTED);

                disconnect();
            }
        }
    }

    private class HandleIpChanged implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            ImsLog.i(mSlotId, "ip is changed");

            if (getDataState() == TelephonyManager.DATA_CONNECTED) {

                if (!isIpChanged()) {
                    ImsLog.i(mSlotId, "ip is changed but ip address is same");
                    return;
                }

                sendDataStateUpdateMessage(mType, EDataState.DATA_STATE_IP_CHANGED);
                return;
            }

            // handle the waiting Ipv6
            if (!hasMessages(EVENT_WAITING_IPV6_ADDRESS)) {
                return;
            }

            removeMessages(EVENT_WAITING_IPV6_ADDRESS);

            updateDataState();
        }
    }

    private class HandleWaitingIpv6Address implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            ImsLog.i(mSlotId, "apn is delayed, data is updated");
            updateDataState();
        }
    }

    private class HandleAirplanemodeChanged implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;

            if (ar == null) {
                ImsLog.d(mSlotId, "ar is null");
                return;
            }

            Boolean radiooff = (Boolean) ar.result;

            if (radiooff == null) {
                ImsLog.d(mSlotId, "radiooff is null");
                return;
            }

            ImsLog.i(mSlotId, "airplane mode = " + radiooff.booleanValue());

            if (radiooff.booleanValue()) {
                disconnect();
            }
        }
    }

    private class HandleDataConnectionFailed implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            ImsLog.d(mSlotId, "");

            if (msg == null) {
                return;
            }

            if (mAPNState != EApnReqState.APN_REQUEST_DONE) {
                ImsLog.w("apn is not requested, ignore event");
                return;
            }

            if (msg.obj == null) {
                ImsLog.w(mSlotId, "msg.obj is null");
                return;
            }

            int causeCode = (int) msg.obj;
            ImsLog.w(mSlotId, "SM cause : " + causeCode);

            notifyPdnConnectionFailed(mType, causeCode);
        }
    }
}
