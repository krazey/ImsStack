package com.android.imsstack.core.agents;

import android.content.Context;
import android.text.TextUtils;

import com.android.imsstack.core.CapabilityConfigs;
import com.android.imsstack.core.agents.agentif.IAgent;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ISystemAPIVoNR;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.ImsLog;

public class VoNRAgent implements IAgent, ISystemAPIVoNR {
    /** VoNR events */
    private static final int EVENT_CALL_READY = 1;
    private static final int EVENT_HANDOFF_INFORMATION = 2;
    private static final int EVENT_UAC_RESPONSE = 3;

    /** call type for IMS signaling UAC */
    private static final int CALL_TYPE_DEFAULT = 0;
    private static final int CALL_TYPE_VOLTE = 1;

    /** voice status for MTK Call */
    private static final int MTK_CALL_START = 0; //MTK_CALL_STOP = 1

    private Context mContext;
    private ISystem mSystem;

    private VoNRListener mVoNRListener;

    private String mIfaceNameForImsApn;

    private final int mSlotId;

    // test code
    private boolean mIsNrRegInfoEnabled = false;

    private class ImsVoNR {
        public ImsVoNR() {

        }
    }

    public VoNRAgent(int slotId) {
        mSlotId = slotId;
    }

    // IAgent
    @Override
    public void init(Context context) {
        mContext = context;

        if (CapabilityConfigs.isVoNrEnabled(mSlotId)) {
            ImsLog.d(mSlotId, "vonr is supported");
            mSystem = SystemInterface.getInstance().getSystem(mSlotId);
            if (mSystem != null) {
                mSystem.setISystemAPIVoNR(this);
            }

            mVoNRListener = new VoNRListener();
            setVoNRListener(mVoNRListener);
        }
    }

    @Override
    public void cleanup() {
        ImsLog.d(mSlotId, "");

        if (mVoNRListener != null) {
            setVoNRListener(null);
        }

        if (mSystem != null) {
            mSystem.setISystemAPIVoNR(null);
            mSystem = null;
        }

        mVoNRListener = null;
    }

    // ISystemAPIVoNR
    @Override
    public int notifyCallState(int type, int state, int sysMode, int direction) {
        ImsLog.d(mSlotId, "type = " + type + " , state = " + state + " , sysMode = " + sysMode +
            " , direction = " + direction);

        ImsVoNR vonr = getVoNR();

        if (vonr == null) {
            return 0;
        }

        int result = 0; //(vonr.notifyCallState(type, state, sysMode, direction) == true) ? 1 : 0;
        ImsLog.d(mSlotId, "result = " + result);
        return result;
    }

    @Override
    public int requestCallPreference(int rat, int type) {
        ImsLog.d(mSlotId, "rat = " + rat + " , type = " + type);

        ImsVoNR vonr = getVoNR();

        if (vonr == null) {
            return 0;
        }

        int result = 0;//(vonr.requestCallPreference(rat, type) == true) ? 1 : 0;
        ImsLog.d(mSlotId, "result = " + result);
        return result;
    }

    @Override
    public int setImsSession(int type, int state) {
        ImsLog.d(mSlotId, "type = " + type + "state = " + state);

        ImsVoNR vonr = getVoNR();

        if (vonr == null) {
            return 0;
        }

        int result = 0;//(vonr.setImsSession(type, state) == true) ? 1 : 0;
        ImsLog.d(mSlotId, "result = " + result);
        return result;
    }

