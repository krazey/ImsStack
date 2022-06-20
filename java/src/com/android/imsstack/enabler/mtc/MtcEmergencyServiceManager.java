package com.android.imsstack.enabler.mtc;

import android.content.Context;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;

import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.IUIMS;
import com.android.imsstack.enabler.mtc.reg.ImsServiceState;
import com.android.imsstack.jni.JNIIms;
import com.android.imsstack.util.ImsLog;

public class MtcEmergencyServiceManager {

    private static final int MSG_SEND_REQUEST = 101;

    private final IBaseContext mContext;
    private MessageHandler mMessageHandler;
    private MtcCall mCall;
    private int mCallAttributes = 0;
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
    }

    public void clear() {
        log("clear");

        mMessageHandler = null;
        mCall = null;
    }

    public void setNativeObject(long nativeObject) {
        mNativeObject = nativeObject;
    }

    public void setCallAndAttributes(MtcCall call, int callAttributes) {
        log("setCallAndAttributes");

        mCall = call;
        mCallAttributes = callAttributes;
    }

    public void openEmergencyService(long nativeObject) {
        log("openEmergencyService");

        setNativeObject(nativeObject);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IUMtcService.OPEN_EMERGENCY_SERVICE);
        sendRequest(parcel);
    }

    public void onEmergencyServiceStateChanged(int state, int reason) {
        logi("onEmergencyServiceStateChanged :: state=" + state + ", reason=" + reason);

        if (state == IUMtcService.ES_OPENED) {
            long nativeCallObject = JNIIms.getInterface(IUIMS.MTC_CALL, mContext.getSlotId());
            boolean wifi = ((mCallAttributes & MtcCall.FLAG_WIFI_EMERGENCY) ==
                    MtcCall.FLAG_WIFI_EMERGENCY);

            mCall.updateNativeCallObject(nativeCallObject);
            mCall.open(wifi, true, false, false);
        }
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
                    if (parcel == null) {
                        break;
                    }

                    byte[] data = parcel.marshall();
                    JNIIms.sendData(getNativeObject(), data);
                    parcel.recycle();
                    break;
                }
                default:
                    break;
            }
        }
    }
}
