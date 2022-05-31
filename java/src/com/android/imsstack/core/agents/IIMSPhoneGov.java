package com.android.imsstack.core.agents;

import android.content.Context;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Registrant;
import android.os.RegistrantList;
import android.telephony.CellInfo;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.imsstack.core.CapabilityConfigs;
import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.core.OperatorInfo;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.agentif.IIMSPhoneAgent;
import com.android.imsstack.core.agents.dcm.DCFactory;
import com.android.imsstack.core.agents.dcmif.IDCNetWatcher;
import com.android.imsstack.system.IJNIUpCallEvt;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ISystemAPIIMSPhone;
import com.android.imsstack.system.ImsEventDef;
import com.android.imsstack.system.JNIUpCallEvtManager;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsProperties;
import com.android.imsstack.util.ImsUtils;
import com.android.imsstack.util.MSimUtils;

import java.util.Arrays;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

public final class IIMSPhoneGov {
    /** Internal events */
    private static final int EVENT_IMS_PHONE_STARTED = 1001;
    private static final int EVENT_IMS_RADIO_STATE_CHANGED = 1002;
    private static final int MAX_PSI_LENGTH = 50;

    private static IIMSPhoneGov sIIMSPHoneGov = null;
    private static ConcurrentHashMap<Integer, IImsPhone> sIImsPhone
            = new ConcurrentHashMap<Integer, IImsPhone>();

    private Context mContext = null;
    private Handler mIIMSPhoneGovHandler;

    private IIMSPhoneGov() {
        ImsLog.d("");
    }

    public static IIMSPhoneGov getGov() {
        if (sIIMSPHoneGov == null) {
            sIIMSPHoneGov = new IIMSPhoneGov();
        }

        return sIIMSPHoneGov;
    }

    public static synchronized IIMSPhoneAgent getInstance(int slotID) {
        if (!sIImsPhone.containsKey(slotID)) {
            return null;
        }

        return sIImsPhone.get(slotID);
    }

    public void init(Context context) {
        if (context == null) {
            return;
        }

        mContext = context;

        // Add BR Listener
        mIIMSPhoneGovHandler = new IIMSPhoneGovHandler();
    }

    public void cleanup() {
        ImsLog.d("");

        if (mIIMSPhoneGovHandler != null) {
            mIIMSPhoneGovHandler.removeCallbacksAndMessages(null);
            mIIMSPhoneGovHandler = null;
        }
    }

    public void start(int slotID) {
        ImsLog.d("SlotId=" + slotID);

        if (!sIImsPhone.containsKey(slotID)) {
            IImsPhone imsPhone = new IImsPhone(slotID);
            sIImsPhone.put(slotID, imsPhone);
            imsPhone.init(mContext);
        }
    }

    public void stop(int slotID) {
        ImsLog.d("SlotId=" + slotID);

        if (sIImsPhone.containsKey(slotID)) {
            IImsPhone imsPhone = sIImsPhone.get(slotID);
            imsPhone.cleanup();
            imsPhone = null;
            sIImsPhone.remove(slotID);
        }
    }

    public static String convertPSIToString(String psi) {
        /**
         * 1st 1 char : meaningless
         * 2nd 1 char : length of value
         * last char : PSI value (Ascii)
         */
        if (TextUtils.isEmpty(psi)) {
            ImsLog.d("Invalid PSI: " + ImsLog.hiddenString(psi));
            return null;
        }

        // At least 5 hex-digits are required
        if (psi.length() <= 4) {
            ImsLog.d("Invalid PSI: " + psi);
            return null;
        }

        // parsing length of PSI string (Hex -> Dec)
        int nPsiLength = Integer.parseInt(psi.substring(2, 4), 16);

        if (nPsiLength > MAX_PSI_LENGTH) {
            ImsLog.w("PSI is garbage value");
            return null;
        }

        // convert Ascii int to String
        int ascii = 0;
        int indexStart = 0;
        int indexEnd = 0;
        String strOutStr = "";

        for (int i = 0; i < nPsiLength; i++) {
            // ignore first 4 characters
            indexStart = 4 + 2 * i;
            indexEnd = indexStart + 2;

            // parsing 2 characters (Hex -> Dec)
            ascii = Integer.parseInt(psi.substring(indexStart, indexEnd), 16);
            strOutStr = strOutStr + (char)ascii;
        }

        ImsLog.d("PSI(SMSoIP): " + strOutStr);

        return strOutStr;
    }

