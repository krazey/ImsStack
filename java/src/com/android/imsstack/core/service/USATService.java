package com.android.imsstack.core.service;

import android.content.Context;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;

import com.android.imsstack.R;
import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.core.UICCHelper;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.agentif.IIMSPhoneAgent;
import com.android.imsstack.core.agents.agentif.ITelephonySubscriber;
import com.android.imsstack.core.agents.dcm.DCFactory;
import com.android.imsstack.core.agents.dcmif.IDCUtil;
import com.android.imsstack.core.service.serviceif.IUSATService;
import com.android.imsstack.core.service.serviceif.IVoLteService;
import com.android.imsstack.enabler.mtc.Call;
import com.android.imsstack.enabler.mtc.CallStateListener;
import com.android.imsstack.enabler.mtc.CallTracker;
import com.android.imsstack.enabler.mtc.FailInfo;
import com.android.imsstack.enabler.mtc.IUMtcCall;
import com.android.imsstack.enabler.mtc.MtcCall;
import com.android.imsstack.internal.imsservice.CallUtils;
import com.android.imsstack.util.ImsConstants;
import com.android.imsstack.util.ImsExtApi;
import com.android.imsstack.util.ImsLog;
import com.android.internal.telephony.PhoneConstants;

import java.io.ByteArrayOutputStream;
import java.util.ArrayList;

public class USATService implements IUSATService {
    /** Checks if STK call setup supports or not */
    private static final boolean STK_CALL_SETUP_AVAILABLE = !ImsConstants.USE_GOOGLE_NATIVE_APPS;
    private static final String UICC_SETUP_REPORT = "CALL_COMMAND_RESPONSE";
    /** Setup result */
    private static final int ACTIVE = 1;
    private static final int DISCONNECT = 2;
    /** Call disconnect reason */
    private static final int DISCONNECT_REASON_UNKNOWN
            = IUSATService.DISCONNECT_REASON_UNKNOWN;
    private static final int DISCONNECT_REASON_USER
            = IUSATService.DISCONNECT_REASON_USER;
    private static final int DISCONNECT_REASON_NW
            = IUSATService.DISCONNECT_REASON_NW;
    private static final int DISCONNECT_REASON_HOLD_FAIL
            = IUSATService.DISCONNECT_REASON_HOLD_FAIL;
    private static final int DISCONNECT_REASON_TERMINATED_FAIL
            = IUSATService.DISCONNECT_REASON_TERMINATED_FAIL;
    /** Envelope state */
    private static final int STATE_IDLE = 0;
    private static final int STATE_PENDING = 1;
    /** Call control tag */
    private static final int ENVELOPE_CALL_CONTROL_TAG = 0xD4;
    /** Device id */
    private static final int ENVELOPE_DEVICE_ID_PARAM_TAG = 0x02;
    private static final int ENVELOPE_DEVICE_ID_DATA_UICC = 0x81;
    private static final int ENVELOPE_DEVICE_ID_DATA_ME = 0x82;
    /** Number ttype */
    private static final int ENVELOPE_ADDRESS_TON_UNKNOWN = 0x81;
    private static final int ENVELOPE_ADDRESS_TON_INTERNATIONAL = 0x91;
    private static final int ENVELOPE_ADDRESS_PARAM_TAG = 0x06;
    /** Location info */
    private static final int ENVELOPE_LOCATION_INFO_PARAM_TAG = 0x13;
    /** Envelope response */
    private static final int ENVELOPE_RESPONSE_ALLOWED = 0x00;
    private static final int ENVELOPE_RESPONSE_NOT_ALLOWED = 0x01;
    private static final int ENVELOPE_RESPONSE_MODIFIED = 0x02;
    /** Internal events */
    private static final int EVENT_CALL_CONTROL_ENVELOPE_RESPONSE = 1001;
    private static final int EVENT_IMS_PHONE_RESTARTED = 1002;
    private static final int EVENT_NUMBER_FORMAT_EXCEPTION = 1003;
    /** Max transacion id */
    private static final int MAX_TRANSACTION_ID = 0XFF;

