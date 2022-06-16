#include "IMSTypeDef.h"
#include "ServiceThread.h"
#include "IMtcContext.h"
#include "MtcContextRepository.h"

MtcContextRepository* MtcContextRepository::s_pThis = IMS_NULL;

PUBLIC
MtcContextRepository::MtcContextRepository() :
        m_objContexts(IMSMap<IMS_SINT32, IMtcContext*>())
{
}

PUBLIC
MtcContextRepository::~MtcContextRepository() {}

PUBLIC GLOBAL MtcContextRepository* MtcContextRepository::GetInstance()
{
    if (s_pThis == IMS_NULL)
    {
        s_pThis = new MtcContextRepository();
    }
    return s_pThis;
}

PUBLIC GLOBAL IMtcContext* MtcContextRepository::GetContext(
        IN IMS_SINT32 nSlotId /* = INVALID_SLOT_ID*/)
{
    if (nSlotId == INVALID_SLOT_ID)
    {
        nSlotId = ThreadService::GetCurrentSlotId();
    }

    return MtcContextRepository::GetInstance()->GetContextBySlot(nSlotId);
}

PUBLIC
IMtcContext* MtcContextRepository::GetContextBySlot(IN IMS_SINT32 nSlotId)
{
    IMS_SLONG nIndex = m_objContexts.GetIndexOfKey(nSlotId);
    if (nIndex < 0)
    {
        return IMS_NULL;
    }
    return m_objContexts.GetValueAt(nIndex);
}

PUBLIC
void MtcContextRepository::AddContext(IN IMS_SINT32 nSlotId, IN IMtcContext* piContext)
{
    m_objContexts.Add(nSlotId, piContext);
}

PUBLIC
void MtcContextRepository::RemoveContext(IN IMS_SINT32 nSlotId)
{
    m_objContexts.Remove(nSlotId);
}
