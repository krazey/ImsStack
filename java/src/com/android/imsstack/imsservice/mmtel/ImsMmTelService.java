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

import android.os.Bundle;
import android.os.Message;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsExternalCallState;
import android.telephony.ims.MediaQualityStatus;
import android.telephony.ims.MediaThreshold;
import android.telephony.ims.SrvccCall;
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

import androidx.annotation.NonNull;

import com.android.imsstack.imsservice.base.ImsContext;
import com.android.imsstack.imsservice.mmtel.base.IMmTelCallListener;
import com.android.imsstack.imsservice.mmtel.base.IMmTelFeatureCapabilityListener;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.internal.imsservice.MmTelFeatureRegistry;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.IndentingPrintWriter;
import com.android.imsstack.util.LocalLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.List;
import java.util.Map;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Executor;
import java.util.function.Consumer;

/**
 * Implements MmTelFeature to provide VoLTE/VT/Emergency features.
 */
public class ImsMmTelService extends MmTelFeature
        implements ImsServiceRecord.Listener, ImsRegistrationTracker.CapabilityUpdateListener {
    private static final int LOG_SIZE = 50;
    private static final Map<Integer, String> STATE_TO_STRING = Map.of(
            ImsFeature.STATE_UNAVAILABLE, "UNAVAILABLE",
            ImsFeature.STATE_INITIALIZING, "INITIALIZING",
            ImsFeature.STATE_READY, "READY");

    private final ImsContext mImsContext;
    private final MmTelFeatureCapabilityListener mFeatureCapabilityListener
            = new MmTelFeatureCapabilityListener();
    protected final MmTelCallListener mCallListener = new MmTelCallListener();
    private boolean mReady = false;
    private CapabilityCallbackProxy mCapabilityCallback;
    private ImsRegistrationTracker mRegTracker;
    private final MmTelFeatureRegistry mMmTelFeatureRegistry;
    private final LocalLog mLocalLog = new LocalLog(LOG_SIZE);
    private final ImsServiceRegistry mServiceRegistry;

    public ImsMmTelService(ImsContext context) {
        this(context, ImsServiceRegistry.getInstance(context.getPhoneId()));
    }

    @VisibleForTesting
    public ImsMmTelService(ImsContext context, ImsServiceRegistry serviceRegistry) {
        super(context.getExecutor());
        mImsContext = context;
        mServiceRegistry = serviceRegistry;
        mMmTelFeatureRegistry = ImsServiceRegistry.getInstance(mImsContext.getSlotId())
                .getMmTelFeatureRegistry();

        initialize(mImsContext.getContext(), mImsContext.getSlotId());
        setFeatureState(ImsFeature.STATE_INITIALIZING);
    }

    public void start() {
        logi("ImsMmTelService starts - slotId=" + mImsContext.getSlotId());

        synchronized (this) {
            mReady = true;
        }

        ImsServiceRecord sr = ImsServiceManager.getServiceRecord(mImsContext.getPhoneId());

        if (sr != null) {
            sr.setListener(this);

            if (sr.isServiceUp()) {
                createCallApp();
                setFeatureState(ImsFeature.STATE_READY);
                mServiceRegistry.setMmTelFeature(this);
            }

            mRegTracker = sr.getRegistrationTracker();
            mRegTracker.setCapabilityUpdateListener(this);
        }
    }

    public void binderDied() {
        logi("ImsMmTelService :: binderDied - slotId=" + mImsContext.getSlotId());
        mLocalLog.log("binderDied");

        if (!isReady()) {
            // Do nothing
            return;
        }

        mServiceRegistry.setMmTelFeature(null);
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
        int phoneId = mImsContext.getPhoneId();
        logi("onServiceRecordStateChanged :: slotId=" + phoneId);

        ImsServiceRecord sr = ImsServiceManager.getServiceRecord(phoneId);

        if (sr != null) {
            int oldState = getFeatureState();
            int newState = sr.isServiceUp()
                    ? ImsFeature.STATE_READY : ImsFeature.STATE_UNAVAILABLE;

            if (newState != oldState) {
                if (newState == ImsFeature.STATE_READY) {
                    createCallApp();
                    mServiceRegistry.setMmTelFeature(this);
                } else if (newState == ImsFeature.STATE_UNAVAILABLE) {
                    mServiceRegistry.setMmTelFeature(null);
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
        ImsCallApp callApp = getCallApp();
        if (callApp != null) {
            callApp.getUtInterface().changeCapabilities(enabledCaps, disabledCaps);
        }
        mLocalLog.log("changeEnabledCapabilities: " + enabledCaps + ", " + disabledCaps);
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
        if (!isReady()) {
            log("Service not ready - getMultiEndpointInterface");
            return super.getMultiEndpoint();
        }

        ImsCallApp callApp = getCallApp();

        return (callApp != null) ? callApp.getMultiEndpointInterface() : null;
    }

    @Override
    public void setUiTtyMode(int mode, Message onCompleteMessage) {
        // onComplete: It can't be passed via Binder.
        // So, we don't send reply using Message.
        log("setUiTtyMode :: ttyMode=" + mode);

        mMmTelFeatureRegistry.setTtyMode(mode);

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
        mLocalLog.log("onFeatureRemoved");

        int phoneId = mImsContext.getPhoneId();
        ImsServiceManager sm = ImsServiceManager.getDefault();
        ImsCallApp callApp = sm.getCallApp(phoneId);
        mServiceRegistry.setMmTelFeature(null);

        if (callApp != null) {
            sm.destroyCallApp(phoneId);
            logi("MmTel - phoneId=" + phoneId + ", appCount=" + sm.getCallAppCount());
        }
    }

    @Override
    public void onFeatureReady() {
        logi("onFeatureReady");
        mLocalLog.log("onFeatureReady");

        ImsServiceManager sm = ImsServiceManager.getDefault();
        ImsCallApp callApp = sm.getCallApp(mImsContext.getPhoneId());

        if (!isReady() || callApp == null) {
            createCallApp();
        }

        mServiceRegistry.setMmTelFeature(this);
    }

    @Override
    public void setTerminalBasedCallWaitingStatus(boolean enabled) {
        mMmTelFeatureRegistry.setTerminalBasedCallWaitingStatus(enabled);
    }

    @Override
    public void notifySrvccStarted(Consumer<List<SrvccCall>> consumer) {
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_STARTED);

        List<SrvccCall> srvccCalls = getCallApp().getCallManager().getSrvccCalls();
        consumer.accept(srvccCalls);
    }

    @Override
    public void notifySrvccCompleted() {
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_COMPLETED);
    }

    @Override
    public void notifySrvccFailed() {
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_FAILED);
    }

    @Override
    public void notifySrvccCanceled() {
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_CANCELED);
    }

    @Override
    public void setMediaThreshold(@MediaQualityStatus.MediaSessionType int mediaSessionType,
            MediaThreshold mediaThreshold) {
        logi("setMediaThreshold=" + mediaThreshold);

        mServiceRegistry.getMmTelMediaRegistry()
                .setMediaThreshold(mediaSessionType, mediaThreshold);
    }

    @Override
    public void clearMediaThreshold(@MediaQualityStatus.MediaSessionType int mediaSessionType) {
        logi("clearMediaThreshold=" + mediaSessionType);
        mServiceRegistry.getMmTelMediaRegistry().setMediaThreshold(mediaSessionType, null);
    }

    /**
     * Dump this instance into a readable format for dumpsys usage.
     */
    public void dump(@NonNull IndentingPrintWriter pw) {
        pw.println("MmTelFeature:");
        pw.increaseIndent();

        pw.println("featureState=" + STATE_TO_STRING.get(getFeatureState()));

        // Local logs
        pw.println("Most recent logs:");
        pw.increaseIndent();
        mLocalLog.dump(pw);
        pw.decreaseIndent();

        pw.decreaseIndent();
    }

    @VisibleForTesting
    protected ImsCallApp createCallApp() {
        ImsServiceManager sm = ImsServiceManager.getDefault();
        return sm.createCallApp(mImsContext, mFeatureCapabilityListener, mCallListener);
    }

    @VisibleForTesting
    protected ImsCallApp getCallApp() {
        ImsServiceManager sm = ImsServiceManager.getDefault();
        return sm.getCallApp(mImsContext.getPhoneId());
    }

    private boolean isReady() {
        synchronized (this) {
            return mReady;
        }
    }

    private void postAndRunTask(Runnable task) {
        mImsContext.getExecutor().execute(task);
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

    private class MmTelFeatureCapabilityListener implements IMmTelFeatureCapabilityListener {
        @Override
        public void onFeatureCapabilityChanged(MmTelFeature.MmTelCapabilities capabilities) {
            log("MmTel :: onFeatureCapabilityChanged");

            postAndRunTask(() -> {
                try {
                    logi("MmTel :: phoneId=" + mImsContext.getPhoneId()
                            + ", " + capabilities.toString());
                    notifyCapabilitiesStatusChanged(capabilities);
                } catch (IllegalStateException e) {
                    loge("onFeatureCapabilityChanged Exception:" + e.toString());
                }
            });
        }
    }

    @VisibleForTesting
    protected class MmTelCallListener implements IMmTelCallListener {
        @Override
        public void onIncomingCallReceived(ImsCallSessionImplBase session) {
            ImsCallSessionImpl incomingSession = (ImsCallSessionImpl) session;

            if (incomingSession == null) {
                throw new IllegalArgumentException("ImsCallSessionImplBase is null");
            }

            ImsCallApp callApp = getCallApp();

            if (callApp == null) {
                throw new IllegalArgumentException("ImsCallApp is null");
            }

            callApp.takeCallSession(incomingSession);

            Bundle extras = new Bundle();
            if ("true".equals(session.getProperty(ImsCallProfile.EXTRA_USSD))) {
                extras.putBoolean(MmTelFeature.EXTRA_IS_USSD, true);
            }

            // `notifyIncomingCall` should be notified using the `default handler` instead of the
            // `call handler`. This is because, in the same flow, Telephony accesses
            // the IMS stack call object, and since the `call thread` is used, a deadlock occurs.
            // The result of `notifyIncomingCall` is processed via the `call thread`.
            Executor defaultExecutor = mImsContext.getDefaultHandler()::post;
            Executor callExecutor = incomingSession.getCallHandler()::post;

            CompletableFuture.supplyAsync(
                    () -> notifyIncomingCall(incomingSession, incomingSession.getCallId(), extras),
                    defaultExecutor)
                    .whenCompleteAsync((listener, exception) -> {
                        incomingSession.onIncomingcallNotified(
                                exception == null && listener != null);
                    }, callExecutor);
        }

        @Override
        public void onImsExternalCallStateChanged(List<ImsExternalCallState> imsExternalCallState) {
            ImsCallApp callApp = getCallApp();
            ImsMultiEndpointImpl multiEndpoint =
                    (callApp != null) ? callApp.getMultiEndpointInterface() : null;

            if (multiEndpoint != null) {
                multiEndpoint.updateDialogState(imsExternalCallState);
            }
        }
    }
}
