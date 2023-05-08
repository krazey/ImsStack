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

import android.annotation.NonNull;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.telephony.Annotation.CallState;
import android.telephony.Annotation.SrvccState;
import android.telephony.BarringInfo;
import android.telephony.CellInfo;
import android.telephony.PreciseCallState;
import android.telephony.PreciseDataConnectionState;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;

import com.android.imsstack.core.agents.IPhoneStateNotifier;
import com.android.imsstack.core.agents.ImsPhoneStateListener;

import java.util.List;

/**
 * A class for notifying the phone state event to the interested components.
 */
public class PhoneStateNotifier implements IPhoneStateNotifier {
    /**
     * An interface to notify the phone state event change.
     */
    public interface EventObserver {
        /**
         * Notifies that the phone state event is changed.
         *
         * @param notifier The {@link IPhoneStateNotifier} instance that requests
         *                 the phone state events.
         * @param events The total events.
         * @param newEvents The newly added events.
         */
        void onPhoneStateEventChanged(IPhoneStateNotifier notifier, int events, int newEvents);
    }

    private final Object mLock = new Object();
    private final PhoneStateHandler mHandler;
    private final EventObserver mEventObserver;
    private ImsPhoneStateListener mListener;
    private int mEvents = ImsPhoneStateListener.LISTEN_NONE;

