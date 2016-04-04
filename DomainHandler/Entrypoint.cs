using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using InjectionHelper;

namespace DomainHandler
{
    public interface IAssemblyLoader
    {
        void LoadAndRun(string file);
    }

    public class EntryPoint
    {
        [DllImport("User32.dll")]
        private static extern short GetAsyncKeyState(Keys vKey);

        [STAThread, Execute]
        public static int Main(string args)
        {
            bool firstLoaded = false;

            while (true)
            {
                if (!firstLoaded)
                {
                    firstLoaded = true;
                    new Domain(args);
                }

                if ((GetAsyncKeyState(Keys.F11) & 1) == 1)
                {
                    new Domain(args);
                }
                else if ((GetAsyncKeyState(Keys.F12) & 1) == 1)
                {
                    return 0;
                }

                Thread.Sleep(10);
            }
        }
    }

    public static class DomainManager
    {
        public static AppDomain CurrentDomain { get; set; }
        public static AssemblyLoader CurrentAssemblyLoader { get; set; }
    }

    public class AssemblyLoader : MarshalByRefObject, IAssemblyLoader
    {
        public AssemblyLoader()
        {
            AppDomain.CurrentDomain.AssemblyResolve += CurrentDomain_AssemblyResolve;
        }

        public void LoadAndRun(string file)
        {
            Assembly asm = Assembly.Load(file);
            MethodInfo entry = asm.EntryPoint;
            object classInstance = Activator.CreateInstance(entry.DeclaringType, null);
            entry.Invoke(classInstance, new object[] { new string[] { "helloworld"} });
        }
        
        private Assembly CurrentDomain_AssemblyResolve(object sender, ResolveEventArgs args)
        {
            if (args.Name == Assembly.GetExecutingAssembly().FullName)
                return Assembly.GetExecutingAssembly();

            string appDir = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            string shortAsmName = Path.GetFileName(args.Name);
            string fileName = Path.Combine(appDir, shortAsmName);

            if (File.Exists(fileName))
                return Assembly.LoadFrom(fileName);

            return Assembly.GetExecutingAssembly().FullName == args.Name ? Assembly.GetExecutingAssembly() : null;
        }
    }

    public class Domain
    {
        private readonly Random _rand = new Random();

        public Domain(string assemblyName)
        {
            try
            {
                string appBase = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
                var ads = new AppDomainSetup() {ApplicationBase = appBase, PrivateBinPath = appBase};
                DomainManager.CurrentDomain = AppDomain.CreateDomain($"Domain_Internal_{_rand.Next(0, 100000)}", null,
                    ads);
                AppDomain.CurrentDomain.AssemblyResolve += CurrentDomain_AssemblyResolve;
                DomainManager.CurrentAssemblyLoader =
                    (AssemblyLoader)
                        DomainManager.CurrentDomain.CreateInstanceAndUnwrap(Assembly.GetExecutingAssembly().FullName,
                            typeof (AssemblyLoader).FullName);

                DomainManager.CurrentAssemblyLoader.LoadAndRun(
                    Path.Combine(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), assemblyName));
            }
            catch (Exception e)
            {
                MessageBox.Show(e.ToString());
            }
            finally
            {
                DomainManager.CurrentAssemblyLoader = null;
                AppDomain.Unload(DomainManager.CurrentDomain);
            }
        }

        private Assembly CurrentDomain_AssemblyResolve(object sender, ResolveEventArgs args)
        {
            try
            {
                Assembly asm = Assembly.Load(args.Name);
                if (asm != null)
                    return asm;
            }
            catch
            {
                // ignore load error
            }

            // *** Try to load by filename - split out the filename of the full assembly name
            // *** and append the base path of the original assembly (ie. look in the same dir)
            // *** NOTE: this doesn't account for special search paths but then that never
            //           worked before either.
            string[] parts = args.Name.Split(',');
            string file = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location) + "\\" + parts[0].Trim() +
                          ".dll";

            return Assembly.LoadFrom(file);
        }
    }
}
