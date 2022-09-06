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

import android.text.TextUtils;

import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.SystemUtils;

import java.util.Date;
import java.util.HashMap;
import java.util.Locale;

public class SscAuthAgent implements ISscAuthAgent {
    public static HashMap<Integer, ISscAuthAgent> sSscAuthAgent = new HashMap<>();

    private int mSlotId = 0;
    private String mCipherSuite = "";
    private String mETag = "";

    private boolean mIsCredentialInfoUpdated = false;
    private SSCAuthCredentials mSSCAuthCredentials;

    private SscAuthAgent(int slotId) {
        ImsLog.d(slotId, "");
        mSlotId = slotId;
        mSSCAuthCredentials = new SSCAuthCredentials();
    }

    public static ISscAuthAgent getInstance(int slotId) {
        if (!sSscAuthAgent.containsKey(slotId)) {
            sSscAuthAgent.put(slotId, new SscAuthAgent(slotId));
        }
        return sSscAuthAgent.get(slotId);
    }

    @Override
    public boolean calculateResponse(String method, String uri, String body) {
        ImsLog.d("");

        mSSCAuthCredentials.setUri(uri);
        return mSSCAuthCredentials.calculateResponse(method, body);
    }

    @Override
    public String getCredentialInfoString() {
        return mSSCAuthCredentials.getAuthInfoString();
    }

    @Override
    public String getRealm() {
        return mSSCAuthCredentials.getRealm();
    }

    @Override
    public void setGbaKeys(String username, String password) {
        mSSCAuthCredentials.setUsername(username);
        mSSCAuthCredentials.setPassword(password);
    }

    @Override
    public String getCipherSuite() {
        return mCipherSuite;
    }

    @Override
    public void setCipherSuite(String cipherSuite) {
        mCipherSuite = cipherSuite;
    }

    @Override
    public String getETag() {
        return mETag;
    }

    @Override
    public void setETag(String tag) {
        mETag = tag;
    }

    @Override
    public String getNafFqdnFromRealm() {
        String realm = mSSCAuthCredentials.getRealm();
        if (TextUtils.isEmpty(realm) == true) {
            ImsLog.d("realm is invalid");
            return null;
        }

        String[] tokens = realm.split("@");
        String nafFqdn = tokens.length > 1 ? tokens[1] : tokens[0];

        ImsLog.d("nafFqdn : "  + nafFqdn);
        return nafFqdn;
    }

    @Override
    public void parse(String wwwAuthenticate) {
        ImsLog.d("");

        if (TextUtils.isEmpty(wwwAuthenticate)) {
            ImsLog.d("wwwAuthenticate is invalid : [" + wwwAuthenticate + "]");
            return;
        }
        ImsLog.d("wwwAuthenticate : " + wwwAuthenticate);

        // skip "Digest "
        int index = wwwAuthenticate.indexOf(' ');
        String tmp = wwwAuthenticate;

        if (index != -1) {
            tmp = tmp.substring(index);
        }

        tmp = tmp.replaceAll("\\p{Space}", "");

        String[] tokens = tmp.split(",");

        for (int i = 0; i < tokens.length; ++i) {
            String[] nameValue = tokens[i].split("=");
            for (int j = 0; j < nameValue.length ; j++) {
                ImsLog.d("nameValue[" + j + "] : " + nameValue[j]);
            }

            if ("realm".equalsIgnoreCase(nameValue[0])) {
                String temp_realm = nameValue[1].replaceAll("\"", "");
                String realm_send[] = temp_realm.split(";");
                mSSCAuthCredentials.setRealm(realm_send[0]);
                ImsLog.d("nameValue :" + nameValue[0] + "/" + nameValue[1]
                        + ", getRealm() : " + mSSCAuthCredentials.getRealm());
            } else if ("nonce".equalsIgnoreCase(nameValue[0])) {
                String temp_nonce = tokens[i].replaceAll("(?i)nonce=","");
                mSSCAuthCredentials.setNonce(temp_nonce.replaceAll("\"",""));
                ImsLog.d("nameValue :" + nameValue[0] + "/" + nameValue[1]
                        + ", getNonce() : " + mSSCAuthCredentials.getNonce());
            } else if ("nextnonce".equalsIgnoreCase(nameValue[0])) {
                // nonce can include '=' character...
                nameValue[1] = tokens[i].replaceAll(nameValue[0] + "=", "");
                mSSCAuthCredentials.setNonce(nameValue[1].replaceAll("\"", ""));
                ImsLog.d("nameValue :" + nameValue[0] + "/" + nameValue[1]
                        + ", getNonce() : " + mSSCAuthCredentials.getNonce());
            } else if ("qop".equalsIgnoreCase(nameValue[0])) {
                mSSCAuthCredentials.setQop(nameValue[1].replaceAll("\"", ""));
                mSSCAuthCredentials.setQopExist(true);
                ImsLog.d("nameValue :" + nameValue[0] + "/" + nameValue[1]
                        + ", getQos() : " + mSSCAuthCredentials.getQop()
                        + ", isQopExist : " + mSSCAuthCredentials.isQopExist());
            } else if ("opaque".equalsIgnoreCase(nameValue[0])) {
                nameValue[1] = tokens[i].replaceAll("opaque=", "");
                mSSCAuthCredentials.setOpaque(nameValue[1].replaceAll("\"", ""));
                ImsLog.d("nameValue :" + nameValue[0] + "/" + nameValue[1]
                        + ", getOpaque() : " + mSSCAuthCredentials.getOpaque());
            } else if ("algorithm".equalsIgnoreCase(nameValue[0])) {
                mSSCAuthCredentials.setAlgorithm(nameValue[1].replaceAll("\"", ""));
                ImsLog.d("nameValue :" + nameValue[0] + "/" + nameValue[1]
                        + ", getAlgorithm() : " + mSSCAuthCredentials.getAlgorithm());
            }
        }

        // Set flag to show credential information is stored
        setIsCredentialInfoUpdated(true);
    }

