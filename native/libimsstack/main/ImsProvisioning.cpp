/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    201010    hwangoo.park@             Created
    </table>

    Description
*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ImsConstDef.h"
#include "private/ConfigurationManager.h"
#include "ImsProvisioning.h"

__IMS_TRACE_TAG_CONF__;

PUBLIC GLOBAL IMS_BOOL ImsProvisioning::Initialize()
{
    //---------------------------------------------------------------------------------------------

    if (!ConfigurationManager::GetInstance()->Initialize(
                IMS_SOLUTION_IMS_CONFIG_DB, ConfigurationManager::MODE_DB))
    {
        IMS_TRACE_E(0, "Initializing a Configuration Manager failed", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_I(">>> IMS configuration db already exists <<<", 0, 0, 0);

    return IMS_TRUE;
}
