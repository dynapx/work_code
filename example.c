#define PY_SSIZE_T_CLEAN
#include <Python.h>
static PyObject *SpamError;

// Function 1: Executes a shell command (supports only position arguments)
static PyObject *spam_system(PyObject *self, PyObject *args)
{
    const char *command;
    int sts;

    if (!PyArg_ParseTuple(args, "s", &command))
        return NULL;
    
    sts = system(command);
    if (sts < 0) {
        PyErr_SetString(SpamError, "System command failed");
        return NULL;
    }

    PyObject *test1 = _PyObject_New(PyLong_FromLong(1000));
    printf("test1=%d\n", Py_REFCNT(test1));
    PyObject* test2 = test1;
    printf("test2=%d\n", Py_REFCNT(test2));

    return test1;
}

// Function 2: Adds two integers (supports only position arguments)
static PyObject *spam_add(PyObject *self, PyObject *args)
{
    int a, b;
    
    if (!PyArg_ParseTuple(args, "ii", &a, &b))
        return NULL;
    
    return PyLong_FromLong(a + b);
}

// Function 3: Multiplies two integers (supports only position arguments)
static PyObject *spam_multiply(PyObject *self, PyObject *args)
{
    int a, b;
    
    if (!PyArg_ParseTuple(args, "ii", &a, &b))
        return NULL;
    
    return PyLong_FromLong(a * b);
}

// Function 4: Returns a string saying "Hello, <name>" (supports position and keyword arguments)
static PyObject *spam_greet(PyObject *self, PyObject *args, PyObject *kwargs)
{
    const char *name;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s", NULL, &name))
        return NULL;
    
    char greeting[100];
    snprintf(greeting, sizeof(greeting), "Hello, %s!", name);
    
    return Py_BuildValue("s", greeting);
}

// Function 5: Returns the square of a number (supports position and keyword arguments)
static PyObject *spam_square(PyObject *self, PyObject *args, PyObject *kwargs)
{
    double number;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "d", NULL, &number))
        return NULL;
    
    return PyFloat_FromDouble(number * number);
}

// Method Table
static PyMethodDef SpamMethods[] = {
    {(char *)"system", (PyCFunction)spam_system, METH_VARARGS, "Execute a shell command."},
    {(char *)"add", (PyCFunction)spam_add, METH_VARARGS, "Add two integers."},
    {(char *)"multiply", (PyCFunction)spam_multiply, METH_VARARGS, "Multiply two integers."},
    //{(char *)"greet", PyCFunctionWithKeywords(spam_greet), METH_VARARGS | METH_KEYWORDS, "Greet with a name."},
    //{(char *)"square", PyCFunctionWithKeywords(spam_square), METH_VARARGS | METH_KEYWORDS, "Return the square of a number."},
    {NULL, NULL, 0, NULL}  /* Sentinel */
};

// Module definition
static struct PyModuleDef spammodule = {
    PyModuleDef_HEAD_INIT,
    "spam",   /* name of module */
    NULL, /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    SpamMethods
};

// Module initialization function
PyMODINIT_FUNC PyInit_spam(void)
{
    PyObject *m;

    m = PyModule_Create(&spammodule);
    if (m == NULL)
        return NULL;

    SpamError = PyErr_NewException("spam.error", NULL, NULL);
    if (PyModule_AddObjectRef(m, "error", SpamError) < 0) {
        Py_CLEAR(SpamError);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}
