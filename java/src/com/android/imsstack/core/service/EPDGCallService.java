package com.android.imsstack.core.service;

import android.content.Context;

import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.core.service.serviceif.IService;
import com.android.imsstack.core.service.serviceif.IVoLteService;
import com.android.imsstack.enabler.mtc.Call;
import com.android.imsstack.enabler.mtc.CallStateListener;
import com.android.imsstack.enabler.mtc.FailInfo;
import com.android.imsstack.enabler.mtc.IUMtcCall;
import com.android.imsstack.enabler.mtc.MtcCall;
import com.android.imsstack.internal.enabler.ImsStateStore;
import com.android.imsstack.internal.imsservice.CallUtils;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsUtils;

import java.util.ArrayList;

public class EPDGCallService implements IService {
    /** Call fail code for DPD */
    private static final int EXTRA_CODE_DPD_REQUEST = 1000;

    private static final int CALL_TYPE_IDLE = 0;
    private static final int CALL_TYPE_VOICE_ONLY = 1;
    private static final int CALL_TYPE_VIDEO_ONLY = 2;
    private static final int CALL_TYPE_VOICE_VIDEO = 3;

    private IVoLteService mVoLteService = null;
    private EPDGCallAgentCallStateListener mCallListener = null;
    private ArrayList<Call> mSessionList = new ArrayList<Call>();

    public EPDGCallService() {
    }

    @Override
    public boolean start(IVoLteService voLteService) {
        mVoLteService = voLteService;

        ImsLog.i(getSlotId(), "");

        if (!isEPDGCallServiceRequired()) {
            return false;
        }

        mCallListener = new EPDGCallAgentCallStateListener();

        CallUtils.addCallStateListener(getSlotId(), mCallListener);

        return true;
    }

    @Override
    public void cleanup(Context context) {
        ImsLog.i(getSlotId(), "");

        CallUtils.removeCallStateListener(getSlotId(), mCallListener);
    }

    @Override
    public void update(Context context) {
    }

    private boolean isEPDGCallServiceRequired() {
        if (ImsGlobal.isOperator(getSlotId(), "VZW")) {
            // VZW does the same in VoWiFiVZW class
            return false;
        }

        if (ImsUtils.isWfcEnabledByPlatform(mVoLteService.getContext(), getSlotId())) {
            return true;
        }

        return false;
    }

    private void updateCallStatus(Call call) {
        int callIndex = findCallInList(call.getNativeCallId());

        ImsLog.w(getSlotId(), "CallId=" + call.getNativeCallId() + ", Index=" + callIndex );

        if (callIndex > -1) {
            mSessionList.set(callIndex, call);
        } else {
            mSessionList.add(call);
        }

        processCallStateChanged();
    }

    private int findCallInList( long sessionID ) {
        for (int index = 0; index < mSessionList.size(); index++) {
            Call call = mSessionList.get(index);

            if (call == null) {
                continue;
            }

            if (call.getNativeCallId() == sessionID) {
                return index;
            }
        }

        return -1;
    }

    private void processCallStateChanged() {
        if (mSessionList.size() == 0) {
            notifyCallTypeToEPDG(CALL_TYPE_IDLE);
        } else {
            notifyCallTypeToEPDG(getCallTypeForEPDG());
        }
    }

    private int getCallTypeForEPDG() {
        boolean voice = false;
        boolean video = false;

        for (int index = 0; index < mSessionList.size(); ++index) {
            Call call = mSessionList.get(index);

            if (call == null) {
                continue;
            }

            int calltype = call.getCallType();
            if (calltype == IUMtcCall.CALLTYPE_VOIP || calltype == IUMtcCall.CALLTYPE_RTT) {
                voice = true;
            } else {
                video = true;
            }
        }

        if (!voice && !video) {
            return CALL_TYPE_IDLE;
        }

        if (!voice && video) {
            return CALL_TYPE_VIDEO_ONLY;
        }

        if (voice && !video) {
            return CALL_TYPE_VOICE_ONLY;
        }

        // voice and video
        return CALL_TYPE_VOICE_VIDEO;
    }

    private void notifyCallTypeToEPDG(int callType) {
        /*ImsExtApi.Data.setCallType(getSlotId(),
                callType, NetworkCapabilities.NET_CAPABILITY_IMS);*/

        ImsLog.i(getSlotId(), "type=" + callType);
    }

    private void processDPDRequest(Call call) {
        if (!ImsGlobal.isOperator(getSlotId(), "TEL")) {
            return;
        }

        Context c = getContext();

        if (c == null) {
            return;
        }

        final int regState = ImsStateStore.getRegState(getSlotId()).getNetworkType();

        if (regState == 0) {
            return;
        }

        MtcCall mtcCall = (MtcCall)call;
        FailInfo failInfo = mtcCall.getTerminationReason();

        if ((failInfo.Code == EXTRA_CODE_DPD_REQUEST
                && failInfo.Reason == IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_TEMPUNAVAILABLE)
                || failInfo.Reason == IUMtcCall.Fail_Reason.FAIL_REASON_MEDIA_NODATA) {
            //ImsExtApi.Data.requestDoDPD(getSlotId());
        }
    }

    private Context getContext() {
        return (mVoLteService != null) ? mVoLteService.getContext() : null;
    }

    private int getSlotId() {
        return (mVoLteService != null) ? mVoLteService.getSlotID() : 0;
    }

    private class EPDGCallAgentCallStateListener extends CallStateListener {
        public EPDGCallAgentCallStateListener() {
        }

        @Override
        public void onCallCreated(Call call) {
            // no-op
        }

        @Override
        public void onCallDestroyed(Call call) {
            int callIndex = findCallInList(call.getNativeCallId());

            ImsLog.i(getSlotId(), "CallID=" + call.getNativeCallId() + ", Index=" + callIndex);

            if (callIndex > -1) {
                mSessionList.remove(callIndex);
                processCallStateChanged();
            }
        }

        @Override
        public void onCallEstablishing(Call call) {
            ImsLog.i(getSlotId(), "");

            mSessionList.add(call);

            updateCallStatus(call);
        }

        @Override
        public void onCallRinging(Call call) {
            ImsLog.i(getSlotId(), "");

            mSessionList.add(call);

            updateCallStatus(call);
        }

        @Override
        public void onCallAccepted(Call call) {
            ImsLog.i(getSlotId(), "");

            updateCallStatus(call);
        }

        @Override
        public void onCallEstablished(Call call) {
            ImsLog.i(getSlotId(), "");

            updateCallStatus(call);
        }

        @Override
        public void onCallUpdated(Call call) {
            ImsLog.i(getSlotId(), "");

            updateCallStatus(call);
        }

        @Override
        public void onCallTerminating(Call call) {
            // no-op
        }

        @Override
        public void onCallTerminated(Call call) {
            int callIndex = findCallInList(call.getNativeCallId());

            processDPDRequest(call);
            ImsLog.i(getSlotId(), "CallID=" + call.getNativeCallId() + ", Index=" + callIndex);

            if (callIndex > -1) {
                mSessionList.remove(callIndex);
            }

            processCallStateChanged();
        }
    }
}
