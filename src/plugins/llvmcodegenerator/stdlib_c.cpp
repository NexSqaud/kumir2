#include "stdlib_c.h"
#include <stdlib/kumirstdlib.hpp>

#include <chrono>
#include <iostream>
#include <list>
#include <memory.h>
#include <stack>
#include <stdarg.h>
#include <string.h>
#include <thread>
#include <wchar.h>

#ifdef USE_MINGW_TOOLCHAIN
#include "dummy_sjlj.cpp"
#endif

#if defined(WIN32) || defined(_WIN32)
#else
#include <sys/resource.h>
#endif

static Kumir::FileType
__kumir_scalar_to_file_type(const __kumir_scalar &scalar);
static __kumir_scalar __kumir_file_type_to_scalar(const Kumir::FileType &f);
static void __kumir_create_string(__kumir_scalar *result,
                                  const std::wstring &wstr);
static __kumir_real __kumir_scalar_as_real(const __kumir_scalar *scalar);
static std::wstring __kumir_scalar_as_wstring(const __kumir_scalar *scalar);
static void __kumir_assert(const bool, int);

EXTERN void __kumir_internal_debug(int32_t code) {
  std::cerr << "Debug " << code << std::endl;
}

inline __kumir_int IMAX(const __kumir_int a, const __kumir_int b) {
  return a < b ? b : a;
}

inline __kumir_int IMIN(const __kumir_int a, const __kumir_int b) {
  return a > b ? b : a;
}

EXTERN void __use_all_types() {
  __kumir_bool b;
  __kumir_int i;
  __kumir_real r;
  __kumir_char c;
  __kumir_string s;
  __kumir_scalar scalar;
  __kumir_array array;
}

EXTERN void __kumir_create_undefined_scalar(__kumir_scalar *result) {
  result->defined = false;
  result->data.s = nullptr;
}

EXTERN void __kumir_create_undefined_array(__kumir_array *result) {
  result->data = nullptr;
  result->dim = 0;
}

EXTERN void __kumir_create_defined_scalar(__kumir_scalar *lvalue,
                                          const __kumir_scalar *rvalue) {
  __kumir_copy_scalar(lvalue, rvalue);
}

EXTERN void __kumir_create_int(__kumir_scalar *result,
                               const __kumir_int value) {
  result->defined = true;
  result->type = __KUMIR_INT;
  result->data.i = value;
}

EXTERN void __kumir_create_real(__kumir_scalar *result,
                                const __kumir_real value) {
  result->defined = true;
  result->type = __KUMIR_REAL;
  result->data.r = value;
}

EXTERN void __kumir_create_bool(__kumir_scalar *result,
                                const __kumir_bool value) {
  result->defined = true;
  result->type = __KUMIR_BOOL;
  result->data.b = value;
}

static void __kumir_create_char(__kumir_scalar *result,
                                const __kumir_char value) {
  result->defined = true;
  result->type = __KUMIR_CHAR;
  result->data.c = value;
}

EXTERN void __kumir_create_char(__kumir_scalar *result, const char *utf8) {
  std::string utf8string(utf8);
  std::wstring wstr;
  Kumir::EncodingError encodingError;
  wstr = Kumir::Core::fromUtf8(utf8string);
  result->defined = true;
  result->type = __KUMIR_CHAR;
  result->data.c = wstr[0];
}

EXTERN void __kumir_create_record(__kumir_scalar *result, int32_t fieldsCount) {
}

EXTERN void __kumir_add_record_field(__kumir_scalar *record, int32_t fieldIndex,
                                     __kumir_scalar *value) {}

EXTERN void __kumir_create_string(__kumir_scalar *result, const char *utf8) {
  std::string utf8string(utf8);
  std::wstring wstr;
  wstr = Kumir::Core::fromUtf8(utf8string);
  result->defined = true;
  result->type = __KUMIR_STRING;
  result->data.s =
      reinterpret_cast<wchar_t *>(calloc(wstr.length() + 1, sizeof(wchar_t)));
  memcpy(result->data.s, wstr.c_str(), wstr.length() * sizeof(wchar_t));
  result->data.s[wstr.length()] = L'\0';
}

EXTERN void __kumir_convert_char_to_string(__kumir_scalar *result,
                                           const __kumir_scalar *source) {
  __kumir_check_value_defined(source);
  result->type = __KUMIR_STRING;
  result->defined = true;
  result->data.s = (__kumir_string)malloc(2 * sizeof(__kumir_char));
  result->data.s[0] = source->data.c;
  result->data.s[1] = 0;
}

EXTERN void __kumir_convert_int_to_real(__kumir_scalar *result,
                                        const __kumir_scalar *source) {
  __kumir_check_value_defined(source);
  result->type = __KUMIR_REAL;
  result->defined = true;
  result->data.r = static_cast<__kumir_real>(source->data.i);
}

EXTERN __kumir_variant __kumir_copy_variant(const __kumir_variant rvalue,
                                            __kumir_scalar_type type) {
  __kumir_variant lvalue;
  switch (type) {
  case __KUMIR_INT:
    lvalue.i = rvalue.i;
    break;
  case __KUMIR_REAL:
    lvalue.r = rvalue.r;
    break;
  case __KUMIR_BOOL:
    lvalue.b = rvalue.b;
    break;
  case __KUMIR_CHAR:
    lvalue.c = rvalue.c;
    break;
  case __KUMIR_STRING: {
    size_t sz = wcslen(rvalue.s);
    lvalue.s = reinterpret_cast<wchar_t *>(calloc(sz + 1u, sizeof(wchar_t)));
    wcsncpy(lvalue.s, rvalue.s, sz);
    lvalue.s[sz] = L'\0';
    break;
  }
  case __KUMIR_RECORD: {
    lvalue.u.nfields = rvalue.u.nfields;
    lvalue.u.types = reinterpret_cast<__kumir_scalar_type *>(
        calloc(lvalue.u.nfields, sizeof(__kumir_scalar_type)));
    lvalue.u.fields = reinterpret_cast<__kumir_variant *>(
        calloc(lvalue.u.nfields, sizeof(__kumir_variant)));
    __kumir_variant *lfields =
        reinterpret_cast<__kumir_variant *>(lvalue.u.fields);
    __kumir_variant *rfields =
        reinterpret_cast<__kumir_variant *>(rvalue.u.fields);
    for (size_t i = 0u; i < rvalue.u.nfields; i++) {
      __kumir_scalar_type type = lvalue.u.types[i] = rvalue.u.types[i];
      lfields[i] = __kumir_copy_variant(rfields[i], type);
    }
    break;
  }
  }
  return lvalue;
}

EXTERN void __kumir_copy_scalar(__kumir_scalar *lvalue,
                                const __kumir_scalar *rvalue) {
  lvalue->defined = rvalue->defined;
  lvalue->type = rvalue->type;
  __kumir_check_value_defined(rvalue);
  if (lvalue->defined) {
    lvalue->data = __kumir_copy_variant(rvalue->data, rvalue->type);
  }
}

EXTERN void __kumir_move_scalar(__kumir_scalar *lvalue,
                                __kumir_scalar *rvalue) {
  lvalue->defined = rvalue->defined;
  lvalue->type = rvalue->type;
  memcpy(&(lvalue->data), &(rvalue->data), sizeof(rvalue->data));
}

EXTERN void __kumir_store_scalar(__kumir_scalar **lvalue_ptr,
                                 const __kumir_scalar *rvalue) {
  __kumir_scalar *lvalue = *lvalue_ptr;
  __kumir_copy_scalar(lvalue, rvalue);
}

EXTERN void __kumir_modify_string(__kumir_stringref *lvalue,
                                  const __kumir_scalar *rvalue) {
  __kumir_check_value_defined(rvalue);
  __kumir_check_value_defined(lvalue->ref);
  const std::wstring r = __kumir_scalar_as_wstring(rvalue);
  std::wstring l = __kumir_scalar_as_wstring(lvalue->ref);
  switch (lvalue->op) {
  case __KUMIR_STRINGREF_APPEND:
    l.append(r);
    break;
  case __KUMIR_STRINGREF_PREPEND:
    l = r + l;
    break;
  case __KUMIR_STRINGREF_INSERT:
    l.insert(lvalue->from, r);
    break;
  case __KUMIR_STRINGREF_REPLACE:
    l.replace(lvalue->from, lvalue->length, r);
    break;
  }
  free(lvalue->ref->data.s);
  __kumir_create_string(lvalue->ref, l);
}

static int32_t __kumir_current_line_number = -1;

EXTERN void __kumir_halt() {
  const std::wstring message = Kumir::Core::fromUtf8("\nСТОП.");
  Kumir::Encoding enc = Kumir::UTF8;
#if defined(WIN32) || defined(_WIN32)
  enc = Kumir::CP866;
#endif
  Kumir::EncodingError encodingError;
  const std::string loc_message =
      Kumir::Coder::encode(enc, message, encodingError);
  std::cout << loc_message;
  exit(0);
}

static int __kumir_call_stack_size = 0;
static const int __KUMIR_MAX_CALL_STACK_SIZE = 4000;

EXTERN void __kumir_check_call_stack() {
  __kumir_call_stack_size++;
  if (__kumir_call_stack_size >= __KUMIR_MAX_CALL_STACK_SIZE) {
    Kumir::Core::abort(
        Kumir::Core::fromUtf8("Слишком много вложенных вызовов алгоритмов"));
  }
}

EXTERN void __kumir_pop_call_stack_counter() { __kumir_call_stack_size--; }

static void __kumir_handle_abort() {
  const std::wstring message =
      __kumir_current_line_number == -1
          ? Kumir::Core::fromUtf8("ОШИБКА ВЫПОЛНЕНИЯ: ") +
                Kumir::Core::getError()
          : Kumir::Core::fromUtf8("ОШИБКА ВЫПОЛНЕНИЯ В СТРОКЕ ") +
                Kumir::Converter::sprintfInt(__kumir_current_line_number, 10, 0,
                                             0) +
                Kumir::Core::fromAscii(": ") + Kumir::Core::getError();
  Kumir::Encoding enc = Kumir::UTF8;
#if defined(WIN32) || defined(_WIN32)
  enc = Kumir::CP866;
#endif
  Kumir::EncodingError encodingError;
  const std::string loc_message =
      Kumir::Coder::encode(enc, message, encodingError);
  std::cerr << loc_message << std::endl;
  exit(1);
}

EXTERN void __kumir_set_current_line_number(const int32_t line_no) {
  __kumir_current_line_number = line_no;
}

EXTERN void __kumir_check_value_defined(const __kumir_scalar *value) {
  if (value == nullptr || !value->defined) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Нет значения у величины"));
  }
}

EXTERN __kumir_bool __kumir_scalar_as_bool(const __kumir_scalar *scalar) {
  __kumir_check_value_defined(scalar);
  return scalar->data.b;
}

EXTERN void __kumir_free_scalar(__kumir_scalar *scalar) {
  if (scalar->defined && __KUMIR_STRING == scalar->type && scalar->data.s) {
    free(scalar->data.s);
    scalar->data.s = 0;
  } else if (scalar->defined && __KUMIR_RECORD == scalar->type) {
    __kumir_scalar_type *types = scalar->data.u.types;
    __kumir_variant *fields =
        reinterpret_cast<__kumir_variant *>(scalar->data.u.fields);
    for (size_t i = 0; i < scalar->data.u.nfields; i++) {
      if (__KUMIR_STRING == types[i]) {
        free(fields[i].s);
      }
    }
    free(scalar->data.u.types);
    free(scalar->data.u.fields);
    scalar->data.u.types = 0;
    scalar->data.u.fields = 0;
  }
  scalar->defined = false;
}

