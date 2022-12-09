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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.TelephonyIntents;

import java.util.HashMap;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

/**
 * This class provides the APIs to manage and control Multi-SIM state.
 *
 * Settings
 * MULTI_SIM_DEFAULT_SUBSCRIPTION
 * MULTI_SIM_VOICE_CALL_SUBSCRIPTION
 * MULTI_SIM_DATA_CALL_SUBSCRIPTION
 *
 * TelephonyManager
 * ACTION_DEFAULT_SUBSCRIPTION_CHANGED
 * ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED
 *
 * Oem Intent : SIM switching button
 * ACTION_DUAL_SIM_SWITCHING_KEY_PRESSED
 * ACTION_DUAL_SIM_SWITCHING_DONE
 */
public final class SubscriptionAgent implements ISubscription {
    /** Arguments - String: iccState, int: subId */
    private static final int EVENT_SIM_STATE_CHANGED = 1001;
    /** Arguments - int: subId, int: phoneId */
    private static final int EVENT_DEFAULT_SUBSCRIPTION_CHANGED = 1002;
    /** Arguments - int: subId */
    private static final int EVENT_DEFAULT_DATA_SUBSCRIPTION_CHANGED = 1003;
    /** Arguments - int: subId, int: phoneId */
    private static final int EVENT_CARRIER_CONFIG_CHANGED = 1004;

    private static SubscriptionAgent sSubsAgent = new SubscriptionAgent();
    private Context mContext;
    private final SubscriptionHandler mHandler = new SubscriptionHandler();
    private final SubscriptionReceiver mReceiver = new SubscriptionReceiver();
    private final SimState mSimState = new SimState();
    private final Set<SubscriptionListener> mSubscriptionListeners
            = new CopyOnWriteArraySet<SubscriptionListener>();
    private DataSubscriptionObserver mDataSubObserver = null;

    private int mPhoneId = MSimUtils.DEFAULT_PHONE_ID;
    private int mSlotId = MSimUtils.INVALID_PHONE_ID;
    private int mSubId = MSimUtils.DEFAULT_SUB_ID;

    public static SubscriptionAgent getInstance() {
        return sSubsAgent;
    }

    @Override
    public void init(Context context) {
        if (context == null) {
            return;
        }

        mContext = context;

        mContext.registerReceiver(mReceiver, mReceiver.getFilter(), Context.RECEIVER_EXPORTED);

        int subId = MSimUtils.getImsDefaultSubId();

        if (subId != MSimUtils.INVALID_SUB_ID) {
            setSubIdAndPhoneId(subId);
        }

        if (MSimUtils.isMultiSimEnabled()) {
            mDataSubObserver = new DataSubscriptionObserver(mHandler);
            mContext.getContentResolver().registerContentObserver(
                    Settings.Global.getUriFor(
                    Settings.Global.MULTI_SIM_DATA_CALL_SUBSCRIPTION),
                    false, mDataSubObserver);
        }

        updateLatestSimState();
    }

    @Override
    public void cleanup() {
        if (mReceiver != null && mContext != null) {
            mContext.unregisterReceiver(mReceiver);
        }

        if (mDataSubObserver != null) {
            if (mContext != null) {
                mContext.getContentResolver().unregisterContentObserver(mDataSubObserver);
            }
            mDataSubObserver = null;
        }
    }

    @Override
    public void addListener(SubscriptionListener listener) {
        mSubscriptionListeners.add(listener);
    }

    @Override
    public void removeListener(SubscriptionListener listener) {
        mSubscriptionListeners.remove(listener);
    }

    @Override
    public int getActiveSubId() {
        int subId = getSubId();

        if (!MSimUtils.isValidSubId(subId)) {
            return MSimUtils.getImsDefaultSubId();
        } else if (subId == MSimUtils.DEFAULT_SUB_ID) {
            int defaultSubId = MSimUtils.getImsDefaultSubId();

            if (MSimUtils.isValidSubId(defaultSubId)) {
                return defaultSubId;
            }
        }

        return subId;
    }

