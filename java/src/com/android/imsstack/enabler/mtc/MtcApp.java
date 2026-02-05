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

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.telephony.emergency.EmergencyNumber.EmergencyCallRouting;

import androidx.annotation.NonNull;

import com.android.imsstack.core.agents.NativeStateInterface;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.externalcalls.ExternalCalls;
import com.android.imsstack.internal.ImsStackRegistry;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.internal.imsservice.MmTelFeatureRegistry;
import com.android.imsstack.jni.JniImsListener;
import com.android.imsstack.jni.JniObjectId;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.IndentingPrintWriter;
import com.android.internal.annotations.VisibleForTesting;

import java.io.Closeable;

public class MtcApp implements Closeable {

    /**
     * Listener for events relating to MTC service state.
     */
    public static class ServiceStateListener {
        /**
         * Notifies the application when the emergency service state is changed.
         */
        public void onEmergencyServiceStateChanged(MtcApp app, int state, int reason) {
            // no-op
        }

        /**
         * Notifies the application when the normal service state is changed.
         */
        public void onServiceStateChanged(MtcApp app, int state, int reason) {
            // no-op
        }
    }

    /**
     * Listener for events relating to MTC service state and incoming call.
     */
    public static class CallListener {
        /**
         * Notifies the application when the dialog state is changed.
         */
        public void onExternalCallStateChanged(MtcApp app, ExternalCalls externalCalls) {
            // no-op
        }

        /**
         * Notifies the application when the incoming call is received from the network.
         */
        public void onPreIncomingCallReceived(MtcApp app, long nativeCallObject) {
            // no-op
        }
    }

    protected static final int MSG_IMS_SERVICE_STARTED = 1;
    protected static final int MSG_SEND_NOTIFICATION = 2;
    protected static final int MSG_MESSAGE_RECEIVED = 3;

    private final IBaseContext mContext;
    private final IMtcCallManager mCM;
    private final MtcAppHandler mHandler;
    private final MtcEmergencyServiceManager mEmergencyServiceManager;
    private final MtcTerminalBasedSupplementaryServiceNotifier mTbSsNotifier;
    private long mNativeObject = 0;
    private JNIImsListenerProxy mNativeListener = new JNIImsListenerProxy();
    private ServiceStateListener mServiceStateListener = null;
    private CallListener mCallListener = null;
    private MtcJniProxy mMtcJniProxy;
    protected MmtelFeatureListener mMmtelFeatureListener = null;
    private NativeStateInterface.Listener mNativeStateListener;
    private long mPreIncomingNativeCallId = 0;
    private long mIncomingCallKey = 0;

    public MtcApp(IBaseContext context) {
        mContext = context;

        mCM = new MtcCallManager(mContext);
        mHandler = new MtcAppHandler(mContext.getCallLooper());
        mEmergencyServiceManager =
                new MtcEmergencyServiceManager(mContext, mCM.getCallStateTracker());
        mTbSsNotifier = new MtcTerminalBasedSupplementaryServiceNotifier(
                mContext.getSlotId(), mContext.getCallLooper());
        mMtcJniProxy = MtcJniProxy.getInstance();

        init();
    }

    @VisibleForTesting
    public MtcApp(IBaseContext context, IMtcCallManager mtcCallManager, Looper looper,
            MtcEmergencyServiceManager mtcEmergencyServiceManager,
            MtcTerminalBasedSupplementaryServiceNotifier tbSsNotifier, MtcJniProxy mtcJniProxy) {
        mContext = context;

        mCM = mtcCallManager;
        mHandler = new MtcAppHandler(looper);
        mEmergencyServiceManager = mtcEmergencyServiceManager;
        mTbSsNotifier = tbSsNotifier;
        mMtcJniProxy = mtcJniProxy;
    }

    public void init() {
        log("init");

        initializeState();

        mCM.init();
        mContext.addImsServiceListener(mHandler);
        mEmergencyServiceManager.init();
        mTbSsNotifier.init();

        MmTelFeatureRegistry mmtelFr = ImsServiceRegistry.getInstance(mContext.getSlotId())
                .getMmTelFeatureRegistry();

        if (mmtelFr != null) {
            mMmtelFeatureListener = new MmtelFeatureListener();
            mmtelFr.addListener(mMmtelFeatureListener);
        }

        bindJNIService();
    }

