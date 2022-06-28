#include "PythonInterp.h"

#include <stdexcept>

#include "Debug.h"

#if defined(WIN32)
#include <stdlib.h>
int setenv(const char* name, const char* value, int overwrite) {
    int errcode = 0;
    if (!overwrite) {
        const char* envStr = getenv(name);
        if (envStr != NULL) {
            errcode = -1;
        } else {
            errcode = 0;
        }
    }
    if (errcode == 0) {
        errcode = _putenv_s(name, value);
    }
    return errcode;
}
#endif

PythonInterp::PythonInterp() {}

/**
 * @brief
 * // https://docs.python.org/3/extending/embedding.html
 * Used to launch functions in Python 3
 * Creates a class that wrappes a python interpreter
 *
 * @param pyName Python file name
 * @param pyFunc Python function name
 * @param pyPath Python 3 Interpreter Path (if NULL, uses default system python3
 * path) (Recommended : Leave as default)
 */
PythonInterp::PythonInterp(const char* pyName, const char* pyFunc,
                           const char* pyPath)
    : m_pyPath(pyPath), m_funcName(pyFunc) {
    // Set custom python interpreter (Not recommended)
    if (m_pyPath) {
        RAYX_D_LOG << "set custom path";
        setenv("PATH", pyPath, 1);
    }

    // Initiliaze the Interpreter
    Py_Initialize();
    // Set module lookup dir, otherwise Python can't run the files
    PyRun_SimpleString("import os");
    PyRun_SimpleString("import sys");
    std::string python_dir(resolvePath("build/bin/python"));
    python_dir = "sys.path.append(\""+python_dir+"\")";
    PyRun_SimpleString(python_dir.c_str());

    // Python file
    m_pName = PyUnicode_DecodeFSDefault(pyName);

    // Load the module object
    m_pModule = PyImport_Import(m_pName);

    if (m_pModule == NULL) {
        // cleanup();
        PyErr_Print();
        //  Throw exception
        throw std::runtime_error("No python module was found");
    }
    // pDict is a borrowed reference
    m_pDict = PyModule_GetDict(m_pModule);

    // pFunc is also a borrowed reference
    // Python function
    m_pFunc = PyDict_GetItemString(m_pDict, pyFunc);
}

/**
 * @brief Executes the python function with the specified interpreter class.
 * @throw runtime_error if Python result is not expected
 */
void PythonInterp::execute() {
    if (m_pFunc && PyCallable_Check(m_pFunc)) {
        if (!m_outputName.empty()) {
            // If provided input file, parse to python interpreter as bytes
            m_pValue = PyTuple_New(2);
            PyTuple_SetItem(m_pValue, 0,
                            PyBytes_FromString(m_outputName.c_str()));
            PyTuple_SetItem(m_pValue, 1, PyLong_FromLong(long(m_plotType)));
        }
        PyErr_Print();
        RAYX_D_LOG << "Launching Python3 Interpreter.";
        m_presult = PyObject_CallObject(m_pFunc, m_pValue);
        PyErr_Print();
    } else {
        PyErr_Print();
        throw std::runtime_error(
            "Error while running the python module: [ERR010]" +
            (std::string)(m_funcName));
    }

    if (PyLong_AsLong(m_presult) == 0 || !m_presult) {
        cleanup();
        throw std::runtime_error(
            "Error while running the python module: [ERR011]" +
            (std::string)(m_funcName));
    }
    cleanup();
}

/**
 * @brief Clean up and stop Python
 *
 */
void PythonInterp::cleanup() {
    // Clean up and free allocated memory
    Py_DECREF(m_pModule);
    Py_DECREF(m_pName);
    // Py_DECREF(m_pValue);
    // Py_DECREF(m_pFunc);
    // Py_DECREF(m_presult);

    // Finish the Python Interpreter
    Py_Finalize();
}
/**
 * @brief Change plot title, call before .execute
 * @param outputName Name of input file. Defaults to NULL (Plotting
 * related)
 */
void PythonInterp::setPlotName(const char* outputName) {
    m_outputName = std::string(outputName);
}
/**
 * @brief Change plot type, call before .execute (1,2,3)
 *
 * @details Supported Types:
 *  _plotLikeRAYUI = 1
 *  _plotForEach = 2
 *  _plotForEachSubplot = 3
 * @param plotType Name of input file. Defaults to NULL (Plotting
 * related)
 */
void PythonInterp::setPlotType(int plotType) {
    if (plotType < 1 || plotType > 3) {
        RAYX_ERR << "Error while setthing the python parameters: [ERR001]";
    }
    m_plotType = plotType;
}
PythonInterp::~PythonInterp() {}