    @Override
    public int getPhoneId() {
        return mPhoneId;
    }

    @Override
    public int getPhoneId(int slotId) {
        // FIXME: phone-id and slot-id is same.
        return slotId; //MSimUtils.getPhoneId(getSubId(slotId), slotId);
    }

    @Override
    public int getSlotId() {
        return mSlotId;
    }

    @Override
    public int getSubId() {
        return mSubId;
    }

    @Override
    public int getSubId(int slotId) {
        return MSimUtils.getSubId(slotId);
    }

    @Override
    public boolean isAllSimAbsent() {
        boolean allSimAbsent = true;
        int activeSimCount = MSimUtils.getActiveSimCount();

        for (int i = 0; i < activeSimCount; i++) {
            if (!isSimAbsent(i)) {
                allSimAbsent = false;
                break;
            }
        }

        return allSimAbsent;
    }

    @Override
    public boolean isAllSimAbsentOrLocked() {
        boolean allSimAbsentOrLocked = true;
        int activeSimCount = MSimUtils.getActiveSimCount();

        for (int i = 0; i < activeSimCount; i++) {
            if (!isSimAbsent(i) && !isSimLocked(i)) {
                allSimAbsentOrLocked = false;
                break;
            }
        }

        return allSimAbsentOrLocked;
    }

    @Override
    public boolean isSimAbsent(int slotId) {
        return mSimState.isAbsent(slotId);
    }

    @Override
    public boolean isSimLoaded(int slotId) {
        return mSimState.isLoaded(slotId);
    }

    @Override
    public boolean isSimLocked(int slotId) {
        return mSimState.isLocked(slotId);
    }

    @Override
    public boolean isSimLoadCompleted(int slotId) {
        return mSimState.isLoadCompleted(slotId);
    }

    @Override
    public boolean isSimLoadCompleted() {
        return mSimState.isLoadCompleted();
    }

    @Override
    public String getSimState() {
        return mSimState.getState(getSlotId());
    }

    private SubscriptionAgent() {
    }

    private void checkAndSetSimState(int slotId, String iccState) {
        if (iccState.equals(mSimState.getState(slotId))) {
            // No state changed
            return;
        }

        if (!isIccStateLoadedOrAbsent(iccState) && !isOtherSimStateRequired()) {
            mSimState.setState(slotId, iccState);
            setStateForMultiSim(slotId);
            return;
        }

        boolean isSimLoadCompleted = false;

        mSimState.setState(slotId, iccState);
        setStateForMultiSim(slotId);

        if (isMultiImsEnabled()) {
            isSimLoadCompleted = mSimState.isLoadCompleted(slotId);
        } else {
            if (mSimState.isLoadCompleted()) {
                isSimLoadCompleted = true;
            } else {
                // Checks if any SIM is deactivated by the settings in multi-SIM scenario.
                if (MSimUtils.isMultiSimEnabled()) {
                    String stateSim1 = mSimState.getState(SimState.SIM1);
                    String stateSim2 = mSimState.getState(SimState.SIM2);

                    isSimLoadCompleted = isIccLoadCompleted(stateSim1, stateSim2);
                }
            }
        }

        if (isSimLoadCompleted) {
            if (!isMultiImsEnabled()) {
                int slotIdFromSubId = MSimUtils.getSlotId(getSubId());

                ImsLog.i(mSlotId, "notifySimLoadCompleted - slotIdFromSubId=" + slotIdFromSubId);

                slotId = isValidSlotId(slotIdFromSubId) ? slotIdFromSubId : slotId;
                // Set Slot ID if the sim is locked
                if (isSimLocked(slotId)) {
                    setSlotId(slotId);
                }
            }

            notifySimLoadCompleted(slotId);
        }
    }

