/*
 * Copyright (C) 2024 The Android Open Source Project
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

import static android.telephony.ims.feature.MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS;
import static android.telephony.ims.feature.MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO;
import static android.telephony.ims.feature.MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE;
import static android.telephony.ims.stub.ImsRegistrationImplBase.REGISTRATION_TECH_IWLAN;
import static android.telephony.ims.stub.ImsRegistrationImplBase.REGISTRATION_TECH_LTE;
import static android.telephony.ims.stub.ImsRegistrationImplBase.REGISTRATION_TECH_NR;

import static com.android.imsstack.its.base.TestConstants.SLOT0;

import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.telephony.TelephonyManager;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.RegistrationManager;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.its.servercontrol.BasicScenarioTemplates;
import com.android.imsstack.its.servercontrol.RuleSet;
import com.android.imsstack.its.servercontrol.ScenarioGeneratorUtils;
import com.android.imsstack.its.servercontrol.ServerMessage;
import com.android.imsstack.its.tests.registration.RegistrationHelper;
import com.android.imsstack.its.tests.registration.RegistrationInfo;
import com.android.imsstack.its.tests.registration.tests.RegistrationTestBase;
import com.android.imsstack.its.tests.registration.util.MessageBuildUtils;
import com.android.imsstack.its.tests.registration.util.TestRegistration;
import com.android.imsstack.its.util.SingleLatch;

import org.junit.After;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class BasicRegistrationTest extends RegistrationTestBase {

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

     // 1. Set up the server to expect a REGISTER request and verify its Contact header includes
     //    MMTEL, audio, video, and SMSIP features.
     // 2. The server then completes the registration and subscription flow.
     // 3. Trigger IMS registration on the device with default configurations.
     // 4. Verify that the device successfully registers on LTE.
    @Test
    public void register_onLte_withDefaultConfig_succeeds() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister()
                .addRuleSet(new RuleSet.Builder("REGISTER(1st) : Valid Contact header")
                        .addRule(new RuleSet.Rule.RuleBuilder("Contact")
                                .addContainRule("+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service"
                                        + ".ims.icsi.mmtel")
                                .addContainRule("audio")
                                .addContainRule("video")
                                .addContainRule("+g.3gpp.smsip")
                                .build())
                        .build())
                .build());
        generator.addMessages("<200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        RegistrationInfo regInfo = mInfoBuilder.build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_LTE);
    }

    // 1. Set up the server to expect a REGISTER request and verify its Contact header includes
    //    MMTEL and audio, but not video or SMSIP.
    // 2. The server then completes the registration and subscription flow.
    // 3. Configure the device to support only the voice capability and disable SMS over IMS.
    // 4. Trigger IMS registration.
    // 5. Verify that the device successfully registers on LTE.
    @Test
    public void register_onLte_withVoiceOnly_succeeds() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister()
                .addRuleSet(new RuleSet.Builder("REGISTER(1st) : Valid Contact header")
                        .addRule(new RuleSet.Rule.RuleBuilder("Contact")
                                .addContainRule("+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service"
                                        + ".ims.icsi.mmtel")
                                .addContainRule("audio")
                                .addNotContainRule("video")
                                .addNotContainRule("+g.3gpp.smsip")
                                .build())
                        .build())
                .build());
        generator.addMessages("<200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_SUPPORTED_BOOL, false);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setEnableCapability(CAPABILITY_TYPE_VOICE,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setDisableCapability(CAPABILITY_TYPE_VIDEO,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setDisableCapability(CAPABILITY_TYPE_SMS,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_LTE);
    }

    // 1. Set up the server to expect a REGISTER request and verify its Contact header includes
    //    MMTEL, audio, and video, but not SMSIP.
    // 2. The server then completes the registration and subscription flow.
    // 3. Configure the device to support voice and video capabilities and disable SMS over IMS.
    // 4. Trigger IMS registration.
    // 5. Verify that the device successfully registers on LTE.
    @Test
    public void register_onLte_withVoiceVideo_succeeds() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister()
                .addRuleSet(new RuleSet.Builder("REGISTER(1st) : Valid Contact header")
                        .addRule(new RuleSet.Rule.RuleBuilder("Contact")
                                .addContainRule("+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service"
                                        + ".ims.icsi.mmtel")
                                .addContainRule("audio")
                                .addContainRule("video")
                                .addNotContainRule("+g.3gpp.smsip")
                                .build())
                        .build())
                .build());
        generator.addMessages("<200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_SUPPORTED_BOOL, false);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setEnableCapability(CAPABILITY_TYPE_VOICE,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setEnableCapability(CAPABILITY_TYPE_VIDEO,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setDisableCapability(CAPABILITY_TYPE_SMS,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_LTE);
    }

    // 1. Set up the server to expect a REGISTER request and verify its Contact header includes
    //    MMTEL, audio, and SMSIP, but not video.
    // 2. The server then completes the registration and subscription flow.
    // 3. Configure the device to support voice and SMS capabilities, but disable video.
    // 4. Trigger IMS registration.
    // 5. Verify that the device successfully registers on LTE.
    @Test
    public void register_onLte_withVoiceSms_succeeds() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister()
                .addRuleSet(new RuleSet.Builder("REGISTER(1st) : Valid Contact header")
                        .addRule(new RuleSet.Rule.RuleBuilder("Contact")
                                .addContainRule("+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service"
                                        + ".ims.icsi.mmtel")
                                .addContainRule("audio")
                                .addNotContainRule("video")
                                .addContainRule("+g.3gpp.smsip")
                                .build())
                        .build())
                .build());
        generator.addMessages("<200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        RegistrationInfo regInfo = mInfoBuilder
                .setEnableCapability(CAPABILITY_TYPE_VOICE,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setDisableCapability(CAPABILITY_TYPE_VIDEO,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setEnableCapability(CAPABILITY_TYPE_SMS,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_LTE);
    }

    // 1. Set up the server to respond to the first REGISTER request with a 423 Interval Too Brief.
    // 2. The server then expects a new REGISTER request and completes the normal registration flow.
    // 3. Trigger IMS registration on the device.
    // 4. Verify that the device retries registration and successfully registers on LTE.
    @Test
    public void register_onLte_with423Response_retriesAndSucceeds() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages(
                "<423-REGISTER | >REGISTER | <200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        RegistrationInfo regInfo = mInfoBuilder.build();
        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_LTE);
    }

    // 1. Set up the server to respond to the REGISTER request with a 403 Forbidden.
    // 2. Configure the device to treat a 403 response as a critical error that
    //    triggers a PLMN block.
    // 3. Trigger IMS registration on the device.
    // 4. Verify that the device enters a deregistered state with the suggested action to
    //    block the PLMN.
    @Test
    public void register_onLte_with403AsCriticalError_triggersPlmnBlock()
            throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages("<403-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // KEY_REGISTRATION_PERMANENT_ERROR_CODE_INT_ARRAY - REG_ERROR_CODE_403
        mConfig.putIntArray(CarrierConfig.Ims.KEY_REGISTRATION_PERMANENT_ERROR_CODE_INT_ARRAY,
                new int[] {REG_ERROR_CODE_403});

        // KEY_EXTRA_REG_ERR_FINAL_TYPE_INT - ERROR_TYPE_CRITICAL
        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_CRITICAL);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK,
                networkType -> networkType == REGISTRATION_TECH_LTE);
    }

    // 1. Set up the server to respond to the REGISTER request with a 404 Not Found.
    // 2. Configure the device via CarrierConfig to define the 404 response as a 'critical'
    //    error type, which results in blocking the current PLMN.
    // 3. Trigger IMS registration on the device.
    // 4. Verify that the device enters a deregistered state with the suggested action to
    //    block the PLMN.
    @Test
    public void register_onLte_with404AsCriticalError_triggersPlmnBlock()
            throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages("<404-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // KEY_REGISTRATION_PERMANENT_ERROR_CODE_INT_ARRAY - REG_ERROR_CODE_404
        mConfig.putIntArray(CarrierConfig.Ims.KEY_REGISTRATION_PERMANENT_ERROR_CODE_INT_ARRAY,
                new int[] {REG_ERROR_CODE_404});

        // KEY_EXTRA_REG_ERR_FINAL_TYPE_INT - ERROR_TYPE_CRITICAL
        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_CRITICAL);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK,
                networkType -> networkType == REGISTRATION_TECH_LTE);
    }

    // 1. Set up the server to respond to the REGISTER request with a 500 Server Internal Error.
    // 2. Configure the device via CarrierConfig to define the 5xx response as a 'repeated'
    //    error type, which results in blocking the PLMN with a specific protocol timer.
    // 3. Trigger IMS registration on the device.
    // 4. Verify that the device enters a deregistered state with the suggested action to
    //    block the PLMN with a timeout.
    @Test
    public void register_onLte_with500AsRepeatedError_triggersPlmnBlockWithTimeout()
            throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages("<500-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY - REG_ERROR_CODE_5XX
        mConfig.putIntArray(CarrierConfig.Ims.KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY,
                new int[] {REG_ERROR_CODE_5XX});

        // KEY_EXTRA_REG_ERR_FINAL_TYPE_INT - ERROR_TYPE_REPEATED
        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_REPEATED);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_LTE);
    }

    // 1. Set up the server to respond to the REGISTER request with a 503 Service Unavailable.
    // 2. Configure the device via CarrierConfig to define the 503 response as a 'repeated'
    //    error type, which results in blocking the PLMN with a specific protocol timer.
    // 3. Trigger IMS registration on the device.
    // 4. Verify that the device enters a deregistered state with the suggested action to
    //    block the PLMN with a timeout.
    @Test
    public void register_onLte_with503AsRepeatedError_triggersPlmnBlockWithTimeout()
            throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages("<503-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY - REG_ERROR_CODE_503
        mConfig.putIntArray(CarrierConfig.Ims.KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY,
                new int[] {REG_ERROR_CODE_503});

        // KEY_EXTRA_REG_ERR_FINAL_TYPE_INT - ERROR_TYPE_REPEATED
        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_REPEATED);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_LTE);
    }

    // 1. Set up the server to respond to the REGISTER request with a 503 Service Unavailable
    //    and a Retry-After header.
    // 2. Configure the device via CarrierConfig to define the 503 response as a 'repeated'
    //    error type.
    // 3. Trigger IMS registration on the device.
    // 4. Verify that the device enters a deregistered state with the suggested action to
    //    block the PLMN with a timeout.
    @Test
    public void register_onLte_with503RetryAfter_triggersPlmnBlockWithTimeout()
            throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("503-REGISTER")
                .setHeader("Retry-After", "100")
                .build());
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY - REG_ERROR_CODE_503
        mConfig.putIntArray(CarrierConfig.Ims.KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY,
                new int[] {REG_ERROR_CODE_503});

        // KEY_EXTRA_REG_ERR_FINAL_TYPE_INT - ERROR_TYPE_REPEATED
        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_REPEATED);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_LTE);

    }

    // 1. Set up the server to respond to the REGISTER request with a 504 Server Time-out.
    // 2. Configure the device via CarrierConfig to define the 504 response as a 'repeated'
    //    error type.
    // 3. Trigger IMS registration on the device.
    // 4. Verify that the device enters a deregistered state with the suggested action to
    //    block the PLMN with a timeout.
    @Test
    public void register_onLte_with504AsRepeatedError_triggersPlmnBlockWithTimeout()
            throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages("<504-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY - REG_ERROR_CODE_504
        mConfig.putIntArray(CarrierConfig.Ims.KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY,
                new int[] {REG_ERROR_CODE_504});

        // KEY_EXTRA_REG_ERR_FINAL_TYPE_INT - ERROR_TYPE_REPEATED
        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_REPEATED);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_LTE);
    }

    // 1. Set up the server to respond to the REGISTER request with a 504 Server Time-out
    //    and a Retry-After header.
    // 2. Configure the device via CarrierConfig to define the 504 response as a 'repeated'
    //    error type.
    // 3. Trigger IMS registration on the device.
    // 4. Verify that the device enters a deregistered state with the suggested action to
    //    block the PLMN with a timeout.
    @Test
    public void register_onLte_with504RetryAfter_triggersPlmnBlockWithTimeout()
            throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("504-REGISTER")
                .setHeader("Retry-After", "100")
                .build());
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY - REG_ERROR_CODE_504
        mConfig.putIntArray(CarrierConfig.Ims.KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY,
                new int[] {REG_ERROR_CODE_504});

        // KEY_EXTRA_REG_ERR_FINAL_TYPE_INT - ERROR_TYPE_REPEATED
        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_REPEATED);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_LTE);
    }

    // 1. Set up the server to complete a normal registration and subscription flow, and then
    //    expect a re-REGISTER.
    // 2. Trigger IMS registration on LTE.
    // 3. Verify that the device registers successfully on LTE.
    // 4. Simulate a data connection handover from LTE to IWLAN.
    // 5. Verify that the device successfully re-registers on IWLAN.
    @Test
    public void handover_fromLte_toWlan_succeeds() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(">REGISTER | <200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE"
                + "| >REGISTER | <200-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        RegistrationInfo regInfo = mInfoBuilder.build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_LTE);

        mRegistration.expect(3000).nothing();

        notifyPreciseDataConnectionState(getLtePreciseDataConnectionState(
                TelephonyManager.DATA_HANDOVER_IN_PROGRESS), regInfo.getSlotId());

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo.getSlotId());

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);
    }

    // 1. Set up the server to complete registration and expect a SUBSCRIBE request.
    // 2. Verify the received SUBSCRIBE request contains a P-Access-Network-Info header
    //    with '3GPP-E-UTRAN'.
    // 3. Trigger IMS registration on LTE.
    // 4. Verify that the device successfully registers on LTE.
    @Test
    public void subscribe_onLte_withDefaultConfig_succeeds() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages("<200-REGISTER");
        generator.addMessage(MessageBuildUtils.getDefaultSubscribe()
                .addRuleSet(new RuleSet.Builder("SUBSCRIBE : Valid P-Access-Network-Info header")
                        .addRule(new RuleSet.Rule.RuleBuilder("P-Access-Network-Info")
                                .addContainRule("3GPP-E-UTRAN")
                                .build())
                        .build())
                .build());
        generator.addMessages("<200-SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        RegistrationInfo regInfo = mInfoBuilder.build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_LTE);
    }

    // 1. The device completes registration and subscription.
    // 2. The server sends a NOTIFY message with event="unregistered" to terminate the registration.
    // 3. The device sends a 200 OK for the NOTIFY.
    // 4. Verify that the device is deregistered locally due to network-initiated detach.
    @Ignore("TISS support is required.")
    @Test
    public void deregister_onLte_byNotifyUnregistered_succeeds() throws Exception {
        // TODO: The following XML files must be included in TISS:
        /*
        File name : tiss/preferencefiles/xml/reg_notify_unregistered.txt
        <?xml version="1.0" encoding="UTF-8"?>
        <reginfo xmlns="urn:ietf:params:xml:ns:reginfo" version="1" state="full">
          <registration aor="sip:001011123456789@ims.mnc001.mcc001.3gppnetwork.org" id="a1"
              state="terminated">
            <contact id="1" state="terminated" event="unregistered">
              <uri>sip:ue_instance@192.168.200.44:5060</uri>
              <unknown-param name="reason">Registration Unregistered by Network</unknown-param>
            </contact>
          </registration>
        </reginfo>
        */

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("NOTIFY-SUBSCRIBE")
                .setXml("reg_notify_unregistered")
                .build());
        generator.addMessages(">200-NOTIFY");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        RegistrationInfo regInfo = mInfoBuilder.build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_LTE);

        mRegistration.expect().deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_LTE);
    }

    // 1. The device completes registration and subscription.
    // 2. The server sends a NOTIFY message with event="unregistered".
    // 3. Configure the device to re-register upon receiving a NOTIFY with "unregistered" event.
    // 4. Verify that the device is deregistered locally.
    // 5. Verify that the device triggers a new registration and successfully registers again.
    @Ignore("TISS support is required.")
    @Test
    public void deregister_onLte_byNotifyUnregistered_triggersRegistration()
            throws Exception {
        // TODO: The following XML files must be included in TISS:
        /*
        File name : tiss/preferencefiles/xml/reg_notify_unregistered.txt
        <?xml version="1.0" encoding="UTF-8"?>
        <reginfo xmlns="urn:ietf:params:xml:ns:reginfo" version="1" state="full">
          <registration aor="sip:001011123456789@ims.mnc001.mcc001.3gppnetwork.org" id="a1"
              state="terminated">
            <contact id="1" state="terminated" event="unregistered">
              <uri>sip:ue_instance@192.168.200.44:5060</uri>
              <unknown-param name="reason">Registration Unregistered by Network</unknown-param>
            </contact>
          </registration>
        </reginfo>
        */

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("NOTIFY-SUBSCRIBE")
                .setXml("reg_notify_unregistered")
                .build());
        generator.addMessages(">200-NOTIFY");
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY - NOTIFY_TERMINATED_UNREGISTERED
        PersistableBundle objNotifyTerminatedForInitRegUsedEvenBundle = new PersistableBundle();
        objNotifyTerminatedForInitRegUsedEvenBundle.putIntArray(
                CarrierConfig.Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY,
                new int[] {NOTIFY_TERMINATED_UNREGISTERED});
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_BUNDLE,
                objNotifyTerminatedForInitRegUsedEvenBundle);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_LTE);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_LTE);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_LTE);
    }

    // 1. The device completes registration and subscription.
    // 2. The server sends a NOTIFY message with event="rejected" to terminate the registration.
    // 3. The device sends a 200 OK for the NOTIFY.
    // 4. Verify that the device is deregistered locally due to network-initiated detach.
    @Ignore("TISS support is required.")
    @Test
    public void deregister_onLte_byNotifyRejected_succeeds() throws Exception {
        // TODO: The following XML files must be included in TISS:
        /*
        File name : tiss/preferencefiles/xml/reg_notify_rejected.txt
        <?xml version="1.0" encoding="UTF-8"?>
        <reginfo xmlns="urn:ietf:params:xml:ns:reginfo" version="1" state="full">
          <registration aor="sip:001011123456789@ims.mnc001.mcc001.3gppnetwork.org" id="a1"
              state="terminated">
            <contact id="1" state="terminated" event="rejected">
              <uri>sip:ue_instance@192.168.200.44:5060</uri>
              <unknown-param name="reason">Registration Rejected by Network</unknown-param>
            </contact>
          </registration>
        </reginfo>
        */

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("NOTIFY-SUBSCRIBE")
                .setXml("reg_notify_rejected")
                .build());
        generator.addMessages(">200-NOTIFY");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        RegistrationInfo regInfo = mInfoBuilder.build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_LTE);
    }

    // 1. The device completes registration and subscription.
    // 2. The server sends a NOTIFY message with event="rejected".
    // 3. Configure the device to re-register upon receiving a NOTIFY with "rejected" event.
    // 4. Verify that the device is deregistered locally.
    // 5. Verify that the device triggers a new registration and successfully registers again.
    @Ignore("TISS support is required.")
    @Test
    public void deregister_onLte_byNotifyRejected_triggersRegistration()
            throws Exception {
        // TODO: The following XML files must be included in TISS:
        /*
        File name : tiss/preferencefiles/xml/reg_notify_rejected.txt
        <?xml version="1.0" encoding="UTF-8"?>
        <reginfo xmlns="urn:ietf:params:xml:ns:reginfo" version="1" state="full">
          <registration aor="sip:001011123456789@ims.mnc001.mcc001.3gppnetwork.org" id="a1"
              state="terminated">
            <contact id="1" state="terminated" event="rejected">
              <uri>sip:ue_instance@192.168.200.44:5060</uri>
              <unknown-param name="reason">Registration Rejected by Network</unknown-param>
            </contact>
          </registration>
        </reginfo>
        */

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("NOTIFY-SUBSCRIBE")
                .setXml("reg_notify_rejected")
                .build());
        generator.addMessages(">200-NOTIFY");
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY - NOTIFY_TERMINATED_REJECTED
        PersistableBundle objNotifyTerminatedForInitRegUsedEvenBundle = new PersistableBundle();
        objNotifyTerminatedForInitRegUsedEvenBundle.putIntArray(
                CarrierConfig.Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY,
                new int[] {NOTIFY_TERMINATED_REJECTED});
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_BUNDLE,
                objNotifyTerminatedForInitRegUsedEvenBundle);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_LTE);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_LTE);
    }

    // 1. The device completes registration and subscription.
    // 2. The server sends a NOTIFY message with event="deactivated" to terminate the registration.
    // 3. The device sends a 200 OK for the NOTIFY.
    // 4. Verify that the device is deregistered locally due to network-initiated detach.
    @Ignore("TISS support is required.")
    @Test
    public void deregister_onLte_byNotifyDeactivated_succeeds() throws Exception {
        // TODO: The following XML files must be included in TISS:
        /*
        File name : tiss/preferencefiles/xml/reg_notify_deactivated.txt
        <?xml version="1.0" encoding="UTF-8"?>
        <reginfo xmlns="urn:ietf:params:xml:ns:reginfo" version="1" state="full">
          <registration aor="sip:001011123456789@ims.mnc001.mcc001.3gppnetwork.org" id="a1"
              state="terminated">
            <contact id="1" state="terminated" event="deactivated">
              <uri>sip:ue_instance@192.168.200.44:5060</uri>
              <unknown-param name="reason">Registration Deactivated by Network</unknown-param>
            </contact>
          </registration>
        </reginfo>
        */

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("NOTIFY-SUBSCRIBE")
                .setXml("reg_notify_deactivated")
                .build());
        generator.addMessages(">200-NOTIFY");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        RegistrationInfo regInfo = mInfoBuilder.build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_LTE);
    }

    // 1. The device completes registration and subscription.
    // 2. The server sends a NOTIFY message with event="deactivated".
    // 3. Configure the device to re-register upon receiving a NOTIFY with "deactivated" event.
    // 4. Verify that the device is deregistered locally.
    // 5. Verify that the device triggers a new registration and successfully registers again.
    @Ignore("TISS support is required.")
    @Test
    public void deregister_onLte_byNotifyDeactivated_triggersRegistration()
            throws Exception {
        // TODO: The following XML files must be included in TISS:
        /*
        File name : tiss/preferencefiles/xml/reg_notify_deactivated.txt
        <?xml version="1.0" encoding="UTF-8"?>
        <reginfo xmlns="urn:ietf:params:xml:ns:reginfo" version="1" state="full">
          <registration aor="sip:001011123456789@ims.mnc001.mcc001.3gppnetwork.org" id="a1"
              state="terminated">
            <contact id="1" state="terminated" event="deactivated">
              <uri>sip:ue_instance@192.168.200.44:5060</uri>
              <unknown-param name="reason">Registration Deactivated by Network</unknown-param>
            </contact>
          </registration>
        </reginfo>
        */

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("NOTIFY-SUBSCRIBE")
                .setXml("reg_notify_deactivated")
                .build());
        generator.addMessages(">200-NOTIFY");
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);

        mServerControlConnection.sendControlCommand(generator.build().toString());

        // KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY - NOTIFY_TERMINATED_DEACTIVATED
        PersistableBundle objNotifyTerminatedForInitRegUsedEvenBundle = new PersistableBundle();
        objNotifyTerminatedForInitRegUsedEvenBundle.putIntArray(
                CarrierConfig.Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY,
                new int[] {NOTIFY_TERMINATED_DEACTIVATED});
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_BUNDLE,
                objNotifyTerminatedForInitRegUsedEvenBundle);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_LTE);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_LTE);
    }

    // 1. Set up the server to expect a REGISTER request and verify its Contact header includes
    //    MMTEL, audio, video, and SMSIP features.
    // 2. The server then completes the registration and subscription flow.
    // 3. Configure the device to be on an NR network.
    // 4. Trigger IMS registration.
    // 5. Verify that the device successfully registers on NR.
    @Test
    public void register_onNr_withDefaultConfig_succeeds() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister()
                .addRuleSet(new RuleSet.Builder("REGISTER(1st) : Valid Contact header")
                        .addRule(new RuleSet.Rule.RuleBuilder("Contact")
                                .addContainRule("+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service"
                                        + ".ims.icsi.mmtel")
                                .addContainRule("audio")
                                .addContainRule("video")
                                .addContainRule("+g.3gpp.smsip")
                                .build())
                        .build())
                .build());
        generator.addMessages("<200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        RegistrationInfo regInfo = mInfoBuilder
                .setServiceState(buildNrServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_NR)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_NR);
    }

    // 1. Set up the server to expect a REGISTER request and verify its Contact header includes
    //    MMTEL and audio, but not video or SMSIP.
    // 2. The server then completes the registration and subscription flow.
    // 3. Configure the device to support only the voice capability in an NR network.
    // 4. Trigger IMS registration.
    // 5. Verify that the device successfully registers on NR.
    @Test
    public void register_onNr_withVoiceOnly_succeeds() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister()
                .addRuleSet(new RuleSet.Builder("REGISTER(1st) : Valid Contact header")
                        .addRule(new RuleSet.Rule.RuleBuilder("Contact")
                                .addContainRule("+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service"
                                        + ".ims.icsi.mmtel")
                                .addContainRule("audio")
                                .addNotContainRule("video")
                                .addNotContainRule("+g.3gpp.smsip")
                                .build())
                        .build())
                .build());
        generator.addMessages("<200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_SUPPORTED_BOOL, false);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildNrServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_NR)
                .setEnableCapability(CAPABILITY_TYPE_VOICE,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setDisableCapability(CAPABILITY_TYPE_VIDEO,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setDisableCapability(CAPABILITY_TYPE_SMS,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_NR);
    }

    // 1. Set up the server to expect a REGISTER request and verify its Contact header includes
    //    MMTEL, audio, and video, but not SMSIP.
    // 2. The server then completes the registration and subscription flow.
    // 3. Configure the device to support voice and video capabilities in an NR network.
    // 4. Trigger IMS registration.
    // 5. Verify that the device successfully registers on NR.
    @Test
    public void register_onNr_withVoiceVideo_succeeds() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister()
                .addRuleSet(new RuleSet.Builder("REGISTER(1st) : Valid Contact header")
                        .addRule(new RuleSet.Rule.RuleBuilder("Contact")
                                .addContainRule("+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service"
                                        + ".ims.icsi.mmtel")
                                .addContainRule("audio")
                                .addContainRule("video")
                                .addNotContainRule("+g.3gpp.smsip")
                                .build())
                        .build())
                .build());
        generator.addMessages("<200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_SUPPORTED_BOOL, false);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildNrServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_NR)
                .setEnableCapability(CAPABILITY_TYPE_VOICE,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setEnableCapability(CAPABILITY_TYPE_VIDEO,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setDisableCapability(CAPABILITY_TYPE_SMS,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_NR);
    }

    // 1. Set up the server to expect a REGISTER request and verify its Contact header includes
    //    MMTEL, audio, and SMSIP, but not video.
    // 2. The server then completes the registration and subscription flow.
    // 3. Configure the device to support voice and SMS capabilities in an NR network.
    // 4. Trigger IMS registration.
    // 5. Verify that the device successfully registers on NR.
    @Test
    public void register_onNr_withVoiceSms_succeeds() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister()
                .addRuleSet(new RuleSet.Builder("REGISTER(1st) : Valid Contact header")
                        .addRule(new RuleSet.Rule.RuleBuilder("Contact")
                                .addContainRule("+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service"
                                        + ".ims.icsi.mmtel")
                                .addContainRule("audio")
                                .addNotContainRule("video")
                                .addContainRule("+g.3gpp.smsip")
                                .build())
                        .build())
                .build());
        generator.addMessages("<200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        RegistrationInfo regInfo = mInfoBuilder
                .setServiceState(buildNrServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_NR)
                .setEnableCapability(CAPABILITY_TYPE_VOICE,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setDisableCapability(CAPABILITY_TYPE_VIDEO,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setEnableCapability(CAPABILITY_TYPE_SMS,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_NR);
    }

    // 1. Set up the server to respond to the first REGISTER request with a 423 Interval Too Brief.
    // 2. The server then expects a new REGISTER request and completes the normal registration flow.
    // 3. Trigger IMS registration on the device in an NR network.
    // 4. Verify that the device retries registration and successfully registers on NR.
    @Test
    public void register_onNr_with423Response_retriesAndSucceeds() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages(
                "<423-REGISTER | >REGISTER | <200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        RegistrationInfo regInfo = mInfoBuilder
                .setServiceState(buildNrServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_NR)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_NR);
    }

    // 1. Set up the server to respond to the REGISTER request with a 403 Forbidden.
    // 2. Configure the device via CarrierConfig to define the 403 response as a 'critical'
    //    error type, which results in blocking the current PLMN.
    // 3. Trigger IMS registration on the device in an NR network.
    // 4. Verify that the device enters a deregistered state with the suggested action to
    //    block the PLMN.
    @Test
    public void register_onNr_with403AsCriticalError_triggersPlmnBlock() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages("<403-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY - REG_ERROR_CODE_403
        mConfig.putIntArray(CarrierConfig.Ims.KEY_REGISTRATION_PERMANENT_ERROR_CODE_INT_ARRAY,
                new int[] {REG_ERROR_CODE_403});

        // KEY_EXTRA_REG_ERR_FINAL_TYPE_INT - ERROR_TYPE_CRITICAL
        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_CRITICAL);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildNrServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_NR)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK,
                networkType -> networkType == REGISTRATION_TECH_NR);
    }

    // 1. Set up the server to respond to the REGISTER request with a 404 Not Found.
    // 2. Configure the device via CarrierConfig to define the 404 response as a 'critical'
    //    error type, which results in blocking the current PLMN.
    // 3. Trigger IMS registration on the device in an NR network.
    // 4. Verify that the device enters a deregistered state with the suggested action to
    //    block the PLMN.
    @Test
    public void register_onNr_with404AsCriticalError_triggersPlmnBlock() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages("<404-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY - REG_ERROR_CODE_404
        mConfig.putIntArray(CarrierConfig.Ims.KEY_REGISTRATION_PERMANENT_ERROR_CODE_INT_ARRAY,
                new int[] {REG_ERROR_CODE_404});

        // KEY_EXTRA_REG_ERR_FINAL_TYPE_INT - ERROR_TYPE_CRITICAL
        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_CRITICAL);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildNrServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_NR)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK,
                networkType -> networkType == REGISTRATION_TECH_NR);
    }

    // 1. Set up the server to respond to the REGISTER request with a 500 Server Internal Error.
    // 2. Configure the device via CarrierConfig to define the 5xx response as a 'repeated'
    //    error type, which results in blocking the PLMN with a specific protocol timer.
    // 3. Trigger IMS registration on the device in an NR network.
    // 4. Verify that the device enters a deregistered state with the suggested action to
    //    block the PLMN with a timeout.
    @Test
    public void register_onNr_with500AsRepeatedError_triggersPlmnBlockWithTimeout()
            throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages("<500-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY - REG_ERROR_CODE_5XX
        mConfig.putIntArray(CarrierConfig.Ims.KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY,
                new int[] {REG_ERROR_CODE_5XX});

        // KEY_EXTRA_REG_ERR_FINAL_TYPE_INT - ERROR_TYPE_REPEATED
        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_REPEATED);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

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
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_NR);
    }

    // 1. Set up the server to respond to the REGISTER request with a 503 Service Unavailable.
    // 2. Configure the device via CarrierConfig to define the 503 response as a 'repeated'
    //    error type, which results in blocking the PLMN with a specific protocol timer.
    // 3. Trigger IMS registration on the device in an NR network.
    // 4. Verify that the device enters a deregistered state with the suggested action to
    //    block the PLMN with a timeout.
    @Test
    public void register_onNr_with503AsRepeatedError_triggersPlmnBlockWithTimeout()
            throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages("<503-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY - REG_ERROR_CODE_503
        mConfig.putIntArray(CarrierConfig.Ims.KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY,
                new int[] {REG_ERROR_CODE_503});

        // KEY_EXTRA_REG_ERR_FINAL_TYPE_INT - ERROR_TYPE_REPEATED
        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_REPEATED);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildNrServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_NR)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        notifyPreciseDataConnectionState(getNrPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo.getSlotId());

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_NR);
    }

    // 1. Set up the server to respond to the REGISTER request with a 503 Service Unavailable
    //    and a Retry-After header.
    // 2. Configure the device via CarrierConfig to define the 503 response as a 'repeated'
    //    error type.
    // 3. Trigger IMS registration on the device in an NR network.
    // 4. Verify that the device enters a deregistered state with the suggested action to
    //    block the PLMN with a timeout.
    @Test
    public void register_onNr_with503RetryAfter_triggersPlmnBlockWithTimeout()
            throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("503-REGISTER")
                .setHeader("Retry-After", "100")
                .build());
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY - REG_ERROR_CODE_503
        mConfig.putIntArray(CarrierConfig.Ims.KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY,
                new int[] {REG_ERROR_CODE_503});

        // KEY_EXTRA_REG_ERR_FINAL_TYPE_INT - ERROR_TYPE_REPEATED
        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_REPEATED);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

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
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_NR);
    }

    // 1. Set up the server to respond to the REGISTER request with a 504 Server Time-out.
    // 2. Configure the device via CarrierConfig to define the 504 response as a 'repeated'
    //    error type.
    // 3. Trigger IMS registration on the device in an NR network.
    // 4. Verify that the device enters a deregistered state with the suggested action to
    //    block the PLMN with a timeout.
    @Test
    public void register_onNr_with504AsRepeatedError_triggersPlmnBlockWithTimeout()
            throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages("<504-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY - REG_ERROR_CODE_504
        mConfig.putIntArray(CarrierConfig.Ims.KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY,
                new int[] {REG_ERROR_CODE_504});

        // KEY_EXTRA_REG_ERR_FINAL_TYPE_INT - ERROR_TYPE_REPEATED
        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_REPEATED);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

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
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_NR);
    }

    // 1. Set up the server to respond to the REGISTER request with a 504 Server Time-out
    //    and a Retry-After header.
    // 2. Configure the device via CarrierConfig to define the 504 response as a 'repeated'
    //    error type.
    // 3. Trigger IMS registration on the device in an NR network.
    // 4. Verify that the device enters a deregistered state with the suggested action to
    //    block the PLMN with a timeout.
    @Test
    public void register_onNr_with504RetryAfter_triggersPlmnBlockWithTimeout()
            throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("504-REGISTER")
                .setHeader("Retry-After", "100")
                .build());
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY - REG_ERROR_CODE_504
        mConfig.putIntArray(CarrierConfig.Ims.KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY,
                new int[] {REG_ERROR_CODE_504});

        // KEY_EXTRA_REG_ERR_FINAL_TYPE_INT - ERROR_TYPE_REPEATED
        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_REPEATED);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

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
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_NR);
    }

    // 1. Set up the server to complete a normal registration and subscription flow, and then
    //    expect a re-REGISTER.
    // 2. Trigger IMS registration on NR.
    // 3. Verify that the device registers successfully on NR.
    // 4. Simulate a data connection handover from NR to IWLAN.
    // 5. Verify that the device successfully re-registers on IWLAN.
    @Test
    public void handover_fromNr_toWlan_succeeds() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(">REGISTER | <200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE"
                + "| >REGISTER | <200-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        RegistrationInfo regInfo = mInfoBuilder
                .setServiceState(buildNrServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_NR)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_NR);

        mRegistration.expect(3000).nothing();

        notifyPreciseDataConnectionState(getNrPreciseDataConnectionState(
                TelephonyManager.DATA_HANDOVER_IN_PROGRESS), regInfo.getSlotId());

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo.getSlotId());

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);
    }

    // 1. Set up the server to complete registration and expect a SUBSCRIBE request.
    // 2. Verify the received SUBSCRIBE request contains a P-Access-Network-Info header
    //    with '3GPP-NR'.
    // 3. Trigger IMS registration on NR.
    // 4. Verify that the device successfully registers on NR.
    @Test
    public void subscribe_onNr_withDefaultConfig_succeeds() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages("<200-REGISTER");
        generator.addMessage(MessageBuildUtils.getDefaultSubscribe()
                .addRuleSet(new RuleSet.Builder("SUBSCRIBE : Valid P-Access-Network-Info header")
                        .addRule(new RuleSet.Rule.RuleBuilder("P-Access-Network-Info")
                                .addContainRule("3GPP-NR")
                                .build())
                        .build())
                .build());
        generator.addMessages("<200-SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        RegistrationInfo regInfo = mInfoBuilder
                .setServiceState(buildNrServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_NR)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_NR);
    }

    // 1. The device completes registration and subscription on NR.
    // 2. The server sends a NOTIFY message with event="unregistered" to terminate the registration.
    // 3. The device sends a 200 OK for the NOTIFY.
    // 4. Verify that the device is deregistered locally due to network-initiated detach.
    @Ignore("TISS support is required.")
    @Test
    public void deregister_onNr_byNotifyUnregistered_succeeds() throws Exception {
        // TODO: The following XML files must be included in TISS:
        /*
        File name : tiss/preferencefiles/xml/reg_notify_unregistered.txt
        <?xml version="1.0" encoding="UTF-8"?>
        <reginfo xmlns="urn:ietf:params:xml:ns:reginfo" version="1" state="full">
          <registration aor="sip:001011123456789@ims.mnc001.mcc001.3gppnetwork.org" id="a1"
              state="terminated">
            <contact id="1" state="terminated" event="unregistered">
              <uri>sip:ue_instance@192.168.200.44:5060</uri>
              <unknown-param name="reason">Registration Unregistered by Network</unknown-param>
            </contact>
          </registration>
        </reginfo>
        */

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("NOTIFY-SUBSCRIBE")
                .setXml("reg_notify_unregistered")
                .build());
        generator.addMessages(">200-NOTIFY");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        RegistrationInfo regInfo = mInfoBuilder
                .setServiceState(buildNrServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_NR)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_NR);
    }

    // 1. The device completes registration and subscription on NR.
    // 2. The server sends a NOTIFY message with event="unregistered".
    // 3. Configure the device to re-register upon receiving a NOTIFY with "unregistered" event.
    // 4. Verify that the device is deregistered locally.
    // 5. Verify that the device triggers a new registration and successfully registers again on NR.
    @Ignore("TISS support is required.")
    @Test
    public void deregister_onNr_byNotifyUnregistered_triggersRegistration()
            throws Exception {
        // TODO: The following XML files must be included in TISS:
        /*
        File name : tiss/preferencefiles/xml/reg_notify_unregistered.txt
        <?xml version="1.0" encoding="UTF-8"?>
        <reginfo xmlns="urn:ietf:params:xml:ns:reginfo" version="1" state="full">
          <registration aor="sip:001011123456789@ims.mnc001.mcc001.3gppnetwork.org" id="a1"
              state="terminated">
            <contact id="1" state="terminated" event="unregistered">
              <uri>sip:ue_instance@192.168.200.44:5060</uri>
              <unknown-param name="reason">Registration Unregistered by Network</unknown-param>
            </contact>
          </registration>
        </reginfo>
        */

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("NOTIFY-SUBSCRIBE")
                .setXml("reg_notify_unregistered")
                .build());
        generator.addMessages(">200-NOTIFY");
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY - NOTIFY_TERMINATED_UNREGISTERED
        PersistableBundle objNotifyTerminatedForInitRegUsedEvenBundle = new PersistableBundle();
        objNotifyTerminatedForInitRegUsedEvenBundle.putIntArray(
                CarrierConfig.Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY,
                new int[] {NOTIFY_TERMINATED_UNREGISTERED});
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_BUNDLE,
                objNotifyTerminatedForInitRegUsedEvenBundle);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildNrServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_NR)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_NR);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_NR);
    }

    // 1. The device completes registration and subscription on NR.
    // 2. The server sends a NOTIFY message with event="rejected" to terminate the registration.
    // 3. The device sends a 200 OK for the NOTIFY.
    // 4. Verify that the device is deregistered locally due to network-initiated detach.
    @Ignore("TISS support is required.")
    @Test
    public void deregister_onNr_byNotifyRejected_succeeds() throws Exception {
        // TODO: The following XML files must be included in TISS:
        /*
        File name : tiss/preferencefiles/xml/reg_notify_rejected.txt
        <?xml version="1.0" encoding="UTF-8"?>
        <reginfo xmlns="urn:ietf:params:xml:ns:reginfo" version="1" state="full">
          <registration aor="sip:001011123456789@ims.mnc001.mcc001.3gppnetwork.org" id="a1"
              state="terminated">
            <contact id="1" state="terminated" event="rejected">
              <uri>sip:ue_instance@192.168.200.44:5060</uri>
              <unknown-param name="reason">Registration Rejected by Network</unknown-param>
            </contact>
          </registration>
        </reginfo>
        */

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("NOTIFY-SUBSCRIBE")
                .setXml("reg_notify_rejected")
                .build());
        generator.addMessages(">200-NOTIFY");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        RegistrationInfo regInfo = mInfoBuilder
                .setServiceState(buildNrServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_NR)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_NR);
    }

    // 1. The device completes registration and subscription on NR.
    // 2. The server sends a NOTIFY message with event="rejected".
    // 3. Configure the device to re-register upon receiving a NOTIFY with "rejected" event.
    // 4. Verify that the device is deregistered locally.
    // 5. Verify that the device triggers a new registration and successfully registers again on NR.
    @Ignore("TISS support is required.")
    @Test
    public void deregister_onNr_byNotifyRejected_triggersRegistration()
            throws Exception {
        // TODO: The following XML files must be included in TISS:
        /*
        File name : tiss/preferencefiles/xml/reg_notify_rejected.txt
        <?xml version="1.0" encoding="UTF-8"?>
        <reginfo xmlns="urn:ietf:params:xml:ns:reginfo" version="1" state="full">
          <registration aor="sip:001011123456789@ims.mnc001.mcc001.3gppnetwork.org" id="a1"
              state="terminated">
            <contact id="1" state="terminated" event="rejected">
              <uri>sip:ue_instance@192.168.200.44:5060</uri>
              <unknown-param name="reason">Registration Rejected by Network</unknown-param>
            </contact>
          </registration>
        </reginfo>
        */

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("NOTIFY-SUBSCRIBE")
                .setXml("reg_notify_rejected")
                .build());
        generator.addMessages(">200-NOTIFY");
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY - NOTIFY_TERMINATED_REJECTED
        PersistableBundle objNotifyTerminatedForInitRegUsedEvenBundle = new PersistableBundle();
        objNotifyTerminatedForInitRegUsedEvenBundle.putIntArray(
                CarrierConfig.Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY,
                new int[] {NOTIFY_TERMINATED_REJECTED});
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_BUNDLE,
                objNotifyTerminatedForInitRegUsedEvenBundle);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildNrServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_NR)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_NR);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_NR);
    }

    // 1. The device completes registration and subscription on NR.
    // 2. The server sends a NOTIFY message with event="deactivated" to terminate the registration.
    // 3. The device sends a 200 OK for the NOTIFY.
    // 4. Verify that the device is deregistered locally due to network-initiated detach.
    @Ignore("TISS support is required.")
    @Test
    public void deregister_onNr_byNotifyDeactivated_succeeds() throws Exception {
        // TODO: The following XML files must be included in TISS:
        /*
        File name : tiss/preferencefiles/xml/reg_notify_deactivated.txt
        <?xml version="1.0" encoding="UTF-8"?>
        <reginfo xmlns="urn:ietf:params:xml:ns:reginfo" version="1" state="full">
          <registration aor="sip:001011123456789@ims.mnc001.mcc001.3gppnetwork.org" id="a1"
              state="terminated">
            <contact id="1" state="terminated" event="deactivated">
              <uri>sip:ue_instance@192.168.200.44:5060</uri>
              <unknown-param name="reason">Registration Deactivated by Network</unknown-param>
            </contact>
          </registration>
        </reginfo>
        */

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("NOTIFY-SUBSCRIBE")
                .setXml("reg_notify_deactivated")
                .build());
        generator.addMessages(">200-NOTIFY");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        RegistrationInfo regInfo = mInfoBuilder
                .setServiceState(buildNrServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_NR)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_NR);
    }

    // 1. The device completes registration and subscription on NR.
    // 2. The server sends a NOTIFY message with event="deactivated".
    // 3. Configure the device to re-register upon receiving a NOTIFY with "deactivated" event.
    // 4. Verify that the device is deregistered locally.
    // 5. Verify that the device triggers a new registration and successfully registers again on NR.
    @Ignore("TISS support is required.")
    @Test
    public void deregister_onNr_byNotifyDeactivated_triggersRegistration()
            throws Exception {
        // TODO: The following XML files must be included in TISS:
        /*
        File name : tiss/preferencefiles/xml/reg_notify_deactivated.txt
        <?xml version="1.0" encoding="UTF-8"?>
        <reginfo xmlns="urn:ietf:params:xml:ns:reginfo" version="1" state="full">
          <registration aor="sip:001011123456789@ims.mnc001.mcc001.3gppnetwork.org" id="a1"
              state="terminated">
            <contact id="1" state="terminated" event="deactivated">
              <uri>sip:ue_instance@192.168.200.44:5060</uri>
              <unknown-param name="reason">Registration Deactivated by Network</unknown-param>
            </contact>
          </registration>
        </reginfo>
        */

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("NOTIFY-SUBSCRIBE")
                .setXml("reg_notify_deactivated")
                .build());
        generator.addMessages(">200-NOTIFY");
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY - NOTIFY_TERMINATED_DEACTIVATED
        PersistableBundle objNotifyTerminatedForInitRegUsedEvenBundle = new PersistableBundle();
        objNotifyTerminatedForInitRegUsedEvenBundle.putIntArray(
                CarrierConfig.Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY,
                new int[] {NOTIFY_TERMINATED_DEACTIVATED});
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_BUNDLE,
                objNotifyTerminatedForInitRegUsedEvenBundle);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildNrServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_NR)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_NR);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_NR);
    }

    // 1. Set up the server to expect a REGISTER request and verify its Contact header includes
    //    MMTEL, audio, video, and SMSIP features.
    // 2. The server then completes the registration and subscription flow.
    // 3. Configure the device to allow IMS registration over WLAN.
    // 4. Trigger IMS registration on the device in a WLAN network.
    // 5. Verify that the device successfully registers on IWLAN.
    @Test
    public void register_onWlan_withDefaultConfig_succeeds() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister()
                .addRuleSet(new RuleSet.Builder("REGISTER(1st) : Valid Contact header")
                        .addRule(new RuleSet.Rule.RuleBuilder("Contact")
                                .addContainRule("+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service"
                                        + ".ims.icsi.mmtel")
                                .addContainRule("audio")
                                .addContainRule("video")
                                .addContainRule("+g.3gpp.smsip")
                                .build())
                        .build())
                .build());
        generator.addMessages("<200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildLteIwlanServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);
    }

    // 1. Set up the server to expect a REGISTER request and verify its Contact header includes
    //    MMTEL and audio, but not video or SMSIP.
    // 2. The server then completes the registration and subscription flow.
    // 3. Configure the device for Wi-Fi calling, supporting only the voice capability.
    // 4. Trigger IMS registration on IWLAN.
    // 5. Verify that the device successfully registers on IWLAN.
    @Test
    public void register_onWlan_withVoiceOnly_succeeds() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister()
                .addRuleSet(new RuleSet.Builder("REGISTER(1st) : Valid Contact header")
                        .addRule(new RuleSet.Rule.RuleBuilder("Contact")
                                .addContainRule("+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service"
                                        + ".ims.icsi.mmtel")
                                .addContainRule("audio")
                                .addNotContainRule("video")
                                .addNotContainRule("+g.3gpp.smsip")
                                .build())
                        .build())
                .build());
        generator.addMessages("<200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_SUPPORTED_BOOL, false);
        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildLteIwlanServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_IWLAN)
                .setEnableCapability(CAPABILITY_TYPE_VOICE,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setDisableCapability(CAPABILITY_TYPE_VIDEO,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setDisableCapability(CAPABILITY_TYPE_SMS,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);
    }

    // 1. Set up the server to expect a REGISTER request and verify its Contact header includes
    //    MMTEL, audio, and video, but not SMSIP.
    // 2. The server then completes the registration and subscription flow.
    // 3. Configure the device for Wi-Fi calling, supporting voice and video capabilities.
    // 4. Trigger IMS registration on IWLAN.
    // 5. Verify that the device successfully registers on IWLAN.
    @Test
    public void register_onWlan_withVoiceVideo_succeeds() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister()
                .addRuleSet(new RuleSet.Builder("REGISTER(1st) : Valid Contact header")
                        .addRule(new RuleSet.Rule.RuleBuilder("Contact")
                                .addContainRule("+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service"
                                        + ".ims.icsi.mmtel")
                                .addContainRule("audio")
                                .addContainRule("video")
                                .addNotContainRule("+g.3gpp.smsip")
                                .build())
                        .build())
                .build());
        generator.addMessages("<200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_SUPPORTED_BOOL, false);
        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildLteIwlanServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_IWLAN)
                .setEnableCapability(CAPABILITY_TYPE_VOICE,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setEnableCapability(CAPABILITY_TYPE_VIDEO,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setDisableCapability(CAPABILITY_TYPE_SMS,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);
    }

    // 1. Set up the server to expect a REGISTER request and verify its Contact header includes
    //    MMTEL, audio, and SMSIP, but not video.
    // 2. The server then completes the registration and subscription flow.
    // 3. Configure the device for Wi-Fi calling, supporting voice and SMS capabilities.
    // 4. Trigger IMS registration on IWLAN.
    // 5. Verify that the device successfully registers on IWLAN.
    @Test
    public void register_onWlan_withVoiceSms_succeeds() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister()
                .addRuleSet(new RuleSet.Builder("REGISTER(1st) : Valid Contact header")
                        .addRule(new RuleSet.Rule.RuleBuilder("Contact")
                                .addContainRule("+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service"
                                        + ".ims.icsi.mmtel")
                                .addContainRule("audio")
                                .addNotContainRule("video")
                                .addContainRule("+g.3gpp.smsip")
                                .build())
                        .build())
                .build());
        generator.addMessages("<200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildLteIwlanServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_IWLAN)
                .setEnableCapability(CAPABILITY_TYPE_VOICE,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setDisableCapability(CAPABILITY_TYPE_VIDEO,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setEnableCapability(CAPABILITY_TYPE_SMS,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);
    }

    // 1. Set up the server to respond to the first REGISTER request with a 423 Interval Too Brief.
    // 2. The server then expects a new REGISTER request and completes the normal registration flow.
    // 3. Trigger IMS registration on the device over IWLAN.
    // 4. Verify that the device retries registration and successfully registers on IWLAN.
    @Test
    public void register_onWlan_with423Response_retriesAndSucceeds() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages(
                "<423-REGISTER | >REGISTER | <200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildLteIwlanServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);
    }

    // 1. Set up the server to respond to the REGISTER request with a 403 Forbidden.
    // 2. Configure the device via CarrierConfig to define the 403 response as a 'critical'
    //    error type, which results in blocking the current PLMN.
    // 3. Trigger IMS registration on the device over IWLAN.
    // 4. Verify that the device enters a deregistered state with the suggested action to
    //    block the PLMN.
    @Test
    public void register_onWlan_with403AsCriticalError_triggersPlmnBlock()
            throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages("<403-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        // KEY_REGISTRATION_PERMANENT_ERROR_CODE_INT_ARRAY - REG_ERROR_CODE_403
        mConfig.putIntArray(CarrierConfig.Ims.KEY_REGISTRATION_PERMANENT_ERROR_CODE_INT_ARRAY,
                new int[] {REG_ERROR_CODE_403});

        // KEY_EXTRA_REG_ERR_FINAL_TYPE_INT - ERROR_TYPE_CRITICAL
        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_CRITICAL);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildLteIwlanServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);
    }

    // 1. Set up the server to respond to the REGISTER request with a 404 Not Found.
    // 2. Configure the device via CarrierConfig to define the 404 response as a 'critical'
    //    error type, which results in blocking the current PLMN.
    // 3. Trigger IMS registration on the device over IWLAN.
    // 4. Verify that the device enters a deregistered state with the suggested action to
    //    block the PLMN.
    @Test
    public void register_onWlan_with404AsCriticalError_triggersPlmnBlock()
            throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages("<404-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        // KEY_REGISTRATION_PERMANENT_ERROR_CODE_INT_ARRAY - REG_ERROR_CODE_404
        mConfig.putIntArray(CarrierConfig.Ims.KEY_REGISTRATION_PERMANENT_ERROR_CODE_INT_ARRAY,
                new int[] {REG_ERROR_CODE_404});

        // KEY_EXTRA_REG_ERR_FINAL_TYPE_INT - ERROR_TYPE_CRITICAL
        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_CRITICAL);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildLteIwlanServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);
    }

    // 1. Set up the server to respond to the REGISTER request with a 500 Server Internal Error.
    // 2. Configure the device via CarrierConfig to define the 5xx response as a 'repeated'
    //    error type, which results in blocking the PLMN with a specific protocol timer.
    // 3. Trigger IMS registration on the device over IWLAN.
    // 4. Verify that the device enters a deregistered state with the suggested action to
    //    block the PLMN with a timeout.
    @Test
    public void register_onWlan_with500AsRepeatedError_triggersPlmnBlockWithTimeout()
            throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages("<500-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        // KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY - REG_ERROR_CODE_5XX
        mConfig.putIntArray(CarrierConfig.Ims.KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY,
                new int[] {REG_ERROR_CODE_5XX});

        // KEY_EXTRA_REG_ERR_FINAL_TYPE_INT - ERROR_TYPE_REPEATED
        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_REPEATED);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

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
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);
    }

    // 1. Set up the server to respond to the REGISTER request with a 503 Service Unavailable.
    // 2. Configure the device via CarrierConfig to define the 503 response as a 'repeated'
    //    error type, which results in blocking the PLMN with a specific protocol timer.
    // 3. Trigger IMS registration on the device over IWLAN.
    // 4. Verify that the device enters a deregistered state with the suggested action to
    //    block the PLMN with a timeout.
    @Test
    public void register_onWlan_with503AsRepeatedError_triggersPlmnBlockWithTimeout()
            throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages("<503-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        // KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY - REG_ERROR_CODE_503
        mConfig.putIntArray(CarrierConfig.Ims.KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY,
                new int[] {REG_ERROR_CODE_503});

        // KEY_EXTRA_REG_ERR_FINAL_TYPE_INT - ERROR_TYPE_REPEATED
        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_REPEATED);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

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
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);
    }

    // 1. Set up the server to respond to the REGISTER request with a 503 Service Unavailable
    //    and a Retry-After header.
    // 2. Configure the device via CarrierConfig to define the 503 response as a 'repeated'
    //    error type.
    // 3. Trigger IMS registration on the device over IWLAN.
    // 4. Verify that the device enters a deregistered state with the suggested action to
    //    block the PLMN with a timeout.
    @Test
    public void register_onWlan_with503RetryAfter_triggersPlmnBlockWithTimeout()
            throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("503-REGISTER")
                .setHeader("Retry-After", "100")
                .build());
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        // KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY - REG_ERROR_CODE_503
        mConfig.putIntArray(CarrierConfig.Ims.KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY,
                new int[] {REG_ERROR_CODE_503});

        // KEY_EXTRA_REG_ERR_FINAL_TYPE_INT - ERROR_TYPE_REPEATED
        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_REPEATED);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

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
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);
    }

    // 1. Set up the server to respond to the REGISTER request with a 504 Server Time-out.
    // 2. Configure the device via CarrierConfig to define the 504 response as a 'repeated'
    //    error type.
    // 3. Trigger IMS registration on the device over IWLAN.
    // 4. Verify that the device enters a deregistered state with the suggested action to
    //    block the PLMN with a timeout.
    @Test
    public void register_onWlan_with504AsRepeatedError_triggersPlmnBlockWithTimeout()
            throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages("<504-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        // KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY - REG_ERROR_CODE_504
        mConfig.putIntArray(CarrierConfig.Ims.KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY,
                new int[] {REG_ERROR_CODE_504});

        // KEY_EXTRA_REG_ERR_FINAL_TYPE_INT - ERROR_TYPE_REPEATED
        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_REPEATED);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

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
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);
    }

    // 1. Set up the server to respond to the REGISTER request with a 504 Server Time-out
    //    and a Retry-After header.
    // 2. Configure the device via CarrierConfig to define the 504 response as a 'repeated'
    //    error type.
    // 3. Trigger IMS registration on the device over IWLAN.
    // 4. Verify that the device enters a deregistered state with the suggested action to
    //    block the PLMN with a timeout.
    @Test
    public void register_onWlan_with504RetryAfter_triggersPlmnBlockWithTimeout()
            throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("504-REGISTER")
                .setHeader("Retry-After", "100")
                .build());
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        // KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY - REG_ERROR_CODE_504
        mConfig.putIntArray(CarrierConfig.Ims.KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY,
                new int[] {REG_ERROR_CODE_504});

        // KEY_EXTRA_REG_ERR_FINAL_TYPE_INT - ERROR_TYPE_REPEATED
        PersistableBundle objExtraRegErrBundle = new PersistableBundle();
        objExtraRegErrBundle.putInt(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, ERROR_TYPE_REPEATED);
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_EXTRA_REG_ERR_BUNDLE, objExtraRegErrBundle);

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
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);
    }

    // 1. Set up the server to complete a normal registration and subscription flow, and then
    //    expect a re-REGISTER.
    // 2. Trigger IMS registration on IWLAN.
    // 3. Verify that the device registers successfully on IWLAN.
    // 4. Simulate a data connection handover from IWLAN to LTE.
    // 5. Verify that the device successfully re-registers on LTE.
    @Test
    public void handover_fromWlan_toLte_succeeds() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(">REGISTER | <200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE"
                + "| >REGISTER | <200-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildLteIwlanServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);

        mRegistration.expect(3000).nothing();

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_HANDOVER_IN_PROGRESS), regInfo.getSlotId());

        notifyPreciseDataConnectionState(getLtePreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo.getSlotId());

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_LTE);
    }

    // 1. Set up the server to complete a normal registration and subscription flow, and then
    //    expect a re-REGISTER.
    // 2. Trigger IMS registration on IWLAN.
    // 3. Verify that the device registers successfully on IWLAN.
    // 4. Simulate a data connection handover from IWLAN to NR.
    // 5. Verify that the device successfully re-registers on NR.
    @Test
    public void handover_fromWlan_toNr_succeeds() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(">REGISTER | <200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE"
                + "| >REGISTER | <200-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildNrIwlanServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);

        mRegistration.expect(3000).nothing();

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_HANDOVER_IN_PROGRESS), regInfo.getSlotId());

        notifyPreciseDataConnectionState(getNrPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo.getSlotId());

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_NR);
    }

    // 1. Set up the server to complete registration and expect a SUBSCRIBE request.
    // 2. Configure the device for Wi-Fi Calling.
    // 3. Trigger IMS registration on IWLAN.
    // 4. Verify the device registers successfully.
    // 5. Verify the device sends a SUBSCRIBE request.
    @Test
    public void subscribe_onWlan_withDefaultConfig_succeeds() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages("<200-REGISTER | >SUBSCRIBE | >SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildLteIwlanServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);
    }

    // 1. The device completes registration and subscription on IWLAN.
    // 2. The server sends a NOTIFY message with event="unregistered" to terminate the registration.
    // 3. The device sends a 200 OK for the NOTIFY.
    // 4. Verify that the device is deregistered locally due to network-initiated detach.
    @Ignore("TISS support is required.")
    @Test
    public void deregister_onWlan_byNotifyUnregistered_succeeds() throws Exception {
        // TODO: The following XML files must be included in TISS:
        /*
        File name : tiss/preferencefiles/xml/reg_notify_unregistered.txt
        <?xml version="1.0" encoding="UTF-8"?>
        <reginfo xmlns="urn:ietf:params:xml:ns:reginfo" version="1" state="full">
          <registration aor="sip:001011123456789@ims.mnc001.mcc001.3gppnetwork.org" id="a1"
              state="terminated">
            <contact id="1" state="terminated" event="unregistered">
              <uri>sip:ue_instance@192.168.200.44:5060</uri>
              <unknown-param name="reason">Registration Unregistered by Network</unknown-param>
            </contact>
          </registration>
        </reginfo>
        */

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("NOTIFY-SUBSCRIBE")
                .setXml("reg_notify_unregistered")
                .build());
        generator.addMessages(">200-NOTIFY");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildLteIwlanServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);
    }

    // 1. The device completes registration and subscription on IWLAN.
    // 2. The server sends a NOTIFY message with event="unregistered".
    // 3. Configure the device to re-register upon receiving a NOTIFY with "unregistered" event.
    // 4. Verify that the device is deregistered locally.
    // 5. Verify that the device triggers a new registration and successfully registers again
    //    on IWLAN.
    @Ignore("TISS support is required.")
    @Test
    public void deregister_onWlan_byNotifyUnregistered_triggersRegistration()
            throws Exception {
        // TODO: The following XML files must be included in TISS:
        /*
        File name : tiss/preferencefiles/xml/reg_notify_unregistered.txt
        <?xml version="1.0" encoding="UTF-8"?>
        <reginfo xmlns="urn:ietf:params:xml:ns:reginfo" version="1" state="full">
          <registration aor="sip:001011123456789@ims.mnc001.mcc001.3gppnetwork.org" id="a1"
              state="terminated">
            <contact id="1" state="terminated" event="unregistered">
              <uri>sip:ue_instance@192.168.200.44:5060</uri>
              <unknown-param name="reason">Registration Unregistered by Network</unknown-param>
            </contact>
          </registration>
        </reginfo>
        */

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("NOTIFY-SUBSCRIBE")
                .setXml("reg_notify_unregistered")
                .build());
        generator.addMessages(">200-NOTIFY");
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        // KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY - NOTIFY_TERMINATED_UNREGISTERED
        PersistableBundle objNotifyTerminatedForInitRegUsedEvenBundle = new PersistableBundle();
        objNotifyTerminatedForInitRegUsedEvenBundle.putIntArray(
                CarrierConfig.Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY,
                new int[] {NOTIFY_TERMINATED_UNREGISTERED});
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_BUNDLE,
                objNotifyTerminatedForInitRegUsedEvenBundle);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildLteIwlanServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);
    }

    // 1. The device completes registration and subscription on IWLAN.
    // 2. The server sends a NOTIFY message with event="rejected" to terminate the registration.
    // 3. The device sends a 200 OK for the NOTIFY.
    // 4. Verify that the device is deregistered locally due to network-initiated detach.
    @Ignore("TISS support is required.")
    @Test
    public void deregister_onWlan_byNotifyRejected_succeeds() throws Exception {
        // TODO: The following XML files must be included in TISS:
        /*
        File name : tiss/preferencefiles/xml/reg_notify_rejected.txt
        <?xml version="1.0" encoding="UTF-8"?>
        <reginfo xmlns="urn:ietf:params:xml:ns:reginfo" version="1" state="full">
          <registration aor="sip:001011123456789@ims.mnc001.mcc001.3gppnetwork.org" id="a1"
              state="terminated">
            <contact id="1" state="terminated" event="rejected">
              <uri>sip:ue_instance@192.168.200.44:5060</uri>
              <unknown-param name="reason">Registration Rejected by Network</unknown-param>
            </contact>
          </registration>
        </reginfo>
        */

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("NOTIFY-SUBSCRIBE")
                .setXml("reg_notify_rejected")
                .build());
        generator.addMessages(">200-NOTIFY");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildLteIwlanServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);
    }

    // 1. The device completes registration and subscription on IWLAN.
    // 2. The server sends a NOTIFY message with event="rejected".
    // 3. Configure the device to re-register upon receiving a NOTIFY with "rejected" event.
    // 4. Verify that the device is deregistered locally.
    // 5. Verify that the device triggers a new registration and successfully registers again
    //    on IWLAN.
    @Ignore("TISS support is required.")
    @Test
    public void deregister_onWlan_byNotifyRejected_triggersRegistration()
            throws Exception {
        // TODO: The following XML files must be included in TISS:
        /*
        File name : tiss/preferencefiles/xml/reg_notify_rejected.txt
        <?xml version="1.0" encoding="UTF-8"?>
        <reginfo xmlns="urn:ietf:params:xml:ns:reginfo" version="1" state="full">
          <registration aor="sip:001011123456789@ims.mnc001.mcc001.3gppnetwork.org" id="a1"
              state="terminated">
            <contact id="1" state="terminated" event="rejected">
              <uri>sip:ue_instance@192.168.200.44:5060</uri>
              <unknown-param name="reason">Registration Rejected by Network</unknown-param>
            </contact>
          </registration>
        </reginfo>
        */

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("NOTIFY-SUBSCRIBE")
                .setXml("reg_notify_rejected")
                .build());
        generator.addMessages(">200-NOTIFY");
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        // KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY - NOTIFY_TERMINATED_REJECTED
        PersistableBundle objNotifyTerminatedForInitRegUsedEvenBundle = new PersistableBundle();
        objNotifyTerminatedForInitRegUsedEvenBundle.putIntArray(
                CarrierConfig.Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY,
                new int[] {NOTIFY_TERMINATED_REJECTED});
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_BUNDLE,
                objNotifyTerminatedForInitRegUsedEvenBundle);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildLteIwlanServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);
    }

    // 1. The device completes registration and subscription on IWLAN.
    // 2. The server sends a NOTIFY message with event="rejected".
    // 3. Configure the device to re-register upon receiving a NOTIFY with "rejected" event.
    // 4. Verify that the device is deregistered locally.
    // 5. Verify that the device triggers a new registration and successfully registers again
    //    on IWLAN.
    @Ignore("TISS support is required.")
    @Test
    public void deregister_onWlan_byNotifyDeactivated_succeeds() throws Exception {
        // TODO: The following XML files must be included in TISS:
        /*
        File name : tiss/preferencefiles/xml/reg_notify_deactivated.txt
        <?xml version="1.0" encoding="UTF-8"?>
        <reginfo xmlns="urn:ietf:params:xml:ns:reginfo" version="1" state="full">
          <registration aor="sip:001011123456789@ims.mnc001.mcc001.3gppnetwork.org" id="a1"
              state="terminated">
            <contact id="1" state="terminated" event="deactivated">
              <uri>sip:ue_instance@192.168.200.44:5060</uri>
              <unknown-param name="reason">Registration Deactivated by Network</unknown-param>
            </contact>
          </registration>
        </reginfo>
        */

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("NOTIFY-SUBSCRIBE")
                .setXml("reg_notify_deactivated")
                .build());
        generator.addMessages(">200-NOTIFY");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildLteIwlanServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);
    }

    // 1. The device completes registration and subscription on IWLAN.
    // 2. The server sends a NOTIFY message with event="deactivated".
    // 3. Configure the device to re-register upon receiving a NOTIFY with "deactivated" event.
    // 4. Verify that the device is deregistered locally.
    // 5. Verify that the device triggers a new registration and successfully registers again
    //    on IWLAN.
    @Ignore("TISS support is required.")
    @Test
    public void deregister_onWlan_byNotifyDeactivated_triggersRegistration()
            throws Exception {
        // TODO: The following XML files must be included in TISS:
        /*
        File name : tiss/preferencefiles/xml/reg_notify_deactivated.txt
        <?xml version="1.0" encoding="UTF-8"?>
        <reginfo xmlns="urn:ietf:params:xml:ns:reginfo" version="1" state="full">
          <registration aor="sip:001011123456789@ims.mnc001.mcc001.3gppnetwork.org" id="a1"
              state="terminated">
            <contact id="1" state="terminated" event="deactivated">
              <uri>sip:ue_instance@192.168.200.44:5060</uri>
              <unknown-param name="reason">Registration Deactivated by Network</unknown-param>
            </contact>
          </registration>
        </reginfo>
        */

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("NOTIFY-SUBSCRIBE")
                .setXml("reg_notify_deactivated")
                .build());
        generator.addMessages(">200-NOTIFY");
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY - NOTIFY_TERMINATED_DEACTIVATED
        PersistableBundle objNotifyTerminatedForInitRegUsedEvenBundle = new PersistableBundle();
        objNotifyTerminatedForInitRegUsedEvenBundle.putIntArray(
                CarrierConfig.Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY,
                new int[] {NOTIFY_TERMINATED_DEACTIVATED});
        mConfig.putPersistableBundle(
                CarrierConfig.Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_BUNDLE,
                objNotifyTerminatedForInitRegUsedEvenBundle);

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildLteIwlanServiceState())
                .setExpectedRegTech(REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);
    }
}
