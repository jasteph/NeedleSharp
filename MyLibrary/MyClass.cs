using System.Windows.Forms;
using InjectionHelper;

namespace MyLibrary
{
    class MyClass
    {
        [Execute]
        public static int ShowMessage(string message)
        {
            MessageBox.Show(message);
            return 1;
        }
    }
}
