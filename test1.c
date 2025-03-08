#include <Python.h>

static PyObject* example_function(PyObject* self, PyObject* args) {
    PyObject* list = PyList_New(3);
    if (!list) {
        return NULL;
    }

    PyObject* item1 = PyLong_FromLong(1);
    PyObject* item2 = PyLong_FromLong(2);
    PyObject* item3 = PyLong_FromLong(3);

    PyList_SetItem(list, 0, item1);  // item1 的引用计数不会增加
    PyList_SetItem(list, 1, item2);  // item2 的引用计数不会增加
    PyList_SetItem(list, 2, item3);  // item3 的引用计数不会增加

    return list;  // 返回 list，但未增加引用计数
}
