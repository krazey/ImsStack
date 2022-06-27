package com.android.imsstack.core.agents.fqdn.impl;

import android.text.TextUtils;

import com.android.imsstack.core.agents.dcmif.DcConstants;
import com.android.imsstack.core.agents.fqdn.Address;
import com.android.imsstack.core.agents.fqdn.FQDNListener;
import com.android.imsstack.util.ImsLog;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class FQDNThread extends Thread {

    private int mSlotId = 0;
    private int mApnType = DcConstants.TYPE_NONE;
    private FQDNListener objFQDNListener;
    private String strAddrType;
    private String strAddress;
    private int nReson;

    private String[] mStrDomainPort = null;
    private String  mStrDomain = "";
    private String mStrPort = "5070";
    private int mResultDnsA = FQDNListener.RESULT_FQDN_FAIL;

    private final int MAX_DNS_RETRY_SRV = 5;
    private final String FQDN_TYPE_TCP = "TCP";
    private final String FQDN_TYPE_UDP = "UDP";
    private final String FQDN_TYPE_TLS = "TLS";

    private final String ADDRESS_TYPE_FQDN = "FQDN";

    public static final int NAPTR_TYPE_UDP = 0;
    public static final int NAPTR_TYPE_TCP = 1;
    public static final int NAPTR_TYPE_TLS = 2;

    public static final int QUERY_FAILED = 0;
    public static final int QUERY_SUCCESS = 1;
    public static final int QUERY_RETRY = 2;

    private static final int MAX_NAPTR_RETRY_SRV = 5;

    public FQDNThread(int slotId, int apnType) {
        mSlotId = slotId;
        mApnType = apnType;
    }

    private class Result {
        public int m_nPriority = 0;
        public int m_nWeight = 0;
        public int m_nPort = 0;
        public String m_strTarget;
    };

    private class Result_NAPTR {
        // ImsStack-Build_DNS
        //public Name m_FQDN = null;
        public int nOrder = 10;
        public int nPreference = 100;
        public String strFlags = "S";
        public String strServices = null;
        public String strRegexp = null;
        // ImsStack-Build_DNS
        //public Name strReplacement = null;
        public String strNaptrResult = null;
    }

    enum NAPTR_ENTYPE {
        SUCCESS, FAIL, RETRY,
    };

    private String strNAPTR_UDP = null;
    private String strNAPTR_TCP = null;
    private String strNAPTR_TLS = null;
    private String strNAPTRType = null;
    private List<Result> m_aResult = new ArrayList<Result>();
    private List<Result_NAPTR> m_Result_NAPTR = new ArrayList<Result_NAPTR>();

    List<Address> objAddressList = new ArrayList<Address>();

    // ---------------------------------------------------------------------------------------
    private final static Comparator<Result> M_COMPARATOR = new Comparator<Result>() {

        // @Override
        public int compare(Result object1, Result object2) {
            if (object1.m_nPriority > object2.m_nPriority) {
                return 1;
            }
            else if (object1.m_nPriority < object2.m_nPriority) {
                return -1;
            }
            else {
                return object1.m_nWeight - object2.m_nWeight;
            }
        }
    };

    // ---------------------------------------------------------------------------------------
    private final static Comparator<Result_NAPTR> M_COMPARATOR_NAPTR =
            new Comparator<Result_NAPTR>() {

        // @Override
        public int compare(Result_NAPTR object1, Result_NAPTR object2) {
            if (object1.nPreference > object2.nPreference) {
                return 1;
            }
            else if (object1.nPreference < object2.nPreference) {
                return -1;
            }
            else if (object1.nOrder > object2.nOrder) {
                return 1;
            }
            else if (object1.nOrder < object2.nOrder) {
                return -1;
            }

            return 0;
        }
    };

    public void setListener(FQDNListener objListener) {
        objFQDNListener = objListener;
    }

    public void query(String strType, String strAddr, int nReasonCode) {
        // TODO Auto-generated method stub
        strAddrType = strType;
        strAddress = strAddr;
        nReson = nReasonCode;

        if (true == isAlive()) {
            ImsLog.i("thread is alived , so thread restart");
            interrupt();
        }

        try {
            start();
        }
        catch (Exception e) {
            ImsLog.e("" + e);
        }
    }

    public void queryDnsA(String strAddr) {
        ImsLog.i("DNSA Query for SHB Operator Address" + strAddr);

        mStrDomainPort = strAddr.split(":");
        if ((mStrDomainPort != null) && (mStrDomainPort.length > 1)) {
            ImsLog.i("IP and Port mStrDomainPort Length" + mStrDomainPort.length);
            mStrDomain = mStrDomainPort[0];
            mStrPort = mStrDomainPort[1];
        }
        else {
            mResultDnsA = FQDNListener.RESULT_FQDN_FAIL;
            return;
        }

        DnsARunnableThread dnsAThread = new DnsARunnableThread();
        Thread dnsAT = new Thread(dnsAThread);
        dnsAT.start();
    }

    public void stopQuery() {
        ImsLog.d("");

        try {
            if (true == isAlive()) {
                ImsLog.d("thread is running, interrupt");
                interrupt();
            }
        }
        catch (Exception e) {
            ImsLog.d(""+e);
        }
    }

    class DnsARunnableThread implements Runnable {
        private String mStrDnsIP = null;
        private final String mStrDnsDomain = mStrDomain;
        private final String mStrDnsPort = mStrPort;

        public DnsARunnableThread() {

        }

        public void run() {
            try {
                ImsLog.i("DNS A lookup for " + mStrDnsDomain);
                mStrDnsIP = InetAddress.getByName(mStrDnsDomain).getHostAddress();
            }
            catch (UnknownHostException e) {
                ImsLog.w("Unknown host for " + mStrDnsDomain);
                mResultDnsA = FQDNListener.RESULT_FQDN_FAIL;
            }

            Address objAddress;

            if (mStrDnsIP != null && mStrDnsPort != null) {
                // UDP
                objAddress = new Address(FQDN_TYPE_UDP, mStrDnsIP, mStrDnsPort, 1, 100, 0, 1);
                objAddressList.add(objAddress);

                // TCP
                objAddress = new Address(FQDN_TYPE_TCP, mStrDnsIP, mStrDnsPort, 1, 100, 0, 1);
                objAddressList.add(objAddress);

                // TLS
                objAddress = new Address(FQDN_TYPE_TLS, mStrDnsIP, mStrDnsPort, 1, 100, 0, 1);
                objAddressList.add(objAddress);

                mResultDnsA = FQDNListener.RESULT_FQDN_SUCCESS;
            }
            else {
                ImsLog.w("IP and Port is null");
                objAddressList.clear();
                mResultDnsA = FQDNListener.RESULT_FQDN_FAIL;
            }

            if (objFQDNListener != null) {
                ImsLog.i("FQDN Listener is not null");
                objFQDNListener.onResolved(mResultDnsA, objAddressList);
            }
            else {
                ImsLog.w("Listener is null");
            }

        }

    }

    @Override
    public void run() {
        if (interrupted()) {
            return;
        }

        int nResult = FQDNListener.RESULT_FQDN_FAIL;

        if (false == refineData()) {
            objAddressList.clear();
            ImsLog.w("FQDN resolution failed");
        }
        else {
            ImsLog.i("FQDN resolution succeeded");
            nResult = FQDNListener.RESULT_FQDN_SUCCESS;
        }

        if (nReson != 0) {
            nResult = nReson;
        }
        if (objFQDNListener != null) {
            objFQDNListener.onResolved(nResult, objAddressList);
        }
        else {
            ImsLog.w("Listener is null");
        }
    }

    // -----------------------------------------------------------------------------
    private boolean refineData() {

        ImsLog.i(strAddrType + "/" + strAddress);
        if (true == TextUtils.isEmpty(strAddress) || true == TextUtils.isEmpty(strAddrType)) {
            ImsLog.w("Address or Type is null! ");
            return false;
        }

        if (false == strAddrType.equalsIgnoreCase(ADDRESS_TYPE_FQDN)) {
            ImsLog.w("invalid Address Type!");
            return false;
        }

        String strHost = "";
        String strUDPDefaultPort = "5060";
        String strTCPDefaultPort = "5060";
        String strTLSDefaultPort = "5060";

        String[] mStrDomainPort = strAddress.split(":");

        if (null == mStrDomainPort) {
            return false;
        }

        strHost = mStrDomainPort[0];
        if (mStrDomainPort.length > 1) {
            strUDPDefaultPort = mStrDomainPort[1];
            strTCPDefaultPort = mStrDomainPort[1];
            strTLSDefaultPort = mStrDomainPort[1];
        }

        boolean bNAPTRResult = false;

        boolean bSkipSrvUdp = false;
        boolean bSkipSrvTcp = false;
        boolean bSkipSrvTls = false;

        try {
            bNAPTRResult = getNAPTRecord(strHost);

            if (false == bNAPTRResult) {
                ImsLog.w("NAPTR query failed!");
                setDefaultNAPTRecord(strHost);
            }
            else {
                bSkipSrvUdp = isSkipSRV(FQDN_TYPE_UDP, strNAPTR_UDP, strUDPDefaultPort);
                bSkipSrvTcp = isSkipSRV(FQDN_TYPE_TCP, strNAPTR_TCP, strTCPDefaultPort);
                bSkipSrvTls = isSkipSRV(FQDN_TYPE_TLS, strNAPTR_TLS, strTLSDefaultPort);
            }

            if (false == getSRVRecord(bSkipSrvUdp, bSkipSrvTcp, bSkipSrvTls)) {
                ImsLog.w("DNS SRV query failed!");
                setFQDNResult(FQDN_TYPE_TLS, strHost, strTLSDefaultPort);
                setFQDNResult(FQDN_TYPE_TCP, strHost, strTCPDefaultPort);
                setFQDNResult(FQDN_TYPE_UDP, strHost, strUDPDefaultPort);
            }
        } catch (Exception e) {
            ImsLog.e("fqdn failed exception : " + e);
        }

        return true;
    }

    private boolean isSkipSRV(String strType, String strAddr, String strDefaultPort) {
        boolean bResult = false;

        if (true == TextUtils.isEmpty(strAddr)) {
            bResult = true;
        } else if (true == IPValiditor.isIpAddress(strAddr)) {
            setFQDNResult(strType, strAddr, strDefaultPort);
            bResult = true;
        }

        ImsLog.i("type=" + strType + "addr=" + strAddr + "result=" + bResult);
        return bResult;
    }
    private void setFQDNResult(String strType, String strHost, String strDefaultPort) {
        if (true == TextUtils.isEmpty(strType) ||
                true == TextUtils.isEmpty(strHost)) {
            ImsLog.w("strType or strHost is null");
            return;
        }
        String[] mStrDomainPort = strHost.split(":");
        String strAddr = mStrDomainPort[0];

        Address objAddress = new Address(strType, strAddr, strDefaultPort, 1, 100, 0, 1);
        objAddressList.add(objAddress);
    }

    private void setDefaultNAPTRecord(String strAddr) {
        strNAPTR_UDP = strAddr;
        strNAPTR_TCP = strAddr;
        strNAPTR_TLS = strAddr;
    }

    private boolean getNAPTRecord(String strHost) {
        ImsLog.d("");

        boolean bRet = false;
        int nRetryNaptr = 0;

        while ((NAPTR_ENTYPE.RETRY == queryNAPT(strHost))
                && (nRetryNaptr < MAX_NAPTR_RETRY_SRV)) {
            nRetryNaptr++;
            ImsLog.w("Fail to NAPTR query : " + nRetryNaptr);
        }

        if (nRetryNaptr < MAX_NAPTR_RETRY_SRV) {
            bRet = true;
        }
        return bRet;
    }

    private void setNAPTRType(int naptrType) {
        switch (naptrType) { // 0:udp, 1:tcp, 2:tls

            case 0:
                strNAPTRType = strNAPTR_UDP;
                break;
            case 1:
                strNAPTRType = strNAPTR_TCP;
                break;
            case 2:
                strNAPTRType = strNAPTR_TLS;
                break;
            default:
                break;
        }

        ImsLog.i("naptrType : " + naptrType + "NAPTR : " + strNAPTRType);
    }

    private NAPTR_ENTYPE queryNAPT(String strAddress) {
        ImsLog.d("");

        if (strAddress == null || strAddress.isEmpty() == true) {
            ImsLog.w("strAddress is not available");
            return NAPTR_ENTYPE.FAIL;
        }

        /* ImsStack-Build_DNS
        Record[] aAnswers = runLookUp(strAddress, Type.NAPTR);

        if (null == aAnswers || aAnswers.length == 0) {
            ImsLog.w("aAnswers is null or length is 0");
            return NAPTR_ENTYPE.RETRY;
        }

        StringBuffer sbNaptrResult = new StringBuffer();
        sbNaptrResult.append("<NAPTR>" + strAddress +
                "   Total result: " + aAnswers.length + "\n");
        try {
            Result_NAPTR objNAPTR = null;
            for (int i = 0; i < aAnswers.length; i++) {
                objNAPTR = new Result_NAPTR();

                objNAPTR.m_FQDN = new Name(strAddress);
                objNAPTR.nOrder = ((NAPTRRecord)aAnswers[i]).getOrder();
                objNAPTR.nPreference = ((NAPTRRecord)aAnswers[i]).getPreference();
                objNAPTR.strFlags = ((NAPTRRecord)aAnswers[i]).getFlags();
                objNAPTR.strServices = ((NAPTRRecord)aAnswers[i]).getService();
                objNAPTR.strRegexp = ((NAPTRRecord)aAnswers[i]).getRegexp();
                objNAPTR.strReplacement = ((NAPTRRecord)aAnswers[i]).getReplacement();

                if (objNAPTR.strRegexp == null || objNAPTR.strRegexp.isEmpty()) {
                    objNAPTR.strNaptrResult = objNAPTR.strReplacement.toString();
                }
                else {
                    if (objNAPTR.m_FQDN == null || objNAPTR.m_FQDN.toString() == null) {
                        ImsLog.w("NAPTR result value is not available");
                        return NAPTR_ENTYPE.FAIL;
                    }
                    objNAPTR.strNaptrResult = objNAPTR.m_FQDN.toString().replace(
                            objNAPTR.strRegexp, objNAPTR.strReplacement.toString());
                }

                m_Result_NAPTR.add(objNAPTR);
                ImsLog.i("runNAPTR::" + objNAPTR.strNaptrResult);

                if (objNAPTR.strServices.equalsIgnoreCase("SIP+D2U")) {
                    strNAPTR_UDP = objNAPTR.strNaptrResult;
                }
                else if (objNAPTR.strServices.equalsIgnoreCase("SIPS+D2T")) {
                    strNAPTR_TLS = objNAPTR.strNaptrResult;
                }
                else if (objNAPTR.strServices.equalsIgnoreCase("SIP+D2T")) {
                    strNAPTR_TCP = objNAPTR.strNaptrResult;
                }
                else {
                    sbNaptrResult.append("Can't read Service name from NAPTR");
                }
            }
        }
        catch (Exception e) {
            ImsLog.e("NAPTR text parse exception");
            return NAPTR_ENTYPE.RETRY;
        }*/

        Collections.sort(m_Result_NAPTR, M_COMPARATOR_NAPTR);
        Collections.reverse(m_Result_NAPTR);

        return NAPTR_ENTYPE.SUCCESS;
    }
/* ImsStack-Build_DNS
    private Record[] runLookUp(String strAddress, int nType) {
        Lookup objLookup = null;

        try {
            Lookup.sNetInterface = mApnType;
            objLookup = new Lookup(strAddress, Type.NAPTR);
        } catch (TextParseException e) {
            ImsLog.e("Exception - NAPTR Lookup");
            return null; // have to NAPTR retry
        }

        Record[] aAnswers = objLookup.run();

        if (Lookup.SUCCESSFUL != objLookup.getResult()) {
            ImsLog.w("NAPTR Result is not SUCCESS");
            return null;
        }

        return aAnswers;
    }
*/
    private boolean getSRVRecord(boolean bSkipSrvUdp, boolean bSkipSrvTcp, boolean bSkipSrvTls) {
        ImsLog.d("");

        boolean bResult_UDP = false;
        boolean bResult_TCP = false;
        boolean bResult_TLS = false;

        if (false == bSkipSrvUdp) {
            bResult_UDP = addSRVRecordToList(NAPTR_TYPE_UDP);
        }
        else {
            bResult_UDP = true;
        }

        if (false == bSkipSrvTcp) {
            bResult_TCP = addSRVRecordToList(NAPTR_TYPE_TCP);
        }
        else {
            bResult_TCP = true;
        }

        if (false == bSkipSrvTls) {
            bResult_TLS = addSRVRecordToList(NAPTR_TYPE_TLS);
        }
        else {
            bResult_TLS = true;
        }

        if (false == bResult_UDP && false == bResult_TCP && false == bResult_TLS) {
            ImsLog.w("fail to get SRV record UDP, TCP, TLS");
            return false;
        }

        return true;
    }

    private int querySRV() {
        ImsLog.d("");
/* ImsStack-Build_DNS
        Lookup objLookup = null;

        if (null == strNAPTRType) {
            ImsLog.w("NAPTR Result is null");
            return QUERY_FAILED;
        }

        try {
            Lookup.sNetInterface = mApnType;
            objLookup = new Lookup(strNAPTRType, Type.SRV);
        }
        catch (TextParseException e) {
            e.printStackTrace();
            return QUERY_RETRY;
        }

        // Get server information
        Record[] aAnswers = objLookup.run();

        if (Lookup.SUCCESSFUL != objLookup.getResult()) {
            ImsLog.w("Lookup.SUCCESSFUL != objLookup.getResult()");
            return QUERY_RETRY;
        }

        if (null == aAnswers) {
            ImsLog.w("aAnswers is null");
            return QUERY_RETRY;
        }

        if (0 == aAnswers.length) {
            ImsLog.w("aAnswers length is 0");
            return QUERY_RETRY;
        }

        StringBuffer sbLog = new StringBuffer();
        sbLog.append(strNAPTRType + "   Total result: " + aAnswers.length + "\n");

        try {
            Result objCur = null;
            for (int i = 0; i < aAnswers.length; i++) {
                objCur = new Result();
                objCur.m_nPriority = ((SRVRecord)aAnswers[i]).getPriority();
                objCur.m_nWeight = ((SRVRecord)aAnswers[i]).getWeight();
                objCur.m_nPort = ((SRVRecord)aAnswers[i]).getPort();
                objCur.m_strTarget = new String(((SRVRecord)aAnswers[i]).getTarget().toString());

                ImsLog.i("bs.DNSsrv::Query : " + "priority[" + objCur.m_nPriority + "] "
                    + "Weight[" + objCur.m_nWeight + "] "
                    + "port[" + objCur.m_nPort + "] "
                    + "target[" + objCur.m_strTarget + "]\n"
                    );

                m_aResult.add(objCur);
            }
        }
        catch (Exception e) {
            ImsLog.e("Exception !!");
            e.printStackTrace();
            return QUERY_RETRY;
        }

        Collections.sort(m_aResult, M_COMPARATOR);
        Collections.reverse(m_aResult);
*/
        return QUERY_SUCCESS;
    }

    private boolean addSRVRecordToList(int nDNSSRVType) {
        int nRetrySRV = 0;
        int eDNS_QUERY = QUERY_FAILED;
        setNAPTRType(nDNSSRVType);

        boolean bResult = false;

        String strType;

        if (NAPTR_TYPE_UDP == nDNSSRVType) {
            strType = FQDN_TYPE_UDP;
        }
        else if (NAPTR_TYPE_TCP == nDNSSRVType) {
            strType = FQDN_TYPE_TCP;
        }
        else {
            strType = FQDN_TYPE_TLS;
        }

        while (nRetrySRV < MAX_DNS_RETRY_SRV) {
            eDNS_QUERY = querySRV();
            if (QUERY_RETRY == eDNS_QUERY) {
                nRetrySRV++;
                ImsLog.d("Retry DNS Srv query " + nRetrySRV);
                continue;
            }
            else {
                ImsLog.d("DNSSrv query result " + eDNS_QUERY);
                break;
            }
        }

        if (nRetrySRV < MAX_DNS_RETRY_SRV && (QUERY_SUCCESS == eDNS_QUERY)) {
            ImsLog.i("Success DNS Srv query , Type" + strType);

            int nMax = getSRVResultMax();

            if (0 < nMax) {
                for (int i = 0; i < nMax; i++) {
                    Address objAddress = new Address(strType, getSRVTarget(i),
                            String.valueOf(getSRVPort(i)), getSRVPriority(i), getSRVWeight(i), i,
                            nMax);

                    objAddressList.add(objAddress);
                }

                bResult = true;
            }
        }
        else {
            ImsLog.w("Fail DNS Srv query " + nRetrySRV);
            bResult = false;
        }

        cleanResult();
        return bResult;

    }

    private int getSRVResultMax() {
        return m_aResult.size();
    }

    private String getSRVTarget(int nIndex) {
        return m_aResult.get(nIndex).m_strTarget;
    }

    private int getSRVPort(int nIndex) {
        return m_aResult.get(nIndex).m_nPort;
    }

    private int getSRVWeight(int nIndex) {
        return m_aResult.get(nIndex).m_nWeight;
    }

    private int getSRVPriority(int nIndex) {
        return m_aResult.get(nIndex).m_nPriority;
    }

    private void cleanResult() {
        m_aResult.clear();
    }

}
