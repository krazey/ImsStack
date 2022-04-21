package com.android.imsstack.enabler.ssc;

import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.SubsInfoInterface;
import com.android.imsstack.core.agents.agentif.IISIM;
import com.android.imsstack.core.agents.agentif.ITelephonySubscriber;
import com.android.imsstack.util.ImsLog;

import java.util.Locale;

public final class SscUtils {
    public static int getTelephonySimType(int slotId) {
        SubsInfoInterface subsInfo = AgentFactory.getInstance().getAgent(
                SubsInfoInterface.class, slotId);

        if (subsInfo == null) {
            return TelephonyManager.APPTYPE_ISIM;
        }

        if (subsInfo.isIsimEnabled()) {
            return TelephonyManager.APPTYPE_ISIM;
        }

        return TelephonyManager.APPTYPE_USIM;
    }

    public static String getImpu(int slotId) {
        // TODO: Get IMPU from UICC value that provided by IMS platform regardless of USIM or ISIM
        IISIM isimAgent = (IISIM) AgentFactory.getAgent(AgentFactory.ISIM, slotId);
        String[] impu = isimAgent != null ? isimAgent.getImpu() : null;
        if (impu == null || impu.length == 0) {
            ImsLog.w("IMPU is invalid !!!");
            return null;
        }
        return impu[0];
    }

    public static String getDomain(int slotId) {
        SubsInfoInterface subsInfo = AgentFactory.getInstance().getAgent(
                SubsInfoInterface.class, slotId);

        if (subsInfo == null) {
            return null;
        }

        String domain = null;
        if (subsInfo.isIsimEnabled() == true) {
            IISIM isimAgent = (IISIM) AgentFactory.getAgent(AgentFactory.ISIM, slotId);
            String impi = isimAgent != null ? isimAgent.getImpi() : null;
            if (impi == null || impi.isEmpty()) {
                ImsLog.w("IMPI is invalid !!!");
                return null;
            }
            domain = impi.substring(impi.lastIndexOf("@") + 1 , impi.length());
        } else if (subsInfo.isUsimEnabled() == true) {
            ITelephonySubscriber ts = (ITelephonySubscriber)AgentFactory.getAgent(
                    AgentFactory.TELEPHONY_SUBSCRIBER, slotId);
            if (ts == null) {
                return null;
            }

            int mnc, mcc;
            try {
                mnc = Integer.parseInt(ts.getMnc(true));
                mcc = Integer.parseInt(ts.getMcc(true));
            } catch (final NumberFormatException e) {
                e.printStackTrace();
                ImsLog.e("Invalid MNC/MCC");
                return null;
            }
            domain = String.format(Locale.US, "ims.mnc%03d.mcc%03d.3gppnetwork.org", mnc, mcc);
        } else {
            String impu = getImpu(slotId);
            if (TextUtils.isEmpty(impu)) {
                ImsLog.w("IMPU is invalid !!!");
                return null;
            }
            domain = impu.substring(impu.lastIndexOf("@") + 1 , impu.length());
        }

        return domain;
    }

    public static String getUtUserAgent(int slotId) {
        int gbaType = SscConfig.getGbaMode(slotId);
        String gbaString = null;
        if (gbaType == SscConfig.GBA_ME) {
            gbaString = "3gpp-gba";
        } else if (gbaType == SscConfig.GBA_U) {
            gbaString = "3gpp-gba-uicc";
        } else if (gbaType == SscConfig.GBA_DIGEST) {
            // TODO: Ut doesn't support digest
            //gbaString = "3gpp-gba-digest";
        }

        String userAgent = SscConfig.getImsUserAgent(slotId);
        if (!TextUtils.isEmpty(gbaString)) {
            if (!TextUtils.isEmpty(userAgent)) {
                userAgent += " " + gbaString;
            } else {
                userAgent = gbaString;
            }
        }

        return userAgent;
    }
}
