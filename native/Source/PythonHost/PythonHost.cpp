#include <Bacon/Bacon.h>
#include <Bacon/BaconInternal.h>

#include <Python.h>

int main(int argc, char * argv[])
{
	// TODO less dodgy
	wchar_t programName[1024];
	mbstowcs(programName, argv[0], sizeof(programName));
	
	wchar_t pythonHome[1024];
	mbstowcs(pythonHome, Platform_GetBundlePath(), sizeof(pythonHome));
	wcscat(pythonHome, L"/Lib");
	
    Py_NoUserSiteDirectory++;
    Py_NoSiteFlag++;
    Py_IgnoreEnvironmentFlag++;
	
	Py_SetProgramName(programName);
	Py_SetPath(pythonHome);
	Py_InitializeEx(0);
	PyRun_SimpleString("print('Hello world!')\n");
	Py_Finalize();
	
    return 0;
}

