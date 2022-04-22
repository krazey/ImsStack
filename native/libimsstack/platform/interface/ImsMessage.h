/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090819  YR@                       Created
    </table>

    Description

*/

#ifndef _IMS_MSG_H_
#define _IMS_MSG_H_

#include "IMSStrLib.h"
#include "AString.h"

class IMSMSG
{
public:
    // This interface will be used in the system(platform) layer
    // to handle the asynchronous operations.
    // The applications can use this interface, but it should not be overused.
    class IMessageCallback
    {
    public:
        virtual void MessageCallback_OnMessage(IN IMSMSG& objMsg) = 0;
    };

public:
    inline IMSMSG()
        : nMSG(0)
        , nWparam(0)
        , nLparam(0)
        , piCallback(IMS_NULL)
    {
        acActivityName[0] = '\0';
    }

    inline IMSMSG(IN IMS_SINT32 nMSG_, IN IMS_UINTP nWparam_, IN IMS_UINTP nLparam_,
            IN const AString &strTarget = AString::ConstEmpty())
        : nMSG(nMSG_)
        , nWparam(nWparam_)
        , nLparam(nLparam_)
        , piCallback(IMS_NULL)
    {
        if (strTarget.GetLength() == 0)
        {
            acActivityName[0] = '\0';
        }
        else
        {
            IMS_StrCpy(acActivityName, MAX_ACTIVITY_NAME + 1, strTarget.GetStr());
        }
    }

    inline IMSMSG(IN IMS_SINT32 nMSG_, IN IMS_UINTP nWparam_, IN IMS_UINTP nLparam_,
            IN IMessageCallback* piCallback_)
        : nMSG(nMSG_)
        , nWparam(nWparam_)
        , nLparam(nLparam_)
        , piCallback(piCallback_)
    {
        acActivityName[0] = '\0';
    }

    inline ~IMSMSG()
    {}

    inline const IMS_CHAR* GetTargetName() const
    { return (acActivityName[0] == '\0') ? IMS_NULL : &acActivityName[0]; }

    inline IMS_SINT32 GetName() const
    { return nMSG; }

    inline IMS_BOOL HasCallback() const
    { return piCallback != IMS_NULL; }

    inline void SetTarget(IN const IMS_CHAR *pszName)
    { IMS_StrCpy(acActivityName, MAX_ACTIVITY_NAME + 1, pszName); }

    inline void SetCallback(IN IMessageCallback* piCallback)
    { this->piCallback = piCallback; }

protected:
    inline void InvokeCallback()
    {
        if (piCallback != IMS_NULL)
        {
            piCallback->MessageCallback_OnMessage(*this);
        }
    }

public:
    IMS_SINT32 nMSG;
    IMS_UINTP nWparam;
    IMS_UINTP nLparam;

private:
    friend class ImsThread;

    static const IMS_SINT32 MAX_ACTIVITY_NAME = 64;

    IMessageCallback* piCallback;

    // Empty Activity name means that the message is a broadcast
    IMS_CHAR acActivityName[MAX_ACTIVITY_NAME + 1];
};

#endif // _IMS_MSG_H_
