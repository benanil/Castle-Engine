using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace ScriptingCastle
{
    public class Program
    {
        // [DllImport("__Internal", EntryPoint = "PrintHelloCsharp")]
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static string PrintHelloCsharp();

        public static void Tick(float deltaTime)
        { 
            
        }

        public static void Main()
        {
            Console.WriteLine("adfasdfasdfasdf");
            PrintHelloCsharp();
        }
    }
}
