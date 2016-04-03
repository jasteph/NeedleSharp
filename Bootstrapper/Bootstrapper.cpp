#include "stdafx.h"
#include <metahost.h>
#include <comdef.h>
#include <string>
#pragma comment(lib, "mscoree.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Framedyn.lib")

using namespace std;

#include "Bootstrapper.h"

ICLRMetaHost* pClrMetaHost = NULL;
ICLRRuntimeInfo* pClrRuntimeInfo = NULL;
ICLRRuntimeHost* pClrRuntimeHost = NULL;

//
// Parses arguments used to invoke a managed assembly
//
struct ClrArgs
{
	static const LPCWSTR DELIM;

	ClrArgs(LPCWSTR command)
	{
		int i = 0;
		wstring s(command);
		wstring* ptrs[] = { &pwzAssemblyPath, &pwzTypeName, &pwzMethodName };

		while (s.find(DELIM) != wstring::npos && i < 3)
		{
			*ptrs[i++] = s.substr(0, s.find(DELIM));
			s.erase(0, s.find(DELIM) + 1);
		}

		if (s.length() > 0)
			pwzArgument = s;
	}

	wstring pwzAssemblyPath;
	wstring pwzTypeName;
	wstring pwzMethodName;
	wstring pwzArgument;
};

const LPCWSTR ClrArgs::DELIM = L"\t"; // delimiter

DllExport void LoadManagedProject(const wchar_t * command)
{
	ICLRRuntimeHost* host = GetRuntimeHost(L"v4.0.30319");	

	if (host != NULL) {
		// parse the arguments
		ClrArgs args(command);

		ExecuteClrCode(host, args.pwzAssemblyPath.c_str(), args.pwzTypeName.c_str(), args.pwzMethodName.c_str(), L"MyLibrary.dll");
		
		// Cleanup
		host->Release();
	}
}


/// <summary>
/// Returns a pointer to a running CLR host of the specified version
/// </summary>
/// <param name="dotNetVersion">The exact version number of the CLR you want to
/// run. This can be obtained by looking in the C:\Windows\Microsoft.NET\Framework
/// directory and copy the name of the last directory.</param>
/// <returns>A running CLR host or NULL. You are responsible for calling Release() on it.</returns>
ICLRRuntimeHost* GetRuntimeHost(LPCWSTR dotNetVersion)
{
	ICLRMetaHost* metaHost = NULL;
	ICLRRuntimeInfo* info = NULL;
	ICLRRuntimeHost* runtimeHost = NULL;

	// Get the CLRMetaHost that tells us about .NET on this machine
	if (S_OK == CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, (LPVOID*)&metaHost))
	{
		// Get the runtime information for the particular version of .NET
		if (S_OK == metaHost->GetRuntime(dotNetVersion, IID_ICLRRuntimeInfo, (LPVOID*)&info))
		{
			// Get the actual host
			if (S_OK == info->GetInterface(CLSID_CLRRuntimeHost, IID_ICLRRuntimeHost, (LPVOID*)&runtimeHost))
			{
				// Start it. This is okay to call even if the CLR is already running
				runtimeHost->Start();
			}
		}
	}
	// Cleanup
	if (info != NULL)
		info->Release();
	if (metaHost != NULL)
		metaHost->Release();

	return runtimeHost;
}

/// <summary>
/// Executes some code in the CLR. The function must have the signature: public static int Function(string param)
/// </summary>
/// <param name="host">A started instance of the CLR</param>
/// <param name="assemblyPath">The full path to your compiled code file.
/// i.e. "C:\MyProject\MyCode.dll"</param>
/// <param name="typeName">The full type name of the class to be called, including the
/// namespace. i.e. "MyCode.MyClass"</param>
/// <param name="function">The name of the function to be called. i.e. "MyFunction"</param>
/// <param name="param">A string parameter to pass to the function.</param>
/// <returns>The integer return code from the function or -1 if the function did not run</returns>
int ExecuteClrCode(ICLRRuntimeHost* host, LPCWSTR assemblyPath, LPCWSTR typeName,
	LPCWSTR function, LPCWSTR param)
{
	if (host == NULL)
		return -1;

	if (!PathFileExists(assemblyPath)) {
		wstring path(assemblyPath);
		wstring errorMessage = L"ExecuteClrCode: Couldn't find dll (" + path + L") to inject";
		MessageBox(0, errorMessage.c_str(), L"Error", MB_OK);
		return -1;
	}

	DWORD result = -1;
	if (host->ExecuteInDefaultAppDomain(assemblyPath, typeName, function, param, &result) != S_OK)
		return -1;

	return result;
}