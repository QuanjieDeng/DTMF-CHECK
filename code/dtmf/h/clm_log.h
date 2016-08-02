#ifndef _CLM_LOG_H
#define _CLM_LOG_H

#include <stdio.h>

#define __CLM_FATAL     		0x02
#define __CLM_ERROR     		0x04
#define __CLM_WARNING   		0x08
#define __CLM_INFO      		0x10
#define __CLM_DEBUG     		0x20

#define __CLM_ADAPT_MODE		0x01
#define __CLM_LINE_MODE			0x02
#define __CLM_COMSOLE_MODE		0x04
#define __CLM_FILE_MODE			0x08
#define __CLM_SYNC_MODE			0x10



#define _ALL_ 					__FILE__, __LINE__

#define CLM_FATAL               __CLM_FATAL,_ALL_
#define CLM_ERROR               __CLM_ERROR,_ALL_
#define CLM_WARNING     		__CLM_WARNING,_ALL_
#define CLM_INFO                __CLM_INFO,_ALL_
#define CLM_DEBUG               __CLM_DEBUG,_ALL_

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/
	
int clm_init_logger(const char *pStrLogPath, const char *pStrFilename, int nLevel, int nSize, int nNum);
int clm_set_level(unsigned int nLevel);
int clm_get_level();
int clm_set_size(unsigned int nSize);
int clm_get_size();
int clm_set_mode(unsigned int nMode);
int clm_get_mode();
int clm_set_number(unsigned int nNum);
int clm_get_number();
/* be carefull,please remember to free the return char* */
char* clm_get_path(char* pStrPath ,int nLen);

/*clm_log(CLM_INFO, "%s", "hello world")*/
int clm_log(int level, const char *file, int line, const char *fmt, ...)
		__attribute__ ((format (printf, 4, 5)));
//synchronous log
int clm_sync_log(int level, const char *file, int line, const char *fmt, ...) 
		__attribute__ ((format (printf, 4, 5)));
void clm_clean_logger();

#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /*_CLM_LOG_H */