    @Override
    public int setImsSignalingForUAC(int type) {
        ImsLog.d(mSlotId, "type = " + type);

        String ifaceName = null;
        if (type == CALL_TYPE_VOLTE) {
            IDcApn dcapn = (IDcApn) DcFactory.getDc(DcFactory.APN, mSlotId);
            if (dcapn != null) {
                ifaceName = dcapn.getIfaceName(EApnType.IMS.getType());
            }
        } else {
            ifaceName = mIfaceNameForImsApn;
        }

        if (TextUtils.isEmpty(ifaceName)) {
            mIfaceNameForImsApn = null;
            return 0;
        }

        mIfaceNameForImsApn = ifaceName;

        ImsLog.d(mSlotId, "interface name = " + mIfaceNameForImsApn);

        ImsVoNR vonr = getVoNR();

        if (vonr == null) {
            return 0;
        }

        int result = 0;//(vonr.setImsSignalingForUAC(type, mIfaceNameForImsApn) == true) ? 1 : 0;
        ImsLog.d(mSlotId, "result = " + result);
        return result;
    }

    @Override
    public int setImsVoice(int state, int sysMode) {
        ImsLog.d(mSlotId, "state = " + state + "sysMode = " + sysMode);

        ImsVoNR vonr = getVoNR();

        if (vonr == null) {
            return 0;
        }

        boolean start = (state == MTK_CALL_START) ? true : false;
        int result = 0;//(vonr.setImsVoice(start, sysMode) == true) ? 1 : 0;
        ImsLog.d(mSlotId, "result = " + result);
        return result;
    }

    @Override
    public int setUacCheck(int type, int state) {
        ImsLog.d(mSlotId, "type = " + type + "state = " + state);

        ImsVoNR vonr = getVoNR();

        if (vonr == null) {
            return 0;
        }

        int result = 0;//(vonr.setUacCheck(type, state) == true) ? 1 : 0;
        ImsLog.d(mSlotId, "result = " + result);
        return result;
    }

    @Override
    public int setVoice(int state, int emergency) {
        ImsLog.d(mSlotId, "state = " + state + "emergency = " + emergency);

        ImsVoNR vonr = getVoNR();

        if (vonr == null) {
            return 0;
        }

        boolean start = (state == MTK_CALL_START) ? true : false;
        boolean isEmergency = (emergency > 0) ? true : false;
        int result = 0;//(vonr.setVoice(start, isEmergency) == true) ? 1 : 0;
        ImsLog.d(mSlotId, "result = " + result);
        return result;
    }

    private ImsVoNR getVoNR() {
        return null;
    }

    private void setVoNRListener(VoNRListener listener) {
        // no-op
    }

    private class VoNRListener {
        public void onCallReady(int sysMode) {
            if (mSystem != null) {
                ImsLog.i(mSlotId, "sys mode = " + sysMode);
                mSystem.notifyCallReady(EVENT_CALL_READY, sysMode);
            }
        }

        public void onHandoffInformation(int status, int sourceRat, int targetRat,
            int reasonType, int reason) {
            if (mSystem != null) {
                ImsLog.i(mSlotId, "status = " + status + ", sRat = " + sourceRat + ", tRat = "
                    + targetRat + ", reason type = " + reasonType + ", reason = " + reason);
                mSystem.notifyHandoffInformation(EVENT_HANDOFF_INFORMATION, status, sourceRat,
                    targetRat, reasonType, reason);
            }
        }

        public void onNrRegistrationInformation(int status, int reason) {
            ImsLog.i(mSlotId, "status = " + status + ", reason = " + reason);

            if (!mIsNrRegInfoEnabled) {
                return;
            }

            IDcNetWatcher dcnw = (IDcNetWatcher) DcFactory.getDc(
                    DcFactory.NETWORK_WATCHER, mSlotId);
            if (dcnw != null) {
                dcnw.setNrRegistrationInfo(status, reason);
            }
        }

        public void onUacBarredAlleviation(int type) {
            ImsLog.i(mSlotId, "service type = " + type);
        }

        public void onUacResponse(int callType, int sysMode, int result, int barringTime) {
            if (mSystem != null) {
                ImsLog.i(mSlotId, "call type = " + callType + ", sys mode = " + sysMode
                    + ", result = " + result + ", barring time = " + barringTime);
                mSystem.notifyUacResponse(EVENT_UAC_RESPONSE,
                        callType, sysMode, result, barringTime);
            }
        }
    }
}
