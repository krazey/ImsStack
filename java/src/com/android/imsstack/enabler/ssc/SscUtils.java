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

package com.android.imsstack.enabler.ssc;

import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ImsRadioInterface;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.SubsInfoInterface;
import com.android.imsstack.core.agents.TelephonyInterface;
import com.android.imsstack.enabler.ssc.SscConfig.CarrierConfigServiceType;
import com.android.imsstack.enabler.ssc.SscConstant.AccessNetworkTypes;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.List;
import java.util.Locale;

/**
 * Provides the APIs for obtaining optimized data
 */
public class SscUtils {
    private static final SscUtils sSscUtils = new SscUtils();
    protected static SscUtils getInstance() {
        return sSscUtils;
    }

    protected int getTelephonySimType(int slotId) {
        SubsInfoInterface subsInfo = getSubsInfoInterface(slotId);
        if (subsInfo == null) {
            return SscConstant.APPTYPE_ISIM;
        }

        if (subsInfo.isIsimEnabled()) {
            return SscConstant.APPTYPE_ISIM;
        }

        return SscConstant.APPTYPE_USIM;
    }

    protected String getImpi(int slotId) {
        SimInterface sim = getSimInterface(slotId);
        String impi = (sim != null) ? sim.getIsimImpi() : null;
        if (TextUtils.isEmpty(impi)) {
            ImsLog.w("wrong IMPI");
            return null;
        }

        return impi;
    }

    protected String getImpu(int slotId) {
        // TODO: Get IMPU from UICC value that provided by IMS platform regardless of USIM or ISIM
        SimInterface sim = getSimInterface(slotId);
        List<String> impuList = (sim != null) ? sim.getIsimImpu() : null;
        if (impuList == null || impuList.isEmpty()) {
            ImsLog.w("wrong IMPU");
            return null;
        }

        return impuList.get(0);
    }

    protected String getDomain(int slotId, boolean forXcapRootUri) {
        SubsInfoInterface subsInfo = getSubsInfoInterface(slotId);
        if (subsInfo == null) {
            return null;
        }

        String domain;
        String impi = getImpi(slotId);
        if (subsInfo.isIsimEnabled() && !TextUtils.isEmpty(impi) && impi.contains("@")) { // ISIM
            domain = impi.substring(impi.lastIndexOf("@") + 1);
        } else { // USIM
            TelephonyInterface telephony = getTelephonyInterface(slotId);
            if (telephony == null) {
                return null;
            }

            String strMnc = telephony.getSimMnc();
            String strMcc = telephony.getSimMcc();
            if (TextUtils.isEmpty(strMnc) || TextUtils.isEmpty(strMcc)) {
                ImsLog.e("Wrong MNC : " + strMnc + " or MCC : " + strMcc);
                return null;
            }

            try {
                domain = String.format(Locale.US, "ims.mnc%03d.mcc%03d.3gppnetwork.org",
                        Integer.parseInt(strMnc), Integer.parseInt(strMcc));
            } catch (NumberFormatException e) {
                ImsLog.e(e.toString());
                return null;
            }
        }

        if (forXcapRootUri) {
            domain = "xcap." + domain;
            if (domain.contains("3gppnetwork.org")) {
                domain = domain.replace("3gppnetwork.org", "pub.3gppnetwork.org");
            }
        }

        return domain;
    }

    protected String getSscUserAgent(int slotId) {
        final String gbaString = "3gpp-gba";
        String userAgent = SscConfig.getImsUserAgent(slotId);
        if (!TextUtils.isEmpty(userAgent)) {
            userAgent += " " + gbaString;
        } else {
            userAgent = gbaString;
        }

        return userAgent;
    }

    protected String getNumberFromUri(int slotId, final String uri) {
        if (TextUtils.isEmpty(uri)) {
            return null;
        }

        String number;
        if (uri.startsWith("tel:") || uri.startsWith("sip:") || uri.startsWith("sips:")) {
            number = uri.substring(uri.indexOf(":") + 1);

            if (number.contains(";")) {
                number = number.substring(0, number.indexOf(";"));
            }

            if (number.contains("@")) {
                number = number.substring(0, number.indexOf("@"));
            }
        } else {
            number = uri;
        }

        final String ccToAdd = SscConfig.getCountryCodeToReplaceZeroWithCountryCode(slotId);
        if (!TextUtils.isEmpty(ccToAdd) && number.startsWith("0")) {
            number = ccToAdd + number.substring(1);
        }

        final String ccToRemove = SscConfig.getCountryCodeToReplaceCountryCodeWithZero(slotId);
        if (!TextUtils.isEmpty(ccToRemove) && number.startsWith(ccToRemove)) {
            number =  "0" + number.substring(ccToRemove.length());
        }

        ImsLog.d("number is " + number);
        return number;
    }

