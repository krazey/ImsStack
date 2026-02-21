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

package com.android.imsstack.enabler.mtc;

import android.os.Bundle;
import android.os.SystemClock;

import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.util.ImsLog;

import java.io.Closeable;

public class Call implements Closeable {
    /**
     * Default user-info for anonymous user
     */
    public static final String ANONYMOUS = "anonymous";

    /**
     * Extra properties for Mtc call.
     */
    /**
     * Boolean extra properties - "true" / "false"
     *  conference : Indicates if the call is for the conference call or not.
     *  e_call : Indicates if the call is for the emergency call or not.
     *  vms : Indicates if the call is connected to the voice mail system or not.
     *  call_mode_changeable : Indicates if the call is able to upgrade/downgrade
     *      the video during VoLTE call
     *  conference_avail : Indicates if the call can be extended to the conference
     */
    public static final String EXTRA_CONFERENCE = "conference";
    public static final String EXTRA_E_CALL = "e_call";
    public static final String EXTRA_VMS = "vms";
    public static final String EXTRA_CALL_MODE_CHANGEABLE = "call_mode_changeable";
    public static final String EXTRA_CONFERENCE_AVAIL = "conference_avail";
    // extensions
    public static final String EXTRA_MMC = "mmc";
    public static final String EXTRA_GTT = "gtt";
    public static final String EXTRA_CONFERENCE_EVENT = "conference_event";
    /** MO e-call only. It indicates that MO e-call is created via WiFi bearer. */
    public static final String EXTRA_WIFI_E_CALL = "wifi_e_call";
    public static final String EXTRA_RTT_AVAIL = "rtt_avail";

    /**
     * Integer extra properties
     *  oir : Rule for originating identity (number) presentation, MO/MT only.
     *      For MO, the following value will be used
     *          {@link SuppInfo#CALLERID_NETWORK}
     *          {@link SuppInfo#CALLERID_RESTRICTED}
     *          {@link SuppInfo#CALLERID_IDENTITY}
     *      For MT, the following value will be used
     *          {@link IncomingMtcCall#OIPTYPE_NONE}
     *          {@link IncomingMtcCall#OIPTYPE_IDENTITY}
     *          {@link IncomingMtcCall#OIPTYPE_RESTRICTED}
     *  cnap : Rule for calling name presentation
     *      {@link SuppInfo#CALLERID_NETWORK}
     *      {@link SuppInfo#CALLERID_RESTRICTED}
     *      {@link SuppInfo#CALLERID_IDENTITY}
     */
    public static final String EXTRA_OIR = "oir";
    public static final String EXTRA_CNAP = "cnap";
    // extensions
    public static final String EXTRA_CDIV_CAUSE = "cdiv_cause";

    /**
     * String extra properties
     *  oi : Originating identity (number), MT only
     *  ti : Terminating identity (number) for TIP service, MO only
     *  ti_origin : Terminating identity(number) which is originally dialed(internal usage)
     *  cna : Calling name
     *  ussd : Result string for supplementary service configuration
     *  conference_id : Conference id to identify a specific call in the conference
     *      It can be used in the call merge operation (CS call style).
     */
    public static final String EXTRA_OI = "oi";
    public static final String EXTRA_TI = "ti";
    public static final String EXTRA_TI_ORIGIN = "ti_origin";
    public static final String EXTRA_CNA = "cna";
    public static final String EXTRA_USSD = "ussd";
    // extensions
    public static final String EXTRA_CDIV_HISTORY = "cdiv_history";
    public static final String EXTRA_CONFERENCE_USER_ID = "conference_user_id";

    /**
     * Internal usages
     *  call_connection_id : Call connection identifier to identify a specific call,
     *      and it's set to the lowest number among unused numbersby ascending order.
     *      It's started with 1 if there is no calls.
     *  remote_number : Literally the number of peer.
     */
    public static final String EXTRA_CALL_CONNECTION_ID = "call_connection_id";
    public static final String EXTRA_REMOTE_NUMBER = "remote_number";

    protected static final int UPDATE_STATE_IDLE = 0;
    protected static final int UPDATE_STATE_SENT = 1;
    protected static final int UPDATE_STATE_RECEIVED = 2;
    protected static final int UPDATE_STATE_RESUME_RECEIVED = 3;
    protected static final int UPDATE_STATE_ACCEPTED = 4;
    protected static final int UPDATE_STATE_REJECTED = 5;
    protected static final int UPDATE_STATE_RESUME_ACCEPTED = 6;

