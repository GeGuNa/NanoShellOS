/*****************************************
		NanoShell Operating System
		  (C) 2021 iProgramInCpp

     Command line kernel shell module
******************************************/
#include <main.h>
#include <shell.h>
#include <keyboard.h>
#include <string.h>
#include <console.h>
#include <vga.h>
#include <video.h>
#include <print.h>
#include <memory.h>
#include <misc.h>
#include <task.h>
#include <storabs.h>
#include <window.h>
#include <icon.h>
#include <vfs.h>
#include <elf.h>
#include <cinterp.h>
#include <sb.h>
#include <fat.h>
#include <pci.h>
#include <config.h>
#include <clip.h>
#include <image.h>

char g_lastCommandExecuted[256] = {0};
extern Console* g_currentConsole;

static JumpBuffer g_setJumpTest_JumpBuffer;

static void Foo(int count)
{
	LogMsg("Foo(%d) has been called.", count);
	LongJump (g_setJumpTest_JumpBuffer, count + 1);
}
static void SetJumpTest()
{
	volatile int count = 0;
	
	if (SetJump(g_setJumpTest_JumpBuffer) != 5)
	{
		count++;
		Foo(count);
	}
}

void MemorySpy();
void ShellTaskTest(int arg)
{
	while (1)	
	{
		SLogMsg("Task %d!", arg);
		for (int i = 0; i < 3; i++)
			hlt;
	}
}

void ShellTaskTest2(int arg)
{
	//This is the entry point of the new thread.
	//You can pass any 32-bit parm in the StartTask call. `arg` is one of them.
	//var represents the next color to set
	int var = 0;
	while (1)	
	{
		// set a 100 pixel tall column at that position:
		for (int y = 100; y < 200; y++)
			VidPlotPixel (arg%GetScreenSizeX(), y+arg/GetScreenSizeX(), var);
		
		//increment color by 32
		var += 32;
		
		//wait for 5 interrupts
		for (int i = 0; i < 5; i++)
			hlt;
	}
}

void TemporaryTask(__attribute__((unused)) int arg)
{
	for (int i = 0; i < 15; i++)
	{
		//for (int j = 0; j < 10000000; j++)
		//	;
		LogMsgNoCr("HI! %d",i);
		for (int i = 0; i < 30; i++)
			hlt;
	}
}

void HeapTest()
{
	/*Heap heap;
	memset(&heap, 0, sizeof heap);
	
	AllocateHeap (&heap, 1024);
	
	UseHeap (&heap);
	
	void*ptr = MmAllocate(100000);
	LogMsg("Allocated %x on %x", ptr, &heap);
	
	FreeHeap (&heap);*/
}

extern void KeTaskDone();

typedef void (*Pointer)(unsigned color, int left, int top, int right, int bottom);

void VidPrintTestingPattern();
void VidPrintTestingPattern2();
void GraphicsTest()
{
	CoClearScreen(g_currentConsole);
	
	// Show the testing pattern first
	
	VidPrintTestingPattern();
	
	g_currentConsole->curX = g_currentConsole->curY = 0;
	LogMsg("Press any key to advance");
	CoGetChar();
	
	VidDrawRect(0x00FF00, 100, 150, 250, 250);
	
	//lines, triangles, polygons, circles perhaps?
	VidDrawHLine (0xEE00FF, 100, 500, 400);
	VidDrawVLine (0xEE00FF, 150, 550, 15);
	
	VidPlotChar('A', 200, 100, 0xFF, 0xFF00);
	VidTextOut("Hello, opaque background world.\nI support newlines too!", 300, 150, 0xFFE, 0xAEBAEB);
	VidTextOut("Hello, transparent background world.\nI support newlines too!", 300, 182, 0xFFFFFF, TRANSPARENT);
	VidShiftScreen(10);
	
	g_currentConsole->curX = g_currentConsole->curY = 0;
	LogMsg("Test complete! Strike a key to exit.");
	CoGetChar();
	g_currentConsole->color = 0x1F;
}

int  g_nextTaskNum    = 0;
bool g_ramDiskMounted = true;
int  g_ramDiskID      = 0x00;//ATA: Prim Mas
int  g_lastReturnCode = 0;
bool CoPrintCharInternal (Console* this, char c, char next);

