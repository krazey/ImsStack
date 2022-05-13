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

package com.android.imsstack.enabler.ssc;

import android.content.Context;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.telephony.TelephonyManager;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.agentif.IAlarmTimer;
import com.android.imsstack.core.agents.dcm.DCFactory;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDCApn;
import com.android.imsstack.core.agents.dcmif.IDCNetWatcher;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;

import java.util.Hashtable;

public class SscNetConnection implements ISscNetConnection {
    protected static final int EVENT_PDN_DATA_STATE_CHANGED = 1001;
    protected static final int EVENT_PDN_CONNECTION_FAILED = 1002;
    protected static final int EVENT_PDN_CONNECTION_TIMEOUT = 1003;
    protected static final int EVENT_PDN_CONNECTION_EXPIRED = 1004;

    protected static final long DISCONNECTION_DELAY = 1000; // 1 sec
    protected static final long PDN_CONNECTION_TIMEOUT_TIMER = 30 * 1000; // 30 sec

    protected Handler mSscTransactionHandler = null;
    protected Handler mSscNetConnectionHandler = null;

    protected Context mContext = null;
    protected int mSlotId  = -1;

    protected EDataState mDataState = EDataState.DATA_STATE_DISCONNECTED;
    protected EApnType mApnType = null;
    protected long mConnectionInactivityTimer = 120 * 1000;

    private Hashtable<Integer, Integer> mTimerIdTable = new Hashtable<Integer, Integer>();

    public SscNetConnection(int slotId) {
        mSlotId = slotId;
    }

    @Override
    public void init(Context context, EApnType apnType) {
        if (context == null) {
            ImsLog.e("Context is null");
            return;
        }

        mContext = context;
        mApnType = apnType;

        HandlerThread handlerThread = new HandlerThread("SscNetConnectionHandler");
        handlerThread.start();
        mSscNetConnectionHandler = new SscNetConnectionHandler(handlerThread.getLooper());

        IDCNetWatcher dnw = (IDCNetWatcher) DCFactory.getDC(DCFactory.NETWORK_WATCHER, mSlotId);
        if (dnw != null) {
            dnw.registerForDataStateChanged(mSscNetConnectionHandler,
                    EVENT_PDN_DATA_STATE_CHANGED, null);
            dnw.registerForPdnConnectionFailed(mSscNetConnectionHandler,
                    EVENT_PDN_CONNECTION_FAILED, null);
        }

        mConnectionInactivityTimer = SscConfig.getXcapApnInactivityTimer(mSlotId);
    }

    @Override
    public void cleanup(Context recentCnx) {
        IDCNetWatcher dnw
                = (IDCNetWatcher)DCFactory.getDC(DCFactory.NETWORK_WATCHER, mSlotId);
        if (dnw != null) {
            dnw.unregisterForDataServiceStateChanged(mSscNetConnectionHandler);
            dnw.unregisterForPdnConnectionFailed(mSscNetConnectionHandler);
        }
    }

    @Override
    public boolean isConnected() {
        ImsLog.d("");

        if (mApnType == null) {
            ImsLog.e("mApnType is null");
            return false;
        }

        int dataState;
        IDCApn dcGovApnCtrl = (IDCApn)DCFactory.getDC(DCFactory.APN, mSlotId);
        if (dcGovApnCtrl != null) {
            dataState = dcGovApnCtrl.getDataState(mApnType.getType());
        } else {
            dataState = EDataState.convertFromTMtoImsType(TelephonyManager.DATA_DISCONNECTED);
        }

        mDataState = EDataState.convertIntTypeToEnum(dataState);
        ImsLog.i("SlotId : " + mSlotId + ", ApnType : " + mApnType.getType()  + ", DataState : "
                + dataState + "/" + mDataState);

        if (EDataState.DATA_STATE_CONNECTED == mDataState) {
            return true;
        }

        return false;
    }

