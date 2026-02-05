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

import com.android.imsstack.util.ImsLog;

import java.util.List;

public final class ConferenceInfoHelper {
    private static int sAnonymousId = 1;

    private ConferenceInfoHelper() {
    }

    public static int getAnonymousId() {
        return sAnonymousId;
    }

    /**
     * Sets the anonymous ID.
     *
     * @param anonymousId The anonymous ID to set.
     */
    public static void setAnonymousId(int anonymousId) {
        sAnonymousId = anonymousId;

        if (sAnonymousId == Integer.MAX_VALUE) {
            sAnonymousId = 1;
        }
    }

    /**
     * Creates ConferenceInfo instance if not existed.
     */
    public static ConferenceInfo createConferenceInfo(String ccid) {
        if (ccid == null) {
            // Special case: it's for the initial merge operation.
            ccid = ConferenceInfo.IMMATURE_CCID;
        }

        ConferenceInfoManager cim = ConferenceInfoManager.getInstance();
        ConferenceInfo ci = cim.getConferenceInfo(ccid);

        if (ci == null) {
            // Create a new ConferenceInfo
            ci = cim.createConferenceInfo(ccid);
        } else if (ConferenceInfo.IMMATURE_CCID.equals(ccid)) {
            log("Immature ConferenceInfo already exists");
        }

        return ci;
    }

    public static void destroyConferenceInfo(String ccid) {
        ConferenceInfoManager cim = ConferenceInfoManager.getInstance();

        if (!cim.hasConferenceInfo()) {
            // no-op
            return;
        }

        if (ccid == null) {
            // Special case: it's for the initial merge failure.
            ccid = ConferenceInfo.IMMATURE_CCID;
        }

        cim.destroyConferenceInfo(ccid);

        if (!cim.hasConferenceInfo()) {
            setAnonymousId(1);
        }
    }

    public static void destroyAllConferenceInfos() {
        ConferenceInfoManager cim = ConferenceInfoManager.getInstance();

        setAnonymousId(1);
        cim.destroyAllConferenceInfos();
    }

    public static boolean addConferenceUser(String ccid,
            String peerConfUserId, int peerCallConnectionId) {
        ConferenceInfo ci = getConferenceInfo(ccid);

        if (ci == null) {
            loge("No ConferenceInfo");
            return false;
        }

        if (!ci.addUserForInterimStatus(peerCallConnectionId, peerConfUserId, "", 0, 0)) {
            loge("User already exists; peer - peerCallConnectionId=" + peerCallConnectionId
                    + ", confUserId=" + dbgLog(peerConfUserId));
        }

        return true;
    }

    public static ConferenceInfo getConferenceInfo(String ccid) {
        return ConferenceInfoManager.getInstance().getConferenceInfo(ccid);
    }

    /**
     * This method will be invoked in the scenario; drop and add a user.
     */
    public static boolean isConferenceUserRemovable(String confUserId) {
        ConferenceInfoManager cim = ConferenceInfoManager.getInstance();
        ConferenceInfo ci = cim.getConferenceInfoByUser(null, confUserId);

        if (ci == null) {
            return false;
        }

        ConferenceInfo.User user = ci.getUser(null, confUserId);

        return (user != null) && user.isRemovable();
    }

    public static void removeConferenceUser(String callId, String confUserId) {
        ConferenceInfoManager cim = ConferenceInfoManager.getInstance();
        ConferenceInfo ci = cim.getConferenceInfoByUser(callId, confUserId);

        if (ci != null) {
            int userCount = ci.getUserCount();

            ci.removeUser(callId, confUserId);

            if (userCount != ci.getUserCount()) {
                logi("removeConferenceUser :: userCount=" + ci.getUserCount());
            }
        }
    }

    public static int removeConferenceUsersByStatus(String ccid, String status) {
        if (status == null) {
            return 0;
        }

        ConferenceInfo ci = getConferenceInfo(ccid);

        if (ci == null) {
            return 0;
        }

        List<ConferenceInfo.User> users = ci.getUsers();

        if (users.isEmpty()) {
            return 0;
        }

        int removedUsers = 0;
        ConferenceInfo.User[] copyUsers = users.toArray(new ConferenceInfo.User[users.size()]);

        for (int i = 0; i < copyUsers.length; ++i) {
            ConferenceInfo.User user = copyUsers[i];

            if (status.equals(user.getStatus())) {
                ci.removeUser(user.getCallId(), user.getId());
                removedUsers++;
            }
        }

        if (removedUsers > 0) {
            logi("removeConferenceUsersByStatus :: userCount="
                    + ci.getUserCount() + ", removed=" + removedUsers);
        }

        return removedUsers;
    }

    public static void removeConferenceUsersOnInvitationFailed(String ccid) {
        ConferenceInfo ci = getConferenceInfo(ccid);

        if (ci == null) {
            return;
        }

        List<ConferenceInfo.User> users = ci.getUsers();

        if (users.isEmpty()) {
            return;
        }

        ConferenceInfo.User[] copyUsers = users.toArray(new ConferenceInfo.User[users.size()]);

        for (int i = 0; i < copyUsers.length; ++i) {
            ConferenceInfo.User user = copyUsers[i];
            String currentStatus = user.getStatus();

            if (ConferenceInfo.User.STATUS_PENDING.equals(currentStatus)) {
                ci.removeUser(user.getCallId(), user.getId());
            }
        }

        log("removeConferenceUsersOnInvitationFailed :: userCount=" + ci.getUserCount());
    }

