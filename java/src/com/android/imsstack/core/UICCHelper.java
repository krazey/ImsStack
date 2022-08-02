package com.android.imsstack.core;

import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.SubsInfoInterface;
import com.android.imsstack.core.agents.agentif.IPreference;
import com.android.imsstack.enabler.mtc.CallReasonInfo;
import com.android.imsstack.test.ImsTestMode;
import com.android.imsstack.util.ImsLog;

import java.io.ByteArrayOutputStream;
import java.util.List;

public class UICCHelper {
    // Constants--------------------------------------------------
    private static final int IMS_REG_NOTIFY_STATE_ACTIVE = 1;
    private static final int IMS_REG_NOTIFY_STATE_INVALID = 2;
    private static final int REG_SUCCESS_RESP = 200;
    private static final int IMPU_LENGTH_MINIMUM = 15;

    // PREFIX
    private static final byte PREFIX = (byte)0x80;

    // TAG
    private static final byte EVENT_DOWNLOAD_TAG = (byte)0xD6;

    // EVENT LIST
    private static final byte EVENT_LIST = (byte)0x19;
    private static final byte CALL_CONNECTED = (byte)0x01;
    private static final byte CALL_DISCONNECTED = (byte)0x02;
    private static final byte IMS_REGISTRATION_EVENT = (byte)0x17;

    // DEVICE IDENTITIES
    private static final byte DEVICE_IDENTITIES = (byte)0x02;
    private static final byte DEVICE_ID_UICC = (byte)0x81;
    private static final byte DEVICE_ID_TERMINAL = (byte)0x82;
    private static final byte DEVICE_ID_NETWORK = (byte)0x83;

    // TRANSACTION IDENTIFIER
    private static final byte TRANSACTION_IDENTIFIER = (byte)0x1C;

    // IMS CALL DISCONNECTION CAUSE
    private static final byte IMS_CALL_DISCONNECTION_CAUSE = (byte)0x55;

    // IMPU_LIST
    private static final byte IMPU_LIST = (byte)0x77;
    // URI_TLV
    private static final byte URI_TLV = (byte)0x80;
    // IMS_STATUS_CODE
    private static final byte IMS_STATUS_CODE = (byte)0x78;

    public static String getStringForIMSRegEvent(int regRespCode, int nSlotID, int reason) {
        ImsLog.d("IMS Reg Status code = " + regRespCode);

        if (regRespCode <= 0) {
            return null;
        }

        if ((ImsTestMode.getInstance().getTestMode(nSlotID).getExtraTestmask() &
                ImsTestMode.TEST_MASK_IMS_STATUS_TO_UICC_OFF) > 0) {
            return null;
        }

        SubsInfoInterface subsInfo = AgentFactory.getInstance().getAgent(
                SubsInfoInterface.class, nSlotID);
        if (subsInfo != null) {
            if (!subsInfo.isIsimEnabled()) {
                return null;
            }
        }

        // Int to String
        String intStatusCodeToString = String.valueOf(regRespCode);

        // ASCII Array
        char[] aSCIIStatusCode = intStatusCodeToString.toCharArray();

        // impu_list or status_code
        byte addInfoLength = 0x00;

        String impu = null;

        if (REG_SUCCESS_RESP == regRespCode) {
            if (reason == IMS_REG_NOTIFY_STATE_INVALID) {
                addInfoLength = 0x0;
            } else {
                if (ImsGlobal.isOperator(nSlotID, "ATT")) {
                    impu = getContactUriFromNotify(nSlotID);
                } else {
                    impu = getSelectedImpu(nSlotID);
                }

                if (impu == null) {
                    ImsLog.w("impu is null");
                    return null;
                }

                addInfoLength = (byte)impu.length();
            }

            ImsLog.d("IMPU = " + impu + " , LENGTH = " + addInfoLength);
        } else {
            addInfoLength = (byte)intStatusCodeToString.length();
            ImsLog.d("STATUS CODE LENGTH = " + addInfoLength);
        }

        //write ByteArray
        ByteArrayOutputStream objImsStatus = new ByteArrayOutputStream();
        objImsStatus.write(EVENT_DOWNLOAD_TAG);

        if (REG_SUCCESS_RESP == regRespCode) {
            objImsStatus.write(0x0B + addInfoLength); // Length A + B + C(IMPU)
        } else {
            objImsStatus.write(0x09 + addInfoLength); // Length A + B + D(Status Code)
        }

        //A: IMS REGISTRATION EVENT
        objImsStatus.write(EVENT_LIST); //0x19 EVENT LIST = 0x19
        objImsStatus.write(0x01); // length
        objImsStatus.write(IMS_REGISTRATION_EVENT);

        //B: Device identities
        objImsStatus.write(DEVICE_IDENTITIES);
        objImsStatus.write(0x02); // length
        objImsStatus.write(DEVICE_ID_NETWORK);  // Src-Network
        objImsStatus.write(DEVICE_ID_UICC);  // Dst-UICC

        if (REG_SUCCESS_RESP == regRespCode) {
            //C : IMPU List
            objImsStatus.write(PREFIX | IMPU_LIST); // 0x80 | 0x77
            objImsStatus.write(0x02 + addInfoLength); // length

            objImsStatus.write(URI_TLV); // URI_TLV tag = 0x80
            objImsStatus.write(addInfoLength); // URI TLV length

            if (reason == IMS_REG_NOTIFY_STATE_ACTIVE) {
                byte[] aImpu = impu.getBytes();
                for (int i = 0; i < aImpu.length; i++) {
                    objImsStatus.write(aImpu[i]);
                }
            }
        } else {
            //D: IMS Status Code
            objImsStatus.write(IMS_STATUS_CODE); // 0x80 | 0x78
            objImsStatus.write(addInfoLength);

            for (int i = 0; i < intStatusCodeToString.length(); i++) {
                if (aSCIIStatusCode[i] != 0) {
                    objImsStatus.write(aSCIIStatusCode[i]);
                }
            }
        }

        byte[] aImsStatus = objImsStatus.toByteArray();

        // LOG OFF
        /*
        ImsLog.d("IMS STATUS : ");
        for (int i = 0; i < aImsStatus.length; i++) {
            ImsLog.i(TAG, " " + i + " " + aImsStatus[i] + " " + String.format("0x%02X ", aImsStatus[i]));
        }
        */

        StringBuilder strImsStatus = new StringBuilder(2 * aImsStatus.length);

        //from IccUtils.java - bytesToHexString(byte[] bytes)
        for (int i = 0; i < aImsStatus.length; i++) {
            int b;
            b = 0x0f & (aImsStatus[i] >> 4);
            strImsStatus.append("0123456789abcdef".charAt(b));
            b = 0x0f & aImsStatus[i];
            strImsStatus.append("0123456789abcdef".charAt(b));
        }

        return strImsStatus.toString();
    }

