package com.android.imsstack.imsservice.mmtel;

import android.os.Bundle;
import android.os.Message;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.feature.CapabilityChangeRequest;
import android.telephony.ims.feature.CapabilityChangeRequest.CapabilityPair;
import android.telephony.ims.feature.ImsFeature;
import android.telephony.ims.feature.MmTelFeature;
import android.telephony.ims.stub.ImsCallSessionImplBase;
import android.telephony.ims.stub.ImsEcbmImplBase;
import android.telephony.ims.stub.ImsMultiEndpointImplBase;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.telephony.ims.stub.ImsSmsImplBase;
import android.telephony.ims.stub.ImsUtImplBase;

import com.android.ims.ImsManager;
import com.android.imsstack.enabler.IContext;
import com.android.imsstack.external.ims.ImsDialogState;
import com.android.imsstack.imsservice.mmtel.base.IMmTelCallListener;
import com.android.imsstack.imsservice.mmtel.base.IMmTelFeatureCapabilityListener;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsUtils;
import com.android.imsstack.util.IndentingPrintWriter;
import com.android.imsstack.util.LocalLog;

import java.util.List;

/**
 * Implements MmTelFeature to provide VoLTE/VT/Emergency features.
 */
public class ImsMmTelService extends MmTelFeature
        implements ImsServiceRecord.Listener, ImsRegistrationTracker.CapabilityUpdateListener {
    private static final int LOG_SIZE = 50;

    private final IContext mIContext;
    private final MmTelFeatureCapabilityListener mFeatureCapabilityListener
            = new MmTelFeatureCapabilityListener();
    private final MmTelCallListener mCallListener = new MmTelCallListener();
    private boolean mReady = false;
    private CapabilityCallbackProxy mCapabilityCallback;
    private ImsRegistrationTracker mRegTracker;
    private final LocalLog mLocalLog = new LocalLog(LOG_SIZE);

    public ImsMmTelService(IContext context) {
        mIContext = context;

        initialize(mIContext.getContext(), mIContext.getSlotId());

        setFeatureState(ImsFeature.STATE_INITIALIZING);
    }

    public void start() {
        logi("ImsMmTelService starts - slotId=" + mIContext.getSlotId());

        synchronized (this) {
            mReady = true;
        }

        ImsServiceRecord sr = ImsServiceManager.getServiceRecord(mIContext.getPhoneId());

        if (sr != null) {
            sr.setListener(this);

            if (sr.isServiceUp()) {
                createCallApp();
                setFeatureState(ImsFeature.STATE_READY);
            }

            mRegTracker = sr.getRegistrationTracker();
            mRegTracker.setCapabilityUpdateListener(this);
        }
    }

    public void binderDied() {
        logi("ImsMmTelService :: binderDied - slotId=" + mIContext.getSlotId());
        mLocalLog.log("binderDied");

        if (!isReady()) {
            // Do nothing
            return;
        }

        ImsCallApp callApp = getCallApp();

        if (callApp != null) {
            callApp.onBinderDied();
        }
    }

    @Override
    public void onCapabilitiesUpdateFailed(int capabilities, int networkType, int reason) {
        if (mCapabilityCallback != null) {
            mCapabilityCallback.onChangeCapabilityConfigurationError(
                    capabilities, networkType, reason);
        }
    }

    @Override
    public void onServiceRecordStateChanged() {
        logi("onServiceRecordStateChanged :: slotId=" + mIContext.getSlotId());

        ImsServiceRecord sr = ImsServiceManager.getServiceRecord(mIContext.getPhoneId());

        if (sr != null) {
            int oldState = getFeatureState();
            int newState = sr.isServiceUp() ?
                    ImsFeature.STATE_READY : ImsFeature.STATE_UNAVAILABLE;

            if (newState != oldState) {
                if (newState == ImsFeature.STATE_READY) {
                    createCallApp();
                }
            }

            setFeatureState(newState);
        }
    }

    // Start :: methods of MmTelFeature class
    @Override
    public boolean queryCapabilityConfiguration(@MmTelCapabilities.MmTelCapability int capability,
            @ImsRegistrationImplBase.ImsRegistrationTech int radioTech) {
        logi("queryCapabilityConfiguration :: capability=" + capability + ", radioTech=" + radioTech);

        if (radioTech == ImsRegistrationImplBase.REGISTRATION_TECH_NONE) {
            return false;
        }

        // FIXME: P-GII
        switch (capability) {
            case MmTelCapabilities.CAPABILITY_TYPE_VOICE:
                return (radioTech == ImsRegistrationImplBase.REGISTRATION_TECH_LTE) ?
                        true : false;
            case MmTelCapabilities.CAPABILITY_TYPE_VIDEO:
                return (radioTech == ImsRegistrationImplBase.REGISTRATION_TECH_LTE) ?
                        true : false;
            case MmTelCapabilities.CAPABILITY_TYPE_UT:
            case MmTelCapabilities.CAPABILITY_TYPE_SMS:
                return false;
            default:
                return false;
        }
    }

    @Override
    public void changeEnabledCapabilities(CapabilityChangeRequest request,
            CapabilityCallbackProxy c) {
        if (request == null || c == null) {
            loge("changeEnabledCapabilities :: Illegal arguments");
            return;
        }

        logi("changeEnabledCapabilities :: capabilities change request from framework");

        List<CapabilityPair> enabledCaps = request.getCapabilitiesToEnable();
        List<CapabilityPair> disabledCaps = request.getCapabilitiesToDisable();

        // Registration off -> voice / video / sms
        // Ut off -> SscServiceImpl
        mCapabilityCallback = c;
        mRegTracker.changeCapabilities(enabledCaps, disabledCaps);
        mLocalLog.log("changeEnabledCapabilities " + enabledCaps + ", " + disabledCaps);
    }

    @Override
    public ImsCallProfile createCallProfile(int serviceType, int callType) {
        if (!isReady()) {
            log("Service not ready - createCallProfile");
            return null;
        }

        ImsCallApp callApp = getCallApp();

        return (callApp != null) ? callApp.createCallProfile(serviceType, callType) : null;
    }

    @Override
    public ImsCallSessionImplBase createCallSession(ImsCallProfile profile) {
        if (!isReady()) {
            log("Service not ready - createCallSession");
            return null;
        }

        ImsCallApp callApp = getCallApp();

        return (callApp != null) ? callApp.createCallSession(profile) : null;
    }

    @Override
    public @ProcessCallResult int shouldProcessCall(String[] numbers) {
        // FIXME: P-GII
        return super.shouldProcessCall(numbers);
    }

    @Override
    public ImsUtImplBase getUt() {
        if (!isReady()) {
            log("Service not ready - getUtInterface");
            return null;
        }

        ImsCallApp callApp = getCallApp();

        return (callApp != null) ? callApp.getUtInterface() : null;
    }

    @Override
    public ImsEcbmImplBase getEcbm() {
        if (!isReady()) {
            log("Service not ready - getEcbmInterface");
            return null;
        }

        ImsCallApp callApp = getCallApp();

        return (callApp != null) ? callApp.getEcbmInterface() : null;
    }

    @Override
    public ImsMultiEndpointImplBase getMultiEndpoint() {
        logi("ImsMultiEndpoint is not supported yet for slot " + mIContext.getSlotId());
        return super.getMultiEndpoint();
    }

    @Override
    public void setUiTtyMode(int mode, Message onCompleteMessage) {
        // onComplete: It can't be passed via Binder.
        // So, we don't send reply using Message.
        log("setUiTtyMode :: ttyMode=" + mode);

        if (!isReady()) {
            log("Service not ready - setUiTtyMode");
            return;
        }

        ImsCallApp callApp = getCallApp();

        if (callApp != null) {
            callApp.setTtyMode(mode);
        }
    }

    // SMS over IMS interfaces
    @Override
    public ImsSmsImplBase getSmsImplementation() {
        if (!isReady()) {
            log("Service not ready - getSmsImplementation");
            return null;
        }
        ImsCallApp callApp = getCallApp();
        return (callApp != null) ? callApp.getSmsInterface() : null;
    }

    @Override
    public void onFeatureRemoved() {
        if (!isReady()) {
            log("Service not ready - onFeatureRemoved");
            return;
        }

        logi("onFeatureRemoved");

        ImsServiceManager sm = ImsServiceManager.getDefault();
        ImsCallApp callApp = sm.getCallApp(mIContext.getPhoneId());

        if (callApp != null) {
            sm.destroyCallApp(mIContext.getPhoneId());

            logi("MmTel - phoneId=" + mIContext.getPhoneId()
                    + ", appCount=" + sm.getCallAppCount());
        }
    }

    @Override
    public void onFeatureReady() {
        logi("onFeatureReady");

        postAndRunTask(() -> {
            logi("updateImsManager");
            ImsUtils.updateImsManager();
        });

        ImsServiceManager sm = ImsServiceManager.getDefault();
        ImsCallApp callApp = sm.getCallApp(mIContext.getPhoneId());

        if (!isReady() || callApp == null) {
            createCallApp();
        }

        // FIXME: P-GII
        // Update feature capabilities and IMS registration state
    }

    private ImsCallApp createCallApp() {
        ImsServiceManager sm = ImsServiceManager.getDefault();
        return sm.createCallApp(mIContext.getPhoneId(),
                mFeatureCapabilityListener, mCallListener);
    }

    private ImsCallApp getCallApp() {
        ImsServiceManager sm = ImsServiceManager.getDefault();
        return sm.getCallApp(mIContext.getPhoneId());
    }

    private boolean isReady() {
        synchronized (this) {
            return mReady;
        }
    }

    private void postAndRunTask(Runnable task) {
        mIContext.getExecutor().execute(task);
    }

    private static void log(String s) {
        ImsLog.d("[GII-IMPL] " + s);
    }

    private static void loge(String s) {
        ImsLog.e("[GII-IMPL] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[GII-IMPL] " + s);
    }

    private class MmTelFeatureCapabilityListener implements IMmTelFeatureCapabilityListener {
        @Override
        public void onFeatureCapabilityChanged(MmTelFeature.MmTelCapabilities capabilities) {
            log("MmTel :: onFeatureCapabilityChanged");

            postAndRunTask(() -> {
                try {
                    logi("MmTel :: phoneId=" + mIContext.getPhoneId()
                            + ", " + capabilities.toString());
                    notifyCapabilitiesStatusChanged(capabilities);
                } catch (IllegalStateException e) {
                    e.printStackTrace();
                }
            });
        }
    }

    private class MmTelCallListener implements IMmTelCallListener {
        @Override
        public void onIncomingCallReceived(ImsCallSessionImplBase session) {
            ImsCallSessionImpl incomingSession = (ImsCallSessionImpl)session;

            if (incomingSession == null) {
                throw new IllegalArgumentException("ImsCallSessionImplBase is null");
            }

            ImsCallApp callApp = getCallApp();

            if (callApp == null) {
                throw new IllegalArgumentException("ImsCallApp is null");
            }

            callApp.takeCallSession(incomingSession);

            Bundle extras = new Bundle();

            // EXTRA_USSD
            String isUSSD = incomingSession.getProperty(ImsCallProfile.EXTRA_USSD);
            if (isUSSD != null && isUSSD.equals("true")) {
                extras.putBoolean(MmTelFeature.EXTRA_IS_USSD, true);
            }

            // EXTRA_IS_UNKNOWN_CALL

            extras.putString(ImsManager.EXTRA_CALL_ID, incomingSession.getCallId());
            extras.putLong(ImsManager.EXTRA_PHONE_ID, mIContext.getPhoneId());

            // If any exception is thrown by this method call,
            // the incoming call is automatically rejected.
            notifyIncomingCall(incomingSession, extras);

            // Notify user alerting to native MTC logic if not USSD.
            if (isUSSD == null || isUSSD.equals("false")) {
                incomingSession.alertUser();
            }
        }

        @Override
        public void onImsDialogStateChanged(ImsDialogState dialogState) {
            ImsCallApp callApp = getCallApp();
            //FIXME: updateDialogState to multiendpoint
        }
    }

    /** Dump this instance into a readable format for dumpsys usage. */
    public void dump(IndentingPrintWriter pw) {
        pw.println("ImsMmTelService:");
        pw.increaseIndent();

        pw.println("slotId=" + mIContext.getSlotId());
        pw.println("featureState=" + getFeatureState());

        // Local logs
        pw.println("Most recent logs:");
        pw.increaseIndent();
        mLocalLog.dump(pw);
        pw.decreaseIndent();

        pw.decreaseIndent();
    }
}
