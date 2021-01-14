#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfast_eewUtils.h"

/*!
 * @brief Creates event output log file names
 * @param [in] eventid event id character string
 * @param [in] outputdir directory for output
 * @param [out] errorLogFileName
 * @param [out] infoLogFileName
 * @param [out] debugLogFileName
 * @param [out] warnLogFileName
 */
void eewUtils_setLogFileNames(const char *eventid,const char *outputDir,
			      char errorLogFileName[PATH_MAX],
			      char infoLogFileName[PATH_MAX],
			      char debugLogFileName[PATH_MAX],
                              char warnLogFileName[PATH_MAX])
{
  // Set the log file names
  memset(errorLogFileName, 0, PATH_MAX*sizeof(char));
  if (outputDir == NULL) {
    strcpy(errorLogFileName, "./\0");
  }
  else {
    strcpy(errorLogFileName, outputDir);
    if (outputDir[strlen(outputDir)-1] != '/') {
      strcat(errorLogFileName, "/\0");
    }
  }
  strcat(errorLogFileName, eventid);
  strcat(errorLogFileName, "_error.log\0");
  
  memset(infoLogFileName, 0, PATH_MAX*sizeof(char));
  strcpy(infoLogFileName, errorLogFileName);  // other files start with same root
  strcat(infoLogFileName, "_info.log\0");
  
  memset(debugLogFileName, 0, PATH_MAX*sizeof(char));
  strcpy(debugLogFileName, errorLogFileName);
  strcat(debugLogFileName, "_debug.log\0");
  
  memset(warnLogFileName, 0, PATH_MAX*sizeof(char));
  strcpy(warnLogFileName, errorLogFileName);
  strcat(warnLogFileName, "_debug.log\0");
  
  return;
}
