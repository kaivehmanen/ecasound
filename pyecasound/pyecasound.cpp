/** 
 * @file pyecasound.cpp Python interface to the ecasound control interface
 */

// ------------------------------------------------------------------------
// pyecasound.cpp: Python interface to the ecasound control interface
// Copyright (C) 2000 Kai Vehmanen (kaiv@wakkanet.fi)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
// ------------------------------------------------------------------------

#include <eca-control.h>
#include <Python.h>

#include "pyecasound.h"

typedef struct {
  PyObject_HEAD
  ECA_SESSION* session;
  ECA_CONTROL* ctrl;
} pyeca_control_t;

static void pyeca_control_del(PyObject *self, PyObject *args);

static PyTypeObject pyeca_control_type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"ECA_CONTROL_INTERFACE",
	sizeof (pyeca_control_t),
	0,
	(destructor)pyeca_control_del,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
};

//  static PyObject *pyeca_control_new(PyObject *self, PyObject *args);
//  static void pyeca_control_del(PyObject *self, PyObject *args);

static PyObject *pyeca_control_new(PyObject *self, PyObject *args) {
  cerr << "pyeca_control_new, C++ constructor" << endl;
//    if (eca_c_rep.ctrl != 0)
//      delete eca_c_rep.ctrl;
//    if (eca_c_rep.session != 0)
//      delete eca_c_rep.session;

//    ecadebug->set_debug_level(ECA_DEBUG::info |
//  			    ECA_DEBUG::module_flow);

//    if (eca_c_rep.ctrl == 0) {
//      eca_c_rep.session = new ECA_SESSION();
//      eca_c_rep.ctrl = new ECA_CONTROL (eca_c_rep.session);
//    }
  self = (PyObject *) PyObject_NEW(pyeca_control_t, &pyeca_control_type);
  return(self);
}

static void pyeca_control_del(PyObject *self, PyObject *args) {
  cerr << "pyeca_control_del, C++ destructor" << endl;
  PyMem_DEL(self);
}

static PyMethodDef libpyecasoundmethods[] = {
  {"ECA_CONTROL_INTERFACE",    pyeca_control_new,    METH_VARARGS },
  {NULL,		       NULL }
};

void initlibpyecasound(void) {
  Py_InitModule("libpyecasound", libpyecasoundmethods);
}
