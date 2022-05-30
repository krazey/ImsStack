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
import android.os.Handler;
import android.os.Message;
import android.telephony.SmsManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.android.imsstack.core.agents.agentif.ISharedState;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;
import com.android.imsstack.util.SimUtils;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

/**
 * This class provides all the SIM states and its related operations.
 */
public class SimAgent implements SimInterface {
    /** Internal events */
    private static final int EVENT_REQUEST_SIM_AUTHENTICATION = 1;
    private static final int EVENT_READ_ISIM_FILE_ATTRIBUTES = 2;
    private static final int EVENT_READ_ISIM_RECORD = 3;
    private static final int EVENT_NATIVE_BOOT_COMPLETED = 4;

    /** SIM related notifications */
    private static final int NOTIFICATION_ISIM_STATE_CHANGED = 102;
    private static final int NOTIFICATION_ISIM_READ_FILE_ATTRIBUTE = 103;
    private static final int NOTIFICATION_ISIM_READ_RECORD = 104;
    private static final int NOTIFICATION_ISIM_AUTH = 105;
    private static final int NOTIFICATION_USIM_AUTH = 106;

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
    private final SimHandler mSimHandler;
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

    public SimAgent(int slotId) {
        mSlotId = slotId;
        mSimHandler = new SimHandler();
        mSimStateReceiver = new SimStateReceiver();
        mUsatAgent = new UsatAgent(this);
    }

