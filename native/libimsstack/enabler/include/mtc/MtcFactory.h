#ifndef MTC_FACTORY_H_
#define MTC_FACTORY_H_

#include "IMSMap.h"
#include "IMSTypeDef.h"
#include "IMutex.h"

class IMtcApp;

class MtcFactory
{
public:
    MtcFactory();
    virtual ~MtcFactory();

    static MtcFactory* GetInstance();

    void Start(IN IMS_SINT32 nSlotId);
    void Stop(IN IMS_SINT32 nSlotId);

private:
    IMutex* m_piLock;
    IMSMap<IMS_UINT32, IMtcApp*> m_objMtcApps;
};
#endif  // MTC_FACTORY_H_