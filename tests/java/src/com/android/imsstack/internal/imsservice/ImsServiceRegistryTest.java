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
package com.android.imsstack.internal.imsservice;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.content.Context;
import android.telephony.ims.feature.MmTelFeature;
import android.telephony.ims.stub.ImsConfigImplBase;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.AppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class ImsServiceRegistryTest {
    private static final int SLOT0 = 0;

    @Mock Context mContext;
    @Mock MmTelFeature mMmTelFeature;
    @Mock ImsConfigImplBase mImsConfigImplBase;
    @Mock ImsServiceRegistry.Listener mListener;
    private ImsServiceRegistry mServiceRegistry;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        AppContext.init(mContext);
        mServiceRegistry = ImsServiceRegistry.getInstance(SLOT0);
        mServiceRegistry.setImsEnabled(false);
        mServiceRegistry.setMmTelFeature(null);
    }

    @After
    public void tearDown() throws Exception {
        AppContext.deinit();
        mListener = null;
        mMmTelFeature = null;
        mImsConfigImplBase = null;
        mContext = null;
    }

    @Test
    @SmallTest
    public void setMmTelFeature() throws Exception {
        mServiceRegistry.setMmTelFeature(mMmTelFeature);
        assertEquals(mMmTelFeature, mServiceRegistry.getMmTelFeature());

        mServiceRegistry.setMmTelFeature(null);
        assertNull(mServiceRegistry.getMmTelFeature());
    }

    @Test
    @SmallTest
    public void setImsConfig() throws Exception {
        mServiceRegistry.setImsConfig(mImsConfigImplBase);
        assertEquals(mImsConfigImplBase, mServiceRegistry.getImsConfig());

        mServiceRegistry.setImsConfig(null);
        assertNull(mServiceRegistry.getImsConfig());
    }

    @Test
    @SmallTest
    public void setImsEnabled() throws Exception {
        mServiceRegistry.setImsEnabled(true);
        assertTrue(mServiceRegistry.isImsEnabled());

        mServiceRegistry.setImsEnabled(false);
        assertFalse(mServiceRegistry.isImsEnabled());
    }

    @Test
    @SmallTest
    public void getMmTelFeatureRegistry() throws Exception {
        assertNotNull(mServiceRegistry.getMmTelFeatureRegistry());
    }

    @Test
    @SmallTest
    public void getMmTelMediaRegistry() {
        assertNotNull(mServiceRegistry.getMmTelMediaRegistry());
    }

    @Test
    @SmallTest
    public void createMediaQualityReporter() {
        assertNotNull(mServiceRegistry.createMediaQualityReporter("1"));
    }

    @Test
    @SmallTest
    public void addListener() throws Exception {
        verify(mListener, never()).onMmTelFeatureChanged();
        verify(mListener, never()).onImsOnOffChanged();

        mServiceRegistry.addListener(mListener);
        mServiceRegistry.setMmTelFeature(mMmTelFeature);
        mServiceRegistry.setImsEnabled(true);
        mServiceRegistry.setMmTelFeature(null);
        mServiceRegistry.setImsEnabled(false);

        verify(mListener, times(2)).onMmTelFeatureChanged();
        verify(mListener, times(2)).onImsOnOffChanged();

        mServiceRegistry.removeListener(mListener);
        mServiceRegistry.setMmTelFeature(mMmTelFeature);
        mServiceRegistry.setImsEnabled(true);
        mServiceRegistry.setMmTelFeature(null);
        mServiceRegistry.setImsEnabled(false);

        verifyNoMoreInteractions(mListener);
    }
}
