#ifndef _REPLACES_H_
#define _REPLACES_H_

#include "AString.h"

/**
 * @brief This class defines a helper class to access Replaces header.
 */
class Replaces
{
public:
    Replaces();
    Replaces(IN CONST AString& strCallId_, IN CONST AString& strLocalTag_,
            IN CONST AString& strRemoteTag_, IN IMS_BOOL bIsEarlyOnly_ = IMS_FALSE);
    Replaces(IN CONST Replaces& objRHS);
    ~Replaces();

public:
    Replaces& operator=(IN CONST Replaces& objRHS);

public:
    /**
     * @brief Parses Replaces header.
     *
     * @param strReplacesHeader Replaces header string
     * @param bUAS Flag to indicate that UA mode of this Replaces header is UAS
     * @return If it's successfully parsed, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL Create(IN CONST AString& strReplacesHeader, IN IMS_BOOL bUAS = IMS_TRUE);
    /**
     * @brief Checks if the given Replaces object is the same or not.
     *
     * @param pOther Pointer to Replaces object to be compared
     * @return If both Replaces are the same, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL Equals(IN CONST Replaces* pOther) const;
    /**
     * @brief Gets call-id value.
     *
     * @return Call-id value of Replaces header.
     */
    const AString& GetCallId() const;
    /**
     * @brief Gets from-tag value.
     *
     * @return From-tag value of Replaces header.
     */
    const AString& GetFromTag() const;
    /**
     * @brief Gets to-tag value.
     *
     * @return To-tag value of Replaces header.
     */
    const AString& GetToTag() const;
    /**
     * @brief Checks if it has "early-only" parameter or not.
     *
     * @return If it has "early-only" parameter, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL IsEarlyOnly() const;
    /**
     * @brief Checks if it's the same dialog or not.
     *
     * It conducts the dialog comparison.
     *
     * @return If it's the same dialog, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL IsSameDialog(IN CONST Replaces* pOther) const;
    /**
     * @brief Returns SIP header string representation of this Repaces object.
     *
     * @return A string representation of this Replaces object.
     */
    AString ToString(IN IMS_BOOL bPercentEncoding) const;

public:
    class Dialog
    {
    public:
        inline Dialog(IN CONST AString& strCallId_, IN CONST AString& strLocalTag_,
                IN CONST AString& strRemoteTag_) :
                strCallId(strCallId_),
                strLocalTag(strLocalTag_),
                strRemoteTag(strRemoteTag_)
        {
        }

        inline ~Dialog() {}

    private:
        Dialog();
        Dialog(IN CONST Dialog& objRHS);

    private:
        Dialog& operator=(IN CONST Dialog& objRHS);

    public:
        inline IMS_BOOL Equals(IN CONST Dialog* pCallLeg) const
        {
            if (pCallLeg == IMS_NULL)
                return IMS_FALSE;

            if (!strCallId.Equals(pCallLeg->strCallId))
                return IMS_FALSE;

            if (!strLocalTag.Equals(pCallLeg->strLocalTag))
                return IMS_FALSE;

            if (!strRemoteTag.Equals(pCallLeg->strRemoteTag))
                return IMS_FALSE;

            return IMS_TRUE;
        }

        inline const AString& GetCallId() const { return strCallId; }
        inline const AString& GetLocalTag() const { return strLocalTag; }
        inline const AString& GetRemoteTag() const { return strRemoteTag; }

    private:
        const AString& strCallId;
        const AString& strLocalTag;
        const AString& strRemoteTag;
    };

private:
    AString strCallId;
    AString strFromTag;
    AString strToTag;
    IMS_BOOL bIsEarlyOnly;

    Dialog* pDialog;
};

#endif  // _REPLACES_H_
