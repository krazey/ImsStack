#ifndef _SIP_METHOD_H_
#define _SIP_METHOD_H_

#include "AString.h"

/**
 * @brief This class provides an interface for SIP methods.
 */
class SIPMethod
{
public:
    SIPMethod(IN IMS_SINT32 nMethod_ = SIPMethod::INVALID);
    SIPMethod(IN CONST IMS_CHAR *pszMethod_);
    SIPMethod(IN CONST AString &strMethod_);
    SIPMethod(IN CONST SIPMethod &objRHS);
    ~SIPMethod();

public:
    SIPMethod& operator=(IN CONST SIPMethod &objRHS);
    SIPMethod& operator=(IN IMS_SINT32 nMethod_);
    SIPMethod& operator=(IN CONST IMS_CHAR *pszMethod_);
    SIPMethod& operator=(IN CONST AString &strMethod_);

public:
    /**
     * @brief Checks if SIP method is the same or not.
     *
     * @param nMethod SIP method enumeration type\n
     *                #ACK\n
     *                #BYE\n
     *                #CANCEL\n
     *                #INVITE\n
     *                #OPTIONS\n
     *                #REGISTER\n
     *                #PRACK\n
     *                #SUBSCRIBE\n
     *                #NOTIFY\n
     *                #UPDATE\n
     *                #MESSAGE\n
     *                #REFER\n
     *                #PUBLISH\n
     *                #INFO\n
     *                #UNKNOWN
     * @return If it matches, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL Equals(IN IMS_SINT32 nMethod) const
    { return (this->nMethod == nMethod); }
    /**
     * @brief Checks if SIP method is the same or not.
     *
     * @param pszMethod SIP method as null-termianted string format
     * @return If it matches, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL Equals(IN CONST IMS_CHAR *pszMethod) const
    { return (this->strMethod.Equals(pszMethod)); }
    /**
     * @brief Checks if SIP method is the same or not.
     *
     * @param strMethod SIP method as string format
     * @return If it matches, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL Equals(IN CONST AString &strMethod) const
    { return (this->strMethod.Equals(strMethod)); }
    /**
     * @brief Checks if SIP method is the same or not.
     *
     * @param objMethod SIP method object
     * @return If it matches, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL Equals(IN CONST SIPMethod &objMethod) const
    {
        return ((this->nMethod == objMethod.nMethod)
                && (this->strMethod.Equals(objMethod.strMethod)));
    }

    /**
     * @brief Gets SIP method as enumeration type.
     *
     * @return SIP method.\n
     *         #ACK\n
     *         #BYE\n
     *         #CANCEL\n
     *         #INVITE\n
     *         #OPTIONS\n
     *         #REGISTER\n
     *         #PRACK\n
     *         #SUBSCRIBE\n
     *         #NOTIFY\n
     *         #UPDATE\n
     *         #MESSAGE\n
     *         #REFER\n
     *         #PUBLISH\n
     *         #INFO\n
     *         #UNKNOWN
     */
    inline IMS_SINT32 ToInt() const { return nMethod; }
    /**
     * @brief Gets SIP method as string format.
     *
     * @return SIP method as string ("INVITE" / "BYE" / ...).
     */
    inline const AString& ToString() const { return strMethod; }

    /**
     * @brief Converts the enumeration type to string format of SIP method.
     *
     * @param nMethod SIP method enumeration type\n
     *                #ACK\n
     *                #BYE\n
     *                #CANCEL\n
     *                #INVITE\n
     *                #OPTIONS\n
     *                #REGISTER\n
     *                #PRACK\n
     *                #SUBSCRIBE\n
     *                #NOTIFY\n
     *                #UPDATE\n
     *                #MESSAGE\n
     *                #REFER\n
     *                #PUBLISH\n
     *                #INFO\n
     *                #UNKNOWN
     * @return SIP method as null-terminated string format ("INVITE" / "BYE" / ...).
     */
    static const IMS_CHAR* ToName(IN IMS_SINT32 nMethod);

private:
    static IMS_SINT32 ConvertStringToMethod(IN CONST AString &strMethod);
    static AString ConvertMethodToString(IN IMS_SINT32 nMethod);

public:
    enum
    {
        INVALID = (-1),

        /// Defined in RFC 3261
        ACK = 0,
        BYE, ///< Defined in RFC 3261
        CANCEL, ///< Defined in RFC 3261
        INVITE, ///< Defined in RFC 3261
        OPTIONS, ///< Defined in RFC 3261
        REGISTER, ///< Defined in RFC 3261

        /// Defined in RFC 3262
        PRACK,

        /// Defined in RFC 3265
        SUBSCRIBE,
        NOTIFY, ///< Defined in RFC 3265

        /// Defined in RFC 3311
        UPDATE,

        /// Defined in RFC 3428
        MESSAGE,

        /// Defined in RFC 3515
        REFER,

        /// Defined in RFC 3903
        PUBLISH,

        /// Defined in RFC 2976
        INFO,

        UNKNOWN,

        MAX
    };

    static const IMS_CHAR* NAME[];
    static const SIPMethod INVALID_METHOD;

private:
    IMS_SINT32 nMethod;
    AString strMethod;
};

#endif // _SIP_METHOD_H_