    public PhoneStateNotifier(Looper looper, @NonNull EventObserver observer) {
        mHandler = (looper != null) ? new PhoneStateHandler(looper) : null;
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
        int oldEvents;

        synchronized (mLock) {
            oldEvents = mEvents;
            mEvents = events;
        }

        if (mEventObserver != null && oldEvents != events) {
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

    /**
     * Notifies that the call state is changed.
     *
     * @param state The call state.
     */
    public void notifyCallState(@CallState int state) {
        if (mHandler == null) {
            onCallStateChanged(state);
            return;
        }

        Message.obtain(mHandler, ImsPhoneStateListener.LISTEN_CALL_STATE,
                state, 0).sendToTarget();
    }

    /**
     * Notifies that the {@link CellInfo} is changed.
     *
     * @param cellInfo The list of {@link CellInfo} to report.
     */
    public void notifyCellInfo(@NonNull List<CellInfo> cellInfo) {
        if (mHandler == null) {
            onCellInfoChanged(cellInfo);
            return;
        }

        Message.obtain(mHandler, ImsPhoneStateListener.LISTEN_CELL_INFO,
                cellInfo).sendToTarget();
    }

    /**
     * Notifies that the {@link PreciseCallState} is changed.
     *
     * @param callState The {@link PreciseCallState} to report.
     */
    public void notifyPreciseCallState(@NonNull PreciseCallState callState) {
        if (mHandler == null) {
            onPreciseCallStateChanged(callState);
            return;
        }

        Message.obtain(mHandler, ImsPhoneStateListener.LISTEN_PRECISE_CALL_STATE,
                callState).sendToTarget();
    }

    /**
     * Notifies that the {@link ServiceState} is changed.
     *
     * @param serviceState The {@link ServiceState} to report.
     */
    public void notifyServiceState(@NonNull ServiceState serviceState) {
        if (mHandler == null) {
            onServiceStateChanged(serviceState);
            return;
        }

        Message.obtain(mHandler, ImsPhoneStateListener.LISTEN_SERVICE_STATE,
                serviceState).sendToTarget();
    }

    /**
     * Notifies that the {@link SignalStrength} is changed.
     *
     * @param signalStrength The {@link SignalStrength} to report.
     */
    public void notifySignalStrengths(@NonNull SignalStrength signalStrength) {
        if (mHandler == null) {
            onSignalStrengthsChanged(signalStrength);
            return;
        }

        Message.obtain(mHandler, ImsPhoneStateListener.LISTEN_SIGNAL_STRENGTHS,
                signalStrength).sendToTarget();
    }

    /**
     * Notifies that the SRVCC state is changed.
     *
     * @param state The SRVCC state to report.
     */
    public void notifySrvccState(@SrvccState int state) {
        if (mHandler == null) {
            onSrvccStateChanged(state);
            return;
        }

        Message.obtain(mHandler, ImsPhoneStateListener.LISTEN_SRVCC_STATE,
                state, 0).sendToTarget();
    }

    /**
     * Notifies that the {@link PreciseDataConnectionState} is changed.
     *
     * @param dataConnectionState The {@link PreciseDataConnectionState} to report.
     */
    public void notifyPreciseDataConnectionState(
            @NonNull PreciseDataConnectionState dataConnectionState) {
        if (mHandler == null) {
            onPreciseDataConnectionStateChanged(dataConnectionState);
            return;
        }

        Message.obtain(mHandler, ImsPhoneStateListener.LISTEN_PRECISE_DATA_CONNECTION_STATE,
                dataConnectionState).sendToTarget();
    }

    /**
     * Notifies that the {@link BarringInfo} is changed.
     *
     * @param barringInfo The {@link BarringInfo} to report.
     */
    public void notifyBarringInfo(@NonNull BarringInfo barringInfo) {
        if (mHandler == null) {
            onBarringInfoChanged(barringInfo);
            return;
        }

        Message.obtain(mHandler, ImsPhoneStateListener.LISTEN_BARRING_INFO,
                barringInfo).sendToTarget();
    }

    protected boolean isEventSet(int event) {
        synchronized (mLock) {
            return (mEvents & event) == event;
        }
    }

    /**
     * Invokes when call state is changed.
     */
    protected void onCallStateChanged(@CallState int state) {
        if (!isEventSet(ImsPhoneStateListener.LISTEN_CALL_STATE)) {
            return;
        }

        ImsPhoneStateListener listener;

        synchronized (mLock) {
            listener = mListener;
        }

        if (listener != null) {
            listener.onCallStateChanged(state);
        }
    }

    /**
     * Invokes when cell info. is changed.
     */
    protected void onCellInfoChanged(@NonNull List<CellInfo> cellInfo) {
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
    protected void onPreciseCallStateChanged(@NonNull PreciseCallState callState) {
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
    protected void onServiceStateChanged(@NonNull ServiceState serviceState) {
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
    protected void onSignalStrengthsChanged(@NonNull SignalStrength signalStrength) {
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
    protected void onSrvccStateChanged(@SrvccState int state) {
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
     * Invokes when precise data connection state is changed.
     */
    protected void onPreciseDataConnectionStateChanged(
            @NonNull PreciseDataConnectionState dataConnectionState) {
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

    /**
     * Invokes when barring information is changed.
     */
    protected void onBarringInfoChanged(@NonNull BarringInfo barringInfo) {
        if (!isEventSet(ImsPhoneStateListener.LISTEN_BARRING_INFO)) {
            return;
        }

        ImsPhoneStateListener listener;

        synchronized (mLock) {
            listener = mListener;
        }

        if (listener != null) {
            listener.onBarringInfoChanged(barringInfo);
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
                    onServiceStateChanged((ServiceState) msg.obj);
                    break;
                }
                case ImsPhoneStateListener.LISTEN_CALL_STATE: {
                    onCallStateChanged(msg.arg1);
                    break;
                }
                case ImsPhoneStateListener.LISTEN_PRECISE_CALL_STATE: {
                    onPreciseCallStateChanged((PreciseCallState) msg.obj);
                    break;
                }
                case ImsPhoneStateListener.LISTEN_SRVCC_STATE: {
                    onSrvccStateChanged(msg.arg1);
                    break;
                }
                case ImsPhoneStateListener.LISTEN_CELL_INFO: {
                    onCellInfoChanged((List<CellInfo>) msg.obj);
                    break;
                }
                case ImsPhoneStateListener.LISTEN_SIGNAL_STRENGTHS: {
                    onSignalStrengthsChanged((SignalStrength) msg.obj);
                    break;
                }
                case ImsPhoneStateListener.LISTEN_PRECISE_DATA_CONNECTION_STATE: {
                    onPreciseDataConnectionStateChanged((PreciseDataConnectionState) msg.obj);
                    break;
                }
                case ImsPhoneStateListener.LISTEN_BARRING_INFO: {
                    onBarringInfoChanged((BarringInfo) msg.obj);
                    break;
                }
                default:
                    break;
            }
        }
    }
}
