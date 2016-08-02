#ifndef __RD_CXML_H__
#define __RD_CXML_H__

#include<string>
#include "tinyxml.h"

using namespace std;


class CXML
{
public:
	static CXML* getInstance()
	{
		if ( instance == NULL )
		{
			instance = new CXML();
		}
		return instance;
	}

private:

    TiXmlDocument m_xml;

    TiXmlElement* pElement;
private:
    TiXmlElement* getFirstElement(string ElementMark,TiXmlElement* pcrElement);

public:
    int ParseXmlStr(string xmlstr);
    int ParseXmlFile(string xmlFile);
    int getFirstElementValue(string ElementMark,string& value);
    int getNextElementValue(string ElementMark,string& value);
    int getElementAttributeValue(string AttributeName,string& value);
    TiXmlElement* getRootElement();
    string getXmlStr();
    void Clear();
    TiXmlElement* addXmlRootElement(string ElementMark);
    TiXmlElement* addXmlChildElement(TiXmlElement* pElement,string ElementMark);
    void addElementValue(TiXmlElement* pElement,string value);
    void addXmlAttribute(TiXmlElement* pElement,string AttributeMark,string value);
    void addXmlDeclaration(string vesion,string encoding,string standalone);
    void addXmlComment(TiXmlElement* pElement,string Comment);
    void saveFile(string FileName);
	
private:
	 CXML(void);
	static  CXML *instance;
};


#endif
