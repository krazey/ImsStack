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

import com.android.imsstack.enabler.mtc.ConferenceInfo.User;
import com.android.imsstack.util.ImsLog;

import java.util.ArrayList;
import java.util.List;

public final class ConferenceInfoManager {
    private static ConferenceInfoManager sConferenceInfoManager = new ConferenceInfoManager();

    // List of conference-info
    private final List<ConferenceInfo> mConferenceInfos = new ArrayList<ConferenceInfo>();

    private ConferenceInfoManager() {
    }

    public static ConferenceInfoManager getInstance() {
        return sConferenceInfoManager;
    }

    /**
     * Creates ConferenceInfo instance if not existed.
     */
    public ConferenceInfo createConferenceInfo(String ccid) {
        if (TextUtils.isEmpty(ccid)) {
            return null;
        }

        ConferenceInfo ci = getConferenceInfo(ccid);

        if (ci != null) {
            return ci;
        }

        ci = new ConferenceInfo(ccid);

        mConferenceInfos.add(ci);

        log("ConferenceInfo(add): ccid=" + ccid + ", size=" + mConferenceInfos.size());

        return ci;
    }

    public void destroyConferenceInfo(String ccid) {
        if (TextUtils.isEmpty(ccid)) {
            return;
        }

        for (int i = 0; i < mConferenceInfos.size(); ++i) {
            ConferenceInfo ci = mConferenceInfos.get(i);

            if ((ci != null) && ccid.equalsIgnoreCase(ci.getCcid())) {
                mConferenceInfos.remove(i);

                log("ConferenceInfo(remove): ccid="
                        + ccid + ", size=" + mConferenceInfos.size());
                break;
            }
        }
    }

    public void destroyAllConferenceInfos() {
        if (mConferenceInfos.isEmpty()) {
            return;
        }

        log("destroyAllConferenceInfos :: size=" + mConferenceInfos.size());

        mConferenceInfos.clear();
    }

    public ConferenceInfo getConferenceInfo(String ccid) {
        if (TextUtils.isEmpty(ccid)) {
            return null;
        }

        for (ConferenceInfo ci : mConferenceInfos) {
            if (ccid.equalsIgnoreCase(ci.getCcid())) {
                return ci;
            }
        }

        return null;
    }

    public ConferenceInfo getConferenceInfoByUser(String callId, String uid) {
        if (TextUtils.isEmpty(callId) && TextUtils.isEmpty(uid)) {
            return null;
        }

        for (ConferenceInfo ci : mConferenceInfos) {
            User user = ci.getUser(callId, uid);

            if (user != null) {
                return ci;
            }
        }

        return null;
    }

    public boolean hasConferenceInfo() {
        return !mConferenceInfos.isEmpty();
    }

    private static void log(String s) {
        ImsLog.d("[ISIL] " + s);
    }
}
