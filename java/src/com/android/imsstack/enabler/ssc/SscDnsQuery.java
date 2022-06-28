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

import com.android.imsstack.core.agents.dcmif.DcConstants;
import com.android.imsstack.util.ImsLog;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.HashSet;
import java.util.Set;

/**
 * SscConfig
 */
public final class SscDnsQuery {

    private static int sNafPort = 0;
    private static int sBsfPort = 0;
    private static String sBsfIPAddress = null;
    private static String sNafIPAddress = null;
    /* ImsStack-Build_DNS
    private Set<SRVRecord> mSrvRecordNAFList = null;
    private Set<SRVRecord> mSrvRecordBSFList = null;
    */
    private Set<Integer> mNAFIndexList = null;
    private Set<Integer> mBSFIndexList = null;
    private static SscDnsQuery sQueryInstance = null;
    private boolean mBSFFailed = false;
    private boolean mNAFFailed = false;

    private SscDnsQuery() {
        ImsLog.d("");
        /* ImsStack-Build_DNS
        mSrvRecordNAFList = new HashSet<SRVRecord>();
        mSrvRecordBSFList = new HashSet<SRVRecord>();
        */
        mNAFIndexList = new HashSet<Integer>();
        mBSFIndexList = new HashSet<Integer>();
    }

    public static SscDnsQuery getInstance() {
        if (sQueryInstance == null) {
            sQueryInstance = new SscDnsQuery();
        }
        return sQueryInstance;
    }

    public int getNAFRecords() {
        /* ImsStack-Build_DNS
        return this.mSrvRecordNAFList.size();
        */
        return 0;
    }

    public int getBSFRecords() {
        /* ImsStack-Build_DNS
        return this.mSrvRecordBSFList.size();
        */
        return 0;
    }

    public IQueryObject makeDnsQuery(int slotId, String fqdn, boolean isBsfAddress) {

        ImsLog.d("makeDnsQuery(" + slotId + "/" + fqdn + "/" + isBsfAddress + ")");

        if (fqdn == null) {
            throw new NullPointerException("Check FQDN");
        }

        // When first Application Gateway fails, try for the next Gateway
        if ((isBsfAddress && mBSFFailed) || (!isBsfAddress && mNAFFailed)) {
            ImsLog.d ("First AFG query Failed & hence trying for second AFG");
            IQueryObject queryObject = getNextQueryObject(slotId, isBsfAddress);
            if (queryObject != null) {
                ImsLog.d ("retrieved next available SRV record");
            } else {
                SscServiceStateAgent.getInstance().setAllSrvAddrTried(slotId, true);
                ImsLog.d ("First AFG query Failed & query object null");
            }
            return queryObject;
        }
        else {
            if (isBsfAddress == false) {
                if (sNafPort != 0 && sNafIPAddress != null) {
                    ImsLog.d("makeDnsQuery and return the cached NAF  address & port");
                    return new QueryObjectImpl(sNafPort, sNafIPAddress);
                }
            } else {
                if (sBsfPort != 0 && sBsfIPAddress != null) {
                    ImsLog.d("makeDnsQuery and return the cached BSF  address & port");
                    return new QueryObjectImpl(sBsfPort, sBsfIPAddress);
                }
            }
        }

        String pdntype = SscConfig.getUtPdnType(slotId);
        if (!TextUtils.isEmpty(pdntype) && pdntype.equals("mobile_xcap")) {
            resolveUtXcapFqdn(slotId, fqdn, DcConstants.TYPE_XCAP, isBsfAddress);
        }
        else {
            resolveUtXcapFqdn(slotId, fqdn, DcConstants.TYPE_INTERNET, isBsfAddress);
        }

        if (isBsfAddress == false) {
            mNAFIndexList.add(getSRVIndexForNAF());
            if (TextUtils.isEmpty(sNafIPAddress)) {
                sNafIPAddress = fqdn;
            }
            return new QueryObjectImpl(sNafPort, sNafIPAddress);
        }
        else {
            mBSFIndexList.add(getSRVIndexForBSF());
            if (TextUtils.isEmpty(sBsfIPAddress)) {
                sBsfIPAddress = fqdn;
            }
            return new QueryObjectImpl(sBsfPort, sBsfIPAddress);
        }
    }

