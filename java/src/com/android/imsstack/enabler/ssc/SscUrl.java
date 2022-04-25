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

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.SubsInfoInterface;
import com.android.imsstack.core.agents.agentif.IISIM;
import com.android.imsstack.core.agents.agentif.ITelephonySubscriber;
import com.android.imsstack.enabler.ssc.data.SscServiceData;
import com.android.imsstack.enabler.ssc.data.SscServiceQueryData;
import com.android.imsstack.util.ImsLog;

import java.net.MalformedURLException;
import java.net.URL;
import java.util.Locale;

public class SscUrl {
    private final String URI_HTTP = "http://";
    private final String URI_HTTPS = "https://";

    private final String URI_AUID = "/simservs.ngn.etsi.org";
    private final String URI_USERS = "/users";
    private final String URI_DOC = "/simservs.xml";
    private final String URI_NODE_SELECTOR_SEPARATOR = "/~~";

    public static SscUrl sSscUrl = null;

    public static SscUrl getInstance() {
        if (sSscUrl == null) {
            sSscUrl = new SscUrl();
        }

        return sSscUrl;
    }

    public URL getConnectionUrl(int slotId, String query) {
        String urlAddr = generateUrlAddress(slotId);
        if (TextUtils.isEmpty(urlAddr)) {
            ImsLog.e(slotId, "urlAddr is null");
            return null;
        }

        URL url = null;
        try {
            url = new URL(urlAddr + query);
        } catch (MalformedURLException e) {
            ImsLog.e(e.toString());
            e.printStackTrace();
            return null;
        }

        return url;
    }

/*
    private String getQueryCFRuleGCF(int slotId, ESsType sstype,
            String xui, int serviceClass, int condition, int cfState)
    {
        ImsLog.d("");

        String strURL = generateUrlAddress(slotId, false);

        if (strURL == null) {
            return null;
        }

        StringBuffer sb = new StringBuffer("/" + strURL.replace("http://", "")
                        + "/simservs.ngn.etsi.org/users/" + xui
                        + "/simservs.xml/~~/"
                        + SscXmlFormat.getURLSS(slotId) + "simservs/"
                        + SscXmlFormat.getURLSS(slotId) + sstype.getSSName());
        // Add name space information
        boolean bNotUsedSS = "".equals(SscXmlFormat.getURLSS(slotId));
        boolean bNotUsedCP = "".equals(SscXmlFormat.getURLCP(slotId));

        if (bNotUsedSS == false || bNotUsedCP == false) {
            sb.append("?");
            if (bNotUsedCP == false) {
                sb.append("xmlns(cp=urn:ietf:params:xml:ns:common-policy)");
            }
            if (bNotUsedSS == false) {
                sb.append("xmlns(ss=http://uri.etsi.org/ngn/params/xml/simservs/xcap)");
            }
        }

        return sb.toString();
    }
*/
    public String getDocumentQueryUri(SscServiceQueryData data, String xui) {
        int slotId = data.getSlotId();
        String urlQuery = getQueryPrefixFromDB(slotId);
        urlQuery += URI_AUID + URI_USERS + "/" + xui + URI_DOC;

        ImsLog.d("urlQuery : " + urlQuery);
        return urlQuery;
    }

    public String getQueryUri(SscServiceQueryData data, String xui) {
        int slotId = data.getSlotId();
        String querUri = getQueryPrefixFromDB(slotId);
        querUri += URI_AUID + URI_USERS + "/" + xui + URI_DOC;

        if (data.getSsType() != ESsType.NONE) {
            querUri += URI_NODE_SELECTOR_SEPARATOR
                    + "/" + SscXmlFormat.getSsElement(slotId, SscXmlFormat.SIMSERVS)
                    + "/" + SscXmlFormat.getSsElement(slotId, data.getSsType().getSSName());
        }

        if (querUri.contains(SscXmlFormat.NS_SS_PREFIX)
                || querUri.contains(SscXmlFormat.NS_CP_PREFIX)) {
            querUri += "?";
            if (querUri.contains(SscXmlFormat.NS_SS_PREFIX)) {
                querUri += SscXmlFormat.NS_SS_URI;
            }
            if (querUri.contains(SscXmlFormat.NS_CP_PREFIX)) {
                querUri += SscXmlFormat.NS_CP_URI;
            }
        }

        ImsLog.d("querUri : " + querUri);
        return querUri;
    }

