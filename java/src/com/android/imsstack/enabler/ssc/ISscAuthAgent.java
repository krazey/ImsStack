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

package com.android.imsstack.enabler.ssc;

public interface ISscAuthAgent {
    boolean calculateResponse(String method, String uri, String body);
    String getCredentialInfoString();

    void setGbaKeys(String username, String password);
    String getCipherSuite();
    void setCipherSuite(String cipherSuite);
    String getETag();
    void setETag(String tag);

    /**
     * Returns Network Application Function(NAF) fully qualified domain name (FQDN) from
     * {@link CarrierConfig.ImsSs#KEY_UT_NAF_FQDN_STRING} or from realm in the network response.
     *
     * @return A URI string of NAF FQDN from {@link CarrierConfig.ImsSs#KEY_UT_NAF_FQDN_STRING}. If
     * it's empty, NAF FQDN is extracted from realm of the network response.
     */
    String getNafFqdn();

    void parse(String wwwAuthenticate);

    boolean isCredentialInfoUpdated();
    void setIsCredentialInfoUpdated(boolean updated);

    String getRealm();
}
