package com.android.imsstack.core.service;

import android.content.Context;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;

import com.android.imsstack.core.CapabilityConfigs;
import com.android.imsstack.core.OperatorInfo;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.IIMSPhoneGov;
import com.android.imsstack.core.agents.ImsPhoneProxyApi;
import com.android.imsstack.core.agents.agentif.IAlarmTimer;
import com.android.imsstack.core.agents.agentif.ICallInfoService;
import com.android.imsstack.core.agents.agentif.IIMSPhoneAgent;
import com.android.imsstack.core.agents.dcm.DCFactory;
import com.android.imsstack.core.agents.dcmif.ApnStateListener;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDCApn;
import com.android.imsstack.core.service.serviceif.IService;
import com.android.imsstack.core.service.serviceif.IVoLteService;
import com.android.imsstack.enabler.mtc.CallReasonInfo;
import com.android.imsstack.enabler.mtc.IUMtcCall;
import com.android.imsstack.system.IJNIUpCallEvt;
import com.android.imsstack.system.ImsEventDef;
import com.android.imsstack.system.JNIUpCallEvtManager;
import com.android.imsstack.util.ImsExtApi;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsProperties;

import java.util.ArrayList;

public class CallInfoService implements IService, ICallInfoService {
    /* RIL constants */
    private static final int RIL_CALL_TOTAL_STATUS_IDLE = 0;
    private static final int RIL_CALL_TOTAL_STATUS_ACTIVE = 1;
    private static final int RIL_CALL_STATUS_IDLE = 0;
    private static final int RIL_CALL_STATUS_RINGING = 1;
    private static final int RIL_CALL_STATUS_OFFHOOK = 2;
    private static final int RIL_CALL_REASON_NORMAL = 0;
    private static final int RIL_CALL_REASON_CS_REDIAL = 1;
    private static final int RIL_CALL_TYPE_VOICE = 0;
    private static final int RIL_CALL_TYPE_VIDEO = 1;
    private static final int RIL_CALL_SYSTEMMODE_LTE = 0;
    private static final int RIL_CALL_SYSTEMMODE_WIFI = 1;
    private static final int RIL_CALL_DIR_MO = 0;
    private static final int RIL_CALL_DIR_MT = 1;
    private static final int RIL_CALL_ETYPE_NORMAL = 0;
    private static final int RIL_CALL_ETYPE_EMERGENCY = 1;
    /** RIL SCM constants */
    private static final int RIL_SCM_TYPE_NONE = 0;
    private static final int RIL_SCM_TYPE_VOICE = 1;
    private static final int RIL_SCM_TYPE_VIDEO = 2;
    private static final int RIL_SCM_MODE_END = 0;
    private static final int RIL_SCM_MODE_START = 1;
    /** Media call type (refer to MediaDef.h) */
    private static final int CALL_MEDIA_TYPE_AUDIO = 0x00000001;
    private static final int CALL_MEDIA_TYPE_VIDEO = 0x00000002;
    private static final int CALL_MEDIA_TYPE_TEXT = 0x00000004;
    /** Media codec type (refer to AudioDef.h) */
    private static final int CALL_MEDIA_CODEC_TYPE_NONE = 0;
    private static final int CALL_MEDIA_CODEC_TYPE_AMR = 1;
    private static final int CALL_MEDIA_CODEC_TYPE_AMRWB = 2;
    private static final int CALL_MEDIA_CODEC_TYPE_EVS = 3;
    private static final int CALL_MEDIA_CODEC_TYPE_G711_PCMU = 4;
    private static final int CALL_MEDIA_CODEC_TYPE_G711_PCMA = 5;
    private static final int CALL_MEDIA_CODEC_TYPE_EVS_NB = 6;
    private static final int CALL_MEDIA_CODEC_TYPE_EVS_WB = 7;
    private static final int CALL_MEDIA_CODEC_TYPE_EVS_SWB = 8;
    private static final int CALL_MEDIA_CODEC_TYPE_EVS_FB = 9;
    /** Media codec bandwidth (refer to MediaDef.h) */
    private static final int AUDIO_CODEC_BANDWIDTH_INVALID = -1;
    private static final int AUDIO_CODEC_BANDWIDTH_NB = 0;
    private static final int AUDIO_CODEC_BANDWIDTH_WB = 1;
    private static final int AUDIO_CODEC_BANDWIDTH_SWB = 2;
    private static final int AUDIO_CODEC_BANDWIDTH_FB = 3;
    /** Media codec bitrate (refer to AudioNego.cpp) */
    private static final String[] AUDIO_CODEC_BITRATE_STRING = {
        "4.75", "5.15", "5.90", "6.70", "7.40", "7.95", "10.20", "12.20", "0", "0",
        "6.60", "8.85", "12.65", "14.25", "15.85", "18.25", "19.85", "23.05", "23.85", "0",
        "5.90", "7.20", "8.00", "9.60", "13.20", "16.40", "24.40", "32.00", "48.00", "64.00",
        "96.00", "128.00", "0", "0", "0", "13.20", "13.20", "13.20", "13.20", "0",
        "6.60", "8.85", "12.65", "14.25", "15.85", "18.25", "19.85", "23.05", "23.85", "0"
    };

    private static final int INDEX_CALL_INFO_WHOLESTATE = 0;
    private static final int INDEX_CALL_INFO_ID = 1;
    private static final int INDEX_CALL_INFO_STATE = 2;
    private static final int INDEX_CALL_INFO_REASON = 3;
    private static final int INDEX_CALL_INFO_TYPE = 4;
    private static final int INDEX_CALL_INFO_SLOTID = 5;
    private static final int INDEX_CALL_INFO_SYSMODE = 6;
    private static final int INDEX_CALL_INFO_DIRECTION = 7;
    private static final int INDEX_CALL_INFO_TOTALSTATE = 8;
    private static final int INDEX_CALL_INFO_EMERGENCYTYPE = 9;

    private static final int EVENT_TIMER_ENABLE_SA = 1;
    private static final int EVENT_TIMER_SEND_CALL_IDLE = 2;