    private QueryObjectImpl getNextQueryObject(int slotId, boolean isBsfAddress) {

        ImsLog.d("getNextQueryObject:: isBsfAddress = " + isBsfAddress);
        int index = 0;
        QueryObjectImpl queryObject = null;

        /* ImsStack-Build_DNS
        if (isBsfAddress == true) {
            for (SRVRecord record : mSrvRecordBSFList) {
                ImsLog.d("Inside loop for BSF  && index  = " + index);
                if (mBSFIndexList.contains(index) == false) {
                    ImsLog.d("BSF info :: getNextQueryObject... Port = " + record.getPort()
                            + " IP address = " + record.getTarget().toString());
                    sBsfIPAddress = record.getTarget().toString();
                    sBsfIPAddress = getAddressFormat(slotId, sBsfIPAddress);
                    sBsfPort = record.getPort();
                    queryObject = new QueryObjectImpl(record.getPort() , sBsfIPAddress);
                    mBSFIndexList.add(index);
                    break;
                }
                index++;
            }
        }
        else {
            for (SRVRecord record : mSrvRecordNAFList) {
                ImsLog.d("Inside loop for NAF && index  = " + index);
                if (mNAFIndexList.contains(index) == false) {
                    ImsLog.d("NAF info :: getNextQueryObject... Port = " + record.getPort()
                            + " IP address = " + record.getTarget().toString());
                    sNafIPAddress = record.getTarget().toString();
                    sNafIPAddress = getAddressFormat(slotId, sNafIPAddress);
                    sNafPort = record.getPort();
                    queryObject = new QueryObjectImpl(record.getPort() , sNafIPAddress);
                    mNAFIndexList.add(index);
                    break;
                }
                index++;
            }
        }*/
        return queryObject;
    }

    private int getSRVIndexForNAF() {
        int index = 0;
        /* ImsStack-Build_DNS
        for (SRVRecord record : mSrvRecordNAFList) {
            if (record.getPort() == sNafPort
                && record.getTarget().toString().equals(sNafIPAddress)) {
                break;
            }
            index++;
        } */
        ImsLog.d("getSRVIndexForNAF and index is " + index);
        return index;
    }

    private int getSRVIndexForBSF() {
        int index = 0;
        /* ImsStack-Build_DNS
        for (SRVRecord record : mSrvRecordBSFList) {
            if (record.getPort() == sBsfPort
                && record.getTarget().toString().equals(sBsfIPAddress)) {
                break;
            }
            index++;
        }*/
        ImsLog.d("getSRVIndexForBSF and index is " + index);
        return index;
    }

    private void resolveUtXcapFqdn(int slotId, String fqdn,
            int interfaceType, boolean isBsfAddress)
    {
        ImsLog.d("NAPTR interface to be used: " + interfaceType);
        /* ImsStack-Build_DNS
        Record[] naptrRecords = performLookup(fqdn, Type.NAPTR, interfaceType);

        if ((naptrRecords != null) && (naptrRecords.length > 0)) {
            ImsLog.d("NAPTR records found: " + naptrRecords.length);

            for (int i = 0; i < naptrRecords.length; i++) {
                NAPTRRecord naptr = (NAPTRRecord)naptrRecords[i];
                if (naptr == null) {
                    ImsLog.e("NAPTR record: null");
                    continue;
                }

                ImsLog.d("NAPTR record: [" + i + "]" + naptr.toString());
                String napService = naptr.getService();
                ImsLog.d("NAPTR record: [" + i + "] s service " + napService);

                if ((napService.contains("HTTPS") && SscConfig.MMTel.isTLS(slotId))
                        || (!napService.contains("HTTPS") && !SscConfig.MMTel.isTLS(slotId))) {
                    // DNS SRV lookup
                    Record[] srvRecords = performLookup(
                            naptr.getReplacement().toString(), Type.SRV, interfaceType);
                    if ((srvRecords != null) && (srvRecords.length > 0)) {
                        SRVRecord srvRecord = getBestDnsSRV(srvRecords, isBsfAddress);
                        if (srvRecord != null) {
                            if (isBsfAddress == false) {
                                sNafIPAddress = srvRecord.getTarget().toString();
                                ImsLog.d("sNafIPAddress=" + sNafIPAddress);
                                sNafIPAddress = getAddressFormat(slotId, sNafIPAddress);
                                sNafPort = srvRecord.getPort();
                                ImsLog.d("NAPTR record [" + i + "] 's best SRV 's Ip: "
                                        + sNafIPAddress + "<port>" + sNafPort);
                            }
                            else {
                                sBsfIPAddress = srvRecord.getTarget().toString();
                                ImsLog.d("sBsfIPAddress=" + sBsfIPAddress);
                                sBsfIPAddress = getAddressFormat(slotId, sBsfIPAddress);
                                sBsfPort = srvRecord.getPort();
                                ImsLog.d("NAPTR record [" + i + "] 's best SRV 's bsfIPAddress: "
                                         + sBsfIPAddress + "<bsfPort>" + sBsfPort);
                            }
                        }
                        else {
                            getPortFromDB(slotId, isBsfAddress, fqdn);
                        }
                    }
                    else {
                        getPortFromDB(slotId, isBsfAddress, fqdn);
                    }
                }
                //ImsLog.d("NAPTR record: [" +i + "] s protocol "  + protocol);
                //if (!napService.contains("HTTPS")) {
                //    break;
                //}
            }
        }
        else {
            getPortFromDB(slotId, isBsfAddress, fqdn);
        }*/

        ImsLog.d("XCAP configuration: " + sNafIPAddress + ":" + sNafPort + ";");
    }

