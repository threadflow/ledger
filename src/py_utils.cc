/*
 * Copyright (c) 2003-2023, John Wiegley.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of New Artisans LLC nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <system.hh>

#include "pyinterp.h"
#include "pyutils.h"

namespace ledger {

using namespace boost::python;

struct bool_to_python
{
  static PyObject * convert(const bool truth)
  {
    if (truth)
      Py_RETURN_TRUE;
    else
      Py_RETURN_FALSE;
  }
};

struct bool_from_python
{
  static void* convertible(PyObject* obj_ptr)
  {
    if (!PyBool_Check(obj_ptr)) return 0;
    return obj_ptr;
  }

  static void construct(PyObject* obj_ptr,
                        converter::rvalue_from_python_stage1_data* data)
  {
    void * storage =
      ((converter::rvalue_from_python_storage<bool>*) data)->storage.bytes;
    if (obj_ptr == Py_True)
      new (storage) bool(true);
    else
      new (storage) bool(false);
    data->convertible = storage;
  }
};

typedef register_python_conversion<bool, bool_to_python, bool_from_python>
  bool_python_conversion;


struct string_to_python
{
  static PyObject* convert(const string& str)
  {
    // Return bytes, not characters; see __unicode__ methods for that
    return incref(object(static_cast<const std::string&>(str)).ptr());
  }
};

struct string_from_python
{
  static void* convertible(PyObject* obj_ptr)
  {
    if (!PyUnicode_Check(obj_ptr))
      return 0;
    return obj_ptr;
  }

  static void construct(PyObject* obj_ptr,
                        converter::rvalue_from_python_stage1_data* data)
  {
    VERIFY(PyUnicode_Check(obj_ptr));

#if PY_MINOR_VERSION < 12
    if (PyUnicode_READY(obj_ptr))
      return;
#endif

    const Py_UNICODE* value;
    Py_ssize_t size;
    string str;

#if PY_MINOR_VERSION >= 3
    size = PyUnicode_GET_LENGTH(obj_ptr);
    switch (PyUnicode_KIND(obj_ptr)) {
      case PyUnicode_1BYTE_KIND: {
        Py_UCS1* value = PyUnicode_1BYTE_DATA(obj_ptr);
        if (value == 0) throw_error_already_set();
        utf8::unchecked::utf16to8(value, value + size, std::back_inserter(str));
        } break;
#if PY_MINOR_VERSION < 12 && Py_UNICODE_SIZE == 2
      case PyUnicode_WCHAR_KIND:
#endif
      case PyUnicode_2BYTE_KIND: {
        Py_UCS2* value = PyUnicode_2BYTE_DATA(obj_ptr);
        if (value == 0) throw_error_already_set();
        utf8::unchecked::utf16to8(value, value + size, std::back_inserter(str));
        } break;
#if PY_MINOR_VERSION < 12 && Py_UNICODE_SIZE == 4
      case PyUnicode_WCHAR_KIND:
#endif
      case PyUnicode_4BYTE_KIND: {
        Py_UCS4* value = PyUnicode_4BYTE_DATA(obj_ptr);
        if (value == 0) throw_error_already_set();
        utf8::unchecked::utf32to8(value, value + size, std::back_inserter(str));
        } break;
      default:
        assert("PyUnicode_KIND returned an unexpected kind" == NULL);
    }
#else
    size = PyUnicode_GET_SIZE(obj_ptr);
    value = PyUnicode_AS_UNICODE(obj_ptr);
#if Py_UNICODE_SIZE == 2 // UTF-16
    utf8::unchecked::utf16to8(value, value + size, std::back_inserter(str));
#elif Py_UNICODE_SIZE == 4 // UTF-32
    utf8::unchecked::utf32to8(value, value + size, std::back_inserter(str));
#else
    assert("Py_UNICODE has an unexpected size" == NULL);
#endif
#endif

    void* storage =
      reinterpret_cast<converter::rvalue_from_python_storage<string> *>
                      (data)->storage.bytes;
    new (storage) string(str);
    data->convertible = storage;
  }
};

typedef register_python_conversion<string, string_to_python, string_from_python>
  string_python_conversion;

void export_utils()
{
  class_< supports_flags<uint_least8_t> > ("SupportFlags8")
    .def(init<supports_flags<uint_least8_t> >())
    .def(init<uint_least8_t>())

    .add_property("flags",
                  &supports_flags<uint_least8_t>::flags,
                  &supports_flags<uint_least8_t>::set_flags)
    .def("has_flags", &supports_flags<uint_least8_t>::has_flags)
    .def("clear_flags", &supports_flags<uint_least8_t>::clear_flags)
    .def("add_flags", &supports_flags<uint_least8_t>::add_flags)
    .def("drop_flags", &supports_flags<uint_least8_t>::drop_flags)
    ;

  class_< supports_flags<uint_least16_t> > ("SupportFlags16")
    .def(init<supports_flags<uint_least16_t> >())
    .def(init<uint_least16_t>())

    .add_property("flags",
                  &supports_flags<uint_least16_t>::flags,
                  &supports_flags<uint_least16_t>::set_flags)
    .def("has_flags", &supports_flags<uint_least16_t>::has_flags)
    .def("clear_flags", &supports_flags<uint_least16_t>::clear_flags)
    .def("add_flags", &supports_flags<uint_least16_t>::add_flags)
    .def("drop_flags", &supports_flags<uint_least16_t>::drop_flags)
    ;

#if 0
  class_< basic_flags_t<uint_least8_t>,
          bases<supports_flags<uint_least8_t> > > ("BasicFlags8")
    .def(init<uint_least8_t>())

    .def("plus_flags", &basic_flags_t<uint_least8_t>::plus_flags)
    .def("minus_flags", &basic_flags_t<uint_least8_t>::minus_flags)
    ;
#endif

  class_< delegates_flags<uint_least16_t>,
          boost::noncopyable > ("DelegatesFlags16", no_init)
    .add_property("flags",
                  &delegates_flags<uint_least16_t>::flags,
                  &delegates_flags<uint_least16_t>::set_flags)
    .def("has_flags", &delegates_flags<uint_least16_t>::has_flags)
    .def("clear_flags", &delegates_flags<uint_least16_t>::clear_flags)
    .def("add_flags", &delegates_flags<uint_least16_t>::add_flags)
    .def("drop_flags", &delegates_flags<uint_least16_t>::drop_flags)
    ;

  bool_python_conversion();
  string_python_conversion();
}

} // namespace ledger