    private IVoLteService mVoLteService = null;
    private USATCallStateListener mCallListener = null;
    private int mCallEventState = 0;
    // Checks if feature for call control by USIM is enabled or not
    private boolean mFeatureEnabledForCallControlByUSIM = false;
    // Checks if USIM supports call control by USIM (EF_UST)
    private boolean mCallControlByUSIMAvailable = false;
    private ArrayList<Transaction> mTransactions = new ArrayList<Transaction>();
    private USATHelperHandler mUSATHelperHandler = new USATHelperHandler();
    private int mTransactionIdForUniqueness = 1;
    private int mState = STATE_IDLE;
    private boolean mSupportedCallStatusEnvelopeMessage = false;

    public USATService() {
    }

    @Override
    public boolean start(IVoLteService voLteService) {
        mVoLteService = voLteService;

        ImsLog.i(getSlotId(), "");

        if (STK_CALL_SETUP_AVAILABLE) {
            mCallListener = new USATCallStateListener();
            CallUtils.addCallStateListener(getSlotId(), mCallListener);
        }

        IIMSPhoneAgent ip = (IIMSPhoneAgent)AgentFactory.getAgent(
                AgentFactory.IMS_PHONE, getSlotId());

        if (ip != null) {
            ip.registerForUSATEnvelopeResponse(mUSATHelperHandler,
                    EVENT_CALL_CONTROL_ENVELOPE_RESPONSE, null);
            ip.registerForImsPhoneConnectionRestarted(mUSATHelperHandler,
                    EVENT_IMS_PHONE_RESTARTED, null);
        }

        startInternal();

        return true;
    }

    @Override
    public void cleanup(Context context) {
        ImsLog.i(getSlotId(), "");

        if (mCallListener != null) {
            CallUtils.removeCallStateListener(getSlotId(), mCallListener);
            mCallListener = null;
        }

        IIMSPhoneAgent ip = (IIMSPhoneAgent)AgentFactory.getAgent(
                AgentFactory.IMS_PHONE, getSlotId());

        if (ip != null) {
            ip.unregisterForUSATEnvelopeResponse(mUSATHelperHandler);
            ip.unregisterForImsPhoneConnectionRestarted(mUSATHelperHandler);
        }

        notifyNotAllowedForAllTransactions();

        mUSATHelperHandler.removeCallbacksAndMessages(null);

        mFeatureEnabledForCallControlByUSIM = false;
        mCallControlByUSIMAvailable = false;

        mSupportedCallStatusEnvelopeMessage = false;
    }

    @Override
    public void update(Context context) {
    }

    @Override
    public void sendActiveCallStateToUICC(int reason) {
        ImsLog.d(getSlotId(), "state=" + mCallEventState + ", reason=" + reason);

        if (mCallEventState == CallTracker.CALL_EVENT_ESTABLISHED) {
            if (reason == DISCONNECT_REASON_HOLD_FAIL) {
                sendIntentToUICC(DISCONNECT, DISCONNECT_REASON_HOLD_FAIL);
            }

            if (reason == DISCONNECT_REASON_TERMINATED_FAIL) {
                sendIntentToUICC(DISCONNECT, DISCONNECT_REASON_TERMINATED_FAIL);
            }
        }
    }

    @Override
    public boolean isUICCSetupCall() {
        //check setup call by UICC
        String isUICCsetup = "no";

        if ("yes".equalsIgnoreCase(isUICCsetup)) {
            ImsLog.i(getSlotId(), "setup call by UICC");
            return true;
        }

        return false;
    }

    @Override
    public boolean isUSATSupported() {
        return mCallControlByUSIMAvailable;
    }

    @Override
    public int isCallAllowedByUSAT(String targetNumber, USATListener listener) {
        if (targetNumber == null || listener == null) {
            ImsLog.e(getSlotId(), "targetNumber is null");
            return INVALID_ID;
        }

        int id = generateTransactionId();

        if (id < 0) {
            ImsLog.e(getSlotId(), "Transaction Id is invalid");
            return INVALID_ID;
        }

        Transaction t = new Transaction(listener, id, targetNumber);
        addTransaction(t);

        if (mState == STATE_IDLE){
            processEnvelopeMessage();
        }

        return id;
    }

