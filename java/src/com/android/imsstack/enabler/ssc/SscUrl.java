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

import com.android.imsstack.enabler.ssc.data.SscServiceData;
import com.android.imsstack.enabler.ssc.data.SscServiceQueryData;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.net.MalformedURLException;
import java.net.URL;

public class SscUrl {
    private static final String URI_HTTP = "http://";
    private static final String URI_HTTPS = "https://";

    private static final String URI_AUID = "/simservs.ngn.etsi.org";
    private static final String URI_USERS = "/users";
    private static final String URI_DOC = "/simservs.xml";
    private static final String URI_NODE_SELECTOR_SEPARATOR = "/~~";

    private static final SscUrl sSscUrl = new SscUrl();

    protected static SscUrl getInstance() {
        return sSscUrl;
    }

    protected URL getConnectionUrl(int slotId, String query) {
        String urlAddr = getXcapRootUri(slotId);
        if (TextUtils.isEmpty(urlAddr)) {
            ImsLog.e(slotId, "urlAddr is null");
            return null;
        }

        URL url = null;
        try {
            url = new URL(urlAddr + query);
        } catch (MalformedURLException e) {
            ImsLog.e(e.toString());
            return null;
        }

        return url;
    }

    protected String getQueryUri(SscServiceQueryData data, String xui) {
        int slotId = data.getSlotId();
        String queryUri = getUriPrefixFromConfig(slotId);
        queryUri += URI_AUID + URI_USERS + "/" + xui + URI_DOC;

        if (data.getSsType() != ESsType.NONE) {
            queryUri += URI_NODE_SELECTOR_SEPARATOR
                    + "/" + SscXmlFormat.getSsElement(slotId, SscXmlFormat.SIMSERVS)
                    + "/" + SscXmlFormat.getSsElement(slotId, data.getSsType().getSsName());
        }

        if (SscXmlFormat.isNamespaceSsSupported(slotId)
                || SscXmlFormat.isNamespaceCpSupported(slotId)) {
            queryUri += "?";
            if (SscXmlFormat.isNamespaceSsSupported(slotId)) {
                queryUri += SscXmlFormat.NS_SS_URI;
            }
            if (SscXmlFormat.isNamespaceCpSupported(slotId)) {
                queryUri += SscXmlFormat.NS_CP_URI;
            }
        }

        ImsLog.d("queryUri : " + queryUri);
        return queryUri;
    }

    protected String getUpdateUri(SscServiceData data, String xui) {
        int slotId = data.getSlotId();
        String updateUri = getUriPrefixFromConfig(slotId);
        updateUri += URI_AUID + URI_USERS + "/" + xui + URI_DOC + URI_NODE_SELECTOR_SEPARATOR;

        switch (data.getSsType()) {
            case CF:
                if (data.getEventNumber() == SscConstant.EVENT_SSC_INSERT_CF) {
                    updateUri += getServiceUri(data);
                } else if (data.getCondition() == SscConstant.CONDITION_CFNR_TIMER) {
                    if (SscXmlFormat.getIsNoReplyTimerOmitted(slotId)) {
                        updateUri += getServiceUri(data);
                    } else {
                        updateUri += getCfnrTimerUri(data);
                    }
                } else {
                    if (data.getState() == SscConstant.ACTION_ACTIVATION ||
                            data.getState() == SscConstant.ACTION_DEACTIVATION) {
                        updateUri += getRuleConditionUri(data);

                    } else {
                        updateUri += getRuleUri(data);
                    }
                }
                break;
            case ICB:
            case OCB:
                if (data.getEventNumber() == SscConstant.EVENT_SSC_INSERT_CB) {
                    updateUri += getServiceUri(data);
                } else {
                    updateUri += getRuleUri(data);
                }
                break;
            case CW:
            case OIR:
            case OIP:
            case TIR:
            case TIP:
                updateUri += getServiceUri(data);
                break;
            default:
                break;
        }

        if (SscXmlFormat.isNamespaceSsSupported(slotId)
                || SscXmlFormat.isNamespaceCpSupported(slotId)) {
            updateUri += "?";
            if (SscXmlFormat.isNamespaceSsSupported(slotId)) {
                updateUri += SscXmlFormat.NS_SS_URI;
            }
            if (SscXmlFormat.isNamespaceCpSupported(slotId)) {
                updateUri += SscXmlFormat.NS_CP_URI;
            }
        }

        ImsLog.d("updateUri : " + updateUri);
        return updateUri;
    }

    private String getServiceUri(SscServiceData data) {
        return "/" + SscXmlFormat.getSsElement(data.getSlotId(), SscXmlFormat.SIMSERVS)
                + "/" + SscXmlFormat.getSsElement(data.getSlotId(), data.getSsType().getSsName());
    }

    private String getRuleUri(SscServiceData data) {
        int mediaType = (data.getServiceClass() == SscServiceClassUtil.SERVICE_CLASS_VIDEO)
                ? SscXmlFormat.MEDIA_TYPE_VIDEO : SscXmlFormat.MEDIA_TYPE_AUDIO;
        String ruleId = SscXmlFormat.getRuleId(
                data.getSlotId(), mediaType, data.getSsType().getSsName(), data.getCondition());

        String uri = getServiceUri(data);
        uri += "/" + SscXmlFormat.getCpElement(data.getSlotId(), SscXmlFormat.RULESET)
                + "/" + SscXmlFormat.getCpElement(data.getSlotId(), SscXmlFormat.RULE)
                + "%5B@id=%22" + ruleId + "%22%5D";

        return uri;
    }

    private String getRuleConditionUri(SscServiceData data) {
        String uri = getRuleUri(data);
        uri += "/" + SscXmlFormat.getCpElement(data.getSlotId(), SscXmlFormat.CONDITIONS);

        return uri;
    }

    private String getCfnrTimerUri(SscServiceData data) {
        String uri = getServiceUri(data);
        uri += "/" + SscXmlFormat.getSsElement(data.getSlotId(), SscXmlFormat.NOREPLYTIMER);

        return uri;
    }

    private String getXcapRootUri(int slotId) {
        String uriAddr = SscConfig.getUtServerFqdn(slotId);
        if (TextUtils.isEmpty(uriAddr)) {
            uriAddr = getSscUtils().getDomain(slotId, true);
            if (TextUtils.isEmpty(uriAddr)) {
                ImsLog.e("uriAddr is null");
                return null;
            }
        }

        int uriPort = SscConfig.getUtServerPort(slotId);
        if (uriPort <= 0) {
            uriPort = SscConfig.isTls(slotId) ? 443 : 80;
        }

        String uriHttp = SscConfig.isTls(slotId) ? URI_HTTPS : URI_HTTP;
        uriAddr = uriHttp + uriAddr + ":" + uriPort;

        ImsLog.d("uriAddr : " + uriAddr);
        return uriAddr;
    }

    private String getUriPrefixFromConfig(int slotId) {
        String uriPrefix = SscConfig.getAuidPrefix(slotId);
        return (TextUtils.isEmpty(uriPrefix)) ? "" :
                (uriPrefix.startsWith("/") ? uriPrefix : "/" + uriPrefix);
    }

    @VisibleForTesting
    protected SscUrl() {
    }

    @VisibleForTesting
    protected SscUtils getSscUtils() {
        return SscUtils.getInstance();
    }
}
