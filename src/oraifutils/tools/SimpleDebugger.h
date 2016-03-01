
#ifndef SIMPLEDEBUGGER_H_
#define SIMPLEDEBUGGER_H_

#include <itkRealTimeClock.h>
#include <itksys/SystemTools.hxx>

// Forward declarations
#include <iosfwd>

/**
 * A slight modification of the standard __FILE__ macro:
 * the source file names are printed without path information.
 */
#define __SHORTFILE__ \
 itksys::SystemTools::GetFilenameName(__FILE__)

/** General source file position information macro in streaming format. **/
#define __CURRENT_POSITON_STREAM__ \
  "[[" << __SHORTFILE__ << " in " << __FUNCTION__ << \
  " @ line " << __LINE__ << "]]"

/**
 * Simple error macro for stream-based message output (relates to 'this'
 * pointer). In contrast to simply calling the Error()-method, this macro
 * provides file, function and line information where the error occurred.
 */
#define SimpleErrorMacro(message) \
{ \
  std::ostringstream os;\
  os message << __CURRENT_POSITON_STREAM__; \
  this->Error(os.str().c_str()); \
}

/**
 * Simple error macro for stream-based message output (relates to the
 * specified object!). In contrast to simply calling the Error()-method,
 * this macro provides file, function and line information where the error
 * occurred. NOTE: not for object pointer, for objects!
 */
#define SimpleErrorMacro2Object(object, message) \
{ \
  std::ostringstream os; \
  os message << __CURRENT_POSITON_STREAM__; \
  (&object)->Error(os.str().c_str()); \
}

/**
 * Simple error macro for stream-based message output (relates to the
 * specified object!). In contrast to simply calling the Error()-method,
 * this macro provides file, function and line information where the error
 * occurred. NOTE: not for objects, for object pointers!
 */
#define SimpleErrorMacro2ObjectPointer(objectPointer, message) \
{ \
  std::ostringstream os; \
  os message << __CURRENT_POSITON_STREAM__; \
  objectPointer->Error(os.str().c_str()); \
}


/**
 * Simple debug macro for stream-based message output (relates to 'this'
 * pointer). This macro version assumes a message level of 0 (ML_DEFAULT).
 */
#define SimpleDebugMacro(message) \
{ \
  std::ostringstream os; \
  os message; \
  this->Debug(os.str().c_str(), SimpleDebugger::ML_DEFAULT); \
}

/**
 * Simple debug macro for stream-based message output (relates to a specified
 * object). This macro version assumes a message level of 0 (ML_DEFAULT).
 * NOTE: not for object pointer, for objects!
 */
#define SimpleDebugMacro2Object(object, message) \
{ \
  std::ostringstream os; \
  os message; \
  (&object)->Debug(os.str().c_str(), SimpleDebugger::ML_DEFAULT); \
}

/**
 * Simple debug macro for stream-based message output (relates to a specified
 * object pointer). This macro version assumes a message level of 0 (ML_DEFAULT).
 * NOTE: not for objects, for object pointers!
 */
#define SimpleDebugMacro2ObjectPointer(objectPointer, message) \
{ \
  std::ostringstream os; \
  os message; \
  objectPointer->Debug(os.str().c_str(), SimpleDebugger::ML_DEFAULT); \
}

/**
 * Debug macro for stream-based message output (relates to 'this'
 * pointer). This macro version considers the specified message level.
 */
#define DebugMacro(message, level) \
{ \
  std::ostringstream os; \
  os message; \
  this->Debug(os.str().c_str(), level); \
}

/**
 * Debug macro for stream-based message output (relates to a specified
 * object). This macro version considers the specified message level.
 * NOTE: not for object pointer, for objects!
 */
#define DebugMacro2Object(object, message, level) \
{ \
  std::ostringstream os; \
  os message; \
  (&object)->Debug(os.str().c_str(), level); \
}

/**
 * Debug macro for stream-based message output (relates to a specified
 * object pointer). This macro version considers the specified message level.
 * NOTE: not for objects, for object pointers!
 */
#define DebugMacro2ObjectPointer(objectPointer, message, level) \
{ \
  std::ostringstream os; \
  os message; \
  objectPointer->Debug(os.str().c_str(), level); \
}

/**
 * Debug macro for stream-based message output (relates to 'this'
 * pointer) on VERY DETAILED level.
 */