    @Override
    public void abortTransaction(int transactionId) {
        removeTransaction(transactionId);
    }

    private boolean isFeatureEnabledForCallControlByUSIM() {
        return mFeatureEnabledForCallControlByUSIM;
    }

    private void startInternal() {
        mFeatureEnabledForCallControlByUSIM = false;
        mCallControlByUSIMAvailable = false;

        setAvailabilityForCallStatusEnvelopeMessage();

        setFeatureForCallControlByUSIM();

        ImsLog.i(getSlotId(), "Call control by USIM :: "
                + (isFeatureEnabledForCallControlByUSIM() ? "enabled" : "disabled"));

        if (!isFeatureEnabledForCallControlByUSIM()) {
            return;
        }

        setAvailabilityForCallControlByUSIM();
    }

    private void sendIntentToUICC(int setupResult, int reason) {
        if (isUICCSetupCall()) {
            Intent intent = new Intent(UICC_SETUP_REPORT);

            intent.addFlags(Intent.FLAG_INCLUDE_STOPPED_PACKAGES);
            intent.addFlags(Intent.FLAG_RECEIVER_INCLUDE_BACKGROUND);

            intent.putExtra("result", setupResult);
            intent.putExtra("reason", reason);
            intent.putExtra(PhoneConstants.SLOT_KEY, getSlotId());

            Context c = getContext();

            if (c != null) {
                c.sendBroadcast(intent);
            }
        }
    }

    private void setAvailabilityForCallStatusEnvelopeMessage() {
        mSupportedCallStatusEnvelopeMessage = false;

        if (ImsGlobal.isOperator(getSlotId(), "ATT")) {
            mSupportedCallStatusEnvelopeMessage = true;
        }
    }

    private boolean isSupportedCallStatusEnvelopeMessage() {
        return mSupportedCallStatusEnvelopeMessage;
    }

    private void sendCallStatusEnvelopeMessage(Call call, int callState) {
        if (isSupportedCallStatusEnvelopeMessage() == false || call == null) {
            return;
        }

        int callConnectionId = call.getCallExtraInt(Call.EXTRA_CALL_CONNECTION_ID, 0);
        String imsStatus = null;

        if (callState == CallTracker.CALL_EVENT_ESTABLISHED) {
            boolean isMO = call.isMO();
            imsStatus = UICCHelper.getStringForCallConnected(callConnectionId, isMO);
        } else if (callState == CallTracker.CALL_EVENT_TERMINATED && call.isOnceInCall()) {
            MtcCall mtcCall = (MtcCall)call;
            FailInfo failInfo = mtcCall.getTerminationReason();
            int failReason = (failInfo != null) ?
                    failInfo.Reason : IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_NONE;
            imsStatus = UICCHelper.getStringForCallDisconnected(callConnectionId, failReason);
        }

        IIMSPhoneAgent ip = (IIMSPhoneAgent)AgentFactory.getAgent(
            AgentFactory.IMS_PHONE, getSlotId());
        if (ip != null && imsStatus != null) {
            ip.sendEnvelope(imsStatus);
        }
    }

    private void setCallState(int callState) {
        if (mCallEventState != callState) {
            ImsLog.i(getSlotId(), "CallState :: " + mCallEventState + " >> " + callState);
            mCallEventState = callState;
        }
    }

    private void setAvailabilityForCallControlByUSIM() {
        byte[] efUst = ImsExtApi.Uicc.getEfUst(getSlotId());

        if ((efUst == null) || (efUst.length < 4)) {
            ImsLog.d(getSlotId(), "EF_UST value is not invalid");
            mCallControlByUSIMAvailable = false;
            return;
        }

        // Call control by USIM : 30th bit
        int maskInt = 32;

        if (((int)efUst[3] & maskInt) == maskInt) {
            mCallControlByUSIMAvailable = true;
            ImsLog.d(getSlotId(), "Call control by USIM is enabled in SIM");
        } else {
            mCallControlByUSIMAvailable = false;
        }
    }

