#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>
#include <stdlib.h>
#include <string>

#include "Injection.h"

using namespace std;

typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

LPFN_ISWOW64PROCESS fnIsWow64Process;

const string SEPERATOR = "\t";

/* In any case, it's useful to have a console window visible
 * for debugging purposes.  Use cout to your heart's content!
 */
int main(int argc,      // Number of strings in array argv
	char *argv[],   // Array of command-line argument strings
	char *envp[])  // Array of environment variable strings
{
	if (argc <= 5 || argc > 7) {
		cout << "Usage: exe_name dll_path full_type_name method_name (param)";
		return 0;
	}

	string bootstrapName(argv[1]);
	string exeName(argv[2]);
	string dllPath(argv[3]);
	string fullTypeName(argv[4]);
	string methodName(argv[5]);
	string param((argc == 7) ? argv[6] : "");

	// ProcessId to inject into
	DWORD procId = GetProcessIdByName(exeName.c_str());

	// Bootstrapper
	char BootstrapperPath[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, BootstrapperPath);
	strcat_s(BootstrapperPath, bootstrapName.c_str());

	string arg = dllPath + SEPERATOR + fullTypeName + SEPERATOR + methodName + SEPERATOR + param;
	wstring w_arg(arg.begin(), arg.end());

	// Dll to inject
	InjectAndRunThenUnload(procId, BootstrapperPath, "LoadManagedProject", w_arg.c_str());

    return 0;
}