    @Override
    public void init(Context context) {
        mSubId = MSimUtils.getSubId(getSlotId());

        mSimStateReceiver.register();

        ISharedState iss = (ISharedState) AgentFactory.getAgent(
                AgentFactory.SHARED_STATE, getSlotId());

        if (iss != null) {
            iss.registerForNativeBootComplete(mSimHandler, EVENT_NATIVE_BOOT_COMPLETED, null);
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

        ISharedState iss = (ISharedState) AgentFactory.getAgent(
                AgentFactory.SHARED_STATE, getSlotId());

        if (iss != null) {
            iss.unregisterForNativeBootComplete(mSimHandler);
        }

        mSimStateReceiver.unregister();
        mSimHandler.removeCallbacksAndMessages(null);

        mSubId = MSimUtils.INVALID_SUB_ID;
        mSimCardState = Sim.STATE_UNKNOWN;
        mSimState = Sim.STATE_UNKNOWN;
        mUst = null;
        mListeners.clear();

        mIsimState = Sim.ISIM_STATE_UNKNOWN;
        mIsimIst = null;
        mIsimDomain = null;
        mIsimImpi = null;
        mIsimImpu.clear();
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
    public byte[] getUsimServiceTable() {
        return mUst;
    }

    @Override
    public String getSmscAddress() {
        SmsManager sm = getSmsManager(getSlotId(), getSubId());

        try {
            return (sm != null) ? sm.getSmscAddress() : null;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
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
     * Reads the file attributes of the specified ISIM record.
     *
     * @param fileId The file id({@link Sim#IsimFileId}) to be read.
     */
    public void readIsimFileAttributes(@Sim.IsimFileId int fileId) {
        Message.obtain(mSimHandler, EVENT_READ_ISIM_FILE_ATTRIBUTES, fileId, 0).sendToTarget();
    }

    /**
     * Reads the value of the specified ISIM record.
     *
     * @param fileId The file id({@link Sim#IsimFileId}) to be read.
     * @param index The index of the record for the given file.
     */
    public void readIsimRecord(@Sim.IsimFileId int fileId, int index) {
        Message.obtain(mSimHandler, EVENT_READ_ISIM_RECORD, fileId, index).sendToTarget();
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

        executeOnIsimThread(()-> {
            handleRequestSimAuthentication(event);
        });
    }
    //// }

    private void handleRequestSimAuthentication(AuthEvent event) {
        String response = "";
        TelephonyManager tm = getTelephonyManager(mSlotId, mSubId);

        if (tm != null) {
            response = tm.getIccAuthentication(event.appType,
                    TelephonyManager.AUTHTYPE_EAP_AKA, event.nonce);
        }

        ImsLog.i(getSlotId(), "[SIM] appType=" + event.appType
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

    private void handleReadIsimFileAttributes(int fileId) {
        String[] fileContent = null;

        switch (fileId) {
            case Sim.ISIM_FILE_ID_IMPI: {
                if (mIsimImpi != null) {
                    fileContent = new String[] { mIsimImpi };
                }
                break;
            }
            case Sim.ISIM_FILE_ID_DOMAIN: {
                if (mIsimDomain != null) {
                    fileContent = new String[] { mIsimDomain };
                }
                break;
            }
            case Sim.ISIM_FILE_ID_IMPU: {
                if (!mIsimImpu.isEmpty()) {
                    fileContent = new String[mIsimImpu.size()];
                    fileContent = mIsimImpu.toArray(fileContent);
                }
                break;
            }
            default: {
                // no-op
                break;
            }
        }

        int recordCount = (fileContent != null) ? fileContent.length : 0;

        ImsLog.i(getSlotId(), "[SIM] ISIM: fileId=" + Integer.toHexString(fileId)
                + ", name=" + isimFileIdToString(fileId)
                + ", count=" + recordCount);

        ISystem system = getSystem(getSlotId());

        if (system != null) {
            system.notifyIsimFileAttributesResponse(
                    NOTIFICATION_ISIM_READ_FILE_ATTRIBUTE,
                    fileId, recordCount, fileContent);
        }
    }

    private void handleReadIsimRecord(int fileId, int recordIndex) {
        String record = null;

        switch (fileId) {
            case Sim.ISIM_FILE_ID_IMPI: {
                record = mIsimImpi;
                break;
            }
            case Sim.ISIM_FILE_ID_DOMAIN: {
                record = mIsimDomain;
                break;
            }
            case Sim.ISIM_FILE_ID_IMPU: {
                if (recordIndex >= 0 && recordIndex < mIsimImpu.size()) {
                    record = mIsimImpu.get(recordIndex);
                }
                break;
            }
            default: {
                // no-op
                break;
            }
        }

        ImsLog.d(getSlotId(), "[SIM] ISIM: fileId=" + Integer.toHexString(fileId)
                + ", name=" + isimFileIdToString(fileId)
                + ", index=" + recordIndex
                + ", record=" + record);

        ISystem system = getSystem(getSlotId());

        if (system != null) {
            system.notifyIsimRecordResponse(
                    NOTIFICATION_ISIM_READ_RECORD, fileId, recordIndex, record);
        }
    }

    private void handleNativeBootCompleted() {
        if (isSimLoaded()) {
            ImsLog.d(getSlotId(), "[SIM] SIM is already loaded");
            return;
        }

        updateSimState();
    }

    private void handleSimApplicationStateChanged(int slotId, int subId, int state) {
        ImsLog.i(getSlotId(), "[SIM] handleSimApplicationStateChanged - slotId=" + slotId
                + ", subId=" + subId + ", newState=" + simStateToString(state)
                + ", state=" + simStateToString(getSimState())
                + ", isimState=" + isimStateToString(getIsimState()));

        if (mSlotId != slotId) {
            return;
        }

        int simState = getSimStateFromTelephonySimState(state);

        if (simState == Sim.STATE_INVALID) {
            return;
        }

        simState = refineSimState(simState);

        boolean oldSimLoaded = isSimLoaded();

        setSubId(subId);
        setSimState(simState);
        updateIsimStateOnSimStateChanged(oldSimLoaded, isSimLoaded());
    }

    private void handleSimCardStateChanged(int slotId, int subId, int state) {
        ImsLog.i(getSlotId(), "[SIM] handleSimCardStateChanged - slotId=" + slotId
                + ", subId=" + subId + ", newState=" + simStateToString(state)
                + ", state=" + simStateToString(getSimCardState()));

        if (mSlotId != slotId) {
            return;
        }

        int simCardState = getSimCardStateFromTelephonySimState(state);

        if (simCardState == Sim.STATE_INVALID) {
            return;
        }

        setSubId(subId);
        setSimCardState(simCardState);
    }

    private void setSubId(int subId) {
        if (mSubId != subId) {
            ImsLog.i(getSlotId(), "[SIM] setSubId: " + mSubId + " >> " + subId);
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
            ImsLog.i(getSlotId(), "[SIM] SimCardState: "
                    + simStateToString(mSimCardState) + " >> " + simStateToString(state));

            mSimCardState = state;

            // Notifies the applications that the SIM card state is changed.
            notifySimCardStateChanged();
        }
    }

    private void loadSimRecords() {
        TelephonyManager tm = getTelephonyManager(getSlotId(), getSubId());

        if (tm == null) {
            return;
        }

        String serviceTable = null; /*TODO: tm.getSimServiceTable(Sim.APP_TYPE_USIM)*/

        mUst = SimUtils.hexStringToBytes(serviceTable);

        ImsLog.d(getSlotId(), "[SIM] SimRecords: ust=" + serviceTable);
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
            ImsLog.i(getSlotId(), "[SIM] SimState: "
                    + simStateToString(mSimState) + " >> " + simStateToString(state));

            mSimState = state;

            if (isSimLoaded()) {
                loadSimRecords();
            }

            // Notifies the applications that the SIM state is changed.
            notifySimStateChanged();
        }
    }

    private void updateSimState() {
        TelephonyManager tm = getTelephonyManager(getSlotId(), getSubId());

        if (tm == null) {
            return;
        }

        int cardState = tm.getSimCardState();
        int simCardState = getSimCardStateFromTelephonySimState(cardState);

        if (simCardState != Sim.STATE_INVALID) {
            setSimCardState(simCardState);
        }

        int state = tm.getSimApplicationState();
        int simState = getSimStateFromTelephonySimState(state);

        if (simState == Sim.STATE_INVALID) {
            ImsLog.d(getSlotId(), "[SIM] Invalid state");
            return;
        }

        simState = refineSimState(simState);

        boolean oldSimLoaded = isSimLoaded();
        int oldSimState = getSimState();

        setSimState(simState);

        int newSimState = getSimState();

        ImsLog.d(getSlotId(), "[SIM] updateSimState: "
                + simStateToString(oldSimState) + " >> " + simStateToString(newSimState));

        if (oldSimState != newSimState) {
            updateIsimStateOnSimStateChanged(oldSimLoaded, isSimLoaded());
        }
    }

    private void loadIsimRecords() {
        TelephonyManager tm = getTelephonyManager(getSlotId(), getSubId());

        if (tm == null) {
            return;
        }

        String serviceTable = tm.getIsimIst();
        String[] isimImpus = tm.getIsimImpu();

        mIsimIst = SimUtils.hexStringToBytes(serviceTable);
        mIsimImpi = tm.getIsimImpi();
        mIsimDomain = tm.getIsimDomain();

        mIsimImpu.clear();

        if (isimImpus != null) {
            mIsimImpu.addAll(Arrays.asList(isimImpus));
        }

        ImsLog.d(getSlotId(), "[SIM] IsimRecords: ist=" + serviceTable
                + ", impi=" + ImsLog.hiddenString(mIsimImpi)
                + ", domain=" + ImsLog.hiddenString(mIsimDomain)
                + ", impu=" + ImsLog.hiddenString(isimImpus));
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

            ImsLog.i(getSlotId(), "[SIM] IsimState: "
                    + isimStateToString(mIsimState) + " >> " + isimStateToString(newIsimState));

            mIsimState = newIsimState;

            if (isIsimLoaded()) {
                loadIsimRecords();
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

    private void updateIsimState() {
        if (isSimLoaded() && !isIsimLoaded()) {
            int isimState = getIsimState();
            int newIsimState = isimState;

            if (isimState == Sim.ISIM_STATE_REFRESH_STARTED) {
                newIsimState = Sim.ISIM_STATE_REFRESH_COMPLETED;
            } else {
                TelephonyManager tm = getTelephonyManager(getSlotId(), getSubId());

                // The isApplicationOnUicc is a hidden API, so it will be removed
                // when a formal ISIM interface is adapted.
                if (tm != null && tm.isApplicationOnUicc(Sim.APP_TYPE_ISIM)) {
                    newIsimState = Sim.ISIM_STATE_LOADED;
                } else {
                    newIsimState = Sim.ISIM_STATE_NOT_PRESENT;
                }
            }

            ImsLog.i(getSlotId(), "[SIM] updateIsimState: subId="
                    + getSubId() + ", state=" + newIsimState);

            setIsimState(newIsimState);
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
                TelephonyManager tm = getTelephonyManager(getSlotId(), getSubId());

                // The isApplicationOnUicc is a hidden API, so it will be removed
                // when a formal ISIM interface is adapted.
                if (tm != null && tm.isApplicationOnUicc(Sim.APP_TYPE_ISIM)) {
                    newIsimState = Sim.ISIM_STATE_LOADED;
                } else {
                    newIsimState = Sim.ISIM_STATE_NOT_PRESENT;
                }
            }
        }

        if (newIsimState == Sim.ISIM_STATE_UNKNOWN) {
            ImsLog.w(getSlotId(), "[SIM] updateIsimStateOnSimStateChanged: unknown state");
            return;
        }

        setIsimState(newIsimState);
    }

    private final class SimStateReceiver extends BroadcastReceiver {
        public void register() {
            IntentFilter intentFilter = new IntentFilter(
                    TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED);

            intentFilter.addAction(TelephonyManager.ACTION_SIM_CARD_STATE_CHANGED);

            AppContext.get().registerReceiver(this, intentFilter, null,
                    mSimHandler, Context.RECEIVER_EXPORTED);
        }

        public void unregister() {
            AppContext.get().unregisterReceiver(this);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            ImsLog.i(mSlotId, "[SIM] " + ImsLog.lastSubString(action, "."));

            if (TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED.equals(action)) {
                handleSimApplicationStateChanged(
                        getExtraSlotIndex(intent),
                        getExtraSubscriptionIndex(intent),
                        getExtraSimState(intent));
            } else if (TelephonyManager.ACTION_SIM_CARD_STATE_CHANGED.equals(action)) {
                handleSimCardStateChanged(
                        getExtraSlotIndex(intent),
                        getExtraSubscriptionIndex(intent),
                        getExtraSimState(intent));
            }
        }
    }

    private final class SimHandler extends Handler {
        SimHandler() {
            super(AppContext.getMainLooper());
        }

        @Override
        public void handleMessage(@NonNull Message msg) {
            ImsLog.i(getSlotId(), "[SIM] handleMessage - msg=" + msg.what);

            switch (msg.what) {
                case EVENT_REQUEST_SIM_AUTHENTICATION: {
                    handleRequestSimAuthentication((AuthEvent) msg.obj);
                    break;
                }
                case EVENT_READ_ISIM_FILE_ATTRIBUTES: {
                    handleReadIsimFileAttributes(msg.arg1);
                    break;
                }
                case EVENT_READ_ISIM_RECORD: {
                    handleReadIsimRecord(msg.arg1, msg.arg2);
                    break;
                }
                case EVENT_NATIVE_BOOT_COMPLETED: {
                    handleNativeBootCompleted();
                    break;
                }
                default:
                    // no-op
                    break;
            }
        }
    }

    private static @Sim.IsimState int getIsimStateFromSimState(@Sim.State int state) {
        switch (state) {
            case Sim.STATE_UNKNOWN: // FALL-THROUGH
            case Sim.STATE_ABSENT:
                return Sim.ISIM_STATE_NOT_PRESENT;
            case Sim.STATE_PRESENT: // FALL-THROUGH
            case Sim.STATE_LOCKED: // FALL-THROUGH
            case Sim.STATE_NOT_READY:
                return Sim.ISIM_STATE_NOT_READY;
            case Sim.STATE_LOADED:
                return Sim.ISIM_STATE_LOADED;
            default:
                return Sim.ISIM_STATE_UNKNOWN;
        }
    }

    private static @Sim.State int getSimCardStateFromTelephonySimState(int cardState) {
        int simCardState = getSimStateFromTelephonySimState(cardState);

        switch (simCardState) {
            case Sim.STATE_UNKNOWN:
                // If the card state is unknown, it is handled as an ABSENT.
                return Sim.STATE_ABSENT;
            case Sim.STATE_ABSENT: // FALL-THROUGH
            case Sim.STATE_PRESENT:
                return simCardState;
            default:
                return Sim.STATE_INVALID;
        }
    }

    private static @Sim.State int getSimStateFromTelephonySimState(int state) {
        switch (state) {
            case TelephonyManager.SIM_STATE_UNKNOWN: // FALL-THROUGH
            case TelephonyManager.SIM_STATE_CARD_IO_ERROR: // FALL-THROUGH
            case TelephonyManager.SIM_STATE_CARD_RESTRICTED:
                return Sim.STATE_UNKNOWN;
            case TelephonyManager.SIM_STATE_ABSENT:
                return Sim.STATE_ABSENT;
            case TelephonyManager.SIM_STATE_PRESENT:
                return Sim.STATE_PRESENT;
            case TelephonyManager.SIM_STATE_NOT_READY: // FALL-THROUGH
            case TelephonyManager.SIM_STATE_READY:
                return Sim.STATE_NOT_READY;
            case TelephonyManager.SIM_STATE_PIN_REQUIRED: // FALL-THROUGH
            case TelephonyManager.SIM_STATE_PUK_REQUIRED: // FALL-THROUGH
            case TelephonyManager.SIM_STATE_NETWORK_LOCKED: // FALL-THROUGH
            case TelephonyManager.SIM_STATE_PERM_DISABLED:
                return Sim.STATE_LOCKED;
            case TelephonyManager.SIM_STATE_LOADED:
                return Sim.STATE_LOADED;
            default:
                return Sim.STATE_INVALID;
        }
    }

    private static int getExtraSimState(Intent intent) {
        return intent.getIntExtra(TelephonyManager.EXTRA_SIM_STATE, Sim.STATE_INVALID);
    }

    private static int getExtraSlotIndex(Intent intent) {
        return intent.getIntExtra(
                SubscriptionManager.EXTRA_SLOT_INDEX, MSimUtils.INVALID_SLOT_ID);
    }

    private static int getExtraSubscriptionIndex(Intent intent) {
        return intent.getIntExtra(
                SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX, MSimUtils.INVALID_SUB_ID);
    }

    private static SmsManager getSmsManager(int slotId, int subId) {
        if (!MSimUtils.isValidSubId(subId)) {
            subId = MSimUtils.getSubId(slotId);
        }

        SmsManager sm = AppContext.getSystemService(SmsManager.class);

        if (sm != null) {
            if (!MSimUtils.isValidSubId(subId)) {
                if (!MSimUtils.isMultiSimEnabled()) {
                    return sm;
                }

                return null;
            }

            return sm.createForSubscriptionId(subId);
        }

        return null;
    }

    private static TelephonyManager getTelephonyManager(int slotId, int subId) {
        if (!MSimUtils.isValidSubId(subId)) {
            subId = MSimUtils.getSubId(slotId);
        }

        if (!MSimUtils.isValidSubId(subId)) {
            if (!MSimUtils.isMultiSimEnabled()) {
                return AppContext.getTelephonyManager();
            }

            return null;
        }

        return AppContext.getTelephonyManager(subId);
    }

    private static ISystem getSystem(int slotId) {
        return SystemInterface.getInstance().getSystem(slotId);
    }

    private static void executeOnIsimThread(Runnable r) {
        new Thread(r, "IsimThread").start();
    }

    private static String isimFileIdToString(@Sim.IsimFileId int fileId) {
        switch (fileId) {
            case Sim.ISIM_FILE_ID_IMPI:
                return "IMPI";
            case Sim.ISIM_FILE_ID_DOMAIN:
                return "DOMAIN";
            case Sim.ISIM_FILE_ID_IMPU:
                return "IMPU";
            default:
                return "UNKNOWN";
        }
    }

    private static String isimStateToString(@Sim.IsimState int state) {
        switch (state) {
            case Sim.ISIM_STATE_UNKNOWN:
                return "UNKNOWN";
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
            default:
                return "INVALID";
        }
    }

    private static String simStateToString(@Sim.State int state) {
        switch (state) {
            case Sim.STATE_UNKNOWN:
                return "UNKNOWN";
            case Sim.STATE_ABSENT:
                return "ABSENT";
            case Sim.STATE_PIN_REQUIRED:
                return "PIN_REQUIRED";
            case Sim.STATE_PUK_REQUIRED:
                return "PUK_REQUIRED";
            case Sim.STATE_NETWORK_LOCKED:
                return "NETWORK_LOCKED";
            case Sim.STATE_PERM_DISABLED:
                return "PERM_DISABLED";
            case Sim.STATE_READY:
                return "READY";
            case Sim.STATE_NOT_READY:
                return "NOT_READY";
            case Sim.STATE_CARD_IO_ERROR:
                return "CARD_IO_ERROR";
            case Sim.STATE_CARD_RESTRICTED:
                return "CARD_RESTRICTED";
            case Sim.STATE_LOCKED:
                return "LOCKED";
            case Sim.STATE_LOADED:
                return "LOADED";
            case Sim.STATE_PRESENT:
                return "PRESENT";
            default:
                return "INVALID";
        }
    }
}
