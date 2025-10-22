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
package com.android.imsstack.its.tests.registration.tests;

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
public class PrimaryRegistrationTest extends RegistrationTestBase {

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
    public void testCarrierDefaultLte_Register_defaultConfig() throws Exception {
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

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_LTE);
    }

    @Test
    public void testCarrierDefaultLte_Register_CapabilityVoice() throws Exception {
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

    @Test
    public void testCarrierDefaultLte_Register_CapabilityVoiceVideo() throws Exception {
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

    @Test
    public void testCarrierDefaultLte_Register_CapabilityVoiceSms() throws Exception {
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

    @Test
    public void testCarrierDefaultLte_Register_Response423() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages(
                "<423-REGISTER | >REGISTER | <200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect().registering();

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_LTE);
    }

    @Test
    public void testCarrierDefaultLte_Register_Response403_TriggerPlmnBlock()
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

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultLte_Register_Response404_TriggerPlmnBlock()
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

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultLte_Register_Response500_TriggerPlmnBlockWithTimeOut()
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

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultLte_Register_Response503_TriggerPlmnBlockWithTimeOut()
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

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultLte_Register_Response503RetryAfter_TriggerPlmnBlockWithTimeOut()
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

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultLte_Register_Response504_TriggerPlmnBlockWithTimeOut()
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

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultLte_Register_Response504RetryAfter_TriggerPlmnBlockWithTimeOut()
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

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultLte_Register_HandoverToWlan() throws Exception {
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
                TelephonyManager.DATA_HANDOVER_IN_PROGRESS), regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultLte_Subscribe_defaultConfig() throws Exception {
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

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_LTE);
    }

    @Ignore("TISS support is required.")
    @Test
    public void testCarrierDefaultLte_Deregister_ByNotifyUnregistered() throws Exception {
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

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_LTE);

        mRegistration.expect().deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_LTE);
    }

    @Ignore("TISS support is required.")
    @Test
    public void testCarrierDefaultLte_Deregister_ByNotifyUnregistered_TriggerRegister()
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

    @Ignore("TISS support is required.")
    @Test
    public void testCarrierDefaultLte_Deregister_ByNotifyRejected() throws Exception {
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

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_LTE);
    }

    @Ignore("TISS support is required.")
    @Test
    public void testCarrierDefaultLte_Deregister_ByNotifyRejected_TriggerRegister()
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

    @Ignore("TISS support is required.")
    @Test
    public void testCarrierDefaultLte_Deregister_ByNotifyDeactivated() throws Exception {
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

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_LTE);
    }

    @Ignore("TISS support is required.")
    @Test
    public void testCarrierDefaultLte_Deregister_ByNotifyDeactivated_TriggerRegister()
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

    @Test
    public void testCarrierDefaultNr_Register_defaultConfig() throws Exception {
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_NR);
    }

    @Test
    public void testCarrierDefaultNr_Register_CapabilityVoice() throws Exception {
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

    @Test
    public void testCarrierDefaultNr_Register_CapabilityVoiceVideo() throws Exception {
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

    @Test
    public void testCarrierDefaultNr_Register_CapabilityVoiceSms() throws Exception {
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

    @Test
    public void testCarrierDefaultNr_Register_Response423() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages(
                "<423-REGISTER | >REGISTER | <200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        RegistrationInfo regInfo = mInfoBuilder
                .setServiceState(buildNrServiceState())
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_NR);
    }

    @Test
    public void testCarrierDefaultNr_Register_Response403_TriggerPlmnBlock() throws Exception {
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK,
                networkType -> networkType == REGISTRATION_TECH_NR);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultNr_Register_Response404_TriggerPlmnBlock() throws Exception {
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK,
                networkType -> networkType == REGISTRATION_TECH_NR);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultNr_Register_Response500_TriggerPlmnBlockWithTimeOut()
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_NR);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultNr_Register_Response503_TriggerPlmnBlockWithTimeOut()
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_NR);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultNr_Register_Response503RetryAfter_TriggerPlmnBlockWithTimeOut()
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_NR);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultNr_Register_Response504_TriggerPlmnBlockWithTimeOut()
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_NR);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultNr_Register_Response504RetryAfter_TriggerPlmnBlockWithTimeOut()
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_NR);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultNr_Register_HandoverToWlan() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(">REGISTER | <200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE"
                + "| >REGISTER | <200-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        RegistrationInfo regInfo = mInfoBuilder
                .setServiceState(buildNrServiceState())
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_NR);

        mRegistration.expect(3000).nothing();

        notifyPreciseDataConnectionState(getNrPreciseDataConnectionState(
                TelephonyManager.DATA_HANDOVER_IN_PROGRESS), regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultNr_Subscribe_defaultConfig() throws Exception {
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_NR);
    }

    @Ignore("TISS support is required.")
    @Test
    public void testCarrierDefaultNr_Deregister_ByNotifyUnregistered() throws Exception {
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_NR);
    }

    @Ignore("TISS support is required.")
    @Test
    public void testCarrierDefaultNr_Deregister_ByNotifyUnregistered_TriggerRegister()
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

    @Ignore("TISS support is required.")
    @Test
    public void testCarrierDefaultNr_Deregister_ByNotifyRejected() throws Exception {
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_NR);
    }

    @Ignore("TISS support is required.")
    @Test
    public void testCarrierDefaultNr_Deregister_ByNotifyRejected_TriggerRegister()
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

    @Ignore("TISS support is required.")
    @Test
    public void testCarrierDefaultNr_Deregister_ByNotifyDeactivated() throws Exception {
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_NR);
    }

    @Ignore("TISS support is required.")
    @Test
    public void testCarrierDefaultNr_Deregister_ByNotifyDeactivated_TriggerRegister()
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

    @Test
    public void testCarrierDefaultWlan_Register_defaultConfig() throws Exception {
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultWlan_Register_CapabilityVoice() throws Exception {
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
                .setEnableCapability(CAPABILITY_TYPE_VOICE,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setDisableCapability(CAPABILITY_TYPE_VIDEO,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setDisableCapability(CAPABILITY_TYPE_SMS,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultWlan_Register_CapabilityVoiceVideo() throws Exception {
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
                .setEnableCapability(CAPABILITY_TYPE_VOICE,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setEnableCapability(CAPABILITY_TYPE_VIDEO,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setDisableCapability(CAPABILITY_TYPE_SMS,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultWlan_Register_CapabilityVoiceSms() throws Exception {
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
                .setEnableCapability(CAPABILITY_TYPE_VOICE,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setDisableCapability(CAPABILITY_TYPE_VIDEO,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .setEnableCapability(CAPABILITY_TYPE_SMS,
                        REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN)
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultWlan_Register_Response423() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages(
                "<423-REGISTER | >REGISTER | <200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildLteIwlanServiceState())
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultWlan_Register_Response403_TriggerPlmnBlock()
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultWlan_Register_Response404_TriggerPlmnBlock()
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultWlan_Register_Response500_TriggerPlmnBlockWithTimeOut()
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultWlan_Register_Response503_TriggerPlmnBlockWithTimeOut()
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultWlan_Register_Response503RetryAfter_TriggerPlmnBlockWithTimeOut()
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultWlan_Register_Response504_TriggerPlmnBlockWithTimeOut()
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultWlan_Register_Response504RetryAfter_TriggerPlmnBlockWithTimeOut()
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect().registering();

        mRegistration.expect().deregistered(
                info -> info.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR,
                action -> action
                        == RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Test
    public void testCarrierDefaultWlan_Register_HandoverToLte() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(">REGISTER | <200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE"
                + "| >REGISTER | <200-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildLteIwlanServiceState())
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);

        mRegistration.expect(3000).nothing();

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_HANDOVER_IN_PROGRESS), regInfo);

        notifyPreciseDataConnectionState(getLtePreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_LTE);
    }

    @Test
    public void testCarrierDefaultWlan_Register_HandoverToNr() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(">REGISTER | <200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE"
                + "| >REGISTER | <200-REGISTER");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildNrIwlanServiceState())
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);

        mRegistration.expect(3000).nothing();

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_HANDOVER_IN_PROGRESS), regInfo);

        notifyPreciseDataConnectionState(getNrPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_NR);
    }

    @Test
    public void testCarrierDefaultWlan_Subscribe_defaultConfig() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessage(MessageBuildUtils.getDefaultRegister().build());
        generator.addMessages("<200-REGISTER | >SUBSCRIBE | >SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);

        RegistrationInfo regInfo = mInfoBuilder
                .addConfig(mConfig)
                .setServiceState(buildLteIwlanServiceState())
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Ignore("TISS support is required.")
    @Test
    public void testCarrierDefaultWlan_Deregister_ByNotifyUnregistered() throws Exception {
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Ignore("TISS support is required.")
    @Test
    public void testCarrierDefaultWlan_Deregister_ByNotifyUnregistered_TriggerRegister()
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Ignore("TISS support is required.")
    @Test
    public void testCarrierDefaultWlan_Deregister_ByNotifyRejected() throws Exception {
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Ignore("TISS support is required.")
    @Test
    public void testCarrierDefaultWlan_Deregister_ByNotifyRejected_TriggerRegister()
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Ignore("TISS support is required.")
    @Test
    public void testCarrierDefaultWlan_Deregister_ByNotifyDeactivated() throws Exception {
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }

    @Ignore("TISS support is required.")
    @Test
    public void testCarrierDefaultWlan_Deregister_ByNotifyDeactivated_TriggerRegister()
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
                .build();

        mRegistrationHelper.triggerRegistration(this, regInfo);

        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), regInfo);

        mRegistration.expect(20000).deregistered(
                reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR
                        && reason.getExtraCode() == ImsReasonInfo.CODE_NETWORK_DETACH,
                action -> action == RegistrationManager.SUGGESTED_ACTION_NONE,
                networkType -> networkType == REGISTRATION_TECH_IWLAN);

        mRegistration.expect().registered(
                attributes -> attributes.getRegistrationTechnology() == REGISTRATION_TECH_IWLAN);

        simulateSimStateChange(regInfo, TelephonyManager.SIM_STATE_ABSENT);
    }
}
