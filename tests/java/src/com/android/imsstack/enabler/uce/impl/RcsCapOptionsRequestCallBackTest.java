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

package com.android.imsstack.enabler.uce.impl;

import android.net.Uri;
import android.telephony.ims.RcsContactUceCapability;
import android.telephony.ims.RcsContactUceCapability.OptionsBuilder;

import com.android.imsstack.enabler.uce.interf.RemoteOptionsCallback;
import com.android.imsstack.util.MessageExecutor;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;

import java.util.HashSet;
import java.util.Set;
@RunWith(JUnit4.class)
public class RcsCapOptionsRequestCallBackTest {

    @Mock
    RemoteOptionsCallback mRemoteOptionsCallback;
    RcsCapOptionsRequestCallback mRcsCapOptionsRequestCallback;

    @Mock
    RcsContactUceCapability mRcsContactUceCapability;
    @Mock
    MessageExecutor mMessageExecutor;

    static final String FEATURE_VIDEO = "urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\";video ";

    @Before
    public void setUp() {
        mRemoteOptionsCallback = Mockito.mock(RemoteOptionsCallback.class);
        mMessageExecutor = new MessageExecutor("requestExecutor");
        mRcsCapOptionsRequestCallback = new RcsCapOptionsRequestCallback(mMessageExecutor);
        mRcsCapOptionsRequestCallback.setCallBack(mRemoteOptionsCallback);
    }

    @Test
    public  void  onRespondToCapabilityRequest() {
        Set<String> ownCapabilities = new HashSet<>();
        OptionsBuilder optionsBuilder = new OptionsBuilder(Uri.parse("+123456789"),
                RcsContactUceCapability.SOURCE_TYPE_CACHED);
        final RcsContactUceCapability capability = optionsBuilder.build();
        mRcsCapOptionsRequestCallback.onRespondToCapabilityRequest(capability, false);
        ownCapabilities.add(FEATURE_VIDEO);
        //verify
        Mockito.verify(mRemoteOptionsCallback, Mockito.timeout(100).times(1))
                .onRespondToCapabilityRequest(capability.getFeatureTags(), false);

        mRcsCapOptionsRequestCallback.onRespondToCapabilityRequest(capability, true);
        //verify
        Mockito.verify(mRemoteOptionsCallback, Mockito.timeout(100).times(1))
                .onRespondToCapabilityRequest(capability.getFeatureTags(), false);
    }

    @Test
    public void onRespondToCapabilityRequestWithError() {
        int code = 200;
        String reason = "ok";
        mRcsCapOptionsRequestCallback.onRespondToCapabilityRequestWithError(code, reason);
        Mockito.verify(mRemoteOptionsCallback, Mockito.timeout(100).times(1))
                .onRespondToCapabilityRequestWithError(code, reason);

        mRcsCapOptionsRequestCallback.onRespondToCapabilityRequestWithError(code, null);
        Mockito.verify(mRemoteOptionsCallback, Mockito.timeout(100).times(1))
                .onRespondToCapabilityRequestWithError(code, null);
    }

    @After
    public void cleanUp() {
        mRcsCapOptionsRequestCallback = null;
        mMessageExecutor = null;
        mRcsContactUceCapability = null;
    }
}
