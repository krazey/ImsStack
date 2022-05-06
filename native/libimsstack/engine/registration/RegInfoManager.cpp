/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100719  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceMutex.h"
#ifdef __IMS_ASYNC_XML_PARSER__
#include "IXmlTransactionProvider.h"
#include "XmlFactory.h"
#endif
#include "RegInfo.h"
#include "RegInfoParser.h"
#include "RegInfoManager.h"

__IMS_TRACE_TAG_REG__;

PRIVATE
RegInfoManager::RegInfoManager() :
        piLock(IMS_NULL),
        objParsers(IMSList<RegInfoParser*>()),
        objRegInfos(IMSMap<RegKey, RegInfo*>())
#ifdef __IMS_ASYNC_XML_PARSER__
        ,
        piXmlTxnProvider(IMS_NULL)
#endif
{
    piLock = MutexService::GetMutexService()->CreateMutex();
}

PRIVATE VIRTUAL RegInfoManager::~RegInfoManager()
{
    LockGuard objLock(piLock);

    if (!objRegInfos.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objRegInfos.GetSize(); ++i)
        {
            RegInfo* pRegInfo = objRegInfos.GetValueAt(i);

            if (pRegInfo != IMS_NULL)
            {
                delete pRegInfo;
            }
        }

        objRegInfos.Clear();
    }

    if (!objParsers.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objParsers.GetSize(); ++i)
        {
            RegInfoParser* pParser = objParsers.GetAt(i);

            if (pParser != IMS_NULL)
            {
                delete pParser;
            }
        }

        objParsers.Clear();
    }

#ifdef __IMS_ASYNC_XML_PARSER__
    if (piXmlTxnProvider != IMS_NULL)
    {
        XmlFactory::GetInstance()->DestroyTransactionProvider(piXmlTxnProvider);
        piXmlTxnProvider = IMS_NULL;
    }
#endif

    MutexService::GetMutexService()->DestroyMutex(piLock);
}

