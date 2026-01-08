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
package com.android.imsstack.base;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.SharedPreferences;

import androidx.test.filters.SmallTest;

import com.android.imsstack.ContextFixture;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.List;

@RunWith(JUnit4.class)
public class ImsPrivatePropertiesTest {
    private static final int SLOT0 = 0;

    private static final String TEST_OPERATOR = "TestOperator";
    private static final String TEST_OPERATOR_SUB = "TestOperatorSub";
    private static final String TEST_COUNTRY = "TestCountry";

    private static final String KEY_TEST_BOOL = "TestBoolean";
    private static final String KEY_TEST_INT = "TestInt";
    private static final String KEY_TEST_STRING = "TestString";
    private static final boolean DEFAULT_VALUE_TEST_BOOL = true;
    private static final int DEFAULT_VALUE_TEST_INT = 100;
    private static final String DEFAULT_VALUE_TEST_STRING = "TestStringValue";
    /** Default values in the code under the test */
    private static final boolean DEFAULT_VALUE_BOOL = false;
    private static final int DEFAULT_VALUE_INT = -1;
    private static final String DEFAULT_VALUE_STRING = null;

    @Mock SharedPreferences mSp;
    @Mock SharedPreferences.Editor mSpEditor;

    TestAppContext mTestAppContext;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mTestAppContext = new TestAppContext(new ContextFixture().getTestDouble());
        mTestAppContext.setUp();

