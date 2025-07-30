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

package com.android.imsstack.enabler.aos;

import static com.android.imsstack.base.TestAppContext.SLOT0;
import static com.android.imsstack.base.TestAppContext.SLOT1;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertThrows;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoInteractions;

import android.os.Looper;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.enabler.aos.service.AosService;
import com.android.imsstack.jni.JniIms;
import com.android.imsstack.jni.JniImsProxy;
import com.android.imsstack.util.IndentingPrintWriter;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class AosFactoryTest {
    private ContextFixture mContextFixture;
    private AosFactory mAosFactory;

    @Mock JniIms mMockJniIms;
    @Mock AosService mMockAosService;
    @Mock AosSettingService mMockAosSettingService;
    @Mock AosDebug mMockAosDebug;

    @Before
    public void setup() {
        if (Looper.myLooper() == null) {
            Looper.prepare();
        }

        MockitoAnnotations.initMocks(this);

        mContextFixture = new ContextFixture();
        AppContext.init(mContextFixture.getTestDouble());
        // This is to avoid error that occurs during nativeGetInterface
        JniImsProxy.setJniIms(mMockJniIms);

        mAosFactory = AosFactory.getInstance();
    }

    @After
    public void tearDown() {
        JniImsProxy.setJniIms(null);
        mAosFactory.cleanup(SLOT0);
        mContextFixture = null;
        AppContext.deinit();
    }

    @Test
    public void init_onNewSlot_createsServicesSuccessfully() {
        assertNull(mAosFactory.getAosRegistration(SLOT0));
        assertNull(mAosFactory.getAosInfo(SLOT0));
        assertNull(mAosFactory.getAosSettingService(SLOT0));

        mAosFactory.init(SLOT0);

        assertNotNull(mAosFactory.getAosRegistration(SLOT0));
        assertNotNull(mAosFactory.getAosInfo(SLOT0));
        assertNotNull(mAosFactory.getAosSettingService(SLOT0));
    }

    @Test
    public void init_createServices_withoutDebugScreen() {
        boolean originalDebugScreenEnabled = getDebugScreenEnabled(SLOT0);
        setDebugScreenEnabled(false, SLOT0);
        assertNull(mAosFactory.getAosDebug(SLOT0));

        mAosFactory.init(SLOT0);
        assertNull(mAosFactory.getAosDebug(SLOT0));

        setDebugScreenEnabled(originalDebugScreenEnabled, SLOT0);
        mAosFactory.cleanup(SLOT0);
    }

    @Test
    public void init_createServices_withDebugScreen() {
        boolean originalDebugScreenEnabled = getDebugScreenEnabled(SLOT0);
        setDebugScreenEnabled(true, SLOT0);
        assertNull(mAosFactory.getAosDebug(SLOT0));

        mAosFactory.init(SLOT0);
        assertNotNull(mAosFactory.getAosDebug(SLOT0));

        setDebugScreenEnabled(originalDebugScreenEnabled, SLOT0);
        mAosFactory.cleanup(SLOT0);
    }

    @Test
    public void cleanup_onInitializedSlot_removesServicesAndCallsCleanup() {
        AosFactory.getInstance().replaceService(SLOT0, mMockAosService);
        AosFactory.getInstance().replaceSettingService(SLOT0, mMockAosSettingService);
        AosFactory.getInstance().replaceDebug(SLOT0, mMockAosDebug);

        mAosFactory.cleanup(SLOT0);

        verify(mMockAosService).cleanup();
        verify(mMockAosSettingService).cleanup();
        assertNull(mAosFactory.getAosRegistration(SLOT0));
        assertNull(mAosFactory.getAosInfo(SLOT0));
        assertNull(mAosFactory.getAosSettingService(SLOT0));
        assertNull(mAosFactory.getAosDebug(SLOT0));
    }

    @Test
    public void cleanup_doesNothingForUninitializedSlot() {
        // GIVEN: SLOT1 is initialized with a mock service, but SLOT0 is not.
        mAosFactory.replaceService(SLOT1, mMockAosService);

        assertNull(mAosFactory.getAosInfo(SLOT0));
        assertNotNull(mAosFactory.getAosInfo(SLOT1));

        // WHEN: cleanup() is called on the uninitialized SLOT0.
        mAosFactory.cleanup(SLOT0);

        // THEN: Explicitly verify that the mock service in SLOT1 was not touched.
        verifyNoInteractions(mMockAosService);
        assertNotNull(mAosFactory.getAosInfo(SLOT1));

        // Cleanup for the next test
        mAosFactory.cleanup(SLOT1);
    }

    @Test
    public void start_onInitializedSlot_delegatesToAosService() {
        mAosFactory.replaceService(SLOT0, mMockAosService);
        mAosFactory.start(SLOT0);

        verify(mMockAosService).start();
    }

    @Test
    public void start_throwsExceptionWhenServiceNotInitialized() {
        // GIVEN: init() is not called for SLOT0.
        assertNull(mAosFactory.getAosInfo(SLOT0));

        // WHEN: start() is called on an uninitialized slot.
        IllegalStateException exception = assertThrows(IllegalStateException.class, () -> {
            mAosFactory.start(SLOT0);
        });

        // THEN: An IllegalStateException is expected, and we can also check its message.
        assertEquals("AosService not found for slotId: 0", exception.getMessage());
    }

    @Test
    public void stop_onInitializedSlot_delegatesToAosService() {
        mAosFactory.replaceService(SLOT0, mMockAosService);
        mAosFactory.stop(SLOT0);

        verify(mMockAosService).stop();
    }

    @Test
    public void dump_delegatesToAosServiceWhenExists() {
        // GIVEN
        mAosFactory.replaceService(SLOT0, mMockAosService);
        IndentingPrintWriter mockPrintWriter = mock(IndentingPrintWriter.class);

        // WHEN
        mAosFactory.dump(SLOT0, mockPrintWriter);

        // THEN
        verify(mMockAosService).dump(mockPrintWriter);
    }

    @Test
    public void dump_doesNothingWhenServiceNotExists() {
        // GIVEN: No AosService is present for SLOT0.
        assertNull(mAosFactory.getAosInfo(SLOT0));
        IndentingPrintWriter mockPrintWriter = mock(IndentingPrintWriter.class);

        // WHEN
        mAosFactory.dump(SLOT0, mockPrintWriter);

        // THEN
        verify(mMockAosService, never()).dump(mockPrintWriter);
    }

    private void setDebugScreenEnabled(boolean isEnabled, int slotId) {
        ImsPrivateProperties.Persistent.setBoolean(
                ImsPrivateProperties.Persistent.KEY_TEST_DEBUG_SCREEN_ENABLED, isEnabled, slotId);
    }

    private boolean getDebugScreenEnabled(int slotId) {
        return ImsPrivateProperties.Persistent.getBoolean(
                ImsPrivateProperties.Persistent.KEY_TEST_DEBUG_SCREEN_ENABLED, false, slotId);
    }
}
