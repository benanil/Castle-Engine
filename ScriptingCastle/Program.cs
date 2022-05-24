using System;
using System.Runtime.CompilerServices;

namespace ScriptingCastle
{
    public unsafe class Program
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void PrintHelloCsharp();
		
		public struct Birki
		{
			public int bir, iki;
		}
	
		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern static Birki RefTest(Birki birki);

		public static void Tick(float deltaTime)
        { 
            
        }

		public static void Initialize()
		{
            
		}

        internal static int ConvertToInt(float* ptr)
        {
            return *(int*)ptr;
        }

        public static int Main()
        {
			float test = 3.0f;
            int[] array = new int[4] { 1, 2, 3, 4 };

            PrintHelloCsharp();

			Birki birki = new Birki();
			
			birki.bir = 1;
			birki.iki = 2;
		
            birki = RefTest(birki);

            Console.WriteLine("csharp: " + birki.bir.ToString() + birki.iki);

            Console.WriteLine(test);
			return 33;
        }
    }
}