    private static int getOtherSlotId(int slotId) {
        // FIXME: if tri-sim is used...
        return (slotId == MSimUtils.DEFAULT_SLOT_ID)
                ? (MSimUtils.DEFAULT_SLOT_ID + 1)
                : MSimUtils.DEFAULT_SLOT_ID;
    }

    private class IIMSPhoneGovHandler extends Handler {
        public void handleMessage(Message msg) {
            if (msg == null) {
                return;
            }

            ImsLog.i("IIMSPhoneGovHandler :: what=" + msg.what);

            switch(msg.what) {
                case EVENT_IMS_PHONE_STARTED:
                    handleImsPhoneStarted();
                    break;

                case EVENT_IMS_RADIO_STATE_CHANGED:
                    handleImsRadioStateChanged(msg);
                    break;

                default:
                    break;
            }
        }

        public void handleImsPhoneStarted() {
            for (int i = 0; i < sIImsPhone.size(); i++) {
                ISystem system = SystemInterface.getInstance().getSystem(i);

                if (system != null) {
                    system.notifyEvent(ImsEventDef.IMS_EVENT_PHONE_RESTARTED, 0, 0);
                }

                IImsPhone imsPhone = sIImsPhone.get(i);

                if (imsPhone != null) {
                    imsPhone.initSysInfo();
                    imsPhone.mSystemServiceRestartedRegistrants.notifyRegistrants();
                }
            }
        }

        private int getSsacBarringFactor(int factor) {
            if (ImsProperties.isChipVendorMtk()) {
                int[] factorArr = new int[] {0, 5, 10, 15, 20, 25, 30, 40, 50,
                        60, 70, 75, 80, 85, 90, 95, 100};

                if ((factor >= 0) && (factor < factorArr.length)) {
                    return factorArr[factor];
                }

                return 100;
            } else {
                return ((factor == 255) ? 100 : factor);
            }
        }

        private int getSsacBarringTime(int time) {
            if (ImsProperties.isChipVendorMtk()) {
                int[] timeArr = new int[] {0, 4, 8, 16, 32, 64, 128, 256, 512};

                if ((time >= 0) && (time < timeArr.length)) {
                    return timeArr[time];
                }

                return 0;
            } else {
                return time;
            }
        }

        private void handleImsRadioStateChanged(Message msg) {
            int slotID = msg.arg1;
            int[] param = (int[])msg.obj;

            if (param == null || param.length < 3) {
                return;
            }

            int type = param[0];

            ImsLog.d(slotID, "ImsRadioStateChanged: state=" + Arrays.toString(param));

            if (type == 203/*IMS_RADIO_STATE_NR_UE_CAPABILITY*/) {
                handleNrUeCapabilityChanged(slotID, param[1]);
                return;
            }

            if (!isLteStateAllowed(type, slotID)) {
                return;
            }

            switch (type) {
                case ImsEventDef.IMS_LTE_BARRING_SSAC: // FALL-THROUGH
                case ImsEventDef.IMS_LTE_BARRING_SSAC_EX:
                    handleLteStateChanged_SSAC(param, slotID);
                    break;
                case ImsEventDef.IMS_LTE_SR_REJECT_WITH_EMM:
                    if (ImsGlobal.isOperator(slotID, "ORG")) {
                        handleLteStateChanged_Common(param[0], param[1], slotID);
                    } else {
                        handleLteStateChanged_Common(param[0], param[2], slotID);
                    }
                    break;
                case ImsEventDef.IMS_LTE_RACH_IGNORE_DURING_T300_3TIMES:
                    // param : 1 minute
                    handleLteStateChanged_Common(ImsEventDef.IMS_LTE_BLOCK_WITH_TIME,
                            60000, slotID);
                    break;
                case ImsEventDef.IMS_LTE_ACCESS_BARRED_FOR_MO_DATA: {
                    ISystem system = SystemInterface.getInstance().getSystem(slotID);

                    if (system != null) {
                        system.notifyEvent(ImsEventDef.IMS_EVENT_PS_BARRING_STATE,
                                param[1], param[2]);
                    }
                    break;
                }
                case 202: // IMS_RADIO_STATE_LTE_ACB:
                    ISystem system = SystemInterface.getInstance().getSystem(slotID);

                    if (system != null) {
                        system.notifyEvent(ImsEventDef.IMS_EVENT_AC_BARRING_STATE,
                                (param[1] == ImsEventDef.IMS_SERVICE_ON) ?
                                ImsEventDef.IMS_SERVICE_ON : ImsEventDef.IMS_SERVICE_OFF, 0);
                    }
                    break;
                default:
                    handleLteStateChanged_Common(param[0], param[1], slotID);
                    break;
            }
        }

