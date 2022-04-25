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

import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.agentif.IAlarmTimer;
import com.android.imsstack.core.agents.dcm.DCFactory;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDCApn;
import com.android.imsstack.core.agents.dcmif.IDCNetWatcher;
import com.android.imsstack.enabler.ssc.SscConfig;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;

import java.util.Hashtable;
import java.util.List;

public class SscNetConnection implements ISscNetConnection {
    // Constants--------------------------------------------------
    protected static final int EVENT_DATA_STATE_CHANGED = 1001;
    protected static final int EVENT_CONNECION_TIMER_EXPIRED = 1002;

    protected static final long DISCONNECTION_TIMER_VALUE = 1000;

    // Variables--------------------------------------------------
    protected Handler mSscTransactionHandler = null;
    protected Handler mSscNetConnectionHandler = null;

    protected Context mContext = null;
    protected int mSlotId  = -1;

    protected EDataState mDataState = EDataState.DATA_STATE_DISCONNECTED;
    protected EApnType mApnType = null;
    protected long mConnectionTimer = 120 * 1000;

    // Hashtabale to store alarmTimer Id
    private Hashtable<Integer, Integer>timerIdTable = new Hashtable<Integer, Integer>();

    public SscNetConnection(int slotId) {
        mSlotId = slotId;
    }

    // Static loading materials ----------------------------------
    // Public methods --------------------------------------------
    @Override
    public void init(Context context, EApnType apnType) {
        mContext = context;
        if (mContext == null) {
            ImsLog.e("Context is null");
            return;
        }

        mApnType = apnType;

        HandlerThread handlerThread = new HandlerThread("HandlerName");
        handlerThread.start();

        mSscNetConnectionHandler = new SscNetConnectionHandler(handlerThread.getLooper());

        IDCNetWatcher netWatcher
                = (IDCNetWatcher)DCFactory.getDC(DCFactory.NETWORK_WATCHER, mSlotId);
        if (netWatcher != null) {
            netWatcher.registerForDataStateChanged(
                    mSscNetConnectionHandler, EVENT_DATA_STATE_CHANGED, null);
        }

        mConnectionTimer = SscConfig.getXcapApnInactivityTimer(mSlotId);
    }

    @Override
    public void cleanup(Context recentCnx) {
        IDCNetWatcher netWatcher
                = (IDCNetWatcher)DCFactory.getDC(DCFactory.NETWORK_WATCHER, mSlotId);
        if (netWatcher != null) {
            netWatcher.unregisterForDataServiceStateChanged(mSscNetConnectionHandler);
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
        }
        else {
            dataState = EDataState.convertFromTMtoImsType(TelephonyManager.DATA_DISCONNECTED);
        }
        mDataState = EDataState.convertIntTypeToEnum(dataState);

        ImsLog.i("SlotId : " + mSlotId
                + ", ApnType : " + mApnType.getType()
                + ", DataState : " + dataState + "/" + mDataState);

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

        if (dcGovApnCtrl.connect(mApnType.getType(), IApn.IPCAN_CATEGORY_MOBILE)) {
            // Timer value can be set as db item.
            startTimer(mConnectionTimer, EVENT_CONNECION_TIMER_EXPIRED);
            return true;
        }
        else {
            return false;
        }
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

        if (mApnType == null) {
            ImsLog.e("mApnType is null");
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
        startTimer(mConnectionTimer, EVENT_CONNECION_TIMER_EXPIRED);
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

        if (timerIdTable == null) {
            ImsLog.e("timerIdTable is null");
            return;
        }

        // If same event time is exist,
        // 1) stop that event timer and remove it from table
        if(timerIdTable.containsKey(eventNum)) {
            ImsLog.i("Restart Timer with Event " + eventNum);
            stopTimer(true, eventNum);
            timerIdTable.remove(eventNum);
        }

        // 2) start event timer with new value
        atm.registerForTimerExpired(timerId, mSscNetConnectionHandler, eventNum, null);

        timerIdTable.put(eventNum, timerId);
        if (!atm.startTimer(timerId, duration)) {
            stopTimer(false, eventNum);
            ImsLog.e(" Starting a validity timer failed");
            return;
        }

        ImsLog.i(eventNum + " timer is started :: tid[" + timerId + "], duration[" + duration + "]");
    }

    private void stopTimer(boolean stopRequired, int eventNum) {
        ImsLog.d("EventNumber : " + eventNum);

        Integer timerId = -1;

        if (timerIdTable == null) {
            ImsLog.e("timerIdTable is null");
            return;
        }

        timerId = timerIdTable.get(eventNum);

        if (timerId == null) {
            return;
        }

        if (timerId <= 0) {
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

        timerIdTable.put(eventNum, -1);
    }

    protected SscNetConnection() {
    }


    // Interface implementation methods --------------------------
    // Private/Protected methods ---------------------------------
    //-------------------------------------------------------------
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
            case EVENT_DATA_STATE_CHANGED:
                proc_dataStateChanged(msg);
                break;
            case EVENT_CONNECION_TIMER_EXPIRED:
                proc_connectionTimerExpired();
                break;
            default:
                break;
            }
        }

        private void proc_dataStateChanged(Message msg) {
            AsyncResult ar = (AsyncResult)msg.obj;
            if (ar == null) {
                return;
            }

            IDCNetWatcher.NotiObj res = (IDCNetWatcher.NotiObj)ar.result;
            if (res == null) {
                return;
            }

            EApnType apnType = res.eApnType;
            EDataState dataState = res.eDataState;

            if (mApnType == null) {
                ImsLog.e("mApnType is null");
                return;
            }

            if (mSscTransactionHandler == null) {
                ImsLog.e("mSscTransactionHandler is null");
                return;
            }

            ImsLog.i("apnType[" + apnType + "], state[" + dataState + "]");

            if (apnType == mApnType) {
                mDataState = dataState;
                // If the handler is needed...
                if (dataState == EDataState.DATA_STATE_CONNECTED) {
                    // notify data connected event only
                    mSscTransactionHandler.sendEmptyMessage(EVENT_DATA_STATE_CHANGED);
                } else if (dataState == EDataState.DATA_STATE_DISCONNECTED) {
                    startTimer(DISCONNECTION_TIMER_VALUE, EVENT_CONNECION_TIMER_EXPIRED);
                }
            } else {
                ImsLog.e("apnType(" + mApnType + ") is not matched : [" + apnType + "]");
            }
        }

        private void proc_connectionTimerExpired() {
            disconnect();
        }
    }
}
