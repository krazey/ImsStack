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
package com.android.imsstack.core.agents.internal;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.telephony.CellInfo;
import android.telephony.PreciseCallState;
import android.telephony.PreciseDataConnectionState;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;

import com.android.imsstack.core.agents.IPhoneStateNotifier;
import com.android.imsstack.core.agents.ImsPhoneStateListener;

import java.util.List;

public class PhoneStateNotifier implements IPhoneStateNotifier {
    public static interface EventObserver {
        public void onPhoneStateEventChanged(IPhoneStateNotifier notifier,
                int events, int newEvents);
    }

    private final Object mLock = new Object();
    private final PhoneStateHandler mHandler;
    private final EventObserver mEventObserver;
    private ImsPhoneStateListener mListener = null;
    private int mEvents = 0;

    public PhoneStateNotifier(EventObserver observer) {
        mHandler = null;
        mEventObserver = observer;
    }

    public PhoneStateNotifier(Looper looper, EventObserver observer) {
        mHandler = new PhoneStateHandler((looper != null) ? looper : Looper.myLooper());
        mEventObserver = observer;
    }

    @Override
    public int getEvents() {
        synchronized (mLock) {
            return mEvents;
        }
    }

    @Override
    public void setEvents(int events) {
        int oldEvents = getEvents();

        synchronized (mLock) {
            mEvents = events;
        }

        if ((mEventObserver != null) && (oldEvents != events)) {
            int newEvents = (oldEvents ^ events) & events;
            mEventObserver.onPhoneStateEventChanged(this, getEvents(), newEvents);
        }
    }

    @Override
    public void setListener(ImsPhoneStateListener listener) {
        synchronized (mLock) {
            mListener = listener;
        }
    }

    public void notifyCallState(int state, String incomingNumber) {
        if (mHandler == null) {
            onCallStateChanged(state, incomingNumber);
            return;
        }

        Message.obtain(mHandler, ImsPhoneStateListener.LISTEN_CALL_STATE,
                state, 0, incomingNumber).sendToTarget();
    }

    public void notifyCellInfo(List<CellInfo> cellInfo) {
        if (mHandler == null) {
            onCellInfoChanged(cellInfo);
            return;
        }

        Message.obtain(mHandler, ImsPhoneStateListener.LISTEN_CELL_INFO,
                cellInfo).sendToTarget();
    }

    public void notifyPreciseCallState(PreciseCallState callState) {
        if (mHandler == null) {
            onPreciseCallStateChanged(callState);
            return;
        }

        Message.obtain(mHandler, ImsPhoneStateListener.LISTEN_PRECISE_CALL_STATE,
                callState).sendToTarget();
    }

    public void notifyServiceState(ServiceState serviceState) {
        if (mHandler == null) {
            onServiceStateChanged(serviceState);
            return;
        }

        Message.obtain(mHandler, ImsPhoneStateListener.LISTEN_SERVICE_STATE,
                serviceState).sendToTarget();
    }

    public void notifySignalStrengths(SignalStrength signalStrength) {
        if (mHandler == null) {
            onSignalStrengthsChanged(signalStrength);
            return;
        }

        Message.obtain(mHandler, ImsPhoneStateListener.LISTEN_SIGNAL_STRENGTHS,
                signalStrength).sendToTarget();
    }

    public void notifySrvccState(int state) {
        if (mHandler == null) {
            onSrvccStateChanged(state);
            return;
        }

        Message.obtain(mHandler, ImsPhoneStateListener.LISTEN_SRVCC_STATE,
                state, 0).sendToTarget();
    }

    public void notifyPcscfUpdated(List<String> pcscf) {
        if (mHandler == null) {
            onPcscfUpdated(pcscf);
            return;
        }

        Message.obtain(mHandler, ImsPhoneStateListener.LISTEN_PCSCF_ADDRESS_INFO,
                pcscf).sendToTarget();
    }

    public void notifyPreciseDataConnectionState(PreciseDataConnectionState dataConnectionState) {
        if (mHandler == null) {
            onPreciseDataConnectionStateChanged(dataConnectionState);
            return;
        }

        Message.obtain(mHandler, ImsPhoneStateListener.LISTEN_PRECISE_DATA_CONNECTION_STATE,
                dataConnectionState).sendToTarget();
    }

    protected boolean isEventSet(int event) {
        synchronized (mLock) {
            return (mEvents & event) == event;
        }
    }

    /**
     * Invokes when call state is changed.
     */
    protected void onCallStateChanged(int state, String incomingNumber) {
        if (!isEventSet(ImsPhoneStateListener.LISTEN_CALL_STATE)) {
            return;
        }

        ImsPhoneStateListener listener;

        synchronized (mLock) {
            listener = mListener;
        }

        if (listener != null) {
            listener.onCallStateChanged(state, incomingNumber);
        }
    }