EXTERN void __kumir_input_stdin(const __kumir_int format,
                                __kumir_scalar **pptr) {
  __kumir_scalar *ptr = *pptr;
  unsigned int f = static_cast<unsigned int>(format);
  if (__KUMIR_CHAR == f) {
    __kumir_char c = Kumir::IO::readChar();
    ptr->defined = true;
    ptr->type = __KUMIR_CHAR;
    ptr->data.c = c;
  } else if (__KUMIR_REAL == f) {
    __kumir_real r = Kumir::IO::readReal();
    __kumir_create_real(ptr, r);
  } else if (__KUMIR_INT == f) {
    __kumir_int v = Kumir::IO::readInteger();
    __kumir_create_int(ptr, v);
  } else if (__KUMIR_BOOL == f) {
    __kumir_bool b = Kumir::IO::readBool();
    __kumir_create_bool(ptr, b);
  } else if (__KUMIR_STRING == f) {
    std::wstring ws = Kumir::IO::readLine();
    __kumir_create_string(ptr, ws);
  }
}

EXTERN void __kumir_input_file(const __kumir_scalar *handle,
                               const __kumir_int format,
                               __kumir_scalar **pptr) {
  if (!handle->defined) {
    __kumir_abort_on_error("Нет значения у ключа файла");
  }
  Kumir::FileType ft = __kumir_scalar_to_file_type(*handle);

  unsigned int f = static_cast<unsigned int>(format);
  __kumir_scalar *ptr = *pptr;
  if (__KUMIR_CHAR == f) {
    __kumir_char c = Kumir::IO::readChar(ft, false);
    ptr->defined = true;
    ptr->type = __KUMIR_CHAR;
    ptr->data.c = c;
  } else if (__KUMIR_REAL == f) {
    __kumir_real r = Kumir::IO::readReal(ft, false);
    __kumir_create_real(ptr, r);
  } else if (__KUMIR_INT == f) {
    __kumir_int v = Kumir::IO::readInteger(ft, false);
    __kumir_create_int(ptr, v);
  } else if (__KUMIR_BOOL == f) {
    __kumir_bool b = Kumir::IO::readBool(ft, false);
    __kumir_create_bool(ptr, b);
  } else if (__KUMIR_STRING == f) {
    std::wstring ws = Kumir::IO::readLine(ft, false);
    __kumir_create_string(ptr, ws);
  }
}

static std::wstring __kumir_program_directory;
static std::list<std::wstring> __kumir_main_arguments;
static bool __kumir_pipe_mode = false;

EXTERN void __kumir_set_main_arguments(int argc, char **argv) {
  Kumir::EncodingError encodingError;
#if defined(WIN32) || defined(_WIN32)
  __kumir_program_directory =
      Kumir::Coder::decode(Kumir::CP1251, std::string(argv[0]), encodingError);
  const std::wstring::size_type slashPos =
      __kumir_program_directory.find_last_of(L'\\');
#else
  __kumir_program_directory =
      Kumir::Coder::decode(Kumir::UTF8, std::string(argv[0]), encodingError);
  const std::wstring::size_type slashPos =
      __kumir_program_directory.find_last_of(L'/');
#endif
  if (slashPos != std::wstring::npos) {
    __kumir_program_directory.resize(slashPos);
  } else {
    __kumir_program_directory.clear();
  }
  for (int i = 1; i < argc; i++) {
    const std::string arg = std::string(argv[i]);
    const Kumir::String warg = Kumir::Core::fromUtf8(arg);
    static const Kumir::String PIPE_MODE_SHORT =
        Kumir::Core::fromAscii("--kumir-p");
    static const Kumir::String PIPE_MODE_LONG =
        Kumir::Core::fromAscii("--kumir-pipe");
    if (warg == PIPE_MODE_SHORT || warg == PIPE_MODE_LONG) {
      __kumir_pipe_mode = true;
    } else {
      __kumir_main_arguments.push_back(warg);
    }
  }
}

#define IS_HEX(x)                                                              \
  ((x >= '0' && x <= '9') || (x >= 'A' && x <= 'F') || (x >= 'a' && x <= 'f'))

static std::wstring __kumir_decodeHttpStringValue(const std::string &s) {
  std::wstring result;
  size_t cpos = 0;
  std::string utf8string;
  utf8string.reserve(s.length());
  Kumir::EncodingError encodingError;
  while (cpos < s.length()) {
    if (s[cpos] == '%' && cpos + 2 < s.length() && IS_HEX(s[cpos + 1]) &&
        IS_HEX(s[cpos + 2])) {
      std::string hexcode = std::string("0x") + s.substr(cpos + 1, 2);
      bool ok;
      int value = Kumir::Converter::stringToInt(
          Kumir::Coder::decode(Kumir::ASCII, hexcode, encodingError), ok);
      char ch = (char)value;
      utf8string.push_back(ch);
      cpos += 3;
    } else {
      utf8string.push_back(s[cpos]);
      cpos += 1;
    }
  }
  result = Kumir::Coder::decode(Kumir::UTF8, utf8string, encodingError);
  return result;
}

EXTERN void __kumir_get_scalar_argument(const char *argNameUtf8,
                                        const __kumir_int format,
                                        __kumir_scalar *res) {
  using namespace Kumir;
  IO::InputStream stream;
  const String name = Core::fromUtf8(std::string(argNameUtf8));
  bool foundValue = false;
#if !defined(WIN32) && !defined(_WIN32)
  char *REQUEST_METHOD = getenv("REQUEST_METHOD");
  char *QUERY_STRING = getenv("QUERY_STRING");
  if (REQUEST_METHOD && std::string(REQUEST_METHOD) == std::string("GET") &&
      QUERY_STRING) {
    String query_string =
        __kumir_decodeHttpStringValue(std::string(QUERY_STRING));
    StringList pairs = Core::splitString(query_string, Char('&'), true);
    for (size_t i = 0; i < pairs.size(); i++) {
      StringList apair = Core::splitString(pairs[i], Char('='), true);
      if (apair.size() == 2) {
        String aname = apair[0];
        String avalue = apair[1];
        if (aname == name) {
          stream = IO::InputStream(avalue);
          foundValue = true;
          break;
        }
      }
    }
  }
#endif
  if (!foundValue && !__kumir_main_arguments.empty()) {
    const String arg = __kumir_main_arguments.front();
    __kumir_main_arguments.pop_front();
    stream = IO::InputStream(arg);
    foundValue = true;
  }
  if (!foundValue) {
    if (!__kumir_pipe_mode) {
      __kumir_output_stdout("Введите ");
      __kumir_output_stdout(argNameUtf8);
      __kumir_output_stdout(": ");
    }
    stream = IO::InputStream(stdin, UTF8);
  }
  // Kumir::Core::AbortHandlerType = nullptr; // temporary disable
  if (__KUMIR_INT == format) {
    int val = IO::readInteger(stream);
    __kumir_create_int(res, val);
  } else if (__KUMIR_REAL == format) {
    double val = IO::readInteger(stream);
    __kumir_create_real(res, val);
  } else if (__KUMIR_BOOL == format) {
    bool val = IO::readBool(stream);
    __kumir_create_bool(res, val);
  } else if (__KUMIR_CHAR == format) {
    Char val = IO::readChar(stream);
    __kumir_create_char(res, val);
  } else if (__KUMIR_STRING == format) {
    String val = IO::readString(stream);
    __kumir_create_string(res, val);
  }
  // Kumir::Core::AbortHandlerType = __kumir_handle_abort; // enable handler
  // again
  if (Core::getError().size() > 0) {
    __kumir_abort_on_error(
        "Не все аргументы первого алгоритма введены корректно");
  }
}

EXTERN void __kumir_get_array_argument(const char *argName,
                                       const __kumir_int format,
                                       __kumir_array *res) {
  std::wstring wn = Kumir::Core::fromUtf8(std::string(argName));
  Kumir::EncodingError encodingError;
  if (1u == res->dim) {
    int32_t start_x = res->shape_left[0];
    int32_t end_x = res->shape_right[0];
    for (int32_t x = start_x; x <= end_x; x++) {
      Kumir::String qwn = wn + Kumir::Core::fromAscii("[");
      qwn += Kumir::Converter::intToString(x);
      qwn += Kumir::Core::fromAscii("]");
      std::string utf8n = Kumir::Coder::encode(Kumir::UTF8, qwn, encodingError);
      __kumir_scalar *elem = 0;
      __kumir_scalar **elem_ptr = &elem;
      __kumir_scalar xx;
      __kumir_create_int(&xx, x);
      __kumir_get_array_1_element(elem_ptr, false, res, &xx);
      __kumir_get_scalar_argument(utf8n.c_str(), format, elem);
    }
  } else if (2u == res->dim) {
    int32_t start_y = res->shape_left[0];
    int32_t end_y = res->shape_right[0];
    for (int32_t y = start_y; y <= end_y; y++) {
      int32_t start_x = res->shape_left[1];
      int32_t end_x = res->shape_right[1];
      for (int32_t x = start_x; x <= end_x; x++) {
        Kumir::String qwn = wn + Kumir::Core::fromAscii("[");
        qwn += Kumir::Converter::intToString(y);
        qwn += Kumir::Core::fromAscii(",");
        qwn += Kumir::Converter::intToString(x);
        qwn += Kumir::Core::fromAscii("]");
        std::string utf8n =
            Kumir::Coder::encode(Kumir::UTF8, qwn, encodingError);
        __kumir_scalar *elem = 0;
        __kumir_scalar **elem_ptr = &elem;
        __kumir_scalar xx, yy;
        __kumir_create_int(&xx, x);
        __kumir_create_int(&yy, y);
        __kumir_get_array_2_element(elem_ptr, false, res, &yy, &xx);
        __kumir_get_scalar_argument(utf8n.c_str(), format, elem);
      }
    }
  } else if (3u == res->dim) {
    int32_t start_z = res->shape_left[0];
    int32_t end_z = res->shape_right[0];
    for (int32_t z = start_z; z <= end_z; z++) {
      int32_t start_y = res->shape_left[0];
      int32_t end_y = res->shape_right[0];
      for (int32_t y = start_y; y <= end_y; y++) {
        int32_t start_x = res->shape_left[1];
        int32_t end_x = res->shape_right[1];
        for (int32_t x = start_x; x <= end_x; x++) {
          Kumir::String qwn = wn + Kumir::Core::fromAscii("[");
          qwn += Kumir::Converter::intToString(z);
          qwn += Kumir::Core::fromAscii(",");
          qwn += Kumir::Converter::intToString(y);
          qwn += Kumir::Core::fromAscii(",");
          qwn += Kumir::Converter::intToString(x);
          qwn += Kumir::Core::fromAscii("]");
          std::string utf8n =
              Kumir::Coder::encode(Kumir::UTF8, qwn, encodingError);
          __kumir_scalar *elem = 0;
          __kumir_scalar **elem_ptr = &elem;
          __kumir_scalar xx, yy, zz;
          __kumir_create_int(&xx, x);
          __kumir_create_int(&yy, y);
          __kumir_create_int(&zz, y);
          __kumir_get_array_3_element(elem_ptr, false, res, &zz, &yy, &xx);
          __kumir_get_scalar_argument(utf8n.c_str(), format, elem);
        }
      }
    }
  }
}

EXTERN void __kumir_output_stdout(const char *utf8) {
  __kumir_scalar sc;
  __kumir_create_string(&sc, utf8);
  __kumir_output_stdout_ii(&sc, __KUMIR_STRING, 0, 0);
  __kumir_free_scalar(&sc);
}

EXTERN void Files_set_encoding(const __kumir_scalar *encoding) {
  __kumir_check_value_defined(encoding);
  const std::wstring enc = __kumir_scalar_as_wstring(encoding);
  Kumir::Files::setFileEncoding(enc);
}

EXTERN void Files_get_console(__kumir_scalar *result) {
  const Kumir::FileType console = Kumir::Files::getConsoleBuffer();
  *result = __kumir_file_type_to_scalar(console);
}

EXTERN void __kumir_print_scalar_variable(const char *name,
                                          const __kumir_scalar_type type,
                                          const __kumir_scalar *value) {
  if (value->defined) {
    if (!__kumir_pipe_mode) {
      const std::wstring wname = Kumir::Core::fromUtf8(name);
      const std::wstring msg =
          wname.empty() ? Kumir::Core::fromUtf8("Значение функции = ")
                        : wname + Kumir::Core::fromAscii(" = ");
      Kumir::IO::writeString(0, msg);
    }
    __kumir_output_stdout_ii(value, type, 0, 0);
  }
}

