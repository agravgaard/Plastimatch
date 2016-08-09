/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _string_util_h_
#define _string_util_h_

#include "plmsys_config.h"
#include <stdarg.h>
#include <string>
#include <vector>
#include "plm_return_code.h"

PLMSYS_API bool string_starts_with (const std::string& s1, const char* s2);
PLMSYS_API bool string_starts_with (const char* s1, const char* s2);
PLMSYS_API int plm_strcmp (const char* s1, const char* s2);
PLMSYS_API std::string make_lowercase (const std::string& s);
PLMSYS_API std::string make_uppercase (const std::string& s);
PLMSYS_API std::string regularize_string (const std::string& s);
PLMSYS_API void string_util_rtrim_whitespace (char *s);
PLMSYS_API Plm_return_code parse_int13 (int *arr, const char *string);
PLMSYS_API Plm_return_code parse_int13 (int *arr, const std::string& string);
PLMSYS_API Plm_return_code parse_float13 (float *arr, const char *string);
PLMSYS_API Plm_return_code parse_float13 (float *arr,
    const std::string& string);
PLMSYS_API int parse_dicom_float2 (float *arr, const char *string);
PLMSYS_API int parse_dicom_float3 (float *arr, const char *string);
PLMSYS_API int parse_dicom_float6 (float *arr, const char *string);
PLMSYS_API std::vector<int> parse_int3_string (const char* s);
PLMSYS_API std::vector<float> parse_float3_string (const char* s);
PLMSYS_API std::vector<float> parse_float3_string (const std::string& s);
PLMSYS_API std::vector<float> parse_float_string (const char* s);
PLMSYS_API std::vector<float> parse_float_string (const std::string& s);
PLMSYS_API const std::string string_trim (
    const std::string& str,
    const std::string& whitespace = " \t\r\n"
);
PLMSYS_API std::string slurp_file (const char* fn);
PLMSYS_API std::string slurp_file (const std::string& fn);
PLMSYS_API std::string string_format_va (const char *fmt, va_list ap);
PLMSYS_API std::string string_format (const char* fmt, ...);
PLMSYS_API size_t ci_find (const std::string& str1, const std::string& str2);

PLMSYS_API bool string_value_true (const char* s);
PLMSYS_API bool string_value_true (const std::string& s);
PLMSYS_API bool string_value_false (const char* s);
PLMSYS_API bool string_value_false (const std::string& s);

template <typename T> PLMSYS_API std::string PLM_to_string(T value);
template <typename T> PLMSYS_API std::string PLM_to_string(T *value, int n);

PLMSYS_API std::vector<std::string>& string_split (const std::string &s, char delim, std::vector<std::string> &elems);
PLMSYS_API std::vector<std::string> string_split (const std::string &s, char delim);
PLMSYS_API bool split_tag_val (const std::string& s, 
    std::string& tag, std::string& val, char delim = '=');

#endif