    @Override
    public boolean connect() {
        ImsLog.d("");

        if (mApnType == null) {
            ImsLog.e("mApnType is null");
            return false;
        }

        if (isConnected()) {
            ImsLog.e("PDN is already connected");
            return true;
        }

        IDCApn dcGovApnCtrl = (IDCApn)DCFactory.getDC(DCFactory.APN, mSlotId);
        if (dcGovApnCtrl == null) {
            ImsLog.e("dcGovApnCtrl is null");
            return false;
        }

        if (!dcGovApnCtrl.connect(mApnType.getType(), IApn.IPCAN_CATEGORY_MOBILE)) {
            return false;
        }

        startTimer(PDN_CONNECTION_TIMEOUT_TIMER, EVENT_PDN_CONNECTION_TIMEOUT);
        return true;
    }

    public boolean isPDNAvailable() {
        ImsLog.d("");

        if (!MSimUtils.isMultiSimEnabled()) {
            return true;
        }
        if (mApnType == null || !mApnType.equals(EApnType.XCAP) ) {
            ImsLog.d(mSlotId, "it's not XCAP, PDN available");
            return true;
        }

        int otherSlotId = mSlotId == 0 ? 1 : 0;
        IDCApn dcGovApnCtrl = (IDCApn)DCFactory.getDC(DCFactory.APN, otherSlotId);
        if (dcGovApnCtrl == null) {
            ImsLog.e(mSlotId, "dcGovApnCtrl for other slot is null, PDN available");
            return true;
        }

        IApn apnCtrl = dcGovApnCtrl.getApnControl(mApnType.getType());
        if (apnCtrl == null) {
            ImsLog.e(mSlotId, "apnCtrl for other slot is null, PDN available");
            return true;
        }

        if (SscConfig.isPdnConnCheckedByDataState(otherSlotId)) {
            return apnCtrl.getDataState() != TelephonyManager.DATA_CONNECTED;
        } else if (apnCtrl.isConnected()) {
            ImsLog.e(mSlotId, "XCAP PDN for other slot is connected, PDN not available");
            return false;
        }

        return true;
    }

    @Override
    public void disconnect() {
        ImsLog.d("");

        stopTimer(true, EVENT_PDN_CONNECTION_TIMEOUT);
        stopTimer(true, EVENT_PDN_CONNECTION_EXPIRED);

        if (mApnType != EApnType.XCAP) {
            ImsLog.e("Don't need to disconnect unless XCAP APN");
            return;
        }

        IDCApn dcGovApnCtrl = (IDCApn)DCFactory.getDC(DCFactory.APN, mSlotId);
        if (dcGovApnCtrl != null) {
            dcGovApnCtrl.disconnect(mApnType.getType(), IApn.DEFAULT_TIME_FOR_RECOVERY,
                IApn.IPCAN_CATEGORY_MOBILE);
        }
    }

    @Override
    public void setTransactionHandler(Handler handler) {
        mSscTransactionHandler = handler;
    }

    @Override
    public void refreshConnectionTimer() {
        ImsLog.d(mSlotId, "");
        startTimer(mConnectionInactivityTimer, EVENT_PDN_CONNECTION_EXPIRED);
    }

    private void startTimer(long duration, int eventNum) {
        ImsLog.d("EventNumber : " + eventNum + ", Time : " + duration);

        IAlarmTimer atm = (IAlarmTimer)AgentFactory.getAgent(AgentFactory.ALARM_TIMER, mSlotId);
        if (atm == null) {
            ImsLog.e("AlamTimerManager is null");
            return;
        }

        int timerId = atm.getTimerId();
        if (timerId <= 0) {
            ImsLog.e("Retry timer id is invalid");
            return;
        }

        if (mTimerIdTable.containsKey(eventNum)) {
            ImsLog.i("Restart Timer with Event " + eventNum);
            stopTimer(true, eventNum);
            mTimerIdTable.remove(eventNum);
        }

        atm.registerForTimerExpired(timerId, mSscNetConnectionHandler, eventNum, null);

        mTimerIdTable.put(eventNum, timerId);
        if (!atm.startTimer(timerId, duration)) {
            stopTimer(false, eventNum);
            ImsLog.e("Starting a timer failed");
            return;
        }

        ImsLog.i(eventNum + " timer is started :: tid[" + timerId + "], duration[" + duration + "]");
    }

