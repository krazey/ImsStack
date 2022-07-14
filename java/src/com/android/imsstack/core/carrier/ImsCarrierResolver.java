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
        } else if (setOperator(carrier, scid.getCarrierId(), scid.getSpecificCarrierId())) {
            // no-op
        } else {
            carrier.setCountry("COM");
        }

        return carrier;
    }

    /**
     * Returns the Carrier object that contains the operator/country information as string
     * from the given information.
     */
    public static Carrier getCarrierFromCarrierId(int slotId, int subId,
            int carrierId, int specificCarrierId) {
        Carrier carrier = new Carrier(carrierId, specificCarrierId);

        if (setTestSim(carrier, carrierId)) {
            carrier.setCountry("COM");
        } else if (setOperatorForKR(carrier, carrierId)) {
            carrier.setCountry("KR");
        } else if (setOperatorForJP(carrier, carrierId)) {
            carrier.setCountry("JP");
        } else if (setOperatorForCA(carrier, carrierId)) {
            carrier.setCountry("CA");
        } else if (setOperatorForUS(carrier, carrierId, specificCarrierId)) {
            carrier.setCountry("US");
        } else if (setOperator(carrier, carrierId, specificCarrierId)) {
            // no-op
        } else {
            carrier.setCountry("COM");
        }

        return carrier;
    }

    private static boolean setOperatorForKR(Carrier carrier, int carrierId) {
        switch (carrierId) {
            case 1890: // KT
                carrier.setOperator("KT");
                break;
            case 1891: // SK Telecom, FALL-THROUGH
            case 2353: // SK Telink
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
            case 850: // NTT DOCOMO, FALL-THROUGH
            case 2116: // Ratel, FALL-THROUGH
            case 2407: // OCN MOBILE ONE, FALL-THROUGH
            case 10007: // docomo_mvno
                carrier.setOperator("DCM");
                break;
            case 1581: // au, FALL-THROUGH
            case 2110: // UQ mobile, FALL-THROUGH
            case 2148: // BIGLOBE
                carrier.setOperator("KDDI");
                break;
            case 1894: // SoftBank, FALL-THROUGH
            case 1917: // eMobile, FALL-THROUGH
            case 2100: // Y!mobile
                carrier.setOperator("SBM");
                break;
            /*
            case 2106: // IIJmio, DCM/KDDI MVNO
            case 2109: // Rakuten Mobile, DCM/KDDI MVNO
            case 2121: // mineo, DCM/SB/KDDI MVNO
            case 2429: // Rakuten Mobile (MNO), JP 4st operator
            case 2432: // nuromobile, DCM/SB/KDDI MVNO
            */
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
            case 1168: // Corr Wireless Communications LLC, FALL-THROUGH
            case 1170: // New Mexico RSA 4 East Ltd. Partnership, FALL-THROUGH
            case 1190: // Cincinnati Bell Wireless LLC, FALL-THROUGH
            case 1244: // Pine Telephone Company dba Pine Cellular, FALL-THROUGH
            case 1245: // Siouxland PCS, FALL-THROUGH
            case 1781: // Union Telephone Company, FALL-THROUGH
            case 1790: // GTA Wireless LLC, FALL-THROUGH
            case 1800: // MTPCS LLC, FALL-THROUGH
            case 1803: // Elkhart Telephone Co. Inc. dba Epic Touch Co., FALL-THROUGH
            case 1808: // Limitless Mobile, FALL-THROUGH
            case 1810: // Arctic Slopo Telephone Association Cooperative, FALL-THROUGH
            case 1815: // Iowa Wireless Services LLC dba I Wireless, FALL-THROUGH
            case 1825: // Advantage Cellular Systems, Inc., FALL-THROUGH
            case 1833: // Cellular Properties Inc., FALL-THROUGH
            case 1854: // CTC Telecom Inc, FALL-THROUGH
            case 1857: // Wireless Communications Venture, FALL-THROUGH
            case 2029: // Defense Mobile, FALL-THROUGH
            case 2080: // Ting, FALL-THROUGH
            case 2217: // GT&T Cellink Plus, FALL-THROUGH
            case 2256: // TelAlaska Cellular
                carrier.setOperator("CCA");
                break;
            case 1187: { // AT&T
                carrier.setOperator("ATT");

                switch (specificCarrierId) {
                    case 10021: // AT&T 5G
                        break;
                    default:
                        break;
                }
                break;
            }
            case 1193: // North East Cellular Inc.
                carrier.setOperator("CCA");
                carrier.setOperatorSub("VAE");
                break;
            case 1240: // NOTYPO: Commnet Wireless, LLC, FALL-THROUGH
            case 1263: // Illinois Valley Cellular, FALL-THROUGH
            case 1789: // Carolina West Wireless, FALL-THROUGH
            case 1829: // James Valley, FALL-THROUGH
            case 1848: // Chat Mobility - 311430, FALL-THROUGH
            case 1893: // nTelos, FALL-THROUGH
            case 2011: // Inland Cellular, FALL-THROUGH
            case 2159: // United Wireless, FALL-THROUGH
            case 2160: // Pine Belt, FALL-THROUGH
            case 2258: // MobileNation, FALL-THROUGH
            case 2259: // Syringa, FALL-THROUGH
            case 2260: // NexTech Ota, FALL-THROUGH
            case 2261: // Blue Wireless
                carrier.setOperator("ACG");
                break;
            case 1779: // Cricket Wireless
                carrier.setOperator("CRK");
                break;
            case 1788: { // Sprint
                carrier.setOperator("SPR");

                switch (specificCarrierId) {
                    case 10004: // sprintprepaid_us
                        break;
                    default:
                        break;
                }
                break;
            }
            case 1802: // cellcom, FALL-THROUGH
            case 1849: // Bluegrass Cellular LLC
                carrier.setOperator("LRA");
                break;
            case 1836: { // C Spire
                carrier.setOperator("ACG");

                switch (specificCarrierId) {
                    case 10009: // C Spire-US
                        carrier.setOperatorSub("CSPIRE");
                        break;
                    case 10010: // C Spire-NL
                        carrier.setOperatorSub("CSPIRE");
                        break;
                    default:
                        break;
                }
                break;
            }
            case 1839: // Verizon Wireless, FALL-THROUGH
            case 1246: // Alltel Communications Inc. - 311270
                carrier.setOperator("VZW");
                break;
            case 1843: // GCI Communications Corp., FALL-THROUGH
                carrier.setOperator("CCA");
                carrier.setOperatorSub("GCI");
                break;
            case 1949: // MetroPCS
                carrier.setOperator("MPCS");
                break;
            case 1951: // Boost Mobile
                carrier.setOperator("DISH");
                break;
            case 1952: // U.S. Cellular
                carrier.setOperator("USC");
                break;
            case 1955: // Claro PR
                carrier.setOperator("TRF_CLR");
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
                    case 10017: // Google Fi-3UK, FALL-THROUGH
                    case 10018: // Google Fi-3HK, FALL-THROUGH
                    case 10019: // Google Fi-3AT, FALL-THROUGH
                    default:
                        break;
                }
                break;
            }
            case 2022: { // TracFone
                switch (specificCarrierId) {
                    case 10000: // Tracfone-ATT
                        carrier.setOperator("TRF_ATT");
                        break;
                    case 10001: // Tracfone-TMO
                        carrier.setOperator("TRF_TMO");
                        break;
                    case 10008: // Tracfone-VZW
                        carrier.setOperator("TRF_VZW");
                        break;
                    default:
                        break;
                }
                break;
            }
            case 2026: { // Red Pocket
                switch (specificCarrierId) {
                    case 10011: // Red Pocket-TMO
                        carrier.setOperator("TMO");
                        break;
                    case 10012: // Red Pocket-ATT
                        carrier.setOperator("ATT");
                        break;
                    default:
                        break;
                }
                break;
            }
            case 2032: // Xfinity Mobile
                carrier.setOperator("CCT");
                break;
            case 2063: // Family Mobile (Walmart)
                carrier.setOperator("TRF_WFM");
                break;
            case 2075: // Republic Wireless (MVNE2)
                carrier.setOperator("TMO");
                carrier.setOperatorSub("RPW");
                break;
            case 2078: // Simple Mobile
                carrier.setOperator("TRF_SM");
                break;
            case 2119: { // FirstNet
                carrier.setOperator("ATT");

                switch (specificCarrierId) {
                    case 10013: // FirstNet Pacific
                        break;
                    default:
                        break;
                }
                break;
            }
            case 2126: // Spectrum Mobile
                carrier.setOperator("CHT");
                break;
            case 2146: // Visible
                carrier.setOperator("VSB");
                break;
            /*
            case 1166: // Consolidated Telcom
            case 1183: // High Plains Midwest LLC, dba Wetlink Communications
            case 1184: // Mohave Cellular L.P.
            case 1185: // Cellular Network Partnership dba Pioneer Cellular
            case 1186: // Guamcell Cellular and Paging
            case 1188: // TX-11 Acquisition LLC
            case 1189: // Wave Runner LLC
            case 1191: // Alaska Digitel LLC
            case 1192: // Numerex Corp.
            case 1194: // TMP Corporation
            case 1195: // Choice Phone LLC
            case 1196: // Public Service Cellular, Inc.
            case 1197: // Airtel Wireless LLC
            case 1198: // VeriSign
            case 1199: // Oklahoma Western Telephone Company
            case 1234: // UBET Wireless
            case 1235: // Globalstar USA
            case 1236: // Mid-Tex Cellular Ltd.
            case 1237: // Chariton Valley Communications Corp., Inc.
            case 1238: // Missouri RSA No. 5 Partnership
            case 1239: // Indigo Wireless, Inc.
            case 1241: // Thumb Cellular Limited Partnership
            case 1242: // Space Data Corporation
            case 1243: // Easterbrooke Cellular Corporation
            case 1247: // MBO Wireless Inc./Cross Telephone Company
            case 1248: // Wilkes Cellular Inc.
            case 1261: // NOTYPO: Commnet Wireless LLC
            case 1262: // Bag Tussel Wireless LLC
            case 1264: // Torrestar Networks Inc
            case 1265: // Stelera Wireless LLC
            case 1776: // MCI
            case 1778: // Mobile Tel Inc.
            case 1780: // North Sight Communications Inc.
            case 1783: // Nevada Wireless LLC
            case 1784: // MTA Communications dba MTA Wireless
            case 1785: // ACS Wireless Inc.
            case 1787: // Pacific Telecom Inc
            case 1792: // West Central Wireless
            case 1793: // Alaska Wireless Communications LLC
            case 1795: // Nep Cellcorp Inc.
            case 1796: // Smith Bagley Inc, dba Cellular One
            case 1797: // AN Subsidiary LLC
            case 1798: // Wireless Solutions International
            case 1804: // Coleman County Telecommunications Inc. (Trans Texas PCS)
            case 1806: // Jasper Wireless Inc.
            case 1807: // AT&T Mobility Vanguard Services
            case 1809: // Cross Valiant Cellular Partnership
            case 1811: // Wireless Solutions International Inc.
            case 1812: // Sea Mobile
            case 1813: // East Kentucky Network LLC dba Appalachian Wireless
            case 1814: // Panhandle Telecommunications Systems Inc.
            case 1816: // Connect Net Inc
            case 1817: // PinPoint Communications Inc.
            case 1818: // Brazos Cellular Communications Ltd.
            case 1819: // South Canaan Cellular Communications Co. LP
            case 1820: // Caprock Cellular Ltd. Partnership
            case 1821: // Edge Mobile LLC
            case 1822: // Aeris Communications, Inc.
            case 1823: // TX RSA 15B2, LP dba Five Star Wireless
            case 1824: // Kaplan Telephone Company Inc.
            case 1826: // Rural Cellular Corporation
            case 1827: // Mid-Rivers
            case 1828: // Southern IL RSA Partnership dba First Cellular of Southern Illinois
            case 1830: // Copper Valley Wireless
            case 1831: // PetroCom LLC
            case 1834: // ARINC
            case 1835: // Farmers Cellular Telephone
            case 1837: // Cordova Wireless Communications Inc
            case 1838: // SLO Cellular Inc. dba CellularOne of San Luis Obispo
            case 1840: // Pinpoint Wireless Inc.
            case 1841: // Rutal Cellular Corporation
            case 1842: // Leaco Rural Telephone Company Inc
            case 1844: // GreenFly LLC
            case 1845: // Midwest Wireless Holdings LLC
            case 1846: // Iowa RSA No.2 Ltd Partnership
            case 1847: // northwestcell
            case 1850: // PTCI
            case 1851: // Fisher Wireless Services Inc
            case 1852: // Vitelcom Cellular Inc dba Innovative Wireless
            case 1853: // Virgin Mobile
            case 1855: // Benton-Lian Wireless
            case 1856: // Crossroads Wireless Inc
            case 1858: // Keystone Wireless Inc
            case 1859: // NOTYPO: Commnet Midwest LLC
            case 1860: // Nextel Communications Inc.
            case 1861: // Southern Communications Services Inc.
            case 1901: // Android Emulator
            case 1910: // COX
            case 2023: // Consumer Cellular
            case 2024: // Locus Telecom
            case 2025: // Telrite/Pure Talk
            case 2027: // Airvoice
            case 2028: // Ztar
            case 2030: // AGMS
            case 2031: // Kore
            case 2060: // Amaysim (MVNE2)
            case 2062: // Bright Spot (Leto)
            case 2064: // GoSmart
            case 2065: // IDT
            case 2066: // Kajeet (Arterra)
            case 2067: // Lyca Mobile
            case 2068: // M2M
            case 2069: // MVNE2/AMDOCS
            case 2070: // Nest
            case 2071: // Plinton
            case 2073: // PWG/Cintex
            case 2074: // Ready Wireless
            case 2076: // Roam Mobility
            case 2077: // Rock Island
            case 2079: // Solavei
            case 2081: // Twilio M2M
            case 2083: // Ultra/Univision
            case 2086: // Vodafone US
            case 2087: // Wyless
            case 2120: // FirstNet (Lab)
            case 2128: // Sprint Wholesale
            case 2143: // Truphone - 31030 ???
            case 2157: // IT&E OverSeas
            case 2158: // North Dakota Network Company
            case 2161: // Missouri RSA No 5 Partnership
            case 2162: // Custer
            case 2253: // ALU Test-SIM
            case 2254: // etex
            case 2255: // NexTech Wireless
            case 2257: // Cleartalk
            case 2288: // GSC
            case 2315: // Norvado
            case 2316: // Mosaic Mobile
            case 2317: // Nemont
            case 2318: // Bravado wireless
            case 2319: // Element Mobile
            case 2320: // strata
            case 2321: // southcentral
            case 2322: // snakeriver
            case 2323: // silverstar
            case 2324: // NNTC Wireless
            case 2325: // Clear Sky Wireless
            case 2340: // Google CBRS
            case 2421: // Testing US
            case 2433: // Assurance Wireless
            case 2449: // Spectrum CBRS
            case 2459: // GigSky
            case 2464: // Mobi
            case 2465: // Spectrum Data
            */
            default:
                return false;
        }

        return true;
    }

    private static boolean setOperatorForCA(Carrier carrier, int carrierId) {
        switch (carrierId) {
            case 576: // Bell Mobility, FALL-THROUGH
            case 2021: // Virgin Mobile Canada, FALL-THROUGH
            case 2122: // Lucky Mobile
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
            case 1403: // Rogers, FALL-THROUGH
            case 1962: // Fido, FALL-THROUGH
            case 2055: // Chatr Mobile", FALL-THROUGH
            case 2056: // Ztar Mobile, FALL-THROUGH
            case 2057: // Cityfone, FALL-THROUGH
            case 2090: // Tbaytel
                carrier.setOperator("RGS");
                break;
            case 1404: // TELUS Mobility, FALL-THROUGH
            case 2020: // Koodo Mobile, FALL-THROUGH
            case 2089: // Public Mobile
                carrier.setOperator("TLS");
                break;
            case 1895: // Freedom Mobile, FALL-THROUGH
            case 2460: // Shaw Mobile
                carrier.setOperator("VTR");
                carrier.setOperatorSub("FRD");
                break;
            case 2008: // Videotron, FALL-THROUGH
            case 2417: // FIZZ
                carrier.setOperator("VTR");
                break;
            case 2053: // PC mobile, FALL-THROUGH
            case 10005: // pcmobile_prepaid_bell
                carrier.setOperator("BELL");
                carrier.setOperatorSub("PCC");
                break;
            case 2054: // Solo Mobile
                carrier.setOperator("BELL");
                carrier.setOperatorSub("SOC");
                break;
            case 2252: // EastLink
                carrier.setOperator("VTR");
                carrier.setOperatorSub("EST");
                break;
            case 2435: // Xplornet
                carrier.setOperator("XPM");
                break;
            case 10006: // pcmobile_postpaid_telus
                carrier.setOperator("TLS");
                carrier.setOperatorSub("PCC");
                break;
            /*
            case 572: // Clearnet
            case 574: // Ice Wireless
            case 575: // Aliant Mobility
            case 577: // Tbay Mobility
            case 579: // CityTel Mobility
            case 581: // Globalstar
            case 2414: // Airtel Wireless
            */
            default:
                return false;
        }

        return true;
    }

    private static boolean setOperator(Carrier carrier, int carrierId, int specificCarrierId) {
        if (carrierId == SimCarrierId.UNKNOWN_ID) {
            carrier.setOperator("OPEN");
            carrier.setCountry("COM");
        } else {
            carrier.setOperator(String.valueOf(carrierId));
            carrier.setCountry(String.valueOf(specificCarrierId));
        }
        return true;
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
}
