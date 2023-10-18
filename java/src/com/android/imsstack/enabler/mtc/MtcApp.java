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

import com.android.imsstack.core.agents.NativeStateInterface;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.externalcalls.ExternalCalls;
import com.android.imsstack.internal.ImsStackRegistry;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.internal.imsservice.MmTelFeatureRegistry;
import com.android.imsstack.jni.JniImsListener;
import com.android.imsstack.jni.JniObjectId;
import com.android.imsstack.util.ImsLog;
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

    private static final int MSG_IMS_SERVICE_STARTED = 1;
    private static final int MSG_SEND_NOTIFICATION = 2;

    private final IBaseContext mContext;
    private final IMtcCallManager mCM;
    private final MtcAppHandler mHandler;
    private final MtcEmergencyServiceManager mEmergencyServiceManager;
    private long mNativeObject = 0;
    private JNIImsListenerProxy mNativeListener = new JNIImsListenerProxy();
    private ServiceStateListener mServiceStateListener = null;
    private CallListener mCallListener = null;
    private MtcJniProxy mMtcJniProxy;
    protected MmtelFeatureListener mMmtelFeatureListener = null;
    private NativeStateInterface.Listener mNativeStateListener;
    private long mPreIncomingNativeCallId = 0;

    public MtcApp(IBaseContext context) {
        mContext = context;

        mCM = new MtcCallManager(mContext);
        mHandler = new MtcAppHandler(mContext.getCallLooper());
        mEmergencyServiceManager =
                new MtcEmergencyServiceManager(mContext, mCM.getCallStateTracker());
        mMtcJniProxy = MtcJniProxy.getInstance();

        init();
    }

    @VisibleForTesting
    public MtcApp(IBaseContext context, IMtcCallManager mtcCallManager, Looper looper,
            MtcEmergencyServiceManager mtcEmergencyServiceManager, MtcJniProxy mtcJniProxy) {
        mContext = context;

        mCM = mtcCallManager;
        mHandler = new MtcAppHandler(looper);
        mEmergencyServiceManager = mtcEmergencyServiceManager;
        mMtcJniProxy = mtcJniProxy;
    }

    public void init() {
        log("init");

        initializeState();

        mCM.init();
        mContext.addImsServiceListener(mHandler);
        mEmergencyServiceManager.init();

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
    }

    public IMtcCallManager getCallManager() {
        return mCM;
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
     * @param callAttributes The information used when creates {@link MtcCall}.
     * @return The created {@link MtcCall}.
     */
    public MtcCall createMtcCallAndAttach(int callAttributes) {
        if (getJNIService() == 0) {
            bindJNIService();

            if (getJNIService() == 0) {
                loge("Native object is null");
                return null;
            }
        }

        MtcCall call = createMtcCall(callAttributes);

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
     * @param callAttributes The information used when creates {@link MtcCall}.
     * @return The created {@link MtcCall}.
     */
    public MtcCall createMtcCall(int callAttributes) {
        return new MtcCall(mContext, mCM.getCallTracker(), callAttributes,
                mCM.getVacantCallIndex(), "");
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
        public void onTerminalBasedCallWaitingStatusChanged() {
            setTerminalBasedCallWaiting();
        }

        @Override
        public void onSrvccStateChanged(int srvccState) {
            logi("onSrvccStateChanged :: SRVCC State = " + srvccState);
            Parcel parcel = Parcel.obtain();

            parcel.writeInt(IUMtcService.SRVCC_STATE_CHANGED);
            parcel.writeInt(srvccState);

            sendNotification(parcel);
        }
    }

    /**
     * Sends received terminal-based call waiting value to the Native.
     */
    public void setTerminalBasedCallWaiting() {
        boolean enabled = false;

        MmTelFeatureRegistry mmtelFr = ImsServiceRegistry.getInstance(mContext.getSlotId())
                .getMmTelFeatureRegistry();

        if (mmtelFr != null) {
            enabled = mmtelFr.isTerminalBasedCallWaitingEnabled();
        }
        logi("setTerminalBasedCallWaiting :: enabled=" + enabled);

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcService.SET_TERMINAL_BASED_CALL_WAITING);
        parcel.writeInt(enabled ? 1 : 0);

        sendNotification(parcel);
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

    @VisibleForTesting
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
            setTerminalBasedCallWaiting();
        }
    }

    private void unbindJNIService() {
        if (mNativeObject != 0) {
            mMtcJniProxy.releaseJniInterfaceAndrRemoveListener(
                    mNativeObject, mNativeListener);
            mNativeObject = 0;

            mEmergencyServiceManager.setNativeObject(mNativeObject);
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
        ImsLog.d("[GII-MTC] " + s);
    }

    private static void loge(String s) {
        ImsLog.e("[GII-MTC] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[GII-MTC] " + s);
    }

    private class JNIImsListenerProxy implements JniImsListener {
        @Override
        public void onMessage(Parcel parcel) {
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
                parcel.readString();

                MtcCall call = createMtcCallAndAttach(0);

                if (mCallListener != null && call != null) {
                    mPreIncomingNativeCallId = call.getNativeCallId();
                    mCallListener.onPreIncomingCallReceived(MtcApp.this, mPreIncomingNativeCallId);
                    call.attach(nativeCallKey);
                } else {
                    rejectAndCloseCall(call);
                }
            } else if (msg == IUMtcService.AUTO_REJECTED_CALL) {
                MtcCall call = getPendingCall(mPreIncomingNativeCallId);
                if (call == null) {
                    call = createMtcCallAndAttach(0);
                    if (mCallListener != null) {
                        mCallListener.onPreIncomingCallReceived(
                                MtcApp.this, call.getNativeCallId());
                    }
                } else {
                    mPreIncomingNativeCallId = 0;
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
                setTerminalBasedCallWaiting();
            }
        }
    }

    private class MtcAppHandler extends Handler implements ImsStackRegistry.ImsServiceListener {
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
}
