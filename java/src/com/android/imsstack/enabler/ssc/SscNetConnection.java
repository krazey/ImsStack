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

import android.os.AsyncResult;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.telephony.TelephonyManager;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.agentif.IAlarmTimer;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Hashtable;

public class SscNetConnection implements ISscNetConnection {
    protected static final int EVENT_PDN_DATA_STATE_CHANGED = 1001;
    protected static final int EVENT_PDN_REQUEST_TIMEOUT = 1002;
    protected static final int EVENT_PDN_CONNECTION_FAILED = 1003;
    protected static final int EVENT_PDN_CONNECTION_EXPIRED = 1004;

    protected static final int EVENT_PDN_CONNECTED = 2001;
    protected static final int EVENT_PDN_DISCONNECTED = 2002;

    protected static final long DISCONNECTION_DELAY = 1000; // 1 sec
    protected static final long PDN_CONNECTION_TIMEOUT_TIMER = 30 * 1000; // 30 sec

    @VisibleForTesting
    protected Handler mSscTransactionHandler = null;
    @VisibleForTesting
    protected Handler mSscNetConnectionHandler = null;

    private final int mSlotId;
    private EApnType mApnType = null;
    @VisibleForTesting
    protected long mConnectionInactivityTimer = 120 * 1000;

    @VisibleForTesting
    protected Hashtable<Integer, Integer> mTimerIdTable = new Hashtable<Integer, Integer>();

    protected SscNetConnection(int slotId) {
        mSlotId = slotId;
    }

    @Override
    public void init(EApnType apnType) {
        mApnType = apnType;

        HandlerThread handlerThread = new HandlerThread("SscNetConnectionHandler");
        handlerThread.start();
        mSscNetConnectionHandler = new SscNetConnectionHandler(handlerThread.getLooper());

        IDcNetWatcher dnw = (IDcNetWatcher) DcFactory.getDc(DcFactory.NETWORK_WATCHER, mSlotId);
        if (dnw != null) {
            dnw.registerForDataStateChanged(mSscNetConnectionHandler,
                    EVENT_PDN_DATA_STATE_CHANGED, null);
            dnw.registerForPdnConnectionFailed(mSscNetConnectionHandler,
                    EVENT_PDN_CONNECTION_FAILED, null);
        }

        mConnectionInactivityTimer = SscConfig.getXcapApnInactivityTimer(mSlotId);
    }

