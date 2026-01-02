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

package com.android.imsstack.imsservice.mmtel.videocall.base;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.when;

import android.os.Handler;
import android.os.Looper;
import android.telephony.ims.ImsStreamMediaProfile;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.ims.internal.IImsVideoCallCallback;
import com.android.imsstack.ImsStackTest;
import com.android.imsstack.enabler.mtc.MtcMediaSession;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.imsservice.mmtel.call.IVideoCallSession;

import org.junit.After;
import org.junit.Before;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.concurrent.Executor;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public abstract class ImsVideoCallProviderTestBase extends ImsStackTest {
    @Mock protected IVideoCallSession mMockCallSession;
    @Mock protected ImsStreamMediaProfile mMockMediaProfile;
    @Mock protected MtcMediaSession mMockMediaSession;
    @Mock protected ImsVideoCallProviderBase mProvider;
    @Mock protected Executor mExecutor;
    @Mock protected IImsVideoCallCallback mMockCallback;
    @Mock protected ICallContext mMockCallContext;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        super.setUp("ImsVideoCallProviderTestBase");

        when(mMockCallSession.getCallContext()).thenReturn(mMockCallContext);
        when(mMockCallContext.getExecutor()).thenReturn(mExecutor);
        doAnswer(invocation -> {
            Runnable command = invocation.getArgument(0);
            new Handler(Looper.myLooper()).post(command);
            return null;
        }).when(mExecutor).execute(any(Runnable.class));

        mProvider = createProvider();
        mProvider.getInterface().setCallback(mMockCallback);
        // This is to complete the callback setting above.
        processAllMessages();
    }

    @After
    public void tearDown() throws Exception {
        super.tearDown();
    }

    protected abstract ImsVideoCallProviderBase createProvider();
}