    protected final Details mCallDetails = new Details();

    protected final IBaseContext mContext;

    /**
     * Call id will be used to identify the conference user as User#mCallId.
     */
    private String mCallId;
    private final Bundle mCallExtras = new Bundle();
    private final int mCallIndex;
    private final String mLogTag;

    private long mNativeCallObject;
    private int mCallType = IUMtcCall.CALLTYPE_VOIP;
    private int mCallState = CallTracker.CALL_STATE_IDLE;
    private int mUpdateState = UPDATE_STATE_IDLE;

    public Call(IBaseContext context, int index, String logTag) {
        mContext = context;
        mCallId = createCallId();
        mCallIndex = index;
        mNativeCallObject = 0;

        if (logTag.isEmpty()) {
            mLogTag = createLogTag();
        } else {
            mLogTag = logTag;
        }
    }

    @Override
    public void close() {
        mNativeCallObject = 0;
    }

    @Override
    public boolean equals(Object o) {
        if (!(o instanceof Call)) {
            return false;
        }

        Call s = (Call)o;

        return mCallId.equals(s.mCallId);
    }

    @Override
    public int hashCode() {
        return mCallId.hashCode();
    }

    public String getCallerId() {
        String callerId = "";
        int oir = getCallExtraInt(EXTRA_OIR, -1);

        if (oir != IncomingMtcCall.OIPTYPE_RESTRICTED) {
            callerId = getCallExtra(EXTRA_OI, "");
        }

        return callerId;
    }

    public String getCallExtra(String name, String defaultValue) {
        return mCallExtras.getString(name, defaultValue);
    }

    public boolean getCallExtraBoolean(String name, boolean defaultValue) {
        return mCallExtras.getBoolean(name, defaultValue);
    }

    public int getCallExtraInt(String name, int defaultValue) {
        return mCallExtras.getInt(name, defaultValue);
    }

    public String getCallId() {
        return mCallId;
    }

    public int getCallState() {
        return mCallState;
    }

    public String getConferenceUserId() {
        return getCallExtra(EXTRA_CONFERENCE_USER_ID, ANONYMOUS);
    }

    public long getNativeCallId() {
        return mNativeCallObject;
    }

    public int getCallType() {
        return mCallType;
    }

    public int getCallIndex() {
        return mCallIndex;
    }

    public String getLogTag() {
        return mLogTag;
    }

    public boolean hasCallExtra(String name) {
        return mCallExtras.containsKey(name);
    }

    public boolean isAdhocGroup() {
        return mCallDetails.is(Details.ADHOC_GROUP);
    }

    public boolean isVideoCapable() {
        return getCallExtraBoolean(EXTRA_CALL_MODE_CHANGEABLE, false);
    }

    public boolean isConference() {
        return getCallExtraBoolean(EXTRA_CONFERENCE, false);
    }

    public boolean isEmergencyCall() {
        return getCallExtraBoolean(EXTRA_E_CALL, false);
    }

    public boolean isInCall() {
        return getCallState() == CallTracker.CALL_STATE_OFFHOOK;
    }

    public boolean isOnceInCall() {
        return mCallDetails.is(Details.ONCE_IN_CALL);
    }

    public boolean isOnHeld() {
        return isInCall() && mCallDetails.is(Details.ON_HELD);
    }

    public boolean isOnHold() {
        return isInCall() && mCallDetails.is(Details.ON_HOLD);
    }

    public boolean isMO() {
        return mCallDetails.is(Details.MO);
    }

    public boolean isCallValid() {
        return mNativeCallObject != 0;
    }

    public boolean isTerminated() {
        return mCallDetails.is(Details.TERMINATED);
    }

    public boolean isTerminatedByCSRetry() {
        return mCallDetails.is(Details.TERMINATED_BY_CS_RETRY);
    }

    public boolean isTerminatedByAutoRejectedCall() {
        return mCallDetails.is(Details.TERMINATED_BY_AUTO_REJECTED_CALL);
    }

    public boolean isOnPreIncoming() {
        return mCallDetails.is(Details.ON_PRE_INCOMING);
    }

    public void setCallConnectionId(int ccId) {
        setCallExtraInt(EXTRA_CALL_CONNECTION_ID, ccId);
    }

