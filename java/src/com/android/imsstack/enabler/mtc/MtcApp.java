/*
    Author
    <table>
    date        author                  description
    --------    --------------          ----------
    20131015    hwangoo.park@           Created
    </table>

    Description
*/

package com.android.imsstack.enabler.mtc;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;

import com.android.imsstack.core.ICommonPackageListener;
import com.android.imsstack.core.agents.agentif.ISharedState;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.IUIMS;
import com.android.imsstack.enabler.mtc.dialogs.DialogsInfo;
import com.android.imsstack.enabler.mtc.dialogs.IUDialogs;
import com.android.imsstack.jni.JNIIms;
import com.android.imsstack.jni.JNIImsListener;
import com.android.imsstack.util.ImsLog;

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
        public void onDialogStateChanged(MtcApp app, DialogsInfo dialogsInfo) {
            // no-op
        }

        /**
         * Notifies the application when the incoming call is received from the network.
         */
        public void onPreIncomingCallReceived(MtcApp app, long nativeCallObject) {
            // no-op
        }

        public void onIncomingCallInfoReceived(IncomingCallInfo incomingCallInfo) {
            // no-op
        }
    }

    private static final int MSG_NATIVE_BOOT_COMPLETED = 1;
    private static final int MSG_COMMON_PACKAGE_READY = 2;
    private static final int MSG_SEND_NOTIFICATION = 3;

    private final IBaseContext mContext;
    private final MtcCallManager mCM;
    private final MtcAppHandler mHandler;
    private long mNativeObject = 0;
    private JNIImsListenerProxy mNativeListener = new JNIImsListenerProxy();
    private ServiceStateListener mServiceStateListener = null;
    private CallListener mCallListener = null;

    public MtcApp(IBaseContext context) {
        mContext = context;

        mCM = new MtcCallManager(mContext);
        mHandler = new MtcAppHandler(mContext.getCallLooper());

        init();
    }

    public void init() {
        log("init");

        MtcStateUtils.initializeState(mContext.getContext(), mContext.getSlotId());

        mCM.init();
        mContext.addCommonPackageListener(mHandler);

        bindJNIService();
    }

    public void clear() {
        log("clear");

        unbindJNIService();

        mContext.removeCommonPackageListener(mHandler);

        mCM.clear();

        MtcStateUtils.initializeState(mContext.getContext(), mContext.getSlotId());
    }

    public MtcCallManager getCallManager() {
        return mCM;
    }

    public MtcCall createCall(int callAttributes) {
        if (getJNIService() == 0) {
            bindJNIService();

            if (getJNIService() == 0) {
                loge("Native object is null");
                return null;
            }
        }

        long nativeCallObject = JNIIms.getInterface(IUIMS.MTC_CALL, mContext.getSlotId());

        MtcCall call = new MtcCall(
                mContext, mCM.getCallTracker(), nativeCallObject, callAttributes,
                mCM.getVacantCallIndex(), "");

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

        mContext.removeCommonPackageListener(mHandler);

        mCM.dispose();

        MtcStateUtils.initializeState(mContext.getContext(), mContext.getSlotId());
    }

    public void notifySrvccStateChanged(int state) {
        logi("notifySrvccStateChanged :: state=" + state);

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcService.SRVCC_STATE_CHANGED);
        parcel.writeInt(state);

        sendNotification(parcel);
    }

    public void setTerminalBasedCallWaiting(boolean provisioned, boolean enabled) {
        logi("setTerminalBasedCallWaiting :: provisioned=" + provisioned + ", enabled=" + enabled);

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcService.SET_TERMINAL_BASED_CALL_WAITING);
        parcel.writeInt(provisioned ? 1 : 0);
        parcel.writeInt(enabled ? 1 : 0);

        sendNotification(parcel);
    }

    public boolean isServiceValid() {
        return mNativeObject != 0;
    }

    private long getJNIService() {
        return mNativeObject;
    }

    private void bindJNIService() {
        if (mNativeObject != 0) {
            log("bindJNIService :: Object already exists");
            return;
        }

        if (!mContext.isCommonPackageReady()) {
            log("bindJNIService :: Wait for common package ready...");
            return;
        }

        ISharedState ss = mContext.getSharedState();

        if (ss != null) {
            if (!ss.isNativeBootCompleted()) {
                ss.unregisterForNativeBootComplete(mHandler);
                ss.registerForNativeBootComplete(mHandler, MSG_NATIVE_BOOT_COMPLETED, null);
                log("bindJNIService :: Wait for native ready...");
                return;
            }
        }

        mNativeObject = JNIIms.getInterface(IUIMS.APP_MTC, mContext.getSlotId());

        if (mNativeObject != 0) {
            JNIIms.setListener(mNativeObject, mNativeListener);

            if (ss != null) {
                ss.unregisterForNativeBootComplete(mHandler);
            }
        }
    }

    private void unbindJNIService() {
        if (mNativeObject != 0) {
            JNIIms.removeListener(mNativeObject, mNativeListener);
            JNIIms.releaseInterface(mNativeObject);
            mNativeObject = 0;
        }
    }

    private void rejectAndCloseCall(final MtcCall call) {
        if (call == null) {
            return;
        }

        mContext.getExecutor().execute(new Runnable() {
            @Override
            public void run() {
                call.reject(IUMtcCall.Reject_Reason.REJECT_REASON_DECLINE_NOANSWER);
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

    private class JNIImsListenerProxy implements JNIImsListener {
        @Override
        public void onMessage(Parcel parcel) {
            int msg = parcel.readInt();

            // LogFilter compatibility: Mtc-MSG
            logi("MtcApp::Mtc-MSG=" + msg);

            switch (msg) {
                case IUMtcService.PRE_INCOMING_CALL: // FALL-THROUGH
                case IUMtcService.INCOMING_CALL_INFO: // FALL-THROUGH
                case IUMtcService.AUTO_REJECTED_CALL: // FALL-THROUGH
                case IUDialogs.NOTIFY_DIALOG_INFO: // FALL-THROUGH
                    onMessageForCallApp(msg, parcel);
                    break;
                case IUMtcService.SERVICE_CHANGED: // FALL-THROUGH
                case IUMtcService.E_SERVICE_CHANGED:
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
                String logTag = parcel.readString();

                MtcCall call = createCall(0);

                if (mCallListener != null && call != null) {
                    mCallListener.onPreIncomingCallReceived(MtcApp.this, call.getNativeCallId());
                    call.attach(nativeCallKey);
                } else {
                    rejectAndCloseCall(call);
                }
            }
            // TODO : consider how to handle this msg.
            // else if (msg == IUMtcService.INCOMING_CALL_INFO) {
            //     IncomingCallInfo incomingCallInfo = new IncomingCallInfo(parcel);

            //     if (mCallListener != null) {
            //         mCallListener.onIncomingCallInfoReceived(incomingCallInfo);
            //     }
            // }
            // else if (msg == IUMtcService.AUTO_REJECTED_CALL) {
            //     IncomingRejectedMtcCall incomingCall = new IncomingRejectedMtcCall(parcel);
            //     MtcCall call = new MtcCall(mContext,
            //             mCM.getCallTracker(), 0/*callObject*/, 0, mCM.getVacantCallIndex(),
            //                     incomingCall.logTag);

            //     MtcCall call = getPendingCall(incomingCall.callMtcKey, false);

            //     if (call == null) {
            //         loge("No pending Mtc call");
            //         return;
            //     }

            //     call.updateCallExtras(incomingCall);

            //     if (mCallListener != null) {
            //         mCM.attachIncomingCall(call);
            //         //mCallListener.onIncomingCallReceived(MtcApp.this, 0, incomingCall);
            //     }
            // }
            else if (msg == IUDialogs.NOTIFY_DIALOG_INFO) {
                DialogsInfo dialogsInfo = new DialogsInfo(parcel);

                if (mCallListener != null) {
                    mCallListener.onDialogStateChanged(MtcApp.this, dialogsInfo);
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

                logi("VOLTE-E-REG :: state=" + state + ", reason=" + reason);

                if (mServiceStateListener != null) {
                    mServiceStateListener.onEmergencyServiceStateChanged(
                            MtcApp.this, state, reason);
                }
            }
        }
    }

    private class MtcAppHandler extends Handler implements ICommonPackageListener {
        public MtcAppHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_NATIVE_BOOT_COMPLETED: {
                    log("bindJNIService :: onBootCompleted");
                    bindJNIService();
                    break;
                }

                case MSG_COMMON_PACKAGE_READY: {
                    log("bindJNIService :: onCommonPackageReady");
                    bindJNIService();
                    break;
                }

                case MSG_SEND_NOTIFICATION: {
                    log("MtcApp :: sendNotification");
                    Parcel parcel = (Parcel)msg.obj;

                    if (parcel == null) {
                        break;
                    }

                    byte[] data = parcel.marshall();

                    JNIIms.sendData(getJNIService(), data);

                    parcel.recycle();
                    break;
                }

                default:
                    // no-op
                    break;
            }
        }

        @Override
        public void onCommonPackageReady(int slotId) {
            logi("onCommonPackageReady :: slotId=" + slotId
                    + ", mySlotId=" + mContext.getSlotId());

            if (slotId != mContext.getSlotId()) {
                return;
            }

            sendEmptyMessage(MSG_COMMON_PACKAGE_READY);
        }

        @Override
        public void onCommonPackageStop(int slotId) {
            logi("onCommonPackageStop :: slotId=" + slotId);
        }
    }
}