    @Override
    public void setIsCredentialInfoUpdated(boolean updated) {
        ImsLog.d(mSlotId, "setIsCredentialInfoUpdated() " + updated);
        mIsCredentialInfoUpdated = updated;

        if (!mIsCredentialInfoUpdated) {
            mCipherSuite = "";
            mSSCAuthCredentials.clear();
        }
    }

    @Override
    public boolean isCredentialInfoUpdated() {
        return mIsCredentialInfoUpdated;
    }

    private class SSCAuthCredentials {
        private String mUsername = null;
        private String mPassword = null;
        private String mRealm = null;
        private String mNonce = null;
        private String mQop = null;
        private String mUri = null;
        private String mResponse = null;
        private String mCnonce = null;
        private String mOpaque = null;
        private String mAlgorithm = null;
        private int mNc = 0;

        private boolean mQopExist = false;

        private void clear() {
            mUsername = null;
            mPassword = null;
            mRealm = null;
            mNonce = null;
            mQop = null;
            mUri = null;
            mResponse = null;
            mCnonce = null;
            mOpaque = null;
            mAlgorithm = null;
            mNc = 0;
            mQopExist = false;
        }

        private boolean calculateResponse(String method, String body) {
            if (TextUtils.isEmpty(mUsername)) {
                return false;
            }

            mCnonce = generateCnonce();
            if (TextUtils.isEmpty(mCnonce)) {
                return false;
            }

            increaseNc();
            String ha1 = calculateHA1(mUsername, mRealm, mPassword, mAlgorithm, mNonce, mCnonce);
            mResponse = calculateResponse(ha1, mNonce, String.format(Locale.US, "%08d", mNc),
                    mCnonce, mQop, method, mUri, body);

            return true;
        }

        private String calculateHA1(String username, String realm, String passwd,
                String algorithm, String nonce, String cnonce) {
            ImsLog.d("\nusername : [" + username + "]"
                    + "\nrealm : [" + realm + "]"
                    + "\npasswd : [" + passwd + "]"
                    + "\nalgorithm : [" + algorithm + "]"
                    + "\nnonce : [" + nonce + "]"
                    + "\ncnonce : [" + cnonce + "]");

            // Remove surrounding quotation marks
            String unqUsername = username.replace("\"", "");
            String unqRealm = realm.replace("\"", "");
            String varA1 = unqUsername + ":" + unqRealm + ":" + passwd;

            if ("MD5-sees".equals(algorithm)) {
                varA1 = SystemUtils.calculateMessageDigest("MD5", varA1);
                String unqNonce = nonce.replace("\"", "");
                String unqCnonce = cnonce.replace("\"", "");
                varA1 = varA1 + ":" + unqNonce + ":" + unqCnonce;
            }

            String varHA1 = SystemUtils.calculateMessageDigest("MD5", varA1);
            ImsLog.d("\nA1 : " + varA1 + "\nHA1 : " + varHA1);

            return varHA1;
        }

