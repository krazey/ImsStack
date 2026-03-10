/*
 * Copyright (C) 2026 The Android Open Source Project
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

package com.android.imsstack.enabler.media;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import android.content.Context;
import android.telephony.imsmedia.ImsMediaManager;
import android.testing.AndroidTestingRunner;

import com.android.imsstack.ImsStackTest;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

@RunWith(AndroidTestingRunner.class)
public class MediaManagerHelperTest extends ImsStackTest {

    @Mock private Context mMockContext;
    @Mock private IMediaConnectionObserver mMockMediaObserver;
    @Mock private ImsMediaManager mMockImsMediaManager;
    @Mock private Executor mMockExecutor;

    private MediaManagerHelper mMediaManagerHelper;

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());
        MockitoAnnotations.initMocks(this);
        try {
            com.android.imsstack.base.AppContext.init(mMockContext);
        } catch (IllegalStateException e) {
            // AppContext might already be initialized in some test environments
        }
        mMediaManagerHelper =
                new MediaManagerHelper(
                        mMockContext, mMockMediaObserver, mMockImsMediaManager, mMockExecutor);
        MediaManagerHelper.setImsMediaConnected(false);
    }

    @After
    public void tearDown() throws Exception {
        mMediaManagerHelper.close();
        super.tearDown();
    }

    @Test
    public void testWaitForConnection_timeout() {
        long startTime = System.currentTimeMillis();
        boolean result = mMediaManagerHelper.waitForConnection(500);
        long duration = System.currentTimeMillis() - startTime;

        assertFalse(result);
        assertTrue("Wait should be at least 500ms", duration >= 500);
    }

    @Test
    public void testWaitForConnection_success() {
        ScheduledExecutorService executor = Executors.newSingleThreadScheduledExecutor();
        var unused =
                executor.schedule(
                        () -> MediaManagerHelper.setImsMediaConnected(true),
                        200,
                        TimeUnit.MILLISECONDS);

        long startTime = System.currentTimeMillis();
        boolean result = mMediaManagerHelper.waitForConnection(MediaConstants.SERVICE_WAIT_TIMEOUT);
        long duration = System.currentTimeMillis() - startTime;

        assertTrue(result);
        assertTrue("Wait should have occurred", duration >= 150);
        executor.shutdown();
    }

    /**
     * Verifies that the wait is aborted and the thread interrupted status is restored when an
     * interruption occurs.
     */
    @Test
    public void testWaitForConnection_interrupted() throws InterruptedException {
        CountDownLatch startLatch = new CountDownLatch(1);
        final boolean[] interruptedStatus = new boolean[1];

        Thread testThread =
                new Thread(
                        () -> {
                            // Signal that the thread has started
                            startLatch.countDown();
                            // This will block for 5 seconds unless interrupted
                            mMediaManagerHelper.waitForConnection(5000);
                            // Capture if the interrupted flag was correctly restored in the handler
                            interruptedStatus[0] = Thread.currentThread().isInterrupted();
                        });

        testThread.start();

        // Ensure the test thread has at least started execution
        assertTrue("Test thread failed to start", startLatch.await(1, TimeUnit.SECONDS));

        // Interrupt the thread. If it's already in wait(), it throws InterruptedException.
        // If it hasn't reached wait() yet, the interrupt flag is set and wait() will throw
        // immediately.
        testThread.interrupt();

        // Wait for the thread to finish processing the interruption
        testThread.join(1000);

        assertFalse("Thread should have terminated after interruption", testThread.isAlive());
        assertTrue(
                "Interrupted status should be restored in the thread context",
                interruptedStatus[0]);
    }

    @Test
    public void testWaitForConnection_alreadyConnected() {
        MediaManagerHelper.setImsMediaConnected(true);
        boolean result = mMediaManagerHelper.waitForConnection(1000);
        assertTrue(result);
    }
}
