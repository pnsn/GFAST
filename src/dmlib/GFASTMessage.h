/*****************************************************************************
 *PNSN GFAST message header for dmlib interface
*****************************************************************************/
#ifndef __GFAST_MESSAGE_h
#define __GFAST_MESSAGE_h

#include "CoreEventInfo.h"
#include "gfast_struct.h"

/** A CoreEventInfo subclass for GFAST messages.
 *  @ingroup dm_lib
 */
class GFASTMessage : public CoreEventInfo
{

public:
  GFASTMessage();
  ~GFASTMessage();
  void setData(const coreInfo_struct *eventdat);
};

#endif

