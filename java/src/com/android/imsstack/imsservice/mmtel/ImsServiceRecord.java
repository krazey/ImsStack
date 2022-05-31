/*
    Author
    <table>
    date        author                  description
    --------    --------------          ----------
    20150618    hwangoo.park@           Created
    </table>

    Description
*/

package com.android.imsstack.imsservice.mmtel;

import android.content.Context;

import com.android.imsstack.imsservice.base.ImsContext;
import com.android.imsstack.imsservice.sipcontroller.ImsSipTransport;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.util.ImsLog;

import java.util.concurrent.Executor;

public final class ImsServiceRecord {
    public static interface Listener {
        public void onServiceRecordStateChanged();
    };

    private final Object mLock = new Object();
    private final ImsContext mContext;
    private Listener mListener = null;
    private ImsConfigImpl mConfig = null;
    private ImsRegistrationImpl mRegistration = null;
    private ImsRegistrationTracker mRegTracker = null;
    private ImsCallApp mCallApp = null;
    private boolean mServiceUp = false;

    //This is an implementation of SipTransport required for single registration support.
    private ImsSipTransport mSipTransport = null;
    private Executor mExecutor = null;
    private int mSlotId = -1;


    public ImsServiceRecord(Context context, Executor executor, int phoneId) {
        log("ImsServiceRecord :: phoneId=" + phoneId);

        mContext = new ImsContext(context, executor, phoneId);

        mExecutor = executor;
        mSlotId = phoneId;//NOTE: Slot id is used at ImsService to get service records mapped
        // phone id.

        // Create ImsRegistrationImplBase object
        getRegistration();
        getRegistrationTracker();
     }

    /**
     * Broadcasts the IMS service up.
     */
    public void broadcastServiceUp() {
        Listener listener = null;

        synchronized (mLock) {
            if (mServiceUp) {
                log("Ims service is already up");
                return;
            }

            mServiceUp = true;

            listener = mListener;
        }

        logi("broadcastServiceUp :: phoneId=" + mContext.getPhoneId());

        if (listener != null) {
            listener.onServiceRecordStateChanged();
        }
    }

    /**
     * Broadcasts the IMS service down.
     */
    public void broadcastServiceDown() {
        Listener listener = null;

        synchronized (mLock) {
            if (!mServiceUp) {
                return;
            }

            mServiceUp = false;

            listener = mListener;
        }

        logi("broadcastServiceDown :: phoneId=" + mContext.getPhoneId());

        if (listener != null) {
            listener.onServiceRecordStateChanged();
        }
    }

    public ImsCallApp getCallApp() {
        synchronized (mLock) {
            return mCallApp;
        }
    }

    public ImsConfigImpl getConfig() {
        logi("getConfig :: slotid =" + mSlotId);

        synchronized (mLock) {
            if (mConfig == null) {
                mConfig = new ImsConfigImpl(mContext);
            }

            return mConfig;
        }
    }

    /**
     * Get the sip transport implementation
     * @return sip transport implementation object
     */
    public ImsSipTransport getSipTransport() {
        logi("getSipTransport :: slotid =" + mSlotId);

        synchronized (mLock) {
            if (mSipTransport == null) {
                 mSipTransport = ImsSipTransport.createImsSipTransport(mSlotId,
                        mContext.getContext(), mExecutor, getRegistration());
            }
            return mSipTransport;
        }
    }
    public ImsRegistrationImpl getRegistration() {
        logi("getRegistration :: slotid =" + mSlotId);
        synchronized (mLock) {
            if (mRegistration == null) {
                mRegistration = new ImsRegistrationImpl();
            }

            return mRegistration;
        }
    }

    public ImsRegistrationTracker getRegistrationTracker() {
        synchronized (mLock) {
            if (mRegTracker == null) {
                mRegTracker = new ImsRegistrationTracker(mContext, getRegistration(),
                        getConfig());
            }

            return mRegTracker;
        }
    }

    public boolean isServiceUp() {
        synchronized (mLock) {
            return mServiceUp;
        }
    }

    public void reconfigure() {
        synchronized (mLock) {
            if (mConfig != null) {
                logi("reconfigure");
                mConfig.clear();
                mConfig.init();
            }
        }
    }

    public void setCallApp(ImsCallApp callApp) {
        synchronized (mLock) {
            mCallApp = callApp;
        }
    }

    public void setListener(Listener listener) {
        synchronized (mLock) {
            mListener = listener;
        }
    }

    public void disableIms() {
        ImsServiceRegistry isr = ImsServiceRegistry.getInstance(mContext.getSlotId());
        isr.setImsEnabled(false);
    }

    public void enableIms() {
        ImsServiceRegistry isr = ImsServiceRegistry.getInstance(mContext.getSlotId());
        isr.setImsEnabled(true);
    }

    private static void log(String s) {
        ImsLog.d("[GII-IMPL] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[GII-IMPL] " + s);
    }
}
