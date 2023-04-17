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

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.telephony.ims.ImsStreamMediaProfile;
import android.text.TextUtils;

import com.android.imsstack.core.CommonStarter;
import com.android.imsstack.core.ICommonPackageListener;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ISubscription;
import com.android.imsstack.core.agents.LocationInterface;
import com.android.imsstack.core.agents.NativeStateInterface;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.UsatInterface;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.enabler.mtc.CallFeature;
import com.android.imsstack.enabler.mtc.IECallStateTracker;
import com.android.imsstack.enabler.mtc.IServiceStateTracker;
import com.android.imsstack.enabler.mtc.MtcApp;
import com.android.imsstack.enabler.mtc.MtcServiceStateTracker;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.imsservice.mmtel.base.ICallLocationPolicy;
import com.android.imsstack.imsservice.mmtel.base.ISrvccStateTracker;
import com.android.imsstack.imsservice.mmtel.base.ImsApp;
import com.android.imsstack.imsservice.mmtel.base.TtyModeTracker;
import com.android.imsstack.imsservice.mmtel.internal.SrvccStateTracker;
import com.android.imsstack.imsservice.mmtel.internal.WfcSettingTracker;
import com.android.imsstack.imsservice.mmtel.videocall.base.VideoCallUtils;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsPrivateProperties;
import com.android.imsstack.util.MSimUtils;
import com.android.internal.annotations.VisibleForTesting;

import java.util.concurrent.Executor;

public class ImsCallContext implements ICallContext {
    private final Context mContext;
    private final Executor mExecutor;
    private final Handler mHandler;
    private final ImsApp mApp;

    private SrvccStateTracker mSrvccStateTracker;
    private TtyModeTracker mTtyModeTracker;
    private ImsCallLocationPolicy mCallLocationPolicy;

    private WfcSettingTracker mWfcSettingTracker;
    private MtcApp mMtcApp;
    private MtcServiceStateTracker mServiceStateTracker;

    public ImsCallContext(Context context, Executor executor, Looper looper, ImsApp app) {
        mContext = context;
        mExecutor = executor;
        mApp = app;
        mHandler = new Handler(looper);

        mWfcSettingTracker = new WfcSettingTracker(this);
        mServiceStateTracker = new MtcServiceStateTracker(this);

        mMtcApp = new MtcApp(this);
        mMtcApp.setServiceStateListener(mServiceStateTracker);
    }

    @VisibleForTesting
    public ImsCallContext(Context context, Executor executor, ImsApp app,
            WfcSettingTracker wfcsettingtracker, MtcServiceStateTracker stateTracker,
            MtcApp mtcApp, Looper looper) {
        mContext = context;
        mExecutor = executor;
        mApp = app;
        mWfcSettingTracker = wfcsettingtracker;
        mServiceStateTracker = stateTracker;
        mMtcApp = mtcApp;
        mMtcApp.setServiceStateListener(mServiceStateTracker);
        mHandler = new Handler(looper);
    }

    public void init() {
        mMtcApp.init();
        mMtcApp.setServiceStateListener(mServiceStateTracker);

        mWfcSettingTracker.init();
    }

    public void clear() {
        log("clear");

        mMtcApp.setServiceStateListener(null);
        mMtcApp.clear();
        mServiceStateTracker.clear();
        mWfcSettingTracker.clear();

        mCallLocationPolicy = null;

        if (mSrvccStateTracker != null) {
            mSrvccStateTracker.dispose();
            mSrvccStateTracker = null;
        }

        mHandler.removeCallbacksAndMessages(null);
    }

    public void dispose() {
        log("dispose");

        mWfcSettingTracker.dispose();
        mServiceStateTracker.dispose();

        mMtcApp.setServiceStateListener(null);
        mMtcApp.close();

        mTtyModeTracker = null;
        mCallLocationPolicy = null;

        if (mSrvccStateTracker != null) {
            mSrvccStateTracker.dispose();
            mSrvccStateTracker = null;
        }

        mHandler.removeCallbacksAndMessages(null);
    }

    @Override
    public Context getContext() {
        return mContext;
    }

    @Override
    public Executor getExecutor() {
        return mExecutor;
    }

    @Override
    public Handler getCallHandler() {
        return mHandler;
    }

    @Override
    public Looper getCallLooper() {
        return getLooper();
    }

    @Override
    public Handler getDefaultHandler() {
        return AppContext.getInstance().getMainHandler();
    }

    @Override
    public Looper getDefaultLooper() {
        return AppContext.getInstance().getMainLooper();
    }

    @Override
    public int getPhoneId() {
        return mApp.getPhoneId();
    }

    @Override
    public int getSlotId() {
        // FIXME: slot-id equals to phone-id
        return getPhoneId();
    }

    @Override
    public int getSubId() {
        ISubscription isub = getSubscription();
        return (isub != null) ? isub.getSubId(getSlotId()) : MSimUtils.getSubId(getPhoneId());
    }

    @Override
    public Looper getLooper() {
        return mHandler.getLooper();
    }

    @Override
    public IServiceStateTracker getServiceStateTracker() {
        return mServiceStateTracker;
    }

    @Override
    public IDcApn getDcApn() {
        return (IDcApn) DcFactory.getDc(DcFactory.APN, getSlotId());
    }

    @Override
    public IDcNetWatcher getDcNetWatcher() {
        return (IDcNetWatcher) DcFactory.getDc(DcFactory.NETWORK_WATCHER, getSlotId());
    }

