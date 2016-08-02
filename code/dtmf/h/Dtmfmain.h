////////////////////////////////////////////////////////////////////////////////
// File name    : sdfmmccsampleappmain.h                                      
//                                                                            
// Description  :
// 	This file contains definitions required by the b2bserver application.
//
//  Date         Name            Reference               Description           
// ---------    -----------     -------------           ----------------      
// 27 Jan, 2006  Shwetha
//                                                                            
//      Copyright (C) 2006 Aricent Inc . All Rights Reserved
////////////////////////////////////////////////////////////////////////////////

#ifndef __CDR_SERVER_SAMPLEAPP_H__
#define __CDR_SERVER_SAMPLEAPP_H__

#include "common.h"


typedef struct Sdf_tg_serverOptions
{
	Sdf_ty_bool dboption;           //DBIPointer
	Sdf_ty_bool helpMode;			// Set if the user sepcified the help option
	Sdf_ty_bool	daemonMode;			// Set if the daemon mode is specified
	Sdf_ty_bool traceErrEnabled;
	Sdf_ty_bool serverRunning;
	char		*pConfigFilePath;	// Path to the XML config file
	char		*pLogDirectory;		// Directory where logs will be stored
	Sdf_ty_U16bit	retries;
	char		*passwordDigest;
	Sdf_ty_bool		versionMode;
	Sdf_tg_serverOptions(void)
	{
		pConfigFilePath = Sdf_co_null;
		pLogDirectory = Sdf_co_null;
		passwordDigest = Sdf_co_null;
		// B2B 8.0 klocwork fix - Srikanth
		dboption = Sdf_co_false;
		helpMode = Sdf_co_false;
		// B2B 8.0 klocwork fix - Srikanth
		daemonMode = Sdf_co_false;
		traceErrEnabled = Sdf_co_false;
		serverRunning = Sdf_co_false;
		retries = 0;
		versionMode = Sdf_co_false;
	}
	~Sdf_tg_serverOptions(void)
	{
		if (Sdf_co_null != pConfigFilePath)
			delete[] pConfigFilePath;
		if (Sdf_co_null != pLogDirectory )
			delete[] pLogDirectory ;
		if(passwordDigest != Sdf_co_null)
			delete[] passwordDigest;
	}
	Sdf_tg_serverOptions(Sdf_tg_serverOptions& src)
	{
		// B2B 8.0 klocwork fix - Srikanth
		dboption = src.dboption;
		helpMode = src.helpMode;
		// B2B 8.0 klocwork fix - Srikanth
		daemonMode = src.daemonMode;
		traceErrEnabled = src.traceErrEnabled;
		serverRunning = src.serverRunning;
		retries = src.retries;

		if(src.pConfigFilePath != Sdf_co_null)
			pConfigFilePath = src.pConfigFilePath;
		else
			pConfigFilePath = Sdf_co_null;
		
		if(src.pLogDirectory != Sdf_co_null)
			pLogDirectory = src.pLogDirectory;
		else
			pLogDirectory = Sdf_co_null;
		
		if(src.passwordDigest != Sdf_co_null)
			passwordDigest = src.passwordDigest;
		else
			passwordDigest = Sdf_co_null;
		
	}
} Sdf_st_serverOptions;


void getConfPath(char* des, char* pConfigPath);

////////////////////////////////////////////////////////////////////////////////
// Function: 
//      Sdfapp_fn_processArgs
// Description:
//      Processes the cmd line arguments after parsing them.
////////////////////////////////////////////////////////////////////////////////
Sdf_ty_U8bit Sdfapp_fn_processArgs(Sdf_ty_S32bit argc, \
        Sdf_ty_S8bit* argv[], Sdf_st_serverOptions* pOptions);
////////////////////////////////////////////////////////////////////////////////
// Funtion :
//      Sdfapp_fn_parseCommandLineOptions
// Description"
//      Internal function used to parse the command line options
//      given by the user.
// Parameters:
// 		argc - number of arguments
//		argv - array of argument strings
//		configOptions - returns the configuration options parsed from
//			the command-line
// Return value:
//		Sdf_co_success - parsed command line options correctly.
//		Sdf_co_fail - command line option parsing failed.
//
////////////////////////////////////////////////////////////////////////////////
Sdf_ty_retVal Sdfapp_fn_parseCommandLineOptions ( Sdf_ty_S32bit argc,\
	Sdf_ty_S8bit *argv[], Sdf_st_serverOptions *pOptions);

////////////////////////////////////////////////////////////////////////////////
// Function: Sdfapp_fn_getLineFromConsole
// 
// Description:
//	This funtion gets a line from the console. 
////////////////////////////////////////////////////////////////////////////////
void Sdfapp_fn_getLineFromConsole (Sdf_ty_S8bit *pInput, Sdf_ty_S32bit limit);

////////////////////////////////////////////////////////////////////////////////
// Function: 
//      Sdfapp_fn_stopServer 
// Description:
//		Function to Shut-down b2b components inc. test components
////////////////////////////////////////////////////////////////////////////////
Sdf_ty_retVal Sdfapp_fn_stopServer ();
Sdf_ty_retVal logInit(Sdf_st_serverOptions *options);


#endif /* Header multiple include protection */

