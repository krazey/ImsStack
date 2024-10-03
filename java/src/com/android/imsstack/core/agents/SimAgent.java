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
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.telephony.TelephonyManager;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.SystemServiceProxy.SmsManagerProxy;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsUtils;
import com.android.internal.annotations.VisibleForTesting;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

/**
 * This class provides all the SIM states and its related operations.
 */
public final class SimAgent implements SimInterface {
    /** SIM related notifications */
    @VisibleForTesting
    static final int NOTIFICATION_ISIM_STATE_CHANGED = 101;
    @VisibleForTesting
    static final int NOTIFICATION_ISIM_AUTH = 102;
    @VisibleForTesting
    static final int NOTIFICATION_USIM_AUTH = 201;

    /** Defines event class for asynchronous SIM(USIM/ISIM) authentication */
    static final class AuthEvent {
        public @Sim.AppType int appType;
        public String nonce;
        public long owner;

        AuthEvent(@Sim.AppType int appType, String nonce, long owner) {
            this.appType = appType;
            this.nonce = nonce;
            this.owner = owner;
        }
    }

    private final int mSlotId;
    private int mSubId = MSimUtils.INVALID_SUB_ID;
    private final Handler mSimHandler;
    private final SimStateReceiver mSimStateReceiver;
    /** USIM states */
    private int mSimCardState = Sim.STATE_UNKNOWN;
    private int mSimState = Sim.STATE_UNKNOWN;
    private byte[] mUst; // Cached USIM service table
    private final UsatAgent mUsatAgent;
    private final Set<Sim.Listener> mListeners = new CopyOnWriteArraySet<>();

    /** ISIM states */
    private int mIsimState = Sim.ISIM_STATE_UNKNOWN;
    private byte[] mIsimIst; // Cached ISIM service table
    private String mIsimDomain;
    private String mIsimImpi;
    private final List<String> mIsimImpu = new ArrayList<>();
    private final Set<Sim.IsimListener> mIsimListeners = new CopyOnWriteArraySet<>();
    private NativeStateInterface.Listener mNativeStateListener;

    public SimAgent(int slotId) {
        this(slotId, AppContext.getInstance().getMainLooper());
    }

    @VisibleForTesting
    public SimAgent(int slotId, @NonNull Looper looper) {
        mSlotId = slotId;
        mSimHandler = new Handler(looper);
        mSimStateReceiver = new SimStateReceiver();
        mUsatAgent = new UsatAgent(this);
    }

    @Override
    public void init(Context context) {
        mSubId = MSimUtils.getSubId(getSlotId());

        mSimStateReceiver.register();

        NativeStateInterface nsi =
                AgentFactory.getInstance().getAgent(NativeStateInterface.class, getSlotId());
        if (nsi != null) {
            mNativeStateListener = new NativeStateInterface.Listener() {
                @Override
                public void onNativeServiceReady() {
                    logi(this, "Native service ready.");
                    handleNativeServiceReady();
                }
            };
            nsi.addListener(mNativeStateListener);
        }

        updateSimState();
    }

