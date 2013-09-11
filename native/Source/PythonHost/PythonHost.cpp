#include <Bacon/Bacon.h>
#include <Bacon/BaconInternal.h>

#include <Python.h>

#include <string>
using namespace std;

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
	
	string scriptPath = string(Platform_GetBundlePath()) + "/examples/quickstart.py";
	
	FILE* fp = fopen(scriptPath.c_str(), "r");
	
	Py_SetProgramName(&programName[0]);
	Py_SetPath(pythonPath.c_str());
	Py_InitializeEx(0);
	PyRun_SimpleFile(fp, scriptPath.c_str());
	Py_Finalize();
	
    return 0;
}

