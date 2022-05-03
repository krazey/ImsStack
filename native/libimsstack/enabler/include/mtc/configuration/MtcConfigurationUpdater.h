#ifndef MTC_CONFIGURATION_UPDATER_H_
#define MTC_CONFIGURATION_UPDATER_H_

#include "IMSTypeDef.h"
#include "IMSMap.h"

class ICarrierConfig;
struct CarrierConfigItems;
struct AssetItems;

class MtcConfigurationUpdater final
{
public:
    static void Update(IN ICarrierConfig* piCc, IN CarrierConfigItems& objCarrierConfigItems,
            IN AssetItems& objAssetItems);

private:
    static void UpdateByCarrierConfig(IN ICarrierConfig* piCc, IN CarrierConfigItems& objItems);
    static void UpdateByAsset(IN ICarrierConfig* piCc, IN AssetItems& objItems);
};

#endif
