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

import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.enabler.aos.IAosInfo;
import com.android.imsstack.enabler.aos.IAosInfo.PhoneNumberState;
import com.android.imsstack.util.ImsLog;

import java.util.concurrent.ConcurrentHashMap;

/**
 * A class for providing the phone number ready to start an IMS service.
 */
public class PhoneNumberAgent {
    /** Internal events */
    private static final int EVENT_PHONE_NUMBER_READY_RETRY = 1001;
    private static final int EVENT_OBTAIN_PHONE_NUMBER_RETRY = 1002;

    private static PhoneNumberAgent sPhoneNumberAgent = null;
    private static ConcurrentHashMap<Integer, PhoneNumber> sPhoneNumber
            = new ConcurrentHashMap<Integer, PhoneNumber>();

    public static PhoneNumberAgent getInstance() {

        if (sPhoneNumberAgent == null) {
            sPhoneNumberAgent = new PhoneNumberAgent();
        }

        return sPhoneNumberAgent;

    }

    public void cleanup() {
        ImsLog.d("size=" + sPhoneNumber.size());
        sPhoneNumber.clear();
    }

    public void start(int slotID) {
        ImsLog.d("size=" + sPhoneNumber.size() + ", slotId=" + slotID);

        if (!sPhoneNumber.containsKey(slotID)) {
            sPhoneNumber.put(slotID, new PhoneNumber(slotID));
        }
    }

    public void stop(int slotID) {
        ImsLog.d("size=" + sPhoneNumber.size() + ", slotId=" + slotID);

        if (sPhoneNumber.containsKey(slotID)) {
            PhoneNumber phoneNumber = sPhoneNumber.get(slotID);
            phoneNumber.cleanup();
            phoneNumber = null;
            sPhoneNumber.remove(slotID);
        }
    }

    private PhoneNumberAgent() {
    }

    private static class PhoneNumber {
        private final int mSlotId;

        private int mTryingCounter = 0;
        private int mRetryMaxCount = 10;
        private int mRetryIntervalSec = 5;
        private PhoneNumberHandler mPhoneNumberHandler = new PhoneNumberHandler(Looper.myLooper());

        public PhoneNumber(int slotID) {
            ImsLog.d(slotID, "");

            mSlotId = slotID;
        }

        public void cleanup() {
            ImsLog.d(mSlotId, "");

            if (mPhoneNumberHandler != null) {
                mPhoneNumberHandler.removeCallbacksAndMessages(null);
            }
        }

        // return : milli-seconds
        private int getRetryInterval() {
            return mRetryIntervalSec * 1000;
        }

        private int getRetryMaxCount() {
            return mRetryMaxCount;
        }

        private void increaseRetryCount() {
            mTryingCounter++;
        }

        private boolean isRetryAllowed() {
            return (mTryingCounter <= getRetryMaxCount());
        }

        private boolean isSimLoaded() {
            SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
            return (sim != null) ? sim.isSimLoaded() : false;
        }

        private void postEmptyMsgDelayed(int event, int interval) {
            mPhoneNumberHandler.sendEmptyMessageDelayed(event, interval);
        }

        private void postMsgDelayed(int event, int obj, int interval) {
            Message msg = Message.obtain(mPhoneNumberHandler, event, obj);
            mPhoneNumberHandler.sendMessageDelayed(msg, interval);
        }

        private void retryForPhoneNumber(boolean isEmptyMsg, int event, int type, int interval) {
            IAosInfo aosInfo = AosFactory.getInstance().getAosInfo(mSlotId);
            if (aosInfo == null) {
                return;
            }

            increaseRetryCount();

            if (isRetryAllowed()) {
                ImsLog.d(mSlotId, "retry for phone number :: "
                        + mTryingCounter + "/" + getRetryMaxCount());

                ITelephonySubscriber ts = (ITelephonySubscriber)AgentFactory.getAgent(
                            AgentFactory.TELEPHONY_SUBSCRIBER, mSlotId);

                if (ts == null) {
                    return;
                }

                if (!isSimLoaded() || ts.getPhoneNumber() == null) {
                    if (isEmptyMsg) {
                        postEmptyMsgDelayed(event, interval);
                    } else {
                        postMsgDelayed(event, type, interval);
                    }
                } else {
                    ImsLog.d(mSlotId, "retry is successful");
                    aosInfo.notifyPhoneNumberState(type > 0, PhoneNumberState.RETRY_SUCCESS);
                }
            } else {
                ImsLog.e(mSlotId, "retry is failed");
                aosInfo.notifyPhoneNumberState(type > 0, PhoneNumberState.RETRY_FAILURE);
            }
        }

        private void retryForObtainPhoneNumberEvent(Message msg) {
            retryForPhoneNumber(false, EVENT_OBTAIN_PHONE_NUMBER_RETRY,
                    (Integer)msg.obj, getRetryInterval());
        }

        private void retryForPhoneNumberReadyEvent() {
            retryForPhoneNumber(true, EVENT_PHONE_NUMBER_READY_RETRY, 1, getRetryInterval());
        }

        private final class PhoneNumberHandler extends Handler {
            PhoneNumberHandler(Looper looper) {
                super(looper);
            }

            public void handleMessage(Message msg) {
                if (msg == null) {
                    return;
                }

                ImsLog.i(mSlotId, "PhoneNumberHandler :: what=" + msg.what);

                switch (msg.what) {
                    case EVENT_PHONE_NUMBER_READY_RETRY:
                        retryForPhoneNumberReadyEvent();
                        break;

                    case EVENT_OBTAIN_PHONE_NUMBER_RETRY:
                        retryForObtainPhoneNumberEvent(msg);
                        break;

                    default:
                        break;
                }
            }
        }
    }
}
