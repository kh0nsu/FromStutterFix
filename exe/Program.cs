using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace FromStutterFix
{
    class Program
    {
        [DllImport("kernel32.dll")]
        private static extern IntPtr OpenProcess(uint dwDesiredAcess, bool bInheritHandle, int dwProcessId);
        const uint PROCESS_ALL_ACCESS = 2035711;

        [DllImport("kernel32.dll")]
        private static extern bool ReadProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, int iSize, ref int lpNumberOfBytesRead);

        [DllImport("kernel32.dll")]
        private static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, int iSize, int lpNumberOfBytesWritten);

        [DllImport("kernel32.dll")]
        private static extern bool CloseHandle(IntPtr hObject);

        public IntPtr ReadPtr(IntPtr handle, IntPtr addr)
        {
            var array = new byte[8];
            var lpNumberOfBytesRead = 1;
            ReadProcessMemory(handle, addr, array, 8, ref lpNumberOfBytesRead);
            return (IntPtr)BitConverter.ToUInt64(array, 0);
        }

        public byte ReadByte(IntPtr handle, IntPtr addr)
        {
            var array = new byte[1];
            var lpNumberOfBytesRead = 1;
            ReadProcessMemory(handle, addr, array, 1, ref lpNumberOfBytesRead);
            return array[0];
        }

        public void WriteByte(IntPtr handle, IntPtr addr, byte val)
        {
            var array = new byte[] { val };
            WriteProcessMemory(handle, addr, array, array.Length, 0);
        }

        List<Game> games = new List<Game>()
        {
            new Game(){ exename = "darksoulsiii", ptrAddr = 0x494E9D8, offset = 0x24b }, //1.15
            new Game(){ exename = "sekiro", ptrAddr = 0x3F42B28, offset = 0x23b }, //1.06
            new Game(){ exename = "eldenring", ptrAddr = 0x44F5828, offset = 0x88b }, //1.05
        };

        public void run()
        {
            bool foundGame = false;
            var processes = Process.GetProcesses();
            foreach (Process process in processes)
            {
                foreach (var game in games)
                {
                    if (process.ProcessName.ToLower().Equals(game.exename.ToLower()) && !process.HasExited)
                    {
                        foundGame = true;
                        Console.WriteLine("Game: " + game.exename);
                        var handle = OpenProcess(PROCESS_ALL_ACCESS, false, process.Id);
                        if (handle == IntPtr.Zero)
                        {
                            Console.WriteLine("Cannot open process");
                            break;
                        }
                        IntPtr baseAddr = IntPtr.Zero;
                        foreach (var module in process.Modules)
                        {
                            var processModule = module as ProcessModule;
                            var modNameLower = processModule.ModuleName.ToLower();
                            if (modNameLower == game.exename + ".exe")
                            {
                                baseAddr = processModule.BaseAddress;
                            }
                        }
                        if (baseAddr == IntPtr.Zero)
                        {
                            Console.WriteLine("Could not find base address");
                            CloseHandle(handle);
                            break;
                        }

                        var addr = ReadPtr(handle, baseAddr + game.ptrAddr) + game.offset;
                        var state = ReadByte(handle, addr);
                        if (state == 1)
                        {
                            Console.WriteLine("Flag already set");
                            CloseHandle(handle);
                            break;
                        }
                        WriteByte(handle, addr, 1);
                        Console.WriteLine("Flag set");
                        CloseHandle(handle);
                    }
                }
            }
            if (!foundGame) { Console.WriteLine("No game found"); }
            Console.ReadLine();
        }

        static void Main(string[] args)
        {
            new Program().run();
        }
    }

    class Game
    {
        public string exename { get; set; }
        //TODO: game version and/or an AOB for checking it
        public int ptrAddr { get; set; }
        //this gets a pointer to the user input manager eg. DLUserInputManagerImpl.
        //the instance is found by RTTI scan. the pointer to it can be found by pointerscan or just scanning for an 8-byte value.
        public int offset { get; set; }
        //offset is a bit harder to find.
        //example from sekiro of the target function (after unpacking with steamless; not needed for other games):
        /*
0000000141A31670 | 40:56                           | push rsi                                              |
0000000141A31672 | 48:83EC 30                      | sub rsp,30                                            |
0000000141A31676 | 48:895C24 40                    | mov qword ptr ss:[rsp+40],rbx                         |
0000000141A3167B | 48:8BF1                         | mov rsi, rcx                                          |
0000000141A3167E | 48:8B59 50                      | mov rbx, qword ptr ds:[rcx+50]                        |
0000000141A31682 | 48:8959 58                      | mov qword ptr ds:[rcx+58],rbx                         |
0000000141A31686 | 80B9 3B020000 00                | cmp byte ptr ds:[rcx+23B],0                           | <--- the flag
0000000141A3168D | 48:8B5C24 40                    | mov rbx, qword ptr ss:[rsp+40]                        |
0000000141A31692 | 75 21                           | jne sekiro.exe.unpacked.141A316B5                     |
0000000141A31694 | 48:8B49 40                      | mov rcx, qword ptr ds:[rcx+40]                        |
0000000141A31698 | 4C:8D05 C1060000                | lea r8, qword ptr ds:[<sub_141A31D60>]                |
0000000141A3169F | 4C:8BCE                         | mov r9, rsi                                           |
0000000141A316A2 | C74424 20 01000000              | mov dword ptr ss:[rsp+20],1                           |
0000000141A316AA | BA 04000000                     | mov edx,4                                             |
0000000141A316AF | 48:8B01                         | mov rax, qword ptr ds:[rcx]                           |
0000000141A316B2 | FF50 20                         | call qword ptr ds:[rax+20]                            |
0000000141A316B5 | 48:83C4 30                      | add rsp,30                                            |
0000000141A316B9 | 5E                              | pop rsi                                               |
0000000141A316BA | C3                              | ret                                                   |
        */
    }
}
