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
package com.android.imsstack.core.config;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.os.Parcel;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.base.TestAppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import java.util.Arrays;

@RunWith(JUnit4.class)
public class CarrierConfigTest {
    private static final int SIP_18X_TIMER_MILLIS = 32000;
    private static final int[] PCSCF_DISCOVERY_METHOD_LIST = { 0, 1 };
    private static final String CALL_TERMINATE_REASON_HEADER_USER_ENDS_CALL_STRING =
            "RELEASE_CAUSE;cause=1;text=\"User ends call\"";
    private static final int[] AMRWB_PAYLOAD_TYPE_LIST = { 97, 98 };
    private static final int[] AMR_CODEC_ATTRIBUTE_MODESET_LIST = { 0, 1, 2 };
    private static final int[] ASSET_AMRWB_PAYLOAD_TYPE_LIST = { 101, 102 };
    private static final int[] ASSET_AMR_CODEC_ATTRIBUTE_MODESET_LIST = { 0, 1, 2, 3, 4 };
    private static final String[] CARRIER_SPECIFIC_SIP_HEADERS = { "P-Header1", "P-Header2" };
    // Test keys and values.
    private static final String KEY_TEST_BOOL_ARRAY = "key_test_bool_array";
    private static final boolean[] TEST_BOOL_LIST = { true, false };
    private static final String KEY_TEST_LONG = "key_test_long";
    private static final long TEST_LONG = 1000L;
    private static final String KEY_TEST_LONG_ARRAY = "key_test_long_array";
    private static final long[] TEST_LONG_LIST = { 10000L, 20000L };
    private static final String KEY_TEST_DOUBLE = "key_test_double";
    private static final double TEST_DOUBLE = 3.14;
    private static final String KEY_TEST_DOUBLE_ARRAY = "key_test_double_array";
    private static final double[] TEST_DOUBLE_LIST = { 3.14, 3.15 };

    private CarrierConfig mCarrierConfig;

    @Before
    public void setUp() throws Exception {
        mCarrierConfig = new CarrierConfig();
    }

    @After
    public void tearDown() throws Exception {
        mCarrierConfig = null;
    }

    @Test
    @SmallTest
    public void testGetDefaultValues() {
        assertNotNull(mCarrierConfig.getConfig());
        assertFalse(mCarrierConfig.isVoLteProvisioningRequired());
        assertFalse(mCarrierConfig.getBoolean(CarrierConfig.Ims.KEY_SIP_COMPACT_FORM_ENABLED_BOOL));
        assertNull(mCarrierConfig.getBooleanArray(KEY_TEST_BOOL_ARRAY));
        assertEquals(-1, mCarrierConfig.getInt(
                CarrierConfig.ImsVoice.KEY_SIP_18X_TIMER_MILLIS_INT));
        assertNull(mCarrierConfig.getIntArray(
                CarrierConfig.Ims.KEY_PCSCF_DISCOVERY_METHOD_INT_ARRAY));
        assertNull(mCarrierConfig.getString(
                CarrierConfig.ImsVoice.KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_CALL_STRING));
        assertNull(mCarrierConfig.getStringArray(
                CarrierConfig.ImsVoice.KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY));
        assertEquals(-1L, mCarrierConfig.getLong(KEY_TEST_LONG));
        assertNull(mCarrierConfig.getLongArray(KEY_TEST_BOOL_ARRAY));
        assertNull(mCarrierConfig.getBundle(
                CarrierConfigManager.ImsVoice.KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE));
    }