EXTERN void __kumir_input_scalar_variable(const char *name,
                                          const __kumir_scalar_type format,
                                          __kumir_scalar *ptr) {
  const std::wstring msg = Kumir::Core::fromUtf8("Введите ") +
                           Kumir::Core::fromUtf8(name) +
                           Kumir::Core::fromAscii(": ");

  Kumir::IO::writeString(0, msg);

  unsigned int f = static_cast<unsigned int>(format);
  if (__KUMIR_CHAR == f) {
    __kumir_char c = Kumir::IO::readChar();
    ptr->defined = true;
    ptr->type = __KUMIR_CHAR;
    ptr->data.c = c;
  } else if (__KUMIR_REAL == f) {
    __kumir_real r = Kumir::IO::readReal();
    __kumir_create_real(ptr, r);
  } else if (__KUMIR_INT == f) {
    __kumir_int v = Kumir::IO::readInteger();
    __kumir_create_int(ptr, v);
  } else if (__KUMIR_BOOL == f) {
    __kumir_bool b = Kumir::IO::readBool();
    __kumir_create_bool(ptr, b);
  } else if (__KUMIR_STRING == f) {
    std::wstring ws = Kumir::IO::readString();
    __kumir_create_string(ptr, ws);
  }
}

EXTERN void __kumir_print_array_variable(const char *name,
                                         const __kumir_scalar_type type,
                                         const __kumir_array *value) {
  // TODO implement me
}

EXTERN void __kumir_input_array_variable(const char *name,
                                         const __kumir_scalar_type type,
                                         __kumir_array *ptr) {
  // TODO implement me
}

EXTERN void __kumir_output_stdout_ii(const __kumir_scalar *value,
                                     const __kumir_scalar_type type,
                                     const int format1, const int format2) {
  __kumir_check_value_defined(value);
  switch (type) {
  case __KUMIR_INT:
    Kumir::IO::writeInteger(format2, value->data.i);
    break;
  case __KUMIR_REAL:
    Kumir::IO::writeReal(format2, format1, __kumir_scalar_as_real(value));
    break;
  case __KUMIR_BOOL:
    Kumir::IO::writeBool(format2, value->data.b);
    break;
  case __KUMIR_CHAR:
    Kumir::IO::writeChar(format2, value->data.c);
    break;
  case __KUMIR_STRING:
    Kumir::IO::writeString(format2, __kumir_scalar_as_wstring(value));
    break;
  default:
    break;
  }
}

EXTERN void __kumir_output_stdout_is(const __kumir_scalar *value,
                                     const __kumir_scalar_type type,
                                     const int format1,
                                     const __kumir_scalar *format2) {
  __kumir_check_value_defined(format2);
  __kumir_output_stdout_ii(value, type, format1, format2->data.i);
}

EXTERN void __kumir_output_stdout_si(const __kumir_scalar *value,
                                     const __kumir_scalar_type type,
                                     const __kumir_scalar *format1,
                                     const int format2) {
  __kumir_check_value_defined(format1);
  __kumir_output_stdout_ii(value, type, format1->data.i, format2);
}

EXTERN void __kumir_output_stdout_ss(const __kumir_scalar *value,
                                     const __kumir_scalar_type type,
                                     const __kumir_scalar *format1,
                                     const __kumir_scalar *format2) {
  __kumir_check_value_defined(format1);
  __kumir_check_value_defined(format2);
  __kumir_output_stdout_ii(value, type, format1->data.i, format2->data.i);
}

EXTERN void __kumir_output_file_ii(const __kumir_scalar *handle,
                                   const __kumir_scalar *value,
                                   const __kumir_scalar_type type,
                                   const int format1, const int format2) {
  if (!handle->defined) {
    __kumir_abort_on_error("Нет значения у ключа файла");
  }
  __kumir_check_value_defined(value);
  Kumir::FileType f = __kumir_scalar_to_file_type(*handle);
  switch (type) {
  case __KUMIR_INT:
    Kumir::IO::writeInteger(format2, value->data.i, f, false);
    break;
  case __KUMIR_REAL:
    Kumir::IO::writeReal(format2, format1, __kumir_scalar_as_real(value), f,
                         false);
    break;
  case __KUMIR_BOOL:
    Kumir::IO::writeBool(format2, value->data.b, f, false);
    break;
  case __KUMIR_CHAR:
    Kumir::IO::writeChar(format2, value->data.c, f, false);
    break;
  case __KUMIR_STRING:
    Kumir::IO::writeString(format2, __kumir_scalar_as_wstring(value), f, false);
    break;
  default:
    break;
  }
}

EXTERN void __kumir_output_file_is(const __kumir_scalar *handle,
                                   const __kumir_scalar *value,
                                   const __kumir_scalar_type type,
                                   const int format1,
                                   const __kumir_scalar *format2) {
  __kumir_check_value_defined(format2);
  __kumir_output_file_ii(handle, value, type, format1, format2->data.i);
}

EXTERN void __kumir_output_file_si(const __kumir_scalar *handle,
                                   const __kumir_scalar *value,
                                   const __kumir_scalar_type type,
                                   const __kumir_scalar *format1,
                                   const int format2) {
  __kumir_check_value_defined(format1);
  __kumir_output_file_ii(handle, value, type, format1->data.i, format2);
}

EXTERN void __kumir_output_file_ss(const __kumir_scalar *handle,
                                   const __kumir_scalar *value,
                                   const __kumir_scalar_type type,
                                   const __kumir_scalar *format1,
                                   const __kumir_scalar *format2) {
  __kumir_check_value_defined(format1);
  __kumir_check_value_defined(format2);
  __kumir_output_file_ii(handle, value, type, format1->data.i, format2->data.i);
}

static __kumir_scalar __kumir_file_type_to_scalar(const Kumir::FileType &f) {
  __kumir_scalar result;
  result.defined = true;
  result.type = __KUMIR_RECORD;
  result.data.u.nfields = 4u;
  __kumir_scalar_type *types = reinterpret_cast<__kumir_scalar_type *>(
      calloc(result.data.u.nfields, sizeof(__kumir_scalar_type)));
  __kumir_variant *fields = reinterpret_cast<__kumir_variant *>(
      calloc(result.data.u.nfields, sizeof(__kumir_variant)));

  result.data.u.fields = fields;
  result.data.u.types = types;

  types[0] = __KUMIR_STRING;
  types[1] = __KUMIR_INT;
  types[2] = __KUMIR_INT;
  types[3] = __KUMIR_BOOL;

  fields[0].s = reinterpret_cast<wchar_t *>(
      calloc(f.fullPath.length() + 1u, sizeof(wchar_t)));
  wcsncpy(fields[0].s, f.fullPath.c_str(), f.fullPath.length());
  fields[0].s[f.fullPath.length()] = L'\0';

  fields[1].i = f.mode;
  fields[2].i = f.type;
  fields[3].b = f.valid;

  return result;
}

static Kumir::FileType
__kumir_scalar_to_file_type(const __kumir_scalar &scalar) {
  Kumir::FileType f;
  __kumir_variant *fields =
      reinterpret_cast<__kumir_variant *>(scalar.data.u.fields);
  f.fullPath = std::wstring(fields[0].s);
  f.mode = fields[1].i;
  f.type = fields[2].i;
  f.valid = fields[3].b;
  return f;
}

static void __kumir_create_string(__kumir_scalar *result,
                                  const std::wstring &wstr) {
  result->type = __KUMIR_STRING;
  result->defined = true;
  result->data.s =
      reinterpret_cast<wchar_t *>(calloc(wstr.length() + 1, sizeof(wchar_t)));
  result->data.s[wstr.length()] = L'\0';
  wcsncpy(result->data.s, wstr.c_str(), wstr.length());
}

// Math

EXTERN void Kumir_Standard_Library_div(__kumir_scalar *result,
                                       const __kumir_scalar *left,
                                       const __kumir_scalar *right) {
  __kumir_check_value_defined(left);
  __kumir_check_value_defined(right);
  __kumir_create_int(result, Kumir::Math::div(left->data.i, right->data.i));
}

EXTERN void Kumir_Standard_Library_mod(__kumir_scalar *result,
                                       const __kumir_scalar *left,
                                       const __kumir_scalar *right) {
  __kumir_check_value_defined(left);
  __kumir_check_value_defined(right);
  __kumir_create_int(result, Kumir::Math::mod(left->data.i, right->data.i));
}

EXTERN void Kumir_Standard_Library_ln(__kumir_scalar *result,
                                      const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_real(result, Kumir::Math::ln(__kumir_scalar_as_real(value)));
}

EXTERN void Kumir_Standard_Library_lg(__kumir_scalar *result,
                                      const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_real(result, Kumir::Math::lg(__kumir_scalar_as_real(value)));
}

EXTERN void Kumir_Standard_Library_exp(__kumir_scalar *result,
                                       const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_real(result, Kumir::Math::exp(__kumir_scalar_as_real(value)));
}

EXTERN void Kumir_Standard_Library_rnd(__kumir_scalar *result,
                                       const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_real(result,
                      Kumir::Random::rrnd(__kumir_scalar_as_real(value)));
}

EXTERN void Kumir_Standard_Library_irnd(__kumir_scalar *result,
                                        const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_int(result, Kumir::Random::irnd(value->data.i));
}

EXTERN void Kumir_Standard_Library_rand(__kumir_scalar *result,
                                        const __kumir_scalar *a,
                                        const __kumir_scalar *b) {
  __kumir_check_value_defined(a);
  __kumir_check_value_defined(b);
  __kumir_create_real(result, Kumir::Random::rrand(__kumir_scalar_as_real(a),
                                                   __kumir_scalar_as_real(b)));
}

EXTERN void Kumir_Standard_Library_irand(__kumir_scalar *result,
                                         const __kumir_scalar *a,
                                         const __kumir_scalar *b) {
  __kumir_check_value_defined(a);
  __kumir_check_value_defined(b);
  __kumir_create_int(result, Kumir::Random::irand(a->data.i, b->data.i));
}

EXTERN void Kumir_Standard_Library_iabs(__kumir_scalar *result,
                                        const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_int(result, Kumir::Math::iabs(value->data.i));
}

EXTERN void Kumir_Standard_Library_abs(__kumir_scalar *result,
                                       const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_real(result, Kumir::Math::abs(__kumir_scalar_as_real(value)));
}

EXTERN void Kumir_Standard_Library_sign(__kumir_scalar *result,
                                        const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_int(result, Kumir::Math::sign(__kumir_scalar_as_real(value)));
}

EXTERN void Kumir_Standard_Library_int(__kumir_scalar *result,
                                       const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_int(result, Kumir::Math::intt(__kumir_scalar_as_real(value)));
}

EXTERN void Kumir_Standard_Library_arcsin(__kumir_scalar *result,
                                          const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_real(result,
                      Kumir::Math::arcsin(__kumir_scalar_as_real(value)));
}

EXTERN void Kumir_Standard_Library_arccos(__kumir_scalar *result,
                                          const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_real(result,
                      Kumir::Math::arccos(__kumir_scalar_as_real(value)));
}

EXTERN void Kumir_Standard_Library_arctg(__kumir_scalar *result,
                                         const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_real(result,
                      Kumir::Math::arctg(__kumir_scalar_as_real(value)));
}

EXTERN void Kumir_Standard_Library_arcctg(__kumir_scalar *result,
                                          const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_real(result,
                      Kumir::Math::arcctg(__kumir_scalar_as_real(value)));
}

EXTERN void Kumir_Standard_Library_tg(__kumir_scalar *result,
                                      const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_real(result, Kumir::Math::tg(__kumir_scalar_as_real(value)));
}

EXTERN void Kumir_Standard_Library_ctg(__kumir_scalar *result,
                                       const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_real(result, Kumir::Math::ctg(__kumir_scalar_as_real(value)));
}

EXTERN void Kumir_Standard_Library_sin(__kumir_scalar *result,
                                       const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_real(result, Kumir::Math::sin(__kumir_scalar_as_real(value)));
}

EXTERN void Kumir_Standard_Library_cos(__kumir_scalar *result,
                                       const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_real(result, Kumir::Math::cos(__kumir_scalar_as_real(value)));
}

