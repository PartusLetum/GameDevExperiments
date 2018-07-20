using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ConsoleApplication1
{
    class Program
    {
        static void Main(string[] args)
        {
            int a, b, c;
            a = 3; b = 1; c = 3;

            int temp;
            if (a > b) { temp = b; b = a; a = temp; }
            if (b > c) { temp = c; c = b; b = temp; }
            if (a > b) { temp = b; b = a; a = temp; }

            Console.ReadKey();
        }
    }
}
