/*
    Author
    <table>
    date        author                  description
    --------    --------------          ----------
    20151209    hwangoo.park@           Created
    </table>

    Description
*/

package com.android.imsstack.imsservice.mmtel;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.agentif.IWifiState;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.internal.enabler.ImsStateStore;

/**
 * IMS registration related utility methods.
 */
public class ImsRegUtils {
    public static boolean isImsRegisteredOnWifi(IBaseContext context) {
        IWifiState ws = (IWifiState)AgentFactory.getAgent(AgentFactory.WIFI_STATE);
        boolean isWifiConnected = (ws != null) ? ws.isWifiConnected() : false;
        boolean isNetworkTypeWifi =
                ImsStateStore.getRegState(context.getPhoneId()).isNetworkTypeWifi();

        return isWifiConnected && isNetworkTypeWifi;
    }
}