    public int getCallConnectionId() {
        return getCallExtraInt(EXTRA_CALL_CONNECTION_ID, 0);
    }

    /**
     * Sets the number of peer.
     */
    public void setRemoteNumber(String remoteNumber) {
        setCallExtra(EXTRA_REMOTE_NUMBER, remoteNumber);
    }

    /**
     * Gets the number of peer.
     */
    public String getRemoteNumber() {
        return getCallExtra(EXTRA_REMOTE_NUMBER, "");
    }

    public void terminate(int reason, boolean immediateCallback) {
        // no-op
    }

    public static String callStateToString(int state) {
        switch (state) {
        case CallTracker.CALL_STATE_IDLE:
            return "IDLE";
        case CallTracker.CALL_STATE_RINGING:
            return "RINGING";
        case CallTracker.CALL_STATE_OFFHOOK:
            return "OFFHOOK";
        case CallTracker.CALL_STATE_RINGBACK:
            return "RINGBACK";
        default:
            return "__UNKNOWN__";
        }
    }

    public static String getCallId(Call call, long callId) {
        return (call != null) ? call.getCallId() :
                ((callId != 0) ? String.valueOf(callId) : null);
    }

    public static boolean isCallExtraBoolean(String name) {
        return (name.equalsIgnoreCase(EXTRA_CONFERENCE)
                || name.equalsIgnoreCase(EXTRA_E_CALL)
                || name.equalsIgnoreCase(EXTRA_VMS)
                || name.equalsIgnoreCase(EXTRA_CALL_MODE_CHANGEABLE)
                || name.equalsIgnoreCase(EXTRA_CONFERENCE_AVAIL)
                || name.equalsIgnoreCase(EXTRA_MMC)
                || name.equalsIgnoreCase(EXTRA_GTT)
                || name.equalsIgnoreCase(EXTRA_CONFERENCE_EVENT));
    }

    public static boolean isCallExtraInt(String name) {
        return (name.equalsIgnoreCase(EXTRA_OIR)
                || name.equalsIgnoreCase(EXTRA_CNAP)
                || name.equalsIgnoreCase(EXTRA_CDIV_CAUSE));
    }

    protected void updateNativeCallObject(long nativeCallObject) {
        mNativeCallObject = nativeCallObject;
    }

    protected String createCallId() {
        StringBuilder sb = new StringBuilder();
        sb = sb.append(Integer.toHexString(super.hashCode()));
        sb = sb.append("-");
        sb = sb.append(Long.toHexString(SystemClock.elapsedRealtime()));
        sb = sb.append("$");
        sb = sb.append(Long.toHexString(mNativeCallObject));
        sb = sb.append(":");
        sb = sb.append(mNativeCallObject);
        return sb.toString();
    }

    protected boolean hasDetails(int attribute) {
        return mCallDetails.is(attribute);
    }

    protected int getUpdateState() {
        return mUpdateState;
    }

    protected void removeCallExtra(String name) {
        mCallExtras.remove(name);
    }

    protected void setCallExtra(String name, String value) {
        mCallExtras.putString(name, value);
    }

    protected void setCallExtraBoolean(String name, boolean value) {
        mCallExtras.putBoolean(name, value);
    }

    protected void setCallExtraInt(String name, int value) {
        mCallExtras.putInt(name, value);
    }

    protected void setAdhocGroup() {
        mCallDetails.set(Details.ADHOC_GROUP);
    }

    protected void setCallTerminated() {
        mCallDetails.set(Details.TERMINATED);
    }

    protected void setDetails(int attribute, boolean flag) {
        if (flag) {
            mCallDetails.set(attribute);
        } else {
            mCallDetails.clear(attribute);
        }
    }

    protected void setOnHeld(boolean onHeld) {
        if (onHeld) {
            mCallDetails.set(Details.ON_HELD);
        } else {
            mCallDetails.clear(Details.ON_HELD);
        }
    }

    protected void setOnHold(boolean onHold) {
        if (onHold) {
            mCallDetails.set(Details.ON_HOLD);
        } else {
            mCallDetails.clear(Details.ON_HOLD);
        }
    }

    protected void setCallType(int callType) {
        mCallType = callType;
    }

    protected void setCallState(int state) {
        if (mCallState != state) {
            logi("Call#setCallState :: "
                    + callStateToString(mCallState)
                    + " >> "
                    + callStateToString(state));
            mCallState = state;

            if (isInCall()) {
                mCallDetails.set(Details.ONCE_IN_CALL);
            }
        }
    }