    private boolean isDdsSimLoadCompleted(String stateSim1, String stateSim2) {
        boolean sim1Loaded = isIccStateLoaded(stateSim1);
        boolean sim2Loaded = isIccStateLoaded(stateSim2);
        int simSlotId = sim1Loaded ? SimState.SIM1 : (sim2Loaded ? SimState.SIM2 : -1);

        if ((simSlotId != (-1))
                && (simSlotId == MSimUtils.getSlotId(MSimUtils.getDefaultDataSubId()))) {
            ImsLog.i(mSlotId, "SIM" + simSlotId + " is loaded & DDS");
            return true;
        }

        return false;
    }

    private boolean isIccLoadCompleted(String stateSim1, String stateSim2) {
        ImsLog.i(mSlotId, "isIccLoadCompleted :: SIM0=" + stateSim1 + ", SIM1=" + stateSim2);

        boolean iccLoadCompleted = false;

        if (isIccStateLoadedOrAbsent(stateSim1)
                && isIccStateNotReady(stateSim2)) {
            int simSlotId = MSimUtils.getSlotId(MSimUtils.getDefaultDataSubId());

            if ((simSlotId == SimState.SIM1)
                    && !isIccStateAbsent(stateSim1)) {
                iccLoadCompleted = true;
                ImsLog.i(mSlotId, "SIM0 is DDS, SIM1 is deactivated");
            }
        } else if (isIccStateLoadedOrAbsent(stateSim2)
                && isIccStateNotReady(stateSim1)) {
            int simSlotId = MSimUtils.getSlotId(MSimUtils.getDefaultDataSubId());

            if ((simSlotId == SimState.SIM2)
                    && !isIccStateAbsent(stateSim2)) {
                iccLoadCompleted = true;
                ImsLog.i(mSlotId, "SIM1 is DDS, SIM0 is deactivated");
            }
        } else if (isIccStateLoaded(stateSim1) && isIccStateLocked(stateSim2)) {
            int simSlotId = MSimUtils.getSlotId(MSimUtils.getDefaultDataSubId());

            if (simSlotId == SimState.SIM1) {
                iccLoadCompleted = true;
                ImsLog.i(mSlotId, "SIM0 is DDS, SIM1 is locked");
            }
        } else if (isIccStateLoaded(stateSim2) && isIccStateLocked(stateSim1)) {
            int simSlotId = MSimUtils.getSlotId(MSimUtils.getDefaultDataSubId());

            if (simSlotId == SimState.SIM2) {
                iccLoadCompleted = true;
                ImsLog.i(mSlotId, "SIM1 is DDS, SIM0 is locked");
            }
        } else {
            iccLoadCompleted = isDdsSimLoadCompleted(stateSim1, stateSim2);
        }

        return iccLoadCompleted;
    }

    private boolean isSubscriptionChanged(int subId) {
        return mSubId != subId;
    }

    private void reselectPhoneAndSubscription(int subId) {
        if (!isSubscriptionChanged(subId)) {
            ImsLog.i(mSlotId, "Subscription is not changed");
            int phoneId = MSimUtils.getPhoneId(getSubId());
            int slotId = MSimUtils.getSlotId(getSubId());
            if ((phoneId != getPhoneId()) || (slotId != getSlotId())) {
                ImsLog.i(mSlotId, "Phone/Slot is mismatched; update phoneId/SlotId");
                setSubIdAndPhoneId(subId);

            }
            return;
        }

        setSubIdAndPhoneId(subId);
    }

    private void setPhoneId(int phoneId) {
        if (mPhoneId != phoneId) {
            ImsLog.i(mSlotId, "setPhoneId :: " + mPhoneId + " >> " + phoneId);
            mPhoneId = phoneId;
        }
    }

    private void setSlotId(int slotId) {
        if (mSlotId != slotId) {
            ImsLog.i(mSlotId, "setSlotId :: " + mSlotId + " >> " + slotId);
            mSlotId = slotId;
        }
    }

