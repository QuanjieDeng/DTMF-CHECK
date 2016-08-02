#ifndef __SDFDBTYPE_H__
#define __SDFDBTYPE_H__
#include <string.h>
//--------------------------------------
//DESCRIPTION: This file include all the data stytle 
//of the handle_database  needed. 
//--------------------------------------
#define    SQL_line   2648
#define    cscf_rd_reg_table  "pl_user_reg_status"
#define    cscf_rd_info_table "pl_user_info"
#define    cscf_rd_config_table   "cscf_rd_config"
#define    cscf_rd_RTPproxy_table "rtpproxy_config"

#define    cdr_record_table  		"tab_cdr_record"
#define    cdr_record_detail_table  "tab_cdr_record_detail"
#define    cdr_rd_rate_table           "tab_cdr_customer_rate"


#define    policy_msg_idlen      80

typedef enum
{
	CSCF_RD_co_fail = 0,
	CSCF_RD_co_success
} CSCF_RD_ty_retVal;



//-------------------------------------
//DTRUCTER:Sdf_st_mmCSCF_RDInfo
//DESCRIPTION:
//-------------------------------------
struct Sdf_st_mmCSCF_RDInfo
{
	char  			pTitle[32];    //info of  title
	char 		    pDomain[32];   //info of  domain
	char            pIp[20];       //info of  ip
	int             port;      //info of  port
	char            pByname[32];   //info of byname
	bool 		    bRegProxy;

	Sdf_st_mmCSCF_RDInfo()
	{
		memset(pTitle,0x00,32);
		memset(pDomain,0x00,32);
		memset(pIp,0x00,20);
		port = 0;
		bRegProxy = false;
		memset(pByname,0x00,32);
	}
	~Sdf_st_mmCSCF_RDInfo()
	{}
	
};


//-------------------------------------
//STRUCTER:Sdf_st_mmCSCF_RDInfo
//DESCRIPTION:
//-------------------------------------
struct Sdf_st_mmCSCF_RDInfo_PQ
{
	char 		public_id[32];
	char 		password[64];
	int  		line_num;
	Sdf_st_mmCSCF_RDInfo_PQ()
	{
		memset(public_id,0x00,32);
		memset(password,0x00,64);
		line_num = 0;
	}
	~Sdf_st_mmCSCF_RDInfo_PQ()
	{}
};


//-------------------------------------
//DTRUCTER:Sdf_st_mmCDR_RDInfo
//DESCRIPTION:
//-------------------------------------
struct Sdf_st_mmCDR_RDInfo
{
	char     StemFrom[policy_msg_idlen];
	char     From[policy_msg_idlen];
	char	   To[policy_msg_idlen];
	char     Identif_ID[policy_msg_idlen];
	char     Devicename[policy_msg_idlen];
	char     Call_type[policy_msg_idlen];
	char     StateCode[policy_msg_idlen];
	char        Msg_type[policy_msg_idlen];
	int        business_type;
	int        PolicyID;
	unsigned int  seqno;
	Sdf_st_mmCDR_RDInfo()
	{
		memset(From,0x00, policy_msg_idlen);
		memset(To,0x00, policy_msg_idlen);
		memset(Identif_ID,0x00,policy_msg_idlen);
		memset(Devicename,0x00,policy_msg_idlen);
		memset(Call_type,0x00,policy_msg_idlen);
		memset(StateCode,0x00,policy_msg_idlen);
		memset(StemFrom,0x00,policy_msg_idlen);
		memset(Msg_type,0x00,policy_msg_idlen);
		business_type = 0;
		PolicyID = 1;
		seqno = 0;
	}
	~Sdf_st_mmCDR_RDInfo()
	{}
	
};


#endif

























