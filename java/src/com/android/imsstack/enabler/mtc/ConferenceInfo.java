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

import android.text.TextUtils;

import com.android.imsstack.util.ImsLog;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

public final class ConferenceInfo {
    /**
     * Temporary conference call identifier for the first call merge.
     */
    public static final String IMMATURE_CCID = "immature_ccid";

    /**
     * conference-info -> user element
     */
    public static class User {
        public static interface Listener {
            public void onConferenceUserStatusUpdated(User user);
        }

        /** Default user id */
        public static final String DEFAULT_CALL_ID = "0";

        /**
         * status-type (String) :
         * "pending" : Endpoint is not yet in the call, but it is anticipated that they will
         *      join in the near future.
         * "dialing-out" : Focus has dialed out to connect the endpoint to the conference,
         *      but the endpoint is not yet in the roster (probably being authenticated).
         * "dialing-in" : Endpoint is dialing into the conference, not yet in the roster
         *      (probably being authenticated).
         * "alerting" : PSTN alerting or SIP 180 Ringing was returned for the outbound call;
         *      endpoint is being alerted.
         * "on-hold" : Active signaling dialog exists between an endpoint and a focus,
         *      but endpoint is "on-hold" for this conference, i.e., they are neither "hearing"
         *      the conference mix nor are their media being mixed in the conference.
         * "connected" : Endpoint is a participant in the conference. Depending on the media
         *      policies, they can send and receive media to and from other participants.
         * "disconnecting" : Focus is in the process of disconnecting the endpoint
         *      (e.g. in SIP a DISCONNECT or BYE was sent to the endpoint).
         * "disconnected" : Endpoint is not a participant in the conference, and no active dialog
         *      exists between the endpoint and the focus.
         * "muted-via-focus" : Active signaling dialog exists between an endpoint and a focus and
         *      the endpoint can "listen" to the conference, but the endpoint's media is not being
         *      mixed into the conference.
         * "connect-fail" : Endpoint fails to join the conference by rejecting the conference call.
         */
        public static final String STATUS_PENDING = "pending";
        public static final String STATUS_DIALING_OUT = "dialing-out";
        public static final String STATUS_DIALING_IN = "dialing-in";
        public static final String STATUS_ALERTING = "alerting";
        public static final String STATUS_ON_HOLD = "on-hold";
        public static final String STATUS_CONNECTED = "connected";
        public static final String STATUS_DISCONNECTING = "disconnecting";
        public static final String STATUS_DISCONNECTED = "disconnected";
        public static final String STATUS_MUTED_VIA_FOCUS = "muted-via-focus";
        public static final String STATUS_CONNECT_FAIL = "connect-fail";

        /**
         * Conference state of user.
         */
        public static final int STATE_NONE = 0;
        public static final int STATE_INTERIM_EVENT = 1;
        public static final int STATE_EVENT = 2;

        // Call-id: when merging two calls, it will used to identify a specific call
        private final String mCallId;

        // It's an universal user id to guarantee the uniqueness of participant
        // in ImsConferenceState. (It's used as key value)
        // And, it's a trick to guarantee the uniqueness of endpoint
        // which is passed to the upper layer.
        private String mUuid = "";

        // user-part from "user -> entity attribute"
        // * UNUSED * : internal only for java & native
        private String mId = "";
        // It contains the same value as mId,
        // but it will only be used to synchronize with native logic.
        private String mIdForNative = "";
        // user (entity)
        private String mUserEntity = "";
        // user > endpoint (entity)
        private String mEndpointEntity = "";
        // 1) user > endpoint > display-text
        // 2) user > display-text
        private String mDisplayText = "";
        // user > endpoint > status
        private String mStatus = "";
        // SIP status code : for SIP result of "refer" event package
        private int mSIPStatusCode = 0;
        // Disconnected cause
        private int mDisconnectedCause = 0;

        private int mState = STATE_NONE;
        private boolean mStatusChanged = false;
        private Listener mListener;

