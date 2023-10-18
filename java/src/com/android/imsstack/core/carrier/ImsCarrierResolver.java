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
package com.android.imsstack.core.carrier;

import android.text.TextUtils;

/**
 * This class provides the carrier identifier conversion from AOSP carrier-id to IMS operators.
 */
public final class ImsCarrierResolver {
    public static class Carrier {
        private final int mCarrierId;
        private final int mSpecificCarrierId;
        private String mOperator = "";
        private String mCountry = "";
        private String mOperatorSub = "";
        private boolean mTestSim = false;

        /* package */ Carrier(int carrierId, int specificCarrierId) {
            mCarrierId = carrierId;
            mSpecificCarrierId = specificCarrierId;
        }

        public int getCarrierId() {
            return mCarrierId;
        }

        public int getSpecificCarrierId() {
            return mSpecificCarrierId;
        }

        public String getOperator() {
            return mOperator;
        }

        public String getOperatorSub() {
            return mOperatorSub;
        }

        public String getCountry() {
            return mCountry;
        }

        public boolean isTestSim() {
            return mTestSim;
        }

        public boolean isValid() {
            return !TextUtils.isEmpty(mOperator);
        }

        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();

            sb.append("[ Carrier :: cid=(");
            sb.append(mCarrierId);
            sb.append("/");
            sb.append(mSpecificCarrierId);
            sb.append("), opco=(");
            sb.append(mOperator);
            sb.append("-");
            sb.append(mCountry);
            sb.append("/");
            sb.append(mOperatorSub);
            sb.append("), test=");
            sb.append(mTestSim);
            sb.append(" ]");

            return sb.toString();
        }

        /* package */ void setOperator(String operator) {
            mOperator = operator;
        }

        /* package */ void setOperatorSub(String operatorSub) {
            mOperatorSub = operatorSub;
        }

        /* package */ void setCountry(String country) {
            mCountry = country;
        }

        /* package */ void setTestSim(boolean testSim) {
            mTestSim = testSim;
        }
    }

    /**
     * Returns the Carrier object that contains the operator/country information as string
     * from the given SimCarrierId.
     */
    public static Carrier getCarrierFromCarrierId(SimCarrierId scid) {
        Carrier carrier = new Carrier(scid.getCarrierId(), scid.getSpecificCarrierId());

        if (setTestSim(carrier, scid.getCarrierId())) {
            carrier.setCountry("COM");
        } else if (setOperatorForKR(carrier, scid.getCarrierId())) {
            carrier.setCountry("KR");
        } else if (setOperatorForJP(carrier, scid.getCarrierId())) {
            carrier.setCountry("JP");
        } else if (setOperatorForCA(carrier, scid.getCarrierId())) {
            carrier.setCountry("CA");
        } else if (setOperatorForUS(carrier, scid.getCarrierId(), scid.getSpecificCarrierId())) {
            carrier.setCountry("US");
        } else {
            setOperator(carrier, scid.getCarrierId(), scid.getSpecificCarrierId());
        }

        return carrier;
    }

    private static boolean setOperatorForKR(Carrier carrier, int carrierId) {
        switch (carrierId) {
            case 1890: // KT
                carrier.setOperator("KT");
                break;
            case 1891: // SK Telecom
                carrier.setOperator("SKT");
                break;
            case 1892: // LG U+
                carrier.setOperator("LGU");
                break;
            default:
                return false;
        }

        return true;
    }

    private static boolean setOperatorForJP(Carrier carrier, int carrierId) {
        switch (carrierId) {
            case 850: // NTT DOCOMO
                carrier.setOperator("DCM");
                break;
            case 1581: // au
                carrier.setOperator("KDDI");
                break;
            case 1894: // SoftBank
                carrier.setOperator("SBM");
                break;
            default:
                return false;
        }

        return true;
    }

    private static boolean setOperatorForUS(Carrier carrier, int carrierId, int specificCarrierId) {
        switch (carrierId) {
            case 1: // T-Mobile - US
                carrier.setOperator("TMO");
                break;
            case 1187: // AT&T
                carrier.setOperator("ATT");
                break;
            case 1779: // Cricket Wireless
                carrier.setOperator("CRK");
                break;
            case 1839: // Verizon Wireless, FALL-THROUGH
            case 1246: // Alltel Communications Inc. - 311270
                carrier.setOperator("VZW");
                break;
            case 1952: // U.S. Cellular
                carrier.setOperator("USC");
                break;
            case 1989: { // Google Fi
                switch (specificCarrierId) {
                    case 10014: // Google Fi-TMO
                        carrier.setOperator("TMO");
                        break;
                    case 10015: // Google Fi-Sprint
                        carrier.setOperator("SPR");
                        break;
                    case 10016: // Google Fi-USCellular
                        carrier.setOperator("USC");
                        break;
                    /*
                    case 10017: // Google Fi-3UK
                    case 10018: // Google Fi-3HK
                    case 10019: // Google Fi-3AT
                    */
                    default:
                        break;
                }
                break;
            }
            case 2119: // FirstNet
                carrier.setOperator("ATT");
                break;
            case 2256: // TelAlaska Cellular
                carrier.setOperator("CCA");
                break;
            default:
                return false;
        }

        return true;
    }

    private static boolean setOperatorForCA(Carrier carrier, int carrierId) {
        switch (carrierId) {
            case 576: // Bell Mobility
                carrier.setOperator("BELL");
                break;
            case 578: // MTS Mobility
                carrier.setOperator("BELL");
                carrier.setOperatorSub("MTS");
                break;
            case 580: // Sask Tel Mobility
                carrier.setOperator("BELL");
                carrier.setOperatorSub("SKC");
                break;
            case 1403: // Rogers
                carrier.setOperator("RGS");
                break;
            case 1404: // TELUS Mobility
                carrier.setOperator("TLS");
                break;
            case 1895: // Freedom Mobile
                carrier.setOperator("VTR");
                carrier.setOperatorSub("FRD");
                break;
            case 2008: // Videotron
                carrier.setOperator("VTR");
                break;
            default:
                return false;
        }

        return true;
    }

    private static void setOperator(Carrier carrier, int carrierId, int specificCarrierId) {
        if (carrierId == SimCarrierId.UNKNOWN_ID) {
            carrier.setOperator("OPEN");
            carrier.setCountry("COM");
        } else {
            carrier.setOperator(String.valueOf(carrierId));
            carrier.setCountry(String.valueOf(specificCarrierId));
        }
    }

    private static boolean setTestSim(Carrier carrier, int carrierId) {
        switch (carrierId) {
            case 1911: // Test Network, Used by GSM test equipment - 00101
                carrier.setTestSim(true);
                carrier.setOperator("TEST");
                break;
            default:
                return false;
        }

        return true;
    }

    private ImsCarrierResolver() {}
}