        private void handleLteStateChanged_Common(int type, int param, int slotID) {
            ISystem system = SystemInterface.getInstance().getSystem(slotID);

            if (system != null) {
                system.notifyEvent(ImsEventDef.IMS_EVENT_LTE_STATE, type, param);
            }

            notifyLteState(type, param, slotID);
        }

        private void handleLteStateChanged_SSAC(int[] param, int slotID) {
            if (param.length < 5) {
                return;
            }

            for (int i = 1; i < 5; i++) {
                if (param[i] == -1) {
                    return;
                }
            }

            int voiceFactor = getSsacBarringFactor(param[1]);
            int voiceTimer = getSsacBarringTime(param[2]);
            int videoFactor = getSsacBarringFactor(param[3]);
            int videoTimer = getSsacBarringTime(param[4]);

            notifyLteState(ImsEventDef.IMS_LTE_BARRING_SSAC, voiceFactor, slotID);

            ImsLog.d("voice(factor=" + voiceFactor + " , timer=" + voiceTimer
                    + ") , video(factor=" + videoFactor + " , timer=" + videoTimer + ")");

            // ImsEventDef.IMS_LTE_BARRING_SSAC_VOICE
            int voiceBarringInfo = voiceFactor << 16 | voiceTimer;
            int videoBarringInfo = videoFactor << 16 | videoTimer
                    | ImsEventDef.IMS_LTE_BARRING_SSAC_VIDEO;

            ISystem system = SystemInterface.getInstance().getSystem(slotID);

            if (system != null) {
                system.notifyEvent(ImsEventDef.IMS_EVENT_LTE_STATE,
                        ImsEventDef.IMS_LTE_BARRING_SSAC, voiceBarringInfo);
                system.notifyEvent(ImsEventDef.IMS_EVENT_LTE_STATE,
                        ImsEventDef.IMS_LTE_BARRING_SSAC, videoBarringInfo);
            }
        }

        private void handleNrUeCapabilityChanged(int slotId, int nrUeCapability) {
            int oldNrUeCapability = ImsUtils.getNrUeCapability(slotId);

            if (oldNrUeCapability != nrUeCapability) {
                ImsLog.i(slotId, "NrUeCapabilityChanged: 0x" +
                        Integer.toHexString(oldNrUeCapability) + " >> 0x" +
                        Integer.toHexString(nrUeCapability));
                ImsUtils.setNrUeCapability(slotId, nrUeCapability);
                OperatorInfo.setSystemConfigForServiceFeature(slotId);
            }
        }