        private String generateCnonce() {
            String date = "" + new Date().getTime();
            String cnonce = SystemUtils.calculateMessageDigest("MD5", date);

            return cnonce;
        }

        private String calculateResponse(String varHA1, String nonce, String nc,
                String cnonce, String qop, String method, String digestUri, String entity) {
            ImsLog.d("HA1 : [" + varHA1 + "]" + ", nonce : [" + nonce + "]" + ", nc : [" + nc + "]"
                    + ", cnonce : [" + cnonce + "]" + ", qop : [" + qop + "]"
                    + ", method : [" + method + "]" + ", digestUri : [" + digestUri + "]"
                    + ", entity: [" + entity + "]");

            String varA2 = method + ":" + digestUri;

            if ("auth-int".equals(qop)) {
                if (entity == null) {
                    ImsLog.w("entity is null ");
                    entity = "";
                }
                varA2 = varA2 + ":" + SystemUtils.calculateMessageDigest("MD5", entity);
            }

            String varHA2 = SystemUtils.calculateMessageDigest("MD5", varA2);
            ImsLog.d("A2 : " + varA2 + ", HA2 : " + varHA2);

            String tmpRsp = null;
            if ("auth".equalsIgnoreCase(qop) || "auth-int".equalsIgnoreCase(qop)) {
                tmpRsp = varHA1 + ":" + nonce + ":"+ nc + ":" + cnonce + ":"
                        + qop.toLowerCase(Locale.ROOT) + ":" + varHA2;
            } else {
                tmpRsp = varHA1 + ":" + nonce + ":" + varHA2;
            }

            String requestDigest = SystemUtils.calculateMessageDigest("MD5", tmpRsp);
            ImsLog.d("temp-response : " + tmpRsp + ", requestDigest : " + requestDigest);

            return requestDigest;
        }

        public String getAuthInfoString() {
            String authorization = "Digest ";
            authorization += "username=\"" + mUsername + "\"";

            if (mRealm != null) {
                authorization += ", realm=\"" + mRealm + "\"";
            }

            if (mNonce != null) {
                authorization += ", nonce=\"" + mNonce + "\"";
            }

            if (mUri != null) {
                authorization += ", uri=\"" + mUri + "\"";
            }

            if (mQop != null) {
                authorization += ", qop=" + mQop;
            }

            if (mNc > 0) {
                authorization += ", nc=" + String.format(Locale.US, "%08d", mNc);
            }

            if (mCnonce != null) {
                authorization += ", cnonce=\"" + mCnonce + "\"";
            }

            if (mResponse != null) {
                authorization += ", response=\"" + mResponse + "\"";
            }

            if (mOpaque != null) {
                authorization += ", opaque=\"" + mOpaque + "\"";
            }

            if (mAlgorithm != null) {
                authorization += ", algorithm=" + mAlgorithm;
            }

            return authorization;
        }

        private String getNonce() {
            return mNonce;
        }

        private void setNonce(String nonce) {
            mNonce = nonce;
        }

        private String getRealm() {
            return mRealm;
        }

        private void setRealm(String realm) {
            mRealm = realm;
        }

        private String getQop() {
            return mQop;
        }

        private void setQop(String qop) {
            mQop = qop;
        }

        private void setQopExist(boolean exist) {
            mQopExist = exist;
        }

        private boolean isQopExist() {
            return mQopExist;
        }

        private String getOpaque() {
            return mOpaque;
        }

        private void setOpaque(String opaque) {
            mOpaque = opaque;
        }

        private String getAlgorithm() {
            return mAlgorithm;
        }

        private void setAlgorithm(String algorithm) {
            mAlgorithm = algorithm;
        }

        private void setUsername(String username) {
            mUsername = username;
        }

        private void setPassword(String password) {
            mPassword = password;
        }

        private void setUri(String uri) {
            mUri = uri;
        }

        private void increaseNc() {
            ImsLog.d("");

            if (mNc == 0x7FFFFFFF) {
                mNc = 1;
            }

            mNc++;
        }
    }
}