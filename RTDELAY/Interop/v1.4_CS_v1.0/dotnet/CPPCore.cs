/*
	Real-Time Audio Delay 2 application for Windows
	Version 1.4 (Interop C# version 1.0)

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

	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern Int32 Initialize();
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern void Deinitialize();
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern void Reset();
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern UIntPtr GetLastErrorMessage();
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern Int32 SetFileInDirectory(UIntPtr fdir);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern Int32 LoadFile_CreateAudioObject();
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern Int32 LoadAudioDeviceList();
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern IntPtr GetAudioDeviceListEntryCount();
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern UIntPtr GetAudioDeviceListEntry(UIntPtr n_entry);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern Int32 ChooseDevice(UIntPtr index);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern Int32 ChooseDefaultDevice();
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern Int32 InitializeAudioObject();
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern Int32 RunPlayback();
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern void StopPlayback();
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern Single GetDryInputAmplitude();
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern Int32 SetDryInputAmplitude(Single amp);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern Single GetOutputAmplitude();
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern Int32 SetOutputAmplitude(Single amp);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern IntPtr GetFFDelay(UIntPtr nfx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern Int32 SetFFDelay(UIntPtr nfx, UIntPtr delay);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern Single GetFFAmplitude(UIntPtr nfx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern Int32 SetFFAmplitude(UIntPtr nfx, Single amp);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern IntPtr GetFBDelay(UIntPtr nfx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern Int32 SetFBDelay(UIntPtr nfx, UIntPtr delay);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern Single GetFBAmplitude(UIntPtr nfx);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern Int32 SetFBAmplitude(UIntPtr nfx, Single amp);
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern Int32 ResetFFParams();
	[DllImport(LIBCORE_DIR, CallingConvention = CallingConvention.StdCall)] public static extern Int32 ResetFBParams();
}
