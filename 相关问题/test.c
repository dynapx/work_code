#include <Python.h>

static PyObject* refcount_error(PyObject* self, PyObject* args) {
    PyObject* obj1 = NULL;
    PyObject* obj2 = NULL;

    // 创建一个 Python 对象
    obj1 = Py_BuildValue("i", 42); // 创建一个整数对象
    if (!obj1) {
        return NULL; // 如果创建失败，返回 NULL
    }

    // 将 obj1 赋值给 obj2，但没有增加引用计数
    obj2 = obj1; // 引用计数错误：obj2 没有正确增加引用计数

    // 释放 obj1，但 obj2 仍然指向同一个对象
    Py_DECREF(obj1);
    Py_DECREF(obj1);
    Py_DECREF(obj1);
    Py_DECREF(obj1);
    Py_DECREF(obj1);

    // 返回 obj2（此时 obj2 指向的对象已经被释放）
    return obj2; // 这里会引发引用计数错误
}

static PyMethodDef RefcountErrorMethods[] = {
    {"refcount_error", refcount_error, METH_VARARGS, "Test reference counting error."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef refcounterrormodule = {
    PyModuleDef_HEAD_INIT,
    "refcounterror",
    "A module to demonstrate reference counting errors.",
    -1,
    RefcountErrorMethods
};

PyMODINIT_FUNC PyInit_refcounterror(void) {
    PyObject* module = PyModule_Create(&refcounterrormodule);
    if (!module) {
        return NULL;
    }
    return module;
}