    @Test
    @SmallTest
    public void testSetConfig() {
        mCarrierConfig.setConfig(createBundle(), TestAppContext.SLOT0);

        assertNotNull(mCarrierConfig.getConfig());
        assertTrue(mCarrierConfig.isVoLteProvisioningRequired());
        assertTrue(mCarrierConfig.getBoolean(CarrierConfig.Ims.KEY_SIP_COMPACT_FORM_ENABLED_BOOL));
        assertTrue(Arrays.equals(TEST_BOOL_LIST,
                mCarrierConfig.getBooleanArray(KEY_TEST_BOOL_ARRAY)));
        assertEquals(SIP_18X_TIMER_MILLIS, mCarrierConfig.getInt(
                CarrierConfig.ImsVoice.KEY_SIP_18X_TIMER_MILLIS_INT));
        assertTrue(Arrays.equals(PCSCF_DISCOVERY_METHOD_LIST, mCarrierConfig.getIntArray(
                CarrierConfig.Ims.KEY_PCSCF_DISCOVERY_METHOD_INT_ARRAY)));
        assertEquals(CALL_TERMINATE_REASON_HEADER_USER_ENDS_CALL_STRING, mCarrierConfig.getString(
                CarrierConfig.ImsVoice.KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_CALL_STRING));
        assertTrue(Arrays.equals(CARRIER_SPECIFIC_SIP_HEADERS, mCarrierConfig.getStringArray(
                CarrierConfig.ImsVoice.KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY)));
        assertEquals(TEST_LONG, mCarrierConfig.getLong(KEY_TEST_LONG));
        assertTrue(Arrays.equals(TEST_LONG_LIST, mCarrierConfig.getLongArray(KEY_TEST_LONG_ARRAY)));
        assertNotNull(mCarrierConfig.getBundle(
                CarrierConfigManager.ImsVoice.KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE));
    }

    @Test
    @SmallTest
    public void testSetConfigForUserAgent() {
        TestAppContext testAppContext = new TestAppContext();
        testAppContext.setUp();
        TelephonyManagerProxy tmp =
                testAppContext.getSystemServiceProxy(TelephonyManagerProxy.class);
        when(tmp.getImei(eq(TestAppContext.SLOT0)))
                .thenReturn("000000000000001", "000000000000001", null, null, null, null);
        when(tmp.getDeviceSoftwareVersion(eq(TestAppContext.SLOT0))).thenReturn("01", null, "1");
        when(tmp.getSimOperator()).thenReturn("001001", "", "00111");

        try {
            PersistableBundle b = new PersistableBundle();
            b.putString(CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING,
                    "#MANUFACTURER#_#MODEL#_Android#AV#_#BUILD#_#IMEI#_#IMEISV#_#TEST#");

            // IMEI: not empty, SV: not empty
            mCarrierConfig.setConfig(b, TestAppContext.SLOT0);

            String userAgent = mCarrierConfig.getString(
                    CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING);
            assertNotNull(userAgent);
            assertFalse(userAgent.contains("#"));
            assertFalse(userAgent.contains(" "));
            assertTrue(userAgent.endsWith("TEST"));

            // IMEI: empty, SV: empty
            mCarrierConfig.setConfig(b, TestAppContext.SLOT0);

            userAgent = mCarrierConfig.getString(
                    CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING);
            assertNotNull(userAgent);
            assertFalse(userAgent.contains("#"));
            assertTrue(userAgent.endsWith("TEST"));

            // IMEI: empty, SV: not empty (1 character)
            mCarrierConfig.setConfig(b, TestAppContext.SLOT0);

            userAgent = mCarrierConfig.getString(
                    CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING);
            assertNotNull(userAgent);
            assertFalse(userAgent.contains("#"));
            assertTrue(userAgent.endsWith("TEST"));

            verify(tmp, times(6)).getImei(anyInt());
            verify(tmp, times(3)).getDeviceSoftwareVersion(anyInt());

            // Use a default UA string
            b.putString(CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING, "IMS-client");
            mCarrierConfig.setConfig(b, TestAppContext.SLOT0);

            userAgent = mCarrierConfig.getString(
                    CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING);
            assertEquals("IMS-client", userAgent);

            // Empty UA string
            b.putString(CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING, "");
            mCarrierConfig.setConfig(b, TestAppContext.SLOT0);

            userAgent = mCarrierConfig.getString(
                    CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING);
            assertEquals("", userAgent);

            // Null UA string
            b.putString(CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING, null);
            mCarrierConfig.setConfig(b, TestAppContext.SLOT0);

            userAgent = mCarrierConfig.getString(
                    CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING);
            assertNull(userAgent);

            // MCCMNC: length 6
            b.putString(CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING,
                    "#MANUFACTURE#/#HWSKU#-#BUILD# device-type/#TYPE# mno-custom/#MCCMNC#");

            mCarrierConfig.setConfig(b, TestAppContext.SLOT0);

            userAgent = mCarrierConfig.getString(
                    CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING);
            assertNotNull(userAgent);
            assertFalse(userAgent.contains("#"));
            assertTrue(userAgent.endsWith("device-type/smart-phone mno-custom/001001"));

            // MCCMNC: empty
            b.putString(CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING,
                    "#MANUFACTURE#/#HWSKU#-#BUILD# device-type/#TYPE# mno-custom/#MCCMNC#");

            mCarrierConfig.setConfig(b, TestAppContext.SLOT0);

            userAgent = mCarrierConfig.getString(
                    CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING);
            assertNotNull(userAgent);
            assertFalse(userAgent.contains("#"));
            assertTrue(userAgent.endsWith("device-type/smart-phone mno-custom/000000"));

            // MCCMNC: length 5
            b.putString(CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING,
                    "#MANUFACTURE#/#HWSKU#-#BUILD# device-type/#TYPE# mno-custom/#MCCMNC#");

            mCarrierConfig.setConfig(b, TestAppContext.SLOT0);

            userAgent = mCarrierConfig.getString(
                    CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING);
            assertNotNull(userAgent);
            assertFalse(userAgent.contains("#"));
            assertTrue(userAgent.endsWith("device-type/smart-phone mno-custom/001011"));
        } finally {
            testAppContext.tearDown();
            testAppContext = null;
        }
    }