    private void setSubId(int subId) {
        if (mSubId != subId) {
            ImsLog.i(mSlotId, "setSubId :: " + mSubId + " >> " + subId);
            mSubId = subId;
        }
    }

    private void setSubIdAndPhoneId(int subId) {
        setSubIdAndPhoneId(subId, MSimUtils.INVALID_PHONE_ID);
    }

    private void setSubIdAndPhoneId(int subId, int slotId) {
        int phoneId = slotId;

        if (slotId == MSimUtils.INVALID_PHONE_ID) {
            // Assume that phoneId & slotId is same in this moment.
            slotId = MSimUtils.getSlotId(subId);

            if (slotId == MSimUtils.INVALID_PHONE_ID) {
                int defaultPhoneId = MSimUtils.isMultiSimEnabled() ?
                        MSimUtils.INVALID_PHONE_ID : MSimUtils.DEFAULT_PHONE_ID;
                phoneId = MSimUtils.getPhoneId(subId, defaultPhoneId);
                slotId = phoneId;
            } else {
                phoneId = slotId;
            }
        }

        setSubId(subId);
        setPhoneId(phoneId);

        int oldSlotId = getSlotId();
        setSlotId(slotId);
    }

    private void setStateForMultiSim(int slotId) {
        if (!MSimUtils.isMultiSimEnabled()) {
            return;
        }

        int otherSlotId = (slotId == SimState.SIM1) ? SimState.SIM2 : SimState.SIM1;

        if (MSimUtils.hasIccCard(otherSlotId)) {
            String otherIccState = MSimUtils.getSimState(otherSlotId);
            ImsLog.i(mSlotId, "SIM" + otherSlotId + " :: " + otherIccState);
            mSimState.setState(otherSlotId, otherIccState);
        } else {
            mSimState.setState(otherSlotId, IccCardConstants.INTENT_VALUE_ICC_ABSENT);
        }
    }

    private void notifyCarrierConfigChanged(int subId, int phoneId) {
        for (SubscriptionListener l : mSubscriptionListeners) {
            l.onCarrierConfigChanged(phoneId, subId);
        }
    }

    private void notifyDefaultDataSubscriptionChanged(int subId) {
        for (SubscriptionListener l : mSubscriptionListeners) {
            l.onDefaultDataSubscriptionChanged(subId);
        }
    }

    private void notifyDefaultSubscriptionChanged(int subId) {
        for (SubscriptionListener l : mSubscriptionListeners) {
            l.onDefaultSubscriptionChanged(subId);
        }
    }

    private void notifySimLoadCompleted(int slotId) {
        for (SubscriptionListener l : mSubscriptionListeners) {
            l.onSimLoadCompleted(slotId);
        }
    }

    private void onDataSubscriptionSettingsChanged() {
        ImsLog.i(mSlotId, "onDataSubscriptionSettingsChanged");

        int subId = MSimUtils.getDefaultDataSubId();

        if (subId != getSubId()) {
            onDefaultDataSubscriptionChanged(subId);
        }
    }

    private void onDefaultDataSubscriptionChanged(int subId) {
        ImsLog.i(mSlotId, "onDefaultDataSubscriptionChanged :: subId=" + subId);

        int defaultDataSubId = MSimUtils.getDefaultDataSubId();

        if (subId != defaultDataSubId) {
            ImsLog.w(mSlotId, "Mismatch :: defaultDataSubId=" + defaultDataSubId + ", subId=" + subId);
            return;
        }

        int oldSubId = getSubId();
        int oldPhoneId = getPhoneId();

        reselectPhoneAndSubscription(subId);

        // FIXME: bypass or check duplication?
        // Notify DDS change if subId / phoneId changes.
        if ((oldSubId != getSubId()) || (oldPhoneId != getPhoneId())) {
            notifyDefaultDataSubscriptionChanged(getSubId());
        }
    }

