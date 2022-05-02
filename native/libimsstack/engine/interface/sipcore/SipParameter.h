#ifndef _SIP_PARAMETER_H_
#define _SIP_PARAMETER_H_

#include "AStringArray.h"

/**
 * @brief This class provides an interface for SIP parameters (URI / header parameters).
 */
class SIPParameter
{
public:
    SIPParameter();
    explicit SIPParameter(IN CONST AString &strName_);
    SIPParameter(IN CONST AString &strName_, IN CONST AString &strValue_);
    SIPParameter(IN CONST AString &strName_, IN CONST AStringArray &objValues_);
    SIPParameter(IN CONST SIPParameter &objRHS);
    ~SIPParameter();

public:
    SIPParameter& operator=(IN CONST SIPParameter &objRHS);

public:
    /**
     * @brief Adds the value of SIP parameter.
     *
     * @param strValue The parameter value to be added
     */
    void AddValue(IN CONST AString &strValue);
    /**
     * @brief Adds the values (comma-separated string format) of SIP parameter.
     *
     * @param strValues The parameter values to be added
     */
    void AddValues(IN CONST AString &strValues);
    /**
     * @brief Parses the SIP parameter.
     *
     * @param strParameter The SIP parameter value format\n
     *                     If it represents multiple values, it's a comma-separated string format.
     */
    IMS_BOOL Create(IN CONST AString &strParameter);
    /**
     * @brief Checks if both SIP parameter is the same or not.
     *
     * @param pParameter The SIP parameter to be compared
     * @return If both SIP parameter is matched, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL Equals(IN CONST SIPParameter *pParameter) const;
    /**
     * @brief Gets the name of SIP parameter.
     *
     * @return The SIP parameter name.
     */
    const AString& GetName() const;
    /**
     * @brief Gets the topmost value of SIP parameter.
     *
     * @return The topmost SIP parameter value.
     */
    const AString& GetValue() const;
    /**
     * @brief Gets all the values of SIP parameter.
     *
     * @return The SIP parameter values.
     */
    const AStringArray& GetValues() const;
    /**
     * @brief Checks if this SIP parameter is name-only parameter.
     *
     * @return If it's a name-only parameter (boolean parameter), returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL IsNameOnly() const;
    /**
     * @brief Removes the specified value from SIP parameter.
     *
     * @param strValue The parameter value to be removed
     */
    void RemoveValue(IN CONST AString &strValue);
    /**
     * @brief Sets the value of SIP parameter.
     *
     * @param strValue The parameter value to be set
     */
    IMS_RESULT SetValue(IN CONST AString &strValue);
    /**
     * @brief Sets the values (comma-separated string format) of SIP parameter.
     *
     * @param strValues The parameter values to be set
     */
    IMS_RESULT SetValues(IN CONST AString &strValues);
    /**
     * @brief Returns SIP parameter value as a string format.
     *
     * @return The string format of SIP parameter.
     */
    AString ToString() const;

private:
    AString strName;
    AStringArray objValues;
};

#endif // _SIP_PARAMETER_H_
