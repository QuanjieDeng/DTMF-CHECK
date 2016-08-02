#include "sdfmessagequeue.h"
#include "sdfthreadpool.h"
#include "common.h"
#include "framework.h"
#include "Dtmf.h"
#include "XML.h"
#include "Dtmf_MessageStack.h"
#include "clm_log.h"
#include "Dtmfmain.h"
#include "ProductInfo.h"
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/resource.h> 


// AppCallMessageStack* AppCallMessageStack::instance = NULL;
Dtmf_ProxyMessageStack* Dtmf_ProxyMessageStack::instance = NULL;

//Modify  by Quanjie.Deng 
Sdf_ty_retVal logInit(Sdf_st_serverOptions *options)
{
	char logFileName[100]="";

	sprintf(logFileName, "DTMFServer_%d", getpid());
	int nRet = -1;
	if( Sdf_co_null == options->pLogDirectory )
	{
		char file[150] = {0};
		char *szTemp;
		if((szTemp = getenv("DTMFServer_ROOT")) == NULL)
		{
			clm_sync_log(CLM_ERROR, "get environment variable DTMFServer_ROOT error");
			sprintf(file,"%s", "./log/");
		}
		else
		{
			clm_sync_log(CLM_INFO, "DTMFServer_ROOT: %s", szTemp);
			sprintf(file, "%s/log/", szTemp);
		}

		delete szTemp;

		nRet=clm_init_logger(file,logFileName,__CLM_FATAL|__CLM_WARNING|__CLM_DEBUG,1,2);
	}
	else
	{
		nRet=clm_init_logger(options->pLogDirectory,logFileName,__CLM_FATAL|__CLM_WARNING|__CLM_DEBUG,1,2);
	}
	if(0 > nRet)
	{
		clm_log(CLM_INFO, "logInit error");
		return  Sdf_co_fail;   
	}

	//init log info
	string log_level = "";
	CXML::getInstance()->getFirstElementValue("loglevel", log_level);
	string log_size;
	CXML::getInstance()->getNextElementValue("logsize", log_size);
	string log_num;
	CXML::getInstance()->getNextElementValue("lognum", log_num);
	//init log info
	char size[15]={0};
	char numb[15]={0};
	char level[15]={0};
	sprintf(size,"%s",log_size.c_str());
	sprintf(numb,"%s",log_num.c_str());
	sprintf(level,"%s",log_level.c_str());
	int nlevel = __CLM_FATAL; 
	if(!strcasecmp(level,"FATAL"))
		nlevel = __CLM_FATAL;
	else if(!strcasecmp(level,"ERROR"))
		nlevel = __CLM_ERROR;
	else if(!strcasecmp(level,"WARNING"))
		nlevel = __CLM_WARNING;
	else if(!strcasecmp(level,"INFO"))
		nlevel = __CLM_INFO;
	else if(!strcasecmp(level,"DEBUG"))
		nlevel = __CLM_DEBUG;
	clm_set_level(nlevel);
	clm_set_size(atoi(size));
	clm_set_number(atoi(numb));
	int mode =clm_get_mode();
	clm_log(CLM_INFO,"mode:%d",mode);
	mode = (mode&~__CLM_COMSOLE_MODE)|__CLM_LINE_MODE;
	clm_set_mode(mode);
	
	return Sdf_co_success;
}

//Modify  by Quanjie.Deng 

Sdf_ty_retVal DTMF_Init()
{
       // printf("enty DTMF_Init()!\n");
	Sdf_cl_commonMessageQueue *pDTMFMsgQueue = new Sdf_cl_commonMessageQueue();
	if ( NULL == pDTMFMsgQueue )
	{
		return Sdf_co_fail;
	}
	DTMFMessageManager *pDTMFMsgManager = new DTMFMessageManager(pDTMFMsgQueue);
	if ( NULL == pDTMFMsgManager )
	{
		return Sdf_co_fail;
	}
	if ( Sdf_co_success != pDTMFMsgManager->init() )
	{
		printf("DTMFCheck initialization failed!\n");
		return Sdf_co_fail;
	}
	framework::getInstance().registerComponent(COMPONENT_DTMF, pDTMFMsgQueue);
    printf("DTMFCheck initialization succed!\n");
	return Sdf_co_success;		
}