    @Override
    public void cleanup() {
        disconnect();

        if (mSscNetConnectionHandler != null) {
            IDcNetWatcher dnw = (IDcNetWatcher) DcFactory.getDc(DcFactory.NETWORK_WATCHER, mSlotId);
            if (dnw != null) {
                dnw.unregisterForDataServiceStateChanged(mSscNetConnectionHandler);
                dnw.unregisterForPdnConnectionFailed(mSscNetConnectionHandler);
            }

            mSscNetConnectionHandler.getLooper().quit();
            mSscNetConnectionHandler = null;
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
        IDcApn dcGovApnCtrl = (IDcApn) DcFactory.getDc(DcFactory.APN, mSlotId);
        if (dcGovApnCtrl != null) {
            dataState = dcGovApnCtrl.getDataState(mApnType.getType());
        } else {
            dataState = EDataState.convertFromTMtoImsType(TelephonyManager.DATA_DISCONNECTED);
        }

        EDataState eDataState = EDataState.convertIntTypeToEnum(dataState);
        ImsLog.i("SlotId : " + mSlotId + ", ApnType : " + mApnType.getType()  + ", DataState : "
                + dataState + "/" + eDataState);

        if (EDataState.DATA_STATE_CONNECTED == eDataState) {
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

        IDcApn dcGovApnCtrl = (IDcApn) DcFactory.getDc(DcFactory.APN, mSlotId);
        if (dcGovApnCtrl == null) {
            ImsLog.e("dcGovApnCtrl is null");
            return false;
        }

        if (!dcGovApnCtrl.connect(mApnType.getType())) {
            return false;
        }

        startTimer(EVENT_PDN_REQUEST_TIMEOUT, PDN_CONNECTION_TIMEOUT_TIMER);
        return true;
    }

    @Override
    public boolean isPdnAvailable() {
        ImsLog.d("");

        if (!MSimUtils.isMultiSimEnabled()) {
            return true;
        }

        /* TODO: Need to re-request after disconnection of other slot's connection
        if (mApnType == null || !mApnType.equals(EApnType.XCAP)) {
            ImsLog.d(mSlotId, "it's not XCAP, PDN available");
            return true;
        }

        // active modem count?
        int otherSlotId = mSlotId == 0 ? 1 : 0;
        IDcApn dcGovApnCtrl = (IDcApn) DcFactory.getDc(DcFactory.APN, otherSlotId);
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
        */
        return true;
    }

    @Override
    public void disconnect() {
        ImsLog.d("");

        stopTimer(EVENT_PDN_REQUEST_TIMEOUT);
        stopTimer(EVENT_PDN_CONNECTION_EXPIRED);

        if (mApnType != EApnType.XCAP) {
            ImsLog.e("Don't need to disconnect unless XCAP APN");
            return;
        }

        IDcApn dcGovApnCtrl = (IDcApn) DcFactory.getDc(DcFactory.APN, mSlotId);
        if (dcGovApnCtrl != null) {
            dcGovApnCtrl.disconnect(mApnType.getType());
        }
    }

    @Override
    public void setCallbackHandler(Handler handler) {
        mSscTransactionHandler = handler;
    }

    @Override
    public void refreshConnectionTimer() {
        ImsLog.d(mSlotId, "");
        startTimer(EVENT_PDN_CONNECTION_EXPIRED, mConnectionInactivityTimer);
    }

    @VisibleForTesting
    protected IAlarmTimer getAlarmTimer() {
        return (IAlarmTimer) AgentFactory.getAgent(AgentFactory.ALARM_TIMER, mSlotId);
    }

    private void startTimer(int eventNum, long duration) {
        ImsLog.d("EventNumber : " + eventNum + ", Time : " + duration);

        IAlarmTimer atm = getAlarmTimer();
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
            ImsLog.d("Restart Timer with Event " + eventNum);
            stopTimer(eventNum);
            mTimerIdTable.remove(eventNum);
        }

        atm.registerForTimerExpired(timerId, mSscNetConnectionHandler, eventNum, null);

        if (!atm.startTimer(timerId, duration)) {
            atm.unregisterForTimerExpired(timerId, mSscNetConnectionHandler);
            ImsLog.e("Starting a timer failed");
            return;
        }

        mTimerIdTable.put(eventNum, timerId);

        ImsLog.d(eventNum + " timer is started - tid[" + timerId + "], duration[" + duration + "]");
    }

    private void stopTimer(int eventNum) {
        ImsLog.d("EventNumber : " + eventNum);

        Integer timerId = mTimerIdTable.get(eventNum);
        if (timerId == null) {
            return;
        }

        IAlarmTimer atm = getAlarmTimer();
        if (atm == null) {
            ImsLog.e(" AlamTimerManager is null");
            return;
        }

        atm.stopTimer(timerId);
        atm.unregisterForTimerExpired(timerId, mSscNetConnectionHandler);
        mTimerIdTable.remove(eventNum);
    }

    private final class SscNetConnectionHandler extends Handler {
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
                case EVENT_PDN_REQUEST_TIMEOUT:
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

            IDcNetWatcher.NotiObj res = (IDcNetWatcher.NotiObj) ar.result;
            if (res == null) {
                return;
            }

            EApnType apnType = res.eApnType;
            if (mApnType == null || mApnType != apnType) {
                ImsLog.e("apnType(" + mApnType + ") is not matched : [" + apnType + "]");
                return;
            }

            ImsLog.i("apnType[" + apnType + "], state[" + res.eDataState + "]");
            EDataState eDataState = res.eDataState;
            if (eDataState == EDataState.DATA_STATE_CONNECTED) {
                stopTimer(EVENT_PDN_REQUEST_TIMEOUT);
                startTimer(EVENT_PDN_CONNECTION_EXPIRED, mConnectionInactivityTimer);
                if (mSscTransactionHandler != null) {
                    mSscTransactionHandler.sendEmptyMessage(EVENT_PDN_CONNECTED);
                }
            } else if (eDataState == EDataState.DATA_STATE_DISCONNECTED) {
                startTimer(EVENT_PDN_CONNECTION_EXPIRED, DISCONNECTION_DELAY);
                if (mSscTransactionHandler != null) {
                    mSscTransactionHandler.sendEmptyMessage(EVENT_PDN_DISCONNECTED);
                }
            }
        }

        private void proc_connectionFailed(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            if (ar == null) {
                return;
            }

            IDcNetWatcher.NotiObj res = (IDcNetWatcher.NotiObj) ar.result;
            if (res == null) {
                return;
            }

            if (mApnType == EApnType.XCAP && res.eApnType == EApnType.XCAP) {
                int smCause = res.mSmCause;
                SscServiceStateAgent.getInstance().setPdnConnectionFailed(mSlotId, smCause);
                ImsLog.d("smCause : " + smCause);

                if (mSscTransactionHandler != null) {
                    mSscTransactionHandler.sendEmptyMessage(EVENT_PDN_CONNECTION_FAILED);
                }

                disconnect();
            }
        }

        private void proc_connectionTimeout() {
            SscServiceStateAgent.getInstance().setPdnConnectionTimeout(mSlotId, true);
            if (mSscTransactionHandler != null) {
                mSscTransactionHandler.sendEmptyMessage(EVENT_PDN_REQUEST_TIMEOUT);
            }

            disconnect();
        }

        private void proc_connectionTimerExpired() {
            disconnect();
        }
    }
}
