

#ifndef ORASTRINGTOOLS_H
#define ORASTRINGTOOLS_H


#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <time.h>
#include <typeinfo>
#include <stdexcept>


namespace ora
{

// This module contains some general tool-functions for string and
// string-related operations not provided by ANSI-C++ or ITK.
//
// @version 1.7
// @author phil 
// @author Markus 

/**
  * Converts a value (from) of type F into another representation of type
  * T using string-streams internally.<br>
  * There are some ANSI-C++ problems concerning template-functions and so I
  * had to directly implement this function here.
  * @param from the value of type F to be converted
  * @return the value in new representation of type T
  */
template<typename T, typename F> inline T StreamConvert(F from)
{
  std::stringstream temp;
  temp << from;
  T to = T();
  temp >> to;

  return to;
}

/**
  * Converts a value (from) of type F into std::string representation using
  * string-streams internally.
  * It is a specialized implementation of
  * @see template<typename T, typename F> inline T StreamConvert(F from).<br>
  * There are some ANSI-C++ problems concerning template-functions and so I
  * had to directly implement this function here.
  * @param from the value of type F to be converted
  * @return the value in new representation of type std::string
  */
template<typename F> inline std::string StreamConvert(F from)
{
  return StreamConvert<std::string>(from);
}

/** BadConversion errors represent problems outside the scope of a program
 * when strings or other data-types are converted. They cannot be easily
 * predicted and can generally only be caught as the program executes.
 */
class BadConversion: public std::runtime_error
{
public:
  /** Constructor.
   * @param s Character string describing the error.
   */
  BadConversion(std::string const& s) :
    std::runtime_error(s)
  {
  }
};

/** Converts an arbitrary type T to a std::string with a string stream,
 * provided T supports syntax like std::cout << x.
 * @param x Value to convert to an std::string.
 * @return Value of \a x as a std::string.
 * @throw BadConversion If conversion fails.
 */
template<typename T>
inline std::string Stringify(T const& x)
{
  std::ostringstream o;
  if (!(o << x))
    throw BadConversion(std::string("Stringify(") + typeid(x).name() + ")");
  return o.str();
}

/** Converts as much of a string \a s as possible/appropriate based on the
 * type of \a x. The conversion is performed with std::istringstream via the
 * overloaded extraction operator >>.
 * @param s String to convert to a value of type T.
 * @param x Result of the string conversion of type T.
 * @param failIfLeftoverChars If TRUE throws a BadConversion exception if the
 *    stream conversion has some left over characters (default is FALSE).
 * @throw BadConversion If conversion fails (string contains characters that are
 *    inappropriate for the type of \a x) or characters are left over if \a
 *    failIfLeftoverChars is TRUE.
 */
template<typename T>
inline void StringConvert(std::string const& s, T& x, bool failIfLeftoverChars =
    false)
{
  std::istringstream i(s);
  char c;
  if (!(i >> x) || (failIfLeftoverChars && i.get(c)))
    throw BadConversion(std::string("StringConvert(") + s + "," + typeid(x).name()
        + "," + Stringify(failIfLeftoverChars) + ")");
}

/** Simplified version of StringConvert() that uses return-by-value conversion.
 * @param s String to convert to a value of type T.
 * @param x Result of the string conversion of type T.
 * @param failIfLeftoverChars If TRUE throws a BadConversion exception if the
 *    stream conversion has some left over characters (default is FALSE).
 * @throw BadConversion If conversion fails (string contains characters that are
 *    inappropriate for the type of \a x) or characters are left over if \a
 *    failIfLeftoverChars is TRUE.
 * @see StringConvert()
 */
template<typename T>
inline T StringConvertTo(std::string const& s, bool failIfLeftoverChars = false)
{
  T x;
  StringConvert(s, x, failIfLeftoverChars);
  return x;
}

/**
  * Trims the referenced string parameter (cuts leading and trailing spaces).
  * @param s the string to be trimmedd
  */
void Trim(std::string &s);

/**
  * Trims the referenced string parameter (cuts leading and trailing spaces),
  * but does not change the parameter itself, it returns the trimmed version
  * of the parameter.
  * @param s the string to be trimmed (but not changed)
  * @return the trimmed version of s
  */
std::string TrimF(std::string s);

/**
 * Convert a string to lower case (by reference).
 */
void ToLowerCase(std::string &s);

/**
 * Convert a string to lower case (return it).
 */
std::string ToLowerCaseF(std::string s);

/**
 * Convert a string to upper case (by reference).
 */
void ToUpperCase(std::string &s);

/**
 * Convert a string to upper case (return it).
 */
std::string ToUpperCaseF(std::string s);

/**
 * Convert an open radART date/time string into time.
 * @return true if successful
 */
bool ORAStringToCDateTime(std::string s, tm &time);

/**
 * Analyze the specified open radART date/time string.
 * @s ORA date/time string
 * @param hasTimePart return true if s includes a valid ORA time part
 * @param hasDatePart return true if s includes a valid ORA date part
 */
void AnalyzeORADateTimeString(std::string s, bool &hasTimePart,
    bool &hasDatePart);

/**
 * Convert a time structure to open radART date format: "YYYY-MM-DD".
 * @param time initialized standard C time structure
 * @param s returned ORA date expression
 */
void CDateTimeToORADateString(tm *time, std::string &s);

/**
 * Convert current date to open radART date format: "YYYY-MM-DD".
 * @param s returned ORA date expression
 */
void CurrentORADateString(std::string &s);

/**
 * Convert a time structure to open radART time format: "HH:NN:SS".
 * @param time initialized standard C time structure
 * @param s returned ORA time expression
 */
void CDateTimeToORATimeString(tm *time, std::string &s);

/**
 * Convert current time to open radART time format: "HH:NN:SS".
 * @param s returned ORA time expression
 */
void CurrentORATimeString(std::string &s);

/**
 * Convert a time structure to open radART date/time format:
 * "YYYY-MM-DD HH:NN:SS".
 * @param time initialized standard C time structure
 * @param s returned ORA date/time expression
 */
void CDateTimeToORADateTimeString(tm *time, std::string &s);

/**
 * Convert current date/time to open radART date/time format:
 * "YYYY-MM-DD HH:NN:SS".
 * @param s returned ORA time expression
 */
void CurrentORADateTimeString(std::string &s);

/**
 * Convert DICOM date to open radART date format:
 * "YYYYMMDD" -> "YYYY-MM-DD"
 * @param s returned ORA date expression
 */
void DICOMDateToORADate(std::string &s);

/**
 * Convert open radART date to DICOM date format:
 * "YYYY-MM-DD" -> "YYYYMMDD"
 * @param s returned DICOM date expression
 */
void ORADateToDICOMDate(std::string &s);

/**
 * Convert DICOM time to open radART time format:
 * "HHNNSS.FFFFFF" -> "HH:NN:SS"
 * @param s returned ORA time expression
 */
void DICOMTimeToORATime(std::string &s);

/**
 * Convert open radART time to DICOM time format:
 * "HH:NN:SS" -> "HHNNSS.FFFFFF"
 * @param s returned DICOM time expression
 */
void ORATimeToDICOMTime(std::string &s);

/**
 * Convert DICOM date/time to open radART date/time format:
 * "YYYYMMDDHHNNSS.FFFFFF&ZZXX" -> "YYYY-MM-DD HH:NN:SS"
 * @param s returned ORA date/time expression
 */
void DICOMDateTimeToORADateTime(std::string &s);

/**
 * Convert open radART date/time to DICOM date/time format:
 * "YYYY-MM-DD HH:NN:SS" -> "YYYYMMDDHHNNSS.FFFFFF&ZZXX"
 * @param s returned DICOM date/time expression
 */
void ORADateTimeToDICOMDateTime(std::string &s);

/**
 * Convert DICOM person name to natural person name:
 * "FamilyName^GivenName^MiddleName^NamePrefix^NameSuffix" ->
 * "NamePrefix GivenName MiddleName FamilyName, NameSuffix"
 * @param s returned natural name
 */
void DICOMPersonNameToNaturalPersonName(std::string &s);

/**
  * Replaces all occurences of specified searchStr with the specified
  * replaceStr directly in specified inputStr.
  * @param inputStr the string to be modified
  * @param searchStr string expression specifying the search-sub-string
  * @param replaceStr string expression specifying the replace-sub-string
  * @return TRUE if at least one occurrence of the search string was found and
  * replaced, FALSE otherwise
  */
bool ReplaceString(std::string &inputStr, std::string searchStr,
  std::string replaceStr);

/**
  * Ensures that the specified string ends with the specified string. It is
  * appended if it does not already end with the specified string. This may
  * be useful for appending trailing slashes at the end for file pathes
  * in UNIX systems or backslashes in Windows systems.
  * @param s the string-reference to be checked (it is directly manipulated)
  * @param endWith s must end with this string (otherwise it is appended)
  */
void EnsureStringEndsWith(std::string &s, const std::string endWith);

/**
  * Ensures that the specified string does not end with the specified string. It
  * is cut if it ends with the specified string. This may
  * be useful for cutting trailing slashes at the end of file pathes
  * in UNIX systems or backslashes in Windows systems.
  * @param s the string-reference to be checked (it is directly manipulated)
  * @param notEndWith s must not end with this string (otherwise it is cut)
  */
void EnsureStringNotEndsWith(std::string &s, const std::string notEndWith);

/**
  * Tokenize a string into pieces (separated by delimiters).
  * @param str (delimited) string to be tokenized
  * @param tokens returned tokens
  * @param delimiters delimiter specification (default: blank)
  */
void Tokenize(const std::string &str, std::vector<std::string> &tokens,
  const std::string &delimiters = " ");

void TokenizeIncludingEmptySpaces(const std::string &str,
  std::vector<std::string> &tokens, const std::string &delimiters = " ");

/**
 * Convert an image direction vector into the typical (DICOM) patient
 * direction format.
 * @param vector the image direction vector to be processed
 * @param DICOMConformant flag indicating whether the letters should be
 * DICOM-conformant (TRUE; R,L,A,P,F,H) or meta-image-conformant
 * (FALSE; R,L,A,P,I,S)
 * @return the converted character string (at most 3 characters)
 */
char *GetOrientationFromVector(double vector[3], bool DICOMConformant);

/**
 * Convert an arbitrary string to a 'clean simple file name' string; i.e.
 * eliminating all characters from the raw input string that is different from:
 * "a"-"z","A"-"Z","0"-"9","-","_","+","#",".","(",")".
 */
std::string CleanStringForSimpleFileName(std::string rawName);

/** @return TRUE if s contains only numbers ('0'-'9') **/
bool IsStrictlyNumeric(std::string s);

/** @return TRUE if s contains a numeric value (including doubles ...) **/
bool IsNumeric(std::string s);

/**
 * Parse a version string of the form "n.n.n.n .." and return the literals in
 * integer representation in a vector that retains the order. NOTE: the version
 * string must really be correct which means that each literal (n) must be
 * strictly numeric and non-empty! Leading and trailing spaces do not matter.
 */
void ParseVersionString(std::string s, std::vector<int> &version);

/**
 * Parse a comma-separated string vector which holds a list of numeric strings
 * (e.g. "1.2,23.2,0,23") and return the converted numbers as double array.
 * @param s string to be parsed
 * @param v returned double vector containing the values if successful;
 * NOTE: This array must be freed manually!
 * @return the number of vector elements
 */
int ParseCommaSeparatedNumericStringVector(std::string s, double *&v);

}

#endif /* ORASTRINGTOOLS_H */
