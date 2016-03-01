

#include "SimpleDebugger.h"

#include <math.h>

#include "oraStringTools.h"
#include "oraMultiOutputStreamBuffer.h"

// Forward declarations
#include <iostream>
#include <fstream>


// define the static members (after declaration):
std::ostream *SimpleDebugger::DEBUG_STREAM = NULL;
bool SimpleDebugger::DEBUGGING_ACTIVE = false;
int SimpleDebugger::MESSAGE_LEVEL_THRESHOLD = ML_DEFAULT;
bool SimpleDebugger::PREPEND_TIMESTAMP = false;
bool SimpleDebugger::PREPEND_MESSAGE_LEVEL = false;
std::ofstream *SimpleDebugger::OUTFILE_STREAM = NULL;
std::streambuf *SimpleDebugger::ORIGINAL_CERR_BUF = NULL;
std::ostream *SimpleDebugger::OUT_CONSOLE_STREAM = NULL;
bool SimpleDebugger::FLUSH_ON_MESSAGE = true;


bool
SimpleDebugger
::MakeGlobalDebugOutput(std::string baseLogFileName,
    bool logFileAppendMode, bool consoleOutput, bool redirectCerrToOutput)
{
  // internally we use ora::merr and ora::merrbuf respectively
  ora::merrbuf.ClearMultipleOutputs(); // remove current ports
  if (OUT_CONSOLE_STREAM)
  {
    delete OUT_CONSOLE_STREAM;
    OUT_CONSOLE_STREAM = NULL;
  }
  DEBUG_STREAM = NULL;

  bool success = true;

  if (redirectCerrToOutput)
  {
    ORIGINAL_CERR_BUF = std::cerr.rdbuf(); // store
    std::cerr.rdbuf(ora::merr.rdbuf()); // redirect
  }
  else
  {
    ORIGINAL_CERR_BUF = NULL;
  }

  if (consoleOutput)
  {
    if (ORIGINAL_CERR_BUF) // output to original std::cerr
    {
      OUT_CONSOLE_STREAM = new std::ostream(ORIGINAL_CERR_BUF);
      ora::merrbuf.AddMultipleOutput(OUT_CONSOLE_STREAM);
    }
    else // simply output to std::cerr
    {
      ora::merrbuf.AddMultipleOutput(&std::cerr);
    }
  }

  if (success && baseLogFileName.length() > 0) // log file output requested
  {
    if (OUTFILE_STREAM) // clean up needed
    {
      if (OUTFILE_STREAM->is_open())
        OUTFILE_STREAM->close();
      delete OUTFILE_STREAM;
      OUTFILE_STREAM = NULL;
    }

    // pre-process file name: support {$DATE} and {$TIME}
    std::string dateString;
    ora::CurrentORADateString(dateString);
    std::string timeString;
    ora::CurrentORATimeString(timeString);
    ora::ReplaceString(baseLogFileName, "{$DATE}", dateString);
    ora::ReplaceString(baseLogFileName, "{$TIME}", timeString);

    // verify that the file path exists - create it if it does not exist
    std::string path = itksys::SystemTools::GetFilenamePath(baseLogFileName);
    if (ora::TrimF(path).length() > 0 &&
        !itksys::SystemTools::FileExists(path.c_str(), false))
      success = success && itksys::SystemTools::MakeDirectory(path.c_str());

    if (success)
    {
      OUTFILE_STREAM = new std::ofstream();
      if (logFileAppendMode) // append mode
        OUTFILE_STREAM->open(baseLogFileName.c_str(), std::ios::app);
      else // override mode
        OUTFILE_STREAM->open(baseLogFileName.c_str(), std::ios::out);

      if (!OUTFILE_STREAM->is_open())
      {
        success = false;
        delete OUTFILE_STREAM;
        OUTFILE_STREAM = NULL;
      }

      if (success)
        ora::merrbuf.AddMultipleOutput(OUTFILE_STREAM);
    }
  }

  if (success)
    DEBUG_STREAM = &ora::merr; // set global output stream

  return success;
}

void
SimpleDebugger
::ReleaseGlobalDebugOutput()
{
  if (OUTFILE_STREAM) // release file stream
  {
    OUTFILE_STREAM->close();
    delete OUTFILE_STREAM;
    OUTFILE_STREAM = NULL;
  }

  if (ORIGINAL_CERR_BUF)
  {
    std::cerr.rdbuf(ORIGINAL_CERR_BUF); // restore
    ORIGINAL_CERR_BUF = NULL;
    if (OUT_CONSOLE_STREAM)
    {
      delete OUT_CONSOLE_STREAM;
      OUT_CONSOLE_STREAM = NULL;
    }
  }

  DEBUG_STREAM = NULL;
  ora::merrbuf.ClearMultipleOutputs();
}


SimpleDebugger
::SimpleDebugger()
{
  m_Clock = itk::RealTimeClock::New();
}

SimpleDebugger
::~SimpleDebugger()
{
  m_Clock = NULL;
}

void
SimpleDebugger
::Debug(const std::string msg, const int level)
{
  if (level < MESSAGE_LEVEL_THRESHOLD)
    return;

  // main stream
  if (DEBUG_STREAM && DEBUGGING_ACTIVE)
  {
    if (PREPEND_TIMESTAMP)
    {
      std::string s;
      double ts = m_Clock->GetTimeStamp();
      ora::CurrentORADateTimeString(s); // YYYY-MM-DD HH-NN-SS
      int msec = (int)((ts -  floor(ts)) * 1000);
      char msecbuff[4];
      sprintf(msecbuff, "%03d", msec);
      (*DEBUG_STREAM) << s << "." << msecbuff << ": ";
    }

    if (PREPEND_MESSAGE_LEVEL)
      (*DEBUG_STREAM) << "[L=" << level << "] ";

    (*DEBUG_STREAM) << msg << std::endl;

    if (FLUSH_ON_MESSAGE) // force flush
      DEBUG_STREAM->flush();
  }
}

void
SimpleDebugger
::Error(const std::string msg)
{
  // main stream
  if (DEBUG_STREAM && DEBUGGING_ACTIVE)
  {
    if (PREPEND_TIMESTAMP)
    {
      std::string s;
      double ts = m_Clock->GetTimeStamp();
      ora::CurrentORADateTimeString(s); // YYYY-MM-DD HH-NN-SS
      int msec = (int)((ts -  floor(ts)) * 1000);
      char msecbuff[4];
      sprintf(msecbuff, "%03d", msec);
      (*DEBUG_STREAM) << s << "." << msecbuff << ": ";
    }

    (*DEBUG_STREAM) << "[ERROR] " << msg << std::endl;

    if (FLUSH_ON_MESSAGE) // force flush
      DEBUG_STREAM->flush();
  }
}

