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
package com.android.imsstack.base;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.database.ContentObserver;
import android.net.Uri;
import android.provider.Settings;
import android.test.mock.MockContentResolver;

import androidx.test.filters.SmallTest;

import com.android.dx.mockito.inline.extended.ExtendedMockito;
import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.ContentProviderProxyImpl.GlobalSettingsProxy;
import com.android.imsstack.base.ContentProviderProxyImpl.SecureSettingsProxy;
import com.android.imsstack.base.ContentProviderProxyImpl.SystemSettingsProxy;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.mockito.MockitoSession;

@RunWith(JUnit4.class)
public class ContentProviderProxyImplTest {
    private static final String TEST_KEY = "test_key";
    private static final String TEST_VALUE = "test_value";
    private static final int TEST_VALUE_INT = 1;
    private static final String TEST_DEFAULT_VALUE = "test_value_default";
    private static final int TEST_DEFAULT_VALUE_INT = 10;

    @Mock private MockContentResolver mContentResolver;

    private ContextFixture mContextFixture;
    private ContentProviderProxyImpl mContentProviderProxy;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mContextFixture = new ContextFixture();
        Context context = mContextFixture.getTestDouble();
        when(context.getContentResolver()).thenReturn(mContentResolver);

        mContentProviderProxy = new ContentProviderProxyImpl(context);
    }

    @After
    public void tearDown() throws Exception {
        mContentProviderProxy = null;
        mContextFixture = null;
    }

    @Test
    @SmallTest
    public void testAccessors() {
        assertNotNull(mContentProviderProxy.getGlobalSettings());
        assertNotNull(mContentProviderProxy.getSecureSettings());
        assertNotNull(mContentProviderProxy.getSystemSettings());
    }

    @Test
    @SmallTest
    public void testRegisterAndUnregisterContentObserver() {
        Uri uri = mock(Uri.class);
        ContentObserver observer = mock(ContentObserver.class);

        mContentProviderProxy.registerContentObserver(uri, observer);
        verify(mContentResolver).registerContentObserver(eq(uri), eq(true), eq(observer));

        mContentProviderProxy.unregisterContentObserver(observer);
        verify(mContentResolver).unregisterContentObserver(eq(observer));
    }

    @Test
    @SmallTest
    public void testGlobalSettings_accessors() {
        MockitoSession mockitoSession =
                ExtendedMockito.mockitoSession()
                        .mockStatic(Settings.Global.class)
                        .startMocking();
        GlobalSettingsProxy settingsProxy = (GlobalSettingsProxy)
                mContentProviderProxy.getGlobalSettings();

        try {
            ExtendedMockito.when(Settings.Global.getInt(
                    eq(mContentResolver), eq(TEST_KEY), eq(TEST_DEFAULT_VALUE_INT)))
                    .thenReturn(TEST_VALUE_INT);
            assertEquals(TEST_VALUE_INT, settingsProxy.getInt(TEST_KEY, TEST_DEFAULT_VALUE_INT));

            ExtendedMockito.doThrow(SecurityException.class)
                    .when(() -> Settings.Global.getInt(any(), eq(TEST_KEY), anyInt()));
            assertEquals(TEST_DEFAULT_VALUE_INT,
                    settingsProxy.getInt(TEST_KEY, TEST_DEFAULT_VALUE_INT));

            ExtendedMockito.when(Settings.Global.getString(eq(mContentResolver), eq(TEST_KEY)))
                    .thenReturn(TEST_VALUE);
            assertEquals(TEST_VALUE, settingsProxy.getString(TEST_KEY, TEST_DEFAULT_VALUE));

            ExtendedMockito.when(Settings.Global.getString(eq(mContentResolver), eq(TEST_KEY)))
                    .thenReturn(null);
            assertEquals(TEST_DEFAULT_VALUE, settingsProxy.getString(TEST_KEY, TEST_DEFAULT_VALUE));

            ExtendedMockito.doThrow(SecurityException.class)
                    .when(() -> Settings.Global.getString(any(), eq(TEST_KEY)));
            assertEquals(TEST_DEFAULT_VALUE, settingsProxy.getString(TEST_KEY, TEST_DEFAULT_VALUE));

            settingsProxy.getUriFor(TEST_KEY);
            ExtendedMockito.verify(() -> Settings.Global.getUriFor(eq(TEST_KEY)));
        } finally {
            mockitoSession.finishMocking();
        }
    }

    @Test
    @SmallTest
    public void testGlobalSettings_registerAndUnregisterContentObserver() {
        ContentObserver observer = mock(ContentObserver.class);
        GlobalSettingsProxy settingsProxy = (GlobalSettingsProxy)
                mContentProviderProxy.getGlobalSettings();

        settingsProxy.registerContentObserver(TEST_KEY, observer);
        verify(mContentResolver).registerContentObserver(any(Uri.class), eq(true), eq(observer));

        settingsProxy.unregisterContentObserver(observer);
        verify(mContentResolver).unregisterContentObserver(eq(observer));
    }

    @Test
    @SmallTest
    public void testSecureSettings_accessors() {
        MockitoSession mockitoSession =
                ExtendedMockito.mockitoSession()
                        .mockStatic(Settings.Secure.class)
                        .startMocking();
        SecureSettingsProxy settingsProxy = (SecureSettingsProxy)
                mContentProviderProxy.getSecureSettings();

        try {
            ExtendedMockito.when(Settings.Secure.getInt(
                    eq(mContentResolver), eq(TEST_KEY), eq(TEST_DEFAULT_VALUE_INT)))
                    .thenReturn(TEST_VALUE_INT);
            assertEquals(TEST_VALUE_INT, settingsProxy.getInt(TEST_KEY, TEST_DEFAULT_VALUE_INT));

            ExtendedMockito.doThrow(SecurityException.class)
                    .when(() -> Settings.Secure.getInt(any(), eq(TEST_KEY), anyInt()));
            assertEquals(TEST_DEFAULT_VALUE_INT,
                    settingsProxy.getInt(TEST_KEY, TEST_DEFAULT_VALUE_INT));

            ExtendedMockito.when(Settings.Secure.getString(eq(mContentResolver), eq(TEST_KEY)))
                    .thenReturn(TEST_VALUE);
            assertEquals(TEST_VALUE, settingsProxy.getString(TEST_KEY, TEST_DEFAULT_VALUE));

            ExtendedMockito.when(Settings.Secure.getString(eq(mContentResolver), eq(TEST_KEY)))
                    .thenReturn(null);
            assertEquals(TEST_DEFAULT_VALUE, settingsProxy.getString(TEST_KEY, TEST_DEFAULT_VALUE));

            ExtendedMockito.doThrow(SecurityException.class)
                    .when(() -> Settings.Secure.getString(any(), eq(TEST_KEY)));
            assertEquals(TEST_DEFAULT_VALUE, settingsProxy.getString(TEST_KEY, TEST_DEFAULT_VALUE));

            settingsProxy.getUriFor(TEST_KEY);
            ExtendedMockito.verify(() -> Settings.Secure.getUriFor(eq(TEST_KEY)));
        } finally {
            mockitoSession.finishMocking();
        }
    }

    @Test
    @SmallTest
    public void testSystemSettings_accessors() {
        MockitoSession mockitoSession =
                ExtendedMockito.mockitoSession()
                        .mockStatic(Settings.System.class)
                        .startMocking();
        SystemSettingsProxy settingsProxy = (SystemSettingsProxy)
                mContentProviderProxy.getSystemSettings();

        try {
            ExtendedMockito.when(Settings.System.getInt(
                    eq(mContentResolver), eq(TEST_KEY), eq(TEST_DEFAULT_VALUE_INT)))
                    .thenReturn(TEST_VALUE_INT);
            assertEquals(TEST_VALUE_INT, settingsProxy.getInt(TEST_KEY, TEST_DEFAULT_VALUE_INT));

            ExtendedMockito.doThrow(SecurityException.class)
                    .when(() -> Settings.System.getInt(any(), eq(TEST_KEY), anyInt()));
            assertEquals(TEST_DEFAULT_VALUE_INT,
                    settingsProxy.getInt(TEST_KEY, TEST_DEFAULT_VALUE_INT));

            ExtendedMockito.when(Settings.System.getString(eq(mContentResolver), eq(TEST_KEY)))
                    .thenReturn(TEST_VALUE);
            assertEquals(TEST_VALUE, settingsProxy.getString(TEST_KEY, TEST_DEFAULT_VALUE));

            ExtendedMockito.when(Settings.System.getString(eq(mContentResolver), eq(TEST_KEY)))
                    .thenReturn(null);
            assertEquals(TEST_DEFAULT_VALUE, settingsProxy.getString(TEST_KEY, TEST_DEFAULT_VALUE));

            ExtendedMockito.doThrow(SecurityException.class)
                    .when(() -> Settings.System.getString(any(), eq(TEST_KEY)));
            assertEquals(TEST_DEFAULT_VALUE, settingsProxy.getString(TEST_KEY, TEST_DEFAULT_VALUE));

            settingsProxy.getUriFor(TEST_KEY);
            ExtendedMockito.verify(() -> Settings.System.getUriFor(eq(TEST_KEY)));
        } finally {
            mockitoSession.finishMocking();
        }
    }
}