    private static final int NR_SA_DISABLED = 1;
    private static final int NR_SA_ENABLED = 0;
    private static final int MAX_DELAY_FOR_ENABLE_SA_ON_IDLE = 2000;
    private static final int MAX_DELAY_FOR_CALL_IDLE = 1000;

    private IVoLteService mVoLteService = null;
    private Handler mCallInfoHandler = null;
    private ImsPhoneProxyApi mIPPApi;
    private ArrayList<CallInfo> mCallInfo = new ArrayList<CallInfo>();
    private int[] mCallInfoForRIL = new int[10];
    public int mCallInfoIDIndex = 0;
    public int mTotalCallState = IUMtcCall.VOLTE_CALL_STATE_IDLE;
    public int mTotalOldCallState = IUMtcCall.VOLTE_CALL_STATE_IDLE;
    private ImsExtApi.TouchService mTouchService = null;
    private boolean mAvailableForNrSaMode = false;
    private int mNrSaMode = NR_SA_ENABLED;
    private CallInfoApnStateListener mApnStateListener = null;
    private int mEnableSaTimer = 0;
    private int mSendCallIdleTimer = 0;
    private int[] mCallInfoForIdle = null;

    public CallInfoService() {
    }

    @Override
    public boolean start(IVoLteService voLteService) {
        mVoLteService = voLteService;

        ImsLog.i(getSlotId(), "");

        if (isRilCallStatusRequired()) {
            mAvailableForNrSaMode = CapabilityConfigs.isVoNrEnabled(getSlotId());

            setCallInfoForRIL(0, 0, 0, 0, 0, 0, 0, 0, 0);

            IIMSPhoneAgent ip = IIMSPhoneGov.getInstance(getSlotId());
            if (ip != null) {
                ip.setImsCallStatus(mCallInfoForRIL[INDEX_CALL_INFO_WHOLESTATE],
                        mCallInfoForRIL[INDEX_CALL_INFO_ID],
                        mCallInfoForRIL[INDEX_CALL_INFO_STATE],
                        mCallInfoForRIL[INDEX_CALL_INFO_REASON],
                        mCallInfoForRIL[INDEX_CALL_INFO_TYPE],
                        mCallInfoForRIL[INDEX_CALL_INFO_SYSMODE],
                        mCallInfoForRIL[INDEX_CALL_INFO_DIRECTION],
                        mCallInfoForRIL[INDEX_CALL_INFO_TOTALSTATE],
                        mCallInfoForRIL[INDEX_CALL_INFO_EMERGENCYTYPE]);
            }

            if (mAvailableForNrSaMode) {
                sendNrSaMode(NR_SA_ENABLED);
            }

            setApnStateListener();
        }

        mCallInfoHandler = new CallInfoHandler();

        if (isSCMRequired()) {
            mIPPApi = new ImsPhoneProxyApi(getSlotId());
        }

        IJNIUpCallEvt jniEvt = JNIUpCallEvtManager.getInstance().getJNIUpCallEvt(getSlotId());

        if (jniEvt != null) {
            jniEvt.setInterfaceForCallInfoService(this);
            jniEvt.registerForCallMediaInfo(mCallInfoHandler,
                    ImsEventDef.IMS_EVENT_CALL_MEIDA_INFO, null);
        }

        mTouchService = new ImsExtApi.TouchService();
        mTouchService.connect();

        return true;
    }

    @Override
    public void cleanup(Context context) {
        ImsLog.i(getSlotId(), "");

        clearApnStateListener();
        stopEnableSaTimer();
        setNrSaMode(NR_SA_ENABLED);
        stopSendCallIdleTimer();
        sendDelayedCallIdle();

        if (mCallInfoHandler != null) {
            IJNIUpCallEvt jniEvt = JNIUpCallEvtManager.getInstance().getJNIUpCallEvt(getSlotId());

            if (jniEvt != null) {
                jniEvt.unregisterForCallMediaInfo(mCallInfoHandler);
                jniEvt.setInterfaceForCallInfoService(null);
            }

            mCallInfoHandler.removeCallbacksAndMessages(null);
            mCallInfoHandler = null;
        }

        mCallInfo.clear();

        for (int i = 0; i < mCallInfoForRIL.length; i++) {
            mCallInfoForRIL[i] = 0;
        }

        mCallInfoIDIndex = 0;
        mTotalCallState = IUMtcCall.VOLTE_CALL_STATE_IDLE;
        mTotalOldCallState = IUMtcCall.VOLTE_CALL_STATE_IDLE;

        if (mTouchService != null) {
            mTouchService.disconnect();
            mTouchService = null;
        }
    }

    @Override
    public void update(Context context) {
    }

    @Override
    public void setCallState(int key, int param) {
        ImsLog.i(getSlotId(), "");
        handleCallInfo(key, param);
    }

    private boolean isRilCallStatusRequired() {
        if (ImsProperties.isChipVendorMtk()) {
            return true;
        }

        return false;
    }

    private boolean isSCMRequired() {

        if (!ImsProperties.isChipVendorMtk()) {
            return false;
        }
        String op = OperatorInfo.getOperator(getSlotId());
        String co = OperatorInfo.getCountry(getSlotId());

        if (OperatorInfo.equalsOperator("KDDI", op)
                || OperatorInfo.equalsOperator("DCM", op)
                || OperatorInfo.equalsOperator("VZW", op)
                || OperatorInfo.equalsOperator("ATT", op)
                || OperatorInfo.equalsCountry("KR", co)
                || OperatorInfo.isGroupTMUS(getSlotId(), op, co)
                || OperatorInfo.isEnablerTypeGlobal(getSlotId())) {
            return true;
        }

        return false;
    }

    private boolean isSCMBlockedForWFC() {
        String op = OperatorInfo.getOperator(getSlotId());
        String co = OperatorInfo.getCountry(getSlotId());

        if (OperatorInfo.isGroupTMUS(getSlotId(), op, co)
                || OperatorInfo.equalsOperator("ATT", op)) {
            return true;
        }

        return false;
    }

