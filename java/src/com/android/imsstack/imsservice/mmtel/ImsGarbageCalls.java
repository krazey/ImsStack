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

package com.android.imsstack.imsservice.mmtel;

import android.telephony.ims.stub.ImsCallSessionImplBase;

import com.android.imsstack.util.ImsLog;

import java.util.ArrayList;
import java.util.List;

public class ImsGarbageCalls {
    /** Maximum garbage calls for each slot */
    public static final int MAX_CALL = 3;

    private static ImsGarbageCalls sGarbageCalls = new ImsGarbageCalls();
    private final List<Call> mCalls = new ArrayList<Call>(4);

    private ImsGarbageCalls() {
    }

    public static ImsGarbageCalls getInstance() {
        return sGarbageCalls;
    }

    public void add(int slotId, ImsCallSessionImplBase session) {
        if (session == null) {
            return;
        }

        String callId = getCallId(session);

        if (contains(callId)) {
            log("ImsGarbageCalls :: add(constains)=" + callId);
            return;
        }

        mCalls.add(new Call(slotId, session));
        log("ImsGarbageCalls :: add=" + callId);
    }

    public boolean contains(ImsCallSessionImplBase session) {
        return (session != null) ? contains(getCallId(session)) : false;
    }

    public int getCount(int slotId) {
        int count = 0;

        for (int i = 0; i < mCalls.size(); ++i) {
            Call call = mCalls.get(i);

            if (slotId == call.mSlotId) {
                count++;
            }
        }

        return count;
    }

    public void remove(ImsCallSessionImplBase session) {
        if (session == null) {
            return;
        }

        String callId = getCallId(session);

        for (int i = 0; i < mCalls.size(); ++i) {
            Call call = mCalls.get(i);

            if (callId.equals(getCallId(call.mSession))) {
                mCalls.remove(i);
                log("ImsGarbageCalls :: remove=" + callId);
                break;
            }
        }
    }

    public void removeAll(int slotId) {
        log("ImsGarbageCalls :: removeAll=" + mCalls.size());

        for (int i = mCalls.size() - 1; i >= 0; i--) {
            Call call = mCalls.get(i);

            if (slotId == call.mSlotId) {
                mCalls.remove(i);
                log("ImsGarbageCalls :: slotId=" + slotId
                        + ", remove=" + getCallId(call.mSession));
            }
        }
    }

    private boolean contains(String callId) {
        for (Call call : mCalls) {
            if (callId.equals(getCallId(call.mSession))) {
                return true;
            }
        }

        return false;
    }

    private static String getCallId(ImsCallSessionImplBase session) {
        try {
            return (session != null) ? session.getCallId() : "";
        } catch (Throwable t) {
            t.printStackTrace();
            return "";
        }
    }

    private static void log(String s) {
        ImsLog.d("[ISIL] " + s);
    }

    private static class Call {
        public int mSlotId;
        public ImsCallSessionImplBase mSession;

        public Call(int slotId, ImsCallSessionImplBase session) {
            mSlotId = slotId;
            mSession = session;
        }
    }
}