PUBLIC
IMS_BOOL RegInfoManager::CreateRegInfo(IN const RegKey& objRegKey)
{
    RegInfo* pRegInfo = const_cast<RegInfo*>(GetRegInfo(objRegKey));

    if (pRegInfo == IMS_NULL)
    {
        pRegInfo = new RegInfo();

        if (pRegInfo == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a RegInfo(%d:%d) failed", objRegKey.GetSlotId(),
                    objRegKey.GetFlowId(), 0);
            return IMS_FALSE;
        }

        LockGuard objLock(piLock);

        if (!objRegInfos.Add(objRegKey, pRegInfo))
        {
            delete pRegInfo;

            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC
void RegInfoManager::DestroyRegInfo(IN const RegKey& objRegKey)
{
    LockGuard objLock(piLock);
    IMS_SLONG nIndex = objRegInfos.GetIndexOfKey(objRegKey);

    if (nIndex < 0)
    {
        return;
    }

    RegInfo* pRegInfo = objRegInfos.GetValueAt(nIndex);

    if (pRegInfo != IMS_NULL)
    {
        delete pRegInfo;
    }

    objRegInfos.RemoveAt(nIndex);

    IMS_TRACE_I("RegInfo(%d:%d) is destroyed", objRegKey.GetSlotId(), objRegKey.GetFlowId(), 0);
}

PUBLIC
RegInfo* RegInfoManager::GetRegInfo(IN const RegKey& objRegKey)
{
    LockGuard objLock(piLock);
    IMS_SLONG nIndex = objRegInfos.GetIndexOfKey(objRegKey);

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    return objRegInfos.GetValueAt(nIndex);
}

PUBLIC
const RegInfo* RegInfoManager::GetRegInfo(IN const RegKey& objRegKey) const
{
    LockGuard objLock(piLock);
    IMS_SLONG nIndex = objRegInfos.GetIndexOfKey(objRegKey);

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    return objRegInfos.GetValueAt(nIndex);
}

PUBLIC
IMS_BOOL RegInfoManager::Initialize()
{
#ifdef __IMS_ASYNC_XML_PARSER__
    if (piXmlTxnProvider != IMS_NULL)
    {
        return IMS_TRUE;
    }

    piXmlTxnProvider = XmlFactory::GetInstance()->CreateTransactionProvider();

    if (piXmlTxnProvider == IMS_NULL)
    {
        return IMS_FALSE;
    }
#endif

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL RegInfoManager::Update(IN const RegKey& objRegKey, IN const AString& strRegInfo)
{
    if (strRegInfo.IsNULL())
    {
        return IMS_TRUE;
    }

    const RegInfo* pRegInfo = GetRegInfo(objRegKey);

    if (pRegInfo == IMS_NULL)
    {
        IMS_TRACE_E(
                0, "No matched RegInfo(%d:%d)", objRegKey.GetSlotId(), objRegKey.GetFlowId(), 0);
        return IMS_FALSE;
    }

#ifdef __IMS_ASYNC_XML_PARSER__

    if (piXmlTxnProvider == IMS_NULL)
    {
        return IMS_FALSE;
    }

    RegInfoParser* pParser = new RegInfoParser(objRegKey, piXmlTxnProvider);

#else

    RegInfoParser* pParser = new RegInfoParser(objRegKey);

#endif  // __IMS_ASYNC_XML_PARSER__

    if (pParser == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating RegInfoParser failed", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!AddRegInfoParser(pParser))
    {
        delete pParser;

        return IMS_FALSE;
    }

    pParser->SetListener(this);

    if (!pParser->Parse(strRegInfo))
    {
        IMS_TRACE_E(0, "Parsing 'reginfo' failed", 0, 0, 0);

        RemoveRegInfoParser(pParser);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
void RegInfoManager::DisplayRegInfo()
{
    for (IMS_UINT32 i = 0; i < objRegInfos.GetSize(); ++i)
    {
        RegInfo* pRegInfo = objRegInfos.GetValueAt(i);

        IMS_TRACE_D("___ REG INFO (%d) -- START ___", i, 0, 0);
        pRegInfo->DisplayRegInfo();
        IMS_TRACE_D("___ REG INFO (%d) -- END ___\n", i, 0, 0);
    }
}

PUBLIC GLOBAL RegInfoManager* RegInfoManager::GetInstance()
{
    static RegInfoManager* pRegInfoMngr = IMS_NULL;

    if (pRegInfoMngr == IMS_NULL)
    {
        pRegInfoMngr = new RegInfoManager();
    }

    return pRegInfoMngr;
}

PRIVATE VIRTUAL void RegInfoManager::RegInfoParser_ParsingCompleted(
        IN RegInfoParser* pParser, IN IDocument* piDocument)
{
    RegInfo* pRegInfo = GetRegInfo(pParser->GetRegKey());

    if (pRegInfo != IMS_NULL)
    {
        pRegInfo->Update(piDocument);
    }
    else
    {
        IMS_TRACE_D("No matching RegInfo(%d:%d)", pParser->GetRegKey().GetSlotId(),
                pParser->GetRegKey().GetFlowId(), 0);
    }

    RemoveRegInfoParser(pParser);
}

PRIVATE VIRTUAL void RegInfoManager::RegInfoParser_ParsingFailed(IN RegInfoParser* pParser)
{
    RegInfo* pRegInfo = GetRegInfo(pParser->GetRegKey());

    IMS_TRACE_D("Parsing 'reginfo' failed", 0, 0, 0);

    if (pRegInfo != IMS_NULL)
    {
        pRegInfo->Update(IMS_NULL);
    }

    RemoveRegInfoParser(pParser);
}

PRIVATE
IMS_BOOL RegInfoManager::AddRegInfoParser(IN RegInfoParser* pParser)
{
    LockGuard objLock(piLock);
    return objParsers.Append(pParser);
}

PRIVATE
void RegInfoManager::RemoveRegInfoParser(IN RegInfoParser*& pParser)
{
    LockGuard objLock(piLock);

    for (IMS_UINT32 i = 0; i < objParsers.GetSize(); ++i)
    {
        RegInfoParser* pTmpParser = objParsers.GetAt(i);

        if (pParser == pTmpParser)
        {
            delete pParser;
            pParser = IMS_NULL;

            objParsers.RemoveAt(i);
            return;
        }
    }
}
