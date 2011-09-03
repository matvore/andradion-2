#include "stdafx.h"
#include "logger.h"

#if defined(_DEBUG)

using namespace std;

// this logger is based on code seen in a gamedev.net
//  article
static NGameLib2::tstring filename;
static bool enabled = true;

void BeginLog(const char *filename_)
{
  // this function simply empties out whatever is
  //  already in filename_, if it exists.
  // If it does not exist, it creates an empty file of
  //  that name
		
  // clear the file contents
  FILE *pFile;
  if(NULL != (pFile = fopen(filename_, "wb")))
    {
      // close it up and return success
      fclose(pFile);
      // copy filename
      filename = filename_;
    }
  else
    {
      // disable log
      filename.clear();
    }
}

void WriteLog(const char *lpszText,...)
{
  // only do this if logging is enabled
  if (0 != filename.c_str() && true == enabled)
    {
      va_list argList;
      FILE *pFile;

      // initialize variable argument list
      va_start(argList, lpszText);

      // open the log file for append
      if (NULL != (pFile = fopen(filename.c_str(), "a+")))
	{
	  // write the text and a newline
	  vfprintf(pFile, lpszText, argList);
	  putc('\n', pFile);

	  // close the file
	  fclose(pFile);
	  va_end(argList);
	}
    }
}

bool LogEnabled()
{
  return enabled;
}

void EnableLog(bool on_or_off)
{
  enabled = on_or_off;
}

#endif