        doReturn(mSp).when(mTestAppContext.getContext())
                .getSharedPreferences(anyString(), anyInt());
        when(mSp.edit()).thenReturn(mSpEditor);
        when(mSp.getString(eq(ImsPrivateProperties.Persistent.KEY_SIM_OPERATOR), anyString()))
                .thenReturn(TEST_OPERATOR);
        when(mSp.getString(eq(ImsPrivateProperties.Persistent.KEY_SIM_OPERATOR_SUB), anyString()))
                .thenReturn(TEST_OPERATOR_SUB);
        when(mSp.getString(eq(ImsPrivateProperties.Persistent.KEY_SIM_COUNTRY), anyString()))
                .thenReturn(TEST_COUNTRY);
        // case: default value is null
        when(mSp.getString(eq(KEY_TEST_STRING), eq(null)))
                .thenReturn(DEFAULT_VALUE_TEST_STRING);
        when(mSp.getString(eq(KEY_TEST_STRING), anyString()))
                .thenReturn(DEFAULT_VALUE_TEST_STRING);
        when(mSp.getString(eq(KEY_TEST_BOOL), anyString()))
                .thenReturn(String.valueOf(DEFAULT_VALUE_TEST_BOOL));
        when(mSp.getString(eq(KEY_TEST_INT), anyString()))
                .thenReturn(String.valueOf(DEFAULT_VALUE_TEST_INT));
    }

    @After
    public void tearDown() throws Exception {
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    @SmallTest
    public void testGetSimCountry() {
        ArgumentCaptor<String> keyCaptor = ArgumentCaptor.forClass(String.class);
        ArgumentCaptor<String> defaultValueCaptor = ArgumentCaptor.forClass(String.class);

        String country = ImsPrivateProperties.getSimCountry(SLOT0);

        verify(mSp).getString(keyCaptor.capture(), defaultValueCaptor.capture());

        assertEquals(ImsPrivateProperties.Persistent.KEY_SIM_COUNTRY, keyCaptor.getValue());
        assertEquals("", defaultValueCaptor.getValue());
        assertEquals(TEST_COUNTRY, country);
    }

    @Test
    @SmallTest
    public void testGetSimOperator() {
        ArgumentCaptor<String> keyCaptor = ArgumentCaptor.forClass(String.class);
        ArgumentCaptor<String> defaultValueCaptor = ArgumentCaptor.forClass(String.class);

        String operator = ImsPrivateProperties.getSimOperator(SLOT0);

        verify(mSp).getString(keyCaptor.capture(), defaultValueCaptor.capture());

        assertEquals(ImsPrivateProperties.Persistent.KEY_SIM_OPERATOR, keyCaptor.getValue());
        assertEquals("", defaultValueCaptor.getValue());
        assertEquals(TEST_OPERATOR, operator);
    }

    @Test
    @SmallTest
    public void testGetSimOperatorSub() {
        ArgumentCaptor<String> keyCaptor = ArgumentCaptor.forClass(String.class);
        ArgumentCaptor<String> defaultValueCaptor = ArgumentCaptor.forClass(String.class);

        String operatorSub = ImsPrivateProperties.getSimOperatorSub(SLOT0);

        verify(mSp).getString(keyCaptor.capture(), defaultValueCaptor.capture());

        assertEquals(ImsPrivateProperties.Persistent.KEY_SIM_OPERATOR_SUB, keyCaptor.getValue());
        assertEquals("", defaultValueCaptor.getValue());
        assertEquals(TEST_OPERATOR_SUB, operatorSub);
    }

    @Test
    @SmallTest
    public void testEphemeralGetOperations() {
        ArgumentCaptor<String> keyCaptor = ArgumentCaptor.forClass(String.class);
        ArgumentCaptor<String> defaultValueCaptor = ArgumentCaptor.forClass(String.class);
        int callCount = 3;

        String strValue = ImsPrivateProperties.Ephemeral.get(KEY_TEST_STRING, SLOT0);
        boolean boolValue = ImsPrivateProperties.Ephemeral.getBoolean(KEY_TEST_BOOL, SLOT0);
        int intValue = ImsPrivateProperties.Ephemeral.getInt(KEY_TEST_INT, SLOT0);

        verify(mSp, times(callCount)).getString(keyCaptor.capture(), defaultValueCaptor.capture());

        List<String> keys = keyCaptor.getAllValues();
        List<String> values = defaultValueCaptor.getAllValues();

        assertEquals(KEY_TEST_STRING, keys.get(0));
        assertEquals(KEY_TEST_BOOL, keys.get(1));
        assertEquals(KEY_TEST_INT, keys.get(2));
        assertEquals(DEFAULT_VALUE_STRING, values.get(0));
        assertEquals(String.valueOf(DEFAULT_VALUE_BOOL), values.get(1));
        assertEquals(String.valueOf(DEFAULT_VALUE_INT), values.get(2));
        assertEquals(DEFAULT_VALUE_TEST_STRING, strValue);
        assertEquals(DEFAULT_VALUE_TEST_BOOL, boolValue);
        assertEquals(DEFAULT_VALUE_TEST_INT, intValue);
    }

    @Test
    @SmallTest
    public void testEphemeralGetOperations_defaultValue() {
        ArgumentCaptor<String> keyCaptor = ArgumentCaptor.forClass(String.class);
        ArgumentCaptor<String> defaultValueCaptor = ArgumentCaptor.forClass(String.class);
        int callCount = 3;

        String strValue = ImsPrivateProperties.Ephemeral.get(
                KEY_TEST_STRING, DEFAULT_VALUE_TEST_STRING, SLOT0);
        boolean boolValue = ImsPrivateProperties.Ephemeral.getBoolean(
                KEY_TEST_BOOL, DEFAULT_VALUE_TEST_BOOL, SLOT0);
        int intValue = ImsPrivateProperties.Ephemeral.getInt(
                KEY_TEST_INT, DEFAULT_VALUE_TEST_INT, SLOT0);

        verify(mSp, times(callCount)).getString(keyCaptor.capture(), defaultValueCaptor.capture());

        List<String> keys = keyCaptor.getAllValues();
        List<String> values = defaultValueCaptor.getAllValues();

        assertEquals(KEY_TEST_STRING, keys.get(0));
        assertEquals(KEY_TEST_BOOL, keys.get(1));
        assertEquals(KEY_TEST_INT, keys.get(2));
        assertEquals(DEFAULT_VALUE_TEST_STRING, values.get(0));
        assertEquals(String.valueOf(DEFAULT_VALUE_TEST_BOOL), values.get(1));
        assertEquals(String.valueOf(DEFAULT_VALUE_TEST_INT), values.get(2));
        assertEquals(DEFAULT_VALUE_TEST_STRING, strValue);
        assertEquals(DEFAULT_VALUE_TEST_BOOL, boolValue);
        assertEquals(DEFAULT_VALUE_TEST_INT, intValue);
    }

    @Test
    @SmallTest
    public void testEphemeralSetOperations() {
        ArgumentCaptor<String> keyCaptor = ArgumentCaptor.forClass(String.class);
        ArgumentCaptor<String> valueCaptor = ArgumentCaptor.forClass(String.class);
        int callCount = 3;

        ImsPrivateProperties.Ephemeral.set(KEY_TEST_STRING, DEFAULT_VALUE_TEST_STRING, SLOT0);
        ImsPrivateProperties.Ephemeral.setBoolean(KEY_TEST_BOOL, DEFAULT_VALUE_TEST_BOOL, SLOT0);
        ImsPrivateProperties.Ephemeral.setInt(KEY_TEST_INT, DEFAULT_VALUE_TEST_INT, SLOT0);

        verify(mSpEditor, times(callCount)).putString(keyCaptor.capture(), valueCaptor.capture());

        List<String> keys = keyCaptor.getAllValues();
        List<String> values = valueCaptor.getAllValues();

        assertEquals(KEY_TEST_STRING, keys.get(0));
        assertEquals(KEY_TEST_BOOL, keys.get(1));
        assertEquals(KEY_TEST_INT, keys.get(2));
        assertEquals(DEFAULT_VALUE_TEST_STRING, values.get(0));
        assertEquals(String.valueOf(DEFAULT_VALUE_TEST_BOOL), values.get(1));
        assertEquals(String.valueOf(DEFAULT_VALUE_TEST_INT), values.get(2));
    }

    @Test
    @SmallTest
    public void testEphemeralRemove() {
        ImsPrivateProperties.Ephemeral.remove(KEY_TEST_STRING, SLOT0);

        verify(mSpEditor).remove(eq(KEY_TEST_STRING));
        verify(mSpEditor).commit();
    }

    @Test
    @SmallTest
    public void testEphemeralRemoveAll() {
        ImsPrivateProperties.Ephemeral.removeAll(SLOT0);

        verify(mSpEditor).clear();
        verify(mSpEditor).commit();
    }

    @Test
    @SmallTest
    public void testPersistentGetOperations() {
        ArgumentCaptor<String> keyCaptor = ArgumentCaptor.forClass(String.class);
        ArgumentCaptor<String> defaultValueCaptor = ArgumentCaptor.forClass(String.class);
        int callCount = 3;

        String strValue = ImsPrivateProperties.Persistent.get(KEY_TEST_STRING, SLOT0);
        boolean boolValue = ImsPrivateProperties.Persistent.getBoolean(KEY_TEST_BOOL, SLOT0);
        int intValue = ImsPrivateProperties.Persistent.getInt(KEY_TEST_INT, SLOT0);

        verify(mSp, times(callCount)).getString(keyCaptor.capture(), defaultValueCaptor.capture());

        List<String> keys = keyCaptor.getAllValues();
        List<String> values = defaultValueCaptor.getAllValues();

        assertEquals(KEY_TEST_STRING, keys.get(0));
        assertEquals(KEY_TEST_BOOL, keys.get(1));
        assertEquals(KEY_TEST_INT, keys.get(2));
        assertEquals(DEFAULT_VALUE_STRING, values.get(0));
        assertEquals(String.valueOf(DEFAULT_VALUE_BOOL), values.get(1));
        assertEquals(String.valueOf(DEFAULT_VALUE_INT), values.get(2));
        assertEquals(DEFAULT_VALUE_TEST_STRING, strValue);
        assertEquals(DEFAULT_VALUE_TEST_BOOL, boolValue);
        assertEquals(DEFAULT_VALUE_TEST_INT, intValue);
    }

    @Test
    @SmallTest
    public void testPersistentGetOperations_defaultValue() {
        ArgumentCaptor<String> keyCaptor = ArgumentCaptor.forClass(String.class);
        ArgumentCaptor<String> defaultValueCaptor = ArgumentCaptor.forClass(String.class);
        int callCount = 3;

        String strValue = ImsPrivateProperties.Persistent.get(
                KEY_TEST_STRING, DEFAULT_VALUE_TEST_STRING, SLOT0);
        boolean boolValue = ImsPrivateProperties.Persistent.getBoolean(
                KEY_TEST_BOOL, DEFAULT_VALUE_TEST_BOOL, SLOT0);
        int intValue = ImsPrivateProperties.Persistent.getInt(
                KEY_TEST_INT, DEFAULT_VALUE_TEST_INT, SLOT0);

        verify(mSp, times(callCount)).getString(keyCaptor.capture(), defaultValueCaptor.capture());

        List<String> keys = keyCaptor.getAllValues();
        List<String> values = defaultValueCaptor.getAllValues();

        assertEquals(KEY_TEST_STRING, keys.get(0));
        assertEquals(KEY_TEST_BOOL, keys.get(1));
        assertEquals(KEY_TEST_INT, keys.get(2));
        assertEquals(DEFAULT_VALUE_TEST_STRING, values.get(0));
        assertEquals(String.valueOf(DEFAULT_VALUE_TEST_BOOL), values.get(1));
        assertEquals(String.valueOf(DEFAULT_VALUE_TEST_INT), values.get(2));
        assertEquals(DEFAULT_VALUE_TEST_STRING, strValue);
        assertEquals(DEFAULT_VALUE_TEST_BOOL, boolValue);
        assertEquals(DEFAULT_VALUE_TEST_INT, intValue);
    }

    @Test
    @SmallTest
    public void testPersistentSetOperations() {
        ArgumentCaptor<String> keyCaptor = ArgumentCaptor.forClass(String.class);
        ArgumentCaptor<String> valueCaptor = ArgumentCaptor.forClass(String.class);
        int callCount = 3;

        ImsPrivateProperties.Persistent.set(KEY_TEST_STRING, DEFAULT_VALUE_TEST_STRING, SLOT0);
        ImsPrivateProperties.Persistent.setBoolean(KEY_TEST_BOOL, DEFAULT_VALUE_TEST_BOOL, SLOT0);
        ImsPrivateProperties.Persistent.setInt(KEY_TEST_INT, DEFAULT_VALUE_TEST_INT, SLOT0);

        verify(mSpEditor, times(callCount)).putString(keyCaptor.capture(), valueCaptor.capture());

        List<String> keys = keyCaptor.getAllValues();
        List<String> values = valueCaptor.getAllValues();

        assertEquals(KEY_TEST_STRING, keys.get(0));
        assertEquals(KEY_TEST_BOOL, keys.get(1));
        assertEquals(KEY_TEST_INT, keys.get(2));
        assertEquals(DEFAULT_VALUE_TEST_STRING, values.get(0));
        assertEquals(String.valueOf(DEFAULT_VALUE_TEST_BOOL), values.get(1));
        assertEquals(String.valueOf(DEFAULT_VALUE_TEST_INT), values.get(2));
    }

    @Test
    @SmallTest
    public void testPersistentRemoveTestProperties() {
        int callCount = ImsPrivateProperties.Persistent.TEST_PROPERTIES.length;

        ImsPrivateProperties.Persistent.removeTestProperties(SLOT0);

        verify(mSpEditor, times(callCount)).remove(anyString());
        verify(mSpEditor).commit();
    }

    @Test
    @SmallTest
    public void testPersistentIsConfigProperty() {
        assertTrue(ImsPrivateProperties.Persistent.isConfigProperty(
                ImsPrivateProperties.Persistent.KEY_CONFIG_PCSCF_ADDRESS_LIST));
        assertTrue(ImsPrivateProperties.Persistent.isConfigProperty(
                ImsPrivateProperties.Persistent.KEY_CONFIG_IMPI));
        assertTrue(ImsPrivateProperties.Persistent.isConfigProperty(
                ImsPrivateProperties.Persistent.KEY_CONFIG_IMPU_LIST));
        assertTrue(ImsPrivateProperties.Persistent.isConfigProperty(
                ImsPrivateProperties.Persistent.KEY_CONFIG_HOME_DOMAIN_NAME));

        assertFalse(ImsPrivateProperties.Persistent.isConfigProperty(KEY_TEST_STRING));
        assertFalse(ImsPrivateProperties.Persistent.isConfigProperty(KEY_TEST_BOOL));
        assertFalse(ImsPrivateProperties.Persistent.isConfigProperty(KEY_TEST_INT));
    }

    @Test
    @SmallTest
    public void testGetInt_returnEmptyOrNullString_defaultValue() {
        when(mSp.getString(eq(KEY_TEST_INT), anyString())).thenReturn("");

        final int defaultIntValue = DEFAULT_VALUE_TEST_INT + 1;

        assertEquals(defaultIntValue,
                ImsPrivateProperties.Persistent.getInt(KEY_TEST_INT, defaultIntValue, SLOT0));
        assertEquals(defaultIntValue,
                ImsPrivateProperties.Ephemeral.getInt(KEY_TEST_INT, defaultIntValue, SLOT0));

        when(mSp.getString(eq(KEY_TEST_INT), anyString())).thenReturn(null);

        assertEquals(defaultIntValue,
                ImsPrivateProperties.Persistent.getInt(KEY_TEST_INT, defaultIntValue, SLOT0));
        assertEquals(defaultIntValue,
                ImsPrivateProperties.Ephemeral.getInt(KEY_TEST_INT, defaultIntValue, SLOT0));
    }
}
