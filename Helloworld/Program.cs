﻿using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Helloworld
{
    class Program
    {
        static void Main(string[] args)
        {
            MessageBox.Show($"The current process is {Process.GetCurrentProcess().ProcessName}!!!", args[0],
                MessageBoxButtons.OK);

        }
    }
}
