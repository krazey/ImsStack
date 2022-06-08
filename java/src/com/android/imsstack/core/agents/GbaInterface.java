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

package com.android.imsstack.core.agents;

import android.util.Pair;

import com.android.imsstack.core.agents.agentif.IAgent;

/**
 * Provides the APIs for getting key pair of bootstrapped security association.
 */
public interface GbaInterface extends IAgent {

    /**
     * This method triggers bootstrapping procedure to get B-TID and Ks_(ext)_NAF.
     *
     * @param appType icc application type
     * @param gbaMode Gba mechanism that depends on NAF Key to be returned
     * @param isTls true=TLS protocol is used between UE and NAF, otherwise false
     * @param nafFqdn A URI to specify Network Application Function(NAF) fully qualified domain
     * name (FQDN)
     * @param securityProtocol Security protocol identifier between UE and NAF
     * @param forceBootStrapping true=trigger bootstrapping, false=do not force bootstrapping
     * @return B-TID and Ks_(ext)_NAF key pair
     */
    Pair<String, String> getGbaKey(int appType, int gbaMode, boolean isTls, String nafFqdn,
            String securityProtocol, boolean forceBootStrapping);
}