    public static String getContactUriFromNotify(int nSlotID) {
        final String strREG_NOTIFY_STATUS = "reg_notify_info";
        final String strCONTACT_URI = "contact_uri";
        String strUriFromNotify = null;

        IPreference pfa = (IPreference)AgentFactory.getAgent(AgentFactory.PREFERENCE, nSlotID);
        if (pfa != null) {
            strUriFromNotify = pfa.getPreferenceStrValue(strREG_NOTIFY_STATUS, strCONTACT_URI, nSlotID);
        }

        return (strUriFromNotify == null) ? "" : strUriFromNotify;
    }

    private static String getSelectedImpu(int nSlotID) {
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, nSlotID);

        if (sim == null) {
            ImsLog.d("SimInterface is null");
            return null;
        }

        List<String> impuList = sim.getIsimImpu();

        if (impuList.isEmpty()) {
            return null;
        }

        ImsLog.w("IMPUs Size = " + impuList.size());

        String impu = impuList.get(0);

        if ((impu == null) || (impu.length() < IMPU_LENGTH_MINIMUM)) {
            return null;
        }

        if (impuList.size() == 1) {
            return impu;
        }

        impu = impuList.get(1);

        if ((impu == null) || (impu.length() < IMPU_LENGTH_MINIMUM)) {
            ImsLog.w("impus[1] is invalid");
            return null;
        }

