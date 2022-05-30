package com.android.imsstack.core.agents;

import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;

import com.android.imsstack.core.agents.agentif.ITelephonySubscriber;
import com.android.imsstack.core.agents.agentif.IVoLteAgent;
import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.enabler.aos.IAosInfo;
import com.android.imsstack.enabler.aos.IAosInfo.PhoneNumberState;
import com.android.imsstack.system.IJNIUpCallEvt;
import com.android.imsstack.system.ImsEventDef;
import com.android.imsstack.system.JNIUpCallEvtManager;
import com.android.imsstack.util.ImsLog;

import java.util.concurrent.ConcurrentHashMap;

public class PhoneNumberAgent implements IVoLteAgent {
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

    @Override
    public void cleanup() {
        ImsLog.d("size=" + sPhoneNumber.size());
        sPhoneNumber.clear();
    }

    @Override
    public void start(int slotID) {
        ImsLog.d("size=" + sPhoneNumber.size() + ", slotId=" + slotID);

        if (!sPhoneNumber.containsKey(slotID)) {
            sPhoneNumber.put(slotID, new PhoneNumber(slotID));
        }
    }

    @Override
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

    public class PhoneNumber {
        private final int mSlotId;

        private int mTryingCounter = 0;
        private int mRetryMaxCount = 10;
        private int mRetryIntervalSec = 5;
        private PhoneNumberHandler mPhoneNumberHandler = new PhoneNumberHandler();

        public PhoneNumber(int slotID) {
            ImsLog.d(slotID, "");

            mSlotId = slotID;

            IJNIUpCallEvt jniEvt = JNIUpCallEvtManager.getInstance().getJNIUpCallEvt(mSlotId);

            if (jniEvt != null) {
                jniEvt.registerForObtainPhoneNumber(mPhoneNumberHandler,
                        ImsEventDef.IMS_EVENT_OBTAIN_PHONE_NUMBER, null);
            }
        }

        public void cleanup() {
            ImsLog.d(mSlotId, "");

            if (mPhoneNumberHandler != null) {
                IJNIUpCallEvt jniEvt = JNIUpCallEvtManager.getInstance().getJNIUpCallEvt(mSlotId);

                if (jniEvt != null) {
                    jniEvt.unregisterForObtainPhoneNumber(mPhoneNumberHandler);
                }

                mPhoneNumberHandler.removeCallbacksAndMessages(null);
            }
        }

        private void clearRetryCount() {
            mTryingCounter = 0;
        }

        // return : milli-seconds
        private int getRetryInterval() {
            return mRetryIntervalSec * 1000;
        }

        private int getRetryMaxCount() {
            return mRetryMaxCount;
        }

        private void handleObtainPhoneNumberEvent(Message msg) {
            AsyncResult ar = (AsyncResult)msg.obj;

            if (ar == null) {
                return;
            }

            Message eventMsg = (Message)ar.result;

            if (eventMsg == null) {
                return;
            }

            if (eventMsg.arg1 == ImsEventDef.IMS_OBTAIN_PHONE_NUMBER_CLEAR) {
                ImsLog.d(mSlotId, "clean up obtaining phone number");
                clearRetryCount();
                mPhoneNumberHandler.removeMessages(EVENT_OBTAIN_PHONE_NUMBER_RETRY);
                return;
            }

            boolean refresh = (eventMsg.arg1 ==
                    ImsEventDef.IMS_OBTAIN_PHONE_NUMBER_REFRESH) ? true : false;

            int obj = (refresh) ?
                    ImsEventDef.IMS_PHONE_NUMBER_REFRESH : ImsEventDef.IMS_PHONE_NUMBER_INITIAL;

            clearRetryCount();

            if (refresh) {
                setRetryMaxCount(15);
                setRetryInterval(3);
            } else {
                setRetryMaxCount(4);
                setRetryInterval(3);
            }

            ImsLog.d(mSlotId, "maxCount=" + getRetryMaxCount() + ", retryInterval="
                    + getRetryInterval() + " (msec)" + ", type=" + obj);

            postMsgDelayed(EVENT_OBTAIN_PHONE_NUMBER_RETRY, obj, getRetryInterval());
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
            retryForPhoneNumber(true, EVENT_PHONE_NUMBER_READY_RETRY,
                    ImsEventDef.IMS_PHONE_NUMBER_REFRESH, getRetryInterval());
        }

        private void setRetryMaxCount(int count) {
            mRetryMaxCount = count;
        }

        private void setRetryInterval(int seconds) {
            mRetryIntervalSec = seconds;
        }

        private final class PhoneNumberHandler extends Handler {
            public void handleMessage(Message msg) {
                if (msg == null) {
                    return;
                }

                ImsLog.i(mSlotId, "PhoneNumberHandler :: what=" + msg.what);

                switch (msg.what) {
                    case ImsEventDef.IMS_EVENT_OBTAIN_PHONE_NUMBER:
                        handleObtainPhoneNumberEvent(msg);
                        break;

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
