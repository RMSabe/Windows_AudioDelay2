/*
	Real-Time Audio Delay 2 application for Windows
	Version 1.3.1 (Interop C# version 1.0)

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

using System;
using System.Runtime.InteropServices;

namespace AudioDelay;

public static class CPPCore
{
#if __X64__
	public const string LIBCORE_DIR = "core64.dll";
#elif __I386__
	public const string LIBCORE_DIR = "core32.dll";
#endif

	public const uint RTDELAY_N_FFCH = 4U;
	public const uint RTDELAY_N_FBCH = 4U;

	private static UIntPtr pProcCtx = 0U;
	private static string errMsg = "";

	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern UIntPtr _procCtxAlloc();
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern Int32 _procCtxFree(UIntPtr p_procctx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern Int32 _coreInit(UIntPtr p_procctx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern void _coreDeinit(UIntPtr p_procctx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern void _coreReset(UIntPtr p_procctx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern UIntPtr _getLastErrorMessage(UIntPtr p_procctx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern Int32 _setFileInDir(UIntPtr p_procctx, UIntPtr fdir);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern Int32 _preparePlayback(UIntPtr p_procctx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern Int32 _runPlayback(UIntPtr p_procctx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern void _stopPlayback(UIntPtr p_procctx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern Int32 _startAudioObject(UIntPtr p_procctx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern Int32 _loadAudioDeviceList(UIntPtr p_procctx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern IntPtr _getAudioDeviceListEntryCount(UIntPtr p_procctx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern UIntPtr _getAudioDeviceListEntry(UIntPtr p_procctx, UIntPtr n_entry);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern Int32 _chooseDevice(UIntPtr p_procctx, UIntPtr index);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern Int32 _chooseDefaultDevice(UIntPtr p_procctx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern Single _getDryInputAmplitude(UIntPtr p_procctx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern Int32 _setDryInputAmplitude(UIntPtr p_procctx, Single amp);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern Single _getOutputAmplitude(UIntPtr p_procctx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern Int32 _setOutputAmplitude(UIntPtr p_procctx, Single amp);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern IntPtr _getFFDelay(UIntPtr p_procctx, UIntPtr nfx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern Int32 _setFFDelay(UIntPtr p_procctx, UIntPtr nfx, UIntPtr delay);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern Single _getFFAmplitude(UIntPtr p_procctx, UIntPtr nfx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern Int32 _setFFAmplitude(UIntPtr p_procctx, UIntPtr nfx, Single amp);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern IntPtr _getFBDelay(UIntPtr p_procctx, UIntPtr nfx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern Int32 _setFBDelay(UIntPtr p_procctx, UIntPtr nfx, UIntPtr delay);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern Single _getFBAmplitude(UIntPtr p_procctx, UIntPtr nfx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern Int32 _setFBAmplitude(UIntPtr p_procctx, UIntPtr nfx, Single amp);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern Int32 _resetFFParams(UIntPtr p_procctx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] private static extern Int32 _resetFBParams(UIntPtr p_procctx);

	private static bool validateProcCtx()
	{
		if(pProcCtx == 0U)
		{
			errMsg = "CPPCore: Error: resource not initialized.";
			return false;
		}

		return true;
	}

	public static bool Init()
	{
		pProcCtx = _procCtxAlloc();
		if(pProcCtx == 0U)
		{
			errMsg = "CPPCore.Init: Error: failed to allocate process context object.";
			return false;
		}

		if(_coreInit(pProcCtx) == 0)
		{
			errMsg = Marshal.PtrToStringUni((nint) _getLastErrorMessage(pProcCtx));
			Deinit();
			return false;
		}

		return true;
	}

	public static void Deinit()
	{
		_coreDeinit(pProcCtx);
		_procCtxFree(pProcCtx);
		pProcCtx = 0U;
	}

	public static void Reset()
	{
		_coreReset(pProcCtx);
	}

	public static string GetLastErrorMessage()
	{
		return errMsg;
	}

	public static unsafe bool SetFileInDir(string fileDir)
	{
		int nRet;

		if(!validateProcCtx()) return false;

		fixed(char *p_fdir = fileDir)
		{
			nRet = _setFileInDir(pProcCtx, (UIntPtr) p_fdir);
		}

		if(nRet == 0)
		{
			errMsg = Marshal.PtrToStringUni((nint) _getLastErrorMessage(pProcCtx));
			return false;
		}

		return true;
	}

	public static bool PreparePlayback()
	{
		if(!validateProcCtx()) return false;

		if(_preparePlayback(pProcCtx) == 0)
		{
			errMsg = Marshal.PtrToStringUni((nint) _getLastErrorMessage(pProcCtx));
			return false;
		}

		return true;
	}

	public static bool RunPlayback()
	{
		if(!validateProcCtx()) return false;

		if(_runPlayback(pProcCtx) == 0)
		{
			errMsg = Marshal.PtrToStringUni((nint) _getLastErrorMessage(pProcCtx));
			return false;
		}

		return true;
	}

	public static void StopPlayback()
	{
		_stopPlayback(pProcCtx);
	}

	public static bool StartAudioObject()
	{
		if(!validateProcCtx()) return false;

		if(_startAudioObject(pProcCtx) == 0)
		{
			errMsg = Marshal.PtrToStringUni((nint) _getLastErrorMessage(pProcCtx));
			return false;
		}

		return true;
	}

	public static bool LoadAudioDeviceList()
	{
		if(!validateProcCtx()) return false;

		if(_loadAudioDeviceList(pProcCtx) == 0)
		{
			errMsg = Marshal.PtrToStringUni((nint) _getLastErrorMessage(pProcCtx));
			return false;
		}

		return true;
	}

	public static nint GetAudioDeviceListEntryCount()
	{
		nint count;

		if(!validateProcCtx()) return -1;

		count = _getAudioDeviceListEntryCount(pProcCtx);

		if(count < 0) errMsg = Marshal.PtrToStringUni((nint) _getLastErrorMessage(pProcCtx));

		return count;
	}

	public static nuint GetAudioDeviceListEntry(nuint nEntry)
	{
		nuint pStrEntry;

		if(!validateProcCtx()) return 0U;

		pStrEntry = _getAudioDeviceListEntry(pProcCtx, nEntry);

		if(pStrEntry == 0U) errMsg = Marshal.PtrToStringUni((nint) _getLastErrorMessage(pProcCtx));

		return pStrEntry;
	}

	public static bool ChooseDevice(nuint index)
	{
		if(!validateProcCtx()) return false;

		if(_chooseDevice(pProcCtx, index) == 0)
		{
			errMsg = Marshal.PtrToStringUni((nint) _getLastErrorMessage(pProcCtx));
			return false;
		}

		return true;
	}

	public static bool ChooseDefaultDevice()
	{
		if(!validateProcCtx()) return false;

		if(_chooseDefaultDevice(pProcCtx) == 0)
		{
			errMsg = Marshal.PtrToStringUni((nint) _getLastErrorMessage(pProcCtx));
			return false;
		}

		return true;
	}

	public static float GetDryInputAmplitude()
	{
		return _getDryInputAmplitude(pProcCtx);
	}

	public static bool SetDryInputAmplitude(float amp)
	{
		if(!validateProcCtx()) return false;

		if(_setDryInputAmplitude(pProcCtx, amp) == 0)
		{
			errMsg = Marshal.PtrToStringUni((nint) _getLastErrorMessage(pProcCtx));
			return false;
		}

		return true;
	}

	public static float GetOutputAmplitude()
	{
		return _getOutputAmplitude(pProcCtx);
	}

	public static bool SetOutputAmplitude(float amp)
	{
		if(!validateProcCtx()) return false;

		if(_setOutputAmplitude(pProcCtx, amp) == 0)
		{
			errMsg = Marshal.PtrToStringUni((nint) _getLastErrorMessage(pProcCtx));
			return false;
		}

		return true;
	}

	public static nint GetFFDelay(nuint nFX)
	{
		nint delay;

		if(!validateProcCtx()) return -1;

		delay = _getFFDelay(pProcCtx, nFX);

		if(delay < 0) errMsg = Marshal.PtrToStringUni((nint) _getLastErrorMessage(pProcCtx));

		return delay;
	}

	public static bool SetFFDelay(nuint nFX, nuint delay)
	{
		if(!validateProcCtx()) return false;

		if(_setFFDelay(pProcCtx, nFX, delay) == 0)
		{
			errMsg = Marshal.PtrToStringUni((nint) _getLastErrorMessage(pProcCtx));
			return false;
		}

		return true;
	}

	public static float GetFFAmplitude(nuint nFX)
	{
		return _getFFAmplitude(pProcCtx, nFX);
	}

	public static bool SetFFAmplitude(nuint nFX, float amp)
	{
		if(!validateProcCtx()) return false;

		if(_setFFAmplitude(pProcCtx, nFX, amp) == 0)
		{
			errMsg = Marshal.PtrToStringUni((nint) _getLastErrorMessage(pProcCtx));
			return false;
		}

		return true;
	}

	public static nint GetFBDelay(nuint nFX)
	{
		nint delay;

		if(!validateProcCtx()) return -1;

		delay = _getFBDelay(pProcCtx, nFX);

		if(delay < 0) errMsg = Marshal.PtrToStringUni((nint) _getLastErrorMessage(pProcCtx));

		return delay;
	}

	public static bool SetFBDelay(nuint nFX, nuint delay)
	{
		if(!validateProcCtx()) return false;

		if(_setFBDelay(pProcCtx, nFX, delay) == 0)
		{
			errMsg = Marshal.PtrToStringUni((nint) _getLastErrorMessage(pProcCtx));
			return false;
		}

		return true;
	}

	public static float GetFBAmplitude(nuint nFX)
	{
		return _getFBAmplitude(pProcCtx, nFX);
	}

	public static bool SetFBAmplitude(nuint nFX, float amp)
	{
		if(!validateProcCtx()) return false;

		if(_setFBAmplitude(pProcCtx, nFX, amp) == 0)
		{
			errMsg = Marshal.PtrToStringUni((nint) _getLastErrorMessage(pProcCtx));
			return false;
		}

		return true;
	}

	public static bool ResetFFParams()
	{
		if(!validateProcCtx()) return false;

		if(_resetFFParams(pProcCtx) == 0)
		{
			errMsg = Marshal.PtrToStringUni((nint) _getLastErrorMessage(pProcCtx));
			return false;
		}

		return true;
	}

	public static bool ResetFBParams()
	{
		if(!validateProcCtx()) return false;

		if(_resetFBParams(pProcCtx) == 0)
		{
			errMsg = Marshal.PtrToStringUni((nint) _getLastErrorMessage(pProcCtx));
			return false;
		}

		return true;
	}
}