        private boolean isLteStateAllowed(int type, int slotID) {
            boolean isAllowed = false;
            String op = ImsGlobal.getOperator(slotID);
            String co = ImsGlobal.getCountry(slotID);

            if (ImsGlobal.equalsOperator("VZW", op)) {
                switch (type) {
                    case ImsEventDef.IMS_LTE_SR_REJECT_WITH_EMM9_EMM10: // FALL-THROUGH
                    case ImsEventDef.IMS_LTE_BARRING_MO_DATA: // FALL-THROUGH
                    case ImsEventDef.IMS_LTE_QOS_DEDICATED_BEARER_COMPLETED: // FALL-THROUGH
                    case ImsEventDef.IMS_LTE_BARRING_SSAC: // FALL-THROUGH
                    case ImsEventDef.IMS_LTE_SR_REJECT_WITH_EMM: // FALL-THROUGH
                    case ImsEventDef.IMS_LTE_TRIGGER_DEREGISTRATION: // FALL-THROUGH
                    case ImsEventDef.IMS_LTE_RACH_IGNORE_DURING_T300_3TIMES:
                        isAllowed = true;
                        break;

                    case ImsEventDef.IMS_LTE_RACH_REJECT_WITH_WAITTIME: // FALL-THROUGH
                    case ImsEventDef.IMS_LTE_BARRING_SSAC_EX: // FALL-THROUGH
                    case ImsEventDef.IMS_LTE_DEACTIVATE_IMS_PDN: // FALL-THROUGH
                    case ImsEventDef.IMS_LTE_CSFB_PREF_SUB_STATE: // FALL-THROUGH
                    case ImsEventDef.IMS_LTE_UPDATE_CURRENT_REG_STATE:
                        isAllowed = true;
                        break;

                    default:
                        break;
                }
            } else {
                switch (type) {
                    case ImsEventDef.IMS_LTE_BARRING_MO_DATA: // FALL-THROUGH
                    case ImsEventDef.IMS_LTE_BARRING_SSAC: // FALL-THROUGH
                    case ImsEventDef.IMS_LTE_BARRING_SSAC_EX: // FALL-THROUGH
                    case ImsEventDef.IMS_LTE_ACCESS_BARRED_FOR_MO_DATA: // FALL-THROUGH
                    case 202: // IMS_RADIO_STATE_LTE_ACB : //FALL-THROUGH
                    case ImsEventDef.IMS_LTE_TRIGGER_DEREGISTRATION:
                        isAllowed = true;
                        break;
                    default:
                        break;
                }
            }

            if (ImsGlobal.equalsOperator("USC", op)
                    || ImsGlobal.equalsOperator("SPR", op)
                    || ImsGlobal.equalsOperator("ACG", op)) {
                switch (type) {
                    case ImsEventDef.IMS_LTE_CSFB_PREF_SUB_STATE: // FALL-THROUGH
                    case ImsEventDef.IMS_LTE_UPDATE_CURRENT_REG_STATE:
                        isAllowed = true;
                        break;

                    default:
                        break;
                }
            }

            if (ImsGlobal.equalsOperator("ORG", op)) {
                switch (type) {
                    case ImsEventDef.IMS_LTE_SR_REJECT_WITH_EMM:
                        isAllowed = true;
                        break;

                    default:
                        break;
                }
            }

            return isAllowed;
        }

        private void notifyLteState(int type, int param, int slotID) {
            Message msg = Message.obtain();
            msg.arg1 = type;
            msg.arg2 = param;

            if (sIImsPhone.containsKey(slotID)) {
                IImsPhone imsPhone = sIImsPhone.get(slotID);
                imsPhone.mLteStateChangedRegistrants.notifyResult(msg);
            }
        }
    }

    public class IImsPhone implements IIMSPhoneAgent, ISystemAPIIMSPhone {
        private RegistrantList mSystemServiceRestartedRegistrants = new RegistrantList();
        private RegistrantList mLteStateChangedRegistrants = new RegistrantList();
        private RegistrantList mModemECCPriorityRegistrants = new RegistrantList();
        private RegistrantList mUSATEnvelopeResponseRegistrants = new RegistrantList();
        // IMS Network Info
        private RegistrantList mLteVoPSChangedRegistrants = new RegistrantList();
        private RegistrantList mWcdmaVoPSChangedRegistrants = new RegistrantList();
        private RegistrantList mEmcBSChangedRegistrants = new RegistrantList();
        private RegistrantList mImsEmcSupportChangedRegistrants = new RegistrantList();
        private RegistrantList mAcBarringForEmegencyChangedRegistrants = new RegistrantList();
        private Context mContext = null;
        private final int mSlotId;
        private Handler mIIMSPhoneHandler;

        public IImsPhone(int slotID) {
            ImsLog.d(slotID, "");

            mSlotId = slotID;

            ISystem system = SystemInterface.getInstance().getSystem(mSlotId);
            if (system != null) {
                system.setISystemAPIIMSPhone(this);
            }

            AgentFactory.setAgentForMIms(this, AgentFactory.IMS_PHONE, mSlotId);

            mIIMSPhoneHandler = new IIMSPhoneHandler();

            IJNIUpCallEvt jniEvt = JNIUpCallEvtManager.getInstance().getJNIUpCallEvt(mSlotId);
            if (jniEvt != null) {
                jniEvt.registerForSendDataToModem(mIIMSPhoneHandler,
                        ImsEventDef.IMS_EVENT_SEND_DATA_TO_MODEM, null);
            }

        }

