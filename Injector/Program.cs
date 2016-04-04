using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Management;
using System.Reflection;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using InjectionHelper;

namespace Injector
{
    class Program
    {
        static string PROCESS_NAME = "Skype.exe";
        static string ASSEMBLY_PATH = @"C:\MyLibrary.dll";
        static string PARAM = "Helloworld.exe";


        static void Main(string[] args)
        {
            if (args.Length < 1 || args.Length > 3)
            {
                Console.WriteLine("Usage: proc_name (param)");
                Console.ReadLine();
                return ;
            }

            PROCESS_NAME = args[0];
            ASSEMBLY_PATH = @"C:\Dev\NeedleSharp\build\DomainHandler.dll";
            PARAM = (args.Length == 2) ? args[1] : string.Empty;

            var myAssembly = Assembly.LoadFrom(ASSEMBLY_PATH);

            var methods = myAssembly.GetTypes()
                      .SelectMany(t => t.GetMethods())
                      .Where(m => m.GetCustomAttributes(typeof(Execute), false).Length > 0)
                      .ToArray();

            var executeMethod = methods.First();

            // dumpbin /headers | findstr "machine"

            var proc = Process.GetProcessesByName(PROCESS_NAME.Replace(".exe", "")).First();
            var filePath = ProcessExecutablePath(proc);
            var isProcess64Bit = Is64BitExe(filePath);

            string bootstrapName = isProcess64Bit
                ? "\\Bootstrapper_x64.dll"
                : "\\Bootstrapper_x86.dll";
            string fullName = executeMethod.DeclaringType?.FullName;
            string methodName = executeMethod.Name;

            string[] argArr = { bootstrapName, PROCESS_NAME, ASSEMBLY_PATH, fullName, methodName, PARAM };
            string argStr = string.Join(" ", argArr);

            string launcherExeName = isProcess64Bit
                ? "\\Launcher_x64.exe"
                : "\\Launcher_x86.exe";

            var launcherPath = Directory.GetCurrentDirectory() + launcherExeName;

            RunProcess(launcherPath, argStr);
        }

        private static string ProcessExecutablePath(Process process)
        {
            try
            {
                return process.MainModule.FileName;
            }
            catch
            {
                string query = "SELECT ExecutablePath, ProcessID FROM Win32_Process";
                ManagementObjectSearcher searcher = new ManagementObjectSearcher(query);

                foreach (ManagementObject item in searcher.Get())
                {
                    object id = item["ProcessID"];
                    object path = item["ExecutablePath"];

                    if (path != null && id.ToString() == process.Id.ToString())
                    {
                        return path.ToString();
                    }
                }
            }

            return "";
        }

        private static void RunProcess(string filePath, string arguments)
        {
            Process proc = new Process();
            proc.StartInfo.FileName = filePath;
            proc.StartInfo.Arguments = arguments;

            proc.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
            /*
            proc.StartInfo.RedirectStandardOutput = true;
            proc.StartInfo.RedirectStandardError = true;
            proc.StartInfo.UseShellExecute = false;
            proc.StartInfo.CreateNoWindow = true;*/

            proc.Start();
            Console.WriteLine("Waiting for proc to end.");
            proc.WaitForExit();
        }

        private static bool Is64BitExe(string filePath)
        {
            string command = $"dumpbin /headers \"{filePath}\"";
            var result = Utils.ExecuteCommandSync(command);

            string[] lines = result.Split('\n');
            var machineLine = lines.FirstOrDefault(l => l.Contains("machine"));

            if (machineLine != null)
            {
                string pat = @"x86";
                Regex r = new Regex(pat, RegexOptions.IgnoreCase);

                return !r.Match(machineLine).Success;
            }

            return false;
        }
    }
}
