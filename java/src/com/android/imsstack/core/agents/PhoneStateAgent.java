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

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.telephony.AccessNetworkConstants;
import android.telephony.CellInfo;
import android.telephony.NetworkRegistrationInfo;
import android.telephony.PreciseCallState;
import android.telephony.PreciseDataConnectionState;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.TelephonyCallback;
import android.telephony.TelephonyManager;

import com.android.imsstack.core.agents.agentif.IPhoneState;
import com.android.imsstack.core.agents.agentif.IPhoneStateNotifier;
import com.android.imsstack.core.agents.agentif.ISubscription;
import com.android.imsstack.core.agents.agentif.ImsPhoneStateListener;
import com.android.imsstack.core.agents.agentif.PhoneCallState;
import com.android.imsstack.core.agents.agentif.SubscriptionListener;
import com.android.imsstack.core.agents.internal.PhoneStateEvents;
import com.android.imsstack.core.agents.internal.PhoneStateNotifier;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsProperties;
import com.android.imsstack.util.MSimUtils;

import java.util.List;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;
import java.util.concurrent.Executor;
import java.util.concurrent.RejectedExecutionException;

/**
 * This class provides the APIs to monitor the phone state (call sate, service state, ...).
 */
public final class PhoneStateAgent implements IPhoneState,
        PhoneStateNotifier.EventObserver {
    private final Object mLock = new Object();
    private Context mContext;
    private final Set<PhoneStateNotifier> mPhoneStateNotifiers =
            new CopyOnWriteArraySet<PhoneStateNotifier>();
    private final PhoneStateEvents mEvents = new PhoneStateEvents();
    private final PhoneStateHandler mHandler;
    private SubscriptionListenerProxy mSubscriptionListener;
    private PhoneStateListener mPhoneStateListener;
    private int mSlotId = 0;

    public PhoneStateAgent(int slotId) {
        mSlotId = slotId;
        mHandler = new PhoneStateHandler();
    }

    @Override
    public void init(Context context) {
        mContext = context;

        mPhoneStateListener = createPhoneStateListener(MSimUtils.getSubId(mSlotId));

        setActivePhoneStateListener();

        mSubscriptionListener = new SubscriptionListenerProxy();

        ISubscription isub = (ISubscription) AgentFactory.getAgent(AgentFactory.SUBSCRIPTION);
        if (isub != null) {
            isub.addListener(mSubscriptionListener);
        }
    }

    @Override
    public void cleanup() {
        mHandler.removeCallbacksAndMessages(null);

        if (mSubscriptionListener != null) {
            ISubscription isub = (ISubscription) AgentFactory.getAgent(AgentFactory.SUBSCRIPTION);
            if (isub != null) {
                isub.removeListener(mSubscriptionListener);
            }
            mSubscriptionListener = null;
        }

        mPhoneStateNotifiers.clear();

        if (mPhoneStateListener != null) {
            mPhoneStateListener.removeListener();
            mPhoneStateListener.dispose();
            mPhoneStateListener = null;
        }
    }

    /**
     * Creates the phone state notifier without Handler.
     * Application SHOULD handle the event after posting the event on callback.
     */
    @Override
    public IPhoneStateNotifier createNotifier(ImsPhoneStateListener listener) {
        PhoneStateNotifier notifier = new PhoneStateNotifier(this);
        notifier.setListener(listener);
        return notifier;
    }

    /**
     * Creates the phone state notifier with Handler of the specified Looper.
     * Application can handle the events directly (on callback flow)
     * because event callback is invoked by its Handler.
     */
    @Override
    public IPhoneStateNotifier createNotifier(ImsPhoneStateListener listener, Looper looper) {
        PhoneStateNotifier notifier = new PhoneStateNotifier(looper, this);
        notifier.setListener(listener);
        return notifier;
    }

    /**
     * Adds the notifier to monitor the phone state (call state, service state, ...).
     */
    @Override
    public void addNotifier(IPhoneStateNotifier notifier) {
        boolean isChanged = mPhoneStateNotifiers.add((PhoneStateNotifier) notifier);

        if (isChanged && (notifier != null) && (notifier.getEvents() != 0)) {
            updateImsEvents(notifier, notifier.getEvents(), notifier.getEvents());
            updatePhoneStateEvents(notifier, notifier.getEvents(), notifier.getEvents());
        }
    }

    /**
     * Removes the notifier to monitor the phone state (call state, service state, ...).
     */
    @Override
    public void removeNotifier(IPhoneStateNotifier notifier) {
        boolean isChanged = mPhoneStateNotifiers.remove((PhoneStateNotifier) notifier);

        if (isChanged && (notifier != null) && (notifier.getEvents() != 0)) {
            updateImsEvents(notifier, 0, 0);
            updatePhoneStateEvents(notifier, 0, 0);
        }
    }

    /**
     * Get data RAT for cellular (TelephonyManager.NETWORK_TYPE_XXX)
     */
    @Override
    public int getCellularDataRAT() {
        return (mPhoneStateListener != null) ? mPhoneStateListener.getCellularDataRAT()
                    : TelephonyManager.NETWORK_TYPE_UNKNOWN;
    }

    @Override
    public void onPhoneStateEventChanged(IPhoneStateNotifier notifier,
            int events, int newEvents) {
        if (!mPhoneStateNotifiers.contains((PhoneStateNotifier) notifier)) {
            return;
        }

        updateImsEvents(notifier, events, newEvents);
        updatePhoneStateEvents(notifier, events, newEvents);
    }

    private void notifyCallState(int state, String incomingNumber) {
        for (PhoneStateNotifier n : mPhoneStateNotifiers) {
            n.notifyCallState(state, incomingNumber);
        }
    }

    private void notifyCellInfo(List<CellInfo> cellInfo) {
        for (PhoneStateNotifier n : mPhoneStateNotifiers) {
            n.notifyCellInfo(cellInfo);
        }
    }

    private void notifyPreciseCallState(PreciseCallState callState) {
        for (PhoneStateNotifier n : mPhoneStateNotifiers) {
            n.notifyPreciseCallState(callState);
        }
    }

    private void notifyServiceState(ServiceState serviceState) {
        for (PhoneStateNotifier n : mPhoneStateNotifiers) {
            n.notifyServiceState(serviceState);
        }
    }

    private void notifySignalStrengths(SignalStrength signalStrength) {
        for (PhoneStateNotifier n : mPhoneStateNotifiers) {
            n.notifySignalStrengths(signalStrength);
        }
    }

    private void notifySrvccState(int state) {
        for (PhoneStateNotifier n : mPhoneStateNotifiers) {
            n.notifySrvccState(state);
        }
    }

    private void notifyPcscfUpdated(List<String> pcscf) {
        for (PhoneStateNotifier n : mPhoneStateNotifiers) {
            n.notifyPcscfUpdated(pcscf);
        }
    }

    private void notifyPreciseDataConnectionState(PreciseDataConnectionState dataConnectionState) {
        for (PhoneStateNotifier n : mPhoneStateNotifiers) {
            n.notifyPreciseDataConnectionState(dataConnectionState);
        }
    }

    private void notifyCurrentStateIfPresent(IPhoneStateNotifier notifier, int events) {
        if (notifier == null) {
            return;
        }

        if (isEventSet(events, PhoneStateEvents.LISTEN_SERVICE_STATE)) {
            ServiceState serviceState = (mPhoneStateListener != null)
                    ? mPhoneStateListener.getServiceState() : null;

            if (serviceState != null) {
                PhoneStateNotifier psn = (PhoneStateNotifier) notifier;
                psn.notifyServiceState(serviceState);
            }
        }

        if (isEventSet(events, PhoneStateEvents.LISTEN_CALL_STATE)) {
            PhoneCallState callState = (mPhoneStateListener != null)
                    ? mPhoneStateListener.getCallState() : null;

            if (callState != null) {
                PhoneStateNotifier psn = (PhoneStateNotifier) notifier;
                psn.notifyCallState(callState.getState(), callState.getIncomingNumber());
            }
        }
    }

    private void notifyCurrentStateIfPresentForImsEvent(
            IPhoneStateNotifier notifier, int events) {
        if (notifier == null) {
            return;
        }

        if (isEventSet(events, PhoneStateEvents.LISTEN_PCSCF_ADDRESS_INFO)) {
            List<String> pcscf = mHandler.getPcscf();

            if (pcscf != null) {
                PhoneStateNotifier psn = (PhoneStateNotifier) notifier;
                psn.notifyPcscfUpdated(pcscf);
            }
        }
    }

    private void listenForImsEventChanged() {
        synchronized (mLock) {
            ImsLog.i(mSlotId, "listenForImsEventChanged :: subId="
                    + ((mPhoneStateListener != null) ? mPhoneStateListener.getSubId() : -1));
        }
    }

    private void listenForPhoneStateEventChanged() {
        synchronized (mLock) {
            if (mPhoneStateListener == null) {
                ImsLog.w(mSlotId, "PhoneStateListener is null");
                return;
            }

            ImsLog.i(mSlotId, "listenForPhoneStateEventChanged :: subId="
                        + mPhoneStateListener.getSubId());
            setActivePhoneStateListener();
        }
    }

    private void listenForSubscriptionChanged(int subId) {
        synchronized (mLock) {
            int slotId = MSimUtils.getSlotId(subId);
            if (mSlotId != slotId) {
                ISubscription isub = (ISubscription) AgentFactory.getAgent(
                        AgentFactory.SUBSCRIPTION);

                if (isub == null) {
                    return;
                }

                subId = isub.getSubId(mSlotId);
            }

            if (mPhoneStateListener == null) {
                mPhoneStateListener = createPhoneStateListener(subId);
            } else {
                if (subId != mPhoneStateListener.getSubId()) {
                    mPhoneStateListener.removeListener();
                    mPhoneStateListener.dispose();
                    mPhoneStateListener = null;
                    mPhoneStateListener = createPhoneStateListener(subId);
                } else {
                    // no-op
                    ImsLog.w(mSlotId, "Subscription is not changed; subId=" + subId);
                    return;
                }
            }

            ImsLog.i(mSlotId, "listenForSubscriptionChanged :: subId=" + subId);
            setActivePhoneStateListener();
        }
    }

    private void listenForSimLoadCompleted(int slotId) {
        if (mSlotId != slotId) {
            return;
        }

        int subId = MSimUtils.getSubId(slotId);

        listenForSubscriptionChanged(subId);

        if (ImsProperties.isChipVendorMtk()) {
            ImsLog.i(mSlotId, "Notify again ServiceState after SIM loaded");

            setServiceStateAfterSimLoad(subId);

            ServiceState serviceState = mPhoneStateListener.getServiceState();
            if (serviceState != null) {
                notifyServiceState(serviceState);
            }
        }
    }

    private void setServiceStateAfterSimLoad(int subId) {
        TelephonyManager tm = getTelephonyManager(subId);

        if (tm == null) {
            return;
        }

        ServiceState serviceState = tm.getServiceState();
        if (serviceState != null) {
            mPhoneStateListener.setServiceState(serviceState);
        }
    }

    private void setActivePhoneStateListener() {
        mPhoneStateListener.setListener(mEvents.getEvents());
    }

    private void updateImsEvents(IPhoneStateNotifier notifier,
            int events, int newEvents) {
        int imsEvents = PhoneStateEvents.getImsEventsFromImsPhoneState(events);

        if (mEvents.updateImsEvents(imsEvents, notifier)) {
            listenForImsEventChanged();
        } else if (newEvents > 0) {
            notifyCurrentStateIfPresentForImsEvent(notifier, newEvents);
        }
    }

    private void updatePhoneStateEvents(IPhoneStateNotifier notifier,
            int events, int newEvents) {
        int psEvents = PhoneStateEvents.getEventsFromImsPhoneState(events);

        if (mEvents.updateEvents(psEvents, notifier)) {
            listenForPhoneStateEventChanged();
        } else if (newEvents > 0) {
            notifyCurrentStateIfPresent(notifier, newEvents);
        }
    }

    private TelephonyManager getTelephonyManager(int subId) {
        if (!MSimUtils.isValidSubId(subId)) {
            return AppContext.getTelephonyManager();
        }

        return AppContext.getTelephonyManager(subId);
    }

    private PhoneStateListener createPhoneStateListener(int subId) {
        if (MSimUtils.isValidSubId(subId)) {
            return new PhoneStateListener(subId);
        }

        return new PhoneStateListener();
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

    private final class PhoneStateHandler extends Handler implements Executor {
        private List<String> mPcscf = null;

        @Override
        public void execute(Runnable command) {
            if (!post(command)) {
                throw new RejectedExecutionException(this + " is shutting down");
            }
        }

        @Override
        public void handleMessage(Message msg) {
            ImsLog.d(mSlotId, "handleMessage :: event=" + msg.what);

            switch (msg.what) {
                default:
                    // no-op
                    break;
            }
        }

        public List<String> getPcscf() {
            return mPcscf;
        }

        private void onPcscfUpdated(List<String> pcscf) {
            ImsLog.i(mSlotId, "onPcscfUpdated :: ");
            notifyPcscfUpdated(pcscf);

            // Store the most recent pcscf
            mPcscf = pcscf;
        }
    }

    private static boolean isEventSet(int events, int event) {
        return (events & event) != 0;
    }

    private final class PhoneStateListener {
        private static final int SS_VOICE_REG_STATE = 0x00000001;
        private static final int SS_VOICE_RAT = 0x00000010;
        private static final int SS_DATA_REG_STATE = 0x00000100;
        private static final int SS_DATA_RAT = 0x00001000;
        private static final int SS_ROAMING = 0x00010000;
        private static final int SS_NETWORK_OPERATOR = 0x00100000;
        private static final int SS_ALL = 0x00111111;

        private final int mSubId;
        private ServiceState mServiceState = null;
        private PhoneCallState mCallState = new PhoneCallState();
        private boolean mDisposed = false;
        private int mCellularDataRAT = TelephonyManager.NETWORK_TYPE_UNKNOWN;

        private CallStateListener mCallStateListener = null;
        private CellInfoListener mCellInfoListener = null;
        private PreciseCallStateListener mPreciseCallStateListener = null;
        private ServiceStateListener mServiceStateListener = null;
        private SignalStrengthsListener mSignalStrengthsListener = null;
        private SrvccStateListener mSrvccStateListener = null;
        private PreciseDataConnectionStateListener mPreciseDataConnectionStateListener = null;

        PhoneStateListener() {
            mSubId = MSimUtils.INVALID_SUB_ID;
        }

        PhoneStateListener(int subId) {
            mSubId = subId;
            ImsLog.i(mSlotId, "PhoneStateListener :: subId=" + subId);
        }

        public void dispose() {
            mDisposed = true;
            mServiceState = null;
            mCallState = null;
            mCallStateListener = null;
            mCellInfoListener = null;
            mPreciseCallStateListener = null;
            mServiceStateListener = null;
            mSignalStrengthsListener = null;
            mSrvccStateListener = null;
            mPreciseDataConnectionStateListener = null;
        }

        /**
         * These onXXX methods can be invoked in each TelephonyCallback's listener.
         */
        public void onCallStateChanged(int state, String incomingNumber) {
            if (isDisposed()) {
                return;
            }

            ImsLog.i(mSlotId, "onCallStateChanged :: state=" + state
                    + ", incomingNumber=" + ImsLog.hiddenString(incomingNumber));
            notifyCallState(state, incomingNumber);

            // Store the most recent call state
            mCallState.setState(state);
            mCallState.setIncomingNumber(incomingNumber);
        }

        public void onCellInfoChanged(List<CellInfo> cellInfo) {
            if (isDisposed()) {
                return;
            }

            if (ImsLog.isDebuggable()) {
                ImsLog.i(mSlotId, "onCellInfoChanged");
            }

            notifyCellInfo(cellInfo);
        }

        public void onPreciseCallStateChanged(PreciseCallState callState) {
            if (isDisposed()) {
                return;
            }

            if (ImsLog.isDebuggable()) {
                ImsLog.i(mSlotId, "onPreciseCallStateChanged :: cs=" + callState);
            }

            notifyPreciseCallState(callState);
        }

        public void onServiceStateChanged(ServiceState serviceState) {
            if (isDisposed()) {
                return;
            }

            ImsLog.i(mSlotId, "onServiceStateChanged :: ss=" + serviceState
                    + ", changed(operator|roaming|data_rat|data_reg|voice_rat|voice_reg)="
                    + Integer.toHexString(getChangedStates(mServiceState, serviceState)));

            updateCellularDataRAT(serviceState);

            notifyServiceState(serviceState);

            // Store the most recent service state
            mServiceState = serviceState;
        }

        public void onSignalStrengthsChanged(SignalStrength signalStrength) {
            if (isDisposed()) {
                return;
            }

            if (ImsLog.isDebuggable()) {
                ImsLog.i(mSlotId, "onSignalStrengthsChanged :: ss=" + signalStrength);
            }

            notifySignalStrengths(signalStrength);
        }

        public void onSrvccStateChanged(int state) {
            if (isDisposed()) {
                return;
            }

            ImsLog.i(mSlotId, "onSrvccStateChanged :: state=" + state);
            notifySrvccState(state);
        }

        public void onPreciseDataConnectionStateChanged(
                PreciseDataConnectionState dataConnectionState) {
            if (isDisposed()) {
                return;
            }

            if (ImsLog.isDebuggable()) {
                ImsLog.i(mSlotId, "onPreciseDataConnectionStateChanged :: dcs="
                        + dataConnectionState);
            }

            notifyPreciseDataConnectionState(dataConnectionState);
        }

        public int getSubId() {
            return mSubId;
        }

        public PhoneCallState getCallState() {
            return mCallState;
        }

        public ServiceState getServiceState() {
            return mServiceState;
        }

        public int getCellularDataRAT() {
            return mCellularDataRAT;
        }

        public void setServiceState(ServiceState serviceState) {
            mServiceState = serviceState;
        }

        public void setListener(int events) {
            TelephonyManager tm = getTelephonyManager(getSubId());

            if (tm == null) {
                return;
            }

            if (isEventSet(events, PhoneStateEvents.LISTEN_SERVICE_STATE)) {
                if (mServiceStateListener == null) {
                    mServiceStateListener = new ServiceStateListener();
                } else {
                    tm.unregisterTelephonyCallback(mServiceStateListener);
                }
                tm.registerTelephonyCallback(mHandler, mServiceStateListener);
            }

            if (isEventSet(events, PhoneStateEvents.LISTEN_CALL_STATE)) {
                if (mCallStateListener == null) {
                    mCallStateListener = new CallStateListener();
                } else {
                    tm.unregisterTelephonyCallback(mCallStateListener);
                }
                tm.registerTelephonyCallback(mHandler, mCallStateListener);
            }

            if (isEventSet(events, PhoneStateEvents.LISTEN_PRECISE_CALL_STATE)) {
                if (mPreciseCallStateListener == null) {
                    mPreciseCallStateListener = new PreciseCallStateListener();
                } else {
                    tm.unregisterTelephonyCallback(mPreciseCallStateListener);
                }
                tm.registerTelephonyCallback(mHandler, mPreciseCallStateListener);
            }

            if (isEventSet(events, PhoneStateEvents.LISTEN_SRVCC_STATE_CHANGED)) {
                if (mSrvccStateListener == null) {
                    mSrvccStateListener = new SrvccStateListener();
                } else {
                    tm.unregisterTelephonyCallback(mSrvccStateListener);
                }
                tm.registerTelephonyCallback(mHandler, mSrvccStateListener);
            }

            if (isEventSet(events, PhoneStateEvents.LISTEN_CELL_INFO)) {
                if (mCellInfoListener == null) {
                    mCellInfoListener = new CellInfoListener();
                } else {
                    tm.unregisterTelephonyCallback(mCellInfoListener);
                }
                tm.registerTelephonyCallback(mHandler, mCellInfoListener);
            }

            if (isEventSet(events, PhoneStateEvents.LISTEN_SIGNAL_STRENGTHS)) {
                if (mSignalStrengthsListener == null) {
                    mSignalStrengthsListener = new SignalStrengthsListener();
                } else {
                    tm.unregisterTelephonyCallback(mSignalStrengthsListener);
                }
                tm.registerTelephonyCallback(mHandler, mSignalStrengthsListener);
            }

            if (isEventSet(events, PhoneStateEvents.LISTEN_PRECISE_DATA_CONNECTION_STATE)) {
                if (mPreciseDataConnectionStateListener == null) {
                    mPreciseDataConnectionStateListener = new PreciseDataConnectionStateListener();
                } else {
                    tm.unregisterTelephonyCallback(mPreciseDataConnectionStateListener);
                }
                tm.registerTelephonyCallback(mHandler, mPreciseDataConnectionStateListener);
            }
        }

        public void removeListener() {
            TelephonyManager tm = getTelephonyManager(getSubId());

            if (tm == null) {
                return;
            }

            if (mServiceStateListener != null) {
                tm.unregisterTelephonyCallback(mServiceStateListener);
            }

            if (mCallStateListener != null) {
                tm.unregisterTelephonyCallback(mCallStateListener);
            }

            if (mPreciseCallStateListener != null) {
                tm.unregisterTelephonyCallback(mPreciseCallStateListener);
            }

            if (mSrvccStateListener != null) {
                tm.unregisterTelephonyCallback(mSrvccStateListener);
            }

            if (mCellInfoListener != null) {
                tm.unregisterTelephonyCallback(mCellInfoListener);
            }

            if (mSignalStrengthsListener != null) {
                tm.unregisterTelephonyCallback(mSignalStrengthsListener);
            }

            if (mPreciseDataConnectionStateListener != null) {
                tm.unregisterTelephonyCallback(mPreciseDataConnectionStateListener);
            }
        }

        private final class CallStateListener extends TelephonyCallback implements
                TelephonyCallback.CallStateListener {
            @Override
            public void onCallStateChanged(int state) {
                PhoneStateListener.this.onCallStateChanged(state, null);
            }
        }

        private final class CellInfoListener extends TelephonyCallback implements
                TelephonyCallback.CellInfoListener {
            @Override
            public void onCellInfoChanged(List<CellInfo> cellInfo) {
                PhoneStateListener.this.onCellInfoChanged(cellInfo);
            }
        }

        private final class PreciseCallStateListener extends TelephonyCallback implements
                TelephonyCallback.PreciseCallStateListener {
            @Override
            public void onPreciseCallStateChanged(PreciseCallState callState) {
                PhoneStateListener.this.onPreciseCallStateChanged(callState);
            }
        }

        private final class ServiceStateListener extends TelephonyCallback implements
                TelephonyCallback.ServiceStateListener {
            @Override
            public void onServiceStateChanged(ServiceState serviceState) {
                PhoneStateListener.this.onServiceStateChanged(serviceState);
            }
        }

        private final class SignalStrengthsListener extends TelephonyCallback implements
                TelephonyCallback.SignalStrengthsListener {
            @Override
            public void onSignalStrengthsChanged(SignalStrength signalStrength) {
                PhoneStateListener.this.onSignalStrengthsChanged(signalStrength);
            }
        }

        public final class SrvccStateListener extends TelephonyCallback implements
                TelephonyCallback.SrvccStateListener {
            @Override
            public void onSrvccStateChanged(int state) {
                PhoneStateListener.this.onSrvccStateChanged(state);
            }
        }

        private final class PreciseDataConnectionStateListener extends TelephonyCallback implements
                TelephonyCallback.PreciseDataConnectionStateListener {
            @Override
            public void onPreciseDataConnectionStateChanged(
                    PreciseDataConnectionState dataConnectionState) {
                PhoneStateListener.this.onPreciseDataConnectionStateChanged(dataConnectionState);
            }
        }

        private int getChangedStates(ServiceState oldSS, ServiceState newSS) {
            if ((oldSS == null) || (newSS == null)) {
                return SS_ALL;
            }

            int changedStates = 0;

            if (oldSS.getState() != newSS.getState()) {
                changedStates |= SS_VOICE_REG_STATE;
            }

            if (getVoiceAccessNetworkTechnology(oldSS) != getVoiceAccessNetworkTechnology(newSS)) {
                changedStates |= SS_VOICE_RAT;
            }

            if (getDataRegistrationState(oldSS) != getDataRegistrationState(newSS)) {
                changedStates |= SS_DATA_REG_STATE;
            }

            if (getDataAccessNetworkTechnology(oldSS) != getDataAccessNetworkTechnology(newSS)) {
                changedStates |= SS_DATA_RAT;
            }

            if (oldSS.getRoaming() != newSS.getRoaming()) {
                changedStates |= SS_ROAMING;
            }

            if (!Objects.deepEquals(oldSS.getOperatorNumeric(), newSS.getOperatorNumeric())) {
                changedStates |= SS_NETWORK_OPERATOR;
            }

            return changedStates;
        }

        private int getDataNetwork() {
            TelephonyManager tm = getTelephonyManager(getSubId());
            if (tm != null) {
                return tm.getDataNetworkType();
            }

            return TelephonyManager.NETWORK_TYPE_UNKNOWN;
        }

        private void updateCellularDataRAT(ServiceState serviceState) {
            if (serviceState == null) {
                return;
            }

            int dataRAT = getDataNetwork();

            if (dataRAT == TelephonyManager.NETWORK_TYPE_IWLAN) {
                NetworkRegistrationInfo networkRegInfo = serviceState.getNetworkRegistrationInfo(
                        NetworkRegistrationInfo.DOMAIN_PS,
                        AccessNetworkConstants.TRANSPORT_TYPE_WWAN);

                mCellularDataRAT = (networkRegInfo == null || !networkRegInfo.isRegistered())
                        ? TelephonyManager.NETWORK_TYPE_UNKNOWN
                        : networkRegInfo.getAccessNetworkTechnology();
            }
        }

        private boolean isDisposed() {
            return mDisposed;
        }
    }

    private final class SubscriptionListenerProxy extends SubscriptionListener {
        SubscriptionListenerProxy() {
        }

        @Override
        public void onSimLoadCompleted(int slotId) {
            listenForSimLoadCompleted(slotId);
        }

        @Override
        public void onDefaultSubscriptionChanged(int subId) {
            listenForSubscriptionChanged(subId);
        }

        @Override
        public void onDefaultDataSubscriptionChanged(int subId) {
            listenForSubscriptionChanged(subId);
        }
    }
}
