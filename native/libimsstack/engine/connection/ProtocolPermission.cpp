/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091027  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#ifdef ___IMS_DYNAMIC_BINDER___
#include "Protocol.h"
#else
#include "SipProtocol.h"
#include "IMSCoreProtocol.h"

#endif  // ___IMS_DYNAMIC_BINDER___

#include "ProtocolPermission.h"

#ifdef ___IMS_DYNAMIC_BINDER___
class ProtocolBinder
{
public:
    inline ProtocolBinder(IN CONST AString& strName_, IN Protocol* pProtocol_) :
            strName(strName_),
            pProtocol(pProtocol_)
    {
    }
    inline ~ProtocolBinder() {}

public:
    inline const AString& GetName() const { return strName; }
    inline Protocol* GetProtocol() const { return pProtocol; }

private:
    AString strName;
    Protocol* pProtocol;
};

class ProtocolPermissionPrivate
{
public:
    inline ProtocolPermissionPrivate() {}
    inline ~ProtocolPermissionPrivate()
    {
        for (IMS_UINT32 i = 0; i < objBinders.GetSize(); i++)
        {
            ProtocolBinder* pBinder = objBinders.GetAt(i);

            if (pBinder != IMS_NULL)
            {
                delete pBinder;
            }
        }
    }

private:
    friend class ProtocolPermission;

    IMSList<ProtocolBinder*> objBinders;
};

#else

class ProtocolPermissionPrivate
{
};

struct ProtocolBinder
{
    const IMS_CHAR* pszName;
    Protocol* pProtocol;
};

static const ProtocolBinder PROTOCOL_BINDER[] = {
        {IMSCore::CONNECTION_SCHEME, IMSCoreProtocol::GetInstance()},
        {Sip::CONNECTION_SCHEME_SIP, SIPProtocol::GetInstance()    }
  //    { Sip::CONNECTION_SCHEME_SIPS, SIPProtocol::GetInstance() }
};
#endif  // ___IMS_DYNAMIC_BINDER___

PRIVATE
ProtocolPermission::ProtocolPermission() :
        pProtocolPermissionP(IMS_NULL)
{
}

PUBLIC
ProtocolPermission::~ProtocolPermission()
{
    if (pProtocolPermissionP != IMS_NULL)
    {
        delete pProtocolPermissionP;
        pProtocolPermissionP = IMS_NULL;
    }
}

/*

Remarks

*/
PUBLIC
Protocol* ProtocolPermission::Lookup(IN const AString& strName)
{
#ifdef ___IMS_DYNAMIC_BINDER___

    for (IMS_UINT32 i = 0; i < pProtocolPermissionP->objBinders.GetSize(); i++)
    {
        ProtocolBinder* pBinder = pProtocolPermissionP->objBinders.GetAt(i);

        if (pBinder != IMS_NULL)
        {
            if (strName.EqualsIgnoreCase(pBinder->GetName()))
            {
                return pBinder->GetProtocol();
            }
        }
    }

#else

    IMS_SINT32 nNumOfBinder = sizeof(PROTOCOL_BINDER) / sizeof(PROTOCOL_BINDER[0]);

    for (IMS_SINT32 i = 0; i < nNumOfBinder; ++i)
    {
        const ProtocolBinder* pBinder = &(PROTOCOL_BINDER[i]);

        if (strName.EqualsIgnoreCase(pBinder->pszName))
        {
            return pBinder->pProtocol;
        }
    }

#endif  // ___IMS_DYNAMIC_BINDER___

    return IMS_NULL;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL ProtocolPermission::Register(IN const AString& strName, IN Protocol* pProtocol)
{
#ifdef ___IMS_DYNAMIC_BINDER___

    ProtocolBinder* pBinder;

    for (IMS_UINT32 i = 0; i < pProtocolPermissionP->objBinders.GetSize(); i++)
    {
        pBinder = pProtocolPermissionP->objBinders.GetAt(i);

        if (pBinder != IMS_NULL)
        {
            if (strName.EqualsIgnoreCase(pBinder->GetName()))
            {
                return IMS_FALSE;
            }
        }
    }

    pBinder = new ProtocolBinder(strName, pProtocol);

    if (pBinder == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!pProtocolPermissionP->objBinders.Append(pBinder))
    {
        delete pBinder;

        return IMS_FALSE;
    }

#else

    (void)strName;
    (void)pProtocol;

#endif  // ___IMS_DYNAMIC_BINDER___

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL ProtocolPermission::Deregister(IN const AString& strName)
{
#ifdef ___IMS_DYNAMIC_BINDER___

    for (IMS_UINT32 i = 0; i < pProtocolPermissionP->objBinders.GetSize(); i++)
    {
        ProtocolBinder* pBinder = pProtocolPermissionP->objBinders.GetAt(i);

        if (pBinder != IMS_NULL)
        {
            if (strName.EqualsIgnoreCase(pBinder->GetName()))
            {
                pProtocolPermissionP->objBinders.RemoveAt(i);

                delete pBinder;
                return IMS_TRUE;
            }
        }
    }

#else

    (void)strName;

#endif  // ___IMS_DYNAMIC_BINDER___

    return IMS_FALSE;
}

/*

Remarks

*/
PUBLIC GLOBAL ProtocolPermission* ProtocolPermission::GetInstance()
{
    static ProtocolPermission* pPotocolPermission = IMS_NULL;

    if (pPotocolPermission == IMS_NULL)
    {
        pPotocolPermission = new ProtocolPermission();
    }

    return pPotocolPermission;
}
