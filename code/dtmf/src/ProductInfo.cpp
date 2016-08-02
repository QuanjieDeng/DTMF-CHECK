// ProductInfo.cpp: implementation of the ProductInfo class.
//
//////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <stdlib.h>
#include "ProductInfo.h"

auto_ptr<ProductInfo>  ProductInfo::s_auto_ptr_instance ;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
ProductInfo::ProductInfo()
:m_iMajorVersionNumber(PRODUCT_MAJOR_VERSION_NUMBER ),
m_iMinorVersionNumber(PRODUCT_MINOR_VERSION_NUMBER ),
m_iRevisionVersionNumber(PRODUCT_REVISION_VERSION_NUMBER ),	
m_iBuildVersionNumber(PRODUCT_BUILD_VERSION_NUMBER),
m_strProductName(PRODUCT_NAME),
m_strPlatform(PRODUCT_PLATFORM ),
m_strTechnicalSupport(PRODUCT_TECHNICAL_SUPPORT),
m_strCopyrightInitialYear(PRODUCT_COPYRIGHT_INITIAL_YEAR),
m_strCopyrightFinalYear(PRODUCT_COPYRIGHT_FINAL_YEAR),
m_strBuildTime(PRODUCT_BUILD_TIME),
m_strBuildRevision(PRODUCT_BUILD_REVISION),
m_strBuilder(PRODUCT_BUILDER)
{}

ProductInfo::~ProductInfo()
{
}


// set a instance
ProductInfo * ProductInfo::Instance()
{
	if(NULL == s_auto_ptr_instance .get())
	{
		s_auto_ptr_instance .reset( new ProductInfo());
	}
	
	return s_auto_ptr_instance.get();
}


//get product name
string ProductInfo::GetProductName()  
{
	return m_strProductName;
}


//get product version
string ProductInfo::GetVersion()   
{	
	string strVersion = "Version " +	ConvertIntToString(m_iMajorVersionNumber) + '.'+
		ConvertIntToString(m_iMinorVersionNumber) + '.'+
		ConvertIntToString(m_iRevisionVersionNumber) + 
		" Build " + ConvertIntToString(m_iBuildVersionNumber) ;
	
	return strVersion;
}


//get build type
string	ProductInfo::GetBuildType( )          
{
	if(PRODUCT_BUILD_TYPE_ISRELEASE )
	{
		m_strBuildType = PRODUCT_BUILD_TYPE_RELEASE ;
	}
	else
	{
		m_strBuildType = PRODUCT_BUILD_TYPE_DEBUD;
	}

	return m_strBuildType;
}


//get platform
string ProductInfo::GetPlatform()       
{
	return m_strPlatform;
	
}


//get technical support
string	ProductInfo::GetTechnicalSupport()        
{
	return m_strTechnicalSupport;
}


//get copyright
string	ProductInfo::GetCopyright()               
{
	string strCopyright =  m_strCopyrightInitialYear  +  '-' + m_strCopyrightFinalYear;
	return strCopyright;
}


//get build time
string	ProductInfo::GetBuildTime()           
{
	 return m_strBuildTime;
}
//get build revision
string ProductInfo::GetBuildRevision()
{
	return m_strBuildRevision;
}
//get builder
string ProductInfo::GetBuilder()              
{
	return m_strBuilder;
}


string ProductInfo::ConvertIntToString(int iNumber)
{
	char InString[20];		
	sprintf(InString,"%d",iNumber);	
	string str = InString;
	return  str;
}


string ProductInfo::GetProductBasicInfo()
{
	string strProductBasicInfo = ProductInfo::GetProductName()  + ", "+ ProductInfo::GetVersion()   + ", "+ 
		ProductInfo::GetBuildType()  + '(' + ProductInfo::GetPlatform()  + ')' ;
	return strProductBasicInfo;
}


string ProductInfo::GetBuildTimeAndBuilder()
{
	string strBuildTimeAndBuilder = "compiled " + ProductInfo::GetBuildTime()    + " by " +
		ProductInfo::GetBuilder() ;
	return strBuildTimeAndBuilder ;
}


string ProductInfo::GetProductAllInfo()
{
	string strProductInfoAll = ProductInfo::GetProductName()  + ", "+ ProductInfo::GetVersion()   +", "+ 
		ProductInfo::GetBuildType()  + '(' + ProductInfo::GetPlatform()  + ')'  + '\n' +   "Technical Support: " + 
		ProductInfo::GetTechnicalSupport()  +	'\n' + "Copyright(c) " + ProductInfo::GetCopyright()  + " by Polylink Holdings, Inc." +
		'\n'  + "compiled at " + ProductInfo::GetBuildTime()+ " by " + ProductInfo::GetBuilder() ;
	return strProductInfoAll ;  
}