    private void getPortFromDB(int slotId, boolean isBsfAddress, String fqdn) {

        if (isBsfAddress == false) {
            ImsLog.d("SRV NAPTR record Req Failed: Use fqdn");
            //Basic DNS resolution impliclity happens as part of Android HTTPURLConnection
            sNafIPAddress = fqdn; //getDnsA(fqdn);
            ImsLog.d("NAPTR record  direct dns of fdqn " + sNafIPAddress);
            int port = SscConfig.getUtServerPort(slotId);
            if (port > 0) {
                sNafPort = port;
            }
        }
        else {
                //Don't Update  so that GBA would use mcc mnc based urls in GBAService module
        }
    }
    /* ImsStack-Build_DNS
    private Record[] performLookup(String domain, int type, int interfaceType) {

        if (domain == null) {
            throw new NullPointerException("Domain Name");
        }

        ImsLog.d("DNS " + type + " lookup for " + domain);
        ImsLog.d("DNS " + "on interface : " + interfaceType);
        try {
            Lookup.sNetInterface = interfaceType;
            Lookup lookup = new Lookup(domain, type);
            lookup.setCache(null); // If the results of this lookup should not
                                   // be permanently cached, null has to be
                                   // provided

            Record[] recordList = lookup.run();
            int code = lookup.getResult();
            if (code != Lookup.SUCCESSFUL) {
                ImsLog.d("Type " + type + " Lookup error: " + code + "/"
                        + lookup.getErrorString());
            }
            return recordList;
        } catch (TextParseException e) {
            // TODO Auto-generated catch block
            ImsLog.d("Not a valid DNS name");
            return null;
        }
    }*/

    private String getDnsA(String domain) {

        try {
            ImsLog.d("DNS A lookup for " + domain);
            return InetAddress.getByName(domain).getHostAddress();
        } catch (UnknownHostException e) {
            ImsLog.d("Unknown host for " + domain);
            return null;
        }
    }

    /* ImsStack-Build_DNS
    private SRVRecord getBestDnsSRV(Record[] records, boolean isBsfAddress) {
        SRVRecord result = null;

        ImsLog.d("getBestDnsSRV and list size :: mSrvRecordBSFList size =  "
                + (mSrvRecordBSFList.size())
                + "mSrvRecordNAFList size = " + (mSrvRecordNAFList.size()));
        for (int i = 0; i < records.length; i++) {
            SRVRecord srv = (SRVRecord)records[i];
            ImsLog.d("SRV record: " + srv.toString());

            if (isBsfAddress == true) {
                mSrvRecordBSFList.add(srv);
            } else {
                mSrvRecordNAFList.add(srv);
            }

            if (result == null) {
                // First record
                result = srv;
            } else {
                // Next record
                if (srv.getPriority() < result.getPriority()) {
                    // Lowest priority
                    result = srv;
                } else if (srv.getPriority() == result.getPriority()) {
                    // Highest weight
                    if (srv.getWeight() > result.getWeight()) {
                        result = srv;
                    }
                }
            }
        }
        ImsLog.d("Exiting getBestDnsSRV and list size :: mSrvRecordBSFList size =  "
                + (mSrvRecordBSFList.size())
                + "mSrvRecordNAFList size = " + (mSrvRecordNAFList.size()));
        return result;
    }*/

    public void cleanup() {
        sNafPort = 0;
        sNafIPAddress = null;
        sBsfPort = 0;
        sBsfIPAddress = null;
        /* ImsStack-Build_DNS
        mSrvRecordNAFList.clear();
        mSrvRecordBSFList.clear();
        */
        mNAFIndexList.clear();
        mBSFIndexList.clear();
    }

    public interface IQueryObject {
        int getPort();
        String getIP();
    }

    private class QueryObjectImpl implements IQueryObject {
        private int port = 0;
        private String ipAddress = null;
        QueryObjectImpl(int port, String ipAddress) {
            this.port = port;
            this.ipAddress = ipAddress;
        }

        @Override
        public int getPort() {
            // TODO Auto-generated method stub
            return port;
        }

        @Override
        public String getIP() {
            // TODO Auto-generated method stub
            return ipAddress;
        }
    }

    public void setNAFFailed (boolean isNAFFailed) {
        this.mNAFFailed = isNAFFailed;
    }

    public void setBSFFailed (boolean isBSFFailed) {
       this.mBSFFailed = isBSFFailed;
    }
}
