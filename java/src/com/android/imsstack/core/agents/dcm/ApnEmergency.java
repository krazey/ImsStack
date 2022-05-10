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
import android.telephony.TelephonyManager;

import com.android.imsstack.core.agents.agentif.MsgProcInterface;
import com.android.imsstack.core.agents.dcmif.EApnReqState;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.IDCUtil;
import com.android.imsstack.util.ImsLog;

/**
 * this is data connection class for emergency
 */
public class ApnEmergency extends Apn {

    // Public methods --------------------------------------------
    public ApnEmergency(Context context, int slotId) {
        super(context, slotId);

        initializeApn();
    }

    // Interface implementation methods --------------------------
    @Override
    public void cleanup() {
        super.cleanup();
    }

    @Override
    public boolean connect() {
        if (!isApnEmployed()) {
            ImsLog.w(mSlotId, "apn is not employed");
            return false;
        }

        if (mAPNState == EApnReqState.APN_REQUEST_DONE) {
            ImsLog.w(mSlotId, "request is already done");
            return true;
        }

        setAPNReqState(EApnReqState.APN_REQUEST_DONE);

        requestNetwork();

        setDataState(TelephonyManager.DATA_CONNECTING);

        return true;
    }

    @Override
    public void disconnect(int nTimeAfterRecover) {
        if (mAPNState != EApnReqState.APN_REQUEST_DONE) {
            ImsLog.w(mSlotId, "request is not done");
            return;
        }

        boolean isPdnConnected = isConnected();
        releaseNetwork();

        setDataState(TelephonyManager.DATA_DISCONNECTED);
        setAPNReqState(EApnReqState.APN_REQUEST_IDLE);

        if (isPdnConnected) {
            sendDataStateUpdateMessage(mType, EDataState.DATA_STATE_DISCONNECTED);
        }
    }

    @Override
    public String getApn() {
        if (mApnString != null) {
            return mApnString;
        }

        return super.getApn();
    }

    // Private/Protected methods ---------------------------------
    protected void initializeApn() {
        mType = EApnType.EMERGENCY;

        registerHandler(EVENT_NETWORK_AVAILABLE,
                new Handle_EVENT_NETWORK_AVAILABLE());
        registerHandler(EVENT_NETWORK_LOST,
                new Handle_EVENT_NETWORK_LOST());
        registerHandler(EVENT_IP_CHANGED,
                new Handle_EVENT_IP_CHANGED());
        registerHandler(EVENT_DATA_CONNECTION_FAILED,
                new Handle_EVENT_DATA_CONNECTION_FAILED());
    }

    private class Handle_EVENT_NETWORK_AVAILABLE implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {

            int curDataState = TelephonyManager.DATA_CONNECTED;

            if (mAPNState != EApnReqState.APN_REQUEST_DONE) {
                ImsLog.w(mSlotId, "apn is not requested, ignore event");
                return;
            }

            if (mDataState != curDataState) {
                IDCUtil dcutil = (IDCUtil) DCFactory.getDC(DCFactory.UTIL, getSlotId());

                if (dcutil != null) {
                    dcutil.updateAllCellInfoForcinglyOnLimitedServiceState();
                }
            }

            if (mDataState != curDataState) {
                ImsLog.i(mSlotId, "data state :: " + mDataState + " >> " + curDataState
                        + ", apn string = " + mApnString + ", network type = " + mNetworkType);

                setDataState(curDataState);

                if (mNetworkType == TelephonyManager.NETWORK_TYPE_UNKNOWN) {
                    updateNetworkType();
                }
                handleIpcanCategory(mNetworkType);

                sendDataStateUpdateMessage(mType, EDataState.DATA_STATE_CONNECTED);
            }
        }
    }

    private class Handle_EVENT_NETWORK_LOST implements MsgProcInterface {
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
                sendDataStateUpdateMessage(mType, EDataState.DATA_STATE_DISCONNECTED);
            }
        }
    }

    private class Handle_EVENT_IP_CHANGED implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            ImsLog.i(mSlotId, "ip is changed");

            if (getDataState() != TelephonyManager.DATA_CONNECTED) {
                return;
            }

            if (!isIPChanged()) {
                ImsLog.i(mSlotId, "ip is changed but ip address is same");
                return;
            }

            sendDataStateUpdateMessage(mType, EDataState.DATA_STATE_IP_CHANGED);
        }
    }

    private class Handle_EVENT_DATA_CONNECTION_FAILED implements MsgProcInterface {
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
                sendDataStateUpdateMessage(mType, EDataState.DATA_STATE_DISCONNECTED);
            }
        }
    }
}
