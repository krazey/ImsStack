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
package com.android.imsstack.core.config;

import com.android.imsstack.core.config.FeatureConfig;

import java.util.ArrayList;
import java.util.List;

/**
 * This class provides the feature information to convert
 * between IMS configuration XML and system configuration.
 */
public final class FeatureTable {
    public static class Feature {
        // Feature string value for IMS configuration
        private String mFeature;
        // Feature mask value for FeatureConfig
        private int mFeatureMask;

        /* package */ Feature(String feature, int featureMask) {
            mFeature = feature;
            mFeatureMask = featureMask;
        }

        public String getFeature() {
            return mFeature;
        }

        public int getFeatureMask() {
            return mFeatureMask;
        }
    }

    private static List<Feature> sServiceFeatureList;

    public static List<Feature> getServiceFeatures() {
        return sServiceFeatureList;
    }

    private FeatureTable() {}

    static {
        // Service feature list
        sServiceFeatureList = new ArrayList<Feature>();

        sServiceFeatureList.add(new Feature(
                FeatureConfig.VOLTE, FeatureConfig.FEATURE_S_VOLTE));
        sServiceFeatureList.add(new Feature(
                FeatureConfig.VOWIFI, FeatureConfig.FEATURE_S_VOWIFI));
        sServiceFeatureList.add(new Feature(
                FeatureConfig.VT, FeatureConfig.FEATURE_S_VT));
        sServiceFeatureList.add(new Feature(
                FeatureConfig.SMS, FeatureConfig.FEATURE_S_SMS));
        sServiceFeatureList.add(new Feature(
                FeatureConfig.UCE, FeatureConfig.FEATURE_S_UCE));
    }
}
