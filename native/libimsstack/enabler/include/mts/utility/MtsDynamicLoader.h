#ifndef MTS_DYNAMIC_LOADER_
#define MTS_DYNAMIC_LOADER_

#include "MtsServiceState.h"
#include "utility/MtsSipFormUtils.h"
#include "utility/MtsStrName.h"

class MtsDynamicLoader final
{
public:
    MtsDynamicLoader(IN IMS_SINT32 nSlotId);
    ~MtsDynamicLoader();

    void Initialize(IN IMS_SINT32 nSlotId);

    MtsServiceState* GetMtsServiceState();
    MtsSipFormUtils* GetMtsSipFormUtils();
    MtsStrName* GetMtsStrName();

private:
    void DestroyAll();

protected:
    MtsServiceState* m_pMtsServiceState;
    MtsSipFormUtils* m_pMtsSipFormUtils;
    MtsStrName* m_pMtsStrName;
};

#endif
