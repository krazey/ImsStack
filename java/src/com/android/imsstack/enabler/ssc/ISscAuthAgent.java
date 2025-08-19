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
     * Returns {@code true} if the credentials are updated. {@code false} otherwise.
     */
    boolean isCredentialInfoUpdated();

    /**
     * Sets a flag indicating whether the credential information has been updated.
     *
     * @param updated {@code true} if the credential information has been updated,
     *                {@code false} otherwise.
     */
    void setIsCredentialInfoUpdated(boolean updated);

    /**
     * Returns Network Application Function(NAF) fully qualified domain name (FQDN) from
     * {@link CarrierConfig.ImsSs#KEY_UT_NAF_FQDN_STRING} or from realm in the network response.
     *
     * @return A URI string of NAF FQDN from {@link CarrierConfig.ImsSs#KEY_UT_NAF_FQDN_STRING}. If
     * it's empty, NAF FQDN is extracted from realm of the network response.
     */
    String getNafFqdn();

    /**
     * Returns the realm used for authentication.
     * <p>
     * The realm is extracted from the WWW-Authenticate header sent by the network.
     *
     * @return The authentication realm as a string.
     */
    String getRealm();

    /**
     * Parses the WWW-Authenticate header from a network response.
     * <p>
     * This method extracts the necessary authentication parameters (e.g. realm, nonce)
     * from the challenge string, which are required to calculate the authentication response.
     *
     * @param wwwAuthenticate The full value of the WWW-Authenticate header.
     */
    void parse(String wwwAuthenticate);
}
