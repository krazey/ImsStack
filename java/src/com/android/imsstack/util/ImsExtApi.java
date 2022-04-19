package com.android.imsstack.util;

import android.content.Context;
import android.content.res.Resources;
import android.os.DeadObjectException;
import android.os.Handler;
import android.os.RemoteException;
import android.telephony.CellSignalStrengthGsm;
import android.telephony.SignalStrength;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.uicc.IccUtils;

import java.util.List;
import java.util.NoSuchElementException;

/**
 * This class provides the IMS extended APIs to control VoLTE / VoWiFi / ViLTE.
 */
public final class ImsExtApi {
    private static final char[] HEX_CHARS = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };

    public static class Uicc {
        // UiccConstants.INTENT_VALUE_ICC_REMOVED
        public static final String SIM_REMOVED = "SIM_REMOVED";
        private static final String KEY_ECCLIST = "sim.ecclist";

        private static final String KEY_EF_UST = "ef_ust";

        // UiccConstants.EF_SMSP
        private static final int EF_SMSP = 0x6F42;

        public static synchronized boolean cacheEccList(int slotId) {
            initEccList();

            return true;
        }

        public static synchronized String getEccList(int slotId) {
            initEccList();

            return ""; // UiccManager.getProperty(KEY_ECCLIST, slotId, "");
        }

        public static String getEfSmsP() {
            return ""; // UiccManager.readIccRecordToString(EF_SMSP);
        }

        public static byte[] getEfUst(int slotId) {
            String efUst = "0"; // UiccManager.getProperty(KEY_EF_UST, slotId, "0");

            if ("0".equals(efUst)) {
                return null;
            }

            return hexStringToBytes(efUst);
        }

        public static String getSimState(int slotId) {
            int simState = SubscriptionManager.getSimStateForSlotIndex(slotId);

            switch (simState) {
            case TelephonyManager.SIM_STATE_ABSENT:
                return IccCardConstants.INTENT_VALUE_ICC_ABSENT;
            case TelephonyManager.SIM_STATE_NOT_READY:
                return IccCardConstants.INTENT_VALUE_ICC_NOT_READY;
            case TelephonyManager.SIM_STATE_LOADED:
                return IccCardConstants.INTENT_VALUE_ICC_LOADED;
            case TelephonyManager.SIM_STATE_READY:
                return IccCardConstants.INTENT_VALUE_ICC_READY;
            case TelephonyManager.SIM_STATE_PIN_REQUIRED: // FALL-THROUGH
            case TelephonyManager.SIM_STATE_PUK_REQUIRED: // FALL-THROUGH
            case TelephonyManager.SIM_STATE_NETWORK_LOCKED: // FALL-THROUGH
            case TelephonyManager.SIM_STATE_PERM_DISABLED:
                return IccCardConstants.INTENT_VALUE_ICC_LOCKED;
            case TelephonyManager.SIM_STATE_CARD_IO_ERROR:
                return IccCardConstants.INTENT_VALUE_ICC_CARD_IO_ERROR;
            case TelephonyManager.SIM_STATE_CARD_RESTRICTED:
                return IccCardConstants.INTENT_VALUE_ICC_CARD_RESTRICTED;
            default:
                return IccCardConstants.INTENT_VALUE_ICC_UNKNOWN;
            }
        }

        public static String bytesToHexString(byte[] bytes) {
            StringBuilder ret = new StringBuilder(2 * bytes.length);

            for (int i = 0; i < bytes.length; i++) {
                int b;
                b = 0x0f & (bytes[i] >> 4);
                ret.append(HEX_CHARS[b]);
                b = 0x0f & bytes[i];
                ret.append(HEX_CHARS[b]);
            }

            return ret.toString();
        }

        public static byte[] hexStringToBytes(String s) {
            byte[] ret;

            if (s == null) return null;
            int sz = s.length();
            ret = new byte[sz / 2];
            for (int i = 0; i < sz; i += 2) {
                ret[i / 2] = (byte) ((hexCharToInt(s.charAt(i)) << 4) |
                        hexCharToInt(s.charAt(i + 1)));
            }

            return ret;
        }

        public static int hexCharToInt(char c) {
            if (c >= '0' && c <= '9') return (c - '0');
            if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
            if (c >= 'a' && c <= 'f') return (c - 'a' + 10);

            throw new RuntimeException("invalid hex char '" + c + "'");
        }

        private static void initEccList() {
            // IMS-JUMP : CACHE_ECC
        }
    }

    public static class PhoneState {
        public static int getGsmDbm_DRA(SignalStrength ss) {
            List<CellSignalStrengthGsm> signalStrengthGsm =
                    ss.getCellSignalStrengths(CellSignalStrengthGsm.class);

            if (!signalStrengthGsm.isEmpty()) {
                return signalStrengthGsm.get(0).getDbm();
            }

            return Integer.MAX_VALUE;
        }
    }

    public static class TouchService {
        public static final int CALL_STATE_IDLE = 0; // CallStatus.IDLE; // 0
        public static final int CALL_STATE_RINGING = 5; // CallStatus.LTE_RINGING; // 5
        public static final int CALL_STATE_OFFHOOK = 6; // CallStatus.LTE_OFFHOOK; // 6

        public TouchService() {
        }

        public void connect() {
        }

        public void disconnect() {
        }

        public void setCallState(int state) {
        }
    }
}