    public void clear() {
        log("clear");

        unbindJNIService();

        mEmergencyServiceManager.clear();
        mContext.removeImsServiceListener(mHandler);
        mCM.clear();
        mTbSsNotifier.deinit();

        initializeState();

        if (mMmtelFeatureListener != null) {
            MmTelFeatureRegistry mmtelFr = ImsServiceRegistry.getInstance(mContext.getSlotId())
                    .getMmTelFeatureRegistry();
            if (mmtelFr != null) {
                mmtelFr.removeListener(mMmtelFeatureListener);
            }
            mMmtelFeatureListener = null;
        }

        if (mNativeStateListener != null) {
            NativeStateInterface nsi = mContext.getNativeStateInterface();
            if (nsi != null) {
                nsi.removeListener(mNativeStateListener);
            }
            mNativeStateListener = null;
        }

        mPreIncomingNativeCallId = 0;
        mIncomingCallKey = 0;
    }

    public IMtcCallManager getCallManager() {
        return mCM;
    }

    public MtcEmergencyServiceManager getMtcEmergencyServiceManager() {
        return mEmergencyServiceManager;
    }

    /**
     * Notifies {@link MtcEmergencyServiceManager} to do registration for emergency call.
     *
     * @param call The target emergency call.
     * @param emergencyRouting The emergency call routing value.
     */
    public void openEmergencyService(MtcCall call, @EmergencyCallRouting int emergencyRouting) {
        mEmergencyServiceManager.setCall(call);
        mEmergencyServiceManager.openEmergencyService(
                emergencyRouting, mContext.getServiceStateTracker());
    }

    /**
     * Creates {@link MtcCall} according to received information and attaches it
     * to {@link MtcCallManager}.
     *
     * @param callAttributes The information used when creating {@link MtcCall}.
     * @return The created {@link MtcCall}.
     */
    public MtcCall createMtcCallAndAttach(int callAttributes) {
        return createMtcCallAndAttach(callAttributes, "");
    }

    /**
     * Creates {@link MtcCall} according to received information and attaches it
     * to {@link MtcCallManager}.
     *
     * @param callAttributes The information used when creating {@link MtcCall}.
     * @param logTag The log tag for the call.
     * @return The created {@link MtcCall}.
     */
    private MtcCall createMtcCallAndAttach(int callAttributes, String logTag) {
        if (getJNIService() == 0) {
            bindJNIService();

            if (getJNIService() == 0) {
                loge("Native object is null");
                return null;
            }
        }

        MtcCall call = createMtcCall(callAttributes, logTag);

        if (call.isMO()) {
            mCM.attachCall(call);
        } else {
            mCM.attachPreIncomingCall(call);
        }

        return call;
    }

    public void destroyCall(MtcCall call) {
        log("destroyCall :: call=" + call);

        if (call != null) {
            call.close();
        }
    }

    /**
     * Creates {@link MtcCall} according to received information.
     *
     * @param callAttributes The information used when creating {@link MtcCall}.
     * @param logTag The log tag for the call.
     * @return The created {@link MtcCall}.
     */
    public MtcCall createMtcCall(int callAttributes, String logTag) {
        return new MtcCall(mContext, mCM.getCallTracker(), callAttributes, mCM.getNextCallIndex(),
                logTag);
    }

    /**
     * Gets pending call for incoming call.
     */
    public MtcCall getPendingCall(long callId) {
        logi("getPendingCall :: callId=" + Long.toHexString(callId));

        MtcCall call = (MtcCall) mCM.getPendingCall(callId);

        if (call == null) {
            return null;
        }

        if (call.isTerminated()) {
            log("Call is cancelled or terminated");
            call.close();
            return null;
        }

        return call;
    }

    /**
     * Clears information related to a incoming call.
     */
    public void onIncomingCallTaken(MtcCall mtcCall) {
        if (mPreIncomingNativeCallId == mtcCall.getNativeCallId()) {
            mPreIncomingNativeCallId = 0;
            mIncomingCallKey = 0;
        }
    }

    public void setCallListener(MtcApp.CallListener listener) {
        mCallListener = listener;
    }

    public void setServiceStateListener(MtcApp.ServiceStateListener listener) {
        mServiceStateListener = listener;
    }

    @Override
    public void close() {
        log("close :: nativeObject=" + getJNIService());

        unbindJNIService();

        mContext.removeImsServiceListener(mHandler);

        mCM.dispose();

        initializeState();

        if (mNativeStateListener != null) {
            NativeStateInterface nsi = mContext.getNativeStateInterface();
            if (nsi != null) {
                nsi.removeListener(mNativeStateListener);
            }
            mNativeStateListener = null;
        }
    }

    @VisibleForTesting
    protected class MmtelFeatureListener implements MmTelFeatureRegistry.Listener {
        @Override
        public void onSrvccStateChanged(int srvccState) {
            logi("onSrvccStateChanged :: SRVCC State = " + srvccState);
            Parcel parcel = Parcel.obtain();

            parcel.writeInt(IUMtcService.SRVCC_STATE_CHANGED);
            parcel.writeInt(srvccState);

            sendNotification(parcel);
        }
    }

