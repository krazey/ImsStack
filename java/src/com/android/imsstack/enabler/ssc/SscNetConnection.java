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

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.telephony.TelephonyManager;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.TimerInterface;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.LinkedHashMap;

public class SscNetConnection implements ISscNetConnection {
    protected static final int EVENT_PDN_DATA_STATE_CHANGED = 1001;
    protected static final int EVENT_PDN_REQUEST_TIMEOUT = 1002;
    protected static final int EVENT_PDN_CONNECTION_FAILED = 1003;
    protected static final int EVENT_PDN_CONNECTION_EXPIRED = 1004;

    protected static final int EVENT_PDN_CONNECTED = 2001;
    protected static final int EVENT_PDN_DISCONNECTED = 2002;
    protected static final int EVENT_PDN_IPCAN_CHANGED = 2003;

    protected static final long DISCONNECTION_DELAY = 1000; // 1 sec
    protected static final long PDN_CONNECTION_TIMEOUT_TIMER = 30 * 1000; // 30 sec

    @VisibleForTesting
    protected Handler mSscTransactionHandler = null;
    @VisibleForTesting
    protected Handler mSscNetConnectionHandler = null;

    private final int mSlotId;
    private EApnType mApnType = null;
    @VisibleForTesting
    protected int mConnectionInactivityTimer = 120 * 1000;
    private final IApn.Listener mApnListener = new ApnListener();
    private IDcNetWatcher.Listener mNetWatcherListener;

    @VisibleForTesting
    protected LinkedHashMap<Integer, Long> mTimerIdTable = new LinkedHashMap<Integer, Long>();
    private final TimerInterface.Listener mTimerListener = new TimerInterface.Listener() {
        @Override
        public void onTimerExpired(long tid) {
            if (mSscNetConnectionHandler != null) {
                mSscNetConnectionHandler.post(() -> {
                    mTimerIdTable.forEach((event, timerId) -> {
                        if (tid == timerId) {
                            mSscNetConnectionHandler.sendEmptyMessage(event);
                        }
                    });
                });
            }
        }
    };

    protected SscNetConnection(int slotId) {
        mSlotId = slotId;
    }

    @Override
    public void init(EApnType apnType) {
        mApnType = apnType;

        HandlerThread handlerThread = new HandlerThread("SscNetConnectionHandler");
        handlerThread.start();
        mSscNetConnectionHandler = new SscNetConnectionHandler(handlerThread.getLooper());

        if (mNetWatcherListener == null) {
            IDcNetWatcher dnw = DcFactory.getDcAgent(IDcNetWatcher.class, mSlotId);
            if (dnw != null) {
                mNetWatcherListener = new NetWatcherListener();
                dnw.addListener(mNetWatcherListener);
            }
        }

        if (mApnType != null) {
            IDcApn dcApn = DcFactory.getDcAgent(IDcApn.class, mSlotId);
            IApn apn = (dcApn != null) ? dcApn.getApnControl(mApnType.getType()) : null;
            if (apn != null) {
                apn.addListener(mApnListener);
            }
        }

        mConnectionInactivityTimer = SscConfig.getXcapApnInactivityTimer(mSlotId);
    }

    @Override
    public void cleanup() {
        ImsLog.d(mSlotId, "cleanup");
        disconnect();

        if (mApnType != null) {
            IDcApn dcApn = DcFactory.getDcAgent(IDcApn.class, mSlotId);
            IApn apn = (dcApn != null) ? dcApn.getApnControl(mApnType.getType()) : null;
            if (apn != null) {
                apn.removeListener(mApnListener);
            }
        }

        if (mNetWatcherListener != null) {
            IDcNetWatcher dnw = DcFactory.getDcAgent(IDcNetWatcher.class, mSlotId);
            if (dnw != null) {
                dnw.removeListener(mNetWatcherListener);
            }
            mNetWatcherListener = null;
        }

        if (mSscNetConnectionHandler != null) {
            mSscNetConnectionHandler.removeCallbacksAndMessages(null);
            mSscNetConnectionHandler.getLooper().quit();
            mSscNetConnectionHandler = null;
        }
    }

    @Override
    public boolean isConnected() {
        ImsLog.d(mSlotId, "");

        if (mApnType == null) {
            ImsLog.e(mSlotId, "mApnType is null");
            return false;
        }

        int dataState;
        IDcApn dcApn = DcFactory.getDcAgent(IDcApn.class, mSlotId);
        if (dcApn != null) {
            dataState = dcApn.getDataState(mApnType.getType());
        } else {
            dataState = EDataState.convertFromTMtoImsType(TelephonyManager.DATA_DISCONNECTED);
        }

        EDataState eDataState = EDataState.convertIntTypeToEnum(dataState);
        ImsLog.i(mSlotId, "ApnType : " + mApnType.getType()  + ", DataState : " + eDataState
                + ", NetworkTye : " + getNetworkType());

        return eDataState == EDataState.DATA_STATE_CONNECTED;
    }

    @Override
    public boolean connect() {
        ImsLog.d(mSlotId, "");

        if (mApnType == null) {
            ImsLog.e(mSlotId, "mApnType is null");
            return false;
        }

        if (isConnected()) {
            ImsLog.e(mSlotId, "PDN is already connected");
            return true;
        }

        IDcApn dcApn = DcFactory.getDcAgent(IDcApn.class, mSlotId);
        if (dcApn == null) {
            ImsLog.e(mSlotId, "dcApn is null");
            return false;
        }

        if (!dcApn.connect(mApnType.getType())) {
            return false;
        }

        startTimer(EVENT_PDN_REQUEST_TIMEOUT, PDN_CONNECTION_TIMEOUT_TIMER);
        return true;
    }