extern char g_cwd[PATH_MAX+2];
FileNode* g_pCwdNode = NULL;

//extern Heap* g_pHeap;
extern bool  g_windowManagerRunning;
void WindowManagerShutdown ();
uint64_t ReadTSC();

void ShellExecuteCommand(char* p)
{
	TokenState state;
	state.m_bInitted = 0;
	char* token = Tokenize (&state, p, " ");
	if (!token)
		return;
	if (*token == 0)
		return;
	
	//TODO
	g_pCwdNode = FsResolvePath (g_cwd);
	
	if (strcmp (token, "help") == 0)
	{
		LogMsg("NanoShell Shell Help");
		LogMsg("cat <file>   - prints the contents of a file");
		LogMsg("cls          - clear screen");
		LogMsg("cm           - character map");
		LogMsg("cd <dir>     - change directory");
		LogMsg("cfg          - list all the kernel configuration parameters");
		LogMsg("crash        - attempt to crash the kernel");
		LogMsg("color <hex>  - change the screen color");
		LogMsg("fd           - attempts to resolve a path, prints non-zero if found");
		LogMsg("ft           - attempts to write 'Hello, world\\n' to a file");
		LogMsg("e <elf>      - executes an ELF from the initrd");
		LogMsg("el           - prints the last returned value from an executed ELF");
		LogMsg("help         - shows this list");
		LogMsg("gt           - run a graphical test");
		LogMsg("kill <pid>   - kill a thread with an id");
		LogMsg("lc           - list clipboard contents");
		LogMsg("lm           - list memory allocations");
		LogMsg("lr           - list the memory ranges provided by the bootloader");
		LogMsg("ls           - list the current working directory (right now just /)");
		LogMsg("lt           - list currently running threads (pauses them during the print)");
		LogMsg("mode X       - change the screen mode");
		LogMsg("mpt          - memory read/write bit test");
		
		//wait for new key
		LogMsg("Strike a key to print more.");
		CoGetChar();
		
		LogMsg("mspy         - Memory Spy! (TM)");
		LogMsg("mrd <file>   - mounts a RAM Disk from a file");
		LogMsg("ph           - prints current heap's address in kernel address space (or NULL for the default heap)");
		LogMsg("rb <--force> - reboots the system");
		LogMsg("sysinfo      - dump system information");
		LogMsg("sysinfoa     - dump advanced system information");
		LogMsg("time         - get timing information");
		LogMsg("sjt          - SetJump() and LongJump() test");
		LogMsg("stt          - spawns a single thread that doesn't last forever");
		LogMsg("st           - spawns a single thread that makes a random line forever");
		LogMsg("tt           - spawns 64 threads that makes random lines forever");
		LogMsg("tte          - spawns 1024 threads that makes random lines forever");
		LogMsg("ttte         - spawns 1024 threads that prints stuff");
		LogMsg("ver          - print system version");
		LogMsg("w            - start desktop manager");
	}
	else if (strcmp (token, "fac") == 0)
	{
		StFlushAllCaches();
	}
	else if (strcmp (token, "lh") == 0)
	{
		StDebugDumpAll();
	}
	else if (strcmp (token, "sjt") == 0)
	{
		SetJumpTest();
	}
	else if (strcmp (token, "mpt") == 0)
	{
		// to test out whether a read only page would actually crash the shell
		void *mem = MmMapPhysMemFastRW (0x7000, false);
		
		LogMsg("Mapped the single page.  Its address in virtual memory is %p. Testing write to it...", mem);
		
		uint32_t* ptr = ((uint32_t*)mem);
		
		uint32_t read = *ptr;
		
		*ptr = 5;
		
		LogMsg("Seems like I can write there.");
		
		*ptr = read;
		
		MmUnmapPhysMemFast(mem);
	}
	else if (strcmp (token, "ta") == 0) // Test ANSI
	{
		LogMsg(
		//"\e[15T" // Scroll 15 lines up
		"You can't see this!"
		"\e[1K"    // Erase from beginning of the line to the cursor's position.
		"\e[30G"   // Move cursor horizontally to x=30
		"Testing at 30 characters!"
		"\e[2;2H"  // Move cursor at (1, 1) - coordinates are 1 based
		"Testing at (1, 1)!"
		"\e[94m"   // Set foreground color to bright red
		" Now in blue."
		"\e[39m"   // Reset foreground color to default
		);
	}
	else if (strcmp (token, "pwd") == 0)
	{
		LogMsg(g_cwd);
	}
	else if (strcmp (token, "rb") == 0)
	{
		bool force = false;
		char* fileName = Tokenize (&state, NULL, " ");
		if (fileName)
		{
			if (strcmp (fileName, "--force") == 0) force = true;
		}
		if (!IsWindowManagerRunning())
		{
			KeRestartSystem();
		}
		else if (force)
		{
			WindowManagerShutdown ();
		}
		else
			LogMsg("Use the launcher's \"Shutdown computer\" option, shut down the computer, and click \"Restart\" to reboot, or use --force.");
	}
	else if (strcmp (token, "th") == 0)
	{
		HeapTest();
	}
	else if (strcmp (token, "ph") == 0)
	{
		LogMsg("Current Heap: %p", MuGetCurrentHeap());
	}
	else if (strcmp (token, "cd") == 0)
	{
		char* fileName = Tokenize (&state, NULL, " ");
		if (!fileName)
			LogMsg("Expected filename");
		else if (*fileName == 0)
			LogMsg("Expected filename");
		else
		{
			int result = FiChangeDir(fileName);
			if (result)
			{
				LogMsg("cd: %s: %s", fileName, GetErrNoString(result) );
			}
		}
	}
	else if (strcmp (token, "cm") == 0)
	{
		for (int y = 0; y < 16; y++)
			for (int x = 0; x < 16; x++)
			{
				CoPlotChar(g_currentConsole, x, y, (y<<4)|x);
			}
	}
	else if (strcmp (token, "el") == 0)
	{
		LogMsg("Last run ELF returned: %d", g_lastReturnCode);
	}
	else if (strcmp (token, "e") == 0)
	{
		char* fileName = Tokenize (&state, NULL, " ");
		if (!fileName)
		{
			LogMsg("Expected filename");
		}
		else if (*fileName == 0)
		{
			LogMsg("Expected filename");
		}
		else
		{
			int* er = MmAllocateK(sizeof(int));
			int ec = ElfRunProgram(fileName, state.m_pContinuation, false, false, GetDefaultHeapSize(), er);
			
			if (ec != ELF_ERROR_NONE)
			{
				LogMsg(ElfGetErrorMsg(ec), fileName);
			}
			
			MmFreeK(er);
			
			LogMsg("");
		}
	}
	else if (strcmp (token, "ec") == 0)
	{
		char* fileName = Tokenize (&state, NULL, " ");
		if (!fileName)
		{
			LogMsg("Expected filename");
		}
		else if (*fileName == 0)
		{
			LogMsg("Expected filename");
		}
		else
		{
			int fd = FiOpen (fileName, O_RDONLY);
			if (fd < 0)
			{
				LogMsg("ec: %s: %s", fileName, GetErrNoString(fd));
				return;
			}
			
			int length = FiTellSize(fd);
			
			char* pData = (char*)MmAllocate(length + 1);
			pData[length] = 0;
			
			FiRead(fd, pData, length);
			
			FiClose(fd);
			
			//ElfExecute(pData, length);
			
			CCSTATUS status = CcRunCCode(pData, length);
			LogMsg("Exited with status %d", status);
			
			MmFree(pData);
			
			LogMsg("");
		}
	}
	else if (strcmp (token, "image") == 0)
	{
		char* fileName = Tokenize (&state, NULL, " ");
		if (!fileName)
		{
			LogMsg("Expected filename");
		}
		else if (*fileName == 0)
		{
			LogMsg("Expected filename");
		}
		else
		{
			int fd = FiOpen (fileName, O_RDONLY);
			if (fd < 0)
			{
				LogMsg("image: %s: %s", fileName, GetErrNoString(fd));
				return;
			}
			
			int length = FiTellSize(fd);
			
			char* pData = (char*)MmAllocate(length + 1);
			pData[length] = 0;
			
			FiRead(fd, pData, length);
			
			FiClose(fd);
			
			// try to load an image
			int error = 0;
			Image* pImg;
			
			pImg = LoadImageFile(pData, &error);
			if (error)
			{
				// can't
				LogMsg("Could not load the image (%d).", error);
			}
			else
			{
				LogMsg("Image %dx%d", pImg->width, pImg->height);
				VidBlitImage(pImg, 0, 0);
				
				MmFree(pImg);
			}
			MmFree(pData);
			
			LogMsg("");
		}
	}
	else if (strcmp (token, "cat") == 0)
	{
		char* fileName = Tokenize (&state, NULL, " ");
		if (!fileName)
		{
			LogMsg("Expected filename");
		}
		else if (*fileName == 0)
		{
			LogMsg("Expected filename");
		}
		else
		{
			int fd = FiOpen (fileName, O_RDONLY);
			if (fd < 0)
			{
				LogMsg("cat: %s: %s", fileName, GetErrNoString(fd));
				return;
			}
			
			FiSeek(fd, 0, SEEK_SET);
			
			int result; char data[2];
			while ((result = FiRead(fd, data, 1), result > 0))
			{
				CoPrintChar(g_currentConsole, data[0]);
			}
			
			FiClose (fd);
			LogMsg("");
		}
	}
	else if (strcmp (token, "rm") == 0)
	{
		char* fileName = Tokenize (&state, NULL, " ");
		if (!fileName)
		{
			LogMsg("Expected filename");
		}
		else if (*fileName == 0)
		{
			LogMsg("Expected filename");
		}
		else
		{
			// Get rid of the file.
			int io = FiRemoveFile (fileName);
			if (io < 0)
			{
				LogMsg("rm: %s: %s", fileName, GetErrNoString(io));
				return;
			}
			
			LogMsg("Done");
		}
	}
	else if (strcmp (token, "movedata") == 0)
	{
		LogMsg("Starting to copy...");
		char* fileNameOut = Tokenize (&state, NULL, " ");
		if (!fileNameOut)
		{
			LogMsg("Usage: movedata <output> <input>");
			goto fail_movedata;
		}
		if (*fileNameOut == 0)
		{
			LogMsg("Usage: movedata <output> <input>");
			goto fail_movedata;
		}
		
		char* fileNameIn = Tokenize (&state, NULL, " ");
		if (!fileNameIn)
		{
			LogMsg("Usage: movedata <output> <input>");
			goto fail_movedata;
		}
		if (*fileNameIn == 0)
		{
			LogMsg("Usage: movedata <output> <input>");
			goto fail_movedata;
		}
		
		int fd_in = FiOpen (fileNameIn, O_RDONLY);
		if (fd_in < 0)
		{
			LogMsg("movedata: Could not open %s for reading: %s", fileNameIn, GetErrNoString(fd_in));
			goto fail_movedata;
		}
		
		int fd_out = FiOpen (fileNameOut, O_WRONLY | O_CREAT);
		if (fd_out < 0)
		{
			LogMsg("movedata: Could not open %s for writing: %s", fileNameOut, GetErrNoString(fd_out));
			FiClose(fd_in);
			goto fail_movedata;
		}
		
		LogMsg("Progress...");
		
		#define MOVEDATA_PRECISION 4096
		
		uint8_t chunk_of_data[MOVEDATA_PRECISION];
		size_t sz = FiTellSize(fd_in);
		
		//write 512 byte blocks
		for (int i = 0; i < sz; i += MOVEDATA_PRECISION)
		{
			size_t read_in = FiRead(fd_in, chunk_of_data, MOVEDATA_PRECISION);
			if ((int)read_in < 0)
			{
				LogMsg("movedata: Could not write all %d bytes to %s - only wrote %d: %s", MOVEDATA_PRECISION, fileNameOut, read_in, GetErrNoString((int)read_in));
				FiClose(fd_in);
				FiClose(fd_out);
				goto fail_movedata;
			}
			
			FiWrite(fd_out, chunk_of_data, read_in);
			
			LogMsgNoCr("\rProgress: %d/%d", i, sz);
		}
		
		// write the last few bytes
		int last_x_block_size = (sz % MOVEDATA_PRECISION);
		for (int i = 0; i < last_x_block_size; i++)
		{
			size_t read_in = FiRead(fd_in, chunk_of_data, 1);
			if ((int)read_in < 0)
			{
				LogMsg("movedata: Could not write 1 byte to %s - only wrote %d: %s", fileNameOut, read_in, GetErrNoString((int)read_in));
				FiClose(fd_in);
				FiClose(fd_out);
				goto fail_movedata;
			}
			
			FiWrite(fd_out, chunk_of_data, read_in);
			
			LogMsgNoCr("\rProgress: %d/%d", i, sz);
		}
		
		LogMsg("");
		
		FiClose(fd_out);
		FiClose(fd_in);
		LogMsg("Done");
		
	fail_movedata:;
	}
	else if (strcmp (token, "fts") == 0)
	{
		char* fileName = Tokenize (&state, NULL, " ");
		if (!fileName)
		{
			LogMsg("Expected filename");
		}
		else if (*fileName == 0)
		{
			LogMsg("Expected filename");
		}
		else
		{
			int fd = FiOpen (fileName, O_WRONLY);
			if (fd < 0)
			{
				LogMsg("fts: %s: %s", fileName, GetErrNoString(fd));
				return;
			}
			
			FiSeek(fd, 0, SEEK_END);
			
			size_t   sz;
			void* data = SbTestGenerateSound(&sz);
			
			FiWrite(fd, data, sz);//do not also print the null terminator
			
			MmFree(data);
			
			FiClose (fd);
			LogMsg("Done");
		}
	}
	else if (strcmp (token, "ft") == 0)
	{
		char* fileName = Tokenize (&state, NULL, " ");
		if (!fileName)
		{
			LogMsg("Expected filename");
		}
		else if (*fileName == 0)
		{
			LogMsg("Expected filename");
		}
		else
		{
			int fd = FiOpen (fileName, O_WRONLY);
			if (fd < 0)
			{
				LogMsg("ft: %s: %s", fileName, GetErrNoString(fd));
				return;
			}
			
			FiSeek(fd, 0, SEEK_END);
			
			char text[] = "Hello World from FiWrite!\n\n\n";
			
			FiWrite(fd, text, sizeof(text)-1);//do not also print the null terminator
			
			FiClose (fd);
			LogMsg("Done");
		}
	}
	else if (strcmp (token, "lr") == 0)
	{
		KePrintMemoryMapInfo();
	}
	else if (strcmp (token, "ls") == 0)
	{
		uint8_t color = g_currentConsole->color;
		
		FileNode* pNode = g_pCwdNode;
		LogMsg("\x01\x0F" "Directory of %s", pNode->m_name, pNode);
		
		bool bareMode = false;
		
		int dd = FiOpenDir (g_cwd);
		if (dd < 0)
		{
			LogMsg("ls: cannot list '%s': %s", g_cwd, GetErrNoString(dd));
			return;
		}
		
		FiRewindDir(dd);
		
		DirEnt* pDirEnt;
		while ((pDirEnt = FiReadDir(dd)) != NULL)
		{
			if (bareMode)
			{
				LogMsg("%s", pDirEnt->m_name);
				continue;
			}
			
			StatResult statResult;
			int res = FiStatAt (dd, pDirEnt->m_name, &statResult);
			
			if (res < 0)
			{
				LogMsg("ls: cannot stat '%s': %s", pDirEnt->m_name, GetErrNoString(res));
				continue;
			}
			#define THING "\x10"
			if (statResult.m_type & FILE_TYPE_DIRECTORY)
			{
				LogMsg("%c%c%c\x02" THING "\x01\x0C%s\x01\x0F",
					"-r"[!!(statResult.m_perms & PERM_READ )],
					"-w"[!!(statResult.m_perms & PERM_WRITE)],
					"-x"[!!(statResult.m_perms & PERM_EXEC )],
					pDirEnt->m_name
				);
			}
			else
			{
				LogMsg("%c%c%c %d\x02" THING "%s",
					"-r"[!!(statResult.m_perms & PERM_READ )],
					"-w"[!!(statResult.m_perms & PERM_WRITE)],
					"-x"[!!(statResult.m_perms & PERM_EXEC )],
					statResult.m_size,
					pDirEnt->m_name
				);
			}
			#undef THING
		}
		
		g_currentConsole->color = color;
		
		FiCloseDir(dd);
	}
	else if (strcmp (token, "gt") == 0)
	{
		GraphicsTest();
	}
	else if (strcmp (token, "xyzzy") == 0)
	{
		LogMsg("Huzzah!");
	}
	else if (strcmp (token, "w") == 0)
	{
		if (VidIsAvailable())
		{
			WindowManagerTask(0);
		}
		else
			LogMsg("Cannot run window manager in text mode.  Restart your computer, then make sure the gfxpayload is valid in GRUB.");
	}
	else if (strcmp (token, "mrd") == 0)
	{
		char* fileName = Tokenize (&state, NULL, " ");
		if (!fileName)
		{
			LogMsg("You want to mount what, exactly?");
		}
		else if (*fileName == 0)
		{
			LogMsg("You want to mount what, exactly?");
		}
		else
		{
			int fd = FiOpen (fileName, O_RDONLY);
			if (fd < 0)
			{
				LogMsg("Got error code %d when opening file", fd);
				return;
			}
			
			int length = FiTellSize(fd);
			
			char* pData = (char*)MmAllocate(length + 1);
			pData[length] = 0;
			
			FiRead(fd, pData, length);
			
			FiClose(fd);
			
			FsMountRamDisk(pData);
			
			//Do not free as the file system now owns this pointer.
		}
	}
	else if (strcmp (token, "cfg") == 0)
	{
		CfgPrintEntries ();
	}
	else if (strcmp (token, "export") == 0)
	{
		char *parms = state.m_pContinuation;
		if (!parms)
			LogMsg("No parms provided");
		else
			CfgLoadFromParms (parms);
	}
	else if (strcmp (token, "kill") == 0)
	{
		char* procNum = Tokenize (&state, NULL, " ");
		if (!procNum)
		{
			LogMsg( "No pid provided" );
		}
		if (*procNum == 0)
		{
			LogMsg( "No pid provided" );
		}
		
		int proc = atoi (procNum);
		
		KeKillThreadByPID (proc);
	}
	else if (strcmp (token, "check") == 0)
	{
		char* fatNum = Tokenize (&state, NULL, " ");
		if (!fatNum)
		{
			goto print_usage1;
		}
		if (*fatNum == 0)
		{
			goto print_usage1;
		}
		
//		int nFat = atoi (fatNum);
		
//		CheckDiskFatMain (nFat);
		
		goto dont_print_usage1;
	print_usage1:
		LogMsg("Check Disk");
		LogMsg("Usage: check <FAT file system number>");
	dont_print_usage1:;
	}
	else if (strcmp (token, "mspy") == 0)
	{
		char* secNum = Tokenize (&state, NULL, " ");
		if (!secNum)
		{
			goto print_usage;
		}
		if (*secNum == 0)
		{
			goto print_usage;
		}
		
		if (strcmp (secNum, "/f") == 0)
		{
			MemorySpy();
			return;
		}
		
		char* nBytesS = Tokenize(&state, NULL, " ");
		if (!nBytesS)
		{
			goto print_usage;
		}
		if (*nBytesS == 0)
		{
			goto print_usage;
		}
		
		char* auxSwitch = Tokenize(&state, NULL, " ");
		bool as_bytes = false;
		if (auxSwitch && *auxSwitch != 0)
		{
			if (strcmp (auxSwitch, "/b") == 0)
				as_bytes = true;
		}
		
		int nAddr = atoihex (secNum);
		int nBytes= atoi (nBytesS);
		
		uint32_t* pAddr = (uint32_t*)((uintptr_t)nAddr);
		
		extern bool MmIsMapped(uintptr_t addr);
		
		DumpBytesAsHex ((void*)pAddr, nBytes, as_bytes);
		
		goto dont_print_usage;
	print_usage:
		LogMsg("Virtual Memory Spy (TM)");
		LogMsg("Usage: mspy <address hex> <numBytes> [/b]");
		LogMsg("  OR   mspy /f");
		LogMsg("- If [/f] is specified, a full-screen menu will popup");
		LogMsg("- bytes will be printed as groups of 4 unless [/b] is specified");
		LogMsg("- numBytes will be capped off at 4096 and rounded down to 32");
		LogMsg("- pageNumber must represent a \x01\x0CVALID\x01\x0F and \x01\x0CMAPPED\x01\x0F address.");
		LogMsg("- if it's not valid or mapped then the system may CRASH or HANG!");
		LogMsg("- pageNumber is in\x01\x0C HEXADECIMAL\x01\x0F");
	dont_print_usage:;
	}
	else if (strcmp (token, "cls") == 0)
	{
		CoClearScreen (g_currentConsole);
		g_currentConsole->curX = g_currentConsole->curY = 0;
	}
	else if (strcmp (token, "gv") == 0)
	{
		extern volatile uint32_t gVmwCounter2;
		LogMsg("gVmwCounter2: %u", gVmwCounter2);
	}
	else if (strcmp (token, "sb") == 0)
	{
		LogMsg("Playing 1000 hz tone... (todo)");
	}
	else if (strcmp (token, "ver") == 0)
	{
		KePrintSystemVersion();
	}
	else if (strcmp (token, "lf") == 0)
	{
		FiDebugDump();
	}
	else if (strcmp (token, "lm") == 0)
	{
		MmDebugDump();
	}
	else if (strcmp (token, "lc") == 0)
	{
		CbDump();
	}
	else if (strcmp (token, "tc") == 0)
	{
		CbCopyText("Hello, world!");
	}
	else if (strcmp (token, "lp") == 0)
	{
		PciDump();
	}
	else if (strcmp (token, "lt") == 0)
	{
		KeTaskDebugDump();
	}
	else if (strcmp (token, "stt") == 0)
	{
		int errorCode = 0;
		Task* task = KeStartTask(TemporaryTask, g_nextTaskNum++, &errorCode);
		LogMsg("Task %d (%x) spawned.  Error code: %x", g_nextTaskNum - 1, task, errorCode);
	}
	else if (strcmp (token, "st") == 0)
	{
		int errorCode = 0;
		Task* task = KeStartTask(ShellTaskTest2, g_nextTaskNum++, &errorCode);
		LogMsg("Task %d (%x) spawned.  Error code: %x", g_nextTaskNum - 1, task, errorCode);
	}
	else if (strcmp (token, "tt") == 0)
	{
		int errorCode = 0;
		for (int i = 0; i < 64; i++)
		{
			KeStartTask(ShellTaskTest2, g_nextTaskNum++, &errorCode);
		}
		LogMsg("Tasks have been spawned.");
	}
	else if (strcmp (token, "tte") == 0)
	{
		int errorCode = 0;
		for (int i = 0; i < 1024; i++)
		{
			KeStartTask(ShellTaskTest2, g_nextTaskNum++, &errorCode);
		}
		LogMsg("Tasks have been spawned.");
	}
	else if (strcmp (token, "ttte") == 0)
	{
		int errorCode = 0;
		for (int i = 0; i < 128; i++)
		{
			KeStartTask(TemporaryTask, g_nextTaskNum++, &errorCode);
		}
		LogMsg("Tasks have been spawned.");
	}
	else if (strcmp (token, "crash") == 0)
	{
		LogMsg("OK");
		*((uint32_t*)0xFFFFFFFF) = 0;
	}
	else if (strcmp (token, "time") == 0)
	{
		uint32_t  hi, lo;
		GetTimeStampCounter(&hi, &lo);
		LogMsg("Timestamp counter: %x%x (%d, %d)", hi, lo, hi, lo);
		
		//int tkc = GetTickCount(), rtkc = GetRawTickCount();
		//LogMsgNoCr("Tick count: %d, Raw tick count: %d", tkc, rtkc);
		LogMsg("Press any key to stop timing.");
		
		while (CoInputBufferEmpty())
		{
			int tkc = GetTickCount(), rtkc = GetRawTickCount();
			LogMsgNoCr("\rTick count: %d, Raw tick count: %d        ", tkc, rtkc);
			//for(int i=0; i<50; i++) 
			hlt;
		}
	}
	else if (strcmp (token, "mode") == 0)
	{
		if (VidIsAvailable())
		{
			LogMsg("Must use emergency text-mode shell to change mode.");
			return;
		}
		char* modeNum = Tokenize (&state, NULL, " ");
		if (!modeNum)
		{
			LogMsg("Expected mode number");
		}
		else if (*modeNum == 0)
		{
			LogMsg("Expected mode number");
		}
		else
		{
			SwitchMode (*modeNum - '0');
			//PrInitialize();
			CoInitAsText(g_currentConsole);
		}
	}
	else if (strcmp (token, "font") == 0)
	{
		char* fontNum = Tokenize (&state, NULL, " ");
		if (!fontNum)
		{
			LogMsg("Expected mode number");
		}
		else if (*fontNum == 0)
		{
			LogMsg("Expected font number");
		}
		else
		{
			VidSetFont (*fontNum - '0');
			LogMsg("the quick brown fox jumps over the lazy dog");
			LogMsg("THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG");
			LogMsg("Font testing done.");
			//PrInitialize();
		}
	}
	else if (strcmp (token, "color") == 0)
	{
		char* colorNum = Tokenize (&state, NULL, " ");
		if (!colorNum)
		{
			LogMsg("Expected color hex");
		}
		else if (*colorNum == 0 || *(colorNum + 1) == 0)
		{
			LogMsg("Expected color hex");
		}
		else
		{
			//SwitchMode (*modeNum - '0');
			char c1 = colorNum[0], c2 = colorNum[1];
			
			/**/ if (c1 >= '0' && c1 <= '9') c1 -= '0';
			else if (c1 >= 'A' && c1 <= 'F') c1 -= 'A'-0xA;
			else if (c1 >= 'a' && c1 <= 'f') c1 -= 'a'-0xA;
			
			/**/ if (c2 >= '0' && c2 <= '9') c2 -= '0';
			else if (c2 >= 'A' && c2 <= 'F') c2 -= 'A'-0xA;
			else if (c2 >= 'a' && c2 <= 'f') c2 -= 'a'-0xA;
			
			g_currentConsole->color = c1 << 4 | c2;
		}
	}
	else if (strcmp (token, "sysinfo") == 0)
	{
		KePrintSystemInfo();
	}
	else if (strcmp (token, "sysinfoa") == 0)
	{
		KePrintSystemInfoAdvanced();
	}
	else
	{
		LogMsg("Unknown command.  Please type 'help'.");
	}
	
	//LogMsg("You typed: '%s'", p);
}