    @Override
    public NativeStateInterface getNativeStateInterface() {
        return AgentFactory.getInstance().getAgent(NativeStateInterface.class, getSlotId());
    }

    @Override
    public ISubscription getSubscription() {
        return (ISubscription)AgentFactory.getAgent(AgentFactory.SUBSCRIPTION);
    }

    @Override
    public ISystem getSystem() {
        return SystemInterface.getInstance().getSystem(getSlotId());
    }

    @Override
    public LocationInterface getLocationInterface() {
        return AgentFactory.getInstance().getAgent(LocationInterface.class, getSlotId());
    }

    @Override
    public UsatInterface getUsatInterface() {
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, getSlotId());

        if (sim != null) {
            return sim.getUsatInterface();
        }

        return null;
    }

    @Override
    public boolean isCommonPackageReady() {
        return CommonStarter.getInstance().getState(getSlotId()) == CommonStarter.STATE_READY;
    }

    @Override
    public void addCommonPackageListener(ICommonPackageListener listener) {
        CommonStarter.getInstance().addListener(listener);
    }

    @Override
    public void removeCommonPackageListener(ICommonPackageListener listener) {
        CommonStarter.getInstance().removeListener(listener);
    }

    @Override
    public ImsApp getApp() {
        return mApp;
    }

    @Override
    public ICallLocationPolicy getCallLocationPolicy() {
        if (mCallLocationPolicy == null) {
            if (isLocationRequiredForCall()) {
                mCallLocationPolicy = new ImsCallLocationPolicy(this);
            }
        }

        return mCallLocationPolicy;
    }

    @Override
    public IECallStateTracker getECallStateTracker() {
        return mMtcApp.getCallManager().getECallStateTracker();
    }

    @Override
    public ISrvccStateTracker getSrvccStateTracker() {
        if (mSrvccStateTracker == null) {
            if (CallFeature.isSrvccSupported(getSlotId())) {
                mSrvccStateTracker = new SrvccStateTracker(this);
            }
        }

        return mSrvccStateTracker;
    }

    @Override
    public TtyModeTracker getTtyModeTracker() {
        if (mTtyModeTracker == null) {
            if (CallFeature.isTtySupported(getSlotId())) {
                // A preferred TTY mode will be set by the ImsPhoneCallTracker
                // when the ImsService is available.
                mTtyModeTracker = new TtyModeTracker();
            }
        }

        return mTtyModeTracker;
    }

    @Override
    public int getMediaCapabilities(int callType, int mediaType) {
        if (mediaType == MEDIA_AUDIO) {
            return getAudioCapabilities(callType);
        } else if (mediaType == MEDIA_VIDEO) {
            return getVideoCapabilities();
        }

        return 0;
    }

    @Override
    public Object getMtcCall(long callId) {
        return mMtcApp.getCallManager().getCall(callId);
    }

    @Override
    public boolean isLocationRequiredForCall() {
        return ImsCallLocationPolicy.isLocationRequired(mContext, getSlotId());
    }

    public MtcApp getMtcApp() {
        return mMtcApp;
    }

    public WfcSettingTracker getWfcSettingTracker() {
        return mWfcSettingTracker;
    }

    public boolean hasAccessBearerCapabilitiesForHDCall() {
        IDcNetWatcher dcnw = getDcNetWatcher();
        return ((dcnw != null) && (dcnw.is4G() || dcnw.is5G()))
                || ((mWfcSettingTracker != null) && mWfcSettingTracker.isWfcEnabled()
                        && mWfcSettingTracker.isWfcAvailable());
    }

    public int getAudioHDQuality() {
        if (CallFeature.isAudioEvsSupported(getSlotId())) {
            return ImsStreamMediaProfile.AUDIO_QUALITY_EVS_SWB;
        }

        return ImsStreamMediaProfile.AUDIO_QUALITY_AMR_WB;
    }

    public int getVideoHDQuality() {
        if (CallFeature.isVideoHevcSupported(getSlotId())) {
            String videoQuality = ImsPrivateProperties.Ephemeral.get(
                    ImsPrivateProperties.Ephemeral.KEY_H265_VIDEO_QUALITY, "720p", getSlotId());

            if (TextUtils.isEmpty(videoQuality)) {
                return ImsStreamMediaProfile.VIDEO_QUALITY_VGA_PORTRAIT;
            }

            // If various resolution is used, then change this logic later...
            return VideoCallUtils.VIDEO_QUALITY_HD_PORTRAIT;
        }

        return ImsStreamMediaProfile.VIDEO_QUALITY_VGA_PORTRAIT;
    }

    private int getAudioCapabilities(int callType) {
        if (ImsCallUtils.isVoiceCall(callType)) {
            return getAudioHDQuality();
        } else if (ImsCallUtils.isVideoCall(callType) && hasAccessBearerCapabilitiesForHDCall()) {
            return getAudioHDQuality();
        }

        return ImsStreamMediaProfile.AUDIO_QUALITY_AMR;
    }

    private int getVideoCapabilities() {
        if (hasAccessBearerCapabilitiesForHDCall()) {
            return getVideoHDQuality();
        }

        return ImsStreamMediaProfile.VIDEO_QUALITY_QVGA_PORTRAIT;
    }

    private static void log(String s) {
        ImsLog.d("[GII-IMPL] " + s);
    }
}
