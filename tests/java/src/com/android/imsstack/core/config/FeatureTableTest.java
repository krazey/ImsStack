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
import static org.junit.Assert.assertNotNull;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class FeatureTableTest {
    private FeatureTable.Feature mFeature;

    @Before
    public void setUp() throws Exception {
        mFeature = new FeatureTable.Feature(FeatureConfig.VOLTE, FeatureConfig.FEATURE_S_VOLTE);
    }

    @After
    public void tearDown() throws Exception {
        mFeature = null;
    }

    @Test
    public void getServiceFeaturesTest() {
        assertNotNull(FeatureTable.getServiceFeatures());
    }

    @Test
    public void getFeatureMaskTest() {
        assertEquals(FeatureConfig.FEATURE_S_VOLTE, mFeature.getFeatureMask());
    }

    @Test
    public void getFeature() {
        assertNotNull(mFeature.getFeature());
        assertEquals(FeatureConfig.VOLTE, mFeature.getFeature());
    }
}