    protected String getUriFromNumber(int slotId, String number) {
        if (TextUtils.isEmpty(number)) {
            return "";
        }

        final String domain;
        String phoneContext = SscConfig.getPhoneContextForTargetAddress(slotId);
        if (!TextUtils.isEmpty(phoneContext)) {
            domain = phoneContext;
        } else {
            domain = getDomain(slotId, false);
        }

        if (domain == null) {
            ImsLog.w("Domain is null !!!");
            return number;
        }

        final String ccToAdd = SscConfig.getCountryCodeToReplaceZeroWithCountryCode(slotId);
        if (!TextUtils.isEmpty(ccToAdd) && number.startsWith("0")) {
            number = ccToAdd + number.substring(1);
        }

        final String ccToRemove = SscConfig.getCountryCodeToReplaceCountryCodeWithZero(slotId);
        if (!TextUtils.isEmpty(ccToRemove) && number.startsWith(ccToRemove)) {
            number =  "0" + number.substring(ccToRemove.length());
        }

        final String format = SscConfig.getTargetAddrScheme(slotId);
        ImsLog.d("number : " + number + ", format : " + format + ", domain : " + domain);

        // IR92 2.2.3 Addressing
        String uri;
        if ("sip".equalsIgnoreCase(format)) {
            uri = "sip:" + number;
            // local numbering
            if (!number.startsWith("+")) {
                uri += ";phone-context=" + domain;
            }
            uri += "@" + domain + ";user=phone";
        } else if ("tel".equalsIgnoreCase(format)) {
            uri = "tel:" + number;
            // local numbering
            if (!number.startsWith("+")) {
                uri += ";phone-context=" + domain;
            }
        } else {
            uri = number;
        }

        ImsLog.d("uri is " + uri);
        return uri;
    }

    /**
     * Converts network type from TelephonyManager#NETWORK_TYPE_XXX to
     * ImsRadioInterface#ACCESS_NETWORK_TYPE_XXX
     *
     * @param networkType One of network type defined in {@link TelephonyManager}. See
     * {@link TelephonyManager#NETWORK_TYPE_IWLAN}, {@link TelephonyManager#NETWORK_TYPE_LTE}, and
     * others.
     * @return Matched network type of {@link ImsRadioInterface.AccessNetworkType}
     */
    protected static int convertToImsRadioNetworkType(int networkType) {
        switch (networkType) {
            case TelephonyManager.NETWORK_TYPE_IWLAN:
                return ImsRadioInterface.ACCESS_NETWORK_TYPE_IWLAN;
            case TelephonyManager.NETWORK_TYPE_UMTS: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_HSDPA: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_HSUPA: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_HSPA: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_HSPAP: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_TD_SCDMA:
                return ImsRadioInterface.ACCESS_NETWORK_TYPE_UTRAN;
            case TelephonyManager.NETWORK_TYPE_LTE:
                return ImsRadioInterface.ACCESS_NETWORK_TYPE_EUTRAN;
            case TelephonyManager.NETWORK_TYPE_NR:
                return ImsRadioInterface.ACCESS_NETWORK_TYPE_NGRAN;
            default:
                return ImsRadioInterface.ACCESS_NETWORK_TYPE_UNKNOWN;
        }
    }