    private void setFeatureForCallControlByUSIM() {
        Context c = getContext();

        if (c == null) {
            return;
        }

        String[] operators = c.getResources().getStringArray(
                R.array.config_operators_for_call_control_by_usim);

        ImsLog.d(getSlotId(), "Call control by USIM :: " + ImsLog.hiddenString(operators));

        if (operators != null && operators.length > 0) {
            // numeric: mccmnc, numeric: mcc, alpha: operator, alpha: operator-country
            ITelephonySubscriber ts = (ITelephonySubscriber)AgentFactory.getAgent(
                    AgentFactory.TELEPHONY_SUBSCRIBER, getSlotId());

            String numeric = (ts != null) ? ts.getMccMnc(true) : null;

            if (contains(operators, numeric)) {
                mFeatureEnabledForCallControlByUSIM = true;
                return;
            }

            String numericMcc = (ts != null) ? ts.getMcc(true) : null;

            if (contains(operators, numericMcc)) {
                mFeatureEnabledForCallControlByUSIM = true;
                return;
            }

            String operator = ImsGlobal.getOperator(getSlotId());

            if (contains(operators, operator)) {
                mFeatureEnabledForCallControlByUSIM = true;
                return;
            }

            String operatorCountry = operator + "-" + ImsGlobal.getCountry(getSlotId());

            if (contains(operators, operatorCountry)) {
                mFeatureEnabledForCallControlByUSIM = true;
                return;
            }
        }

        mFeatureEnabledForCallControlByUSIM = false;
    }

    private synchronized int generateTransactionId() {
        int allocatedId = (-1);
        int tid = mTransactionIdForUniqueness;
        Transaction t = null;

        while (true) {
            t = getTransaction(tid, false);

            if (t == null) {
                allocatedId = tid;
                break;
            }

            tid++;

            if (tid == MAX_TRANSACTION_ID) {
                tid = 1;
            } else if (tid == mTransactionIdForUniqueness) {
                ImsLog.e(getSlotId(), "There is no available transaction id");
                break;
            }
        }

        if (allocatedId > 0) {
            mTransactionIdForUniqueness = allocatedId + 1;

            if (mTransactionIdForUniqueness == MAX_TRANSACTION_ID) {
                mTransactionIdForUniqueness = 1;
            }
        }

        return allocatedId;
    }

    private void addTransaction(Transaction transaction) {
        synchronized (mTransactions) {
            mTransactions.add(transaction);
        }
    }

    private String makeEnvelopeMessageForCallControl(String targetNumber) {
        ByteArrayOutputStream buf = new ByteArrayOutputStream();

        //1. CALL_CONTROL_TAG
        int tag = ENVELOPE_CALL_CONTROL_TAG;
        buf.write(tag);

        int msg_length = 0x16;
        buf.write(msg_length);

        //2. DEVICE_INDENTIES(TERMINAL->UICC)
        tag = ENVELOPE_DEVICE_ID_PARAM_TAG; //Device Identities(0x82)
        buf.write(tag);

        msg_length = 0x02;
        buf.write(msg_length);

        buf.write(ENVELOPE_DEVICE_ID_DATA_ME);
        buf.write(ENVELOPE_DEVICE_ID_DATA_UICC);

        //3. ADDRESS_NUMBER(PHONE NUMBER)
        int typeOfNum = ENVELOPE_ADDRESS_TON_UNKNOWN; //TON(unknown : 0x81, international : 0x91)

        if (targetNumber.startsWith("+")) {
            targetNumber = targetNumber.substring(1);
            typeOfNum = ENVELOPE_ADDRESS_TON_INTERNATIONAL;
        }

        int[] changedNum = changeAddressFormat(targetNumber);

        if (changedNum == null) {
            ImsLog.i(getSlotId(), "Phone Number transformation is failed");
            return null;
        }

        tag = ENVELOPE_ADDRESS_PARAM_TAG; //Address(0x86)
        buf.write(tag);

        msg_length = changedNum.length + 1; //TON + Address
        buf.write(msg_length);
        buf.write(typeOfNum);

        for (int i = 0; i < changedNum.length; i++) {
            buf.write(changedNum[i]);
        }

        //4. LOCATION_INFORMATION(MCC/MNC/TAC/CellID)
        tag = ENVELOPE_LOCATION_INFO_PARAM_TAG; //Location Tag(0x13)
        buf.write(tag);

        int[] locationInfo = getLocationInfo(getSlotId());

        if (locationInfo == null) {
            msg_length = 0;
            buf.write(msg_length);
        } else {
            msg_length = locationInfo.length;
            buf.write(msg_length);

            for (int i = 0; i < locationInfo.length; i++) {
                buf.write(locationInfo[i]);
            }
        }

        byte[] rawData = buf.toByteArray();

        //5. WRITE_REAL_ENVELOPE_MESSAGE_LENGTH
        int len = rawData.length - 2; // minus (tag + length)
        rawData[1] = (byte) len;

        return ImsExtApi.Uicc.bytesToHexString(rawData);
    }

