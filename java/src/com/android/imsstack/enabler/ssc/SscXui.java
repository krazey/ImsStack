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
import com.android.imsstack.core.agents.PreferenceInterface;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Locale;

public class SscXui {
    private static final SscXui sSscXui = new SscXui();

    @VisibleForTesting
    protected static final String IMPU_FILE_NAME = "impu_list";
    @VisibleForTesting
    protected static final String IMPU_LIST_SIZE = "size";
    private static final String XUI_TOP_PREFERRED = "top";
    private static final String XUI_SIP_PREFERRED = "sip";

    private SscXui() {
    }

    protected static SscXui getInstance() {
        return sSscXui;
    }

    protected String getXui(int slotId, String password) {
        String xui = null;

        int paidListSize = getPAssociatedUriSize(slotId);
        if (paidListSize > 0) {
            String paidFormat = XUI_TOP_PREFERRED;
            if (!TextUtils.isEmpty(password)) {
                paidFormat = XUI_SIP_PREFERRED;
            }

            if (XUI_TOP_PREFERRED.equalsIgnoreCase(paidFormat)) {
                xui = getPAssociatedUriValue(slotId, 0);
            } else {
                for (int i = 0; i < paidListSize; i++) {
                    String paid = getPAssociatedUriValue(slotId, i);
                    if (paid != null && paid.toLowerCase(Locale.ROOT).startsWith(paidFormat)) {
                        xui = paid;
                        break;
                    }
                }

                if (xui == null) {
                    // in case of no xui for preferred type, return top value
                    xui = getPAssociatedUriValue(slotId, 0);
                }
            }
        }

        if (xui == null) {
            xui = SscUtils.getInstance().getImpu(slotId);
        }

        ImsLog.d("XUI :" + xui);
        return setUserInfoWithPassword(xui, password);
    }

    private String setUserInfoWithPassword(String xui, String password) {
        if (TextUtils.isEmpty(xui)) {
            return null;
        }

        if (TextUtils.isEmpty(password)) {
            return xui;
        }

        if (!xui.toLowerCase(Locale.ROOT).startsWith("sip")) {
            ImsLog.e("XUI isn't a SIP URI.");
            return xui;
        }

        String[] tokens = xui.split("@");
        if (tokens.length < 2) {
            return xui; // abnormal uri case
        }

        return tokens[0] + ":" + password + "@" + tokens[1];
    }

    private int getPAssociatedUriSize(int slotId) {
        PreferenceInterface preference =
                AgentFactory.getInstance().getAgent(PreferenceInterface.class);
        if (preference == null) {
            return 0;
        }

        String strNumberOfImpu = preference.getString(IMPU_FILE_NAME, IMPU_LIST_SIZE, slotId);
        int numberOfImpu = 0;
        try {
            numberOfImpu = Integer.parseInt(strNumberOfImpu);
        } catch (NumberFormatException e) {
            ImsLog.d(e.toString());
            e.printStackTrace();
        }

        ImsLog.d("slotId/numberOfImpu : " + slotId + "/" + numberOfImpu);
        return numberOfImpu;
    }

    private String getPAssociatedUriValue(int slotId, int index) {
        PreferenceInterface preference =
                AgentFactory.getInstance().getAgent(PreferenceInterface.class);
        if (preference == null) {
            return null;
        }

        String impu = preference.getString(IMPU_FILE_NAME, Integer.toString(index), slotId);
        ImsLog.d("SlotId/Index : " + slotId + "/" + index + ", Impu : " + impu);
        return impu;
    }
}