        @Override
        public void init(Context context) {
            ImsLog.d(mSlotId, "");

            if (context != null) {
                mContext = context;
                initSysInfo();
            }
        }

        public void cleanup() {
            ImsLog.d(mSlotId, "");

            if (MSimUtils.isMultiLteEnabled()) {
                // TRM is initialized
                setTrm(0);
            }

            AgentFactory.setAgentForMIms(null, AgentFactory.IMS_PHONE, mSlotId);

            if (mIIMSPhoneHandler != null) {
                mIIMSPhoneHandler.removeCallbacksAndMessages(null);
                mIIMSPhoneHandler = null;
            }

            clearImsConfig();
        }

        private void initSysInfo() {
            ImsLog.d(mSlotId, "");

            // Some Modem values should be initialized, in case of restarting Ims...
            // Set IMS Registration ON/OFF Info for De-registration guard time (MODEM)

            if (MSimUtils.isMultiLteEnabled()) {
                // TRM is initialized
                setTrm(0);
            }

            setImsConfig();
        }

        // ImsPhoneProxy.Listener {
        public void onCommand(int cmd, int param) {
            ImsLog.d(mSlotId, "onCommand :: cmd=" + cmd + ", param=" + param);

            if (cmd == 1/*Command.IMS_REGISTRATION*/) {
                processImsRegistrationControl(param);
            }
        }

        public void onImsRadioStateChanged(int[] imsRadioState) {
            ImsLog.d(mSlotId, "onImsRadioStateChanged :: ");

            if (mIIMSPhoneGovHandler != null) {
                Message.obtain(mIIMSPhoneGovHandler, EVENT_IMS_RADIO_STATE_CHANGED,
                        mSlotId, 0, (Object)imsRadioState).sendToTarget();
            }
        }

        public void onNetworkFeatureChanged(Object feature,
                int changedFeatures) {
            ImsLog.i(mSlotId, "onNetworkFeatureChanged :: " + feature
                    + ", changed=0x" + Integer.toHexString(changedFeatures));

            if (feature == null) {
                return;
            }

            /*
            if (hasVoPS(changedFeatures)) {
                mLteVoPSChangedRegistrants.notifyResult(
                        Integer.valueOf(feature.getVoPS()));
            }

            if (hasVoPSOn3G(changedFeatures)) {
                mWcdmaVoPSChangedRegistrants.notifyResult(
                        Integer.valueOf(feature.getVoPSOn3G()));
            }

            if (hasEmcBs(changedFeatures)) {
                mEmcBSChangedRegistrants.notifyResult(
                        Integer.valueOf(feature.getEmcBs()));
            }

            if (hasImsEmergencySupport(changedFeatures)) {
                mImsEmcSupportChangedRegistrants.notifyResult(
                        Integer.valueOf(feature.getImsEmergencySupport()));
            }

            if (hasAcBarringForEmergency(changedFeatures)) {
                mAcBarringForEmegencyChangedRegistrants.notifyResult(
                        Integer.valueOf(feature.getAcBarringForEmergency()));
            }*/
        }

        public void onModemInfoReadCompleted(int item, int intValue, String value) {
            ImsLog.d(mSlotId, "onModemInfoReadCompleted :: item=" + item
                    + ", intValue=" + intValue + ", value=" + value);

            if (item == 1/*MODEM_I_ECC_PRIORITY*/) {
                processModemECCPriority(intValue);
            }
        }

        public void onEfRecordReadCompleted(int efId, String data) {
            ImsLog.d(mSlotId, "onEfRecordReadCompleted :: efId="
                    + Integer.toHexString(efId) + ", data=" + data);

            if (efId == 0x6FE5/*EF_PSI*/) {
                final String rawPsi = data;
                AppContext.runTask(() -> {
                        processPsiRecord(rawPsi);
                    }, 0);
            }
        }