Sdf_ty_S32bit main(int argc, char *argv[])
{
	struct rlimit l;
	memset(&l, 0, sizeof(l));
	l.rlim_cur = RLIM_INFINITY;
	l.rlim_max = RLIM_INFINITY;
	if (setrlimit(RLIMIT_CORE, &l)) 
	{
		printf("Unable to disable core size resource limit\n");
	}
        Sdf_st_serverOptions *options= new Sdf_st_serverOptions;
	int    modey = Sdfapp_fn_processArgs(argc, argv, options);
	int    nRet=-1;
	CXML::getInstance()->ParseXmlFile(options->pConfigFilePath); 	
	if(Sdf_co_fail == logInit(options))
	{
		return Sdf_co_fail;
	}
	if(Sdf_co_success != DTMF_Init())
	{
		return   -1;
	}
	for (;;)
	{
		sleep(1);
	}

	return 0;
}


void getConfPath(char* des, char* pConfigPath)
{
	char temp[200] = "";
	
	char* pBegin = NULL;
	char* pEnd   = NULL;
	char* p      = NULL;
	pBegin = pConfigPath;
	p      = temp;
	while(1)
	{
		pEnd = strstr(pBegin, "/");
		if ( pEnd != NULL )
		{
			strncpy(p, pBegin, pEnd - pBegin + 1);
			p += pEnd - pBegin + 1;
			pBegin = pEnd + 1;
			pEnd   = NULL;
		}
		else
		{
		    break;
		}
	}
	strcpy(des, temp);
}

////////////////////////////////////////////////////////////////////////////////
// Function: Sdfapp_fn_getLineFromConsole
// 
// Description:
//	This funtion gets a line from the console. 
////////////////////////////////////////////////////////////////////////////////
void Sdfapp_fn_getLineFromConsole (Sdf_ty_S8bit *pInput, Sdf_ty_S32bit limit)
{
	Sdf_ty_S8bit c;
	Sdf_ty_S32bit i=0;
	while(limit-- > 1)
	{
		c=getchar();
		if (c=='\n')
		{
			pInput[i++] = '\0';
			return;
		}
		else
			pInput[i++] = c;
	}
	pInput[i]='\0';
}

////////////////////////////////////////////////////////////////////////////////
// Function: 
//      Sdfapp_fn_processArgs
// Description:
//      Processes the cmd line arguments after parsing them.
////////////////////////////////////////////////////////////////////////////////
Sdf_ty_U8bit Sdfapp_fn_processArgs(\
        Sdf_ty_S32bit argc, Sdf_ty_S8bit* argv[], Sdf_st_serverOptions* pOptions)
{
        Sdf_ty_S8bit *helpString = (Sdf_ty_S8bit *)\
		"--------------------------------------------------------------------\n"
		"DTMFServer Server usage:\n"
		"    DTMFServer  -c <config_file> [-t [<trace_dir>]] [-d]\n"
		"    DTMFServer -h\n"
		"    DTMFServer -v\n"
		"\n"
		"PARAMETERS\n"
		"    -c     Specifies the location of the configuration file.\n"
		"           The DTMFServer will not start if this is unspecified or if\n"
		"           the configuration file cannot be read. The absolute\n"
		"           path of the file should be used. Relative path is not\n"
		"           accepted.\n"
		"    -t     Enables trace and error logging. \n"
		"           Specifies the directory of the trace and error files.\n"
		"           The absolute path of the file should be used. Relative\n"
		"           path is not accepted. Names of the log files are:\n"
		"           Traces go into a file named \"log\"\n"
		"           and errors go into a file named - \"error\"\n"
		"           in the directory specified.\n"
		"           If the directory is unspecified, logs will be \n"
		"           displayed on the console.\n"
		"    -d     Run the server in daemon mode. The server will detach.\n"
		"    -v     Show the version and exit.\n"
		"    -h     Print this help message and exit.\n"
		"-------------------------------------------------------------------\n";

	// Parse command line options
	if (Sdf_co_fail == Sdfapp_fn_parseCommandLineOptions(argc, argv, pOptions))
	{
		std::cout << helpString;
		exit(1);
	}
	if (Sdf_co_true == pOptions->helpMode)
	{
		std::cout << helpString;
		exit(0);
	}
	
	
	if (Sdf_co_true == pOptions->versionMode)
	{
		cout <<endl <<  ProductInfo::Instance()->GetProductAllInfo() << endl << endl;
		exit(0);
	}
	if ( Sdf_co_null == pOptions->pConfigFilePath)
	{
		std::cout << "Configuration file not specified" << std::endl;
		std::cout << helpString;
		exit(1);
	}
	cout <<endl <<  ProductInfo::Instance()->GetProductAllInfo() << endl << endl;
	
       return 2;
}

