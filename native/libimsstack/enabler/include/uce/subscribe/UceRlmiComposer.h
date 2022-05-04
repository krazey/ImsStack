/*


*/

#ifndef _UCE_RLMI_COMPOSER_H_
#define _UCE_RLMI_COMPOSER_H_

#include "AString.h"

class IXmlStreamWriter;
class XmlFactory;

class UceRlmiComposer {
  /* ------------------------------------------------------------------------------------------
      Constructor, Destructor, Operator Overloading
  ---------------------------------------------------------------------------------------------
*/
 public:
  UceRlmiComposer();

  ~UceRlmiComposer();
  /* ------------------------------------------------------------------------------------------
      Methods
  ---------------------------------------------------------------------------------------------
*/
 public:
  AString ComposeRLMIList(IN IMSList<AString>& pContactInfoList);  //
 private:
  void EncodeResourceXMLNameSpace(IN IXmlStreamWriter*& piWriter);  //
  /* ------------------------------------------------------------------------------------------
      Variables
  ---------------------------------------------------------------------------------------------
*/
 private:
  XmlFactory* m_pXMLOutputFactory;
};
#endif  // _UCE_RLMI_COMPOSER_H_