void ShellInit()
{
	strcpy (g_cwd, "/");
	g_pCwdNode = FsResolvePath (g_cwd);
	
	bool b = CbCopyText("movedata /Device/Sb16 /Fat0/sup/crap.raw\n");
	if (!b)
		LogMsg("Error copying text");
}

void ShellPrintMotd()
{
	if (!CfgEntryMatches("Shell::ShowMotd", "yes")) return;
	
	//TODO: Hmm, maybe we should allow multiline MOTD
	const char *pValue = CfgGetEntryValue("Shell::Motd");
	
	bool bCenter = CfgEntryMatches ("Shell::MotdCenter", "yes");
	
	if (bCenter)
	{
		int slen = strlen (pValue);
		if (slen <= g_currentConsole->width)
		{
			//output a number of spaces until the motd itself
			int up_to = (g_currentConsole->width - slen) / 2;
			for (int i = 0; i < up_to; i++) LogMsgNoCr(" ");
		}
	}
	LogMsg("%s", pValue);
}

void ShellRun(UNUSED int unused_arg)
{
	ShellPrintMotd();
	
	while (1) 
	{
		LogMsgNoCr("%s>", g_cwd);
		char buffer[256];
		CoGetString (buffer, 256);
		memcpy (g_lastCommandExecuted, buffer, 256);
		
		ShellExecuteCommand (buffer);
		
		WaitMS (1);
	}
}