    private void sendEnvelopeMessage(int id, String envelopeMessage) {
        ImsLog.d(getSlotId(), "id=" + id + ", envelopeMessage=" + envelopeMessage);

        IIMSPhoneAgent ip = (IIMSPhoneAgent)AgentFactory.getAgent(
                AgentFactory.IMS_PHONE, getSlotId());

        if (ip != null) {
            ip.sendEnvelopeMessage(id, envelopeMessage);
        }
    }

    private void processEnvelopeMessage() {
        Transaction t = null;

        synchronized (mTransactions) {
            if (!mTransactions.isEmpty()) {
                t = mTransactions.get(0);
            }
        }

        if ((t != null) && (mState == STATE_IDLE)) {
            String envelopeMessage = makeEnvelopeMessageForCallControl(
                    t.getTargetPhoneNumber());

            if (envelopeMessage == null) {
                // To preserve returning isCallAllowedByUSAT() method
                mUSATHelperHandler.sendMessageDelayed(mUSATHelperHandler.obtainMessage(
                        EVENT_NUMBER_FORMAT_EXCEPTION, t.getId(), 0), 100);
                return;
            }

            sendEnvelopeMessage(t.getId(), envelopeMessage);
            setState(STATE_PENDING);
        }
    }

    private void notifyEnvelopeResponse(int id, int response, String modifiedInfo) {
        Transaction t = getTransaction(id, true);

        if (t != null) {
            t.notifyResult(response, modifiedInfo);
        }

        setState(STATE_IDLE);
        processEnvelopeMessage();
    }

    private void setState(int state) {
        if (mState != state) {
            ImsLog.d(getSlotId(), "setState :: " + mState + " >> " + state);
            mState = state;
        }
    }

    private Transaction getTransaction(int id, boolean remove) {
        synchronized (mTransactions) {
            for (int i = 0; i < mTransactions.size(); i++) {
                Transaction t = mTransactions.get(i);

                if (id == t.getId()) {
                    if (remove) {
                        mTransactions.remove(i);
                    }
                    return t;
                }
            }
        }

        return null;
    }

    private void removeTransaction(int transactionId) {
        synchronized (mTransactions) {
            if (mTransactions.isEmpty()) {
                return;
            }

            for (int i = 0; i < mTransactions.size(); i++) {
                Transaction t = mTransactions.get(i);

                if (t.getId() == transactionId) {
                    mTransactions.remove(i);
                    break;
                }
            }
        }
    }

    private void notifyNotAllowedForAllTransactions() {
        synchronized (mTransactions) {
            if (mTransactions.isEmpty()) {
                return;
            }

            for (Transaction t : mTransactions) {
                t.notifyResult(ENVELOPE_RESPONSE_NOT_ALLOWED, null);
            }

            mTransactions.clear();
        }

        setState(STATE_IDLE);
    }