    public String getUpdateUri(SscServiceData data, String xui) {
        int slotId = data.getSlotId();
        String updateUri = getQueryPrefixFromDB(slotId);
        updateUri += URI_AUID + URI_USERS + "/" + xui + URI_DOC + URI_NODE_SELECTOR_SEPARATOR;

        switch (data.getSsType()) {
            case CF:
                if (data.getCondition() == SscConstant.CONDITION_CFNR_TIMER) {
                    updateUri += getCfnrTimerUri(data);
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
            case ICBA:
                updateUri += getRuleUri(data);
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

        if (updateUri.contains(SscXmlFormat.NS_SS_PREFIX)
                || updateUri.contains(SscXmlFormat.NS_CP_PREFIX)) {
            updateUri += "?";
            if (updateUri.contains(SscXmlFormat.NS_SS_PREFIX)) {
                updateUri += SscXmlFormat.NS_SS_URI;
            }
            if (updateUri.contains(SscXmlFormat.NS_CP_PREFIX)) {
                updateUri += SscXmlFormat.NS_CP_URI;
            }
        }

        ImsLog.d("updateUri : " + updateUri);
        return updateUri;
    }

    private String getServiceUri(SscServiceData data) {
        return "/" + SscXmlFormat.getSsElement(data.getSlotId(), SscXmlFormat.SIMSERVS)
                + "/" + SscXmlFormat.getSsElement(data.getSlotId(), data.getSsType().getSSName());
    }

    private String getRuleUri(SscServiceData data) {
        int mediaType = (data.getServiceClass() == SscServiceClassUtil.SERVICE_CLASS_DATA) ?
                SscXmlFormat.MEDIA_VIDEO : SscXmlFormat.MEDIA_AUDIO;
        String ruleId = SscXmlFormat.getRuleId(
                data.getSlotId(), mediaType, data.getSsType().getSSName(), data.getCondition());

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
        uri += "/" + SscXmlFormat.getCpElement(data.getSlotId(), SscXmlFormat.NOREPLYTIMER);

        return uri;
    }

    private String makeDNSQuery(int slotId, String fqdn) {
        // TODO: SRV query will be refactored
        /*
        ImsLog.d("");
        String urlAddr = null;
        SscDnsQuery.IQueryObject queryObject = SscDnsQuery.getInstance().makeDnsQuery(slotId, fqdn);
        if (queryObject == null) {
            return null;
        }

        String urlPort = Integer.toString(queryObject.getPort());
        String ipAddress = queryObject.getIP();
        if (SscConfig.MMTel.isTLS(slotId)) {
            sHttpString = "https://";
        } else {
            sHttpString = "http://";
        }

        if (ipAddress != null) {
            // for IPv6
            if (ipAddress.contains(":") && !ipAddress.contains("[") && !ipAddress.contains("]")) {
                ipAddress = "[" + ipAddress + "]";
            }
        }

        if (!TextUtils.isEmpty(urlPort)) {
            urlAddr = sHttpString + ipAddress + ":" + urlPort;
        } else {
            urlAddr = sHttpString + ipAddress;
        }

        ImsLog.d("urlAddr : " + urlAddr);
        return urlAddr;
         */
        return null;
    }

    private String generateUrlAddress(int slotId) {
        SubsInfoInterface subsInfo = AgentFactory.getInstance().getAgent(
                SubsInfoInterface.class, slotId);

        if (subsInfo == null) {
            return null;
        }

        String uriAddr = SscConfig.getUtServerFqdn(slotId);
        if (TextUtils.isEmpty(uriAddr)) {
            uriAddr = "xcap.";

            IISIM isimAgent = (IISIM) AgentFactory.getAgent(AgentFactory.ISIM, slotId);
            String impi = isimAgent != null ? isimAgent.getImpi() : null;

            if (subsInfo.isIsimEnabled() && !TextUtils.isEmpty(impi)) {
                String substringImpi = impi.substring(impi.lastIndexOf("@") + 1, impi.length());
                ImsLog.d("substring of IMPI : " + substringImpi);

                if (substringImpi.contains("3gppnetwork.org")) {
                    uriAddr += substringImpi.replace("3gppnetwork.org", "pub.3gppnetwork.org");
                } else {
                    uriAddr += substringImpi;
                }
            } else {
                int mnc = -1;
                int mcc = -1;
                try {
                    ITelephonySubscriber ts = (ITelephonySubscriber) AgentFactory.getAgent(
                            AgentFactory.TELEPHONY_SUBSCRIBER, slotId);
                    if (ts != null) {
                        mnc = Integer.parseInt(ts.getMnc(true));
                        mcc = Integer.parseInt(ts.getMcc(true));
                    }

                    if (mnc != -1 && mcc != -1) {
                        uriAddr += String.format(Locale.US,
                                "ims.mnc%03d.mcc%03d.pub.3gppnetwork.org", mnc, mcc);
                    } else {
                        ImsLog.e("Invalid mnc & mcc value");
                        return null;
                    }
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                    ImsLog.e(e.toString());
                    return null;
                }
            }
        }

        if (SscConfig.isSrvRecordsRequired(slotId)) {
            // TODO: will be used after implementation NAPTR/SRV query
            // uriAddr = queryNaptrSrv(slotId, uriAddr);
        } else {
            int uriPort = SscConfig.getUtServerPort(slotId);
            if (uriPort <= 0) {
                uriPort = SscConfig.isTls(slotId) ? 443 : 80;
            }

            String uriHttp = SscConfig.isTls(slotId) ? URI_HTTPS : URI_HTTP;
            uriAddr = uriHttp + uriAddr + ":" + Integer.toString(uriPort);
        }

        ImsLog.d("uriAddr : " + uriAddr);

        return uriAddr;
    }

    private static String getQueryPrefixFromDB(int slotId) {
        String urlQueryPrefix = SscConfig.getAuidPrefix(slotId);
        return (TextUtils.isEmpty(urlQueryPrefix)) ? "" :
                (urlQueryPrefix.startsWith("/") ? urlQueryPrefix : "/" + urlQueryPrefix);
    }
}