    private void handleCallInfo(int wParam, int lParam) {
        int sessionKey = updateCallInfo(wParam, lParam);

        if (sessionKey != 0) {
            setNrSaModeOnCallStateChanged();
            sendSCMToRIL(sessionKey);
            sendToRIL(sessionKey);
            setVoNRStatus(sessionKey);
            setCallStateForTouch(sessionKey);

            deleteCallInfo(sessionKey);
        }
    }

    private void handleCallMediaInfo(int wParam, int lParam) {
        int sessionKey = (wParam & 0xffffffff);
        int mediaType = (lParam & 0x0000000f);
        int codecType = ((lParam & 0x000000f0) >> 4);
        int codecBandWidth = ((lParam & 0x00000f00) >> 8);
        int codecBitrate = ((lParam & 0x00ff0000) >> 16);

        ImsLog.d(getSlotId(), "sessionKey=" + sessionKey + ", mediaType=" + mediaType);

        if (sessionKey == 0) {
            return;
        }

        switch (mediaType) {
            case CALL_MEDIA_TYPE_AUDIO:
                handleAudioMediaInfo(lParam);
                break;

            case CALL_MEDIA_TYPE_VIDEO:
                break;

            case CALL_MEDIA_TYPE_TEXT:
                break;

            default:
                break;
        }
    }

    private int updateCallInfo(int key, int param) {
        int sessionKey = 0;
        int callState = IUMtcCall.VOLTE_CALL_STATE_IDLE;
        int callStateReason = CallReasonInfo.CODE_NONE;
        int callType = IUMtcCall.VOLTE_CALL_TYPE_NORMAL;
        int callSessionType = IUMtcCall.CALLTYPE_VOIP;

        sessionKey = (key & 0xffffffff);
        callState = ((param & 0x0000000f));
        callStateReason = ((param & 0x0000fff0) >> 4);
        callType = ((param & 0x000f0000) >> 16);
        callSessionType = ((param & 0x00f00000) >> 20);

        ImsLog.i(getSlotId(), "sessionKey=" + sessionKey + ", state=" + callState
                + ", reason=" + callStateReason + ", callType=" + callType
                + ", callSessionType=" + callSessionType);

        CallInfo callInfo = getCallInfo(sessionKey);
        if (callInfo != null) {
            callInfo.update(callState, callStateReason, callType, callSessionType);
        } else {
            addCallInfo(sessionKey, callState, callStateReason, callType, callSessionType);
        }

        int nCallNum = mCallInfo.size();

        mTotalOldCallState = mTotalCallState;
        if (nCallNum > 1) {
            mTotalCallState = IUMtcCall.VOLTE_CALL_STATE_OFFHOOK;
        } else {
            mTotalCallState = callState;
        }

        ImsLog.i(getSlotId(), "TotalNum=" + nCallNum + ", TotalOldState=" + mTotalOldCallState
                + ", TotalState=" + mTotalCallState);

        return sessionKey;
    }

