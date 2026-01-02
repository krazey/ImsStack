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

package com.android.imsstack.imsservice.mmtel.videocall;

import static com.google.common.truth.Truth.assertThat;

import static org.mockito.Mockito.when;

import androidx.test.filters.SmallTest;

import com.android.imsstack.enabler.mtc.MtcMediaSession;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.imsservice.mmtel.call.IVideoCallSession;
import com.android.imsstack.imsservice.mmtel.videocall.base.ImsVideoCallProviderBase;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.concurrent.Executor;

@RunWith(JUnit4.class)
public class ImsVideoCallProviderFactoryTest {
    @Mock private IVideoCallSession mMockCallSession;
    @Mock private MtcMediaSession mMockMediaSession;
    @Mock private ICallContext mMockCallContext;
    @Mock private Executor mExecutor;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);

        when(mMockCallSession.getCallContext()).thenReturn(mMockCallContext);
        when(mMockCallContext.getExecutor()).thenReturn(mExecutor);
    }

    @Test
    @SmallTest
    public void testCreateVideoCallProvider() {
        ImsVideoCallProviderBase provider = ImsVideoCallProviderFactory.createVideoCallProvider(
                mMockCallSession, mMockMediaSession);

        assertThat(provider).isNotNull();
        assertThat(provider).isInstanceOf(ImsVideoCallProviderImpl.class);
    }
}
