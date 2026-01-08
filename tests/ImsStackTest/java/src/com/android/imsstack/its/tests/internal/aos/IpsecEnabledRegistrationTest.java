/*
 * Copyright (C) 2025 The Android Open Source Project
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
package com.android.imsstack.its.tests.internal.aos;

import static android.telephony.ims.stub.ImsRegistrationImplBase.REGISTRATION_TECH_IWLAN;
import static android.telephony.ims.stub.ImsRegistrationImplBase.REGISTRATION_TECH_LTE;
import static android.telephony.ims.stub.ImsRegistrationImplBase.REGISTRATION_TECH_NR;

import static com.android.imsstack.its.base.TestConstants.SLOT0;

import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.RegistrationManager;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.its.filters.P1;
import com.android.imsstack.its.servercontrol.RuleSet;
import com.android.imsstack.its.servercontrol.ScenarioGeneratorUtils;
import com.android.imsstack.its.tests.registration.RegistrationHelper;
import com.android.imsstack.its.tests.registration.RegistrationInfo;
import com.android.imsstack.its.tests.registration.tests.RegistrationTestBase;
import com.android.imsstack.its.tests.registration.util.MessageBuildUtils;
import com.android.imsstack.its.tests.registration.util.TestRegistration;
import com.android.imsstack.its.util.SingleLatch;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class IpsecEnabledRegistrationTest extends RegistrationTestBase {

    @Before
    public void setUp() throws Exception {
        setUpBase(SLOT0);

        mRegistration = new TestRegistration(mImsServiceConnector.getRegistration());
        createControlConnection(mRegistration);

        mRegistrationHelper = new RegistrationHelper();
        mConfig = new PersistableBundle();
        mInfoBuilder = new RegistrationInfo.Builder();
    }

    @After
    public void tearDown() throws Exception {
        mServerControlConnection.disconnect();
        tearDownBase(SLOT0);

        mEventLatch.sleep(SingleLatch.LONG_SLEEP_MS);
    }

    @Test
    @P1
    public void reregister_onLte_withoutIpsec_after406Response_succeeds()
            throws Exception {
        // 1. The device first attempts to register on LTE with IPSec enabled.
        // 2. The server rejects this request with a 406 Not Acceptable response.
        // 3. The device is configured via CarrierConfig to retry registration without IPSec upon
        //    receiving a 406 error.
        // 4. The device initiates a new REGISTER request, this time without IPSec headers.
        // 5. The server accepts this second request, and the test verifies that the device
        //    successfully registers on LTE.

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister()
                .addRuleSet(new RuleSet.Builder("REGISTER: Includes sec-agree")
                        .addRule(new RuleSet.Rule.RuleBuilder("Require")
                                .addContainRule("sec-agree")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("REGISTER: Includes Security-Client")
                        .addRule(new RuleSet.Rule.RuleBuilder("Security-Client")
                                .addContainRule("ipsec-3gpp")
                                .build())
                        .build())
                .build());

        generator.addMessages("<406-REGISTER");

        generator.addMessage(MessageBuildUtils.getDefaultRegister()
                .addRuleSet(new RuleSet.Builder("REGISTER: Not includes sec-agree")
                        .addRule(new RuleSet.Rule.RuleBuilder("Require")
                                .addNotContainRule("sec-agree")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("REGISTER: Not includes Security-Client")
                        .addRule(new RuleSet.Rule.RuleBuilder("Security-Client")
                                .addNotContainRule("ipsec-3gpp")
                                .build())
                        .build())
                .build());

        generator.addMessages("<200-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putIntArray(CarrierConfig.Ims.KEY_REG_RETRY_ERR_CODE_WITHOUT_IPSEC_INT_ARRAY,
                new int[] {REG_ERROR_CODE_406});

        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_REPEATED);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

        mConfig.putInt(CarrierConfigManager.Ims.KEY_SIP_PREFERRED_TRANSPORT_INT,
                CarrierConfigManager.Ims.PREFERRED_TRANSPORT_TCP);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action
                        == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_LTE);

        mRegistration.expect().registering();

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_LTE);
    }

    @Test
    @P1
    public void reregister_onNr_withoutIpsec_after406Response_succeeds()
            throws Exception {
        // 1. The device first attempts to register on NR with IPSec enabled.
        // 2. The server rejects this request with a 406 Not Acceptable response.
        // 3. The device is configured via CarrierConfig to retry registration without IPSec upon
        //    receiving a 406 error.
        // 4. The device initiates a new REGISTER request, this time without IPSec headers.
        // 5. The server accepts this second request, and the test verifies that the device
        //    successfully registers on NR.

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister()
                .addRuleSet(new RuleSet.Builder("REGISTER: Includes sec-agree")
                        .addRule(new RuleSet.Rule.RuleBuilder("Require")
                                .addContainRule("sec-agree")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("REGISTER: Includes Security-Client")
                        .addRule(new RuleSet.Rule.RuleBuilder("Security-Client")
                                .addContainRule("ipsec-3gpp")
                                .build())
                        .build())
                .build());

        generator.addMessages("<406-REGISTER");

        generator.addMessage(MessageBuildUtils.getDefaultRegister()
                .addRuleSet(new RuleSet.Builder("REGISTER: Not includes sec-agree")
                        .addRule(new RuleSet.Rule.RuleBuilder("Require")
                                .addNotContainRule("sec-agree")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("REGISTER: Not includes Security-Client")
                        .addRule(new RuleSet.Rule.RuleBuilder("Security-Client")
                                .addNotContainRule("ipsec-3gpp")
                                .build())
                        .build())
                .build());

        generator.addMessages("<200-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putIntArray(CarrierConfig.Ims.KEY_REG_RETRY_ERR_CODE_WITHOUT_IPSEC_INT_ARRAY,
                new int[] {REG_ERROR_CODE_406});

        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_REPEATED);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

        mConfig.putInt(CarrierConfigManager.Ims.KEY_SIP_PREFERRED_TRANSPORT_INT,
                CarrierConfigManager.Ims.PREFERRED_TRANSPORT_TCP);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildNrServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_NR)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action
                        == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_NR);

        mRegistration.expect().registering();

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_NR);
    }

    @Test
    @P1
    public void reregister_onWlan_withoutIpsec_after406Response_succeeds()
            throws Exception {
        // 1. The device first attempts to register on IWLAN with IPSec enabled.
        // 2. The server rejects this request with a 406 Not Acceptable response.
        // 3. The device is configured via CarrierConfig to retry registration without IPSec upon
        //    receiving a 406 error.
        // 4. The device initiates a new REGISTER request, this time without IPSec headers.
        // 5. The server accepts this second request, and the test verifies that the device
        //    successfully registers on IWLAN.

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister()
                .addRuleSet(new RuleSet.Builder("REGISTER: Includes sec-agree")
                        .addRule(new RuleSet.Rule.RuleBuilder("Require")
                                .addContainRule("sec-agree")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("REGISTER: Includes Security-Client")
                        .addRule(new RuleSet.Rule.RuleBuilder("Security-Client")
                                .addContainRule("ipsec-3gpp")
                                .build())
                        .build())
                .build());

        generator.addMessages("<406-REGISTER");

        generator.addMessage(MessageBuildUtils.getDefaultRegister()
                .addRuleSet(new RuleSet.Builder("REGISTER: Not includes sec-agree")
                        .addRule(new RuleSet.Rule.RuleBuilder("Require")
                                .addNotContainRule("sec-agree")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("REGISTER: Not includes Security-Client")
                        .addRule(new RuleSet.Rule.RuleBuilder("Security-Client")
                                .addNotContainRule("ipsec-3gpp")
                                .build())
                        .build())
                .build());

        generator.addMessages("<200-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);
        mConfig.putIntArray(CarrierConfig.Ims.KEY_REG_RETRY_ERR_CODE_WITHOUT_IPSEC_INT_ARRAY,
                new int[] {REG_ERROR_CODE_406});

        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_REPEATED);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

        mConfig.putInt(CarrierConfigManager.Ims.KEY_SIP_PREFERRED_TRANSPORT_INT,
                CarrierConfigManager.Ims.PREFERRED_TRANSPORT_TCP);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildLteIwlanServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action
                        == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);

        mRegistration.expect().registering();

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);
    }
}
