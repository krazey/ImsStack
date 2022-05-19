package com.android.imsstack.imsservice.mmtel;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.telephony.ims.ImsStreamMediaProfile;
import android.text.TextUtils;

import com.android.imsstack.core.CommonStarter;
import com.android.imsstack.core.ICommonPackageListener;
import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.core.VoLteFactory;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.UsatInterface;
import com.android.imsstack.core.agents.agentif.ILocationAgent;
import com.android.imsstack.core.agents.agentif.ILocationAgentManager;
import com.android.imsstack.core.agents.agentif.ISharedState;
import com.android.imsstack.core.agents.agentif.ISubscription;
import com.android.imsstack.core.agents.dcm.DCFactory;
import com.android.imsstack.core.agents.dcmif.IDCApn;
import com.android.imsstack.core.agents.dcmif.IDCNetWatcher;
import com.android.imsstack.core.service.serviceif.IUSATService;
import com.android.imsstack.core.service.serviceif.IVoLteService;
import com.android.imsstack.enabler.mtc.CallFeature;
import com.android.imsstack.enabler.mtc.IECallStateTracker;
import com.android.imsstack.enabler.mtc.IServiceStateTracker;
import com.android.imsstack.enabler.mtc.MtcApp;
import com.android.imsstack.enabler.mtc.MtcServiceStateTracker;
import com.android.imsstack.external.ims.ImsFeatureProvider;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.imsservice.mmtel.base.ICallLocationPolicy;
import com.android.imsstack.imsservice.mmtel.base.IFDNTracker;
import com.android.imsstack.imsservice.mmtel.base.ISrvccStateTracker;
import com.android.imsstack.imsservice.mmtel.base.ImsApp;
import com.android.imsstack.imsservice.mmtel.base.TtyModeTracker;
import com.android.imsstack.imsservice.mmtel.internal.SrvccStateTracker;
import com.android.imsstack.imsservice.mmtel.internal.WfcSettingTracker;
import com.android.imsstack.imsservice.mmtel.videocall.base.VideoCallUtils;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.test.IImsTestMode;
import com.android.imsstack.test.ImsTestMode;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.FeatureUtils;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsPrivateProperties;
import com.android.imsstack.util.MSimUtils;
import com.android.imsstack.util.SettingsUtils;

import java.util.concurrent.Executor;

public class ImsCallContext implements ICallContext {
    private final Context mContext;
    private final Executor mExecutor;
    private final ImsApp mApp;

    private SrvccStateTracker mSrvccStateTracker;
    private TtyModeTracker mTtyModeTracker;
    private ImsCallLocationPolicy mCallLocationPolicy;

    private WfcSettingTracker mWfcSettingTracker;
    private ImsFDNTracker mFdnTracker;
    private MtcApp mMtcApp;
    private MtcServiceStateTracker mServiceStateTracker;

    public ImsCallContext(Context context, Executor executor, ImsApp app) {
        mContext = context;
        mExecutor = executor;
        mApp = app;

        CallFeature.init(getSlotId());

        mWfcSettingTracker = new WfcSettingTracker(this);
        mFdnTracker = new ImsFDNTracker(this);

        mServiceStateTracker = new MtcServiceStateTracker(this);

        mMtcApp = new MtcApp(this);
        mMtcApp.setServiceStateListener(mServiceStateTracker);
    }

    public void init() {
        int oldFeatures = CallFeature.getFeatures(getSlotId());

        CallFeature.init(getSlotId());

        int newFeatures = CallFeature.getFeatures(getSlotId());

        log("init :: oldFeatures=0x" + Integer.toHexString(oldFeatures)
                + ", newFeatures=0x" + Integer.toHexString(newFeatures));

        mMtcApp.init();
        mMtcApp.setServiceStateListener(mServiceStateTracker);

        mWfcSettingTracker.init();
        mFdnTracker.init();
    }

    public void clear() {
        log("clear");

        mMtcApp.setServiceStateListener(null);
        mMtcApp.clear();

        mServiceStateTracker.clear();

        mWfcSettingTracker.clear();
        mFdnTracker.clear();

        mCallLocationPolicy = null;

        if (mSrvccStateTracker != null) {
            mSrvccStateTracker.dispose();
            mSrvccStateTracker = null;
        }
    }