EXTERN void Kumir_Standard_Library_sqrt(__kumir_scalar *result,
                                        const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_real(result, Kumir::Math::sqrt(__kumir_scalar_as_real(value)));
}

EXTERN void Kumir_Standard_Library_max(__kumir_scalar *result,
                                       const __kumir_scalar *value1,
                                       const __kumir_scalar *value2) {
  __kumir_check_value_defined(value1);
  __kumir_check_value_defined(value2);
  __kumir_create_real(result,
                      Kumir::Math::rmax(__kumir_scalar_as_real(value1),
                                        __kumir_scalar_as_real(value2)));
}

EXTERN void Kumir_Standard_Library_min(__kumir_scalar *result,
                                       const __kumir_scalar *value1,
                                       const __kumir_scalar *value2) {
  __kumir_check_value_defined(value1);
  __kumir_check_value_defined(value2);
  __kumir_create_real(result,
                      Kumir::Math::rmin(__kumir_scalar_as_real(value1),
                                        __kumir_scalar_as_real(value2)));
}

EXTERN void Kumir_Standard_Library_imax(__kumir_scalar *result,
                                        const __kumir_scalar *value1,
                                        const __kumir_scalar *value2) {
  __kumir_check_value_defined(value1);
  __kumir_check_value_defined(value2);
  __kumir_create_int(result, Kumir::Math::imax(value1->data.i, value2->data.i));
}

EXTERN void Kumir_Standard_Library_imin(__kumir_scalar *result,
                                        const __kumir_scalar *value1,
                                        const __kumir_scalar *value2) {
  __kumir_check_value_defined(value1);
  __kumir_check_value_defined(value2);
  __kumir_create_int(result, Kumir::Math::imin(value1->data.i, value2->data.i));
}

EXTERN void Kumir_Standard_Library_string_of_int(__kumir_scalar *result,
                                                 const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_string(result, Kumir::Converter::intToString(value->data.i));
}

EXTERN void Kumir_Standard_Library_string_of_real(__kumir_scalar *result,
                                                  const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_string(
      result, Kumir::Converter::realToString(__kumir_scalar_as_real(value)));
}

EXTERN void Kumir_Standard_Library_length(__kumir_scalar *result,
                                          const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_int(result, __kumir_scalar_as_wstring(value).length());
}

EXTERN void Kumir_Standard_Library_unicode(__kumir_scalar *result,
                                           const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_int(result, Kumir::StringUtils::unicode(value->data.c));
}

EXTERN void Kumir_Standard_Library_code(__kumir_scalar *result,
                                        const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  __kumir_create_int(result, Kumir::StringUtils::code(value->data.c));
}

EXTERN void Kumir_Standard_Library_symbol(__kumir_scalar *result,
                                          const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  result->defined = true;
  result->type = __KUMIR_CHAR;
  result->data.c = Kumir::StringUtils::symbol(value->data.i);
}

EXTERN void Kumir_Standard_Library_unisymbol(__kumir_scalar *result,
                                             const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  result->defined = true;
  result->type = __KUMIR_CHAR;
  result->data.c = Kumir::StringUtils::unisymbol(value->data.i);
}

EXTERN void String_Utilities_index_after(__kumir_scalar *result,
                                         const __kumir_scalar *ot,
                                         const __kumir_scalar *fragment,
                                         const __kumir_scalar *thestring) {
  String_Utilities_find_after(result, ot, fragment, thestring);
}

EXTERN void String_Utilities_find_after(__kumir_scalar *result,
                                        const __kumir_scalar *ot,
                                        const __kumir_scalar *fragment,
                                        const __kumir_scalar *thestring) {
  __kumir_check_value_defined(ot);
  __kumir_check_value_defined(fragment);
  __kumir_check_value_defined(thestring);
  const int from = ot->data.i;
  const std::wstring pattern = __kumir_scalar_as_wstring(fragment);
  const std::wstring source = __kumir_scalar_as_wstring(thestring);
  __kumir_create_int(result,
                     Kumir::StringUtils::find(from + 1, pattern, source));
}

EXTERN void String_Utilities_index(__kumir_scalar *result,
                                   const __kumir_scalar *fragment,
                                   const __kumir_scalar *thestring) {
  String_Utilities_find(result, fragment, thestring);
}

EXTERN void String_Utilities_find(__kumir_scalar *result,
                                  const __kumir_scalar *fragment,
                                  const __kumir_scalar *thestring) {
  __kumir_check_value_defined(fragment);
  __kumir_check_value_defined(thestring);
  const std::wstring pattern = __kumir_scalar_as_wstring(fragment);
  const std::wstring source = __kumir_scalar_as_wstring(thestring);
  __kumir_create_int(result, Kumir::StringUtils::find(1, pattern, source));
}

EXTERN void String_Utilities_lowercase(__kumir_scalar *result,
                                       const __kumir_scalar *thestring) {
  __kumir_check_value_defined(thestring);
  const std::wstring source = __kumir_scalar_as_wstring(thestring);
  __kumir_create_string(result, Kumir::StringUtils::toLowerCase(source));
}

EXTERN void String_Utilities_uppercase(__kumir_scalar *result,
                                       const __kumir_scalar *thestring) {
  __kumir_check_value_defined(thestring);
  const std::wstring source = __kumir_scalar_as_wstring(thestring);
  __kumir_create_string(result, Kumir::StringUtils::toLowerCase(source));
}

EXTERN void String_Utilities_insert(const __kumir_scalar *fragment,
                                    __kumir_scalar *thestring,
                                    const __kumir_scalar *position) {
  __kumir_check_value_defined(fragment);
  __kumir_check_value_defined(thestring);
  __kumir_check_value_defined(position);
  const std::wstring sub = __kumir_scalar_as_wstring(fragment);
  const int pos = position->data.i;
  std::wstring s = __kumir_scalar_as_wstring(thestring);
  Kumir::StringUtils::insert(sub, s, pos);
  __kumir_free_scalar(thestring);
  __kumir_create_string(thestring, s);
}

EXTERN void String_Utilities_remove(__kumir_scalar *thestring,
                                    const __kumir_scalar *position,
                                    const __kumir_scalar *thelength) {
  __kumir_check_value_defined(thestring);
  __kumir_check_value_defined(position);
  __kumir_check_value_defined(thelength);
  std::wstring s = __kumir_scalar_as_wstring(thestring);
  int pos = position->data.i;
  int len = thelength->data.i;
  Kumir::StringUtils::remove(s, pos, len);
  __kumir_free_scalar(thestring);
  __kumir_create_string(thestring, s);
}

EXTERN void String_Utilities_replace(__kumir_scalar *source,
                                     const __kumir_scalar *before,
                                     const __kumir_scalar *after,
                                     const __kumir_scalar *each) {
  __kumir_check_value_defined(source);
  __kumir_check_value_defined(before);
  __kumir_check_value_defined(after);
  __kumir_check_value_defined(each);
  std::wstring s = __kumir_scalar_as_wstring(source);
  const std::wstring b = __kumir_scalar_as_wstring(before);
  const std::wstring a = __kumir_scalar_as_wstring(after);
  const bool e = __kumir_scalar_as_bool(each);
  Kumir::StringUtils::replace(s, b, a, e);
  __kumir_free_scalar(source);
  __kumir_create_string(source, s);
}

EXTERN void Kumir_Standard_Library_string_to_real(__kumir_scalar *result,
                                                  const __kumir_scalar *value,
                                                  __kumir_scalar *success) {
  __kumir_check_value_defined(value);
  bool ok;
  double res =
      Kumir::Converter::stringToReal(__kumir_scalar_as_wstring(value), ok);
  __kumir_create_real(result, res);
  __kumir_create_bool(success, ok);
}

EXTERN void Kumir_Standard_Library_string_to_int(__kumir_scalar *result,
                                                 const __kumir_scalar *value,
                                                 __kumir_scalar *success) {
  __kumir_check_value_defined(value);
  bool ok;
  int res = Kumir::Converter::stringToInt(__kumir_scalar_as_wstring(value), ok);
  __kumir_create_int(result, res);
  __kumir_create_bool(success, ok);
}

EXTERN void Kumir_Standard_Library_Integer(__kumir_scalar *result,
                                           const __kumir_scalar *lexem,
                                           const __kumir_scalar *def) {
  __kumir_check_value_defined(lexem);
  __kumir_check_value_defined(def);
  const std::wstring s = __kumir_scalar_as_wstring(lexem);
  const int d = def->data.i;
  const int v = Kumir::Converter::stringToIntDef(s, d);
  __kumir_create_int(result, v);
}

EXTERN void Kumir_Standard_Library_Real(__kumir_scalar *result,
                                        const __kumir_scalar *lexem,
                                        const __kumir_scalar *def) {
  __kumir_check_value_defined(lexem);
  __kumir_check_value_defined(def);
  const std::wstring s = __kumir_scalar_as_wstring(lexem);
  const double d = def->data.r;
  const double v = Kumir::Converter::stringToRealDef(s, d);
  __kumir_create_real(result, v);
}

EXTERN void Kumir_Standard_Library_Boolean(__kumir_scalar *result,
                                           const __kumir_scalar *lexem,
                                           const __kumir_scalar *def) {
  __kumir_check_value_defined(lexem);
  __kumir_check_value_defined(def);
  const std::wstring s = __kumir_scalar_as_wstring(lexem);
  const bool d = def->data.b;
  const bool v = Kumir::Converter::stringToBoolDef(s, d);
  __kumir_create_bool(result, v);
}

// Files
EXTERN void Files_nonempty(__kumir_scalar *result,
                           const __kumir_scalar *handle) {
  __kumir_check_value_defined(handle);
  Kumir::FileType f = __kumir_scalar_to_file_type(*handle);
  bool res = Kumir::Files::hasData(f);
  __kumir_create_bool(result, res);
}

EXTERN void Files_PROGRAM_DIR(__kumir_scalar *result) {
  const std::wstring res = __kumir_program_directory.empty()
                               ? Kumir::Files::CurrentWorkingDirectory()
                               : __kumir_program_directory;
  __kumir_create_string(result, res);
}

EXTERN void Files_CURRENT_DIR(__kumir_scalar *result) {
  const std::wstring res = Kumir::Files::CurrentWorkingDirectory();
  __kumir_create_string(result, res);
}

EXTERN void Files_read_open(__kumir_scalar *result,
                            const __kumir_scalar *name) {
  __kumir_check_value_defined(name);
  std::wstring wsname(name->data.s);
  Kumir::FileType f =
      Kumir::Files::open(wsname, Kumir::FileType::Read, true, 0);
  *result = __kumir_file_type_to_scalar(f);
}

EXTERN void Files_write_open(__kumir_scalar *result,
                             const __kumir_scalar *name) {
  __kumir_check_value_defined(name);
  std::wstring wsname(name->data.s);
  Kumir::FileType f =
      Kumir::Files::open(wsname, Kumir::FileType::Write, true, 0);
  *result = __kumir_file_type_to_scalar(f);
}

EXTERN void Files_append_open(__kumir_scalar *result,
                              const __kumir_scalar *name) {
  __kumir_check_value_defined(name);
  std::wstring wsname(name->data.s);
  Kumir::FileType f =
      Kumir::Files::open(wsname, Kumir::FileType::Append, true, 0);
  *result = __kumir_file_type_to_scalar(f);
}

EXTERN void Files_readable(__kumir_scalar *result, const __kumir_scalar *name) {
  __kumir_check_value_defined(name);
  std::wstring wsname(name->data.s);
  __kumir_create_bool(result, Kumir::Files::canOpenForRead(wsname));
}

EXTERN void Files_writable(__kumir_scalar *result, const __kumir_scalar *name) {
  __kumir_check_value_defined(name);
  std::wstring wsname(name->data.s);
  __kumir_create_bool(result, Kumir::Files::canOpenForWrite(wsname));
}

EXTERN void Files_close(const __kumir_scalar *handle) {
  __kumir_check_value_defined(handle);
  Kumir::FileType f = __kumir_scalar_to_file_type(*handle);
  Kumir::Files::close(f);
}

