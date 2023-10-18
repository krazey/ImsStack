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

package com.android.imsstack.imsservice.mmtel.ut;

import android.text.TextUtils;

import com.android.imsstack.enabler.ssc.SscServiceClassUtil;

/**
 * Provides API that convert supplementary service configuration to MMI code according to TS 22.030
 */
public final class UtMmiConverter {
    static final int CATEGORY_CB = 0;
    static final int CATEGORY_CF = 1;
    static final int CATEGORY_CW = 2;
    static final int CATEGORY_OIR = 3;
    static final int CATEGORY_OIP = 4;
    static final int CATEGORY_TIR = 5;
    static final int CATEGORY_TIP = 6;

    private static final String CODE_END = "#";
    private static final String CODE_SEPARATOR = "*";

    private static final String CODE_DEACTIVATE = "#";
    private static final String CODE_ACTIVATE = "*";
    private static final String CODE_REGISTER = "**";
    private static final String CODE_ERASURE = "##";
    private static final String CODE_INTERROGATE = "*#";

    private static final String[] ACTION_CODE = {
            // SscConstant Action param order
            CODE_DEACTIVATE,
            CODE_ACTIVATE,
            null,
            CODE_REGISTER,
            CODE_ERASURE,
            CODE_INTERROGATE
    };

    // Call Forwarding
    private static final String SC_CFU = "21";
    private static final String SC_CFB = "67";
    private static final String SC_CFNR = "61";
    private static final String SC_CFNRC = "62";
    private static final String SC_CFA = "002";
    private static final String SC_CFAC = "004";

    // Call Waiting
    private static final String SC_CW = "43";

    // Call Barring
    private static final String SC_BAOC = "33";
    private static final String SC_BOIC = "331";
    private static final String SC_BOIC_EXHC = "332";
    private static final String SC_BAIC = "35";
    private static final String SC_BIC_WR = "351";
    private static final String SC_ACR = "157";

    // line presentation/restriction
    private static final String SC_CLIP = "30";
    private static final String SC_CLIR = "31";
    private static final String SC_COLP = "76";
    private static final String SC_COLR = "77";

    // Telecommunication Service
    private static final String SC_VOICE = "11";
    private static final String SC_VIDEO = "22";

    private static final String[][] SERVICE_CODE = {
            { // CATEGORY_CB, SscConstant CB Condition params value order
                null, SC_BAIC, SC_BAOC, SC_BOIC, SC_BOIC_EXHC, SC_BIC_WR, SC_ACR
            },
            { // CATEGORY_CF, SscConstant CF Condition params value order
                SC_CFU, SC_CFB, SC_CFNR, SC_CFNRC, SC_CFA, SC_CFAC
            },
            { // CATEGORY_CW
                SC_CW
            },
            { // CATEGORY_OIR
                SC_CLIR
            },
            { // CATEGORY_OIP
                SC_CLIP
            },
            { // CATEGORY_TIR
                SC_COLR
            },
            { // CATEGORY_TIP
                SC_COLP
            }
    };

    /**
     * Refer getMmiCode(int category, int action, int condition)
     */
    public static String getMmiCode(int category, int action) {
        return getMmiCode(category, action, 0);
    }

    /**
     * returns a dial code that matches an XCAP operation.
     * @param category service category
     * @param action action
     * @param condition condition
     * @return converted MMI code
     */
    public static String getMmiCode(int category, int action, int condition) {
        String mmiCode = getActionCode(action) + getServiceCode(category, condition);
        return mmiCode + CODE_END;
    }

    /**
     * This method is for WAIT. It returns a dial code that matches an XCAP operation.
     * @param category service category
     * @param action action
     * @param serviceClass supplementary information A
     * @return converted MMI code
     */
    public static String getMmiCodeSia(int category, int action, int serviceClass) {
        String mmiCode = getActionCode(action) + getServiceCode(category, 0);

        String sia = "";
        if (serviceClass == SscServiceClassUtil.SERVICE_CLASS_VOICE) {
            sia = CODE_SEPARATOR + SC_VOICE;
        } else if (serviceClass == SscServiceClassUtil.SERVICE_CLASS_VIDEO) {
            sia = CODE_SEPARATOR + SC_VIDEO;
        }

        return mmiCode + sia + CODE_END;
    }

    /**
     * refer getMmiCode(int category, int action, int condition, String number, int serviceClass,
     * int timeSeconds)
     */
    public static String getMmiCodeSiaSib(int category, int action, int condition, String number,
            int serviceClass) {
        return getMmiCodeSiaSibSic(category, action, condition, number, serviceClass, 0);
    }

    /**
     * This method is for CF and CB. It returns a dial code that matches an XCAP operation.
     * number parameter could be either of forward-to number for CF or password for CB
     * @param category service category
     * @param action action
     * @param condition condition
     * @param number supplementary information A
     * @param serviceClass supplementary information B
     * @param timeSeconds supplementary information C
     * @return converted MMI code
     */
    public static String getMmiCodeSiaSibSic(int category, int action, int condition, String number,
            int serviceClass, int timeSeconds) {
        String mmiCode = getActionCode(action) + getServiceCode(category, condition);

        String sia = "";
        if (!TextUtils.isEmpty(number)) {
            sia = number;
        }

        String sib = "";
        if (serviceClass == SscServiceClassUtil.SERVICE_CLASS_VOICE) {
            sib = SC_VOICE;
        } else if (serviceClass == SscServiceClassUtil.SERVICE_CLASS_VIDEO) {
            sib = SC_VIDEO;
        }

        String sic = "";
        if (timeSeconds > 0) {
            sic = Integer.toString(timeSeconds);
        }

        if (!TextUtils.isEmpty(sia) || !TextUtils.isEmpty(sib) || !TextUtils.isEmpty(sic)) {
            sia = CODE_SEPARATOR + sia;

            if (!TextUtils.isEmpty(sib) || !TextUtils.isEmpty(sic)) {
                sib = CODE_SEPARATOR + sib;

                if (!TextUtils.isEmpty(sic)) {
                    sic = CODE_SEPARATOR + sic;
                }
            }
        }

        return mmiCode + sia + sib + sic + CODE_END;
    }

    private static String getActionCode(int action) {
        if (action < 0 || action >= ACTION_CODE.length) {
            return "";
        }

        return ACTION_CODE[action];
    }

    private static String getServiceCode(int category, int condition) {
        if (category < CATEGORY_CB || category > CATEGORY_TIP) {
            return "";
        }

        if (condition < 0 || condition >= SERVICE_CODE[category].length) {
            return "";
        }

        return SERVICE_CODE[category][condition];
    }
}
