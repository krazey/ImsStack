/*
 * Copyright (C) 2023 The Android Open Source Project
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

package com.android.imsstack.core;

import static com.android.imsstack.core.VoLteFactory.AGENT_LOCATION_AGENT_MANAGER;
import static com.android.imsstack.core.VoLteFactory.AGENT_PHONENUMBER;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;

import android.content.Context;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import androidx.test.filters.SmallTest;

import com.android.imsstack.core.service.serviceif.IVoLteService;
import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class VoLteFactoryTest {
    private Context mContext;
    private VoLteFactory mVoLteFactory;
    private static final int SLOT_1 = 1;

    @Before
    public void setUp() throws Exception {
        mContext = mock(Context.class);
        AppContext.init(mContext);
        mVoLteFactory = VoLteFactory.getInstance();
    }

    @After
    public void tearDown() throws Exception {
        mVoLteFactory.clear();
        AppContext.deinit();
        mVoLteFactory = null;
    }

    @Test
    public void startService() {

        mVoLteFactory.startService(mContext, SLOT_1);
        assertNotNull(mVoLteFactory.getAgent(AGENT_PHONENUMBER));
        assertNotNull(mVoLteFactory.getAgent(AGENT_LOCATION_AGENT_MANAGER));
        assertNotNull(mVoLteFactory.getContext());
        assertNotNull(mVoLteFactory.getService(SLOT_1));
        mVoLteFactory.stopService(SLOT_1);
    }

    @Test
    @SmallTest
    public void stopService() {
        mVoLteFactory.startService(mContext, SLOT_1);
        IVoLteService voLteService = Mockito.mock(IVoLteService.class);
        mVoLteFactory.setService(SLOT_1, voLteService);
        mVoLteFactory.stopService(SLOT_1);
        assertNull(mVoLteFactory.getService(SLOT_1));
        verify(voLteService).cleanup(mContext);
    }

    @Test
    @SmallTest
    public void updateService() {
        mVoLteFactory.startService(mContext, SLOT_1);
        IVoLteService voLteService = Mockito.mock(IVoLteService.class);
        mVoLteFactory.setService(SLOT_1, voLteService);

        mVoLteFactory.updateService(SLOT_1);
        verify(voLteService).update(mContext);
        mVoLteFactory.stopService(SLOT_1);
    }

    @Test
    @SmallTest
    public void clear() {
        mVoLteFactory.clear();
        assertNull(mVoLteFactory.getAgent(AGENT_PHONENUMBER));
        assertNull(mVoLteFactory.getAgent(AGENT_LOCATION_AGENT_MANAGER));
    }
}