    private void sendToRIL(int sessionKey) {
        if (isRilCallStatusRequired() == false) {
            return;
        }

        int ril_TotalState = RIL_CALL_TOTAL_STATUS_IDLE;
        int oldState = RIL_CALL_STATUS_IDLE;
        int ril_State = RIL_CALL_STATUS_IDLE;
        int ril_Reason = RIL_CALL_REASON_NORMAL;
        int ril_Type = RIL_CALL_TYPE_VOICE;
        int ril_SystemMode = RIL_CALL_SYSTEMMODE_LTE;
        int ril_ID = 0;
        int ril_emergencyType = RIL_CALL_ETYPE_NORMAL;

        CallInfo callInfo = getCallInfo(sessionKey);

        if (callInfo == null) {
            return;
        }

        if (mTotalCallState != IUMtcCall.VOLTE_CALL_STATE_IDLE
                && mTotalCallState != IUMtcCall.VOLTE_CALL_STATE_TERMINATING) {
            ril_TotalState = RIL_CALL_TOTAL_STATUS_ACTIVE;
        }

        ImsLog.i(getSlotId(), "Total=" + mTotalCallState + ", state=" + callInfo.mState);

        switch (callInfo.mState) {
            case IUMtcCall.VOLTE_CALL_STATE_IDLE:
                return;

            case IUMtcCall.VOLTE_CALL_STATE_RINGING:
                ril_State = RIL_CALL_STATUS_RINGING;
                break;

            case IUMtcCall.VOLTE_CALL_STATE_OFFHOOK:
                ril_State = RIL_CALL_STATUS_OFFHOOK;
                break;

            case IUMtcCall.VOLTE_CALL_STATE_RINGBACK:
                ril_State = RIL_CALL_STATUS_OFFHOOK;
                break;

            case IUMtcCall.VOLTE_CALL_STATE_ALERTING:
                return;

            case IUMtcCall.VOLTE_CALL_STATE_TERMINATING:
                ril_State = RIL_CALL_STATUS_IDLE;
                break;

            default:
                return;
        }

        switch (callInfo.mType) {
            case IUMtcCall.VOLTE_CALL_TYPE_NORMAL:
            case IUMtcCall.VOLTE_CALL_TYPE_EMERGENCY:
            case IUMtcCall.VOLTE_CALL_TYPE_OFFLINE_REG_RECOVERY:
            case IUMtcCall.VOLTE_CALL_TYPE_OFFLINE_REG_REGRESSION:
            case IUMtcCall.VOLTE_CALL_TYPE_USSI:
                ril_SystemMode = RIL_CALL_SYSTEMMODE_LTE;
                break;
            case IUMtcCall.VOLTE_CALL_TYPE_NORMAL_WIFI:
            case IUMtcCall.VOLTE_CALL_TYPE_EMERGENCY_WIFI:
                ril_SystemMode = RIL_CALL_SYSTEMMODE_WIFI;
                break;
            default:
                ril_SystemMode = RIL_CALL_SYSTEMMODE_LTE;
                break;
        }

        switch (callInfo.mSessionType) {
            case IUMtcCall.CALLTYPE_VOIP: // FALL-THROUGH
            case IUMtcCall.CALLTYPE_RTT:
                ril_Type = RIL_CALL_TYPE_VOICE;
                break;
            case IUMtcCall.CALLTYPE_VT: // FALL-THROUGH
            case IUMtcCall.CALLTYPE_VIDEO_RTT:
                ril_Type = RIL_CALL_TYPE_VIDEO;
                break;
            default:
                ril_Type = RIL_CALL_TYPE_VOICE;
                break;
        }

        if (callInfo.mType == IUMtcCall.VOLTE_CALL_TYPE_EMERGENCY
                || callInfo.mType == IUMtcCall.VOLTE_CALL_TYPE_EMERGENCY_WIFI) {
            ril_emergencyType = RIL_CALL_ETYPE_EMERGENCY;
        }

        // TODO : need to modify this after emergency domain selection policy is decided.
        if (callInfo.mStateReason == CallReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED
            /*callInfo.mStateReason == IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RETRY
                || callInfo.mStateReason == IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RETRY1X
                || callInfo.mStateReason == IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RETRY_RAT
                || callInfo.mStateReason ==
                        IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RETRY_SILENT*/) {
            ril_Reason = RIL_CALL_REASON_CS_REDIAL;
        }

        ril_ID = callInfo.mCallInfoID;

        int totalCallStateForRil = getTotalStateForRil();
        ImsLog.i(getSlotId(), "totalState=" + ril_TotalState + ", state=" + ril_State
                + ", reason=" + ril_Reason + ", type=" + ril_Type
                + ", totalCallStateForRil=" + totalCallStateForRil
                + ", emergencyType=" + ril_emergencyType);

        setCallInfoForRIL(ril_TotalState, ril_ID, ril_State, ril_Reason, ril_Type,
                ril_SystemMode, callInfo.mDir, totalCallStateForRil, ril_emergencyType);

        stopSendCallIdleTimer();

        if (mNrSaMode == NR_SA_DISABLED
                && ril_TotalState == RIL_CALL_TOTAL_STATUS_IDLE) {
            synchronized (this) {
                if (mCallInfoForIdle == null) {
                    mCallInfoForIdle = new int[mCallInfoForRIL.length];
                }

                System.arraycopy(mCallInfoForRIL, 0,
                        mCallInfoForIdle, 0, mCallInfoForRIL.length);
            }
            return;
        }

        boolean sendCallIdle = false;

        synchronized (this) {
            // index-4: call-type, index-7: call-direction
            if (mCallInfoForIdle != null) {
                if ((mCallInfoForIdle[INDEX_CALL_INFO_TYPE]
                        != mCallInfoForRIL[INDEX_CALL_INFO_TYPE])
                    || (mCallInfoForIdle[INDEX_CALL_INFO_DIRECTION]
                        != mCallInfoForRIL[INDEX_CALL_INFO_DIRECTION])) {
                    sendCallIdle = true;
                } else {
                    mCallInfoForIdle = null;
                }
            }
        }

        IIMSPhoneAgent ip = IIMSPhoneGov.getInstance(getSlotId());

        if (ip != null) {
            ip.setImsCallStatus(mCallInfoForRIL[INDEX_CALL_INFO_WHOLESTATE],
                    mCallInfoForRIL[INDEX_CALL_INFO_ID],
                    mCallInfoForRIL[INDEX_CALL_INFO_STATE],
                    mCallInfoForRIL[INDEX_CALL_INFO_REASON],
                    mCallInfoForRIL[INDEX_CALL_INFO_TYPE],
                    mCallInfoForRIL[INDEX_CALL_INFO_SYSMODE],
                    mCallInfoForRIL[INDEX_CALL_INFO_DIRECTION],
                    mCallInfoForRIL[INDEX_CALL_INFO_TOTALSTATE],
                    mCallInfoForRIL[INDEX_CALL_INFO_EMERGENCYTYPE]);
        }

        if (sendCallIdle) {
            if (mCallInfoHandler != null) {
                mCallInfoHandler.postDelayed(() -> {
                    sendDelayedCallIdle();
                }, 100);
            } else {
                android.os.SystemClock.sleep(100);
                sendDelayedCallIdle();
            }
        }
    }

    private int getTotalStateForRil() {

        // 0:END / 1:START / 2:CONNECTED
        if (mTotalCallState == IUMtcCall.VOLTE_CALL_STATE_IDLE
                || mTotalCallState == IUMtcCall.VOLTE_CALL_STATE_TERMINATING) {
            return 0;
        } else if (mTotalCallState == IUMtcCall.VOLTE_CALL_STATE_OFFHOOK) {
            return 2;
        }

        return 1;
    }

    private void setVoNRStatus(int sessionKey) {

        if (isVoNRStatusUpdateRequired() == false) {
            return;
        }

        CallInfo callInfo = getCallInfo(sessionKey);
        if (callInfo == null) {
            return;
        }

        boolean start = false;
        switch (callInfo.mState) {
            case IUMtcCall.VOLTE_CALL_STATE_IDLE:
                // start = false
                break;
            case IUMtcCall.VOLTE_CALL_STATE_RINGBACK:
            case IUMtcCall.VOLTE_CALL_STATE_RINGING:
                start = true;
                break;
            case IUMtcCall.VOLTE_CALL_STATE_TERMINATING:
            case IUMtcCall.VOLTE_CALL_STATE_ALERTING:
            case IUMtcCall.VOLTE_CALL_STATE_OFFHOOK:
            default:
                // ignore
                return;
        }

        ImsLog.d(getSlotId(), "setVoNRStatus start=" + start);

        if (mCallInfo.size() > 1) {
            int otherCallType = RIL_CALL_ETYPE_NORMAL;
            for (int i = 0; i < mCallInfo.size(); i++) {
                CallInfo temp = mCallInfo.get(i);
                if (temp.mSessionKey == sessionKey) {
                    continue;
                }

                if (temp.mType == IUMtcCall.VOLTE_CALL_TYPE_EMERGENCY
                        || temp.mType == IUMtcCall.VOLTE_CALL_TYPE_EMERGENCY_WIFI) {
                    otherCallType = RIL_CALL_ETYPE_EMERGENCY;
                }
            }

            if (mCallInfoForRIL[INDEX_CALL_INFO_EMERGENCYTYPE] != otherCallType) {
                // new emergency call during normal call
                setVoiceDomainStatus(null, start);
            }
        } else {
            setVoiceDomainStatus(null, start);
            setImsVoiceDomainStatus(null, start);
        }
    }

