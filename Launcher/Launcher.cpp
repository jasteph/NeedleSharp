#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>
#include <stdlib.h>
#include <string>

#include "Injection.h"

using namespace std;

typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

LPFN_ISWOW64PROCESS fnIsWow64Process;

/* There are two defined entry points here, one for a  Windows
 * application, and another for a console application.
 * 
 * To switch between the two versions, we need to change our
 * subsystem dependency and then rebuild. The compiler will
 * automatically choose the appropriate entry point.
 * 
 * To enable Windows (hidden) mode, go to project properties, find the
 * 'linker' section and change the SubSystem option to Windows.
 * 
 * To enable console (debugging) mode, go to project properties, find
 * the 'linker' section and change the SubSystem option to Console.
 */

/* Since there are two entry points for this program, we really
 * ought to get to a common point as soon as possible.  This is that
 * common point.
 */
void true_main() {
    // Bootstrapper
    char DllName[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, DllName);
    strcat_s(DllName, "\\Bootstrapper.dll");

    // ExampleProject
    wchar_t DllNameW[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, DllNameW);
    wcscat_s(DllNameW, L"\\ExampleProject.dll");
    
    DWORD Pid = GetProcessIdByName("Skype.exe");
    InjectAndRunThenUnload(Pid, DllName, "LoadManagedProject", DllNameW);
}

/* By starting as a Windows application but not displaying any
 * windows, we can become effectively invisible.
 */
int __stdcall WinMain (HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPSTR lpCmdLine,
                       int cmdShow)
{
    true_main();
    return 0;
}

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