#include "ServiceTrace.h"
#include "ect/EctManager.h"
#include "ect/EctController.h"
#include "helper/ObjectAsyncDestroyer.h"
#include "IMtcContext.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
EctManager::EctManager(IN IMtcContext& objContext) :
        m_objContext(objContext),
        m_pController(IMS_NULL),
        m_objDestroyer(ObjectAsyncDestroyer<EctController>())
{
    IMS_TRACE_D("+EctManager", 0, 0, 0);
}

PUBLIC
EctManager::~EctManager()
{
    IMS_TRACE_D("~EctManager", 0, 0, 0);
}

PUBLIC VIRTUAL void EctManager::OnEctCompleted()
{
    m_objDestroyer.Destroy(m_pController);
}

PUBLIC
void EctManager::Transfer(IN CallKey nCallKey, IN const AString& strNumber)
{
    if (m_pController)
    {
        IMS_TRACE_E(0, "no multiple ECT is supported.", 0, 0, 0);
        // TODO: send error to UI.
        return;
    }

    if (strNumber.GetLength() > 0)
    {
        // TODO: BlindTransferController
        m_pController = new EctController(m_objContext, nCallKey);
        m_pController->Transfer(strNumber);
    }
    else
    {
        // TODO: ConsultativeTransferController
        m_pController = new EctController(m_objContext, nCallKey);
        m_pController->Transfer();
    }
}