    private boolean isVoNRStatusUpdateRequired() {

        if (CapabilityConfigs.isVoNrEnabled(getSlotId())) {
            // In VoNR case, native UC sets the information
            return false;
        }

        if (ImsProperties.System.get("ro.vendor.mtk_ps1_rat", "").contains("N") == false) {
            // only N70 chipset requires this information even in NSA models.
            return false;
        }

        return true;
    }

    private void setVoiceDomainStatus(Object vonr, boolean start) {
        boolean bEmergency
                = mCallInfoForRIL[INDEX_CALL_INFO_EMERGENCYTYPE] == RIL_CALL_ETYPE_EMERGENCY;
        ImsLog.d(getSlotId(), "setVoiceDomainStatus start=" + start + " emergency=" + bEmergency);
        //vonr.setVoice(start, bEmergency);
    }

    private void setImsVoiceDomainStatus(Object vonr, boolean start) {
        // sysmode must NOT be NR because NSA device.
        // AndroidVoNR.h : MTK_RADIO_IF_LTE = 0 / MTK_RADIO_IF_WLAN = 1 / MTK_RADIO_IF_NR5G = 5
        int sysMode = 0;
        if (mCallInfoForRIL[INDEX_CALL_INFO_SYSMODE] == RIL_CALL_SYSTEMMODE_WIFI) {
            sysMode = 1;
        }
        ImsLog.d(getSlotId(), "setImsVoiceDomainStatus start=" + start + " sysMode=" + sysMode);
        //vonr.setImsVoice(start, sysMode);
    }

    private void sendSCMToRIL(int sessionKey) {
        if (!isSCMRequired()) {
            return;
        }

        int type = RIL_SCM_TYPE_VOICE;
        int mode = RIL_SCM_MODE_END;
        int emergencyType = RIL_CALL_ETYPE_NORMAL;

        IIMSPhoneAgent ip = IIMSPhoneGov.getInstance(getSlotId());

        if (ip == null) {
            return;
        }

        CallInfo callInfo = getCallInfo(sessionKey);

        if (callInfo == null) {
            return;
        }

        if (isSCMBlockedForWFC()
                && (callInfo.mType == IUMtcCall.VOLTE_CALL_TYPE_NORMAL_WIFI
                        || callInfo.mType == IUMtcCall.VOLTE_CALL_TYPE_EMERGENCY_WIFI)) {
            return;
        }

        if (callInfo.mType == IUMtcCall.VOLTE_CALL_TYPE_EMERGENCY
                || callInfo.mType == IUMtcCall.VOLTE_CALL_TYPE_EMERGENCY_WIFI) {
            emergencyType = RIL_CALL_ETYPE_EMERGENCY;
        }

        if (callInfo.mState == IUMtcCall.VOLTE_CALL_STATE_RINGBACK) {
            int size = mCallInfo.size();

            for (int i = 0; i < size; ++i) {
                CallInfo refCallInfo = mCallInfo.get(i);

                if (refCallInfo == null) {
                    continue;
                }

                if (refCallInfo.mSessionKey == sessionKey) {
                    continue;
                } else {
                    if (refCallInfo.mSessionType == callInfo.mSessionType) {
                        return;
                    }
                }
            }

            switch (callInfo.mSessionType) {
                case IUMtcCall.CALLTYPE_VOIP: // FALL-THROUGH
                case IUMtcCall.CALLTYPE_RTT:
                    type = RIL_SCM_TYPE_VOICE;
                    break;

                case IUMtcCall.CALLTYPE_VT: // FALL-THROUGH
                case IUMtcCall.CALLTYPE_VIDEO_RTT:
                    type = RIL_SCM_TYPE_VIDEO;
                    break;

                default:
                    type = RIL_SCM_TYPE_VOICE;
                    break;
            }

            callInfo.setSCMType(type);

            if (emergencyType == RIL_CALL_ETYPE_EMERGENCY) {
                ip.setScmMode(type, RIL_SCM_MODE_START, emergencyType);
            } else {
                mIPPApi.setScmMode(type, RIL_SCM_MODE_START, emergencyType);
            }
        }

        if (callInfo.mState == IUMtcCall.VOLTE_CALL_STATE_IDLE) {
            if (callInfo.mSCMType > RIL_SCM_TYPE_NONE) {
                ip.setScmMode(type, RIL_SCM_MODE_END, emergencyType);
            }
        }
    }

    private void setCallStateForTouch(int sessionKey) {
        if (mTouchService == null) {
            return;
        }

        CallInfo callInfo = getCallInfo(sessionKey);

        if (callInfo == null) {
            return;
        }

        int callStateForTouch = convertCallStateForTouch(mTotalCallState);

        if (convertCallStateForTouch(mTotalOldCallState) == callStateForTouch) {
            ImsLog.d(getSlotId(), "callStateForTouch: " + callStateForTouch + " - ignored");
            return;
        }

        ImsLog.d(getSlotId(), "callStateForTouch: " + callStateForTouch);
        mTouchService.setCallState(callStateForTouch);
    }

