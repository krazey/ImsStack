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

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.mockito.Mockito.verify;

import android.os.Looper;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.enabler.aos.service.AosService;
import com.android.imsstack.jni.JniIms;
import com.android.imsstack.jni.JniImsProxy;

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
    public void init_creatingServices() {
        assertNull(mAosFactory.getAosRegistration(SLOT0));
        assertNull(mAosFactory.getAosInfo(SLOT0));
        assertNull(mAosFactory.getAosSettingService(SLOT0));

        mAosFactory.init(SLOT0);

        assertNotNull(mAosFactory.getAosRegistration(SLOT0));
        assertNotNull(mAosFactory.getAosInfo(SLOT0));
        assertNotNull(mAosFactory.getAosSettingService(SLOT0));
    }

    @Test
    public void init_creatingServices_withoutDebugScreen() {
        boolean originalDebugScreenEnabled = getDebugScreenEnabled(SLOT0);
        setDebugScreenEnabled(false, SLOT0);
        assertNull(mAosFactory.getAosDebug(SLOT0));

        mAosFactory.init(SLOT0);
        assertNull(mAosFactory.getAosDebug(SLOT0));

        setDebugScreenEnabled(originalDebugScreenEnabled, SLOT0);
        mAosFactory.cleanup(SLOT0);
    }

    @Test
    public void init_creatingServices_withDebugScreen() {
        boolean originalDebugScreenEnabled = getDebugScreenEnabled(SLOT0);
        setDebugScreenEnabled(true, SLOT0);
        assertNull(mAosFactory.getAosDebug(SLOT0));

        mAosFactory.init(SLOT0);
        assertNotNull(mAosFactory.getAosDebug(SLOT0));

        setDebugScreenEnabled(originalDebugScreenEnabled, SLOT0);
        mAosFactory.cleanup(SLOT0);
    }

    @Test
    public void cleanup_removingServices() {
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
    public void start_startService() {
        mAosFactory.replaceService(SLOT0, mMockAosService);
        mAosFactory.start(SLOT0);

        verify(mMockAosService).start();
    }

    @Test
    public void stop_stopService() {
        mAosFactory.replaceService(SLOT0, mMockAosService);
        mAosFactory.stop(SLOT0);

        verify(mMockAosService).stop();
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
