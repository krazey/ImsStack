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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.os.Looper;
import android.telephony.TelephonyManager;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.enabler.aos.service.AosService;
import com.android.imsstack.jni.JniIms;
import com.android.imsstack.jni.JniImsProxy;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsPrivateProperties;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class AosFactoryTest {
    private static final int MAX_SIM_SLOT = 1;
    private static final int SLOT_0 = 0;
    private static Context sContext;

    private AosFactory mAosFactory;

    @Mock JniIms mMockJniIms;
    @Mock AosService mMockAosService;
    @Mock AosSettingService mMockAosSettingService;
    @Mock AosDebug mMockAosDebug;

    @BeforeClass
    public static void setUpOnce() {
        sContext = new ContextFixture().getTestDouble();
        AppContext.init(sContext);

        TelephonyManager tm = sContext.getSystemService(TelephonyManager.class);
        when(tm.createForSubscriptionId(anyInt())).thenReturn(tm);
        when(tm.getActiveModemCount()).thenReturn(MAX_SIM_SLOT);
        when(tm.getSupportedModemCount()).thenReturn(MAX_SIM_SLOT);
    }

    @Before
    public void setup() {
        if (Looper.myLooper() == null) {
            Looper.prepare();
        }

        MockitoAnnotations.initMocks(this);

        // This is to avoid error that occurs during nativeGetInterface
        JniImsProxy.setJniIms(mMockJniIms);

        mAosFactory = AosFactory.getInstance();
    }

    @After
    public void tearDown() {
        JniImsProxy.setJniIms(null);
    }

    @AfterClass
    public static void tearDownOnce() {
        AppContext.deinit();
    }

    @Test
    public void init_creatingServices() {
        assertNull(mAosFactory.getAosRegistration(SLOT_0));
        assertNull(mAosFactory.getAosInfo(SLOT_0));
        assertEquals(0, mAosFactory.mAosSettingServices.size());

        mAosFactory.init(SLOT_0);

        assertNotNull(mAosFactory.getAosRegistration(SLOT_0));
        assertNotNull(mAosFactory.getAosInfo(SLOT_0));
        assertEquals(1, mAosFactory.mAosSettingServices.size());
    }

    @Test
    public void init_creatingServices_withoutDebugScreen() {
        boolean originalDebugScreenEnabled = getDebugScreenEnabled(SLOT_0);
        setDebugScreenEnabled(false, SLOT_0);
        assertNull(mAosFactory.getAosDebug(SLOT_0));

        mAosFactory.init(SLOT_0);
        assertNull(mAosFactory.getAosDebug(SLOT_0));

        setDebugScreenEnabled(originalDebugScreenEnabled, SLOT_0);
        mAosFactory.cleanup(SLOT_0);
    }

    @Test
    public void init_creatingServices_withDebugScreen() {
        boolean originalDebugScreenEnabled = getDebugScreenEnabled(SLOT_0);
        setDebugScreenEnabled(true, SLOT_0);
        assertNull(mAosFactory.getAosDebug(SLOT_0));

        mAosFactory.init(SLOT_0);
        assertNotNull(mAosFactory.getAosDebug(SLOT_0));

        setDebugScreenEnabled(originalDebugScreenEnabled, SLOT_0);
        mAosFactory.cleanup(SLOT_0);
    }

    @Test
    public void cleanup_removingServices() {
        mAosFactory.mAosServices.put(SLOT_0, mMockAosService);
        mAosFactory.mAosSettingServices.put(SLOT_0, mMockAosSettingService);
        mAosFactory.mAosDebugs.put(SLOT_0, mMockAosDebug);

        mAosFactory.cleanup(SLOT_0);

        verify(mMockAosService).cleanup();
        verify(mMockAosSettingService).cleanup();
        assertNull(mAosFactory.getAosRegistration(SLOT_0));
        assertNull(mAosFactory.getAosInfo(SLOT_0));
        assertEquals(0, mAosFactory.mAosSettingServices.size());
        assertNull(mAosFactory.getAosDebug(SLOT_0));
    }

    @Test
    public void start_startService() {
        mAosFactory.mAosServices.put(SLOT_0, mMockAosService);

        mAosFactory.start(SLOT_0);

        verify(mMockAosService).start();
    }

    @Test
    public void stop_stopService() {
        mAosFactory.mAosServices.put(SLOT_0, mMockAosService);

        mAosFactory.stop(SLOT_0);

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