    public void dispose() {
        log("dispose");

        mWfcSettingTracker.dispose();
        mFdnTracker.dispose();
        mServiceStateTracker.dispose();

        mMtcApp.setServiceStateListener(null);
        mMtcApp.close();

        mTtyModeTracker = null;
        mCallLocationPolicy = null;

        if (mSrvccStateTracker != null) {
            mSrvccStateTracker.dispose();
            mSrvccStateTracker = null;
        }
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
        return ImsGlobal.getInstance().getCallHandler();
    }

    @Override
    public Looper getCallLooper() {
        return ImsGlobal.getInstance().getCallLooper();
    }

    @Override
    public Handler getDefaultHandler() {
        return AppContext.getMainHandler();
    }

    @Override
    public Looper getDefaultLooper() {
        return AppContext.getMainLooper();
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
    public IServiceStateTracker getServiceStateTracker() {
        return mServiceStateTracker;
    }

    @Override
    public IDCApn getDCApn() {
        return (IDCApn)DCFactory.getDC(DCFactory.APN, getSlotId());
    }

    @Override
    public IDCNetWatcher getDCNetWatcher() {
        return (IDCNetWatcher)DCFactory.getDC(DCFactory.NETWORK_WATCHER, getSlotId());
    }

    @Override
    public ISharedState getSharedState() {
        return (ISharedState)AgentFactory.getAgent(AgentFactory.SHARED_STATE, getSlotId());
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
    public IImsTestMode getTestMode() {
        return ImsTestMode.getInstance().getTestMode(getSlotId());
    }

    @Override
    public ILocationAgent getLocationAgent() {
        ILocationAgentManager lam = (ILocationAgentManager)VoLteFactory.getInstance().getAgent(
                VoLteFactory.AGENT_LOCATION_AGENT_MANAGER);

        if (lam != null) {
            return lam.getAgent(getSlotId());
        }

        return null;
    }

    @Override
    public IUSATService getUSATService() {
        IVoLteService vs = VoLteFactory.getInstance().getService(getSlotId());

        if (vs != null) {
            return (IUSATService)vs.getService(IVoLteService.TYPE_USAT);
        }

        return null;
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
    public IFDNTracker getFDNTracker() {
        return mFdnTracker;
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
                // Reads the settings
                mTtyModeTracker = new TtyModeTracker(
                        SettingsUtils.getTtyMode(mContext.getContentResolver()));
            }
        }

        return mTtyModeTracker;
    }

    @Override
    public int getMediaCapabilities(int callType, int mediaType) {
        if (mediaType == MEDIA_AUDIO) {
            return getAudioCapabilities(callType);
        } else if (mediaType == MEDIA_VIDEO) {
            return getVideoCapabilities(callType);
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
        IDCNetWatcher dcnw = getDCNetWatcher();
        IImsTestMode itm = getTestMode();
        // __TEST_MODE__ :: call over WiFi
        return ((dcnw != null) && (dcnw.is4G() || dcnw.is5G()))
                || ((mWfcSettingTracker != null) && mWfcSettingTracker.isWfcEnabled()
                        && mWfcSettingTracker.isWfcAvailable())
                || ((itm != null) && itm.isCallOverWifiEnabled());
    }

    public int getAudioHDQuality() {
        if (CallFeature.isAudioEvsSupported(getSlotId())) {
            if (ImsFeatureProvider.hasMediaEvsWb(mContext)) {
                return ImsStreamMediaProfile.AUDIO_QUALITY_EVS_WB;
            }

            return ImsStreamMediaProfile.AUDIO_QUALITY_EVS_SWB;
        }

        return ImsStreamMediaProfile.AUDIO_QUALITY_AMR_WB;
    }

    public int getVideoHDQuality() {
        if (FeatureUtils.isMediaHevcSupported(mContext)) {
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
        } else {
            IImsTestMode itm = getTestMode();
            if ((itm != null) && itm.isCallOverWifiEnabled()) {
                // __TEST_MODE__ :: call over WiFi
                return getAudioHDQuality();
            }
        }

        return ImsStreamMediaProfile.AUDIO_QUALITY_AMR;
    }

    private int getVideoCapabilities(int callType) {
        if (hasAccessBearerCapabilitiesForHDCall()) {
            return getVideoHDQuality();
        } else if (CallFeature.isVideoResolutionQcifSupported(getSlotId())) {
            return ImsStreamMediaProfile.VIDEO_QUALITY_QCIF;
        }

        return ImsStreamMediaProfile.VIDEO_QUALITY_QVGA_PORTRAIT;
    }

    private static void log(String s) {
        ImsLog.d("[GII-IMPL] " + s);
    }
}
