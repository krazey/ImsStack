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

import com.android.imsstack.its.servercontrol.BasicScenarioTemplates;
import com.android.imsstack.its.servercontrol.ControlConnection;
import com.android.imsstack.its.servercontrol.ScenarioGeneratorUtils;
import com.android.imsstack.its.servercontrol.ServerFailureHandler;
import com.android.imsstack.its.tests.ImsStackTestBase;
import com.android.imsstack.its.tests.registration.RegistrationHelper;
import com.android.imsstack.its.tests.registration.RegistrationInfo;
import com.android.imsstack.its.tests.registration.util.TestRegistration;
import com.android.imsstack.its.util.SingleLatch;

/**
 * Base class for Registration ImsStack tests.
 * Provides common test fixtures (like APN settings), configuration helpers,
 * and utility methods required by most registration test scenarios.
 */
public class RegistrationTestBase extends ImsStackTestBase {
    protected final SingleLatch mEventLatch = new SingleLatch(
            RegistrationTestBase.class.getSimpleName());
    protected static final int REG_ERROR_CODE_403 = 403;
    protected static final int REG_ERROR_CODE_404 = 404;
    protected static final int REG_ERROR_CODE_406 = 406;
    protected static final int REG_ERROR_CODE_5XX = 5;
    protected static final int REG_ERROR_CODE_503 = 503;
    protected static final int REG_ERROR_CODE_504 = 504;

    protected static final int ERROR_TYPE_NOT_SPECIFIED = 0;
    protected static final int ERROR_TYPE_REPEATED = 1;
    protected static final int ERROR_TYPE_CRITICAL = 2;
    protected static final int ERROR_TYPE_REPEATED_WITH_ONLY_ATTACHED_NETWORK = 4;
    protected static final int ERROR_TYPE_RAT_BLOCK = 5;

    protected static final int NOTIFY_TERMINATED_EXPIRED = 1;
    protected static final int NOTIFY_TERMINATED_DEACTIVATED = 2;
    protected static final int NOTIFY_TERMINATED_PROBATION = 3;
    protected static final int NOTIFY_TERMINATED_UNREGISTERED = 4;
    protected static final int NOTIFY_TERMINATED_REJECTED = 5;

    protected static final int ACCESS_NETWORK_TYPE_EUTRAN = 3;
    protected static final int ACCESS_NETWORK_TYPE_IWLAN = 5;
    protected static final int ACCESS_NETWORK_TYPE_NGRAN = 6;

    protected static final int PREFERRED_ACCESSTYPE_FEATURE_TAG_DISABLED = 0;
    protected static final int PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED = 1;
    protected static final int PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED_WITHOUT_NUMERICAL_VALUE = 2;

    protected ControlConnection mServerControlConnection;
    protected TestRegistration mRegistration;
    protected RegistrationHelper mRegistrationHelper;
    protected PersistableBundle mConfig;
    protected RegistrationInfo.Builder mInfoBuilder;


    /**
     * Initializes the {@link #mServerControlConnection} using the primary P-CSCF address.
     *
     * @param serverFailureHandler The handler to notify if the server connection failures.
     */
    protected void createControlConnection(ServerFailureHandler serverFailureHandler) {
        mServerControlConnection =
                new ControlConnection(serverFailureHandler, getPcscfAddresses().get(0));
    }

    /**
     * Configures the test SIP server to expect and respond to a typical registration
     * and subscription flow.
     */
    protected void setDefaultRegistrationScenario() {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        mServerControlConnection.sendControlCommand(generator.build().toString());
    }
}