    private void setCallInfoForRIL(int wholeState, int id, int state, int reason,
            int type, int systemMode, int dir, int totalCallState, int emergencyType) {
        // whole state
        mCallInfoForRIL[INDEX_CALL_INFO_WHOLESTATE] = wholeState;

        // identification
        mCallInfoForRIL[INDEX_CALL_INFO_ID] = id;

        // individual state
        mCallInfoForRIL[INDEX_CALL_INFO_STATE] = state;

        // individual reason
        mCallInfoForRIL[INDEX_CALL_INFO_REASON] = reason;

        // individual call type
        mCallInfoForRIL[INDEX_CALL_INFO_TYPE] = type;

        // slot id
        mCallInfoForRIL[INDEX_CALL_INFO_SLOTID] = getSlotId();

        // systemMode
        mCallInfoForRIL[INDEX_CALL_INFO_SYSMODE] = systemMode;

        // direction
        mCallInfoForRIL[INDEX_CALL_INFO_DIRECTION] = dir;

        // total state
        mCallInfoForRIL[INDEX_CALL_INFO_TOTALSTATE] = totalCallState;

        // emergency call type
        mCallInfoForRIL[INDEX_CALL_INFO_EMERGENCYTYPE] = emergencyType;
    }

    private int addCallInfo(int sessionKey, int state, int reason, int type, int sessionType) {
        ImsLog.i(getSlotId(), "sessionKey=" + sessionKey);

        if (sessionKey == 0) {
            return 0;
        }

        CallInfo callInfo = new CallInfo(sessionKey);
        callInfo.init(state, reason, type, sessionType);
        mCallInfo.add(callInfo);

        return callInfo.mCallInfoID;
    }

    private void deleteCallInfo(int sessionKey) {
        ImsLog.i(getSlotId(), "sessionKey=" + sessionKey);

        CallInfo callInfo = getCallInfo(sessionKey);

        if (callInfo == null) {
            return;
        }

        if (callInfo.mState == IUMtcCall.VOLTE_CALL_STATE_IDLE) {
            mCallInfo.remove(callInfo);
            mCallInfoIDIndex--;
        }
    }

    private CallInfo getCallInfo(int sessionKey) {
        int size = mCallInfo.size();

        for (int i = 0; i < size; ++i) {
            CallInfo callInfo = mCallInfo.get(i);

            if (callInfo == null) {
                continue;
            }

            if (callInfo.mSessionKey == sessionKey) {
                return callInfo;
            }
        }

        return null;
    }

    private void handleAudioMediaInfo(int mediaParam) {
        int codecType = ((mediaParam & 0x000000f0) >> 4);
        int codecBandWidth = ((mediaParam & 0x00000f00) >> 8);
        int codecBitrate = ((mediaParam & 0x00ff0000) >> 16);

        ImsLog.d(getSlotId(), "codecType=" + codecType + ", codecBandWidth=" + codecBandWidth
                + ", codecBitrate=" + codecBitrate);
    }

    private static String convertCodecType(int codecType) {
        String strCodecType = "_";

        switch (codecType) {
            case CALL_MEDIA_CODEC_TYPE_NONE:
                strCodecType = "_";
                break;

            case CALL_MEDIA_CODEC_TYPE_AMR:
                strCodecType = "AMR";
                break;

            case CALL_MEDIA_CODEC_TYPE_AMRWB:
                strCodecType = "AMRWB";
                break;

            case CALL_MEDIA_CODEC_TYPE_G711_PCMU:
            case CALL_MEDIA_CODEC_TYPE_G711_PCMA:
                strCodecType = "G711";
                break;

            case CALL_MEDIA_CODEC_TYPE_EVS:
            case CALL_MEDIA_CODEC_TYPE_EVS_NB:
            case CALL_MEDIA_CODEC_TYPE_EVS_WB:
            case CALL_MEDIA_CODEC_TYPE_EVS_SWB:
            case CALL_MEDIA_CODEC_TYPE_EVS_FB:
                strCodecType = "EVS";
                break;

            default:
                strCodecType = "_";
                break;
        }

        return strCodecType;
    }

    private static String convertCodecBandWidth(int codecBandWidth) {
        String strCodecBandWidth = "_";

        switch (codecBandWidth) {
            case AUDIO_CODEC_BANDWIDTH_NB:
                strCodecBandWidth = "NB";
                break;

            case AUDIO_CODEC_BANDWIDTH_WB:
                strCodecBandWidth = "WB";
                break;

            case AUDIO_CODEC_BANDWIDTH_SWB:
                strCodecBandWidth = "SWB";
                break;

            case AUDIO_CODEC_BANDWIDTH_FB:
                strCodecBandWidth = "FB";
                break;

            default:
                strCodecBandWidth = "_";
                break;
        }

        return strCodecBandWidth;
    }

    private static String convertCodecBitrate(int codecBitrate) {
        String strCodecBitrate = "_";

        if ((codecBitrate >= 0) && (codecBitrate < AUDIO_CODEC_BITRATE_STRING.length)) {
            strCodecBitrate = AUDIO_CODEC_BITRATE_STRING[codecBitrate];
        } else {
            strCodecBitrate = "0";
        }

        return strCodecBitrate;
    }

    private static int convertCallStateForTouch(int callState) {
        if (callState == IUMtcCall.VOLTE_CALL_STATE_IDLE) {
            return ImsExtApi.TouchService.CALL_STATE_IDLE;
        } else if (callState == IUMtcCall.VOLTE_CALL_STATE_TERMINATING) {
            return ImsExtApi.TouchService.CALL_STATE_IDLE;
        } else if (callState == IUMtcCall.VOLTE_CALL_STATE_RINGBACK) {
            return ImsExtApi.TouchService.CALL_STATE_OFFHOOK;
        } else if (callState == IUMtcCall.VOLTE_CALL_STATE_RINGING) {
            return ImsExtApi.TouchService.CALL_STATE_RINGING;
        } else if (callState == IUMtcCall.VOLTE_CALL_STATE_ALERTING) {
            return ImsExtApi.TouchService.CALL_STATE_RINGING;
        } else if (callState == IUMtcCall.VOLTE_CALL_STATE_OFFHOOK) {
            return ImsExtApi.TouchService.CALL_STATE_OFFHOOK;
        }

        return ImsExtApi.TouchService.CALL_STATE_IDLE;
    }

    private int getSlotId() {
        return (mVoLteService != null) ? mVoLteService.getSlotID() : 0;
    }

