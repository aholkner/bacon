#include <Bacon/Bacon.h>
#include <Bacon/BaconInternal.h>

#include <Python.h>

#include <string>
using namespace std;

PyObject* BaconModule_Init();

wstring StringToWString(string const& s)
{
	std::wstring ws(s.size(), '\0');
	ws.resize(mbstowcs(&ws[0], s.c_str(), s.size()));
	return ws;
}

int main(int argc, char * argv[])
{
	Py_NoSiteFlag = 1;
	Py_NoUserSiteDirectory = 1;
	Py_IgnoreEnvironmentFlag = 1;
	
	wstring programName = StringToWString(argv[0]);
	wstring bundlePath = StringToWString(Platform_GetBundlePath());
	
	wstring pythonPath = bundlePath + L"/Lib:";
	pythonPath += L":" + bundlePath;
	
	string scriptPath = string(Platform_GetBundlePath()) + "/MonthlyVisitor/MonthlyVisitor.py";
	
	FILE* fp = fopen(scriptPath.c_str(), "r");
	
	Py_SetProgramName(&programName[0]);
	Py_SetPath(pythonPath.c_str());
	
	PyImport_AppendInittab("_bacon", BaconModule_Init);
	
	Py_InitializeEx(0);
	
	wstring wscriptPath = StringToWString(scriptPath);
	wchar_t* wargv0 = &wscriptPath[0];
	PySys_SetArgv(1, &wargv0);
	

	PyRun_SimpleFile(fp, scriptPath.c_str());
	Py_Finalize();
	
    return 0;
}