    private void onDefaultSubscriptionChanged(int subId, int phoneId) {
        ImsLog.i(mSlotId, "onDefaultSubscriptionChanged :: subId=" + subId + ", phoneId=" + phoneId);

        if (MSimUtils.isMultiSimEnabled()) {
            int defaultDataSubId = MSimUtils.getDefaultDataSubId();

            if (subId != defaultDataSubId) {
                ImsLog.w(mSlotId, "Mismatch :: defaultDataSubId=" + defaultDataSubId + ", subId=" + subId);
                return;
            }
        }

        int oldSubId = getSubId();
        int oldPhoneId = getPhoneId();

        if (subId != MSimUtils.INVALID_SUB_ID) {
            setSubIdAndPhoneId(subId);
        } else {
            // Is it hot swap?
        }

        // FIXME: bypass or check duplication?
        if ((oldSubId != getSubId()) || (oldPhoneId != getPhoneId())) {
            notifyDefaultSubscriptionChanged(getSubId());
        }
    }

    private void onSimStateChanged(String iccState, int subId, int slotId) {
        boolean isMultiSimEnabled = MSimUtils.isMultiSimEnabled();

        if (isMultiSimEnabled) {
            if (subId == MSimUtils.INVALID_SUB_ID) {
                if (slotId == MSimUtils.INVALID_PHONE_ID) {
                    ImsLog.w(mSlotId, "onSimStateChanged :: No subId; Ignore it");
                    return;
                } else {
                    ImsLog.w(mSlotId, "onSimStateChanged :: No subId, but valid slotId");
                }
            }
        } else if (subId == MSimUtils.INVALID_SUB_ID) {
            subId = MSimUtils.getSubId(MSimUtils.DEFAULT_PHONE_ID);
        }

        if (slotId == MSimUtils.INVALID_PHONE_ID) {
            slotId = MSimUtils.getSlotId(subId);
        }

        // ISSUE: SIM activation/deactivation procedure of MTK chipset
        if (isMultiSimEnabled) {
            if (isSimLoadedBySimActivation(mSimState.getState(slotId), iccState, slotId)) {
                ImsLog.i(mSlotId, "SimActivation :: " + iccState + " >> LOADED");
                iccState = IccCardConstants.INTENT_VALUE_ICC_LOADED;
            }
        }

        boolean hasIccCard1 = MSimUtils.hasIccCard(SimState.SIM1);
        boolean hasIccCard2 = MSimUtils.hasIccCard(SimState.SIM2);

        ImsLog.i(mSlotId, "onSimStateChanged :: state=" + iccState
                + ", subId=" + subId + ", slotId=" + slotId
                + ", card1=" + hasIccCard1 + ", card2=" + hasIccCard2);

        if (!isValidSlotId(slotId)) {
            if (isIccStateLoadedOrAbsent(iccState)) {
                // Mark SIM1 as absent...
                if (!hasIccCard1) {
                    checkAndSetSimState(SimState.SIM1, iccState);
                }

                if (isMultiSimEnabled && !hasIccCard2) {
                    checkAndSetSimState(SimState.SIM2, iccState);
                }
            }

            // If slot id is invalid, ignore the event
            return;
        }

        // If device uses single sim and icc state is absent,
        // the subId will be set to SIM's subscription id.
        if (!isMultiSimEnabled) {
            if (isIccStateAbsentOrLockedInSingleSim(iccState)) {
                setSubIdAndPhoneId(subId, slotId);
            } else if (isIccStateLoaded(iccState) && MSimUtils.isValidSubId(subId)) {
                setSubId(subId);
            }
        }

        checkAndSetSimState(slotId, iccState);
    }