    @Test
    @SmallTest
    public void getValue() {
        PersistableBundle config = new PersistableBundle();

        assertEquals("false", CarrierConfig.getValue(config, "test_config_boolean"));
        assertEquals("false", CarrierConfig.getValue(config,
                CarrierConfig.Ims.KEY_SIP_COMPACT_FORM_ENABLED_BOOL));
        assertEquals("null", CarrierConfig.getValue(config, KEY_TEST_BOOL_ARRAY));
        assertEquals("-1", CarrierConfig.getValue(config,
                CarrierConfig.ImsVoice.KEY_SIP_18X_TIMER_MILLIS_INT));
        assertEquals("null", CarrierConfig.getValue(config,
                CarrierConfig.Ims.KEY_PCSCF_DISCOVERY_METHOD_INT_ARRAY));
        assertEquals(null, CarrierConfig.getValue(config,
                CarrierConfig.ImsVoice.KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_CALL_STRING));
        assertEquals("null", CarrierConfig.getValue(config,
                CarrierConfig.ImsVoice.KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY));
        assertEquals("-1", CarrierConfig.getValue(config, KEY_TEST_LONG));
        assertEquals("null", CarrierConfig.getValue(config, KEY_TEST_LONG_ARRAY));
        assertEquals("0.0", CarrierConfig.getValue(config, KEY_TEST_DOUBLE));
        assertEquals("null", CarrierConfig.getValue(config, KEY_TEST_DOUBLE_ARRAY));
        assertEquals("null", CarrierConfig.getValue(config,
                CarrierConfigManager.ImsVoice.KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE));

        config = createBundle();

        assertNotEquals("false", CarrierConfig.getValue(config,
                CarrierConfig.Ims.KEY_SIP_COMPACT_FORM_ENABLED_BOOL));
        assertNotEquals("null", CarrierConfig.getValue(config, KEY_TEST_BOOL_ARRAY));
        assertNotEquals("-1", CarrierConfig.getValue(config,
                CarrierConfig.ImsVoice.KEY_SIP_18X_TIMER_MILLIS_INT));
        assertNotEquals("null", CarrierConfig.getValue(config,
                CarrierConfig.Ims.KEY_PCSCF_DISCOVERY_METHOD_INT_ARRAY));
        assertNotEquals(null, CarrierConfig.getValue(config,
                CarrierConfig.ImsVoice.KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_CALL_STRING));
        assertNotEquals("null", CarrierConfig.getValue(config,
                CarrierConfig.ImsVoice.KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY));
        assertNotEquals("-1", CarrierConfig.getValue(config, KEY_TEST_LONG));
        assertNotEquals("null", CarrierConfig.getValue(config, KEY_TEST_LONG_ARRAY));
        assertNotEquals("0.0", CarrierConfig.getValue(config, KEY_TEST_DOUBLE));
        assertNotEquals("null", CarrierConfig.getValue(config, KEY_TEST_DOUBLE_ARRAY));
        assertNotEquals("null", CarrierConfig.getValue(config,
                CarrierConfigManager.ImsVoice.KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE));
        assertNotNull(CarrierConfig.getValue(config, "test"));
        assertNotEquals("null", CarrierConfig.getValue(config,
                CarrierConfigManager.KEY_IGNORE_DATA_ENABLED_CHANGED_FOR_VIDEO_CALLS));

        // Digit keys
        PersistableBundle codecConfig = createNestedBundle(
                AMRWB_PAYLOAD_TYPE_LIST, AMR_CODEC_ATTRIBUTE_MODESET_LIST);
        PersistableBundle amrWbDescConfig = codecConfig.getPersistableBundle(
                CarrierConfigManager.ImsVoice.KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE);

        assertNotEquals("null", CarrierConfig.getValue(amrWbDescConfig,
                String.valueOf(AMRWB_PAYLOAD_TYPE_LIST[1])));
    }