EXTERN void Files_operator_neq(__kumir_scalar *result, const __kumir_scalar *a,
                               const __kumir_scalar *b) {
  __kumir_check_value_defined(a);
  __kumir_check_value_defined(b);
  Kumir::FileType af = __kumir_scalar_to_file_type(*a);
  Kumir::FileType bf = __kumir_scalar_to_file_type(*b);
  bool equal = af.fullPath == bf.fullPath;
  __kumir_create_bool(result, !equal);
}

EXTERN void Files_operator_eq(__kumir_scalar *result, const __kumir_scalar *a,
                              const __kumir_scalar *b) {
  __kumir_check_value_defined(a);
  __kumir_check_value_defined(b);
  Kumir::FileType af = __kumir_scalar_to_file_type(*a);
  Kumir::FileType bf = __kumir_scalar_to_file_type(*b);
  bool equal = af.fullPath == bf.fullPath;
  __kumir_create_bool(result, equal);
}

EXTERN void Files_start_reading(const __kumir_scalar *handle) {
  __kumir_check_value_defined(handle);
  Kumir::FileType f = __kumir_scalar_to_file_type(*handle);
  Kumir::Files::reset(f);
}

EXTERN void Files_exists(__kumir_scalar *result, const __kumir_scalar *name) {
  __kumir_check_value_defined(name);
  const std::wstring sname = __kumir_scalar_as_wstring(name);
  __kumir_create_bool(result, Kumir::Files::exist(sname));
}

EXTERN void Files_isdir(__kumir_scalar *result, const __kumir_scalar *name) {
  __kumir_check_value_defined(name);
  const std::wstring sname = __kumir_scalar_as_wstring(name);
  __kumir_create_bool(result, Kumir::Files::isDirectory(sname));
}

EXTERN void Files_mkdir(__kumir_scalar *result, const __kumir_scalar *name) {
  __kumir_check_value_defined(name);
  const std::wstring sname = __kumir_scalar_as_wstring(name);
  __kumir_create_bool(result, Kumir::Files::mkdir(sname));
}

EXTERN void Files_rm(__kumir_scalar *result, const __kumir_scalar *name) {
  __kumir_check_value_defined(name);
  const std::wstring sname = __kumir_scalar_as_wstring(name);
  __kumir_create_bool(result, Kumir::Files::unlink(sname));
}

EXTERN void Files_rmdir(__kumir_scalar *result, const __kumir_scalar *name) {
  __kumir_check_value_defined(name);
  const std::wstring sname = __kumir_scalar_as_wstring(name);
  __kumir_create_bool(result, Kumir::Files::rmdir(sname));
}

EXTERN void Files_abspath(__kumir_scalar *result, const __kumir_scalar *name) {
  __kumir_check_value_defined(name);
  const std::wstring sname = __kumir_scalar_as_wstring(name);
  __kumir_create_string(result, Kumir::Files::getAbsolutePath(sname));
}

EXTERN void Files_eof(__kumir_scalar *result, const __kumir_scalar *handle) {
  __kumir_check_value_defined(handle);
  Kumir::FileType f = __kumir_scalar_to_file_type(*handle);
  __kumir_create_bool(result, Kumir::Files::eof(f));
}

EXTERN void Kumir_Standard_Library_time(__kumir_scalar *result) {
  __kumir_create_int(result, Kumir::System::time());
}

EXTERN void Kumir_Standard_Library_wait(const __kumir_scalar *value) {
  __kumir_check_value_defined(value);
  if (value->data.i < 0) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Отрицательное время"));
  }
  uint32_t msec = static_cast<uint32_t>(value->data.i);
#if defined(WIN32) || defined(_WIN32)
  Sleep(msec);
#else
  std::this_thread::sleep_for(std::chrono::milliseconds(msec));
  return;

  // uint32_t sec = msec / 1000;
  // uint32_t usec = (msec - sec * 1000) * 1000;
  //// usleep works in range [0, 1000000), so
  //// call sleep(sec) first for long periods
  // sleep(sec);
  // usleep(usec);
#endif
}

static __kumir_real __kumir_scalar_as_real(const __kumir_scalar *scalar) {
  __kumir_real result = 0.;
  if (__KUMIR_REAL == scalar->type) {
    result = scalar->data.r;
  } else if (__KUMIR_INT == scalar->type) {
    result = static_cast<__kumir_real>(scalar->data.i);
  }
  return result;
}

static std::wstring __kumir_scalar_as_wstring(const __kumir_scalar *scalar) {
  std::wstring result;
  if (__KUMIR_CHAR == scalar->type) {
    result.push_back(scalar->data.c);
  } else if (__KUMIR_STRING == scalar->type) {
    result = std::wstring(scalar->data.s);
  }
  return result;
}

static signed char __kumir_compare_scalars(const __kumir_scalar *left,
                                           const __kumir_scalar *right) {
  signed char result = 0;
  if (__KUMIR_INT == left->type && __KUMIR_INT == right->type) {
    const __kumir_int l = left->data.i;
    const __kumir_int r = right->data.i;
    if (l < r)
      result = 1;
    else if (l > r)
      result = -1;
    else if (l == r)
      result = 0;
  } else if (__KUMIR_REAL == left->type || __KUMIR_REAL == right->type) {
    const __kumir_real l = __kumir_scalar_as_real(left);
    const __kumir_real r = __kumir_scalar_as_real(right);
    if (l < r)
      result = 1;
    else if (l > r)
      result = -1;
    else if (l == r)
      result = 0;
  } else if (__KUMIR_BOOL == left->type && __KUMIR_BOOL == right->type) {
    const __kumir_bool l = left->data.b;
    const __kumir_bool r = right->data.b;
    if (l == r)
      result = 0;
    else
      result = 1;
  } else if (__KUMIR_STRING == left->type || __KUMIR_STRING == right->type) {
    const std::wstring l = __kumir_scalar_as_wstring(left);
    const std::wstring r = __kumir_scalar_as_wstring(right);
    if (l < r)
      result = 1;
    else if (l > r)
      result = -1;
    else if (l == r)
      result = 0;
  } else if (__KUMIR_CHAR == left->type && __KUMIR_CHAR == right->type) {
    const __kumir_char l = left->data.c;
    const __kumir_char r = right->data.c;
    if (l < r)
      result = 1;
    else if (l > r)
      result = -1;
    else if (l == r)
      result = 0;
  } else if (__KUMIR_RECORD == left->type && __KUMIR_RECORD == right->type &&
             left->data.u.nfields == right->data.u.nfields) {
    // Check for equality only, otherwise return 1
    result = 0;
    const __kumir_variant *L =
        reinterpret_cast<const __kumir_variant *>(left->data.u.fields);
    const __kumir_variant *R =
        reinterpret_cast<const __kumir_variant *>(right->data.u.fields);
    const __kumir_scalar_type *LT = left->data.u.types;
    const __kumir_scalar_type *RT = right->data.u.types;
    for (size_t i = 0u; i < left->data.u.nfields; i++) {
      __kumir_scalar l, r;
      l.defined = true;
      l.data = L[i];
      l.type = LT[i];
      r.defined = true;
      r.data = R[i];
      r.type = RT[i];
      signed char field_result = __kumir_compare_scalars(&l, &r);
      if (field_result != 0) {
        return 1;
      }
    }
  }
  return result;
}

EXTERN void __kumir_operator_eq(__kumir_scalar *result,
                                const __kumir_scalar *left,
                                const __kumir_scalar *right) {
  __kumir_check_value_defined(left);
  __kumir_check_value_defined(right);
  result->defined = true;
  result->type = __KUMIR_BOOL;
  signed char v = __kumir_compare_scalars(left, right);
  result->data.b = v == 0;
}

EXTERN void __kumir_operator_ls(__kumir_scalar *result,
                                const __kumir_scalar *left,
                                const __kumir_scalar *right) {
  __kumir_check_value_defined(left);
  __kumir_check_value_defined(right);

  result->defined = true;
  result->type = __KUMIR_BOOL;
  result->data.b = __kumir_compare_scalars(left, right) == 1;
}

EXTERN void __kumir_operator_gt(__kumir_scalar *result,
                                const __kumir_scalar *left,
                                const __kumir_scalar *right) {
  __kumir_check_value_defined(left);
  __kumir_check_value_defined(right);

  result->defined = true;
  result->type = __KUMIR_BOOL;
  result->data.b = __kumir_compare_scalars(left, right) == -1;
}

EXTERN void __kumir_operator_lq(__kumir_scalar *result,
                                const __kumir_scalar *left,
                                const __kumir_scalar *right) {
  __kumir_check_value_defined(left);
  __kumir_check_value_defined(right);

  result->defined = true;
  result->type = __KUMIR_BOOL;
  result->data.b = __kumir_compare_scalars(left, right) >= 0;
}

EXTERN void __kumir_operator_gq(__kumir_scalar *result,
                                const __kumir_scalar *left,
                                const __kumir_scalar *right) {
  __kumir_check_value_defined(left);
  __kumir_check_value_defined(right);

  result->defined = true;
  result->type = __KUMIR_BOOL;
  result->data.b = __kumir_compare_scalars(left, right) <= 0;
}

EXTERN void __kumir_operator_neq(__kumir_scalar *result,
                                 const __kumir_scalar *left,
                                 const __kumir_scalar *right) {
  __kumir_operator_eq(result, left, right);
  result->data.b = !result->data.b;
}

EXTERN void __kumir_operator_sum(__kumir_scalar *result,
                                 const __kumir_scalar *left,
                                 const __kumir_scalar *right) {
  __kumir_check_value_defined(left);
  __kumir_check_value_defined(right);

  if (__KUMIR_INT == left->type && __KUMIR_INT == right->type) {
    if (!Kumir::Math::checkSumm(left->data.i, right->data.i)) {
      Kumir::Core::abort(Kumir::Core::fromUtf8("Целочисленное переполнение"));
    }
    __kumir_create_int(result, left->data.i + right->data.i);
  } else if (__KUMIR_REAL == left->type || __KUMIR_REAL == right->type) {
    const __kumir_real l = __kumir_scalar_as_real(left);
    const __kumir_real r = __kumir_scalar_as_real(right);
    const __kumir_real res = l + r;
    if (!Kumir::Math::isCorrectReal(res)) {
      Kumir::Core::abort(Kumir::Core::fromUtf8("Вещественное переполнение"));
    }
    __kumir_create_real(result, res);
  } else if (__KUMIR_STRING == left->type || __KUMIR_CHAR == left->type) {
    const std::wstring l = __kumir_scalar_as_wstring(left);
    const std::wstring r = __kumir_scalar_as_wstring(right);
    const std::wstring res = l + r;
    __kumir_create_string(result, res);
  }
}

EXTERN void __kumir_operator_sub(__kumir_scalar *result,
                                 const __kumir_scalar *left,
                                 const __kumir_scalar *right) {
  __kumir_check_value_defined(left);
  __kumir_check_value_defined(right);

  if (__KUMIR_INT == left->type && __KUMIR_INT == right->type) {
    if (!Kumir::Math::checkDiff(left->data.i, right->data.i)) {
      Kumir::Core::abort(Kumir::Core::fromUtf8("Целочисленное переполнение"));
    }
    __kumir_create_int(result, left->data.i - right->data.i);
  } else if (__KUMIR_REAL == left->type || __KUMIR_REAL == right->type) {
    const __kumir_real l = __kumir_scalar_as_real(left);
    const __kumir_real r = __kumir_scalar_as_real(right);
    const __kumir_real res = l - r;
    if (!Kumir::Math::isCorrectReal(res)) {
      Kumir::Core::abort(Kumir::Core::fromUtf8("Вещественное переполнение"));
    }
    __kumir_create_real(result, res);
  }
}

