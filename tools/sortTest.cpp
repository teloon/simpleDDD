#include <Python.h>
#include <iostream>
#include <stdio.h>

using namespace std;

int main()
{
	FILE *pFile;
	pFile = fopen("remove_single.py", "r");
	Py_Initialize();

	if(PyRun_SimpleFile(pFile, "remove_single.py") != 0)
		cout << "err?\n";
	fclose(pFile);
	/*
	PyObject *pMod = NULL;
	PyObject *pFunc = NULL;
	PyObject *pName = NULL;

	pName = PyString_FromString("remove_single");
	pMod = PyImport_Import(pName);
	pFunc = PyImport_ImportModule("__main__");
	if(!pMod)
		cout << "pMod not get!\n";
	pFunc = PyObject_GetAttrString(pMod, "__main__");
	if(!pFunc)
		cout << "pFunc not get!\n";
	PyEval_CallObject(pFunc, NULL);
*/	Py_Finalize();
	return 0;
}
