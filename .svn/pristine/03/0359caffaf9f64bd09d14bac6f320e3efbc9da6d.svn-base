/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmsys_config.h"
#include <algorithm>
#include <ctype.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iterator>

#include "plm_va_copy.h"
#include "string_util.h"

bool
string_starts_with (const std::string& s1, const char* s2)
{
    return string_starts_with (s1.c_str(), s2);
}

bool
string_starts_with (const char* s1, const char* s2)
{
    return strncmp (s1, s2, strlen(s2)) == 0;
}

int
plm_strcmp (const char* s1, const char* s2)
{
    return strncmp (s1, s2, strlen(s2));
}

std::string 
make_lowercase (const std::string& s)
{
    std::string out;
    std::transform (s.begin(), s.end(), std::back_inserter(out), ::tolower);
    return out;
}

std::string 
make_uppercase (const std::string& s)
{
    std::string out;
    std::transform (s.begin(), s.end(), std::back_inserter(out), ::toupper);
    return out;
}

static int 
regularize_string_callback (int c)
{
    int o = ::tolower (c);
    if (o == '-') {
        o = '_';
    }
    return o;
}

/* Make lower case, and convert dash '-' to underscore '_' */
std::string 
regularize_string (const std::string& s)
{
    std::string out;
    std::transform (s.begin(), s.end(), std::back_inserter(out), 
        regularize_string_callback);
    return out;
}

void
string_util_rtrim_whitespace (char *s)
{
    int len = (int)strlen (s);
    while (len > 0 && isspace(s[len-1])) {
        s[len-1] = 0;
        len--;
    }
}

Plm_return_code
parse_int13 (int *arr, const char *string)
{
    int rc;
    rc = sscanf (string, "%d %d %d", &arr[0], &arr[1], &arr[2]);
    if (rc == 3) {
        return PLM_SUCCESS;
    } else if (rc == 1) {
        arr[1] = arr[2] = arr[0];
        return PLM_SUCCESS;
    } else {
        return PLM_ERROR;
    }
}

Plm_return_code
parse_int13 (int *arr, const std::string& string)
{
    return parse_int13 (arr, string.c_str());
}

Plm_return_code
parse_float13 (float *arr, const char *string)
{
    int rc;
    rc = sscanf (string, "%g %g %g", &arr[0], &arr[1], &arr[2]);
    if (rc == 3) {
        return PLM_SUCCESS;
    } else if (rc == 1) {
        arr[1] = arr[2] = arr[0];
        return PLM_SUCCESS;
    } else {
        return PLM_ERROR;
    }
}

Plm_return_code
parse_float13 (float *arr, const std::string& string)
{
    return parse_float13 (arr, string.c_str());
}

int
parse_dicom_float2 (float *arr, const char *string)
{
    int rc;
    rc = sscanf (string, "%f\\%f", &arr[0], &arr[1]);
    if (rc == 2) {
        return 0;
    } else {
        /* Failure */
        return 1;
    }
}

int
parse_dicom_float3 (float *arr, const char *string)
{
    int rc;
    rc = sscanf (string, "%f\\%f\\%f", &arr[0], &arr[1], &arr[2]);
    if (rc == 3) {
        return 0;
    } else {
        /* Failure */
        return 1;
    }
}

int
parse_dicom_float6 (float *arr, const char *string)
{
    int rc;
    rc = sscanf (string, "%f\\%f\\%f\\%f\\%f\\%f", 
        &arr[0], &arr[1], &arr[2], &arr[3], &arr[4], &arr[5]);
    if (rc == 6) {
        return 0;
    } else {
        /* Failure */
        return 1;
    }
}

/* Parse a string of the form "3 22 -1; 3 4 66; 3 1 0" */
std::vector<int>
parse_int3_string (const char* s)
{
    std::vector<int> int_list;
    const char* p = s;
    int rc = 0;
    int n;

    do {
        int v[3];

        n = 0;
        rc = sscanf (p, "%d %d %d;%n", &v[0], &v[1], &v[2], &n);
        p += n;
        if (rc >= 3) {
            int_list.push_back (v[0]);
            int_list.push_back (v[1]);
            int_list.push_back (v[2]);
        }
    } while (rc >= 3 && n > 0);
    return int_list;
}

/* Parse a string of the form "3 22 -1; 3 4 66; 3 1 0" */
std::vector<float>
parse_float3_string (const char* s)
{
    std::vector<float> float_list;
    const char* p = s;
    int rc = 0;
    int n;

    do {
        float v[3];

        n = 0;
        rc = sscanf (p, "%f %f %f;%n", &v[0], &v[1], &v[2], &n);
        p += n;
        if (rc >= 3) {
            float_list.push_back (v[0]);
            float_list.push_back (v[1]);
            float_list.push_back (v[2]);
        }
    } while (rc >= 3 && n > 0);
    return float_list;
}

std::vector<float>
parse_float3_string (const std::string& s)
{
    return parse_float3_string (s.c_str());
}

