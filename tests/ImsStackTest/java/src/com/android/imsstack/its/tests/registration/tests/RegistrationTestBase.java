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

import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;

import com.android.imsstack.its.base.TelephonyManagerProxyImpl;
import com.android.imsstack.its.servercontrol.BasicScenarioTemplates;
import com.android.imsstack.its.servercontrol.ControlConnection;
import com.android.imsstack.its.servercontrol.ScenarioGeneratorUtils;
import com.android.imsstack.its.servercontrol.ServerFailureHandler;
import com.android.imsstack.its.tests.ImsStackTestBase;
import com.android.imsstack.its.tests.registration.RegistrationHelper;
import com.android.imsstack.its.tests.registration.RegistrationInfo;
import com.android.imsstack.its.tests.registration.util.TestRegistration;

public class RegistrationTestBase extends ImsStackTestBase {

    protected ControlConnection mServerControlConnection;
    protected TelephonyManagerProxyImpl mTelephony;
    protected TestRegistration mRegistration;
    protected RegistrationHelper mRegistrationHelper;
    protected PersistableBundle mConfig;
    protected RegistrationInfo.Builder mInfoBuilder;

    protected void createControlConnection(ServerFailureHandler serverFailureHandler) {
        mServerControlConnection =
                new ControlConnection(serverFailureHandler, getPcscfAddresses().get(0));
    }

    protected void setRegistrationBaseConfig() {
        mConfig = new PersistableBundle();
        mConfig.putBoolean(CarrierConfigManager.Ims.KEY_SIP_OVER_IPSEC_ENABLED_BOOL, false);
    }

    protected void setDefaultRegistrationScenario() {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        mServerControlConnection.sendControlCommand(generator.build().toString());
    }
}