////////////////////////////////////////////////////////////////////////////////
// Funtion :
//      Sdfapp_fn_parseCommandLineOptions
// Description:
//      This is an internal function which is used to parse the
//      command line options given by the user.
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
Sdf_ty_retVal Sdfapp_fn_parseCommandLineOptions\
(
	Sdf_ty_S32bit argc,\
	Sdf_ty_S8bit *argv[],\
	Sdf_st_serverOptions *pOptions\
)
{
	// Command line options accepted are
	// -c <file> : Use <file> as configuration file
	// -h : help

	pOptions->helpMode = Sdf_co_false;
	pOptions->traceErrEnabled = Sdf_co_false;
	pOptions->pConfigFilePath = Sdf_co_null;
	pOptions->pLogDirectory = Sdf_co_null;
	
	// Check for arguments
	if (argc <= 1)
	{
		std::cout << "########################################" << std::endl;
		std::cout << "Command line parameters missing." << std::endl;
		std::cout << "########################################" << std::endl;
		return Sdf_co_fail;
	}
	// Scan through arguments
	// state : 0 - accept option like  -h, -c
	// state : 1 - got -c and expecting filename
	// state : 2 - got -t and expecting directory name
	Sdf_ty_S32bit state = 0;

	for (Sdf_ty_S32bit i=1; i<argc; i++)
	{
		if (2 == state)
		{
			if ('-' == argv[i][0])	
			{
				// Another option.
				state = 0;
			}
		}

		if (0 == state)
		{
			if (strcmp(argv[i],"-v") == 0)
			{
				pOptions->versionMode = Sdf_co_true;
				return Sdf_co_success;
			}
			if (strcmp(argv[i],"-V") == 0)
			{
				pOptions->versionMode = Sdf_co_true;
				return Sdf_co_success;
			}

			if (strcmp(argv[i],"-d") == 0)
			{
				pOptions->daemonMode = Sdf_co_true;
			}	
			else if (strcmp(argv[i],"-c") == 0)
			{
				state = 1;
			}
			else if (strcmp(argv[i],"-h") == 0)
			{
				pOptions->helpMode = Sdf_co_true;
				return Sdf_co_success;
			}
			else if (strcmp(argv[i],"-t") == 0)
			{
				pOptions->traceErrEnabled = Sdf_co_true;
				state = 2;
			}
			else
			{
				std::cout << "########################################" \
					<< std::endl;
				std::cout << "Invalid option " << argv[i] << std::endl;
				std::cout << "########################################" \
					<< std::endl;
				return Sdf_co_fail;
			}
		}
		else
		{
			// Configuration file path expected here
			if ('-' == argv[i][0])	
			{
				// Another option
				std::cout << "########################################" \
					<< std::endl;
				std::cout << "Configuration file path expected "\
					"after -c." << std::endl;
				std::cout << "########################################" \
					<< std::endl;
				return Sdf_co_fail;
			}
			if (state == 1)
			{
// SPR 8670 Start
				if(Sdf_co_null != pOptions->pConfigFilePath)
					delete[] pOptions->pConfigFilePath;
// SPR 8670 End
				pOptions->pConfigFilePath = argv[i];
			}
			else if (state == 2)
				pOptions->pLogDirectory = argv[i];
			state = 0;
		}
	}
	if ((0 == state) || (2 == state))
		return Sdf_co_success;
	else
		return Sdf_co_fail;
}

