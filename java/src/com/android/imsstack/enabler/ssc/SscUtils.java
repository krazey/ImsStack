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

import android.text.TextUtils;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.SubsInfoInterface;
import com.android.imsstack.core.agents.agentif.ITelephonySubscriber;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.List;
import java.util.Locale;

/**
 * Provides the APIs for obtaining optimized data
 */
public class SscUtils {
    private static SscUtils sSscUtils = new SscUtils();
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

        String domain = null;
        String impi = getImpi(slotId);
        if (subsInfo.isIsimEnabled() && !TextUtils.isEmpty(impi)) { // ISIM
            domain = impi.substring(impi.lastIndexOf("@") + 1, impi.length());
        } else { // USIM
            ITelephonySubscriber ts = getTelephonySubscriber(slotId);
            if (ts == null) {
                return null;
            }

            String strMnc = ts.getMnc(true);
            String strMcc = ts.getMcc(true);
            if (TextUtils.isEmpty(strMnc) || TextUtils.isEmpty(strMcc)) {
                ImsLog.e("Wrong MNC : " + strMnc + " or MCC : " + strMcc);
                return null;
            }

            try {
                domain = String.format(Locale.US, "ims.mnc%03d.mcc%03d.3gppnetwork.org",
                        Integer.parseInt(strMnc), Integer.parseInt(strMcc));
            } catch (NumberFormatException e) {
                e.printStackTrace();
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

    protected String getNumberFromUri(final String uri) {
        if (TextUtils.isEmpty(uri)) {
            return null;
        }

        String number = "";
        if (uri.startsWith("tel:")) {
            // tel:+4477009900123 -> +4477009900123
            // tel:004477009900123;phone-context=exampl.com -> 004477009900123
            int beginIndex = 4;
            int endIndex = uri.indexOf(";phone-context");
            if (endIndex == -1 || beginIndex > endIndex || beginIndex >= uri.length()) {
                number = uri.substring(beginIndex);
            } else {
                number = uri.substring(beginIndex, endIndex);
            }
        } else if (uri.startsWith("sip:")) {
            // sip:+4477009900123@example.com;user=phone -> +4477009900123
            // sip:004477009900123;phone-context=example.com@example.com;user=phone
            // -> 004477009900123
            int beginIndex = 4;
            int endIndex = uri.indexOf(";phone-context");
            if (endIndex == -1) {
                endIndex = uri.indexOf("@");
            }

            if (endIndex == -1 || beginIndex > endIndex || beginIndex >= uri.length()) {
                number = uri.substring(beginIndex);
            } else {
                number = uri.substring(beginIndex, endIndex);
            }
        } else {
            number = uri;
        }

        ImsLog.d("number is " + number);
        return number;
    }

    protected String getUriFromNumber(int slotId, String number) {
        if (TextUtils.isEmpty(number)) {
            ImsLog.d("Number is empty !!!");
            return null;
        }

        String domain = null;
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
        if (!TextUtils.isEmpty(ccToAdd) && !number.startsWith("+")) {
            if (number.startsWith("0")) {
                number = number.substring(1);
            }
            number = ccToAdd + number;
        }

        final String ccToRemove = SscConfig.getCountryCodeToReplaceCountryCodeWithZero(slotId);
        if (!TextUtils.isEmpty(ccToRemove) && number.startsWith(ccToRemove)) {
            number =  "0" + number.substring(ccToRemove.length());
        }

        final String format = SscConfig.getTargetAddrScheme(slotId);
        ImsLog.d("number : " + number + ", format : " + format + ", domain : " + domain);

        // IR92 2.2.3 Addressing
        String uri = null;
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
    protected ITelephonySubscriber getTelephonySubscriber(int slotId) {
        return (ITelephonySubscriber) AgentFactory.getAgent(AgentFactory.TELEPHONY_SUBSCRIBER,
                slotId);
    }
}
