/*
	Real-Time Audio Delay 2 application for Windows
	Version 1.3.1 (Interop C# version 1.0)

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

using System;
using System.Runtime.InteropServices;

namespace AudioDelay;

public static class Win32Aux
{
	public const uint EXITCODE_SUCCESS = 0U;
	public const uint EXITCODE_ERROR_GENERIC = 0xffffffffU;

	public const uint MB_OK = 0x00000000U;
	public const uint MB_OKCANCEL = 0x00000001U;
	public const uint MB_YESNO = 0x00000004U;
	public const uint MB_YESNOCANCEL = 0x00000003U;
	public const uint MB_RETRYCANCEL = 0x00000005U;
	public const uint MB_ABORTRETRYIGNORE = 0x00000002U;
	public const uint MB_CANCELTRYCONTINUE = 0x00000006U;
	public const uint MB_HELP = 0x00004000U;

	public const uint MB_ICONEXCLAMATION = 0x00000030U;
	public const uint MB_ICONINFORMATION = 0x00000040U;
	public const uint MB_ICONQUESTION = 0x00000020U;
	public const uint MB_ICONSTOP = 0x00000010U;

	public const int IDOK = 1;
	public const int IDCANCEL = 2;
	public const int IDNO = 7;
	public const int IDYES = 6;
	public const int IDABORT = 3;
	public const int IDRETRY = 4;
	public const int IDIGNORE = 5;
	public const int IDTRYAGAIN = 10;
	public const int IDCONTINUE = 11;

	public const uint COINIT_APARTMENTTHREADED = 0x2U;
	public const uint COINIT_MULTITHREADED = 0x0U;
	public const uint COINIT_DISABLE_OLE1DDE = 0x4U;
	public const uint COINIT_SPEED_OVER_MEMORY = 0x8U;

	public const int S_OK = 0x0;
	public const int S_FALSE = 0x1;

	[DllImport("user32.dll", CharSet = CharSet.Unicode)] public static extern Int32 MessageBox(UIntPtr hWnd, UIntPtr lpText, UIntPtr lpCaption, UInt32 uType);

	[DllImport("ole32.dll")] public static extern Int32 CoInitializeEx(UIntPtr pvReserved, UInt32 dwCoInit);
	[DllImport("ole32.dll")] public static extern void CoUninitialize();

	[DllImport("kernel32.dll")] public static extern void Sleep(UInt32 dwMilliseconds);
	[DllImport("kernel32.dll")] public static extern void ExitProcess(UInt32 nExitCode);

	public static unsafe int _MessageBox(string text, string caption, uint type)
	{
		int nRet;

		fixed(char *p_text = text)
		{
			fixed(char *p_caption = caption)
			{
				nRet = MessageBox(0U, (nuint) p_text, (nuint) p_caption, type);
			}
		}

		return nRet;
	}

	public static unsafe int _MessageBox(nuint ptext, string caption, uint type)
	{
		int nRet;

		fixed(char *p_caption = caption)
		{
			nRet = MessageBox(0U, ptext, (nuint) p_caption, type);
		}

		return nRet;
	}
}
