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

import static org.junit.Assert.assertEquals;

import com.android.imsstack.enabler.ssc.SscConstant;
import com.android.imsstack.enabler.ssc.SscServiceClassUtil;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class UtMmiConverterTest {

    @Test
    public void getMmiCode_baicQuery() {
        int category = UtMmiConverter.CATEGORY_CB;
        int action = SscConstant.ACTION_INTERROGATION;
        int condition = SscConstant.CONDITION_BAIC;

        String mmiCode = UtMmiConverter.getMmiCode(category, action, condition);
        assertEquals("*#35#", mmiCode);
    }

    @Test
    public void getMmiCodeSiaSib_baocActivation() {
        int category = UtMmiConverter.CATEGORY_CB;
        int action = SscConstant.ACTION_ACTIVATION;
        int condition = SscConstant.CONDITION_BAOC;
        String password = null;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_NONE;

        String mmiCode = UtMmiConverter.getMmiCodeSiaSib(category, action, condition, password,
                serviceClass);
        assertEquals("*33#", mmiCode);
    }

    @Test
    public void getMmiCodeSiaSib_boicDeactivationeWithPassword() {
        int category = UtMmiConverter.CATEGORY_CB;
        int action = SscConstant.ACTION_DEACTIVATION;
        int condition = SscConstant.CONDITION_BOIC;
        String password = "1234";
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_NONE;

        String mmiCode = UtMmiConverter.getMmiCodeSiaSib(category, action, condition, password,
                serviceClass);
        assertEquals("#331*1234#", mmiCode);
    }

    @Test
    public void getMmiCodeSiaSib_boicExhcActivationWithPasswordForVoice() {
        int category = UtMmiConverter.CATEGORY_CB;
        int action = SscConstant.ACTION_ACTIVATION;
        int condition = SscConstant.CONDITION_BOIC_EXHC;
        String password = "12345";
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_VOICE;

        String mmiCode = UtMmiConverter.getMmiCodeSiaSib(category, action, condition, password,
                serviceClass);
        assertEquals("*332*12345*11#", mmiCode);
    }

    @Test
    public void getMmiCodeSiaSib_bicWrDeactivationForVideo() {
        int category = UtMmiConverter.CATEGORY_CB;
        int action = SscConstant.ACTION_DEACTIVATION;
        int condition = SscConstant.CONDITION_BIC_WR;
        String password = null;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_VIDEO;

        String mmiCode = UtMmiConverter.getMmiCodeSiaSib(category, action, condition, password,
                serviceClass);
        assertEquals("#351**22#", mmiCode);
    }

    @Test
    public void getMmiCode_cfuQuery() {
        int category = UtMmiConverter.CATEGORY_CF;
        int action = SscConstant.ACTION_INTERROGATION;
        int condition = SscConstant.CONDITION_CFU;

        String mmiCode = UtMmiConverter.getMmiCode(category, action, condition);
        assertEquals("*#21#", mmiCode);
    }

    @Test
    public void getMmiCodeSiaSibSic_cfbDeactivation() {
        int category = UtMmiConverter.CATEGORY_CF;
        int action = SscConstant.ACTION_DEACTIVATION;
        int condition = SscConstant.CONDITION_CFB;
        String targetNumber = null;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_NONE;
        int noReplyTimer = 0;

        String mmiCode = UtMmiConverter.getMmiCodeSiaSibSic(category, action, condition,
                targetNumber, serviceClass, noReplyTimer);
        assertEquals("#67#", mmiCode);
    }

    @Test
    public void getMmiCodeSiaSibSic_cfnrActivationWithTimer() {
        int category = UtMmiConverter.CATEGORY_CF;
        int action = SscConstant.ACTION_ACTIVATION;
        int condition = SscConstant.CONDITION_CFNR;
        String targetNumber = null;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_NONE;
        int noReplyTimer = 20;

        String mmiCode = UtMmiConverter.getMmiCodeSiaSibSic(category, action, condition,
                targetNumber, serviceClass, noReplyTimer);
        assertEquals("*61***20#", mmiCode);
    }

    @Test
    public void getMmiCodeSiaSibSic_cfnrRegistrationWithTimerForVoice() {
        int category = UtMmiConverter.CATEGORY_CF;
        int action = SscConstant.ACTION_REGISTRATION;
        int condition = SscConstant.CONDITION_CFNR;
        String targetNumber = null;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_VOICE;
        int noReplyTimer = 20;

        String mmiCode = UtMmiConverter.getMmiCodeSiaSibSic(category, action, condition,
                targetNumber, serviceClass, noReplyTimer);
        assertEquals("**61**11*20#", mmiCode);
    }

    @Test
    public void getMmiCodeSiaSibSic_cfnrRegistrationWithTargetAndTimerForVideo() {
        int category = UtMmiConverter.CATEGORY_CF;
        int action = SscConstant.ACTION_REGISTRATION;
        int condition = SscConstant.CONDITION_CFNR;
        String targetNumber = "+12345678901";
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_VIDEO;
        int noReplyTimer = 20;

        String mmiCode = UtMmiConverter.getMmiCodeSiaSibSic(category, action, condition,
                targetNumber, serviceClass, noReplyTimer);
        assertEquals("**61*+12345678901*22*20#", mmiCode);
    }

    @Test
    public void getMmiCodeSiaSibSic_cfnrRegistrationWithTargetAndTimer() {
        int category = UtMmiConverter.CATEGORY_CF;
        int action = SscConstant.ACTION_REGISTRATION;
        int condition = SscConstant.CONDITION_CFNR;
        String targetNumber = "+12345678901";
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_NONE;
        int noReplyTimer = 20;

        String mmiCode = UtMmiConverter.getMmiCodeSiaSibSic(category, action, condition,
                targetNumber, serviceClass, noReplyTimer);
        assertEquals("**61*+12345678901**20#", mmiCode);
    }

    @Test
    public void getMmiCodeSiaSibSic_cfnrcErasure() {
        int category = UtMmiConverter.CATEGORY_CF;
        int action = SscConstant.ACTION_ERASURE;
        int condition = SscConstant.CONDITION_CFNRC;
        String targetNumber = null;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_NONE;
        int noReplyTimer = 0;

        String mmiCode = UtMmiConverter.getMmiCodeSiaSibSic(category, action, condition,
                targetNumber, serviceClass, noReplyTimer);
        assertEquals("##62#", mmiCode);
    }

    @Test
    public void getMmiCode_cfaQuery() {
        int category = UtMmiConverter.CATEGORY_CF;
        int action = SscConstant.ACTION_INTERROGATION;
        int condition = SscConstant.CONDITION_CFA;
        String targetNumber = null;

        String mmiCode = UtMmiConverter.getMmiCode(category, action, condition);
        assertEquals("*#002#", mmiCode);
    }

    @Test
    public void getMmiCode_cfacQuery() {
        int category = UtMmiConverter.CATEGORY_CF;
        int action = SscConstant.ACTION_INTERROGATION;
        int condition = SscConstant.CONDITION_CFAC;
        String targetNumber = null;

        String mmiCode = UtMmiConverter.getMmiCode(category, action, condition);
        assertEquals("*#004#", mmiCode);
    }

    @Test
    public void getMmiCodeSia_cwActivation() {
        int category = UtMmiConverter.CATEGORY_CW;
        int action = SscConstant.ACTION_ACTIVATION;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_NONE;

        String mmiCode = UtMmiConverter.getMmiCodeSia(category, action, serviceClass);
        assertEquals("*43#", mmiCode);
    }

    @Test
    public void getMmiCodeSia_cwDeactivationForVoice() {
        int category = UtMmiConverter.CATEGORY_CW;
        int action = SscConstant.ACTION_DEACTIVATION;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_VOICE;

        String mmiCode = UtMmiConverter.getMmiCodeSia(category, action, serviceClass);
        assertEquals("#43*11#", mmiCode);
    }

    @Test
    public void getMmiCode_oirQuery() {
        int category = UtMmiConverter.CATEGORY_OIR;
        int action = SscConstant.ACTION_INTERROGATION;

        String mmiCode = UtMmiConverter.getMmiCode(category, action);
        assertEquals("*#31#", mmiCode);
    }

    @Test
    public void getMmiCode_oipActivation() {
        int category = UtMmiConverter.CATEGORY_OIP;
        int action = SscConstant.ACTION_ACTIVATION;

        String mmiCode = UtMmiConverter.getMmiCode(category, action);
        assertEquals("*30#", mmiCode);
    }

    @Test
    public void getMmiCode_tirDeactivation() {
        int category = UtMmiConverter.CATEGORY_TIR;
        int action = SscConstant.ACTION_DEACTIVATION;

        String mmiCode = UtMmiConverter.getMmiCode(category, action);
        assertEquals("#77#", mmiCode);
    }

    @Test
    public void getMmiCode_tipQuery() {
        int category = UtMmiConverter.CATEGORY_TIP;
        int action = SscConstant.ACTION_INTERROGATION;

        String mmiCode = UtMmiConverter.getMmiCode(category, action);
        assertEquals("*#76#", mmiCode);
    }

    @Test
    public void getMmiCode_invalidCategory() {
        int category = UtMmiConverter.CATEGORY_TIP + 1;
        int action = SscConstant.ACTION_INTERROGATION;

        String mmiCode = UtMmiConverter.getMmiCode(category, action);
        assertEquals("*##", mmiCode);
    }

    @Test
    public void getMmiCode_invalidAction() {
        int category = UtMmiConverter.CATEGORY_TIP;
        int action = SscConstant.ACTION_INTERROGATION + 1;

        String mmiCode = UtMmiConverter.getMmiCode(category, action);
        assertEquals("76#", mmiCode);
    }

    @Test
    public void getMmiCode_invalidCondition() {
        int category = UtMmiConverter.CATEGORY_CF;
        int action = SscConstant.ACTION_INTERROGATION;
        int condition = SscConstant.CONDITION_CFNL + 1;

        String mmiCode = UtMmiConverter.getMmiCode(category, action, condition);
        assertEquals("*##", mmiCode);
    }
}