        public void onEnvelopeMessageResponseReceived(Object envelope,
                int response, String extraInfo) {
            if (envelope == null) {
                return;
            }

            ImsLog.d(mSlotId, "onEnvelopeMessageResponseReceived :: envelope=" + envelope
                    + ", response=" + response + ", extraInfo=" + extraInfo);

            // USAT envelope only exists.
            processUSATEnvelopeResponse(0/*envelope.getTid()*/, response, extraInfo);
        }

        public void onDataLimitChanged(boolean dataEnabled) {
            ImsLog.d(mSlotId, "onDataLimitChanged :: dataEnabled=" + dataEnabled);

            processDataLimitControl(dataEnabled ? 1 : 0);
        }

        public void onHVoLtePreferenceChanged(Object preference) {
            ImsLog.d(mSlotId, "onHVoLtePreferenceChanged :: " + preference);

            if (preference != null) {
                processHVoLtePreference(preference);
            }
        }

        public void onModemRestarted() {
            ImsLog.i(mSlotId, "onModemRestarted");
        }
        // }

        @Override
        public void registerForImsPhoneConnectionRestarted(Handler h, int what, Object obj) {
            mSystemServiceRestartedRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForImsPhoneConnectionRestarted(Handler h) {
            mSystemServiceRestartedRegistrants.remove(h);
        }

        @Override
        public void registerForLteStateChanged(Handler h, int what, Object obj) {
            mLteStateChangedRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForLteStateChanged(Handler h) {
            mLteStateChangedRegistrants.remove(h);
        }

        @Override
        public void registerForModemECCPriority(Handler h, int what, Object obj) {
            mModemECCPriorityRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForModemECCPriority(Handler h) {
            mModemECCPriorityRegistrants.remove(h);
        }

        @Override
        public void registerForUSATEnvelopeResponse(Handler h, int what, Object obj) {
            mUSATEnvelopeResponseRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForUSATEnvelopeResponse(Handler h) {
            mUSATEnvelopeResponseRegistrants.remove(h);
        }

        @Override
        public void registerForWcdmaVoPSChanged(Handler h, int what, Object obj) {
            mWcdmaVoPSChangedRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForWcdmaVoPSChanged(Handler h) {
            mWcdmaVoPSChangedRegistrants.remove(h);
        }

        @Override
        public void registerForImsEmcSupportChanged(Handler h, int what, Object obj) {
            mImsEmcSupportChangedRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForImsEmcSupportChanged(Handler h) {
            mImsEmcSupportChangedRegistrants.remove(h);
        }

        @Override
        public void registerForAcBarringForEmergencyChanged(Handler h, int what, Object obj) {
            mAcBarringForEmegencyChangedRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForAcBarringForEmergencyChanged(Handler h) {
            mAcBarringForEmegencyChangedRegistrants.remove(h);
        }

        // Checks if the emergency attach is supported on the LTE network
        @Override
        public boolean isEmergencyAttachSupported() {
            return true;
        }

        // Checks if the emergency bearer service is supported on the LTE network
        @Override
        public boolean isEmergencyBearerServiceSupported() {
            IDCNetWatcher dcnw = (IDCNetWatcher)DCFactory.getDC(
                DCFactory.NETWORK_WATCHER, mSlotId);

            if (dcnw == null) {
                return false;
            }

            return dcnw.isEmergencyServiceSupported();
        }

        // Checks if the voice call is supported on the LTE network
        @Override
        public boolean isImsVoiceCallSupported() {
            IDCNetWatcher dcnw = (IDCNetWatcher)DCFactory.getDC(
                DCFactory.NETWORK_WATCHER, mSlotId);

            if (dcnw == null) {
                return false;
            }

            return dcnw.isVops();
        }

        @Override
        public int getPlmn2FromSIB1() {
            return 0;
        }

        @Override
        public int getCSCallState() {
            int callState = TelephonyManager.CALL_STATE_IDLE;

            ImsLog.d(mSlotId, "CallState(from Phone): state=" + callState);

            return callState;
        }

        @Override
        public int getCSCallStateInOtherSlot() {
            int otherSlotId = getOtherSlotId(mSlotId);
            int callState = TelephonyManager.CALL_STATE_IDLE;

            ImsLog.d(mSlotId, "CallState(from Phone): state="
                    + callState + ", slotId=" + otherSlotId);

            return callState;
        }

        @Override
        public void getEccPriority() {
        }

        @Override
        public int getSignalStrength() {
            return 0;
        }

        @Override
        public void setE911CallStateForGPS(int state, int reason) {
            // P-OS : Modem RAT selection is always supported.
            if (state != 1/*EMERGENCY_CALL_STOP_WITH_ECB*/) {
                // if modem supports RAT selection,
                // no need to notify except EMERGENCY_CALL_STOP_WITH_ECB
                return;
            }

            // setEmergencyCallStateForGps(state, reason);
        }

        @Override
        public void setPeerSimSuspend(boolean enable) {
            if (!MSimUtils.isMultiSimEnabled()) {
                return;
            }

            //setPeerSimSuspend(enable);
        }

        @Override
        public String getApn(String apnType) {
            return null;
        }

        @Override
        public void sendEnvelopeMessage(int id, String userinfo) {
            //sendEnvelope(envelope);
        }

        @Override
        public void setTrm(int type) {
            //setTrm(type);
        }

        @Override
        public int getPsiRecord() {
            //getEfRecord(0x6FE5/*EF_PSI*/, true);
            return ImsPhoneProxyApi.RESULT_NOK;
        }

        @Override
        public void clearNetworkInfo() {
            //requestNetworkInfo(true);
        }

        @Override
        public void requestNetworkInfo() {
            //requestNetworkInfo(false);
        }

        @Override
        public void sendEnvelope(String data) {
            //sendEnvelope(envelope);
        }

        @Override
        public void setImsRegistrationStatus(int state, int services, int detailState,
               int systemMode, int reason) {
            /*setImsRegistrationStatus(state,
                    services, detailState, systemMode, reason);*/
        }

        @Override
        public void setImsCallStatus(int wholeCallState, int id, int state, int reason,
                int type, int systemMode, int direction, int wholeCallStateEx,
                int emergency) {
            /*setImsCallStatus(wholeCallState, id, state, reason,
                    type, systemMode, direction, wholeCallStateEx,
                    emergency);*/
        }

        @Override
        public void setScmMode(int type, int state, int emergency) {
            //setScmMode(type, state, emergency);
        }

        @Override
        public List<CellInfo> getAllCellInfo() {
            //getAllCellInfo();
            TelephonyManager tm = AppContext.getTelephonyManager();
            return (tm != null) ? tm.getAllCellInfo() : null;
        }

        @Override
        public void updateImsConfig() {
            setImsConfig();
        }

        @Override
        public int isImsVoiceCallSupported4Sys() {
            return isImsVoiceCallSupported() ? 1 : 0;
        }

        private void processPsiRecord(String psi) {
            // no-op
        }

        private void processModemECCPriority(int param1) {
            Bundle bundle = new Bundle();
            bundle.putInt("param1", param1);

            mModemECCPriorityRegistrants.notifyResult(bundle);
        }

        private void processUSATEnvelopeResponse(int id, int response, String modifiedinfo) {
            Bundle bundle = new Bundle();
            bundle.putInt("id", id);
            bundle.putInt("response", response);
            bundle.putString("modifiedinfo", modifiedinfo);

            mUSATEnvelopeResponseRegistrants.notifyResult(bundle);
        }

        private void processImsRegistrationControl(int controlType) {
            ISystem system = SystemInterface.getInstance().getSystem(mSlotId);

            if (system != null) {
                system.notifyEvent(ImsEventDef.IMS_EVENT_REG_CONTROL, controlType, 0);
            }
        }

        private void processDataLimitControl(int dataEnabled) {
            ISystem system = SystemInterface.getInstance().getSystem(mSlotId);

            if (system != null) {
                system.notifyEvent(ImsEventDef.IMS_EVENT_MOBILE_DATA_LIMIT_CHANGED,
                        dataEnabled, 0);
            }
        }

        private void processHVoLtePreference(Object preference) {
            int systemMode = 0x08;//preference.getSysMode();
            int preferredService = 0x03;//preference.getPreferredServices();

            preprocImsPreferenceState(systemMode, preferredService);

            ISystem system = SystemInterface.getInstance().getSystem(mSlotId);

            if (system != null) {
                system.notifyEvent(ImsEventDef.IMS_EVENT_REG_PREF_STATE,
                        systemMode, preferredService);
            }
        }

        private void preprocImsPreferenceState(int systemMode, int regState) {
            // systemMode : LTE (8)
            // regState : VOIP or SMS+VOIP (1 or 3)
        }

        private void setDataToModem(int itemIndex, int param1, int param2, String paramEx) {
            if (param1 == ImsEventDef.IMS_EVENT_DATA_FLUSH_ENABLED) {
                //enableImsDataFlush();
            }
        }

        private boolean isImsConfigRequired() {
            return false;
        }

        private boolean isRoamingConditionCheckedForImsConfig() {
            if (ImsGlobal.isCountry(mSlotId, "CA")) {
                return false;
            }

            return true;
        }

        private void clearImsConfig() {
            if (!isImsConfigRequired()) {
                return;
            }

            /**
             * [0] : volteEnable
             * [1] : vilteEnable
             * [2] : vowifiEnable
             * [3] : viwifiEnable
             * [4] : smsEnable
             * [5] : eimsEnable
             * ([4] and [5] are enabled due to mtk modem behaviour)
             */
            int[] imsConfig = new int[] {0, 0, 0, 0, 1, 1};

            //setImsConfig(imsConfig);
        }

        private void setImsConfig() {
            if (!isImsConfigRequired()) {
                return;
            }

            /**
             * [0] : volteEnable
             * [1] : vilteEnable
             * [2] : vowifiEnable
             * [3] : viwifiEnable
             * [4] : smsEnable
             * [5] : eimsEnable
             * ([4] and [5] are enabled due to mtk modem behaviour)
             */
            int[] imsConfig = new int[6];
            boolean isVoLteEnabled = ImsGlobal.isVoLteEnabled(mContext, mSlotId)
                    || OperatorInfo.isSupportVoLteEmergency(mSlotId);
            imsConfig[0] = isVoLteEnabled ? 1 : 0;
            imsConfig[1] = (ImsGlobal.isVtEnabled(mContext, mSlotId)) ? 1 : 0;
            imsConfig[2] = (ImsGlobal.isWfcEnabled(mContext, mSlotId)) ? 1 : 0;
            imsConfig[3] = imsConfig[2];
            imsConfig[4] = 1;
            imsConfig[5] = 1;

            if (isRoamingConditionCheckedForImsConfig()) {
                IDCNetWatcher dcnw = (IDCNetWatcher)DCFactory.getDC(
                        DCFactory.NETWORK_WATCHER, mSlotId);

                if (dcnw != null && mContext != null && dcnw.isRoaming()) {
                    if (!CapabilityConfigs.isVoLteRoamingEnabled(mSlotId)) {
                        ImsLog.i(mSlotId, "volte is not supported in roaming");

                        for (int i = 0; i < imsConfig.length - 2; i++) {
                            imsConfig[i] = 0;
                        }
                    }
                }
            }

            ImsLog.i(mSlotId, "volteEnable=" + imsConfig[0] + ", vilteEnable=" + imsConfig[1]
                    + ", vowifiEnable=" + imsConfig[2] + ", viwifiEnable=" + imsConfig[3]
                    + ", smsEnable=" + imsConfig[4] + ", eimsEnable=" + imsConfig[5]);

            //setImsConfig(imsConfig);
        }

        private class IIMSPhoneHandler extends Handler {
            public void handleMessage(Message msg) {
                if (msg == null) {
                    return;
                }

                ImsLog.i(mSlotId, "IIMSPhoneHandler :: what=" + msg.what);

                switch(msg.what) {
                    case ImsEventDef.IMS_EVENT_SEND_DATA_TO_MODEM: {
                        AsyncResult ar = (AsyncResult)msg.obj;

                        if (ar == null) {
                            return;
                        }

                        Message eventMsg = (Message)ar.result;

                        if (eventMsg == null) {
                            return;
                        }
                        setDataToModem(eventMsg.what, eventMsg.arg1, eventMsg.arg2, "");
                        break;
                    }

                    default:
                        break;
                }
            }
        }
    }
}