EXTERN void __kumir_operator_mul(__kumir_scalar *result,
                                 const __kumir_scalar *left,
                                 const __kumir_scalar *right) {
  __kumir_check_value_defined(left);
  __kumir_check_value_defined(right);

  if (__KUMIR_INT == left->type && __KUMIR_INT == right->type) {
    if (!Kumir::Math::checkProd(left->data.i, right->data.i)) {
      Kumir::Core::abort(Kumir::Core::fromUtf8("Целочисленное переполнение"));
    }
    __kumir_create_int(result, left->data.i * right->data.i);
  } else if (__KUMIR_REAL == left->type || __KUMIR_REAL == right->type) {
    const __kumir_real l = __kumir_scalar_as_real(left);
    const __kumir_real r = __kumir_scalar_as_real(right);
    __kumir_create_real(result, l * r);
    if (!Kumir::Math::isCorrectReal(result->data.r)) {
      Kumir::Core::abort(Kumir::Core::fromUtf8("Вещественное переполнение"));
    }
  }
}

EXTERN void __kumir_operator_div(__kumir_scalar *result,
                                 const __kumir_scalar *left,
                                 const __kumir_scalar *right) {
  __kumir_check_value_defined(left);
  __kumir_check_value_defined(right);

  if ((__KUMIR_INT == right->type && 0 == right->data.i) ||
      (__KUMIR_REAL == right->type && 0. == right->data.r)) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Деление на ноль"));
  }

  const __kumir_real l = __kumir_scalar_as_real(left);
  const __kumir_real r = __kumir_scalar_as_real(right);
  __kumir_create_real(result, l / r);
  if (!Kumir::Math::isCorrectReal(result->data.r)) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Вещественное переполнение"));
  }
}

EXTERN void __kumir_operator_pow(__kumir_scalar *result,
                                 const __kumir_scalar *left,
                                 const __kumir_scalar *right) {
  __kumir_check_value_defined(left);
  __kumir_check_value_defined(right);

  if (__KUMIR_INT == left->type && __KUMIR_INT == right->type) {
    __kumir_create_int(result, Kumir::Math::ipow(left->data.i, right->data.i));
  } else {
    const __kumir_real l = __kumir_scalar_as_real(left);
    const __kumir_real r = __kumir_scalar_as_real(right);
    __kumir_create_real(result, Kumir::Math::pow(l, r));
  }
}

EXTERN void __kumir_operator_neg(__kumir_scalar *result,
                                 const __kumir_scalar *left) {
  __kumir_check_value_defined(left);
  result->defined = true;
  result->type = left->type;
  if (__KUMIR_BOOL == left->type) {
    result->data.b = !left->data.b;
  } else if (__KUMIR_INT == left->type) {
    result->data.i = -left->data.i;
  } else if (__KUMIR_REAL == left->type) {
    result->data.r = 0.0 - left->data.r;
  }
}

EXTERN void __kumir_assert(const __kumir_scalar *assumption) {
  __kumir_check_value_defined(assumption);
  bool value = assumption->data.b;
  if (!value) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Утверждение ложно"));
  }
}

static void __kumir_assert(const bool assumption, int line) {
  if (!assumption) {
    std::cerr << "Assert at " << __FILE__ << " : " << line << "\n";
    exit(120);
  }
}

EXTERN void __kumir_abort_on_error(const char *message) {
  Kumir::Core::abort(Kumir::Core::fromUtf8(std::string(message)));
}

EXTERN void __kumir_init_stdlib() {
  // Set stack size some greater...
#if defined(WIN32) || defined(_WIN32)
#else
  static const rlim_t kStackSize = 32 * 1024 * 1024;
  struct rlimit rl;
  int result;
  result = getrlimit(RLIMIT_STACK, &rl);
  if (result == 0) {
    if (rl.rlim_cur < kStackSize) {
      rl.rlim_cur = kStackSize;
      result = setrlimit(RLIMIT_STACK, &rl);
      if (result != 0) {
        std::cerr << "Warning: Can't set rlimit stack size\n";
      }
    }
  } else {
    std::cerr << "Warning: Can't get rlimit stack size\n";
  }
#endif
  // Init Standard library
  Kumir::initStandardLibrary();

  // Set abort and print message on error handler
  // Kumir::Core::AbortHandlerType = __kumir_handle_abort;
}

EXTERN void __kumir_create_array_1(__kumir_array *result,
                                   const __kumir_scalar *left_1,
                                   const __kumir_scalar *right_1) {
  __kumir_check_value_defined(left_1);
  __kumir_check_value_defined(right_1);
  result->dim = 1u;
  result->shape_left[0] = result->size_left[0] = left_1->data.i;
  result->shape_right[0] = result->size_right[0] = right_1->data.i;
  result->shape_left[1] = result->size_left[1] = 0;
  result->shape_right[1] = result->size_right[1] = 0;
  result->shape_left[2] = result->size_left[2] = 0;
  result->shape_right[2] = result->size_right[2] = 0;
  int isz = 1 + right_1->data.i - left_1->data.i;
  if (isz < 1) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Неверный размер таблицы"));
  } else {
    size_t sz = static_cast<size_t>(isz);
    if (sz) {
      __kumir_scalar *scalars = reinterpret_cast<__kumir_scalar *>(
          calloc(sz, sizeof(__kumir_scalar)));
      result->data = scalars;
      for (size_t i = 0; i < sz; i++) {
        __kumir_create_undefined_scalar(&scalars[i]);
      }
    }
  }
}

EXTERN void __kumir_create_array_2(__kumir_array *result,
                                   const __kumir_scalar *left_1,
                                   const __kumir_scalar *right_1,
                                   const __kumir_scalar *left_2,
                                   const __kumir_scalar *right_2) {
  __kumir_check_value_defined(left_1);
  __kumir_check_value_defined(right_1);
  __kumir_check_value_defined(left_2);
  __kumir_check_value_defined(right_2);
  result->dim = 2u;
  result->shape_left[0] = result->size_left[0] = left_1->data.i;
  result->shape_right[0] = result->size_right[0] = right_1->data.i;
  result->shape_left[1] = result->size_left[1] = left_2->data.i;
  result->shape_right[1] = result->size_right[1] = right_2->data.i;
  result->shape_left[2] = result->size_left[2] = 0;
  result->shape_right[2] = result->size_right[2] = 0;
  int isz1 = 1 + right_1->data.i - left_1->data.i;
  int isz2 = 1 + right_2->data.i - left_2->data.i;
  if (isz1 < 1 || isz2 < 1) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Неверный размер таблицы"));
  } else {
    size_t sz = static_cast<size_t>(isz1 * isz2);
    if (sz) {
      __kumir_scalar *scalars = reinterpret_cast<__kumir_scalar *>(
          calloc(sz, sizeof(__kumir_scalar)));
      result->data = scalars;
      for (size_t i = 0; i < sz; i++) {
        __kumir_create_undefined_scalar(&scalars[i]);
      }
    }
  }
}

EXTERN void __kumir_create_array_3(__kumir_array *result,
                                   const __kumir_scalar *left_1,
                                   const __kumir_scalar *right_1,
                                   const __kumir_scalar *left_2,
                                   const __kumir_scalar *right_2,
                                   const __kumir_scalar *left_3,
                                   const __kumir_scalar *right_3) {
  __kumir_check_value_defined(left_1);
  __kumir_check_value_defined(right_1);
  __kumir_check_value_defined(left_2);
  __kumir_check_value_defined(right_2);
  __kumir_check_value_defined(left_3);
  __kumir_check_value_defined(right_3);
  result->dim = 3u;
  result->shape_left[0] = result->size_left[0] = left_1->data.i;
  result->shape_right[0] = result->size_right[0] = right_1->data.i;
  result->shape_left[1] = result->size_left[1] = left_2->data.i;
  result->shape_right[1] = result->size_right[1] = right_2->data.i;
  result->shape_left[2] = result->size_left[2] = left_3->data.i;
  result->shape_right[2] = result->size_right[2] = right_3->data.i;
  int isz1 = 1 + right_1->data.i - left_1->data.i;
  int isz2 = 1 + right_2->data.i - left_2->data.i;
  int isz3 = 1 + right_3->data.i - left_3->data.i;
  if (isz1 < 1 || isz2 < 1 || isz3 < 1) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Неверный размер таблицы"));
  } else {
    size_t sz = static_cast<size_t>(isz1 * isz2 * isz3);
    if (sz) {
      __kumir_scalar *scalars = reinterpret_cast<__kumir_scalar *>(
          calloc(sz, sizeof(__kumir_scalar)));
      result->data = scalars;
      for (size_t i = 0; i < sz; i++) {
        __kumir_create_undefined_scalar(&scalars[i]);
      }
    }
  }
}

EXTERN void __kumir_create_array_ref_1(__kumir_array *result,
                                       const __kumir_scalar *left_1,
                                       const __kumir_scalar *right_1) {
  __kumir_check_value_defined(left_1);
  __kumir_check_value_defined(right_1);
  if (left_1->data.i > right_1->data.i) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Неверный размер таблицы"));
  }
  result->shape_left[0] = IMAX(result->shape_left[0], left_1->data.i);
  result->shape_right[0] = IMIN(result->shape_right[0], right_1->data.i);
}

EXTERN void __kumir_create_array_ref_2(__kumir_array *result,
                                       const __kumir_scalar *left_1,
                                       const __kumir_scalar *right_1,
                                       const __kumir_scalar *left_2,
                                       const __kumir_scalar *right_2) {
  __kumir_check_value_defined(left_1);
  __kumir_check_value_defined(right_1);
  __kumir_check_value_defined(left_2);
  __kumir_check_value_defined(right_2);
  if (right_1->data.i < left_1->data.i) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Неверный размер таблицы"));
  }
  if (right_2->data.i < left_2->data.i) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Неверный размер таблицы"));
  }
  result->shape_left[0] = IMAX(result->shape_left[0], left_1->data.i);
  result->shape_right[0] = IMIN(result->shape_right[0], right_1->data.i);
  result->shape_left[1] = IMAX(result->shape_left[1], left_2->data.i);
  result->shape_right[1] = IMIN(result->shape_right[1], right_2->data.i);
}

EXTERN void __kumir_create_array_ref_3(__kumir_array *result,
                                       const __kumir_scalar *left_1,
                                       const __kumir_scalar *right_1,
                                       const __kumir_scalar *left_2,
                                       const __kumir_scalar *right_2,
                                       const __kumir_scalar *left_3,
                                       const __kumir_scalar *right_3) {
  __kumir_check_value_defined(left_1);
  __kumir_check_value_defined(right_1);
  __kumir_check_value_defined(left_2);
  __kumir_check_value_defined(right_2);
  __kumir_check_value_defined(left_3);
  __kumir_check_value_defined(right_3);
  if (right_1->data.i < left_1->data.i) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Неверный размер таблицы"));
  }
  if (right_2->data.i < left_2->data.i) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Неверный размер таблицы"));
  }
  if (right_3->data.i < left_3->data.i) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Неверный размер таблицы"));
  }
  result->shape_left[0] = IMAX(result->shape_left[0], left_1->data.i);
  result->shape_right[0] = IMIN(result->shape_right[0], right_1->data.i);
  result->shape_left[1] = IMAX(result->shape_left[1], left_2->data.i);
  result->shape_right[1] = IMIN(result->shape_right[1], right_2->data.i);
  result->shape_left[2] = IMAX(result->shape_left[2], left_3->data.i);
  result->shape_right[2] = IMIN(result->shape_right[2], right_3->data.i);
}