    private static boolean contains(String[] stringArray, String value) {
        if (TextUtils.isEmpty(value)) {
            return false;
        }

        for (String item : stringArray) {
            if (value.equalsIgnoreCase(item)) {
                return true;
            }
        }

        return false;
    }

    private static int[] changeAddressFormat(String targetNumber) {
        if (targetNumber == null) {
            return null;
        }

        if (targetNumber.contains("*")) {
            targetNumber = targetNumber.replace("*", "A");
        }

        if (targetNumber.contains("#")) {
            targetNumber = targetNumber.replace("#", "B");
        }

        int[] changedNumberArray = changeStringToIntArray(reverseString(targetNumber));

        return changedNumberArray;
    }

    private static int[] getLocationInfo(int slotID) {
        IDCUtil dcutil = (IDCUtil)DCFactory.getDC(DCFactory.UTIL, slotID);

        if (dcutil == null) {
            return null;
        }

        IDCUtil.AccessNetworkInfo ani = dcutil.getAccessNetworkInfo(0);
        String[] accessNetworkInfo = (ani != null) ? ani.mANI : null;

        if (accessNetworkInfo == null) {
            return null;
        }

        if (accessNetworkInfo.length < 4 || TextUtils.isEmpty(accessNetworkInfo[0])
                || TextUtils.isEmpty(accessNetworkInfo[1]) || TextUtils.isEmpty(accessNetworkInfo[3])) {
            return null;
        }

        StringBuilder sb = new StringBuilder();

        // MCC, MNC
        sb.append(accessNetworkInfo[0]);

        if (accessNetworkInfo[1].length() == 2) {
            sb.append("F");
        }
        sb.append(accessNetworkInfo[1]);

        String locationInfo = reverseString(sb.toString());
        sb.setLength(0);
        sb.append(locationInfo);

        // TAC(LAC)
        if (accessNetworkInfo[3].length() < 4) {
            for (int i = 0; i < 4 - accessNetworkInfo[3].length(); i++) {
                sb.append("0");
            }
        }
        sb.append(accessNetworkInfo[3]);

        // Changed the Cell ID format(Add the padding characters)
        if (!TextUtils.isEmpty(accessNetworkInfo[2])) {
            if (accessNetworkInfo[2].length() < 7) {
                for (int i = 0; i < 7 - accessNetworkInfo[2].length(); i++) {
                    sb.append("0");
                }
            }
            sb.append(accessNetworkInfo[2]);
            sb.append("F");
        }

        int[] changedLocationInfoArray = changeStringToIntArray(sb.toString());

        if (changedLocationInfoArray == null) {
            return null;
        }

        return changedLocationInfoArray;
    }

    private static String reverseString(String str) {
        if (TextUtils.isEmpty(str)) {
            return null;
        }

        if ((str.length() % 2) != 0) {
            str = str + "F";
        }

        StringBuilder sb = new StringBuilder();

        sb.append(str);
        char tempChar = 0;

        for (int i = 0; i < sb.length(); i += 2) {
            tempChar = sb.charAt(i);
            sb.setCharAt(i, sb.charAt(i + 1));
            sb.setCharAt(i + 1, tempChar);
        }

        return sb.toString();
    }

    private static int[] changeStringToIntArray(String str) {
        if (TextUtils.isEmpty(str)) {
            return null;
        }

        int[] array = new int[str.length() / 2];
        int arrayIndex = 0;

        for (int i = 0; i < str.length(); i += 2) {
            try {
                array[arrayIndex] = Integer.parseInt(str.substring(i, i + 2), 16);
            } catch (Exception e) {
                ImsLog.e("Exception :: " + e.toString());
                return null;
            }

            arrayIndex++;
        }

        return array;
    }

    private Context getContext() {
        return (mVoLteService != null) ? mVoLteService.getContext() : null;
    }

    private int getSlotId() {
        return (mVoLteService != null) ? mVoLteService.getSlotID() : 0;
    }

