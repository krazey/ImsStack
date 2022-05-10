#ifndef MTS_FACTORY_H_
#define MTS_FACTORY_H_

#include "IMSMap.h"

class IMtsApp;
class MtsApp;

class MtsFactory final
{
public:
    MtsFactory();
    ~MtsFactory();

    static MtsFactory* GetInstance();

    void Destroy(IN IMS_SINT32 nSlotId);
    void StartMts(IN IMS_SINT32 nSlotId);
    void StopMts(IN IMS_SINT32 nSlotId);

    IMS_BOOL DestroyMtsApp(IN IMS_SINT32 nSlotId);
    MtsApp* GetMtsApp(IN IMS_SINT32 nSlotId);
    IMS_UINT32 GetMtsAppListSize();

private:
    IMtsApp* CreateMtsApp(IN IMS_SINT32 nSlotId);

private:
    IMSMap<IMS_SINT32, IMtsApp*> m_objMtsApp;
};

#endif