        return impu;
    }

    public static String getStringForCallConnected(int callConnectionId, boolean isMO) {
        ByteArrayOutputStream objImsStatus = new ByteArrayOutputStream();

        // Event download tag
        objImsStatus.write(EVENT_DOWNLOAD_TAG);
        // Length (A+B+C)
        objImsStatus.write(0x0A); // (3+4+3)

        // A : Event list
        objImsStatus.write(EVENT_LIST); // Event list tag
        objImsStatus.write(0x01); // Length
        objImsStatus.write(CALL_CONNECTED); // CALL_CONNECTED

        // B : Device identities
        objImsStatus.write(DEVICE_IDENTITIES); // Device identities tag
        objImsStatus.write(0x02); // Length
        if (isMO) {
            objImsStatus.write(DEVICE_ID_NETWORK);  // Source device identity
            objImsStatus.write(DEVICE_ID_UICC);  // Destination device identity
        } else {
            objImsStatus.write(DEVICE_ID_TERMINAL);  // Source device identity
            objImsStatus.write(DEVICE_ID_UICC);  // Destination device identity
        }

        // C : Transaction identifier (3GPP TS 31.111)
        objImsStatus.write(TRANSACTION_IDENTIFIER); // Transaction identifier tag
        objImsStatus.write(0x01); // Length
        // bits 1 to 4 = RFU; (Reserved for Future Use)
        // bits 5 to 7 = TI value;
        // - TI value is an identifier generated by the terminal to uniquely identify the call,
        //   regardless of the bearer of the call.
        // bit 8 = TI flag.
        // - TI flag is:
        // - Call connected event: "1"
        byte identifier = (byte)(callConnectionId);
        identifier = (byte)(identifier << 4);
        identifier = (byte)(identifier | 0x80);
        objImsStatus.write(identifier);

        return getHexString(objImsStatus);
    }

    /**
    * gets string that has information of call disconnection.
    */
    public static String getStringForCallDisconnected(int callConnectionId, int reason) {
        ByteArrayOutputStream objImsStatus = new ByteArrayOutputStream();

        // Event download tag
        objImsStatus.write(EVENT_DOWNLOAD_TAG);
        // Length (A+B+C+D)
        if ((reason >= CallReasonInfo.CODE_LOCAL_NETWORK_NO_SERVICE)
                && (reason <= CallReasonInfo.CODE_WIFI_LOST)) {
            objImsStatus.write(0x0C); // Length (3+4+3+2)
        } else {
            objImsStatus.write(0x0D); // Length (3+4+3+3)
        }

        // A : Event list
        objImsStatus.write(EVENT_LIST); // Event list tag
        objImsStatus.write(0x01); // Length
        objImsStatus.write(CALL_DISCONNECTED); // CALL_CONNECTED

        // B : Device identities
        objImsStatus.write(DEVICE_IDENTITIES); // Device identities tag
        objImsStatus.write(0x02); // Length
        if (reason == CallReasonInfo.CODE_USER_TERMINATED) { // far end
            objImsStatus.write(DEVICE_ID_NETWORK);  // Source device identity
            objImsStatus.write(DEVICE_ID_UICC);  // Destination device identity
        } else { // near end
            objImsStatus.write(DEVICE_ID_TERMINAL);  // Source device identity
            objImsStatus.write(DEVICE_ID_UICC);  // Destination device identity
        }

        // C : Transaction identifier (3GPP TS 31.111)
        objImsStatus.write(TRANSACTION_IDENTIFIER); // Transaction identifier tag
        objImsStatus.write(0x01); // Length
        // bits 1 to 4 = RFU; (Reserved for Future Use)
        // bits 5 to 7 = TI value;
        // - TI value is an identifier generated by the terminal to uniquely identify the call,
        //   regardless of the bearer of the call.
        // bit 8 = TI flag.
        // - TI flag is:
        // - Call disconnected event: "0" if caller disconnects the call, "1" otherwise
        byte identifier = (byte)(callConnectionId);
        identifier = (byte)(identifier << 4);
        if (reason == CallReasonInfo.CODE_USER_TERMINATED) {
            identifier = (byte)(identifier & 0x7F);
        } else {
            identifier = (byte)(identifier | 0x80);
        }
        objImsStatus.write(identifier);

        // D : IMS call disconnection cause
        objImsStatus.write(IMS_CALL_DISCONNECTION_CAUSE); // IMS call disconnection cause tag
        if ((reason >= CallReasonInfo.CODE_LOCAL_NETWORK_NO_SERVICE)
                && (reason <= CallReasonInfo.CODE_WIFI_LOST)) {
            objImsStatus.write(0x00); // Length
        } else {
            objImsStatus.write(0x01); // Length
            objImsStatus.write(0x10); // Cause No.16 "normal call clearing"
        }

        return getHexString(objImsStatus);
    }

    private static String getHexString(ByteArrayOutputStream objImsStatus) {
        if (objImsStatus == null) {
            return null;
        }

        byte[] aImsStatus = objImsStatus.toByteArray();
        StringBuilder strImsStatus = new StringBuilder(2 * aImsStatus.length);

        //from IccUtils.java - bytesToHexString(byte[] bytes)
        for (int i = 0; i < aImsStatus.length; i++) {
            int b = 0x0f & (aImsStatus[i] >> 4);
            strImsStatus.append("0123456789abcdef".charAt(b));
            b = 0x0f & aImsStatus[i];
            strImsStatus.append("0123456789abcdef".charAt(b));
        }

        return strImsStatus.toString();
    }
}