    @Test
    @SmallTest
    public void overrideNestedBundles() {
        PersistableBundle defaultConfig = createNestedBundle(
                AMRWB_PAYLOAD_TYPE_LIST, AMR_CODEC_ATTRIBUTE_MODESET_LIST);
        PersistableBundle overrideConfig = createNestedBundle(
                ASSET_AMRWB_PAYLOAD_TYPE_LIST, ASSET_AMR_CODEC_ATTRIBUTE_MODESET_LIST);

        CarrierConfig.overrideNestedBundles(defaultConfig, overrideConfig);

        PersistableBundle audioPayloadTypes = overrideConfig.getPersistableBundle(
                CarrierConfigManager.ImsVoice.KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE);
        assertTrue(Arrays.equals(ASSET_AMRWB_PAYLOAD_TYPE_LIST, audioPayloadTypes.getIntArray(
                CarrierConfigManager.ImsVoice.KEY_AMRWB_PAYLOAD_TYPE_INT_ARRAY)));

        PersistableBundle amrWbPayloadDescription = overrideConfig.getPersistableBundle(
                CarrierConfigManager.ImsVoice.KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE);
        assertNotNull(amrWbPayloadDescription);

        PersistableBundle amrWbPayload0 = amrWbPayloadDescription.getPersistableBundle(
                String.valueOf(ASSET_AMRWB_PAYLOAD_TYPE_LIST[0]));
        assertNotNull(amrWbPayload0);
        assertEquals(CarrierConfigManager.ImsVoice.BANDWIDTH_EFFICIENT, amrWbPayload0.getInt(
                CarrierConfigManager.ImsVoice.KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT,
                CarrierConfigManager.ImsVoice.BANDWIDTH_EFFICIENT));
        assertTrue(Arrays.equals(ASSET_AMR_CODEC_ATTRIBUTE_MODESET_LIST, amrWbPayload0.getIntArray(
                CarrierConfigManager.ImsVoice.KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY)));

        PersistableBundle amrWbPayload1 = amrWbPayloadDescription.getPersistableBundle(
                String.valueOf(ASSET_AMRWB_PAYLOAD_TYPE_LIST[1]));
        assertNotNull(amrWbPayload1);
        assertEquals(CarrierConfigManager.ImsVoice.OCTET_ALIGNED, amrWbPayload1.getInt(
                CarrierConfigManager.ImsVoice.KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT,
                CarrierConfigManager.ImsVoice.BANDWIDTH_EFFICIENT));
        assertTrue(Arrays.equals(ASSET_AMR_CODEC_ATTRIBUTE_MODESET_LIST, amrWbPayload1.getIntArray(
                CarrierConfigManager.ImsVoice.KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY)));
    }