    protected void setUpdateState(int state) {
        if (mUpdateState != state) {
            logi("Call#setUpdateState :: "
                    + updateStateToString(mUpdateState)
                    + " >> "
                    + updateStateToString(state));
            mUpdateState = state;
        }
    }

    protected static String updateStateToString(int state) {
        switch (state) {
        case UPDATE_STATE_IDLE:
            return "UPDATE_IDLE";
        case UPDATE_STATE_SENT:
            return "UPDATE_SENT";
        case UPDATE_STATE_RECEIVED:
            return "UPDATE_RECEIVED";
        case UPDATE_STATE_RESUME_RECEIVED:
            return "UPDATE_RESUME_RECEIVED";
        case UPDATE_STATE_ACCEPTED:
            return "UPDATE_ACCEPTED";
        case UPDATE_STATE_REJECTED:
            return "UPDATE_REJECTED";
        case UPDATE_STATE_RESUME_ACCEPTED:
            return "UPDATE_RESUME_ACCEPTED";
        default:
            return "__UNKNOWN__";
        }
    }

    private String createLogTag() {
        String id = "";
        if (mContext.getSlotId() == 0) {
            id = mCallIndex + "x";
        } else {
            id = "x" + mCallIndex;
        }

        return "MO_" + id;
    }

    private void logi(String s) {
        ImsLog.i("[ISIL][" + mLogTag + "] " + s);
    }

    /**
     * This class describes the detail information of this call.
     */
    protected static class Details {
        public static final int NONE = 0x00000000;
        /**
         * Call is terminated.
         */
        public static final int TERMINATED = 0x00000001;
        /**
         * Call is on hold.
         */
        public static final int ON_HOLD = 0x00000002;
        /**
         * Call is held by the remote party.
         */
        public static final int ON_HELD = 0x00000004;
        /**
         * Indicates that the conference host call is created by the adhoc group.
         */
        public static final int ADHOC_GROUP = 0x00000008;
        /**
         * Indicates that this call is detached from the active call group.
         */
        public static final int DETACHED = 0x00000010;
        /**
         * Indicates that this call is terminated as the reason of the CS retry by the network.
         */
        public static final int TERMINATED_BY_CS_RETRY = 0x00000020;
        /**
         * Indicates that the call has been transited to OFFHOOK state.
         */
        public static final int ONCE_IN_CALL = 0x00000040;
        /**
         * Indicates that the call is mobile-originated or not.
         */
        public static final int MO = 0x00000080;
        /**
         * Indicates that this call is terminated
         * as it is rejected by IMS internal logic without alerting to UI.
         */
        public static final int TERMINATED_BY_AUTO_REJECTED_CALL = 0x00000100;
        /**
         * Indicates that this call is pre-incoming status.
         * This status will be maintained until receiving INCOMING_CALL_RECEIVED event.
         */
        public static final int ON_PRE_INCOMING = 0x00000200;

        private int mDetails = NONE;

        public Details() {
        }

        public boolean is(int attribute) {
            return (mDetails & attribute) != 0;
        }

        public void clear(int attribute) {
            mDetails &= (~attribute);
        }

        public void set(int attribute) {
            mDetails |= attribute;
        }

        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();

            sb.append("[ Details: onHold=");
            sb.append(is(ON_HOLD) ? "Y" : "N");
            sb.append(", onHeld=");
            sb.append(is(ON_HELD) ? "Y" : "N");
            sb.append(", terminated=");
            sb.append(is(TERMINATED) ? "Y" : "N");
            sb.append(", adhocGroup=");
            sb.append(is(ADHOC_GROUP) ? "Y" : "N");
            sb.append(", detached=");
            sb.append(is(DETACHED) ? "Y" : "N");
            sb.append(", terminatedByCSRetry=");
            sb.append(is(TERMINATED_BY_CS_RETRY) ? "Y" : "N");
            sb.append(", onceInCall=");
            sb.append(is(ONCE_IN_CALL) ? "Y" : "N");
            sb.append(", mo=");
            sb.append(is(MO) ? "Y" : "N");
            sb.append(", onPreIncoming=");
            sb.append(is(ON_PRE_INCOMING) ? "Y" : "N");
            sb.append(" ]");

            return sb.toString();
        }
    }
}