    private void updateLatestSimState() {
        Intent intent = mContext.registerReceiver(null,
                new IntentFilter(TelephonyIntents.ACTION_SIM_STATE_CHANGED),
                Context.RECEIVER_EXPORTED);

        if (intent != null) {
            String iccState = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);

            if (iccState != null) {
                int defaultSlotId = MSimUtils.isMultiSimEnabled() ?
                        MSimUtils.INVALID_SLOT_ID : MSimUtils.DEFAULT_SLOT_ID;
                int slotId = intent.getIntExtra(SubscriptionManager.EXTRA_SLOT_INDEX,
                        defaultSlotId);
                int subId = intent.getIntExtra(SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX,
                        MSimUtils.INVALID_SUB_ID);

                ImsLog.i(mSlotId, "IccState :: state=" + iccState
                        + ", subId=" + subId + ", slotId=" + slotId);

                onSimStateChanged(iccState, subId, slotId);
            }
        }
    }

    private static boolean isIccStateLoadedOrAbsent(String iccState) {
        return (isIccStateLoaded(iccState)
                || isIccStateAbsent(iccState) || isIccStateLocked(iccState));
    }

    private static boolean isIccStateAbsentOrLockedInSingleSim(String iccState) {
        return (isIccStateAbsent(iccState) || isIccStateLocked(iccState));
    }

    private static boolean isIccStateAbsent(String iccState) {
        return IccCardConstants.INTENT_VALUE_ICC_ABSENT.equals(iccState);
    }

    private static boolean isIccStateLoaded(String iccState) {
        return IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(iccState);
    }

    private static boolean isIccStateNotReady(String iccState) {
        return IccCardConstants.INTENT_VALUE_ICC_NOT_READY.equals(iccState);
    }

    private static boolean isIccStateLocked(String iccState) {
        return IccCardConstants.INTENT_VALUE_ICC_LOCKED.equals(iccState);
    }

    private static boolean isMultiImsEnabled() {
        return MSimUtils.isMultiImsEnabled();
    }

    private static boolean isOtherSimStateRequired() {
        if (MSimUtils.isMultiSimEnabled()) {
            return true;
        } else {
            return false;
        }
    }

    public static boolean isSimLoadedBySimActivation(
            String oldState, String newState, int slotId) {
        return false;
    }

    private static boolean isValidSlotId(int slotId) {
        return (slotId == SimState.SIM1) || (slotId == SimState.SIM2);
    }

    private static final class SimState {
        public static final int SIM1 = 0;
        public static final int SIM2 = 1;

        public HashMap<Integer, String> mState = new HashMap<Integer, String>();

        public SimState() {
        }

        public String getState(int slotId) {
            if (!isValidSlotId(slotId)) {
                return IccCardConstants.INTENT_VALUE_ICC_UNKNOWN;
            }

            return mState.get(slotId);
        }

        public boolean isAbsent(int slotId) {
            String state = getState(slotId);
            return (state != null) && state.equals(IccCardConstants.INTENT_VALUE_ICC_ABSENT);
        }

        public boolean isLoaded(int slotId) {
            String state = getState(slotId);
            return (state != null) && state.equals(IccCardConstants.INTENT_VALUE_ICC_LOADED);
        }

        public boolean isLocked(int slotId) {
            String state = getState(slotId);
            return (state != null) && state.equals(IccCardConstants.INTENT_VALUE_ICC_LOCKED);
        }

        public boolean isLoadCompleted() {
            String stateSim1 = getState(SIM1);

            if (MSimUtils.isMultiSimEnabled()) {
                String stateSim2 = getState(SIM2);
                return (mState.size() > 1)
                        && isIccStateLoadedOrAbsent(stateSim1)
                        && isIccStateLoadedOrAbsent(stateSim2);
            } else {
                return isIccStateLoadedOrAbsent(stateSim1);
            }
        }

        public boolean isLoadCompleted(int slotId) {
            return isIccStateLoadedOrAbsent(getState(slotId));
        }

        public void removeState(int slotId) {
            if (!isValidSlotId(slotId)) {
                return;
            }

            mState.remove(Integer.valueOf(slotId));
        }

        public void setState(int slotId, String state) {
            if (!isValidSlotId(slotId)) {
                return;
            }

            mState.put(slotId, state);
        }
    }

    private final class SubscriptionHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            ImsLog.i(mSlotId, "SubscriptionHandler :: msg=" + msg.what);

            switch (msg.what) {
            case EVENT_SIM_STATE_CHANGED: {
                String iccState = (String)msg.obj;

                if (iccState == null) {
                    break;
                }

                onSimStateChanged(iccState, msg.arg1, msg.arg2);
                break;
            }
            case EVENT_DEFAULT_SUBSCRIPTION_CHANGED: {
                onDefaultSubscriptionChanged(msg.arg1, msg.arg2);
                break;
            }
            case EVENT_DEFAULT_DATA_SUBSCRIPTION_CHANGED: {
                onDefaultDataSubscriptionChanged(msg.arg1);
                break;
            }
            case EVENT_CARRIER_CONFIG_CHANGED: {
                notifyCarrierConfigChanged(msg.arg1, msg.arg2);
                break;
            }

            default:
                break;
            }
        }
    }

    private final class SubscriptionReceiver extends BroadcastReceiver {
        IntentFilter mIntentFilter = new IntentFilter();

        public SubscriptionReceiver() {
            mIntentFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
            mIntentFilter.addAction(SubscriptionManager.ACTION_DEFAULT_SUBSCRIPTION_CHANGED);
            mIntentFilter.addAction(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED);

            if (MSimUtils.isMultiSimEnabled()) {
                mIntentFilter.addAction(TelephonyManager.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED);
            }
        }

        public IntentFilter getFilter() {
            return mIntentFilter;
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            ImsLog.i(mSlotId, ImsLog.lastSubString(action, "."));

            if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(action)) {
                String iccState = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);

                if (iccState == null) {
                    ImsLog.w(mSlotId, "onSimStateChanged :: IccState is null");
                    return;
                }
                int defaultSlotId = MSimUtils.isMultiSimEnabled() ?
                        MSimUtils.INVALID_SLOT_ID : MSimUtils.DEFAULT_SLOT_ID;
                int slotId = intent.getIntExtra(
                        SubscriptionManager.EXTRA_SLOT_INDEX, defaultSlotId);
                int subId = intent.getIntExtra(
                        SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX, MSimUtils.INVALID_SUB_ID);

                Message.obtain(mHandler,
                    EVENT_SIM_STATE_CHANGED, subId, slotId, iccState).sendToTarget();
            } else if (SubscriptionManager.ACTION_DEFAULT_SUBSCRIPTION_CHANGED.equals(action)) {
                int subId = intent.getIntExtra(
                        SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX, MSimUtils.INVALID_SUB_ID);
                int slotId = intent.getIntExtra(SubscriptionManager.EXTRA_SLOT_INDEX,
                        MSimUtils.INVALID_SLOT_ID);

                Message.obtain(mHandler,
                    EVENT_DEFAULT_SUBSCRIPTION_CHANGED, subId, slotId).sendToTarget();
            } else if (TelephonyManager.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED.equals(action)) {
                int subId = intent.getIntExtra(
                        SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX, MSimUtils.INVALID_SUB_ID);

                Message.obtain(mHandler,
                    EVENT_DEFAULT_DATA_SUBSCRIPTION_CHANGED, subId, 0).sendToTarget();
            } else if (CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED.equals(action)) {
                int slotId = intent.getIntExtra(CarrierConfigManager.EXTRA_SLOT_INDEX,
                        MSimUtils.INVALID_SLOT_ID);
                int subId = intent.getIntExtra(
                        SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX, MSimUtils.INVALID_SUB_ID);

                Message.obtain(mHandler,
                    EVENT_CARRIER_CONFIG_CHANGED, subId, slotId).sendToTarget();
            }
        }
    }

    private final class DataSubscriptionObserver extends ContentObserver {

        public DataSubscriptionObserver(Handler handler) {
            super(handler);
        }

        @Override
        public void onChange(boolean selfChange) {
            onDataSubscriptionSettingsChanged();
        }
    }
}
