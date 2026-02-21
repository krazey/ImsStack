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
package com.android.imsstack.enabler.uce.impl.utils;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import android.util.ArraySet;

import androidx.test.filters.SmallTest;

import com.android.imsstack.enabler.uce.impl.define.UceFeatureTags;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.MockitoAnnotations;

import java.util.Set;

@RunWith(JUnit4.class)
public class UceUtilsTest {

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
    }

    @After
    public void cleanUp() {
    }

    @Test
    @SmallTest
    public void test_getCapabilities() throws Exception {
        Set<String> capabilities = new ArraySet<>();
        long result = 0L;
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_PRESENCE.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_PRESENCE.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_IPCALL_VOICE.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_IPCALL_VOICE.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_IPCALL_VIDEO.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_IPCALL_VIDEO.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_PAGER_MESSAGING.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_PAGER_MESSAGING.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_LARGE_MESSAGING.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_LARGE_MESSAGING.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_CPM_CHAT.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_CPM_CHAT.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_SIMPLE_IM.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_SIMPLE_IM.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_STORE_AND_FORWARD_GROUP_CHAT.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_STORE_AND_FORWARD_GROUP_CHAT.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_FILE_TRANSFER_THUMBNAIL.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_FILE_TRANSFER_THUMBNAIL.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_FT_STORE_AND_FORWARD.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_FT_STORE_AND_FORWARD.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_FT_HTTP.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_FT_HTTP.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_GEOLOCATION_PUSH.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_GEOLOCATION_PUSH.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_FILE_TRANSFER.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_FILE_TRANSFER.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_SHARED_MAP.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_SHARED_MAP.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_SHARED_SKETCH.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_SHARED_SKETCH.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_CALL_COMPOSER.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_CALL_COMPOSER.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_CALL_COMPOSER_MMTEL.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_CALL_COMPOSER_MMTEL.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_CALL_POST_CALL.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_CALL_POST_CALL.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_FT_SMS.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_FT_SMS.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_GEOLOCATION_SMS.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_GEOLOCATION_SMS.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_SESSION.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_SESSION.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_SA.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_SA.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_IS_BOT.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_IS_BOT.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_VERSION_V1.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_VERSION_V1.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);

        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_VERSION_V2.getTag());
        result |= UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_VERSION_V2.getCapa();
        assertEquals(UceUtils.getCapabilities(capabilities), result);
    }

    @Test
    @SmallTest
    public void test_getFeatureTags() throws Exception {
        long result = 0L;
        Set<String> capabilities = UceUtils.getFeatureTags(result);
        assertTrue(capabilities.isEmpty());

        result |= UceFeatureTags.Tags.FEATURE_TAG_PRESENCE.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(capabilities.contains(UceFeatureTags.Tags.FEATURE_TAG_PRESENCE.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_IPCALL_VOICE.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(capabilities.contains(UceFeatureTags.Tags.FEATURE_TAG_IPCALL_VOICE.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_IPCALL_VIDEO.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(capabilities.contains(UceFeatureTags.Tags.FEATURE_TAG_IPCALL_VIDEO.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_PAGER_MESSAGING.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(
                capabilities.contains(UceFeatureTags.Tags.FEATURE_TAG_PAGER_MESSAGING.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_LARGE_MESSAGING.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(
                capabilities.contains(UceFeatureTags.Tags.FEATURE_TAG_LARGE_MESSAGING.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_CPM_CHAT.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(capabilities.contains(UceFeatureTags.Tags.FEATURE_TAG_CPM_CHAT.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_SIMPLE_IM.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(capabilities.contains(UceFeatureTags.Tags.FEATURE_TAG_SIMPLE_IM.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_STORE_AND_FORWARD_GROUP_CHAT.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(capabilities.contains(
                UceFeatureTags.Tags.FEATURE_TAG_STORE_AND_FORWARD_GROUP_CHAT.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_FILE_TRANSFER_THUMBNAIL.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(capabilities.contains(
                UceFeatureTags.Tags.FEATURE_TAG_FILE_TRANSFER_THUMBNAIL.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_FT_STORE_AND_FORWARD.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(capabilities.contains(
                UceFeatureTags.Tags.FEATURE_TAG_FT_STORE_AND_FORWARD.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_FT_HTTP.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(capabilities.contains(UceFeatureTags.Tags.FEATURE_TAG_FT_HTTP.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_GEOLOCATION_PUSH.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(
                capabilities.contains(UceFeatureTags.Tags.FEATURE_TAG_GEOLOCATION_PUSH.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_FILE_TRANSFER.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(capabilities.contains(UceFeatureTags.Tags.FEATURE_TAG_FILE_TRANSFER.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_SHARED_MAP.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(capabilities.contains(UceFeatureTags.Tags.FEATURE_TAG_SHARED_MAP.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_SHARED_SKETCH.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(capabilities.contains(UceFeatureTags.Tags.FEATURE_TAG_SHARED_SKETCH.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_CALL_COMPOSER.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(capabilities.contains(UceFeatureTags.Tags.FEATURE_TAG_CALL_COMPOSER.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_CALL_COMPOSER_MMTEL.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(capabilities.contains(
                UceFeatureTags.Tags.FEATURE_TAG_CALL_COMPOSER_MMTEL.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_CALL_POST_CALL.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(capabilities.contains(UceFeatureTags.Tags.FEATURE_TAG_CALL_POST_CALL.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_FT_SMS.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(capabilities.contains(UceFeatureTags.Tags.FEATURE_TAG_FT_SMS.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_GEOLOCATION_SMS.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(capabilities.contains(UceFeatureTags.Tags.FEATURE_TAG_GEOLOCATION_SMS.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_SESSION.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(capabilities.contains(UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_SESSION.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_SA.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(capabilities.contains(UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_SA.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_IS_BOT.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(capabilities.contains(UceFeatureTags.Tags.FEATURE_TAG_IS_BOT.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_VERSION_V1.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(
                capabilities.contains(UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_VERSION_V1.getTag()));

        result |= UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_VERSION_V2.getCapa();
        capabilities = UceUtils.getFeatureTags(result);
        assertTrue(
                capabilities.contains(UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_VERSION_V2.getTag()));
    }
}