    private void clearApnStateListener() {
        // More consideration: Emergency PDN
        if (mApnStateListener == null) {
            return;
        }

        IDCApn dcApn = (IDCApn)DCFactory.getDC(DCFactory.APN, getSlotId());
        IApn apn = (dcApn != null) ? dcApn.getApnControl(EApnType.IMS.getType()) : null;

        if (apn != null) {
            apn.removeListener(mApnStateListener);
        }

        mApnStateListener = null;
    }

    private void setApnStateListener() {
        // More consideration: Emergency PDN
        if (!mAvailableForNrSaMode) {
            return;
        }

        IDCApn dcApn = (IDCApn)DCFactory.getDC(DCFactory.APN, getSlotId());
        IApn apn = (dcApn != null) ? dcApn.getApnControl(EApnType.IMS.getType()) : null;

        if (apn != null) {
            mApnStateListener = new CallInfoApnStateListener();
            apn.addListener(mApnStateListener);
        }
    }

    private int getIpcanCategory() {
        // More consideration: Emergency PDN
        IDCApn dcApn = (IDCApn)DCFactory.getDC(DCFactory.APN, getSlotId());
        IApn apn = (dcApn != null) ? dcApn.getApnControl(EApnType.IMS.getType()) : null;
        return (apn != null) ? apn.getIpcanCategory() : IApn.IPCAN_CATEGORY_MOBILE;
    }

    // VoNR: 5G_NR_SA_MODE_CONTROL {
    private void sendNrSaMode(int nrSaMode) {
        /*setModemInfo(12 MODEM_I_SET_NR_SA_MODE,
                1 MODEM_I_DATA_TYPE_INT, nrSaMode, "");*/
    }

    private void setNrSaMode(int nrSaMode) {
        if (!mAvailableForNrSaMode) {
            return;
        }

        if (mNrSaMode == nrSaMode) {
            return;
        }

        ImsLog.i(getSlotId(), "setNrSaMode: " + mNrSaMode + " >> " + nrSaMode);

        mNrSaMode = nrSaMode;

        sendNrSaMode(mNrSaMode);
    }

    private void setNrSaModeOnCallStateChanged() {
        if (!mAvailableForNrSaMode) {
            return;
        }

        if (mTotalCallState > IUMtcCall.VOLTE_CALL_STATE_TERMINATING) {
            stopEnableSaTimer();
        }

        // Check old total call state to avoid duplicated call state for TERMINATING
        if ((mTotalOldCallState != IUMtcCall.VOLTE_CALL_STATE_TERMINATING)
                && ((mTotalCallState == IUMtcCall.VOLTE_CALL_STATE_TERMINATING)
                    || (mTotalCallState == IUMtcCall.VOLTE_CALL_STATE_IDLE))) {
            stopEnableSaTimer();

            if (mNrSaMode == NR_SA_DISABLED) {
                int ipcanCategory = getIpcanCategory();

                if (ipcanCategory == IApn.IPCAN_CATEGORY_WLAN) {
                    setNrSaMode(NR_SA_ENABLED);
                } else if (startEnableSaTimer()) {
                    ImsLog.i(getSlotId(), "NrSaMode: enable SA (delay) - S");
                } else {
                    setNrSaMode(NR_SA_ENABLED);
                }
            }
        } else if (mTotalCallState == IUMtcCall.VOLTE_CALL_STATE_OFFHOOK) {
            int ipcanCategory = getIpcanCategory();

            if (ipcanCategory == IApn.IPCAN_CATEGORY_WLAN) {
                setNrSaMode(NR_SA_DISABLED);
            } else {
                // Do not enable SA before call ends
                // setNrSaMode(NR_SA_ENABLED);
            }
        }
    }

    private boolean sendDelayedCallIdle() {
        int[] callInfoForIdle = null;

        synchronized (this) {
            callInfoForIdle = mCallInfoForIdle;
            mCallInfoForIdle = null;
        }

        if (callInfoForIdle != null) {
            ImsLog.i(getSlotId(), "sendDelayedCallIdle");

            IIMSPhoneAgent ip = IIMSPhoneGov.getInstance(getSlotId());

            if (ip != null) {
                ip.setImsCallStatus(callInfoForIdle[INDEX_CALL_INFO_WHOLESTATE],
                        callInfoForIdle[INDEX_CALL_INFO_ID],
                        callInfoForIdle[INDEX_CALL_INFO_STATE],
                        callInfoForIdle[INDEX_CALL_INFO_REASON],
                        callInfoForIdle[INDEX_CALL_INFO_TYPE],
                        callInfoForIdle[INDEX_CALL_INFO_SYSMODE],
                        callInfoForIdle[INDEX_CALL_INFO_DIRECTION],
                        callInfoForIdle[INDEX_CALL_INFO_TOTALSTATE],
                        callInfoForIdle[INDEX_CALL_INFO_EMERGENCYTYPE]);
            }

            return true;
        }

        return false;
    }

    private boolean startEnableSaTimer() {
        mEnableSaTimer = startTimer(
                MAX_DELAY_FOR_ENABLE_SA_ON_IDLE,
                EVENT_TIMER_ENABLE_SA,
                "NrSaMode-EnableSa");
        return isTimerStarted(mEnableSaTimer);
    }

    private void stopEnableSaTimer() {
        stopTimer(mEnableSaTimer, "NrSaMode-EnableSa");
        mEnableSaTimer = -1;
    }

    private boolean startSendCallIdleTimer() {
        mSendCallIdleTimer = startTimer(
                MAX_DELAY_FOR_CALL_IDLE,
                EVENT_TIMER_SEND_CALL_IDLE,
                "NrSaMode-sendCallIdle");
        return isTimerStarted(mSendCallIdleTimer);
    }

    private void stopSendCallIdleTimer() {
        stopTimer(mSendCallIdleTimer, "NrSaMode-sendCallIdle");
        mSendCallIdleTimer = -1;
    }