    @Test
    @SmallTest
    public void overrideNestedBundlesForDefaultOnly() {
        PersistableBundle defaultConfig = createNestedBundle(
                AMRWB_PAYLOAD_TYPE_LIST, AMR_CODEC_ATTRIBUTE_MODESET_LIST);
        PersistableBundle overrideConfig = new PersistableBundle();

        CarrierConfig.overrideNestedBundles(defaultConfig, overrideConfig);

        PersistableBundle audioPayloadTypes = overrideConfig.getPersistableBundle(
                CarrierConfigManager.ImsVoice.KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE);
        assertTrue(Arrays.equals(AMRWB_PAYLOAD_TYPE_LIST, audioPayloadTypes.getIntArray(
                CarrierConfigManager.ImsVoice.KEY_AMRWB_PAYLOAD_TYPE_INT_ARRAY)));

        PersistableBundle amrWbPayloadDescription = overrideConfig.getPersistableBundle(
                CarrierConfigManager.ImsVoice.KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE);
        assertNotNull(amrWbPayloadDescription);

        PersistableBundle amrWbPayload0 = amrWbPayloadDescription.getPersistableBundle(
                String.valueOf(AMRWB_PAYLOAD_TYPE_LIST[0]));
        assertNotNull(amrWbPayload0);
        assertEquals(CarrierConfigManager.ImsVoice.BANDWIDTH_EFFICIENT, amrWbPayload0.getInt(
                CarrierConfigManager.ImsVoice.KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT,
                CarrierConfigManager.ImsVoice.BANDWIDTH_EFFICIENT));
        assertTrue(Arrays.equals(AMR_CODEC_ATTRIBUTE_MODESET_LIST, amrWbPayload0.getIntArray(
                CarrierConfigManager.ImsVoice.KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY)));

        PersistableBundle amrWbPayload1 = amrWbPayloadDescription.getPersistableBundle(
                String.valueOf(AMRWB_PAYLOAD_TYPE_LIST[1]));
        assertNotNull(amrWbPayload1);
        assertEquals(CarrierConfigManager.ImsVoice.OCTET_ALIGNED, amrWbPayload1.getInt(
                CarrierConfigManager.ImsVoice.KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT,
                CarrierConfigManager.ImsVoice.BANDWIDTH_EFFICIENT));
        assertTrue(Arrays.equals(AMR_CODEC_ATTRIBUTE_MODESET_LIST, amrWbPayload1.getIntArray(
                CarrierConfigManager.ImsVoice.KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY)));
    }

    @Test
    @SmallTest
    public void overrideNestedBundlesForOverrideOnly() {
        PersistableBundle defaultConfig = new PersistableBundle();
        PersistableBundle overrideConfig = createNestedBundle(
                ASSET_AMRWB_PAYLOAD_TYPE_LIST, ASSET_AMR_CODEC_ATTRIBUTE_MODESET_LIST);

        CarrierConfig.overrideNestedBundles(defaultConfig, overrideConfig);

        PersistableBundle audioPayloadTypes = overrideConfig.getPersistableBundle(
                CarrierConfigManager.ImsVoice.KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE);
        assertTrue(Arrays.equals(ASSET_AMRWB_PAYLOAD_TYPE_LIST, audioPayloadTypes.getIntArray(
                CarrierConfigManager.ImsVoice.KEY_AMRWB_PAYLOAD_TYPE_INT_ARRAY)));

        PersistableBundle amrWbPayloadDescription = overrideConfig.getPersistableBundle(
                CarrierConfigManager.ImsVoice.KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE);
        assertNotNull(amrWbPayloadDescription);

        PersistableBundle amrWbPayload0 = amrWbPayloadDescription.getPersistableBundle(
                String.valueOf(ASSET_AMRWB_PAYLOAD_TYPE_LIST[0]));
        assertNotNull(amrWbPayload0);
        assertEquals(CarrierConfigManager.ImsVoice.BANDWIDTH_EFFICIENT, amrWbPayload0.getInt(
                CarrierConfigManager.ImsVoice.KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT,
                CarrierConfigManager.ImsVoice.BANDWIDTH_EFFICIENT));
        assertTrue(Arrays.equals(ASSET_AMR_CODEC_ATTRIBUTE_MODESET_LIST, amrWbPayload0.getIntArray(
                CarrierConfigManager.ImsVoice.KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY)));

        PersistableBundle amrWbPayload1 = amrWbPayloadDescription.getPersistableBundle(
                String.valueOf(ASSET_AMRWB_PAYLOAD_TYPE_LIST[1]));
        assertNotNull(amrWbPayload1);
        assertEquals(CarrierConfigManager.ImsVoice.OCTET_ALIGNED, amrWbPayload1.getInt(
                CarrierConfigManager.ImsVoice.KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT,
                CarrierConfigManager.ImsVoice.BANDWIDTH_EFFICIENT));
        assertTrue(Arrays.equals(ASSET_AMR_CODEC_ATTRIBUTE_MODESET_LIST, amrWbPayload1.getIntArray(
                CarrierConfigManager.ImsVoice.KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY)));
    }

