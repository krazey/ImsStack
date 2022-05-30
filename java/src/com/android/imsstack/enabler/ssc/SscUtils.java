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
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.SubsInfoInterface;
import com.android.imsstack.core.agents.agentif.ITelephonySubscriber;
import com.android.imsstack.util.ImsLog;

import java.util.List;
import java.util.Locale;

public final class SscUtils {
    public static int getTelephonySimType(int slotId) {
        SubsInfoInterface subsInfo = AgentFactory.getInstance().getAgent(
                SubsInfoInterface.class, slotId);

        if (subsInfo == null) {
            return TelephonyManager.APPTYPE_ISIM;
        }

        if (subsInfo.isIsimEnabled()) {
            return TelephonyManager.APPTYPE_ISIM;
        }

        return TelephonyManager.APPTYPE_USIM;
    }

    public static String getImpu(int slotId) {
        // TODO: Get IMPU from UICC value that provided by IMS platform regardless of USIM or ISIM
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, slotId);
        List<String> impuList = sim != null ? sim.getIsimImpu() : null;

        if (impuList.isEmpty()) {
            ImsLog.w("IMPU is empty!!!");
            return null;
        }

        return impuList.get(0);
    }

    public static String getDomain(int slotId) {
        SubsInfoInterface subsInfo = AgentFactory.getInstance().getAgent(
                SubsInfoInterface.class, slotId);

        if (subsInfo == null) {
            return null;
        }

        String domain = null;
        if (subsInfo.isIsimEnabled() == true) {
            SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, slotId);
            String impi = sim != null ? sim.getIsimImpi() : null;
            if (impi == null || impi.isEmpty()) {
                ImsLog.w("IMPI is invalid !!!");
                return null;
            }
            domain = impi.substring(impi.lastIndexOf("@") + 1 , impi.length());
        } else if (subsInfo.isUsimEnabled() == true) {
            ITelephonySubscriber ts = (ITelephonySubscriber)AgentFactory.getAgent(
                    AgentFactory.TELEPHONY_SUBSCRIBER, slotId);
            if (ts == null) {
                return null;
            }

            int mnc, mcc;
            try {
                mnc = Integer.parseInt(ts.getMnc(true));
                mcc = Integer.parseInt(ts.getMcc(true));
            } catch (final NumberFormatException e) {
                e.printStackTrace();
                ImsLog.e("Invalid MNC/MCC");
                return null;
            }
            domain = String.format(Locale.US, "ims.mnc%03d.mcc%03d.3gppnetwork.org", mnc, mcc);
        } else {
            String impu = getImpu(slotId);
            if (TextUtils.isEmpty(impu)) {
                ImsLog.w("IMPU is invalid !!!");
                return null;
            }
            domain = impu.substring(impu.lastIndexOf("@") + 1 , impu.length());
        }

        return domain;
    }

    public static String getUtUserAgent(int slotId) {
        final String gbaString = "3gpp-gba";
        String userAgent = SscConfig.getImsUserAgent(slotId);
        if (!TextUtils.isEmpty(gbaString)) {
            if (!TextUtils.isEmpty(userAgent)) {
                userAgent += " " + gbaString;
            } else {
                userAgent = gbaString;
            }
        }

        return userAgent;
    }
}
