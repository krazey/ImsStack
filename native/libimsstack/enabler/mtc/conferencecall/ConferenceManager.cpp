#include "ServiceTrace.h"

#include "IMtcContext.h"
#include "conferencecall/ConferenceManager.h"
#include "conferencecall/ConferenceController.h"
#include "conferencecall/MergeController.h"
#include "conferencecall/GroupCallController.h"
#include "conferencecall/ExpandController.h"
#include "conferencecall/ConferenceDef.h"
#include "helper/CallStateProxy.h"

__IMS_TRACE_TAG_COM_MTC__;


PUBLIC
ConferenceManager::ConferenceManager(IN IMtcContext& objContext) :
        m_objContext(objContext),
        m_objControllers(IMSMap<CallKey, ConferenceController*>()),
        m_objDestroyer(ObjectAsyncDestroyer<ConferenceController>()),
        m_objCallConnectionIdManager(CallConnectionIdManager(objContext))
{
    IMS_TRACE_D("+ConferenceManager", 0, 0, 0);
}

PUBLIC
ConferenceManager::~ConferenceManager()
{
    IMS_TRACE_D("~ConferenceManager", 0, 0, 0);

    for (IMS_UINT32 i = 0; i < m_objControllers.GetSize(); i++)
    {
        ConferenceController* pController = m_objControllers.GetValueAt(i);
        delete pController;
    }

    m_objControllers.Clear();
}

PUBLIC
void ConferenceManager::OnClosed(IN ConferenceController* pController)
{
    IMS_TRACE_D("OnClosed", 0, 0, 0);
    ReleaseController(pController);
}

PUBLIC
IConferenceController& ConferenceManager::CreateController(IN CallKey nCallKey,
        IN ConferenceType eType)
{
    ConferenceController* pController = IMS_NULL;

    IMS_TRACE_D("CreateController : type=[%d]", eType, 0, 0);

    switch (eType)
    {
        case ConferenceType::PARTICIPANT:
            pController =
                    new ConferenceController(nCallKey, m_objContext, m_objCallConnectionIdManager);
            break;
        case ConferenceType::GROUP_CALL:
            pController =
                    new GroupCallController(nCallKey, m_objContext, m_objCallConnectionIdManager);
            break;
        case ConferenceType::MERGE_CALL:
            pController =
                    new MergeController(nCallKey, m_objContext, m_objCallConnectionIdManager);
            break;
        case ConferenceType::EXPAND_CALL:
            pController =
                    new ExpandController(nCallKey, m_objContext, m_objCallConnectionIdManager);
            break;

        default:
            IMS_TRACE_E(0, "invalid conference manager type. Create MergeController", 0, 0, 0);
            pController = new MergeController(nCallKey, m_objContext, m_objCallConnectionIdManager);
            break;
    }

    pController->SetListener(this);
    m_objCallConnectionIdManager.OnConferenceCallStarted(pController, IMS_TRUE);
    m_objControllers.Add(nCallKey, pController);
    return *pController;
}

PUBLIC
IConferenceController* ConferenceManager::GetController(IN IMS_UINTP nCallKey) const
{
    IMS_SLONG nIndex = m_objControllers.GetIndexOfKey(nCallKey);
    if (nIndex >= 0)
    {
        return m_objControllers.GetValueAt(nIndex);
    }

    return IMS_NULL;
}

PRIVATE
void ConferenceManager::ReleaseController(IN ConferenceController* pController)
{
    IMS_TRACE_D("ReleaseController : size=[%d]", m_objControllers.GetSize(), 0, 0);

    for (IMS_UINT32 i = 0; i < m_objControllers.GetSize(); i++)
    {
        ConferenceController* pTempController = m_objControllers.GetValueAt(i);
        if (pController == pTempController)
        {
            m_objControllers.RemoveAt(i);
            m_objCallConnectionIdManager.OnConferenceCallStarted(pController, IMS_FALSE);
            m_objDestroyer.Destroy(pController);
            break;
        }
    }
}
