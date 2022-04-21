#ifndef MTC_CONTEXT_REPOSITORY_H_
#define MTC_CONTEXT_REPOSITORY_H_

#include "IMSMap.h"
#include "IMSTypeDef.h"

class IMtcContext;

class MtcContextRepository
{
private:
    MtcContextRepository();
    ~MtcContextRepository();
    MtcContextRepository(IN const MtcContextRepository&);
    MtcContextRepository& operator=(IN const MtcContextRepository&);

public:
    static MtcContextRepository* GetInstance();

public:
    static IMtcContext* GetContext(IN IMS_SINT32 nSlotId = INVALID_SLOT_ID);
    IMtcContext* GetContextBySlot(IN IMS_SINT32 nSlotId);
    void AddContext(IN IMS_SINT32 nSlotId, IN IMtcContext* piContext);
    void RemoveContext(IN IMS_SINT32 nSlotId);

private:
    static MtcContextRepository* s_pThis;
    static const IMS_SINT32 INVALID_SLOT_ID = -1;
    IMSMap<IMS_SINT32, IMtcContext*> m_objContexts;
};

#endif