    public static void replaceImmatureCcid(String origCcid, String newCcid) {
        ConferenceInfo ci = getConferenceInfo(origCcid);

        if (ci != null) {
            ci.replaceImmatureCcid(newCcid);
        } else {
            log("No immature ConferenceInfo; origCcid=" + origCcid);
        }
    }

    public static void setListenerForConferenceUser(String callId, String confUserId,
            ConferenceInfo.User.Listener listener) {
        ConferenceInfoManager cim = ConferenceInfoManager.getInstance();
        ConferenceInfo ci = cim.getConferenceInfoByUser(callId, confUserId);

        if (ci != null) {
            ConferenceInfo.User user = ci.getUser(callId, confUserId);

            if (user != null) {
                user.setListener(listener);
            }
        }
    }

    public static void updateAndNotifyDisconnectedForAllConferenceUsers(String ccid) {
        ConferenceInfo ci = getConferenceInfo(ccid);

        if (ci == null) {
            return;
        }

        List<ConferenceInfo.User> users = ci.getUsers();

        for (ConferenceInfo.User user : users) {
            user.setSIPStatusCode(0);
            user.updateStatus(ConferenceInfo.User.STATUS_DISCONNECTED);
        }

        ci.notifyUserStatusIfChanged();
    }

    public static boolean updateConferenceUser(ConferenceInfo ci, String callId, String uid,
            String userEntity, String endpointEntity, String displayText, String status,
            int sipStatusCode, int disconnectedCause) {
        if (ci == null) {
            return false;
        }

        if (ConferenceInfo.isInterimUser(callId, uid, userEntity, endpointEntity)) {
            if (!ci.updateUserForInterimStatus(callId, uid, status,
                    sipStatusCode, disconnectedCause)) {
                // If the user does not exist and the status is "disconnecting" or "disconnected",
                // then we don't need to update this user.
                if (ConferenceInfo.User.STATUS_DISCONNECTED.equals(status)
                        || ConferenceInfo.User.STATUS_DISCONNECTING.equals(status)) {
                    log("updateConferenceUser(addInterim) :: Ignored status=" + status);
                    return false;
                }

                // New user detected
                ci.addUserForInterimStatus(0, uid, status,
                        sipStatusCode, disconnectedCause);

                log("updateConferenceUser(addInterim) :: userCount=" + ci.getUserCount());
                return true;
            }
        } else {
            if (!ci.updateUser(callId, uid,
                    userEntity, endpointEntity,
                    displayText, status, sipStatusCode, disconnectedCause)) {
                // If the user does not exist and the status is "disconnecting" or "disconnected",
                // then we don't need to update this user.
                if (ConferenceInfo.User.STATUS_DISCONNECTED.equals(status)
                        || ConferenceInfo.User.STATUS_DISCONNECTING.equals(status)) {
                    log("updateConferenceUser(add) :: Ignored status=" + status);
                    return false;
                }

                // New user detected
                ci.addUser(callId, uid,
                        userEntity, endpointEntity,
                        displayText, status, sipStatusCode, disconnectedCause);

                log("updateConferenceUser(add) :: userCount=" + ci.getUserCount());
                return true;
            }
        }

        log("updateConferenceUser :: userCount=" + ci.getUserCount());

        return true;
    }

    public static void updateConferenceUsersOnInvitationFailed(String ccid,
            String status, int sipStatusCode) {
        ConferenceInfo ci = getConferenceInfo(ccid);

        if (ci == null) {
            return;
        }

        List<ConferenceInfo.User> users = ci.getUsers();

        for (ConferenceInfo.User user : users) {
            String currentStatus = user.getStatus();

            if (ConferenceInfo.User.STATUS_PENDING.equals(currentStatus)
                    || (user.isInterim()
                        && ConferenceInfo.User.STATUS_DIALING_IN.equals(currentStatus))) {
                user.updateStatus(status);
                user.setSIPStatusCode(sipStatusCode);
            }
        }
    }

    public static void updateConferenceUsersOnRemoval(String ccid) {
        ConferenceInfo ci = getConferenceInfo(ccid);

        if (ci == null) {
            return;
        }

        List<ConferenceInfo.User> users = ci.getUsers();

        for (ConferenceInfo.User user : users) {
            String status = user.getStatus();

            if (ConferenceInfo.User.STATUS_DISCONNECTING.equals(status)) {
                user.updateStatus(ConferenceInfo.User.STATUS_DISCONNECTED);
                user.setSIPStatusCode(0);
            }
        }
    }

    private static void log(String s) {
        ImsLog.d("[ISIL] " + s);
    }

    private static void loge(String s) {
        ImsLog.e("[ISIL] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[ISIL] " + s);
    }

    private static String dbgLog(String s) {
        return ImsLog.hiddenString(s);
    }
}