    /**
     * Invokes when cell info. is changed.
     */
    protected void onCellInfoChanged(List<CellInfo> cellInfo) {
        if (!isEventSet(ImsPhoneStateListener.LISTEN_CELL_INFO)) {
            return;
        }

        ImsPhoneStateListener listener;

        synchronized (mLock) {
            listener = mListener;
        }

        if (listener != null) {
            listener.onCellInfoChanged(cellInfo);
        }
    }

    /**
     * Invokes when precise call state is changed.
     */
    protected void onPreciseCallStateChanged(PreciseCallState callState) {
        if (!isEventSet(ImsPhoneStateListener.LISTEN_PRECISE_CALL_STATE)) {
            return;
        }

        ImsPhoneStateListener listener;

        synchronized (mLock) {
            listener = mListener;
        }

        if (listener != null) {
            listener.onPreciseCallStateChanged(callState);
        }
    }

    /**
     * Invokes when service state is changed.
     */
    protected void onServiceStateChanged(ServiceState serviceState) {
        if (!isEventSet(ImsPhoneStateListener.LISTEN_SERVICE_STATE)) {
            return;
        }

        ImsPhoneStateListener listener;

        synchronized (mLock) {
            listener = mListener;
        }

        if (listener != null) {
            listener.onServiceStateChanged(serviceState);
        }
    }

    /**
     * Invokes when signal strengths is changed.
     */
    protected void onSignalStrengthsChanged(SignalStrength signalStrength) {
        if (!isEventSet(ImsPhoneStateListener.LISTEN_SIGNAL_STRENGTHS)) {
            return;
        }

        ImsPhoneStateListener listener;

        synchronized (mLock) {
            listener = mListener;
        }

        if (listener != null) {
            listener.onSignalStrengthsChanged(signalStrength);
        }
    }

    /**
     * Invokes when SRVCC state is changed.
     */
    protected void onSrvccStateChanged(int state) {
        if (!isEventSet(ImsPhoneStateListener.LISTEN_SRVCC_STATE)) {
            return;
        }

        ImsPhoneStateListener listener;

        synchronized (mLock) {
            listener = mListener;
        }

        if (listener != null) {
            listener.onSrvccStateChanged(state);
        }
    }

    /**
     * Invokes when P-CSCF addresses are changed.
     */
    protected void onPcscfUpdated(List<String> pcscf) {
        if (!isEventSet(ImsPhoneStateListener.LISTEN_PCSCF_ADDRESS_INFO)) {
            return;
        }

        ImsPhoneStateListener listener;

        synchronized (mLock) {
            listener = mListener;
        }

        if (listener != null) {
            listener.onPcscfUpdated(pcscf);
        }
    }

    /**
     * Invokes when precise data connection state is changed.
     */
    protected void onPreciseDataConnectionStateChanged(
            PreciseDataConnectionState dataConnectionState) {
        if (!isEventSet(ImsPhoneStateListener.LISTEN_PRECISE_DATA_CONNECTION_STATE)) {
            return;
        }

        ImsPhoneStateListener listener;

        synchronized (mLock) {
            listener = mListener;
        }

        if (listener != null) {
            listener.onPreciseDataConnectionStateChanged(dataConnectionState);
        }
    }

    private final class PhoneStateHandler extends Handler {
        public PhoneStateHandler(Looper looper) {
            super(looper);
        }

        @Override
        @SuppressWarnings("unchecked")
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case ImsPhoneStateListener.LISTEN_SERVICE_STATE: {
                onServiceStateChanged((ServiceState)msg.obj);
                break;
            }
            case ImsPhoneStateListener.LISTEN_CALL_STATE: {
                onCallStateChanged(msg.arg1, (String)msg.obj);
                break;
            }
            case ImsPhoneStateListener.LISTEN_PRECISE_CALL_STATE: {
                onPreciseCallStateChanged((PreciseCallState)msg.obj);
                break;
            }
            case ImsPhoneStateListener.LISTEN_SRVCC_STATE: {
                onSrvccStateChanged(msg.arg1);
                break;
            }
            case ImsPhoneStateListener.LISTEN_CELL_INFO: {
                onCellInfoChanged((List<CellInfo>)msg.obj);
                break;
            }
            case ImsPhoneStateListener.LISTEN_SIGNAL_STRENGTHS: {
                onSignalStrengthsChanged((SignalStrength)msg.obj);
                break;
            }
            case ImsPhoneStateListener.LISTEN_PCSCF_ADDRESS_INFO: {
                onPcscfUpdated((List<String>)msg.obj);
                break;
            }
            case ImsPhoneStateListener.LISTEN_PRECISE_DATA_CONNECTION_STATE: {
                onPreciseDataConnectionStateChanged((PreciseDataConnectionState)msg.obj);
                break;
            }
            default:
                break;
            }
        }
    }
}