    public boolean isServiceValid() {
        return mNativeObject != 0;
    }

    /**
     * Initializes {@code ImsStateStore}.
     */
    public void initializeState() {
        MtcStateUtils.initializeState(mContext.getContext(), mContext.getSlotId());
    }

    public MtcAppHandler getHandler() {
        return mHandler;
    }

    @VisibleForTesting
    public JniImsListener getNativeListener() {
        return mNativeListener;
    }

    @VisibleForTesting
    public void setNativeObj(long natieObj) {
        mNativeObject = natieObj;
    }

    public long getJNIService() {
        return mNativeObject;
    }

    /**
     * Checks if an outgoing call is barred based on the call type and the recipient's number.
     *
     * @param callType The type of the outgoing call.
     * @param callee The phone number of the recipient of the outgoing call to check
     *               if it is an international number.
     * @return {@code true} if the outgoing call is barred, {@code false} otherwise.
     */
    public boolean isOutgoingCallBarringActivated(int callType, String callee) {
        return mTbSsNotifier.isOutgoingCallBarringActivated(callType, callee);
    }

    private void bindJNIService() {
        if (mNativeObject != 0) {
            log("bindJNIService :: Object already exists");
            return;
        }

        if (!mContext.isImsServiceStarted()) {
            log("bindJNIService :: Wait for IMS service start.");
            return;
        }

        NativeStateInterface nsi = mContext.getNativeStateInterface();

        if (nsi != null) {
            if (!nsi.isServiceReady()) {
                if (mNativeStateListener == null) {
                    mNativeStateListener = new NativeStateInterface.Listener() {
                        @Override
                        public void onNativeServiceReady() {
                            log("bindJNIService :: onNativeServiceReady.");
                            bindJNIService();
                        }
                    };
                } else {
                    nsi.removeListener(mNativeStateListener);
                }
                nsi.addListener(mNativeStateListener);
                log("bindJNIService :: Wait for native ready...");
                return;
            }
        }
        mNativeObject = mMtcJniProxy.getJniInterfaceAndSetListener(
                mContext.getSlotId(), JniObjectId.MTC, mNativeListener);

        if (mNativeObject != 0) {
            if (nsi != null && mNativeStateListener != null) {
                nsi.removeListener(mNativeStateListener);
                mNativeStateListener = null;
            }

            mEmergencyServiceManager.setNativeObject(mNativeObject);
            mTbSsNotifier.setHandler(mHandler);
            mTbSsNotifier.notifyInfo();
        }
    }

    private void unbindJNIService() {
        if (mNativeObject != 0) {
            mMtcJniProxy.releaseJniInterfaceAndrRemoveListener(
                    mNativeObject, mNativeListener);
            mNativeObject = 0;

            mEmergencyServiceManager.setNativeObject(mNativeObject);
            mTbSsNotifier.setHandler(null);
        }
    }

    private void rejectAndCloseCall(final MtcCall call) {
        if (call == null) {
            return;
        }

        mContext.getExecutor().execute(new Runnable() {
            @Override
            public void run() {
                call.reject(CallReasonInfo.CODE_USER_NOANSWER);
                android.os.SystemClock.sleep(200);
                call.close();
            }
        });
    }