EXTERN void __kumir_cleanup_array_in_shape(__kumir_array *result) {
  // TODO Must clean everything or just in shape ???

  // ===== clean everithing implementation
  size_t size =
      static_cast<size_t>(result->size_right[0] - result->size_left[0] + 1);
  if (result->dim > 1u) {
    size *=
        static_cast<size_t>(result->size_right[1] - result->size_left[1] + 1);
  }
  if (result->dim > 2u) {
    size *=
        static_cast<size_t>(result->size_right[2] - result->size_left[2] + 1);
  }
  for (size_t i = 0; i < size; i++) {
    __kumir_scalar *s = result->data + i;
    __kumir_free_scalar(s);
  }

  // ===== below is clean share implementation

  //    switch (result->dim) {
  //    case 1u: {
  //        const size_t start_pos = static_cast<size_t>(result->shape_left[0] -
  //        result->size_left[0]); const size_t items_count = start_pos +
  //                static_cast<size_t>(result->shape_right[0] -
  //                result->shape_left[0] + 1);
  //        for (size_t x = start_pos; x<items_count; x++) {
  //            __kumir_free_scalar(result->data + x);
  //        }
  //        break;
  //    }
  //    case 2u: {
  //        const size_t size1 = static_cast<size_t>(
  //                    1 + result->shape_right[0] - result->shape_left[0]
  //                    );

  //        const size_t size2 = static_cast<size_t>(
  //                    1 + result->shape_right[1] - result->shape_left[1]
  //                    );
  //        const size_t start_pos1 = static_cast<size_t>(result->shape_left[0]
  //        - result->size_left[0]); const size_t start_pos2 =
  //        static_cast<size_t>(result->shape_left[1] - result->size_left[1]);
  //        for (size_t y = start_pos1; y<size1; y++) {
  //            for (size_t x = start_pos2; x<size2; x++) {
  //                const size_t index = y * size1 + x;
  //                __kumir_free_scalar(result->data + index);
  //            }
  //        }
  //        break;
  //    }
  //    case 3u: {
  //        const size_t size1 = static_cast<size_t>(
  //                    1 + result->shape_right[0] - result->shape_left[0]
  //                    );

  //        const size_t size2 = static_cast<size_t>(
  //                    1 + result->shape_right[1] - result->shape_left[1]
  //                    );

  //        const size_t size3 = static_cast<size_t>(
  //                    1 + result->shape_right[2] - result->shape_left[2]
  //                    );
  //        const size_t start_pos1 = static_cast<size_t>(result->shape_left[0]
  //        - result->size_left[0]); const size_t start_pos2 =
  //        static_cast<size_t>(result->shape_left[1] - result->size_left[1]);
  //        const size_t start_pos3 = static_cast<size_t>(result->shape_left[2]
  //        - result->size_left[2]); for (size_t z = start_pos1; z<size1; z++) {
  //            for (size_t y = start_pos2; y<size2; y++) {
  //                for (size_t x = start_pos3; x<size3; x++) {
  //                    const size_t index = z * size1 * size2 + y * size1 + x;
  //                    __kumir_free_scalar(result->data + index);
  //                }
  //            }
  //        }
  //        break;
  //    }
  //    default: break;
  //    }
}

EXTERN void __kumir_create_array_copy_1(__kumir_array *result,
                                        const __kumir_scalar *left_1,
                                        const __kumir_scalar *right_1) {
  __kumir_create_array_ref_1(result, left_1, right_1);

  const size_t source_start_x =
      static_cast<size_t>(result->shape_left[0] - result->size_left[0]);

  const size_t source_size_x =
      static_cast<size_t>(result->size_right[0] - result->size_left[0] + 1);

  const size_t target_size_x =
      static_cast<size_t>(result->shape_right[0] - result->shape_left[0] + 1);

  __kumir_assert(target_size_x <= source_size_x, __LINE__);

  const size_t block_size = target_size_x;

  __kumir_scalar *const data = reinterpret_cast<__kumir_scalar *>(
      calloc(block_size, sizeof(__kumir_scalar)));

  for (size_t x = 0u; x < target_size_x; x++) {

    const size_t source_index = source_start_x + x;
    const size_t target_index = x;

    const __kumir_scalar *const source_element = result->data + source_index;
    __kumir_scalar *const target_element = data + target_index;

    target_element->defined = source_element->defined;
    target_element->type = source_element->type;
    target_element->data =
        __kumir_copy_variant(source_element->data, source_element->type);
  }

  result->data = data;
  result->size_left[0] = result->shape_left[0];
  result->size_right[0] = result->shape_right[0];
}

EXTERN void __kumir_create_array_copy_2(__kumir_array *result,
                                        const __kumir_scalar *left_1,
                                        const __kumir_scalar *right_1,
                                        const __kumir_scalar *left_2,
                                        const __kumir_scalar *right_2) {
  __kumir_create_array_ref_2(result, left_1, right_1, left_2, right_2);

  const size_t source_start_y =
      static_cast<size_t>(result->shape_left[0] - result->size_left[0]);

  const size_t source_size_y =
      static_cast<size_t>(result->size_right[0] - result->size_left[0] + 1);

  const size_t target_size_y =
      static_cast<size_t>(result->shape_right[0] - result->shape_left[0] + 1);

  __kumir_assert(target_size_y <= source_size_y, __LINE__);

  const size_t source_start_x =
      static_cast<size_t>(result->shape_left[1] - result->size_left[1]);

  const size_t source_size_x =
      static_cast<size_t>(result->size_right[1] - result->size_left[1] + 1);

  const size_t target_size_x =
      static_cast<size_t>(result->shape_right[1] - result->shape_left[1] + 1);

  __kumir_assert(target_size_x <= source_size_x, __LINE__);

  const size_t block_size = target_size_y * target_size_x;

  __kumir_scalar *const data = reinterpret_cast<__kumir_scalar *>(
      calloc(block_size, sizeof(__kumir_scalar)));

  for (size_t y = 0u; y < target_size_y; y++) {

    for (size_t x = 0u; x < target_size_x; x++) {

      const size_t source_index =
          (source_start_y + y) * source_size_x + source_start_x + x;

      const size_t target_index = target_size_x * y + x;

      const __kumir_scalar *const source_element = result->data + source_index;
      __kumir_scalar *const target_element = data + target_index;

      target_element->defined = source_element->defined;
      target_element->type = source_element->type;
      target_element->data =
          __kumir_copy_variant(source_element->data, source_element->type);
    }
  }

  result->data = data;
  result->size_left[0] = result->shape_left[0];
  result->size_left[1] = result->shape_left[1];
  result->size_right[0] = result->shape_right[0];
  result->size_right[1] = result->shape_right[1];
}

EXTERN void __kumir_create_array_copy_3(__kumir_array *result,
                                        const __kumir_scalar *left_1,
                                        const __kumir_scalar *right_1,
                                        const __kumir_scalar *left_2,
                                        const __kumir_scalar *right_2,
                                        const __kumir_scalar *left_3,
                                        const __kumir_scalar *right_3) {
  __kumir_create_array_ref_3(result, left_1, right_1, left_2, right_2, left_3,
                             right_3);

  const size_t source_start_z =
      static_cast<size_t>(result->shape_left[0] - result->size_left[0]);

  const size_t source_size_z =
      static_cast<size_t>(result->size_right[0] - result->size_left[0] + 1);

  const size_t target_size_z =
      static_cast<size_t>(result->shape_right[0] - result->shape_left[0] + 1);

  __kumir_assert(target_size_z <= source_size_z, __LINE__);

  const size_t source_start_y =
      static_cast<size_t>(result->shape_left[1] - result->size_left[1]);

  const size_t source_size_y =
      static_cast<size_t>(result->size_right[1] - result->size_left[1] + 1);

  const size_t target_size_y =
      static_cast<size_t>(result->shape_right[1] - result->shape_left[1] + 1);

  __kumir_assert(target_size_y <= source_size_y, __LINE__);

  const size_t source_start_x =
      static_cast<size_t>(result->shape_left[2] - result->size_left[2]);

  const size_t source_size_x =
      static_cast<size_t>(result->size_right[2] - result->size_left[2] + 1);

  const size_t target_size_x =
      static_cast<size_t>(result->shape_right[2] - result->shape_left[2] + 1);

  __kumir_assert(target_size_x <= source_size_x, __LINE__);

  const size_t block_size = target_size_z * target_size_y * target_size_x;

  __kumir_scalar *const data = reinterpret_cast<__kumir_scalar *>(
      calloc(block_size, sizeof(__kumir_scalar)));

  for (size_t z = 0u; z < target_size_z; z++) {

    for (size_t y = 0u; y < target_size_y; y++) {

      for (size_t x = 0u; x < target_size_x; x++) {

        const size_t source_index =
            (source_start_z + z) * source_size_y * source_size_x +
            (source_start_y + y) * source_size_x + source_start_x + x;

        const size_t target_index =
            target_size_x * target_size_y * z + target_size_x * y + x;

        const __kumir_scalar *const source_element =
            result->data + source_index;
        __kumir_scalar *const target_element = data + target_index;

        target_element->defined = source_element->defined;
        target_element->type = source_element->type;
        target_element->data =
            __kumir_copy_variant(source_element->data, source_element->type);
      }
    }
  }

  result->data = data;
  result->size_left[0] = result->shape_left[0];
  result->size_left[1] = result->shape_left[1];
  result->size_left[2] = result->shape_left[2];
  result->size_right[0] = result->shape_right[0];
  result->size_right[1] = result->shape_right[1];
  result->size_right[2] = result->shape_right[2];
}

template <typename T> static T __eat(const char **pdata) {
  const char *data = *pdata;
  const T *idata = reinterpret_cast<const T *>(data);
  T val = idata[0];
  //    std::cerr << "Eat: " << val << std::endl;
  idata++;
  data = reinterpret_cast<const char *>(idata);
  *pdata = data;
  return val;
}

static void __kumir_fill_array_1(__kumir_array *array, const char *data,
                                 const __kumir_scalar_type type) {
  const char *p = data;
  __kumir_int isz = __eat<__kumir_int>(&p);
  //    std::cerr << "Size = " << isz << std::endl;
  size_t index = 0u;
  __kumir_scalar *adata = reinterpret_cast<__kumir_scalar *>(array->data);
  while (isz) {
    adata[index].defined = __eat<__kumir_bool>(&p);
    //        std::cerr << "defined = " << adata[index].defined << "\n";
    adata[index].type = type;
    if (__KUMIR_INT == type) {
      adata[index].data.i = __eat<__kumir_int>(&p);
    } else if (__KUMIR_REAL == type) {
      adata[index].data.r = __eat<__kumir_real>(&p);
    } else if (__KUMIR_BOOL == type) {
      adata[index].data.b = __eat<__kumir_bool>(&p);
    } else if (__KUMIR_CHAR == type) {
      uint16_t code = __eat<uint16_t>(&p);
      //            std::cerr << "code = " << code << "\n";
      adata[index].data.c = code;
    }
    isz--;
    index++;
  }
}

static unsigned char __kumir_hex_to_byte(const char c) {
  switch (c) {
  case '0':
    return 0x0u;
  case '1':
    return 0x1u;
  case '2':
    return 0x2u;
  case '3':
    return 0x3u;
  case '4':
    return 0x4u;
  case '5':
    return 0x5u;
  case '6':
    return 0x6u;
  case '7':
    return 0x7u;
  case '8':
    return 0x8u;
  case '9':
    return 0x9u;
  case 'a':
  case 'A':
    return 0xAu;
  case 'b':
  case 'B':
    return 0xBu;
  case 'c':
  case 'C':
    return 0xCu;
  case 'd':
  case 'D':
    return 0xDu;
  case 'e':
  case 'E':
    return 0xEu;
  case 'f':
  case 'F':
    return 0xFu;
  default:
    return 0u;
  }
}

static char *__kumir_deserialize_hex(const char *s) {
  const size_t buffer_size = (strlen(s) + 1) / 3;
  char *bytes = reinterpret_cast<char *>(malloc(buffer_size));
  for (size_t i = 0; i < buffer_size; ++i) {
    unsigned char h = __kumir_hex_to_byte(s[i * 3]);
    unsigned char l = __kumir_hex_to_byte(s[i * 3 + 1]);
    unsigned char byte = h << 4 | l;
    bytes[i] = byte;
  }
  return bytes;
}

EXTERN void __kumir_fill_array_i(__kumir_array *array, const char *data) {
  //    std::cerr << data << std::endl;
  char *buffer = __kumir_deserialize_hex(data);
  if (1u == array->dim)
    __kumir_fill_array_1(array, buffer, __KUMIR_INT);
  free(buffer);
}

EXTERN void __kumir_fill_array_r(__kumir_array *array, const char *data) {
  char *buffer = __kumir_deserialize_hex(data);
  if (1u == array->dim)
    __kumir_fill_array_1(array, buffer, __KUMIR_REAL);
  free(buffer);
}

EXTERN void __kumir_fill_array_b(__kumir_array *array, const char *data) {
  char *buffer = __kumir_deserialize_hex(data);
  if (1u == array->dim)
    __kumir_fill_array_1(array, buffer, __KUMIR_BOOL);
  free(buffer);
}