    @Override
    public void cleanup() {
        ISystem system = getSystem(getSlotId());

        if (system != null) {
            system.notifyIsimState(
                    NOTIFICATION_ISIM_STATE_CHANGED,
                    isimStateToString(Sim.ISIM_STATE_REMOVED));
        }

        if (mNativeStateListener != null) {
            NativeStateInterface nsi =
                    AgentFactory.getInstance().getAgent(NativeStateInterface.class, getSlotId());
            if (nsi != null) {
                nsi.removeListener(mNativeStateListener);
            }
            mNativeStateListener = null;
        }

        mSimStateReceiver.unregister();
        mSimHandler.removeCallbacksAndMessages(null);

        mSubId = MSimUtils.INVALID_SUB_ID;
        mSimCardState = Sim.STATE_UNKNOWN;
        mSimState = Sim.STATE_UNKNOWN;
        clearSimRecords();
        mListeners.clear();

        mIsimState = Sim.ISIM_STATE_UNKNOWN;
        clearIsimRecords();
        mIsimListeners.clear();
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
    public @Sim.State int getSimCardState() {
        return mSimCardState;
    }

    @Override
    public @Sim.State int getSimState() {
        return mSimState;
    }

    @Override
    public boolean isSimLoaded() {
        return mSimState == Sim.STATE_LOADED;
    }

    @Override
    public boolean isSimLoadCompleted() {
        return mSimState == Sim.STATE_LOADED
                || mSimState == Sim.STATE_LOCKED
                || mSimState == Sim.STATE_ABSENT;
    }

    @Override
    public byte[] getUsimServiceTable() {
        return mUst;
    }

    @Override
    public String getSmscAddress() {
        SmsManagerProxy smp = getSmsManagerProxy(getSlotId(), getSubId());

        try {
            if (smp != null) {
                return smp.getSmscAddress();
            }
        } catch (Exception e) {
            logw(this, "getSmscAddress: " + e);
        }
        return null;
    }

    @Override
    public void addListener(Sim.Listener listener) {
        mListeners.add(listener);
    }

    @Override
    public void removeListener(Sim.Listener listener) {
        mListeners.remove(listener);
    }

    @Override
    public UsatInterface getUsatInterface() {
        return mUsatAgent;
    }

    @Override
    public @Sim.IsimState int getIsimState() {
        return mIsimState;
    }

    @Override
    public boolean isIsimLoaded() {
        return mIsimState == Sim.ISIM_STATE_LOADED;
    }

    @Override
    public byte[] getIsimServiceTable() {
        return mIsimIst;
    }

    @Override
    public String getIsimImpi() {
        return mIsimImpi;
    }

    @Override
    public String getIsimDomain() {
        return mIsimDomain;
    }

    @Override
    public @NonNull List<String> getIsimImpu() {
        return mIsimImpu;
    }

    @Override
    public boolean isGbaAvailable() {
        if (mIsimIst != null && mIsimIst.length > 0) {
            return (mIsimIst[0] & 0x02) != 0;
        }

        return false;
    }

    @Override
    public void addIsimListener(Sim.IsimListener listener) {
        mIsimListeners.add(listener);
    }

    @Override
    public void removeIsimListener(Sim.IsimListener listener) {
        mIsimListeners.remove(listener);
    }

    //// System call interface {
    /**
     * Returns the ISIM state as a string.
     */
    public String getIsimStateString() {
        return isimStateToString(getIsimState());
    }

    /**
     * Returns the response of SIM authentication for the specified application type.
     *
     * @param appType The SIM application type, like {@link Sim#APP_TYPE_ISIM}.
     * @param nonce The authentication challenge data, base64 encoded.
     * @param owner The owner of this request.
     */
    public void requestSimAuthentication(@Sim.AppType int appType, String nonce, long owner) {
        final AuthEvent event = new AuthEvent(appType, nonce, owner);

        mSimHandler.post(()-> {
            handleRequestSimAuthentication(event);
        });
    }
    //// }

    private void handleRequestSimAuthentication(AuthEvent event) {
        String response = "";
        TelephonyManagerProxy tmp = getTelephonyManagerProxy(getSlotId(), getSubId());

        if (tmp != null) {
            response = tmp.getIccAuthentication(event.appType,
                    TelephonyManager.AUTHTYPE_EAP_AKA, event.nonce);
        }

        logi(this, "Auth - appType=" + event.appType
                + ", nonce=" + event.nonce
                + ", owner=" + event.owner
                + ", response=" + response);

        ISystem system = getSystem(getSlotId());

        if (system != null) {
            if (event.appType == Sim.APP_TYPE_ISIM) {
                system.notifyIsimAuthenticationResponse(
                        NOTIFICATION_ISIM_AUTH, response, event.owner);
            } else if (event.appType == Sim.APP_TYPE_USIM) {
                system.notifyUsimAuthenticationResponse(
                        NOTIFICATION_USIM_AUTH, response, event.owner);
            }
        }
    }

    private void handleNativeServiceReady() {
        if (isSimLoaded()) {
            logd(this, "SIM is already loaded");
            return;
        }

        updateSimState();
    }

    private void handleSimApplicationStateChanged(int slotId, int subId, int state) {
        logi(this, "handleSimApplicationStateChanged - slotId=" + slotId
                + ", subId=" + subId + ", newState=" + Sim.stateToString(state)
                + ", state=" + Sim.stateToString(getSimState())
                + ", isimState=" + isimStateToString(getIsimState()));

        if (mSlotId == slotId) {
            int simState = Sim.getSimStateFromTelephonySimState(state);

            if (simState != Sim.STATE_INVALID) {
                simState = refineSimState(simState);

                boolean oldSimLoaded = isSimLoaded();

                setSubId(subId);
                setSimState(simState);
                updateIsimStateOnSimStateChanged(oldSimLoaded, isSimLoaded());
            }
        }
    }

    private void handleSimCardStateChanged(int slotId, int subId, int state) {
        logi(this, "handleSimCardStateChanged - slotId=" + slotId
                + ", subId=" + subId + ", newState=" + Sim.stateToString(state)
                + ", state=" + Sim.stateToString(getSimCardState()));

        if (mSlotId == slotId) {
            int simCardState = Sim.getSimCardStateFromTelephonySimState(state);

            if (simCardState != Sim.STATE_INVALID) {
                setSubId(subId);
                setSimCardState(simCardState);
            }
        }
    }

    private void setSubId(int subId) {
        if (mSubId != subId) {
            logi(this, "setSubId: " + mSubId + " >> " + subId);
            mSubId = subId;
        }
    }

    private void notifySimCardStateChanged() {
        for (Sim.Listener l : mListeners) {
            l.onSimCardStateChanged();
        }
    }

    private void setSimCardState(int state) {
        if (mSimCardState != state) {
            logi(this, "SimCardState: "
                    + Sim.stateToString(mSimCardState) + " >> " + Sim.stateToString(state));

            mSimCardState = state;

            // Notifies the applications that the SIM card state is changed.
            notifySimCardStateChanged();
        }
    }

    private void clearSimRecords() {
        mUst = null;
    }

    private void loadSimRecords() {
        TelephonyManagerProxy tmp = getTelephonyManagerProxy(getSlotId(), getSubId());

        if (tmp != null) {
            String serviceTable = tmp.getSimServiceTable(Sim.APP_TYPE_USIM);

            mUst = ImsUtils.hexStringToBytes(serviceTable);

            logi(this, "SimRecords: ust=" + serviceTable);
        }
    }

    private void notifySimStateChanged() {
        for (Sim.Listener l : mListeners) {
            l.onSimStateChanged();
        }
    }

    private int refineSimState(int state) {
        if (state == Sim.STATE_UNKNOWN
                && getSimCardState() == Sim.STATE_ABSENT) {
            // The SIM state is handled as the ABSENT if the SIM card state is in the ABSENT
            // because the UNKNOWN state is used for the initial state and it needs to be
            // differentiated with the non-initial state.
            return Sim.STATE_ABSENT;
        }

        return state;
    }

    private void setSimState(int state) {
        if (mSimState != state) {
            logi(this, "SimState: "
                    + Sim.stateToString(mSimState) + " >> " + Sim.stateToString(state));

            mSimState = state;

            if (isSimLoaded()) {
                loadSimRecords();
            } else if (mSimState == Sim.STATE_UNKNOWN
                    || mSimState == Sim.STATE_ABSENT) {
                clearSimRecords();
            }

            // Notifies the applications that the SIM state is changed.
            notifySimStateChanged();
        }
    }

    @VisibleForTesting
    void updateSimState() {
        TelephonyManagerProxy tmp = getTelephonyManagerProxy(getSlotId(), getSubId());
        int telephonyCardState = Sim.STATE_INVALID;
        int telephonySimState = Sim.STATE_INVALID;

        if (tmp != null) {
            telephonyCardState = tmp.getSimCardState();
            telephonySimState = tmp.getSimApplicationState();
        }

        int simCardState = Sim.getSimCardStateFromTelephonySimState(telephonyCardState);
        int simState = Sim.getSimStateFromTelephonySimState(telephonySimState);

        logd(this, "updateSimState: cardState=" + Sim.stateToString(simCardState)
                + ", simState=" + Sim.stateToString(simState));

        if (simCardState != Sim.STATE_INVALID) {
            setSimCardState(simCardState);
        }

        if (simState != Sim.STATE_INVALID) {
            simState = refineSimState(simState);

            boolean oldSimLoaded = isSimLoaded();
            int oldSimState = getSimState();

            setSimState(simState);

            int newSimState = getSimState();

            logd(this, "updateSimState: "
                    + Sim.stateToString(oldSimState) + " >> " + Sim.stateToString(newSimState));

            if (oldSimState != newSimState) {
                updateIsimStateOnSimStateChanged(oldSimLoaded, isSimLoaded());
            }
        }
    }

    private void clearIsimRecords() {
        mIsimIst = null;
        mIsimImpi = null;
        mIsimDomain = null;
        mIsimImpu.clear();
    }

    private void loadIsimRecords() {
        TelephonyManagerProxy tmp = getTelephonyManagerProxy(getSlotId(), getSubId());

        if (tmp != null) {
            String serviceTable = tmp.getSimServiceTable(Sim.APP_TYPE_ISIM);
            mIsimIst = ImsUtils.hexStringToBytes(serviceTable);
            mIsimDomain = tmp.getIsimDomain();

            try {
                mIsimImpi = tmp.getImsPrivateUserIdentity();
            } catch (RuntimeException e) {
                logw(this, "Reading IMPI failed: " + e.toString());
                mIsimImpi = null;
            }

            List<Uri> uris = Collections.emptyList();
            try {
                uris = tmp.getImsPublicUserIdentities();
            } catch (RuntimeException e) {
                logw(this, "Reading IMPU failed: " + e.toString());
            }

            mIsimImpu.clear();

            for (Uri uri : uris) {
                mIsimImpu.add(uri.toString());
            }

            logi(this, "IsimRecords: ist=" + serviceTable
                    + ", impi=" + ImsLog.hiddenString(mIsimImpi)
                    + ", domain=" + ImsLog.hiddenString(mIsimDomain)
                    + ", impu=" + ImsLog.hiddenString(mIsimImpu.toArray(new String[0])));
        }
    }

    private void notifyIsimStateChanged() {
        for (Sim.IsimListener l : mIsimListeners) {
            l.onIsimStateChanged();
        }
    }

    private void setIsimState(int state) {
        if (mIsimState != state) {
            int newIsimState = state;

            if (state == Sim.ISIM_STATE_REFRESH_COMPLETED) {
                newIsimState = Sim.ISIM_STATE_LOADED;
            }

            logi(this, "IsimState: "
                    + isimStateToString(mIsimState) + " >> " + isimStateToString(newIsimState));

            mIsimState = newIsimState;

            if (isIsimLoaded()) {
                loadIsimRecords();
            } else if (mIsimState == Sim.ISIM_STATE_UNKNOWN
                    || mIsimState == Sim.ISIM_STATE_NOT_PRESENT
                    || mIsimState == Sim.ISIM_STATE_NOT_READY
                    || mIsimState == Sim.ISIM_STATE_REFRESH_STARTED) {
                clearIsimRecords();
            }

            ISystem system = getSystem(getSlotId());

            if (system != null) {
                system.notifyIsimState(
                        NOTIFICATION_ISIM_STATE_CHANGED,
                        isimStateToString(state));
            }

            // Notifies the applications that the ISIM state is changed.
            notifyIsimStateChanged();
        }
    }

    private void updateIsimStateOnSimStateChanged(boolean oldSimLoaded, boolean newSimLoaded) {
        int isimState = getIsimState();
        int newIsimState = getIsimStateFromSimState(getSimState());

        if (oldSimLoaded && !newSimLoaded) {
            if (isimState == Sim.ISIM_STATE_LOADED
                    || isimState == Sim.ISIM_STATE_REFRESH_STARTED) {
                newIsimState = Sim.ISIM_STATE_REFRESH_STARTED;
            }
        } else if (newSimLoaded) {
            if (isimState == Sim.ISIM_STATE_REFRESH_STARTED) {
                newIsimState = Sim.ISIM_STATE_REFRESH_COMPLETED;
            } else {
                TelephonyManagerProxy tmp = getTelephonyManagerProxy(getSlotId(), getSubId());

                if (tmp != null && tmp.isApplicationOnUicc(Sim.APP_TYPE_ISIM)) {
                    newIsimState = Sim.ISIM_STATE_LOADED;
                } else {
                    newIsimState = Sim.ISIM_STATE_NOT_PRESENT;
                }
            }
        }

        setIsimState(newIsimState);
    }

    private void logd(Object o, String s) {
        ImsLog.d(o, getSlotId(), "SIM: " + s);
    }

    private void logi(Object o, String s) {
        ImsLog.i(o, getSlotId(), "SIM: " + s);
    }

    private void logw(Object o, String s) {
        ImsLog.w(o, getSlotId(), "SIM: " + s);
    }

    private final class SimStateReceiver extends BroadcastReceiver {
        public void register() {
            IntentFilter filter = new IntentFilter(
                    TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED);
            filter.addAction(TelephonyManager.ACTION_SIM_CARD_STATE_CHANGED);

            AppContext.getInstance().getBroadcastReceiverProxy()
                    .registerReceiver(this, filter, mSimHandler);
        }

        public void unregister() {
            AppContext.getInstance().getBroadcastReceiverProxy().unregisterReceiver(this);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            logi(this, ImsLog.lastSubString(action, "."));

            if (TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED.equals(action)) {
                handleSimApplicationStateChanged(
                        Sim.getExtraSlotIndex(intent),
                        Sim.getExtraSubscriptionIndex(intent),
                        Sim.getExtraSimState(intent));
            } else if (TelephonyManager.ACTION_SIM_CARD_STATE_CHANGED.equals(action)) {
                handleSimCardStateChanged(
                        Sim.getExtraSlotIndex(intent),
                        Sim.getExtraSubscriptionIndex(intent),
                        Sim.getExtraSimState(intent));
            }
        }
    }

    private static @Sim.IsimState int getIsimStateFromSimState(@Sim.State int state) {
        switch (state) {
            case Sim.STATE_UNKNOWN: // fall through
            case Sim.STATE_ABSENT:
                return Sim.ISIM_STATE_NOT_PRESENT;
            case Sim.STATE_LOADED:
                return Sim.ISIM_STATE_LOADED;
            case Sim.STATE_PRESENT: // fall through
            case Sim.STATE_LOCKED: // fall through
            case Sim.STATE_NOT_READY: // fall through
            default:
                return Sim.ISIM_STATE_NOT_READY;
        }
    }

    private static SmsManagerProxy getSmsManagerProxy(int slotId, int subId) {
        if (!MSimUtils.isValidSubId(subId)) {
            subId = MSimUtils.getSubId(slotId);
        }
        return MSimUtils.isValidSubId(subId) ? AppContext.getSmsManagerProxy(subId) : null;
    }

    private static TelephonyManagerProxy getTelephonyManagerProxy(int slotId, int subId) {
        if (!MSimUtils.isValidSubId(subId)) {
            subId = MSimUtils.getSubId(slotId);
        }
        return MSimUtils.isValidSubId(subId) ? AppContext.getTelephonyManagerProxy(subId) : null;
    }

    private static ISystem getSystem(int slotId) {
        return SystemInterface.getInstance().getSystem(slotId);
    }

    private static String isimStateToString(@Sim.IsimState int state) {
        switch (state) {
            case Sim.ISIM_STATE_NOT_PRESENT:
                return "NOT_PRESENT";
            case Sim.ISIM_STATE_NOT_READY:
                return "NOT_READY";
            case Sim.ISIM_STATE_LOADED:
                return "LOADED";
            case Sim.ISIM_STATE_REFRESH_STARTED:
                return "REFRESH_STARTED";
            case Sim.ISIM_STATE_REFRESH_COMPLETED:
                return "REFRESH_COMPLETED";
            case Sim.ISIM_STATE_REMOVED:
                return "SIM_REMOVED";
            case Sim.ISIM_STATE_UNKNOWN: // fall through
            default:
                return "UNKNOWN";
        }
    }
}