#define VeryDetailedDebugMacro(message) \
{ \
  DebugMacro(message, SimpleDebugger::ML_VERY_DETAILED); \
}

/**
 * Debug macro for stream-based message output (relates to a specified
 * object) on VERY DETAILED level.
 */
#define VeryDetailedDebugMacro2Object(object, message) \
{ \
  DebugMacro2Object(object, message, SimpleDebugger::ML_VERY_DETAILED); \
}

/**
 * Debug macro for stream-based message output (relates to a specified
 * object pointer) on VERY DETAILED level.
 */
#define VeryDetailedDebugMacro2ObjectPointer(objectPointer, message) \
{ \
  DebugMacro2ObjectPointer(objectPointer, message, \
    SimpleDebugger::ML_VERY_DETAILED); \
}

/**
 * Debug macro for stream-based message output (relates to 'this'
 * pointer) on DETAILED level.
 */
#define DetailedDebugMacro(message) \
{ \
  DebugMacro(message, SimpleDebugger::ML_DETAILED); \
}

/**
 * Debug macro for stream-based message output (relates to a specified
 * object) on DETAILED level.
 */
#define DetailedDebugMacro2Object(object, message) \
{ \
  DebugMacro2Object(object, message, SimpleDebugger::ML_DETAILED); \
}

/**
 * Debug macro for stream-based message output (relates to a specified
 * object pointer) on DETAILED level.
 */
#define DetailedDebugMacro2ObjectPointer(objectPointer, message) \
{ \
  DebugMacro2ObjectPointer(objectPointer, message, \
    SimpleDebugger::ML_DETAILED); \
}

/**
 * Debug macro for stream-based message output (relates to 'this'
 * pointer) on DEFAULT level.
 */
#define DefaultDebugMacro(message) \
{ \
  DebugMacro(message, SimpleDebugger::ML_DEFAULT); \
}

/**
 * Debug macro for stream-based message output (relates to a specified
 * object) on DEFAULT level.
 */
#define DefaultDebugMacro2Object(object, message) \
{ \
  DebugMacro2Object(object, message, SimpleDebugger::ML_DEFAULT); \
}

/**
 * Debug macro for stream-based message output (relates to a specified
 * object pointer) on DEFAULT level.
 */
#define DefaultDebugMacro2ObjectPointer(objectPointer, message) \
{ \
  DebugMacro2ObjectPointer(objectPointer, message, \
      SimpleDebugger::ML_DEFAULT); \
}

/**
 * Debug macro for stream-based message output (relates to 'this'
 * pointer) on INFORMATIVE level.
 */
#define InformativeDebugMacro(message) \
{ \
  DebugMacro(message, SimpleDebugger::ML_INFORMATIVE); \
}

/**
 * Debug macro for stream-based message output (relates to a specified
 * object) on INFORMATIVE level.
 */
#define InformativeDebugMacro2Object(object, message) \
{ \
  DebugMacro2Object(object, message, SimpleDebugger::ML_INFORMATIVE); \
}

/**
 * Debug macro for stream-based message output (relates to a specified
 * object pointer) on INFORMATIVE level.
 */
#define InformativeDebugMacro2ObjectPointer(objectPointer, message) \
{ \
  DebugMacro2ObjectPointer(objectPointer, message, \
      SimpleDebugger::ML_INFORMATIVE); \
}

/**
 * Debug macro for stream-based message output (relates to 'this'
 * pointer) on SKETCHY level.
 */
#define SketchyDebugMacro(message) \
{ \
  DebugMacro(message, SimpleDebugger::ML_SKETCHY); \
}

/**
 * Debug macro for stream-based message output (relates to a specified
 * object) on SKETCHY level.
 */
#define SketchyDebugMacro2Object(object, message) \
{ \
  DebugMacro2Object(object, message, SimpleDebugger::ML_SKETCHY); \
}

/**
 * Debug macro for stream-based message output (relates to a specified
 * object pointer) on SKETCHY level.
 */
#define SketchyDebugMacro2ObjectPointer(objectPointer, message) \
{ \
  DebugMacro2ObjectPointer(objectPointer, message, \
      SimpleDebugger::ML_SKETCHY); \
}

/**
 * A class for simple debug support. E.g. debug messages on standard output.
 * @author phil 
 * @version 1.0
 */