        public User(String callId, String id) {
            mCallId = callId;
            mId = id;
            updateIdForNative(id);

            UUID uuid = UUID.randomUUID();
            mUuid = (uuid != null) ? uuid.toString() : "";

            if (ImsLog.isDebuggable()) {
                log("User :: callId=" + mCallId + ", id=" + dbgLog(mId) + ", uuid=" + mUuid);
            }
        }

        public static boolean isDefaultCallId(String callId) {
            return TextUtils.isEmpty(callId) ?
                    true : (callId.equals(DEFAULT_CALL_ID) ? true : false);
        }

        public void init(String userEntity, String endpointEntity,
                String displayText, String status) {
            mUserEntity = userEntity;
            mEndpointEntity = endpointEntity;
            mDisplayText = displayText;

            if (!TextUtils.isEmpty(status)) {
                updateStatus(status);
            }

            setState(STATE_EVENT);
        }

        public void initForInterimEvent(String id, String userEntity, String endpointEntity,
                String displayText, String status) {
            mId = id;
            mUserEntity = userEntity;
            mEndpointEntity = endpointEntity;
            mDisplayText = displayText;

            updateIdForNative(id);

            if (!TextUtils.isEmpty(status)) {
                updateStatus(status);
            }

            setState(STATE_INTERIM_EVENT);
        }

        /**
         * Call identifier to identify a specific call when merging or extending the calls
         */
        public String getCallId() {
            return mCallId;
        }

        /**
         * Call connection identifier to identify a specific call.
         */
        public int getCallConnectionId() {
            return Integer.parseInt(getCallId());
        }

        /**
         * Disconnected cause value.
         */
        public int getDisconnectedCause() {
            return mDisconnectedCause;
        }

        /**
         * 1) user > endpoint > display-text
         * 2) user > display-text
         */
        public String getDisplayText() {
            return mDisplayText;
        }

        /**
         * user > endpoint (entity)
         */
        public String getEndpoint() {
            return mEndpointEntity;
        }

        /**
         * user-part from "user -> entity attribute"
         */
        public String getId() {
            return mId;
        }

        /**
         * Returns the same value as getId(),
         * but it's only used for the synchronization between java and native logic.
         * (It's a hidden field for the external module)
         */
        public String getIdForNative() {
            return mIdForNative;
        }

        /**
         * SIP status code : for SIP result of "refer" event package
         */
        public int getSIPStatusCode() {
            return mSIPStatusCode;
        }

        /**
         * user > endpoint > status
         */
        public String getStatus() {
            return mStatus;
        }

        /**
         * user (entity)
         */
        public String getUser() {
            return mUserEntity;
        }

        /**
         * Universal user id.
         */
        public String getUuid() {
            return mUuid;
        }

        /**
         * Checks if the user is initialized.
         */
        public boolean isInitialized() {
            return (mState == STATE_EVENT) || (mState == STATE_INTERIM_EVENT);
        }

        /**
         * Checks if the user is the type of interim event
         * This type will be maintained until receiving the conference state via event package.
         */
        public boolean isInterim() {
            return (mState == STATE_INTERIM_EVENT);
        }

        /**
         * Checks if the user can be removable from the conference state.
         */
        public boolean isRemovable() {
            return STATUS_DISCONNECTED.equals(mStatus)
                    || STATUS_DISCONNECTING.equals(mStatus)
                    || STATUS_CONNECT_FAIL.equals(mStatus);
        }

        /**
         * Checks if the user is available to notify the status.
         */
        public boolean isReportable() {
            return !TextUtils.isEmpty(mStatus)
                    && ((mState == STATE_EVENT) || (mState == STATE_INTERIM_EVENT));
        }

        /**
         * Notifies the application that the status is changed.
         */
        public void notifyStatusChange() {
            if (mStatusChanged) {
                if (mListener != null) {
                    mListener.onConferenceUserStatusUpdated(this);
                    mStatusChanged = false;
                }
            }
        }

