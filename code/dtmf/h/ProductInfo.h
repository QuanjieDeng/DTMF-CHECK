// ProductInfo.h: interface for the ProductInfo class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __AS_PRODUCTINFO_H__
#define __AS_PRODUCTINFO_H__

#include <string>
#include <memory>

using namespace std;
const bool PRODUCT_BUILD_TYPE_ISRELEASE = false;
const int PRODUCT_MAJOR_VERSION_NUMBER = 1;
const int PRODUCT_MINOR_VERSION_NUMBER = 0;
const int PRODUCT_REVISION_VERSION_NUMBER = 1;
const int PRODUCT_BUILD_VERSION_NUMBER = 0;
#ifdef SDF_HA
const string PRODUCT_NAME = "Polylink  DtmfServer(HA)"; 
#else
const string PRODUCT_NAME = "Polylink DtmfServer"; 
#endif 
const string PRODUCT_BUILD_TYPE_RELEASE = "RELEASE SOFTWARE";
const string PRODUCT_BUILD_TYPE_DEBUD = "DEBUG SOFTWARE";
const string PRODUCT_PLATFORM = "CentOS6.4";
const string PRODUCT_TECHNICAL_SUPPORT = "http://www.polylink.net"; 
const string PRODUCT_COPYRIGHT_INITIAL_YEAR = "2015";
const string PRODUCT_COPYRIGHT_FINAL_YEAR = "2015";
const string PRODUCT_BUILD_TIME = "2014-12-02_16:15:36";
const string PRODUCT_BUILD_REVISION = "r";
const string PRODUCT_BUILDER = "polylink_VC_team";
class ProductInfo  
{
public:
	
	~ProductInfo();	
	static ProductInfo *Instance();
	string GetProductName();
	string GetVersion();	
	string GetBuildType();	
	string GetPlatform();	
	string GetTechnicalSupport();
	string GetCopyright();
	string GetBuildTime();
	string GetBuilder();
	string GetProductAllInfo();
	string GetBuildTimeAndBuilder();
	string GetBuildRevision();
	string GetProductBasicInfo();

private:
	
	ProductInfo();	
	string ConvertIntToString(int iNumber);	
private:	
	static auto_ptr< ProductInfo >s_auto_ptr_instance ;			
	int  m_iMajorVersionNumber;	
	int  m_iMinorVersionNumber;
	int  m_iRevisionVersionNumber;	
	int  m_iBuildVersionNumber;

	string m_strProductName;    
	string m_strBuildType;
	string m_strPlatform;
	string m_strTechnicalSupport;
	string m_strCopyrightInitialYear;
	string m_strCopyrightFinalYear;
	string m_strBuildTime;
	string m_strBuildRevision;
	string m_strBuilder;

};
#endif //#ifndef __AS_PRODUCTINFO_H__