    private void sendNotification(Parcel parcel) {
        if (!isServiceValid()) {
            parcel.recycle();
            loge("Service is not valid");
            return;
        }

        Message.obtain(mHandler, MSG_SEND_NOTIFICATION, parcel).sendToTarget();
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

    private class JNIImsListenerProxy implements JniImsListener {
        @Override
        public void onMessage(Parcel parcel) {
            byte[] data = parcel.marshall();

            if (data == null) {
                return;
            }

            Message.obtain(mHandler, MSG_MESSAGE_RECEIVED, data).sendToTarget();
        }

        private void handleMessage(Parcel parcel) {
            int msg = parcel.readInt();

            // LogFilter compatibility: Mtc-MSG
            logi("MtcApp::Mtc-MSG=" + msg);

            switch (msg) {
                case IUMtcService.PRE_INCOMING_CALL: // FALL-THROUGH
                case IUMtcService.AUTO_REJECTED_CALL: // FALL-THROUGH
                case IUMtcService.EXTERNAL_CALLS_CHANGED: // FALL-THROUGH
                    onMessageForCallApp(msg, parcel);
                    break;
                case IUMtcService.SERVICE_CHANGED: // FALL-THROUGH
                case IUMtcService.E_SERVICE_CHANGED: // FALL-THROUGH
                case IUMtcService.JNI_READY:
                    onMessageForRegApp(msg, parcel);
                    break;
                default:
                    // no-op
                    break;
            }
        }

        private void onMessageForCallApp(int msg, Parcel parcel) {
            if (msg == IUMtcService.PRE_INCOMING_CALL) {
                long nativeCallKey = parcel.readLong();

                if (mIncomingCallKey != 0 && mIncomingCallKey != nativeCallKey) {
                    loge("Only one incoming call can be processed at a time");
                    return;
                }

                mIncomingCallKey = nativeCallKey;

                MtcCall call = createMtcCallAndAttach(0, parcel.readString());
                call.attach(nativeCallKey);

                if (mCallListener != null) {
                    mPreIncomingNativeCallId = call.getNativeCallId();
                    mCallListener.onPreIncomingCallReceived(MtcApp.this, mPreIncomingNativeCallId);
                } else {
                    rejectAndCloseCall(call);
                }
            } else if (msg == IUMtcService.AUTO_REJECTED_CALL) {
                long nativeCallKey = parcel.readLong();

                MtcCall call;
                if (mIncomingCallKey == nativeCallKey) {
                    // PRE_INCOMING_CALL and AUTO_REJECTED_CALL for a same incoming call.
                    parcel.readString();
                    call = getPendingCall(mPreIncomingNativeCallId);
                } else {
                    // only AUTO_REJECTED_CALL for a differencet incoming call.
                    call = createMtcCallAndAttach(0, parcel.readString());
                    call.attach(nativeCallKey);

                    if (mCallListener != null) {
                        mCallListener.onPreIncomingCallReceived(
                                MtcApp.this, call.getNativeCallId());
                    }
                }

                call.invokeIncomingCallReceivedForAutoRejecting(
                        new IncomingRejectedMtcCall(parcel));
            } else if (msg == IUMtcService.EXTERNAL_CALLS_CHANGED) {
                ExternalCalls externalCalls = new ExternalCalls(parcel);

                if (mCallListener != null) {
                    mCallListener.onExternalCallStateChanged(MtcApp.this, externalCalls);
                }
            }
        }

        private void onMessageForRegApp(int msg, Parcel parcel) {
            if (msg == IUMtcService.SERVICE_CHANGED) {
                int state = parcel.readInt();
                int reason = parcel.readInt();

                logi("VOLTE-REG :: state=" + state + ", reason=" + reason);

                if (mServiceStateListener != null) {
                    mServiceStateListener.onServiceStateChanged(
                            MtcApp.this, state, reason);
                }
            }
            else if (msg == IUMtcService.E_SERVICE_CHANGED) {
                int state = parcel.readInt();
                int reason = parcel.readInt();
                int serviceType = parcel.readInt();

                logi("VOLTE-E-REG :: state=" + state + ", reason=" + reason +
                        ", serviceType=" + serviceType);

                mEmergencyServiceManager.onEmergencyServiceStateChanged(state, reason, serviceType);

                if (mServiceStateListener != null) {
                    mServiceStateListener.onEmergencyServiceStateChanged(
                            MtcApp.this, state, reason);
                }
            }
            else if (msg == IUMtcService.JNI_READY) {
            }
        }
    }

    protected class MtcAppHandler extends Handler implements ImsStackRegistry.ImsServiceListener {
        public MtcAppHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_IMS_SERVICE_STARTED: {
                    log("bindJNIService :: onImsServiceStarted");
                    bindJNIService();
                    break;
                }

                case MSG_SEND_NOTIFICATION: {
                    log("MtcApp :: sendNotification");
                    Parcel parcel = (Parcel)msg.obj;

                    mMtcJniProxy.sendDataToNative(getJNIService(), parcel);
                    break;
                }

                case MSG_MESSAGE_RECEIVED: {
                    byte[] data = (byte[]) msg.obj;
                    Parcel parcel = Parcel.obtain();
                    parcel.unmarshall(data, 0, data.length);
                    parcel.setDataPosition(0);

                    try {
                        mNativeListener.handleMessage(parcel);
                    } finally {
                        parcel.recycle();
                    }
                    break;
                }

                default:
                    // no-op
                    break;
            }
        }

        @Override
        public void onImsServiceStarted(int slotId) {
            logi("onImsServiceStarted: slotId=" + slotId + ", mySlotId=" + mContext.getSlotId());

            if (slotId != mContext.getSlotId()) {
                return;
            }

            sendEmptyMessage(MSG_IMS_SERVICE_STARTED);
        }

        @Override
        public void onImsServiceStopped(int slotId) {
            logi("onImsServiceStopped: slotId=" + slotId);
        }
    }

    /**
     * Dump this instance into a readable format for dumpsys usage.
     */
    public void dump(@NonNull IndentingPrintWriter pw) {
        pw.println("Mtc:");
        pw.increaseIndent();
        pw.println("jniService=" + getJNIService());
        pw.decreaseIndent();
    }
}