    private int startTimer(int duration, int event, String logTag) {
        if (mCallInfoHandler == null) {
            return -1;
        }

        IAlarmTimer atm = (IAlarmTimer)AgentFactory.getAgent(
                AgentFactory.ALARM_TIMER, getSlotId());
        int timerId = (atm != null) ? atm.getTimerId() : -1;

        if (timerId > 0) {
            atm.registerForTimerExpired(timerId, mCallInfoHandler, event, null);

            if (!atm.startTimer(timerId, duration)) {
                atm.unregisterForTimerExpired(timerId, mCallInfoHandler);
                return -1;
            }

            ImsLog.d(getSlotId(), logTag + ": startTimer=" + timerId);
            return timerId;
        }

        return -1;
    }

    private void stopTimer(int timerId, String logTag) {
        if (!isTimerStarted(timerId)) {
            return;
        }

        IAlarmTimer atm = (IAlarmTimer)AgentFactory.getAgent(
                AgentFactory.ALARM_TIMER, getSlotId());

        if (atm == null) {
            return;
        }

        ImsLog.d(getSlotId(), logTag + ": stopTimer=" + timerId);

        atm.unregisterForTimerExpired(timerId, mCallInfoHandler);
        atm.stopTimer(timerId);
    }

    private static boolean isTimerStarted(int timerId) {
        return timerId > 0;
    }
    // }

    private class CallInfoHandler extends Handler {
        public void handleMessage(Message msg) {
            if (msg == null) {
                return;
            }

            ImsLog.i("CallInfoHandler :: what=" + msg.what);

            switch (msg.what) {
                case ImsEventDef.IMS_EVENT_CALL_MEIDA_INFO: {
                    AsyncResult ar = (AsyncResult)msg.obj;
                    Message eventMsg = (ar != null) ? (Message)ar.result : null;

                    if (eventMsg != null) {
                        handleCallMediaInfo(eventMsg.arg1, eventMsg.arg2);
                    }
                    break;
                }
                case EVENT_TIMER_ENABLE_SA: {
                    ImsLog.i(getSlotId(), "NrSaMode: enable SA (delay) - E");
                    stopEnableSaTimer();
                    setNrSaMode(NR_SA_ENABLED);

                    if (startSendCallIdleTimer()) {
                        ImsLog.i(getSlotId(), "NrSaMode: sendCallIdleDelayed - S");
                    } else {
                        sendDelayedCallIdle();
                    }
                    break;
                }
                case EVENT_TIMER_SEND_CALL_IDLE: {
                    ImsLog.i(getSlotId(), "NrSaMode: sendCallIdleDelayed - E");
                    stopSendCallIdleTimer();
                    sendDelayedCallIdle();
                    break;
                }
                default:
                    // no-op
                    break;
            }
        }
    }

    public class CallInfo {
        public int mSessionKey = 0;
        public int mCallInfoID = 0;

        public int mState = IUMtcCall.VOLTE_CALL_STATE_IDLE;
        public int mOldState = IUMtcCall.VOLTE_CALL_STATE_IDLE;
        public int mStateReason = CallReasonInfo.CODE_NONE;
        public int mType = IUMtcCall.VOLTE_CALL_TYPE_NORMAL;
        public int mSessionType = IUMtcCall.CALLTYPE_VOIP;
        public int mSCMType = RIL_SCM_TYPE_NONE;
        public int mDir = RIL_CALL_DIR_MO;

        public CallInfo(int sessionKey) {
            mSessionKey = sessionKey;
            mCallInfoID = ++mCallInfoIDIndex;

            ImsLog.i(getSlotId(), "key=" + mSessionKey + ", id=" + mCallInfoID);
        }

        public void init(int state, int reason, int type, int sessionType) {
            mOldState = mState;
            mState = state;
            mStateReason = reason;
            mType = type;
            mSessionType = sessionType;

            if (state == IUMtcCall.VOLTE_CALL_STATE_RINGING
                    || state == IUMtcCall.VOLTE_CALL_STATE_ALERTING) {
                mDir = RIL_CALL_DIR_MT;
            }
        }

        public void update(int state, int reason, int type, int sessionType) {
            ImsLog.i(getSlotId(), "state=" + state + ", reason=" + reason
                    + ", type=" + type + ", sessionType=" + sessionType);

            if (mState != state) {
                mOldState = mState;
                mState = state;
                mStateReason = reason;
            }

            mType = type;
            mSessionType = sessionType;

            if (state == IUMtcCall.VOLTE_CALL_STATE_RINGING
                    || state == IUMtcCall.VOLTE_CALL_STATE_ALERTING) {
                mDir = RIL_CALL_DIR_MT;
            }
        }

        public void setState(int state, int reason) {
            if (mState == state) {
                return;
            }

            ImsLog.i(getSlotId(), "state :: " + mState + " >> " + state + ", reason=" + reason);

            mOldState = mState;
            mState = state;
            mStateReason = reason;
        }

        public void setType(int type) {
            if (mType == type) {
                return;
            }

            ImsLog.i(getSlotId(), "type :: " + mType + " >> " + type);
            mType = type;
        }

        public void setSessionType(int sessionType) {
            if (mSessionType == sessionType) {
                return;
            }

            ImsLog.i(getSlotId(), "sessionType :: " + mSessionType + " >> " + sessionType);
            mSessionType = sessionType;
        }

        public void setSCMType(int scmtype) {
            if (mSCMType == scmtype) {
                return;
            }

            ImsLog.i(getSlotId(), "scmType :: " + mSCMType + " >> " + scmtype);
            mSCMType = scmtype;
        }
    }

    private class CallInfoApnStateListener extends ApnStateListener {
        @Override
        public void onIpcanCategoryChanged(int apnType, int ipcanCategory) {
            ImsLog.i(getSlotId(), "onIpcanCategoryChanged :: apnType="
                    + apnType + ", ipcanCategory=" + ipcanCategory);

            if (mTotalCallState == IUMtcCall.VOLTE_CALL_STATE_OFFHOOK) {
                if (ipcanCategory == IApn.IPCAN_CATEGORY_WLAN) {
                    setNrSaMode(NR_SA_DISABLED);
                } else {
                    // Do not enable SA before call ends
                    // setNrSaMode(NR_SA_ENABLED);
                }
            }
        }
    }
}
