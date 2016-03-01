

#include "oraStringTools.h"

#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <locale>
#include <algorithm>
#include <cctype>


namespace ora
{


void
Trim(std::string &s)
{
  std::string::size_type pos = s.find_last_not_of(' ');
  if(pos != std::string::npos)
  {
    s.erase(pos + 1);
    pos = s.find_first_not_of(' ');
    if(pos != std::string::npos)
      s.erase(0, pos);
  }
  else
    s.erase(s.begin(), s.end());
}

std::string
TrimF(std::string s)
{
  Trim(s);
  return s;
}

// Functor for MSVC
struct ToLower
{
  char operator()(char c)
  {
    return std::tolower(c, loc_);
  }
private:
  std::locale loc_; // default locale of the user environment
};

void
ToLowerCase(std::string &s)
{
  std::transform(s.begin(), s.end(), s.begin(), ToLower());
}

std::string
ToLowerCaseF(std::string s)
{
  std::string retstr = s;

  ToLowerCase(retstr);

  return retstr;
}

void
ToUpperCase(std::string &s)
{
  for (std::string::iterator it = s.begin(); it != s.end(); ++it)
    *it = std::toupper(*it);
}

std::string
ToUpperCaseF(std::string s)
{
  std::string retstr = s;

  ToUpperCase(retstr);

  return retstr;
}

bool
ORAStringToCDateTime(std::string s, tm &time)
{
  // ORA-format: "yyyy-mm-dd" [" hh:nn:ss"]

  time.tm_sec = 0; // init structure
  time.tm_min = 0;
  time.tm_hour = 0;
  time.tm_mday = 0;
  time.tm_mon = 0;
  time.tm_year = 0;
  time.tm_wday = 0;
  time.tm_yday = 0;
  time.tm_isdst = 0;

  if (s.length() <= 0) // at least full date-specification needed
    return false;

  int n;

  if ((n = sscanf(s.c_str(), "%d-%d-%d %d:%d:%d", &time.tm_year, &time.tm_mon,
      &time.tm_mday, &time.tm_hour, &time.tm_min, &time.tm_sec)) >= 3)
  {
    // correct month-/year-specification:
    time.tm_mon -= 1;
    time.tm_year -= 1900;

    return true;
  }
  else
    return false;
}

void
AnalyzeORADateTimeString(std::string s, bool &hasTimePart, bool &hasDatePart)
{
  // ORA-format: "yyyy-mm-dd" [" hh:nn:ss"]

  hasDatePart = false;
  hasTimePart = false;
  Trim(s);
  if (s.length() < 8)
    return;

  std::string s2;
  if (s.length() >= 8) // time?
  {
    if (s.length() == 19)
      s2 = s.substr(11, 8);
    else if (s.length() == 8)
      s2 = s;
    else
      s2 = "";
    if (s2.length() == 8 &&
        IsStrictlyNumeric(s2.substr(0, 2)) &&
        IsStrictlyNumeric(s2.substr(3, 2)) &&
        IsStrictlyNumeric(s2.substr(6, 2)) &&
        s2.substr(2, 1) == ":" &&
        s2.substr(5, 1) == ":")
      hasTimePart = true;
  }
  if (s.length() >= 10) // date?
  {
    if (s.length() == 19)
      s2 = s.substr(0, 10);
    else if (s.length() == 10)
      s2 = s;
    else
      s2 = "";
    if (s2.length() == 10 &&
        IsStrictlyNumeric(s2.substr(0, 4)) &&
        IsStrictlyNumeric(s2.substr(5, 2)) &&
        IsStrictlyNumeric(s2.substr(8, 2)) &&
        s2.substr(4, 1) == "-" &&
        s2.substr(7, 1) == "-")
      hasDatePart = true;
  }
}

void
CDateTimeToORADateString(tm *time, std::string &s)
{
  char buff[11];

  // "YYYY-MM-DD"
  int y = time->tm_year + 1900;
  int m = time->tm_mon + 1;
  int d = time->tm_mday;
  sprintf(buff, "%4d-%02d-%02d", y, m, d);
  s = std::string(buff);
}

void
CurrentORADateString(std::string &s)
{
  time_t nowsec = time (NULL);
  tm *now = localtime(&nowsec);
  CDateTimeToORADateString(now, s);
}

void
CDateTimeToORATimeString(tm *time, std::string &s)
{
  char buff[9];

  // "HH-MM-SS"
  int h = time->tm_hour;
  int m = time->tm_min;
  int se = time->tm_sec;
  sprintf(buff, "%02d:%02d:%02d", h, m, se);
  s = std::string(buff);
}

void
CurrentORATimeString(std::string &s)
{
  time_t nowsec = time (NULL);
  tm *now = localtime(&nowsec);
  CDateTimeToORATimeString(now, s);
}

void
CDateTimeToORADateTimeString(tm *time, std::string &s)
{
  char buff[20];

  // "YYYY-MM-DD HH-MM-SS"
  int y = time->tm_year + 1900;
  int m = time->tm_mon + 1;
  int d = time->tm_mday;
  int h = time->tm_hour;
  int n = time->tm_min;
  int se = time->tm_sec;
  sprintf(buff, "%4d-%02d-%02d %02d:%02d:%02d", y, m, d, h, n, se);
  s = std::string(buff);
}

void
CurrentORADateTimeString(std::string &s)
{
  time_t nowsec = time (NULL);
  tm *now = localtime(&nowsec);
  CDateTimeToORADateTimeString(now, s);
}

void
DICOMDateToORADate(std::string &s)
{
  if (s.length() == 8)
  {
    s.insert(6, "-");
    s.insert(4, "-");
  }
}

void
ORADateToDICOMDate(std::string &s)
{
  if (s.length() == 10)
  {
    s.erase(7, 1);
    s.erase(4, 1);
  }
}

void
DICOMTimeToORATime(std::string &s)
{
  if (s.length() > 6) // fractions of a second not recognized in ORA format
    s = s.substr(0, 6);
  if (s.length() == 6)
  {
    s.insert(4, ":");
    s.insert(2, ":");
  }
  else if (s.length() == 4)
  {
    s.insert(2, ":");
    s += ":00";
  }
  else if (s.length() == 2)
    s += ":00:00";
}

void
ORATimeToDICOMTime(std::string &s)
{
  if (s.length() == 8)
  {
    s.erase(5, 1);
    s.erase(2, 1);
  }
}

void
DICOMDateTimeToORADateTime(std::string &s)
{
  if (s.length() > 14)
    s = s.substr(0, 14);
  if (s.length() == 14)
  {
    std::string s1 = s.substr(0, 8);
    DICOMDateToORADate(s1);
    std::string s2 = s.substr(8, 6);
    DICOMTimeToORATime(s2);
    s = s1 + " " + s2;
  }
}

void
ORADateTimeToDICOMDateTime(std::string &s)
{
  if (s.length() == 19)
  {
    std::string s1 = s.substr(0, 10);
    ORADateToDICOMDate(s1);
    std::string s2 = s.substr(11, 8);
    ORATimeToDICOMTime(s2);
    s = s1 + s2;
  }
}

void
DICOMPersonNameToNaturalPersonName(std::string &s)
{
  std::vector<std::string> tok;
  TokenizeIncludingEmptySpaces(s, tok, "^");
  s = "";
  if (tok.size() > 0)
  {
    if (tok.size() > 3 && tok[3].length() > 0)
      s = tok[3] + " ";
    if (tok.size() > 1 && tok[1].length() > 0)
      s += tok[1] + " ";
    if (tok.size() > 2 && tok[2].length() > 0)
      s += tok[2] + " ";
    s += ToUpperCaseF(tok[0]) + " ";
    if (tok.size() > 4 && tok[4].length() > 0)
    {
      Trim(s);
      s += ", " + tok[4];
    }
    Trim(s);
  }
}

bool
ReplaceString(std::string &inputStr, std::string searchStr,
  std::string replaceStr)
{
  std::string::size_type pos = 0;
  bool foundAndReplaced = false;

  pos = inputStr.find(searchStr);
  while (pos != std::string::npos)
  {
    inputStr.replace(pos, searchStr.length(), replaceStr);
    pos = inputStr.find(searchStr, pos + 1);
    foundAndReplaced = true;
  }

  return foundAndReplaced;
}

void
EnsureStringEndsWith(std::string &s, const std::string endWith)
{
  int ewl = endWith.length();
  int sl = s.length();

  if (ewl > 0 && ewl <= sl && s.substr(sl - ewl, ewl) != endWith)
    s += endWith;
}

void
EnsureStringNotEndsWith(std::string &s, const std::string notEndWith)
{
  int newl = notEndWith.length();
  int sl = s.length();

  if (newl > 0 && newl <= sl && s.substr(sl - newl, newl) == notEndWith)
    s = s.substr(0, sl - newl);
}

void
Tokenize(const std::string &str, std::vector<std::string> &tokens,
  const std::string &delimiters)
{

  std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
  std::string::size_type pos = str.find_first_of(delimiters, lastPos);

  while (std::string::npos != pos || std::string::npos != lastPos)
  {
    tokens.push_back(str.substr(lastPos, pos - lastPos));
    lastPos = str.find_first_not_of(delimiters, pos);
    pos = str.find_first_of(delimiters, lastPos);
  }
}

void
TokenizeIncludingEmptySpaces(const std::string &str,
  std::vector<std::string> &tokens, const std::string &delimiters)
{
  std::string::size_type lastPos = 0;
  std::string::size_type pos = str.find_first_of(delimiters, lastPos);
  while ( pos <= str.length() && lastPos <= str.length())
  {
    tokens.push_back(str.substr(lastPos, pos - lastPos));
    lastPos = pos + 1;
    pos = str.find_first_of(delimiters, lastPos);
    if (pos > str.length())
      pos = str.length();
  }
}

char *
GetOrientationFromVector(double vector[3], bool DICOMConformant)
{
  char *orientation = new char[4];
  char *optr = orientation;
  *optr = '\0';

  char orientationX;
  char orientationY;
  char orientationZ;
  if (DICOMConformant) // DICOM characters
  {
    orientationX = vector[0] < 0 ? 'R' : 'L';
    orientationY = vector[1] < 0 ? 'A' : 'P';
    orientationZ = vector[2] < 0 ? 'F' : 'H';
  }
  else // meta image characters
  {
    orientationX = vector[0] < 0 ? 'R' : 'L';
    orientationY = vector[1] < 0 ? 'A' : 'P';
    orientationZ = vector[2] < 0 ? 'I' : 'S';
  }

  double absX = fabs(vector[0]);
  double absY = fabs(vector[1]);
  double absZ = fabs(vector[2]);

  for (int i = 0; i < 3; ++i)
  {
    if (absX > .0001 && absX > absY && absX > absZ)
    {
      *optr++ = orientationX;
      absX = 0;
    }
    else if (absY > .0001 && absY > absX && absY > absZ)
    {
      *optr++ = orientationY;
      absY = 0;
    }
    else if (absZ > .0001 && absZ > absX && absZ > absY)
    {
      *optr++ = orientationZ;
      absZ = 0;
    }
    else
      break;
    *optr = '\0';
  }

  return orientation;
}

std::string
CleanStringForSimpleFileName(std::string rawName)
{
  std::ostringstream os;
  os.str("");
  bool ok = false;
  for (std::size_t i = 0; i < rawName.length(); i++)
  {
    // "a"-"z","A"-"Z","0"-"9","-","_","+","#",".","(",")"
    char c = rawName[i];
    ok = false;
    ok = ok || (c >= 'A' && c <= 'Z');
    ok = ok || (c >= 'a' && c <= 'z');
    ok = ok || (c >= '0' && c <= '9');
    ok = ok || (c == '-');
    ok = ok || (c == '_');
    ok = ok || (c == '+');
    ok = ok || (c == '#');
    ok = ok || (c == '.');
    ok = ok || (c == ',');
    ok = ok || (c == '(');
    ok = ok || (c == ')');
    if (ok)
      os << c;
  }
  return os.str();
}

bool
IsStrictlyNumeric(std::string s)
{
  Trim(s);
  for (std::size_t i = 0; i < s.length(); i++)
    if (s[i] < '0' || s[i] > '9')
      return false;
  if (s.length() > 0)
    return true;
  else
    return false;
}

bool
IsNumeric(std::string s)
{
  Trim(s);
  std::istringstream inpStream(s);
  if (s.length() <= 0)
    return false;
  double inpValue = 0.0;
  if (inpStream >> inpValue) // NOTE: this works also for e.g. "0.0"!
    return true;
  else
    return false;
}

void
ParseVersionString(std::string s, std::vector<int> &version)
{
  version.clear();
  Trim(s);
  if (s.length() <= 0)
    return;
  std::vector<std::string> literals;
  TokenizeIncludingEmptySpaces(s, literals, ".");
  bool error = false;
  for (std::size_t i = 0; i < literals.size() && !error; i++)
  {
    if (literals[i].length() > 0 || IsStrictlyNumeric(literals[i]))
      version.push_back(StreamConvert<int, std::string>(literals[i]));
    else
      error = true;
  }
  if (error)
    version.clear();
}

int
ParseCommaSeparatedNumericStringVector(std::string s, double *&v)
{
  Trim(s);
  if (s.length() <= 0)
    return 0;
  std::vector<std::string> toks;
  Tokenize(s, toks, ",");
  bool num = true;
  for (std::size_t u = 0; u < toks.size(); u++)
    num &= IsNumeric(toks[u]);
  if (num)
  {
    v = new double[toks.size()];
    for (std::size_t u = 0; u < toks.size(); u++)
      v[u] = atof(toks[u].c_str());
    return toks.size();
  }
  return 0;
}


}
