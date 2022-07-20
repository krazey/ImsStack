package com.android.imsstack.enabler.mtc;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;

import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.util.ImsLog;

public class MtcEmergencyServiceManager {

    private static final int MSG_SEND_REQUEST = 101;

    private final IBaseContext mContext;
    private MessageHandler mMessageHandler = null;
    private MtcCall mCall = null;
    private long mNativeObject = 0;

    public MtcEmergencyServiceManager(IBaseContext context) {
        mContext = context;
    }

    public void dispose() {
        log("dispose");

        clear();
    }

    public void init() {
        log("init");

        mMessageHandler = new MessageHandler(mContext.getCallLooper());
        mCall = null;
        mNativeObject = 0;
    }

    public void clear() {
        log("clear");

        mMessageHandler = null;
        mCall = null;
        mNativeObject = 0;
    }

    public void setNativeObject(long nativeObject) {
        mNativeObject = nativeObject;
    }

    public void setCall(MtcCall call) {
        log("setCall");

        mCall = call;
    }

    public void openEmergencyService() {
        log("openEmergencyService");

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IUMtcService.OPEN_EMERGENCY_SERVICE);
        sendRequest(parcel);
    }

    public void onEmergencyServiceStateChanged(int state, int reason, int serviceType) {
        logi("onEmergencyServiceStateChanged :: state=" + state + ", reason=" + reason +
                ", serviceType=" + serviceType);

        switch (state) {
            case IUMtcService.ES_IDLE:
                onEsIdle();
                break;
            case IUMtcService.ES_OPENING:
                onEsOpening();
                break;
            case IUMtcService.ES_OPENED:
                onEsOpened(serviceType);
                break;
            case IUMtcService.ES_UNAVAILABLE:
                onEsUnavailable();
                break;
            case IUMtcService.ES_IN_CALL:
                onEsInCall();
                break;
            default:
                break;
        }
    }

    private void onEsIdle() {
        log("onEsIdle");
        mCall = null;
    }

    private void onEsOpening() {
        log("onEsOpening");
    }

    private void onEsOpened(int serviceType) {
        log("onEsOpened :: serviceType=" + serviceType);

        if (mCall == null) {
            return;
        }

        mCall.createNativeCallObject();
        mCall.open(serviceType, true, false, false);
    }

    private void onEsUnavailable() {
        log("onEsUnavailable");
        mCall = null;
    }

    private void onEsInCall() {
        log("onEsInCall");
    }

    private void sendRequest(Parcel parcel) {
        Message.obtain(mMessageHandler, MSG_SEND_REQUEST, parcel).sendToTarget();
    }

    private long getNativeObject() {
        return mNativeObject;
    }

    private static void log(String s) {
        ImsLog.d("[GII-MTC] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[GII-MTC] " + s);
    }

    private class MessageHandler extends Handler {
        public MessageHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_SEND_REQUEST: {
                    log("MtcEmergencyServiceManager :: sendRequest");

                    Parcel parcel = (Parcel)msg.obj;

                    MtcJniProxy.getInstance().sendDataToNative(getNativeObject(), parcel);
                    break;
                }
                default:
                    break;
            }
        }
    }
}
