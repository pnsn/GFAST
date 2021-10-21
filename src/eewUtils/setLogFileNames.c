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
void eewUtils_setLogFileNames(const char *eventid,
                              const char *outputDir,
                              char errorLogFileName[PATH_MAX],
                              char infoLogFileName[PATH_MAX],
                              char debugLogFileName[PATH_MAX],
                              char warnLogFileName[PATH_MAX])
{
  // Set the log file names
  char fileNameBase[PATH_MAX];
  memset(fileNameBase, 0, PATH_MAX*sizeof(char));
  if (outputDir == NULL) {
    strcpy(fileNameBase, "./\0");
  } else {
    strcpy(fileNameBase, outputDir);
    if (outputDir[strlen(outputDir)-1] != '/') {
      strcat(fileNameBase, "/\0");
    }
  }
  strcat(fileNameBase, eventid);

  memset(errorLogFileName, 0, PATH_MAX*sizeof(char));
  strcpy(errorLogFileName, fileNameBase);
  strcat(errorLogFileName, "_error.log\0");
  
  memset(infoLogFileName, 0, PATH_MAX*sizeof(char));
  strcpy(infoLogFileName, fileNameBase);
  strcat(infoLogFileName, "_info.log\0");
  
  memset(debugLogFileName, 0, PATH_MAX*sizeof(char));
  strcpy(debugLogFileName, fileNameBase);
  strcat(debugLogFileName, "_debug.log\0");
  
  memset(warnLogFileName, 0, PATH_MAX*sizeof(char));
  strcpy(warnLogFileName, fileNameBase);
  strcat(warnLogFileName, "_debug.log\0");
  
  return;
}