        /**
         * Disconnected cause value.
         */
        public void setDisconnectedCause(int disconnectedCause) {
            mDisconnectedCause = disconnectedCause;
        }

        /**
         * Sets the listener to monitor the user's status change
         */
        public void setListener(User.Listener listener) {
            mListener = listener;
        }

        /**
         * SIP status code : for SIP result of "refer" event package
         */
        public void setSIPStatusCode(int statusCode) {
            mSIPStatusCode = statusCode;
        }

        /**
         * Updates "id" field for native logic.
         */
        public void updateIdForNative(String id) {
            mIdForNative = id;
        }

        /**
         * user > endpoint > status
         */
        public void updateStatus(String status) {
            if (TextUtils.isEmpty(status)) {
                log("User#updateStatus: Invalid status; callId="
                        + mCallId + ", id=" + dbgLog(mId) + ", idForN=" + dbgLog(mIdForNative));
                return;
            }

            if (!mStatus.equalsIgnoreCase(status)) {
                logi("User#updateStatus: callId=" + mCallId + ", id=" + dbgLog(mId)
                        + ", idForN=" + dbgLog(mIdForNative) + ", user=" + dbgLog(mUserEntity)
                        + ", oldStatus=" + mStatus + ", status=" + status);

                mStatus = status;
                mStatusChanged = true;
            }
        }

        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();

            sb.append("[ ConferenceInfo#User: uuid=");
            sb.append(dbgLog(mUuid));
            sb.append(", state=");
            sb.append(stateToString(mState));
            sb.append(", callId=");
            sb.append(mCallId);
            sb.append(", id=");
            sb.append(dbgLog(mId));
            sb.append(", idForN=");
            sb.append(dbgLog(mIdForNative));
            sb.append(", user=");
            sb.append(dbgLog(mUserEntity));
            sb.append(", endpoint=");
            sb.append(dbgLog(mEndpointEntity));
            sb.append(", display-text=");
            sb.append(mDisplayText);
            sb.append(", status=");
            sb.append(mStatus);
            sb.append(", sipstatuscode=");
            sb.append(mSIPStatusCode);
            sb.append(", disconnectedCause=");
            sb.append(mDisconnectedCause);
            sb.append(" ]");

            return sb.toString();
        }

        private void setState(int state) {
            if (mState != state) {
                logi("User#setState :: " + stateToString(mState) + " >> " + stateToString(state));
                mState = state;
            }
        }