class SimpleDebugger
{
public:
  /** Some suggested constants for message levels. **/
  /** Message Level: very detailed information - absolute debugging **/
  static const int ML_VERY_DETAILED = -10;
  /** Message Level: detailed information **/
  static const int ML_DETAILED = -5;
  /** Message Level: default level, normal messages **/
  static const int ML_DEFAULT = 0;
  /** Message Level: just information **/
  static const int ML_INFORMATIVE = 5;
  /** Message Level: just sketchy **/
  static const int ML_SKETCHY = 10;

  /** Main output stream for debugging (can globally be modified) **/
  static std::ostream *DEBUG_STREAM;
  /** Activates/deactivates debugging output globally **/
  static bool DEBUGGING_ACTIVE;
  /**
   * Determines the threshold level for message output (the lower the level,
   * the more detailed the messages). See also the ML_xxx constants. Internally,
   * a ">=" relation is applied.
   */
  static int MESSAGE_LEVEL_THRESHOLD;
  /**
   * Activates/deactivates prepending a timestamp.
   */
  static bool PREPEND_TIMESTAMP;
  /**
   * Activates/deactivates prepending the message level.
   */
  static bool PREPEND_MESSAGE_LEVEL;
  /**
   * Activates/deactivates auto-flushing on each debug/error message.
   */
  static bool FLUSH_ON_MESSAGE;

  /**
   * Automatically generate a global debugging output for SimpleDebugger.
   * This includes LOG file generation, cerr-redirection and console output.
   * NOTE: internally the ora::merrbuf and ora::merr objects are used for
   * outputting!
   * @param baseLogFileName the base file name of the debugging log file; if
   * an empty string is applied then no LOG file will be generated (the path
   * is automatically created if it does not exist); NOTE that you can apply
   * two generic constants within the string: {$DATE} and {$TIME} which will
   * be replaced by current date and time in ORA-format respectively
   * @param logFileAppendMode if TRUE, the LOG file is opened in std::ios::app
   * mode (which will append new contents to the current file content); if
   * FALSE the LOG file is opened in std::ios::out mode (which will override
   * current file contents)
   * @param consoleOutput if TRUE the debugging messages will be printed to
   * the console (std::cerr port) as well
   * @param redirectCerrToOutput if TRUE all streaming content sent to std::cerr
   * will be forwarded to the central debug output (certainly this setting does
   * not interfere with consoleOutput; the implementation takes care of that!)
   * @return TRUE if a debugging output has successfully been generated and set
   * @see ora::MultiOutputStreamBuffer
   */
  static bool MakeGlobalDebugOutput(std::string baseLogFileName,
      bool logFileAppendMode, bool consoleOutput,
      bool redirectCerrToOutput);

  /**
   * Release the global debugging output of SimpleDebugger. This includes
   * std::cerr restauration if necessary (previous redirection), LOG file
   * closing and resetting the global ora::merrbuf.
   * @see ora::MultiOutputStreamBuffer
   */
  static void ReleaseGlobalDebugOutput();

  /** Default constructor **/
  SimpleDebugger();

  /** Default destructor **/
  virtual ~SimpleDebugger();

  /**
   * Debug output for string messages. Message is printed to configured
   * output stream. Additionally the message level
   * is considered which means that the message level must be greater than or
   * equal the global message level threshold.
   * @param msg the string message to be printed
   * @param level the message's level
   * @see MESSAGE_LEVEL_THRESHOLD
   */
  void Debug(const std::string msg, const int level);

  /**
   * Error output for string messages. Message is printed to configured
   * output stream. An error output is definitely written to output
   * stream - the message level is regardless in this context.
   * @param msg the string message to be printed (in addition error
   * location information is provided)
   */
  void Error(const std::string msg);

protected:
  /** helper stream for LOG-file output **/
  static std::ofstream *OUTFILE_STREAM;
  /** helper pointer to the original cerr stream buffer **/
  static std::streambuf *ORIGINAL_CERR_BUF;
  /** helper stream for outputting to original console error stream **/
  static std::ostream *OUT_CONSOLE_STREAM;
  /** platform-independent clock **/
  itk::RealTimeClock::Pointer m_Clock;

};


#endif /* SIMPLEDEBUGGER_H_ */