    @Test
    @SmallTest
    public void writeToParcel() {
        Parcel p = Parcel.obtain();
        assertEquals(0, p.dataSize());

        mCarrierConfig.writeToParcel(p);
        p.setDataPosition(0);

        // Length only (4 bytes)
        assertEquals(4, p.dataSize());
        assertEquals(0, p.readInt());

        p.recycle();
        p = Parcel.obtain();

        mCarrierConfig.setConfig(createBundle(), TestAppContext.SLOT0);
        mCarrierConfig.writeToParcel(p);

        // Greater than length field (4 bytes)
        assertTrue(p.dataSize() > 4);

        p.recycle();
    }

    private PersistableBundle createBundle() {
        PersistableBundle b = new PersistableBundle();
        b.putBoolean(CarrierConfigManager.KEY_CARRIER_VOLTE_PROVISIONING_REQUIRED_BOOL, true);
        b.putBoolean(CarrierConfig.Ims.KEY_SIP_COMPACT_FORM_ENABLED_BOOL, true);
        b.putBooleanArray(KEY_TEST_BOOL_ARRAY, TEST_BOOL_LIST);
        b.putInt(CarrierConfig.ImsVoice.KEY_SIP_18X_TIMER_MILLIS_INT, SIP_18X_TIMER_MILLIS);
        b.putIntArray(CarrierConfig.Ims.KEY_PCSCF_DISCOVERY_METHOD_INT_ARRAY,
                PCSCF_DISCOVERY_METHOD_LIST);
        b.putString(CarrierConfig.ImsVoice.KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_CALL_STRING,
                CALL_TERMINATE_REASON_HEADER_USER_ENDS_CALL_STRING);
        b.putStringArray(CarrierConfig.ImsVoice.KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                CARRIER_SPECIFIC_SIP_HEADERS);
        b.putLong(KEY_TEST_LONG, TEST_LONG);
        b.putLongArray(KEY_TEST_LONG_ARRAY, TEST_LONG_LIST);
        b.putDouble(KEY_TEST_DOUBLE, TEST_DOUBLE);
        b.putDoubleArray(KEY_TEST_DOUBLE_ARRAY, TEST_DOUBLE_LIST);

        PersistableBundle audioPayloadTypes = new PersistableBundle();
        audioPayloadTypes.putIntArray(
                CarrierConfigManager.ImsVoice.KEY_AMRWB_PAYLOAD_TYPE_INT_ARRAY,
                AMRWB_PAYLOAD_TYPE_LIST);
        b.putPersistableBundle(
                CarrierConfigManager.ImsVoice.KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE,
                audioPayloadTypes);

        return b;
    }

    private PersistableBundle createNestedBundle(int[] payloadTypes, int[] modesets) {
        PersistableBundle b = new PersistableBundle();

        PersistableBundle audioPayloadTypes = new PersistableBundle();
        audioPayloadTypes.putIntArray(
                CarrierConfigManager.ImsVoice.KEY_AMRWB_PAYLOAD_TYPE_INT_ARRAY,
                payloadTypes);
        b.putPersistableBundle(
                CarrierConfigManager.ImsVoice.KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE,
                audioPayloadTypes);

        PersistableBundle amrWbPayloadDescription = new PersistableBundle();
        PersistableBundle amrWbPayload0 = new PersistableBundle();
        amrWbPayload0.putIntArray(
                CarrierConfigManager.ImsVoice.KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY,
                modesets);
        amrWbPayloadDescription.putPersistableBundle(
                String.valueOf(payloadTypes[0]), amrWbPayload0);

        PersistableBundle amrWbPayload1 = new PersistableBundle();
        amrWbPayload1.putInt(
                CarrierConfigManager.ImsVoice.KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT,
                CarrierConfigManager.ImsVoice.OCTET_ALIGNED);
        amrWbPayload1.putIntArray(
                CarrierConfigManager.ImsVoice.KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY,
                modesets);
        amrWbPayloadDescription.putPersistableBundle(
                String.valueOf(payloadTypes[1]), amrWbPayload1);
        b.putPersistableBundle(
                CarrierConfigManager.ImsVoice.KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE,
                amrWbPayloadDescription);

        return b;
    }
}
