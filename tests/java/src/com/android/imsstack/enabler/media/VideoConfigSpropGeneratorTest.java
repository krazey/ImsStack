/**
 * Copyright (C) 2024 The Android Open Source Project
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

package com.android.imsstack.enabler.media;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class VideoConfigSpropGeneratorTest {

    @Mock ConfigInterface mMockConfigInterface;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        AgentFactory.getInstance().setAgent(
                ConfigInterface.class, mMockConfigInterface, TestAppContext.SLOT0);
    }

    @After
    public void tearDown() throws Exception {
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, TestAppContext.SLOT0);
    }

    @Test
    public void testInitAndCleanup() {
        VideoConfigSpropGenerator.cleanup(TestAppContext.SLOT0);

        VideoConfigSpropGenerator.init(TestAppContext.SLOT0);
        assertTrue(VideoConfigSpropGenerator.hasConfigChangeListener(TestAppContext.SLOT0));

        VideoConfigSpropGenerator.cleanup(TestAppContext.SLOT0);
        assertFalse(VideoConfigSpropGenerator.hasConfigChangeListener(TestAppContext.SLOT0));
    }
}