    private class Transaction {
        private USATListener mListener = null;
        private int mId;
        private String mTargetPhoneNumber = null;

        public Transaction(USATListener listener, int id, String targetPhoneNumber) {
            mListener = listener;
            mId = id;
            mTargetPhoneNumber = targetPhoneNumber;
        }

        public int getId() {
            return mId;
        }

        public String getTargetPhoneNumber() {
            return mTargetPhoneNumber;
        }

        public void notifyResult(int response, String modifiedInfo) {
            if (mListener != null) {
                mListener.onNotifyCallAllowed(response, modifiedInfo);
            }
        }
    }

    public class USATHelperHandler extends Handler {
        public void handleMessage(Message msg) {
            if (msg == null) {
                return;
            }

            ImsLog.i(getSlotId(), "USATHelperHandler :: what=" + msg.what);

            switch (msg.what) {
                case EVENT_CALL_CONTROL_ENVELOPE_RESPONSE: {
                    if (msg.obj == null) {
                        break;
                    }

                    AsyncResult ar = (AsyncResult)msg.obj;

                    if (ar == null) {
                        break;
                    }

                    Bundle bundle = (Bundle)ar.result;
                    int id = bundle.getInt("id", -1);
                    int response = bundle.getInt("response", -1);
                    String modifiedInfo = bundle.getString("modifiedinfo", null);

                    ImsLog.d(getSlotId(), "response=" + response + ", modifiedInfo=" + modifiedInfo);
                    notifyEnvelopeResponse(id, response, modifiedInfo);
                    break;
                }

                case EVENT_IMS_PHONE_RESTARTED:
                    notifyNotAllowedForAllTransactions();
                    break;

                case EVENT_NUMBER_FORMAT_EXCEPTION:
                    notifyEnvelopeResponse(msg.arg1, ENVELOPE_RESPONSE_ALLOWED, null);
                    break;

                default:
                    break;
            }
        }
    }

    private class USATCallStateListener extends CallStateListener {
        public USATCallStateListener() {
        }

        @Override
        public void onCallCreated(Call call) {
            setCallState(CallTracker.CALL_EVENT_CREATE);
        }

        @Override
        public void onCallDestroyed(Call call) {
            setCallState(CallTracker.CALL_EVENT_DESTROY);
        }

        @Override
        public void onCallEstablishing(Call call) {
            setCallState(CallTracker.CALL_EVENT_ESTABLISHING);
        }

        @Override
        public void onCallRinging(Call call) {
            setCallState(CallTracker.CALL_EVENT_RINGING);
        }

        @Override
        public void onCallAccepted(Call call) {
            setCallState(CallTracker.CALL_EVENT_ACCEPT);
        }

        @Override
        public void onCallEstablished(Call call) {
            if (mCallEventState == CallTracker.CALL_EVENT_ESTABLISHING) {
                sendIntentToUICC(ACTIVE, -1);
            }

            setCallState(CallTracker.CALL_EVENT_ESTABLISHED);

            sendCallStatusEnvelopeMessage(call, CallTracker.CALL_EVENT_ESTABLISHED);
        }

        @Override
        public void onCallUpdated(Call call) {
            setCallState(CallTracker.CALL_EVENT_UPDATED);
        }

        @Override
        public void onCallTerminating(Call call) {
            if (mCallEventState == CallTracker.CALL_EVENT_ESTABLISHING) {
                sendIntentToUICC(DISCONNECT, DISCONNECT_REASON_USER);
            }

            setCallState(CallTracker.CALL_EVENT_TERMINATING);
        }

        @Override
        public void onCallTerminated(Call call) {
            if (mCallEventState == CallTracker.CALL_EVENT_ESTABLISHING) {
                sendIntentToUICC(DISCONNECT, DISCONNECT_REASON_NW);
            }

            setCallState(CallTracker.CALL_EVENT_TERMINATED);

            sendCallStatusEnvelopeMessage(call, CallTracker.CALL_EVENT_TERMINATED);
        }
    }
}
