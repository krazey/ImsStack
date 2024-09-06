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

package com.android.imsstack.core.agents;

import android.annotation.NonNull;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.telephony.AccessNetworkConstants;
import android.telephony.Annotation.CallState;
import android.telephony.Annotation.NetworkType;
import android.telephony.Annotation.SrvccState;
import android.telephony.BarringInfo;
import android.telephony.CellInfo;
import android.telephony.NetworkRegistrationInfo;
import android.telephony.PreciseCallState;
import android.telephony.PreciseDataConnectionState;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.TelephonyCallback;
import android.telephony.TelephonyManager;
import android.util.SparseArray;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.core.agents.internal.PhoneStateEvents;
import com.android.imsstack.core.agents.internal.PhoneStateNotifier;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ImsEventDef;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.List;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * A class for providing an interface to monitor the phone state (call sate, service state, ...).
 */
public class PhoneStateAgent implements PhoneStateInterface,
        PhoneStateNotifier.EventObserver {
    private final Object mLock = new Object();
    private final Set<PhoneStateNotifier> mPhoneStateNotifiers =
            new CopyOnWriteArraySet<PhoneStateNotifier>();
    private final PhoneStateEvents mEvents = new PhoneStateEvents();
    private final int mSlotId;
    private Sim.Listener mSimListener;
    private PhoneStateListener mPhoneStateListener;
    private ServiceState mServiceState;
    private BarringInfo mBarringInfo;
    private int mCallState = TelephonyManager.CALL_STATE_IDLE;
    private int mCsCallState = TelephonyManager.CALL_STATE_IDLE;
    private final AtomicInteger mImsCallState = new AtomicInteger(TelephonyManager.CALL_STATE_IDLE);
    private int mCellularDataNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;

    public PhoneStateAgent(int slotId) {
        mSlotId = slotId;
    }

    @Override
    public void init(Context context) {
        mPhoneStateListener = new PhoneStateListener(MSimUtils.getSubId(mSlotId));
        mPhoneStateListener.registerCallbacks(mEvents.getEvents());

        mSimListener = new Sim.Listener() {
                @Override
                public void onSimStateChanged() {
                    handleSimStateChanged();
                }
            };
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
        if (sim != null) {
            sim.addListener(mSimListener);
        }
    }

    @Override
    public void cleanup() {
        if (mSimListener != null) {
            SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
            if (sim != null) {
                sim.removeListener(mSimListener);
            }
            mSimListener = null;
        }

        mPhoneStateNotifiers.clear();

        if (mPhoneStateListener != null) {
            mPhoneStateListener.unregisterCallbacks();
            mPhoneStateListener.dispose();
            mPhoneStateListener = null;
        }

        mServiceState = null;
        mBarringInfo = null;
        mCsCallState = TelephonyManager.CALL_STATE_IDLE;
        mImsCallState.set(TelephonyManager.CALL_STATE_IDLE);
        mCellularDataNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
    }

    @Override
    public IPhoneStateNotifier createNotifier(ImsPhoneStateListener listener) {
        PhoneStateNotifier notifier = new PhoneStateNotifier(null, this);
        notifier.setListener(listener);
        return notifier;
    }

    @Override
    public IPhoneStateNotifier createNotifier(ImsPhoneStateListener listener,
            @NonNull Looper looper) {
        PhoneStateNotifier notifier = new PhoneStateNotifier(looper, this);
        notifier.setListener(listener);
        return notifier;
    }

    @Override
    public void addNotifier(IPhoneStateNotifier notifier) {
        boolean isChanged = mPhoneStateNotifiers.add((PhoneStateNotifier) notifier);

        if (isChanged && (notifier != null) && (notifier.getEvents() != 0)) {
            updatePhoneStateEvents(notifier, notifier.getEvents(), notifier.getEvents());
        }
    }

    @Override
    public void removeNotifier(IPhoneStateNotifier notifier) {
        boolean isChanged = mPhoneStateNotifiers.remove((PhoneStateNotifier) notifier);

        if (isChanged && (notifier != null) && (notifier.getEvents() != 0)) {
            updatePhoneStateEvents(notifier, 0, 0);
        }
    }

    @Override
    public @NetworkType int getCellularDataNetworkType() {
        return mCellularDataNetworkType;
    }

    @Override
    public @CallState int getCsCallState() {
        return mCsCallState;
    }

    @Override
    public @CallState int getImsCallState() {
        return mImsCallState.get();
    }

    @Override
    public void setImsCallState(@CallState int state) {
        if (mImsCallState.get() != state) {
            ImsLog.i(mSlotId, "IMS call state: "
                    + TelephonyInterface.callStateToString(mImsCallState.get()) + " -> "
                    + TelephonyInterface.callStateToString(state));
            mImsCallState.set(state);
        }
    }

    @Override
    public void onPhoneStateEventChanged(IPhoneStateNotifier notifier,
            int events, int newEvents) {
        if (!mPhoneStateNotifiers.contains((PhoneStateNotifier) notifier)) {
            return;
        }

        updatePhoneStateEvents(notifier, events, newEvents);
    }

    /**
     * Returns the {@link PhoneStateEvents} object for testing.
     *
     * @return A {@link PhoneStateEvents} instance.
     */
    @VisibleForTesting
    public PhoneStateEvents getPhoneStateEvents() {
        return mEvents;
    }

    private void updateCsCallState(@CallState int state) {
        if (mImsCallState.get() != TelephonyManager.CALL_STATE_IDLE) {
            if (mCsCallState == TelephonyManager.CALL_STATE_IDLE) {
                return;
            }
            // Update CS call state to CALL_STATE_IDLE
            // because IMS call is in progress or active.
            state = TelephonyManager.CALL_STATE_IDLE;
        }

        if (state != TelephonyManager.CALL_STATE_IDLE
                && !MSimUtils.isValidSubId(mPhoneStateListener.getSubId())
                && !AgentUtils.isAllSimAbsent()) {
            // Ignore the current call state change event
            // because this call state is changed from the other slot.
            return;
        }

        if (mCsCallState != state) {
            ImsLog.i(mSlotId, "CS call state: "
                    + TelephonyInterface.callStateToString(mCsCallState) + " -> "
                    + TelephonyInterface.callStateToString(state));
            mCsCallState = state;
            ISystem system = SystemInterface.getInstance().getSystem(mSlotId);
            if (system != null) {
                system.notifyEvent(ImsEventDef.IMS_EVENT_CSCALL_STATE, mCsCallState, 0);
            }
        }
    }

    private void notifyCallState(@CallState int state) {
        for (PhoneStateNotifier n : mPhoneStateNotifiers) {
            n.notifyCallState(state);
        }
    }

    private void notifyCellInfo(@NonNull List<CellInfo> cellInfo) {
        for (PhoneStateNotifier n : mPhoneStateNotifiers) {
            n.notifyCellInfo(cellInfo);
        }
    }

    private void notifyPreciseCallState(@NonNull PreciseCallState callState) {
        for (PhoneStateNotifier n : mPhoneStateNotifiers) {
            n.notifyPreciseCallState(callState);
        }
    }

    private void notifyServiceState(@NonNull ServiceState serviceState) {
        for (PhoneStateNotifier n : mPhoneStateNotifiers) {
            n.notifyServiceState(serviceState);
        }
    }

    private void notifySignalStrengths(@NonNull SignalStrength signalStrength) {
        for (PhoneStateNotifier n : mPhoneStateNotifiers) {
            n.notifySignalStrengths(signalStrength);
        }
    }

    private void notifySrvccState(@SrvccState int state) {
        for (PhoneStateNotifier n : mPhoneStateNotifiers) {
            n.notifySrvccState(state);
        }
    }

    private void notifyPreciseDataConnectionState(
            @NonNull PreciseDataConnectionState dataConnectionState) {
        for (PhoneStateNotifier n : mPhoneStateNotifiers) {
            n.notifyPreciseDataConnectionState(dataConnectionState);
        }
    }

    private void notifyBarringInfo(@NonNull BarringInfo barringInfo) {
        for (PhoneStateNotifier n : mPhoneStateNotifiers) {
            n.notifyBarringInfo(barringInfo);
        }
    }

    private void notifyCurrentStateIfPresent(IPhoneStateNotifier notifier, int events) {
        if (PhoneStateEvents.isEventSet(events, ImsPhoneStateListener.LISTEN_SERVICE_STATE)) {
            final ServiceState serviceState = mServiceState;
            if (serviceState != null) {
                PhoneStateNotifier psn = (PhoneStateNotifier) notifier;
                psn.notifyServiceState(serviceState);
            }
        }

        if (PhoneStateEvents.isEventSet(events, ImsPhoneStateListener.LISTEN_CALL_STATE)) {
            PhoneStateNotifier psn = (PhoneStateNotifier) notifier;
            psn.notifyCallState(mCallState);
        }

        if (PhoneStateEvents.isEventSet(events, ImsPhoneStateListener.LISTEN_BARRING_INFO)) {
            final BarringInfo barringInfo = mBarringInfo;
            if (barringInfo != null) {
                PhoneStateNotifier psn = (PhoneStateNotifier) notifier;
                psn.notifyBarringInfo(barringInfo);
            }
        }
    }

    private void listenForPhoneStateEventChanged() {
        synchronized (mLock) {
            if (mPhoneStateListener != null) {
                ImsLog.i(mSlotId, "listenForPhoneStateEventChanged: subId="
                        + mPhoneStateListener.getSubId());
                mPhoneStateListener.registerCallbacks(mEvents.getEvents());
            }
        }
    }

    private void handleSimStateChanged() {
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);

        if (sim == null || !sim.isSimLoadCompleted()) {
            return;
        }

        synchronized (mLock) {
            int subId = sim.getSubId();

            if (subId == mPhoneStateListener.getSubId()) {
                // no-op
                ImsLog.w(mSlotId, "Subscription is not changed; subId=" + subId);
                return;
            }

            ImsLog.i(mSlotId, "handleSimStateChanged: subId=" + subId);
            mPhoneStateListener.unregisterCallbacks();
            mPhoneStateListener.dispose();

            mPhoneStateListener = new PhoneStateListener(subId);
            mPhoneStateListener.registerCallbacks(mEvents.getEvents());
        }
    }

    private void updatePhoneStateEvents(IPhoneStateNotifier notifier,
            int events, int newEvents) {
        if (mEvents.updateEvents(events, notifier)) {
            listenForPhoneStateEventChanged();
        } else if (newEvents > 0) {
            notifyCurrentStateIfPresent(notifier, newEvents);
        }
    }

    private static TelephonyManagerProxy getTelephonyManagerProxy(int subId) {
        if (!MSimUtils.isValidSubId(subId)) {
            return AppContext.getInstance().getSystemServiceProxy(TelephonyManagerProxy.class);
        }

        return AppContext.getTelephonyManagerProxy(subId);
    }

    private static int getDataAccessNetworkTechnology(ServiceState ss) {
        final NetworkRegistrationInfo nri = ss.getNetworkRegistrationInfo(
                NetworkRegistrationInfo.DOMAIN_PS, AccessNetworkConstants.TRANSPORT_TYPE_WWAN);

        if (nri != null) {
            return nri.getAccessNetworkTechnology();
        }

        return TelephonyManager.NETWORK_TYPE_UNKNOWN;
    }

    private static int getVoiceAccessNetworkTechnology(ServiceState ss) {
        final NetworkRegistrationInfo nri = ss.getNetworkRegistrationInfo(
                NetworkRegistrationInfo.DOMAIN_CS, AccessNetworkConstants.TRANSPORT_TYPE_WWAN);

        if (nri != null) {
            return nri.getAccessNetworkTechnology();
        }

        return TelephonyManager.NETWORK_TYPE_UNKNOWN;
    }

    private static int getDataRegistrationState(ServiceState ss) {
        final NetworkRegistrationInfo wwanRegInfo = ss.getNetworkRegistrationInfo(
                NetworkRegistrationInfo.DOMAIN_PS, AccessNetworkConstants.TRANSPORT_TYPE_WWAN);

        return (wwanRegInfo != null) ? wwanRegInfo.getRegistrationState() :
                NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING;
    }

    private final class PhoneStateListener extends Handler {
        private static final int SS_VOICE_REG_STATE = 0x00000001;
        private static final int SS_VOICE_RAT = 0x00000010;
        private static final int SS_DATA_REG_STATE = 0x00000100;
        private static final int SS_DATA_RAT = 0x00001000;
        private static final int SS_ROAMING = 0x00010000;
        private static final int SS_NETWORK_OPERATOR = 0x00100000;
        private static final int SS_ALL = (SS_VOICE_REG_STATE
                | SS_VOICE_RAT | SS_DATA_REG_STATE | SS_DATA_RAT
                | SS_ROAMING | SS_NETWORK_OPERATOR);

        private final int mSubId;
        private final SparseArray<TelephonyCallback> mTelephonyCallbacks =
                new SparseArray<>(8);
        private final SparseArray<TelephonyCallback> mRegisteredTelephonyCallbacks =
                new SparseArray<>(8);

        PhoneStateListener(int subId) {
            super(AppContext.getInstance().getMainLooper());
            mSubId = subId;
            ImsLog.i(mSlotId, "PhoneStateListener: subId=" + subId);

            mTelephonyCallbacks.put(ImsPhoneStateListener.LISTEN_SERVICE_STATE,
                    new ServiceStateListener());
            mTelephonyCallbacks.put(ImsPhoneStateListener.LISTEN_CALL_STATE,
                    new CallStateListener());
            mTelephonyCallbacks.put(ImsPhoneStateListener.LISTEN_PRECISE_CALL_STATE,
                    new PreciseCallStateListener());
            mTelephonyCallbacks.put(ImsPhoneStateListener.LISTEN_SRVCC_STATE,
                    new SrvccStateListener());
            mTelephonyCallbacks.put(ImsPhoneStateListener.LISTEN_CELL_INFO,
                    new CellInfoListener());
            mTelephonyCallbacks.put(ImsPhoneStateListener.LISTEN_SIGNAL_STRENGTHS,
                    new SignalStrengthsListener());
            mTelephonyCallbacks.put(ImsPhoneStateListener.LISTEN_PRECISE_DATA_CONNECTION_STATE,
                    new PreciseDataConnectionStateListener());
            mTelephonyCallbacks.put(ImsPhoneStateListener.LISTEN_BARRING_INFO,
                    new BarringInfoListener());
        }

        public void dispose() {
            removeCallbacksAndMessages(null);
            mTelephonyCallbacks.clear();
            mRegisteredTelephonyCallbacks.clear();
            mServiceState = null;
            mBarringInfo = null;
            mCallState = TelephonyManager.CALL_STATE_IDLE;
            mCellularDataNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        }

        public int getSubId() {
            return mSubId;
        }

        public void registerCallbacks(int events) {
            TelephonyManagerProxy tmp = getTelephonyManagerProxy(getSubId());

            for (int i = 0; i < mTelephonyCallbacks.size(); ++i) {
                int event = mTelephonyCallbacks.keyAt(i);
                TelephonyCallback callback = mRegisteredTelephonyCallbacks.get(event);

                if (callback != null) {
                    tmp.unregisterTelephonyCallback(callback);
                    mRegisteredTelephonyCallbacks.remove(event);
                }

                if (PhoneStateEvents.isEventSet(events, event)) {
                    callback = mTelephonyCallbacks.valueAt(i);
                    mRegisteredTelephonyCallbacks.put(event, callback);
                    tmp.registerTelephonyCallback(this::post, callback);
                }
            }
        }

        public void unregisterCallbacks() {
            TelephonyManagerProxy tmp = getTelephonyManagerProxy(getSubId());

            for (int i = 0; i < mRegisteredTelephonyCallbacks.size(); ++i) {
                TelephonyCallback callback = mRegisteredTelephonyCallbacks.valueAt(i);
                if (callback != null) {
                    tmp.unregisterTelephonyCallback(callback);
                }
            }

            mRegisteredTelephonyCallbacks.clear();
        }

        private final class CallStateListener extends TelephonyCallback implements
                TelephonyCallback.CallStateListener {
            @Override
            public void onCallStateChanged(@CallState int state) {
                ImsLog.i(mSlotId, "onCallStateChanged: "
                        + TelephonyInterface.callStateToString(mCallState) + " -> "
                        + TelephonyInterface.callStateToString(state));
                if (state == TelephonyManager.CALL_STATE_IDLE) {
                    setImsCallState(state);
                }
                updateCsCallState(state);
                // Store the most recent call state
                mCallState = state;
                notifyCallState(state);
            }
        }

        private final class CellInfoListener extends TelephonyCallback implements
                TelephonyCallback.CellInfoListener {
            @Override
            public void onCellInfoChanged(@NonNull List<CellInfo> cellInfo) {
                if (ImsLog.isDebuggable()) {
                    ImsLog.i(mSlotId, "onCellInfoChanged");
                }
                notifyCellInfo(cellInfo);
            }
        }

        private final class PreciseCallStateListener extends TelephonyCallback implements
                TelephonyCallback.PreciseCallStateListener {
            @Override
            public void onPreciseCallStateChanged(@NonNull PreciseCallState callState) {
                if (ImsLog.isDebuggable()) {
                    ImsLog.i(mSlotId, "onPreciseCallStateChanged: cs=" + callState);
                }
                notifyPreciseCallState(callState);
            }
        }

        private final class ServiceStateListener extends TelephonyCallback implements
                TelephonyCallback.ServiceStateListener {
            @Override
            public void onServiceStateChanged(@NonNull ServiceState serviceState) {
                ImsLog.i(mSlotId, "onServiceStateChanged: ss=" + serviceState
                        + ", changed(operator|roaming|data_rat|data_reg|voice_rat|voice_reg)="
                        + Integer.toHexString(getChangedStates(mServiceState, serviceState)));

                // Store the most recent service state
                mServiceState = serviceState;
                updateCellularDataNetworkType(serviceState);
                notifyServiceState(serviceState);
            }
        }

        private final class SignalStrengthsListener extends TelephonyCallback implements
                TelephonyCallback.SignalStrengthsListener {
            @Override
            public void onSignalStrengthsChanged(@NonNull SignalStrength signalStrength) {
                if (ImsLog.isDebuggable()) {
                    ImsLog.i(mSlotId, "onSignalStrengthsChanged: ss=" + signalStrength);
                }
                notifySignalStrengths(signalStrength);
            }
        }

        public final class SrvccStateListener extends TelephonyCallback implements
                TelephonyCallback.SrvccStateListener {
            @Override
            public void onSrvccStateChanged(@SrvccState int state) {
                ImsLog.i(mSlotId, "onSrvccStateChanged: state=" + state);
                notifySrvccState(state);
            }
        }

        private final class PreciseDataConnectionStateListener extends TelephonyCallback implements
                TelephonyCallback.PreciseDataConnectionStateListener {
            @Override
            public void onPreciseDataConnectionStateChanged(
                    @NonNull PreciseDataConnectionState dataConnectionState) {
                if (ImsLog.isDebuggable()) {
                    ImsLog.i(mSlotId, "onPreciseDataConnectionStateChanged: dcs="
                            + dataConnectionState);
                }
                notifyPreciseDataConnectionState(dataConnectionState);
            }
        }

        private final class BarringInfoListener extends TelephonyCallback implements
                TelephonyCallback.BarringInfoListener {
            @Override
            public void onBarringInfoChanged(@NonNull BarringInfo barringInfo) {
                // Store the most recent barring info
                mBarringInfo = barringInfo;
                notifyBarringInfo(barringInfo);
            }
        }

        private int getChangedStates(ServiceState oldSs, ServiceState newSs) {
            if (oldSs == null) {
                return SS_ALL;
            }

            int changedStates = 0;

            if (oldSs.getState() != newSs.getState()) {
                changedStates |= SS_VOICE_REG_STATE;
            }

            if (getVoiceAccessNetworkTechnology(oldSs) != getVoiceAccessNetworkTechnology(newSs)) {
                changedStates |= SS_VOICE_RAT;
            }

            if (getDataRegistrationState(oldSs) != getDataRegistrationState(newSs)) {
                changedStates |= SS_DATA_REG_STATE;
            }

            if (getDataAccessNetworkTechnology(oldSs) != getDataAccessNetworkTechnology(newSs)) {
                changedStates |= SS_DATA_RAT;
            }

            if (oldSs.getRoaming() != newSs.getRoaming()) {
                changedStates |= SS_ROAMING;
            }

            if (!Objects.equals(oldSs.getOperatorNumeric(), newSs.getOperatorNumeric())) {
                changedStates |= SS_NETWORK_OPERATOR;
            }

            return changedStates;
        }

        private void updateCellularDataNetworkType(ServiceState serviceState) {
            TelephonyManagerProxy tmp = getTelephonyManagerProxy(getSubId());
            int dataNetworkType = tmp.getDataNetworkType();

            if (dataNetworkType == TelephonyManager.NETWORK_TYPE_IWLAN) {
                NetworkRegistrationInfo nri = serviceState.getNetworkRegistrationInfo(
                        NetworkRegistrationInfo.DOMAIN_PS,
                        AccessNetworkConstants.TRANSPORT_TYPE_WWAN);

                mCellularDataNetworkType = (nri == null || !nri.isNetworkRegistered())
                        ? TelephonyManager.NETWORK_TYPE_UNKNOWN
                        : nri.getAccessNetworkTechnology();
            }
        }
    }
}