        private static String stateToString(int state) {
            if (state == STATE_EVENT) {
                return "EVENT";
            } else if (state == STATE_INTERIM_EVENT) {
                return "INTERIM_EVENT";
            } else {
                return "NONE";
            }
        }
    }

    // Call identifier of the conference focus
    private String mCcid;
    private final List<User> mUsers = new ArrayList<User>();

    public ConferenceInfo(String ccid) {
        mCcid = ccid;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();

        sb.append("[ ConferenceInfo: ccid=");
        sb.append(mCcid);
        sb.append(", userCount=");
        sb.append(getUserCount());
        sb.append(" ]");

        return sb.toString();
    }

    public boolean addUser(String callId, String uid, String userEntity,
            String endpointEntity, String displayText, String status, int sipStatusCode,
            int disconnectedCause) {
        if (callId == null) {
            callId = User.DEFAULT_CALL_ID;
        }

        User user = new User(callId, uid);

        user.init(userEntity, endpointEntity, displayText, status);
        user.setSIPStatusCode(sipStatusCode);
        user.setDisconnectedCause(disconnectedCause);

        mUsers.add(user);

        log("addUser: callId=" + callId + ", uid=" + dbgLog(uid) + ", status=" + status);

        return true;
    }

    /**
     * Adds Conference user
     */
    public boolean addUserForInterimStatus(int callConnectionId, String uid,
            String status, int sipStatusCode, int disconnectedCause) {
        String callId = String.valueOf(callConnectionId);

        if (callId == null) {
            callId = User.DEFAULT_CALL_ID;
        }

        User user = new User(callId, uid);

        user.initForInterimEvent(uid, uid, uid, "", status);
        user.setSIPStatusCode(sipStatusCode);
        user.setDisconnectedCause(disconnectedCause);

        mUsers.add(user);

        log("addUserForInterimStatus: callId=" + callId
                + ", uid=" + dbgLog(uid) + ", status=" + status);

        return true;
    }

    /**
     * Conference call's identifier
     */
    public String getCcid() {
        return mCcid;
    }

    /**
     * Returns the user's count which are participated in the conference call.
     */
    public int getUserCount() {
        return mUsers.size();
    }

    /**
     * Returns the users which are participated in the conference call.
     */
    public List<User> getUsers() {
        return mUsers;
    }

    /**
     * Returns a user who matches with callId or uid.
     * The top priority will be callId, and the next is uid.
     * (The priority is adjusted for multi-device requirement)
     * It is also possible to find out User for the partical matching of callId.
     */
    public User getUser(String callId, String uid) {
        boolean validCallId = !TextUtils.isEmpty(callId) && !User.isDefaultCallId(callId);

        // "callId" is checked first for multi-device requirement.
        if (validCallId) {
            User user = getUserForCallId(callId);

            if (user == null) {
                // Partial matching for call-id
                user = getUserForCallIdWithPartialMatch(callId);
            }

            return user;
        }

        return getUser(uid);
    }

    /**
     * CALL_CONNECTION_ID
     * Returns a user which matches with the given call connection identifier.
     * Call connection identifier indicates the call connected order.
     */
    public User getUserForCallConnectionId(int callConnectionId) {
        if (callConnectionId > 0) {
            for (User user : mUsers) {
                if (user.getCallConnectionId() == callConnectionId) {
                    return user;
                }
            }
        }

        return null;
    }

    /**
     * Returns a user which matches with the given ordered key.
     * It's for the convenience to find a user.
     */
    public User getUserForEntity(String endpointEntity, String userEntity) {
        User user = getUserForUuid(endpointEntity);

        if (user == null) {
            user = getUserForEndpointEntity(endpointEntity);
        }

        return (user != null) ? user : getUserForUserEntity(userEntity);
    }

    public boolean hasActiveUser() {
        if (mUsers.isEmpty()) {
            return false;
        }

        for (User user : mUsers) {
            if (user.isReportable()) {
                return true;
            }
        }

        return false;
    }

    public void notifyUserStatusIfChanged() {
        for (User user : mUsers) {
            user.notifyStatusChange();
        }
    }

    public void removeUser(String callId, String uid) {
        User user = getUser(callId, uid);

        if (user != null) {
            mUsers.remove(user);
        }
    }

    public void replaceImmatureCcid(String ccid) {
        log("replaceImmatureCcid :: " + mCcid + " >> " + ccid);
        mCcid = ccid;
    }

    /**
     * Updates the user's status.
     */
    public boolean updateUser(String callId, String uid, String status) {
        User user = getUser(callId, uid);

        if (user == null) {
            log("No user; callId="
                    + callId + ", uid=" + dbgLog(uid) + ", status=" + status);
            return false;
        }

        user.updateStatus(status);

        return true;
    }

    /**
     * Updates the user's status.
     */
    public boolean updateUser(String callId, String uid,
            String userEntity, String endpointEntity, String displayText,
            String status, int sipStatusCode, int disconnectedCause) {
        User user = getUser(callId, uid);

        if (user == null) {
            log("updateUser :: No user - callId="
                    + callId + ", uid=" + dbgLog(uid) + ", status=" + status);
            return false;
        }

        if (!user.isInitialized()
                || (user.isInterim()
                    && !isInterimUser(callId, uid, userEntity, endpointEntity))) {
            user.updateIdForNative(uid);
            user.init(userEntity, endpointEntity, displayText, status);
        } else {
            user.updateStatus(status);
        }

        if (sipStatusCode >= 0) {
            user.setSIPStatusCode(sipStatusCode);
        }

        user.setDisconnectedCause(disconnectedCause);

        return true;
    }

    /**
     * Updates the user's status.
     * It's for the convenience to find a user.
     */
    public boolean updateUserForEntity(String endpointEntity, String userEntity, String status) {
        User user = getUserForEndpointEntity(endpointEntity);

        if (user == null) {
            user = getUserForUserEntity(userEntity);

            if (user == null) {
                log("No user - endpointEntity=" + dbgLog(endpointEntity)
                        + ", userEntity=" + dbgLog(userEntity)
                        + ", status=" + status);
                return false;
            }
        }

        user.updateStatus(status);

        return true;
    }

    /**
     * Updates the user's status for interim status.
     * These users are not joined in the conference call yet,
     * but those will be joined or not in the near future.
     */
    public boolean updateUserForInterimStatus(String callId, String uid,
            String status, int sipStatusCode, int disconnectedCause) {
        User user = getUser(callId, uid);

        if (user == null) {
            log("updateUserForInterimStatus :: No user - callId="
                    + callId + ", uid=" + dbgLog(uid) + ", status=" + status);
            return false;
        }

        user.updateStatus(status);

        if (sipStatusCode >= 0) {
            user.setSIPStatusCode(sipStatusCode);
        }

        user.setDisconnectedCause(disconnectedCause);

        return true;
    }

    public static boolean isInterimUser(String callId, String uid,
            String userEntity, String endpointEntity) {
        if (!TextUtils.isEmpty(callId) && TextUtils.isEmpty(uid)
                && TextUtils.isEmpty(userEntity) && TextUtils.isEmpty(endpointEntity)) {
            return true;
        } else if (TextUtils.isEmpty(callId) && !TextUtils.isEmpty(uid)
                && TextUtils.isEmpty(userEntity) && TextUtils.isEmpty(endpointEntity)) {
            return true;
        }

        return false;
    }

    /**
     * Returns a user which matches with the given key.
     */
    private User getUser(String uid) {
        if (uid != null) {
            for (User user : mUsers) {
                if (uid.equals(user.getId())) {
                    return user;
                }
            }
        }

        return null;
    }

    /**
     * Returns a user which matches with the given call identifier.
     */
    private User getUserForCallId(String callId) {
        if (callId != null) {
            for (User user : mUsers) {
                if (callId.equals(user.getCallId())) {
                    return user;
                }
            }
        }

        return null;
    }

    /**
     * Returns a user which matches with the given call identifier.
     */
    private User getUserForCallIdWithPartialMatch(String callId) {
        if (callId != null) {
            for (User user : mUsers) {
                String userCallId = user.getCallId();
                if ((userCallId != null) && userCallId.contains(callId)) {
                    return user;
                }
            }
        }

        return null;
    }

    /**
     * Returns a user which matches with the given entity of endpoint.
     */
    private User getUserForEndpointEntity(String endpointEntity) {
        if (endpointEntity != null) {
            for (User user : mUsers) {
                if (endpointEntity.equals(user.getEndpoint())) {
                    return user;
                }
            }
        }

        return null;
    }

    /**
     * Returns a user which matches with the given entity of user.
     */
    private User getUserForUserEntity(String userEntity) {
        if (userEntity != null) {
            for (User user : mUsers) {
                if (userEntity.equals(user.getUser())) {
                    return user;
                }
            }
        }

        return null;
    }

    /**
     * Returns a user which matches with the given uuid.
     */
    private User getUserForUuid(String uuid) {
        if (!TextUtils.isEmpty(uuid)) {
            for (User user : mUsers) {
                if (uuid.equals(user.getUuid())) {
                    return user;
                }
            }
        }

        return null;
    }

    private static void log(String s) {
        ImsLog.d("[ISIL] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[ISIL] " + s);
    }

    private static String dbgLog(String s) {
        return ImsLog.hiddenString(s);
    }
}
