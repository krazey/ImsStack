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

package com.android.imsstack.imsservice.mmtel;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.mockito.Mockito.when;

import android.location.Location;
import android.telephony.ims.ImsCallProfile;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.mtc.SuppInfo;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;

@RunWith(JUnit4.class)
public class ImsSuppInfoUtilsTest {
    private static final String TAG = "[ImsSuppInfoUtilsTest]";
    private static final int SLOT_ID = 0;

    private int mInfo = 1;
    private SuppInfo mSuppInfo;
    private ImsCallProfile mCallProfile;
    private ICallContext mMockContext;
    private CarrierConfig mMockCarrierConfig;
    private ConfigInterface mMockConfigInterface;

    @Before
    public void setUp() throws Exception {
        mCallProfile = new ImsCallProfile();
        mSuppInfo = new SuppInfo();
        mMockContext = Mockito.mock(ICallContext.class);
        mMockCarrierConfig = Mockito.mock(CarrierConfig.class);
        mMockConfigInterface = Mockito.mock(ConfigInterface.class);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, SLOT_ID);
    }

    @After
    public void tearDown() throws Exception {
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT_ID);
        mCallProfile = null;
        mSuppInfo = null;
    }

    @Test
    public void test_getCallExtraNameForBoolean() {
        assertEquals(null, ImsSuppInfoUtils.getCallExtraNameForBoolean(mMockContext, mInfo));
    }

    @Test
    public void test_getCallExtraNameForInt() {
        assertEquals(null, ImsSuppInfoUtils.getCallExtraNameForInt(mMockContext, mInfo));
    }

    @Test
    public void test_getCallExtraNameForString() {
        assertEquals(null, ImsSuppInfoUtils.getCallExtraNameForString(mMockContext, mInfo));
    }

    @Test
    public void test_addCallExtraForApp() {
        mSuppInfo.addService_int(SuppInfo.TYPE_CDIV_CAUSE, 1);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.ImsVoice.KEY_SUPPINFO_CDIV_CAUSE_REQUIRED_BOOL)).thenReturn(true);
        ImsSuppInfoUtils.addCallExtraForApp(mMockContext, mSuppInfo, mCallProfile);
        assertEquals(1, mCallProfile.getCallExtraInt(ImsCallUtils.EXTRA_CDIV_CAUSE));

        mSuppInfo = new SuppInfo();
        ImsSuppInfoUtils.addCallExtraForApp(mMockContext, mSuppInfo, mCallProfile);
        assertEquals(-1, mCallProfile.getCallExtraInt(ImsCallUtils.EXTRA_CDIV_CAUSE));
    }

    @Test
    public void test_addSuppInfoForIms() {
        mCallProfile.setCallExtraBoolean(ImsSuppInfoUtils.EXTRA_GEOLOCATION, true);
        ImsSuppInfoUtils.addSuppInfoForIms(mMockContext, mCallProfile, mSuppInfo);
        SuppInfo.SuppService ss = mSuppInfo.getService(SuppInfo.TYPE_GEOLOCATION);
        assertNotNull(ss);
        assertEquals(true, ss.boolValue);
    }

    @Test
    public void test_addCallExtraForCallComposerWhenEmpty() {
        ImsSuppInfoUtils.addCallExtraForCallComposer(mSuppInfo, mCallProfile);

        assertEquals(-1, mCallProfile.getCallExtraInt(ImsCallProfile.EXTRA_PRIORITY));
        assertEquals("", mCallProfile.getCallExtra(ImsCallProfile.EXTRA_CALL_SUBJECT));
        assertEquals("", mCallProfile.getCallExtra(ImsCallProfile.EXTRA_PICTURE_URL));
        assertEquals(null, mCallProfile.getCallExtraParcelable(ImsCallProfile.EXTRA_LOCATION));
        assertEquals(false,
                mCallProfile.getCallExtraBoolean(ImsCallProfile.EXTRA_IS_BUSINESS_CALL));
    }

    @Test
    public void test_addCallExtraForCallComposerWhenLatitudeInvalid() {
        final double longitude = 2.0;
        final String invalidNumber = "a";

        mSuppInfo.addService_str(SuppInfo.TYPE_CALL_COMPOSER_LOCATION_LAT, invalidNumber);
        mSuppInfo.addService_str(SuppInfo.TYPE_CALL_COMPOSER_LOCATION_LONG,
                String.valueOf(longitude));
        ImsSuppInfoUtils.addCallExtraForCallComposer(mSuppInfo, mCallProfile);
        assertEquals(null, mCallProfile.getCallExtraParcelable(ImsCallProfile.EXTRA_LOCATION));
    }

    @Test
    public void test_addCallExtraForCallComposerWhenLongitudeInvalid() {
        final double latitude = 1.0;
        final String invalidNumber = "a";

        mSuppInfo.addService_str(SuppInfo.TYPE_CALL_COMPOSER_LOCATION_LAT,
                String.valueOf(latitude));
        mSuppInfo.addService_str(SuppInfo.TYPE_CALL_COMPOSER_LOCATION_LONG, invalidNumber);
        ImsSuppInfoUtils.addCallExtraForCallComposer(mSuppInfo, mCallProfile);
        assertEquals(null, mCallProfile.getCallExtraParcelable(ImsCallProfile.EXTRA_LOCATION));
    }

    @Test
    public void test_addCallExtraForCallComposer() {
        final int priority = 1;
        final String subject = "subject";
        final String picture = "picture";
        final double latitude = 1.0;
        final double longitude = 2.0;
        final boolean business = true;

        mSuppInfo.addService_int(SuppInfo.TYPE_CALL_COMPOSER_PRIORITY, priority);
        mSuppInfo.addService_str(SuppInfo.TYPE_CALL_COMPOSER_SUBJECT, subject);
        mSuppInfo.addService_str(SuppInfo.TYPE_CALL_COMPOSER_PICTURE_URL, picture);
        mSuppInfo.addService_str(SuppInfo.TYPE_CALL_COMPOSER_LOCATION_LAT,
                String.valueOf(latitude));
        mSuppInfo.addService_str(SuppInfo.TYPE_CALL_COMPOSER_LOCATION_LONG,
                String.valueOf(longitude));
        mSuppInfo.addService_bool(SuppInfo.TYPE_CALL_COMPOSER_IS_BUSINESS, business);

        ImsSuppInfoUtils.addCallExtraForCallComposer(mSuppInfo, mCallProfile);

        assertEquals(priority, mCallProfile.getCallExtraInt(ImsCallProfile.EXTRA_PRIORITY));
        assertEquals(subject, mCallProfile.getCallExtra(ImsCallProfile.EXTRA_CALL_SUBJECT));
        assertEquals(picture, mCallProfile.getCallExtra(ImsCallProfile.EXTRA_PICTURE_URL));
        Location location = mCallProfile.getCallExtraParcelable(ImsCallProfile.EXTRA_LOCATION);
        assertEquals(latitude, location.getLatitude(), 0.00001);
        assertEquals(longitude, location.getLongitude(), 0.00001);
        assertEquals(business,
                mCallProfile.getCallExtraBoolean(ImsCallProfile.EXTRA_IS_BUSINESS_CALL));
    }

    @Test
    public void test_addSuppInfoForCallComposerWhenEmpty() {
        ImsSuppInfoUtils.addSuppInfoForCallComposer(mCallProfile, mSuppInfo);

        assertNull(mSuppInfo.getService(SuppInfo.TYPE_CALL_COMPOSER_PRIORITY));
        assertNull(mSuppInfo.getService(SuppInfo.TYPE_CALL_COMPOSER_SUBJECT));
        assertNull(mSuppInfo.getService(SuppInfo.TYPE_CALL_COMPOSER_PICTURE_URL));
        assertNull(mSuppInfo.getService(SuppInfo.TYPE_CALL_COMPOSER_LOCATION_LAT));
        assertNull(mSuppInfo.getService(SuppInfo.TYPE_CALL_COMPOSER_LOCATION_LONG));
        assertNull(mSuppInfo.getService(SuppInfo.TYPE_CALL_COMPOSER_IS_BUSINESS));
    }

    @Test
    public void test_addSuppInfoForCallComposer() {
        final int priority = 1;
        final String subject = "subject";
        final String picture = "picture";
        final Location location = new Location("");
        location.setLatitude(1.0);
        location.setLongitude(2.0);
        final boolean business = true;
        mCallProfile.setCallExtraInt(ImsCallProfile.EXTRA_PRIORITY, priority);
        mCallProfile.setCallExtra(ImsCallProfile.EXTRA_CALL_SUBJECT, subject);
        mCallProfile.setCallExtra(ImsCallProfile.EXTRA_PICTURE_URL, picture);
        mCallProfile.setCallExtraParcelable(ImsCallProfile.EXTRA_LOCATION, location);
        mCallProfile.setCallExtraBoolean(ImsCallProfile.EXTRA_IS_BUSINESS_CALL, business);

        ImsSuppInfoUtils.addSuppInfoForCallComposer(mCallProfile, mSuppInfo);

        SuppInfo.SuppService ss = mSuppInfo.getService(SuppInfo.TYPE_CALL_COMPOSER_PRIORITY);
        assertNotNull(ss);
        assertEquals(priority, ss.intValue);
        ss = mSuppInfo.getService(SuppInfo.TYPE_CALL_COMPOSER_SUBJECT);
        assertNotNull(ss);
        assertEquals(subject, ss.strValue);
        ss = mSuppInfo.getService(SuppInfo.TYPE_CALL_COMPOSER_PICTURE_URL);
        assertNotNull(ss);
        assertEquals(picture, ss.strValue);
        ss = mSuppInfo.getService(SuppInfo.TYPE_CALL_COMPOSER_LOCATION_LAT);
        assertNotNull(ss);
        assertEquals(String.valueOf(location.getLatitude()), ss.strValue);
        ss = mSuppInfo.getService(SuppInfo.TYPE_CALL_COMPOSER_LOCATION_LONG);
        assertNotNull(ss);
        assertEquals(String.valueOf(location.getLongitude()), ss.strValue);

        // UE cannot make a business call
        assertNull(mSuppInfo.getService(SuppInfo.TYPE_CALL_COMPOSER_IS_BUSINESS));
    }
}