    @Override
    public void disconnect() {
        ImsLog.d(mSlotId, "");

        stopTimer(EVENT_PDN_REQUEST_TIMEOUT);
        stopTimer(EVENT_PDN_CONNECTION_EXPIRED);

        if (mApnType != EApnType.XCAP) {
            ImsLog.e(mSlotId, "Don't need to disconnect unless XCAP APN");
            return;
        }

        IDcApn dcApn = DcFactory.getDcAgent(IDcApn.class, mSlotId);
        if (dcApn != null) {
            dcApn.disconnect(mApnType.getType());
        }
    }

    @Override
    public int getNetworkType() {
        int ipcanCategory = IApn.IPCAN_CATEGORY_MOBILE;

        IDcApn dcApn = DcFactory.getDcAgent(IDcApn.class, mSlotId);
        if (dcApn != null) {
            ipcanCategory = dcApn.getIpcanCategory(mApnType.getType());
        }

        int networkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        if (ipcanCategory == IApn.IPCAN_CATEGORY_WLAN) {
            networkType = TelephonyManager.NETWORK_TYPE_IWLAN;
        } else {
            IDcNetWatcher dnw = DcFactory.getDcAgent(IDcNetWatcher.class, mSlotId);
            if (dnw != null) {
                networkType = dnw.getNetworkType();
            }
        }

        return networkType;
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
    protected TimerInterface getTimerInterface() {
        return AgentFactory.getInstance().getAgent(TimerInterface.class);
    }

    private void startTimer(int eventNum, long duration) {
        ImsLog.d(mSlotId, "EventNumber : " + eventNum + ", Time : " + duration);

        TimerInterface timer = getTimerInterface();
        if (timer == null) {
            ImsLog.e(mSlotId, "TimerInterface is null");
            return;
        }

        long timerId = timer.startTimer(duration, mTimerListener);

        if (timerId == TimerInterface.INVALID_TID) {
            ImsLog.e(mSlotId, "Starting a timer failed");
            return;
        }

        if (mTimerIdTable.containsKey(eventNum)) {
            ImsLog.d(mSlotId, "Restart Timer with Event " + eventNum);
            stopTimer(eventNum);
            mTimerIdTable.remove(eventNum);
        }

        mTimerIdTable.put(eventNum, timerId);

        ImsLog.d(mSlotId, eventNum + " timer is started - tid[" + timerId + "], duration["
                + duration + "]");
    }

    private void stopTimer(int eventNum) {
        ImsLog.d(mSlotId, "EventNumber : " + eventNum);

        Long timerId = mTimerIdTable.get(eventNum);
        if (timerId == null) {
            return;
        }

        TimerInterface timer = getTimerInterface();
        if (timer != null) {
            timer.stopTimer(timerId);
        }
        mTimerIdTable.remove(eventNum);
    }

    private final class ApnListener implements IApn.Listener {
        @Override
        public void onIpcanCategoryChanged(int apnType, int ipcanCategory) {
            ImsLog.d(mSlotId, "IPCAN changed : " + ipcanCategory);
            if (mSscTransactionHandler != null) {
                mSscTransactionHandler.sendEmptyMessage(EVENT_PDN_IPCAN_CHANGED);
            }
        }
    }

    private final class NetWatcherListener implements IDcNetWatcher.Listener {
        @Override
        public void onDataConnectionStateChanged(EApnType apnType, EDataState dataState) {
            if (mSscTransactionHandler != null) {
                Message.obtain(mSscNetConnectionHandler, EVENT_PDN_DATA_STATE_CHANGED,
                        apnType.getType(), dataState.getState()).sendToTarget();
            }
        }

        @Override
        public void onPdnConnectionFailed(EApnType apnType, int smCause) {
            if (mSscTransactionHandler != null) {
                Message.obtain(mSscNetConnectionHandler, EVENT_PDN_CONNECTION_FAILED,
                    apnType.getType(), smCause).sendToTarget();
            }
        }
    }

    private final class SscNetConnectionHandler extends Handler {
        SscNetConnectionHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            if (msg == null) {
                ImsLog.e(mSlotId, "msg is null");
                return;
            }

            ImsLog.d(mSlotId, "received a message[" + msg.what + "]");

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
            if (mApnType == null || mApnType.getType() != msg.arg1) {
                return;
            }
            EDataState dataState = EDataState.convertIntTypeToEnum(msg.arg2);
            ImsLog.i(mSlotId, "apnType[" + mApnType + "], state[" + dataState + "]");

            if (dataState == EDataState.DATA_STATE_CONNECTED) {
                stopTimer(EVENT_PDN_REQUEST_TIMEOUT);
                startTimer(EVENT_PDN_CONNECTION_EXPIRED, mConnectionInactivityTimer);
                if (mSscTransactionHandler != null) {
                    mSscTransactionHandler.sendEmptyMessage(EVENT_PDN_CONNECTED);
                }
            } else if (dataState == EDataState.DATA_STATE_DISCONNECTED) {
                startTimer(EVENT_PDN_CONNECTION_EXPIRED, DISCONNECTION_DELAY);
                if (mSscTransactionHandler != null) {
                    mSscTransactionHandler.sendEmptyMessage(EVENT_PDN_DISCONNECTED);
                }
            }
        }

        private void proc_connectionFailed(Message msg) {
            if (mApnType == EApnType.XCAP && msg.arg1 == EApnType.XCAP.getType()) {
                ImsLog.d(mSlotId, "smCause : " + msg.arg2);

                SscServiceStateAgent.getInstance().setPdnConnectionFailed(mSlotId, msg.arg2);

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
