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
package com.android.imsstack.util;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.verify;

import android.test.suitebuilder.annotation.SmallTest;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class ThreadMessageExecutorTest {
    private static final int CALLBACK_WAIT_TIME = 100;

    @Mock Runnable mCallback;
    @Mock Runnable mExceptionCallback;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
    }

    @After
    public void tearDown() throws Exception {
    }

    @Test
    @SmallTest
    public void execute_threadNotStarted() {
        ThreadMessageExecutor executor = new ThreadMessageExecutor(
                ThreadMessageExecutorTest.class.getSimpleName() + ":NotStarted");

        assertFalse(executor.isAlive());

        executor.execute(mCallback);
        verify(mCallback, timeout(CALLBACK_WAIT_TIME)).run();
    }

    @Test
    @SmallTest
    public void execute_threadStarted() throws Exception {
        ThreadMessageExecutor executor = new ThreadMessageExecutor(
                ThreadMessageExecutorTest.class.getSimpleName() + ":Started");

        executor.start();
        assertTrue(executor.isAlive());

        executor.execute(mCallback);
        verify(mCallback, timeout(CALLBACK_WAIT_TIME)).run();

        doThrow(new RuntimeException("ThreadMessageExecutorTest!!!"))
                .when(mExceptionCallback).run();
        executor.execute(mExceptionCallback);

        // Expected: Any exception should not be thrown when calling execute(...).
    }
}