EXTERN void __kumir_fill_array_c(__kumir_array *array, const char *data) {
  char *buffer = __kumir_deserialize_hex(data);
  if (1u == array->dim)
    __kumir_fill_array_1(array, buffer, __KUMIR_CHAR);
  free(buffer);
}

EXTERN void __kumir_fill_array_s(__kumir_array *array, const char *data) {
  char *buffer = __kumir_deserialize_hex(data);
  if (1u == array->dim)
    __kumir_fill_array_1(array, buffer, __KUMIR_STRING);
  free(buffer);
}

EXTERN void __kumir_get_array_1_element(__kumir_scalar **result,
                                        bool value_expected,
                                        __kumir_array *array,
                                        const __kumir_scalar *x) {
  __kumir_scalar *data = reinterpret_cast<__kumir_scalar *>(array->data);
  __kumir_check_value_defined(x);
  __kumir_int xx = x->data.i;
  if (xx < array->shape_left[0] || xx > array->shape_right[0]) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Выход за границу таблицы"));
  } else {
    const size_t index = static_cast<size_t>(xx - array->size_left[0]);
    if (value_expected && !data[index].defined) {
      Kumir::Core::abort(
          Kumir::Core::fromUtf8("Значение элемента таблицы не определено"));
    }
    *result = data + index;
  }
}

EXTERN void __kumir_get_array_2_element(__kumir_scalar **result,
                                        bool value_expected,
                                        __kumir_array *array,
                                        const __kumir_scalar *y,
                                        const __kumir_scalar *x) {
  __kumir_scalar *data = reinterpret_cast<__kumir_scalar *>(array->data);
  __kumir_check_value_defined(x);
  __kumir_check_value_defined(y);
  __kumir_int xx = x->data.i;
  __kumir_int yy = y->data.i;
  if (yy < array->shape_left[0] || yy > array->shape_right[0]) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Выход за границу таблицы"));
  } else if (xx < array->shape_left[1] || xx > array->shape_right[1]) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Выход за границу таблицы"));
  } else {
    const size_t size_x =
        static_cast<size_t>(1 + array->size_right[1] - array->size_left[1]);
    const size_t index = static_cast<size_t>(
        (size_x * (yy - array->size_left[0])) + (xx - array->size_left[1]));
    if (value_expected && !data[index].defined) {
      Kumir::Core::abort(
          Kumir::Core::fromUtf8("Значение элемента таблицы не определено"));
    }
    *result = &data[index];
  }
}

EXTERN void
__kumir_get_array_3_element(__kumir_scalar **result, bool value_expected,
                            __kumir_array *array, const __kumir_scalar *z,
                            const __kumir_scalar *y, const __kumir_scalar *x) {
  __kumir_scalar *data = reinterpret_cast<__kumir_scalar *>(array->data);
  __kumir_check_value_defined(x);
  __kumir_check_value_defined(y);
  __kumir_check_value_defined(z);
  __kumir_int xx = x->data.i;
  __kumir_int yy = y->data.i;
  __kumir_int zz = z->data.i;
  if (zz < array->shape_left[0] || zz > array->shape_right[0]) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Выход за границу таблицы"));
  } else if (yy < array->shape_left[1] || yy > array->shape_right[1]) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Выход за границу таблицы"));
  } else if (xx < array->shape_left[2] || xx > array->shape_right[2]) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Выход за границу таблицы"));
  } else {
    const size_t size_y =
        static_cast<size_t>(1 + array->size_right[1] - array->size_left[1]);
    const size_t size_x =
        static_cast<size_t>(1 + array->size_right[2] - array->size_left[2]);
    const size_t index = static_cast<size_t>(
        (size_y * size_x * (zz - array->size_left[0])) +
        (size_x * (yy - array->size_left[1])) + (xx - array->size_left[2]));
    if (value_expected && !data[index].defined) {
      Kumir::Core::abort(
          Kumir::Core::fromUtf8("Значение элемента таблицы не определено"));
    }
    *result = &data[index];
  }
}

EXTERN void __kumir_link_array(__kumir_array *result,
                               const __kumir_array *from) {
  memcpy(result, from, sizeof(__kumir_array));
}

EXTERN void __kumir_free_array(__kumir_array *array) {
  if (array->dim > 0u && array->data) {
    const __kumir_scalar &first = array->data[0];
    if (__KUMIR_STRING == first.type) {
      size_t elems = 0u;
      elems =
          static_cast<size_t>(1 + array->size_right[0] - array->size_left[0]);
      if (array->dim > 1u) {
        elems *=
            static_cast<size_t>(1 + array->size_right[1] - array->size_left[1]);
      }
      if (array->dim > 2u) {
        elems *=
            static_cast<size_t>(1 + array->size_right[2] - array->size_left[2]);
      }
      for (size_t i = 0; i < elems; i++) {
        __kumir_free_scalar(&(array->data[i]));
      }
    }
    free(array->data);
  }
}

EXTERN void __kumir_get_string_slice_ref(__kumir_stringref *result,
                                         __kumir_scalar **sptr,
                                         const __kumir_scalar *from,
                                         const __kumir_scalar *to) {
  __kumir_scalar *sval = *sptr;
  __kumir_check_value_defined(sval);
  __kumir_check_value_defined(from);
  __kumir_check_value_defined(to);
  const std::wstring s = __kumir_scalar_as_wstring(sval);
  const int kfrom = from->data.i;
  const int kto = to->data.i;
  const int klength = s.length();
  result->ref = sval;
  if (kto < kfrom && kfrom == 0) {
    result->from = result->length = 0u;
    result->op = __KUMIR_STRINGREF_PREPEND;
  } else if (kfrom > klength) {
    Kumir::Core::abort(
        Kumir::Core::fromUtf8("Левая граница вырезки за пределами строки"));
  } else if (kto < kfrom && kfrom > 0) {
    result->from = static_cast<size_t>(kfrom);
    result->length = 0u;
    result->op = __KUMIR_STRINGREF_INSERT;
  } else if (kfrom == klength + 1 && kto <= kfrom) {
    result->from = result->length = 0u;
    result->op = __KUMIR_STRINGREF_APPEND;
  } else if (kto < 1 || kto > klength) {
    Kumir::Core::abort(
        Kumir::Core::fromUtf8("Правая граница вырезки за пределами строки"));
  } else if (kfrom < 1 || kfrom > klength) {
    Kumir::Core::abort(
        Kumir::Core::fromUtf8("Левая граница вырезки за пределами строки"));
  } else if (kto < kfrom) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Ошибка в границах вырезки"));
  } else {
    result->from = static_cast<size_t>(kfrom - 1);
    result->length = static_cast<size_t>(kto - kfrom + 1);
    result->op = __KUMIR_STRINGREF_REPLACE;
  }
}

EXTERN void __kumir_get_string_element_ref(__kumir_stringref *result,
                                           __kumir_scalar **sptr,
                                           const __kumir_scalar *at) {
  __kumir_scalar *sval = *sptr;
  __kumir_check_value_defined(sval);
  __kumir_check_value_defined(at);
  const std::wstring s = __kumir_scalar_as_wstring(sval);
  const int kindex = at->data.i;
  const int klength = s.length();
  if (kindex < 1) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Индекс символа меньше 1"));
  } else if (kindex > klength) {
    Kumir::Core::abort(
        Kumir::Core::fromUtf8("Индекс символа больше длины строки"));
  } else {
    const size_t index = static_cast<size_t>(kindex - 1);
    result->ref = sval;
    result->op = __KUMIR_STRINGREF_REPLACE;
    result->from = index;
    result->length = 1u;
  }
}

EXTERN void __kumir_get_string_slice(__kumir_scalar *result,
                                     const __kumir_scalar **sptr,
                                     const __kumir_scalar *from,
                                     const __kumir_scalar *to) {
  const __kumir_scalar *sval = *sptr;
  __kumir_check_value_defined(sval);
  __kumir_check_value_defined(from);
  __kumir_check_value_defined(to);
  const std::wstring s = __kumir_scalar_as_wstring(sval);
  const int kfrom = from->data.i;
  const int kto = to->data.i;
  const int klength = s.length();
  std::wstring res;
  if (kfrom < 1 || kfrom > klength) {
    Kumir::Core::abort(
        Kumir::Core::fromUtf8("Левая граница вырезки за пределами строки"));
  } else if (kto < kfrom) {
    // keep empty string
  } else if (kto < 1 || kto > klength) {
    Kumir::Core::abort(
        Kumir::Core::fromUtf8("Правая граница вырезки за пределами строки"));
  } else {
    const size_t index_from = static_cast<size_t>(kfrom - 1);
    const size_t res_length = static_cast<size_t>(kto - kfrom + 1);
    res = s.substr(index_from, res_length);
  }
  __kumir_create_string(result, res);
}

EXTERN void __kumir_get_string_element(__kumir_scalar *result,
                                       const __kumir_scalar **sptr,
                                       const __kumir_scalar *at) {
  const __kumir_scalar *sval = *sptr;
  __kumir_check_value_defined(sval);
  __kumir_check_value_defined(at);
  const std::wstring s = __kumir_scalar_as_wstring(sval);
  const int kindex = at->data.i;
  const int klength = s.length();
  if (kindex < 1) {
    Kumir::Core::abort(Kumir::Core::fromUtf8("Индекс символа меньше 1"));
  } else if (kindex > klength) {
    Kumir::Core::abort(
        Kumir::Core::fromUtf8("Индекс символа больше длины строки"));
  } else {
    const size_t index = static_cast<size_t>(kindex - 1);
    const wchar_t ch = s.at(index);
    result->defined = true;
    result->type = __KUMIR_CHAR;
    result->data.c = ch;
  }
}

typedef struct {
  int32_t counter;
  int32_t from;
  int32_t to;
  int32_t step;
} for_spec;

static std::stack<for_spec> for_counters;

EXTERN void __kumir_loop_for_from_to_init_counter(const __kumir_scalar *from,
                                                  const __kumir_scalar *to) {
  for_spec spec;
  spec.counter = from->data.i - 1;
  spec.from = from->data.i;
  spec.to = to->data.i;
  spec.step = 1;
  //    std::cerr << "\n=== init for ? from " << spec.from << " to " << spec.to
  //    << " step " << spec.step << "\n";
  for_counters.push(spec);
}

EXTERN void
__kumir_loop_for_from_to_step_init_counter(const __kumir_scalar *from,
                                           const __kumir_scalar *to,
                                           const __kumir_scalar *step) {
  for_spec spec;
  spec.counter = from->data.i - step->data.i;
  spec.from = from->data.i;
  spec.to = to->data.i;
  spec.step = step->data.i;
  //    std::cerr << "\n=== init for ? from " << spec.from << " to " << spec.to
  //    << " step " << spec.step << "\n";
  for_counters.push(spec);
}

EXTERN __kumir_bool __kumir_loop_for_check_counter(__kumir_scalar *variable) {
  for_spec &spec = for_counters.top();
  int32_t &i = spec.counter;
  const int32_t f = spec.from;
  const int32_t t = spec.to;
  const int32_t s = spec.step;
  i += s;
  bool result = s >= 0 ? f <= i && i <= t : t <= i && i <= f;
  //    std::cerr << "\n=== check for " << spec.counter << " from " << spec.from
  //    << " to " << spec.to << " step " << spec.step << " : " << result <<
  //    "\n";
  if (result) {
    variable->data.i = i;
    variable->defined = true;
    variable->type = __KUMIR_INT;
  }
  return result;
}

EXTERN void __kumir_loop_times_init_counter(const __kumir_scalar *from) {
  for_spec spec;
  spec.counter = from->data.i;
  for_counters.push(spec);
}

EXTERN __kumir_bool __kumir_loop_times_check_counter() {
  for_spec &times = for_counters.top();
  if (times.counter > 0) {
    times.counter--;
    return true;
  } else {
    return false;
  }
}

EXTERN void __kumir_loop_end_counter() { for_counters.pop(); }

EXTERN void Kumir_Standard_Library___init__() { Kumir::initStandardLibrary(); }
