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
package com.android.imsstack.enabler.aos;

import java.util.Set;
import android.net.Uri;

public class AosRegistrationListener implements IAosRegistrationListener {
    @Override
    public void notifyRegistered(int networkType, int featureTagBits,
            Set<String> featureTags) {}

    @Override
    public void notifyRegistering(int networkType, int featureTagBits,
            Set<String> featureTags) {}

    @Override
    public void notifyDeregistered(int reason) {}

    @Override
    public void notifyTechnologyChangeFailed(int networkType, int causeCode) {}

    @Override
    public void notifyAssociatedUriChanged(Uri[] uris) {}

    @Override
    public void notifyCapabilitiesUpdateFailed(int capabilities, int networkType,
            int reason) {}
}