    /**
     * Converts network type from TelephonyManager#NETWORK_TYPE_XXX to
     * AccessNetworkConstants.AccessNetworkType
     *
     * @param networkType One of network type defined in {@link TelephonyManager}. See
     * {@link TelephonyManager#NETWORK_TYPE_IWLAN}, {@link TelephonyManager#NETWORK_TYPE_LTE}, and
     * others.
     * @return Matched network type of {@link SscConstant.AccessNetworkTypes}
     */
    protected static @AccessNetworkTypes int convertToAccessNetworkType(int networkType) {
        switch (networkType) {
            case TelephonyManager.NETWORK_TYPE_IWLAN:
                return SscConstant.NETWORK_TYPE_IWLAN;
            case TelephonyManager.NETWORK_TYPE_GPRS: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_GSM: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_EDGE:
                return SscConstant.NETWORK_TYPE_GERAN;
            case TelephonyManager.NETWORK_TYPE_UMTS: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_HSDPA: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_HSUPA: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_HSPA: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_HSPAP: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_TD_SCDMA:
                return SscConstant.NETWORK_TYPE_UTRAN;
            case TelephonyManager.NETWORK_TYPE_LTE:
                return SscConstant.NETWORK_TYPE_EUTRAN;
            case TelephonyManager.NETWORK_TYPE_NR:
                return SscConstant.NETWORK_TYPE_NGRAN;
            default:
                return SscConstant.NETWORK_TYPE_UNKNOWN;
        }
    }

    protected static @CarrierConfigServiceType int getSupplementaryServiceTypeForCarrierConfig(
            ESsType ssType, int condition) {
        switch (ssType) {
            case OIP:
                return SscConfig.SERVICE_TYPE_OIP;
            case OIR:
                return SscConfig.SERVICE_TYPE_OIR;
            case TIP:
                return SscConfig.SERVICE_TYPE_TIP;
            case TIR:
                return SscConfig.SERVICE_TYPE_TIR;
            case CF:
                if (condition == SscConstant.CONDITION_CFU) {
                    return SscConfig.SERVICE_TYPE_CFU;
                } else if (condition == SscConstant.CONDITION_CFB) {
                    return SscConfig.SERVICE_TYPE_CFB;
                } else if (condition == SscConstant.CONDITION_CFNR) {
                    return SscConfig.SERVICE_TYPE_CFNRY;
                } else if (condition == SscConstant.CONDITION_CFNRC) {
                    return SscConfig.SERVICE_TYPE_CFNRC;
                } else if (condition == SscConstant.CONDITION_CFA) {
                    return SscConfig.SERVICE_TYPE_CFA;
                } else if (condition == SscConstant.CONDITION_CFAC) {
                    return SscConfig.SERVICE_TYPE_CFAC;
                } else if (condition == SscConstant.CONDITION_CFNL) {
                    return SscConfig.SERVICE_TYPE_CFNL;
                }

                return SscConfig.SERVICE_TYPE_INVALID;
            case OCB: // FALL-THROUGH
            case ICB:
                if (condition == SscConstant.CONDITION_BAOC) {
                    return SscConfig.SERVICE_TYPE_BAOC;
                } else if (condition == SscConstant.CONDITION_BOIC) {
                    return SscConfig.SERVICE_TYPE_BOIC;
                } else if (condition == SscConstant.CONDITION_BOIC_EXHC) {
                    return SscConfig.SERVICE_TYPE_BOIC_EXHC;
                } else if (condition == SscConstant.CONDITION_BAIC) {
                    return SscConfig.SERVICE_TYPE_BAIC;
                } else if (condition == SscConstant.CONDITION_BIC_WR) {
                    return SscConfig.SERVICE_TYPE_BIC_ROAM;
                } else if (condition == SscConstant.CONDITION_ACR) {
                    return SscConfig.SERVICE_TYPE_ACR;
                }

                return SscConfig.SERVICE_TYPE_INVALID;
            case CW:
                return SscConfig.SERVICE_TYPE_CW;
            default:
                return SscConfig.SERVICE_TYPE_INVALID;
        }
    }

    @VisibleForTesting
    protected SscUtils() {
    }

    @VisibleForTesting
    protected SimInterface getSimInterface(int slotId) {
        return AgentFactory.getInstance().getAgent(SimInterface.class, slotId);
    }

    @VisibleForTesting
    protected SubsInfoInterface getSubsInfoInterface(int slotId) {
        return AgentFactory.getInstance().getAgent(SubsInfoInterface.class, slotId);
    }

    @VisibleForTesting
    protected TelephonyInterface getTelephonyInterface(int slotId) {
        return AgentFactory.getInstance().getAgent(TelephonyInterface.class, slotId);
    }
}