/* Parse a string of the form "3 0.2 1e-3" or "3,0.2,1e-3"*/
std::vector<float>
parse_float_string (const char* s)
{
    std::vector<float> float_list;
    const char* p = s;
    int rc = 0;
    int n;

    do {
        float v;
        n = 0;
        rc = sscanf (p, " %f%n", &v, &n);
        if (rc == 0) {
            rc = sscanf (p, " , %f%n", &v, &n);
        }
        if (rc == 0 || rc == EOF) {
            break;
        }
        p += n;
        float_list.push_back (v);
    } while (1);
    return float_list;
}

std::vector<float>
parse_float_string (const std::string& s)
{
    return parse_float_string (s.c_str());
}

/* String trimming by GMan.
   http://stackoverflow.com/questions/1798112/removing-leading-and-trailing-spaces-from-a-string/1798170#1798170
   Distributed under Attribution-ShareAlike 3.0 Unported license (CC BY-SA 3.0) 
   http://creativecommons.org/licenses/by-sa/3.0/
*/
const std::string
string_trim (
    const std::string& str,
    const std::string& whitespace
)
{
    const size_t begin_str = str.find_first_not_of (whitespace);
    if (begin_str == std::string::npos)
    {
        // no content
        return "";
    }

    const size_t end_str = str.find_last_not_of(whitespace);
    const size_t range = end_str - begin_str + 1;

    return str.substr (begin_str, range);
}

std::string
slurp_file (const char* fn)
{
    /* Read file into string */
    std::ifstream t (fn);
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

std::string
slurp_file (const std::string& fn)
{
    return slurp_file (fn.c_str());
}

/* std::string formatting by Erik Aronesty
   http://stackoverflow.com/questions/2342162/stdstring-formating-like-sprintf
   Distributed under Attribution-ShareAlike 3.0 Unported license (CC BY-SA 3.0) 
   http://creativecommons.org/licenses/by-sa/3.0/
*/
std::string 
string_format_va (const char *fmt, va_list ap)
{
    int size=100;
    std::string str;
    while (1) {
        str.resize(size);
        va_list ap_copy;
        va_copy (ap_copy, ap);
        //int n = vsnprintf((char *)str.c_str(), size, fmt, ap_copy);
        int n = vsnprintf((char *)str.data(), size, fmt, ap_copy);
        va_end (ap_copy);
        if (n > -1 && n < size) {
            str = std::string (str.c_str());  /* Strip excess padding */
            return str;
        }
        if (n > -1)
            size=n+1;
        else
            size*=2;
    }
}

std::string 
string_format (const char *fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
    std::string string = string_format_va (fmt, ap);
    va_end (ap);
    return string;
}

/* Case-insensitive string::find() by Kirill V. Lyadvinsky 
http://stackoverflow.com/questions/3152241/case-insensitive-stdstring-find
   Distributed under Attribution-ShareAlike 3.0 Unported license (CC BY-SA 3.0) 
   http://creativecommons.org/licenses/by-sa/3.0/
*/
// templated version of my_equal so it could work with both char and wchar_t
struct my_equal {
    bool operator()(char ch1, char ch2) {
#ifdef _WIN32
      return toupper(ch1) == toupper(ch2);
#else
      return std::toupper(ch1) == std::toupper(ch2);
#endif
    }
};

// find substring (case insensitive)
size_t ci_find (const std::string& str1, const std::string& str2)
{
    std::string::const_iterator it = std::search (str1.begin(), str1.end(), 
        str2.begin(), str2.end(), my_equal());
    if (it != str1.end()) return it - str1.begin();
    else return std::string::npos;
}
// Return true for "true", "1", or "on"
bool string_value_true (const char* s)
{
    return string_value_true (std::string(s));
}

bool string_value_true (const std::string& s)
{
    std::string t = make_lowercase (s);
    return t == "1" || t == "true" || t == "on" || t == "yes";
}

bool string_value_false (const char* s)
{
    return string_value_false (std::string(s));
}

bool string_value_false (const std::string& s)
{
    return !string_value_true (s);
}

// compiler does not recognize the std::to_string (c++11), so made our own
template <typename T>
std::string PLM_to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}

/* http://stackoverflow.com/questions/236129/split-a-string-in-c
   This version also trims, therefore different than the one described 
   in stackoverflow. */
std::vector<std::string>& string_split (
    const std::string &s, 
    char delim, 
    std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        item = string_trim (item);
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> string_split (
    const std::string &s, 
    char delim)
{
    std::vector<std::string> elems;
    string_split (s, delim, elems);
    return elems;
}

bool split_tag_val (
    const std::string& s, 
    std::string& tag,
    std::string& val,
    char delim)
{
    size_t loc = s.find (delim);
    if (loc == std::string::npos) {
        tag = "";
        val = "";
        return false;
    }
    tag = string_trim (s.substr (0, loc));
    val = string_trim (s.substr (loc + 1));
    return true;
}