    private void stopTimer(boolean stopRequired, int eventNum) {
        ImsLog.d("EventNumber : " + eventNum);

        Integer timerId = mTimerIdTable.get(eventNum);
        if (timerId == null || timerId <= 0) {
            return;
        }

        IAlarmTimer atm = (IAlarmTimer)AgentFactory.getAgent(AgentFactory.ALARM_TIMER, mSlotId);
        if (atm == null) {
            ImsLog.e(" AlamTimerManager is null");
            return;
        }

        if (stopRequired) {
            atm.stopTimer(timerId);
        }

        atm.unregisterForTimerExpired(timerId, mSscNetConnectionHandler);
        mTimerIdTable.remove(eventNum);
    }

    protected SscNetConnection() {
    }

    protected final class SscNetConnectionHandler extends Handler {
        public SscNetConnectionHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            if (msg == null) {
                ImsLog.e("msg is null");
                return;
            }

            ImsLog.d("received a message[" + msg.what + "]");

            switch (msg.what) {
                case EVENT_PDN_DATA_STATE_CHANGED:
                    proc_dataStateChanged(msg);
                    break;
                case EVENT_PDN_CONNECTION_FAILED:
                    proc_connectionFailed(msg);
                    break;
                case EVENT_PDN_CONNECTION_TIMEOUT:
                    proc_connectionTimeout();
                    break;
                case EVENT_PDN_CONNECTION_EXPIRED:
                    proc_connectionTimerExpired();
                    break;
                default:
                    break;
            }
        }

        private void proc_dataStateChanged(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            if (ar == null) {
                return;
            }

            IDCNetWatcher.NotiObj res = (IDCNetWatcher.NotiObj) ar.result;
            if (res == null) {
                return;
            }

            EApnType apnType = res.eApnType;
            if (mApnType == null || mApnType != apnType) {
                ImsLog.e("apnType(" + mApnType + ") is not matched : [" + apnType + "]");
                return;
            }

            ImsLog.i("apnType[" + apnType + "], state[" + res.eDataState + "]");
            mDataState = res.eDataState;
            if (mDataState == EDataState.DATA_STATE_CONNECTED) {
                stopTimer(true, EVENT_PDN_CONNECTION_TIMEOUT);
                startTimer(mConnectionInactivityTimer, EVENT_PDN_CONNECTION_EXPIRED);
                if (mSscTransactionHandler != null) {
                    mSscTransactionHandler.sendEmptyMessage(EVENT_PDN_DATA_STATE_CHANGED);
                }
            } else if (mDataState == EDataState.DATA_STATE_DISCONNECTED) {
                startTimer(DISCONNECTION_DELAY, EVENT_PDN_CONNECTION_EXPIRED);
            }
        }

        private void proc_connectionFailed(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            if (ar == null) {
                return;
            }

            EApnType apnType = (EApnType) ar.result;
            if (apnType == null) {
                return;
            }

            ImsLog.d("ApnType : " + apnType.toString());
            // Change only for XCAP apn type.
            if (apnType == EApnType.XCAP) {
                IDCApn dcapn = (IDCApn) DCFactory.getDC(DCFactory.APN, mSlotId);
                IApn apn = (dcapn != null) ? dcapn.getApnControl(apnType.getType()) : null;
                boolean isPermanentFailure = (apn != null)
                        ? apn.isESMCausePermanentFailure() : false;
                // TODO: Need to check KEY_UT_SM_CAUSE_TEMPORARY_BLOCK_INT_ARRAY
                SscServiceStateAgent.getInstance().setPdnConnectionFailed(mSlotId,
                        isPermanentFailure);
                if (mSscTransactionHandler != null) {
                    mSscTransactionHandler.sendEmptyMessage(EVENT_PDN_CONNECTION_FAILED);
                }
                disconnect();
            }
        }

        private void proc_connectionTimeout() {
            SscServiceStateAgent.getInstance().setPdnConnectionTimeout(mSlotId, true);
            if (mSscTransactionHandler != null) {
                mSscTransactionHandler.sendEmptyMessage(EVENT_PDN_CONNECTION_TIMEOUT);
            }
            disconnect();
        }

        private void proc_connectionTimerExpired() {
            disconnect();
        }
    }
}
