/*
	Real-Time Audio Delay 2 application for Windows
	Version 1.3.1 (Interop C# version 1.0)

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

using System;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows;
using System.Windows.Threading;

namespace AudioDelay;

public static class RuntimeHandler
{
	public const int ATTEMPTUPDATEPARAM_DRYINAMP = 1;
	public const int ATTEMPTUPDATEPARAM_OUTAMP = 2;
	public const int ATTEMPTUPDATEPARAM_FFDELAY = 3;
	public const int ATTEMPTUPDATEPARAM_FFAMP = 4;
	public const int ATTEMPTUPDATEPARAM_FBDELAY = 5;
	public const int ATTEMPTUPDATEPARAM_FBAMP = 6;

	public static Thread? mainthread = null;
	public static Thread? audiothread = null;

	public static bool AppInit()
	{
		if(!CPPCore.Init())
		{
			Win32Aux._MessageBox(CPPCore.GetLastErrorMessage(), "INIT ERROR", (Win32Aux.MB_ICONSTOP | Win32Aux.MB_OK));
			AppDeinit();
			return false;
		}

		return true;
	}

	public static void AppDeinit()
	{
		CPPCore.Deinit();
	}

	public static void AppExit(uint exitCode, string exitMsg)
	{
		AppDeinit();

		if(exitMsg.Length > 0) Win32Aux._MessageBox(exitMsg, "PROCESS EXIT CALLED", (Win32Aux.MB_ICONSTOP | Win32Aux.MB_OK));

		Win32Aux.ExitProcess(exitCode);

		while(true) Win32Aux.Sleep(16U);
	}

	public static void ProcStartAudioObject()
	{
		if(!CPPCore.StartAudioObject())
		{
			Win32Aux._MessageBox(CPPCore.GetLastErrorMessage(), "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
			return;
		}

		((MainWindow) Application.Current.MainWindow).GoToChooseAudioDevicePage();
	}

	public static void ProcPrepareRunPlayback()
	{
		if(!CPPCore.PreparePlayback())
		{
			Win32Aux._MessageBox(CPPCore.GetLastErrorMessage(), "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
			return;
		}

		audiothread = new Thread(audiothreadProc);
		audiothread.Start();

		((MainWindow) Application.Current.MainWindow).GoToPlaybackRunningPage();
	}

	public static void ProcOnPlaybackFinished()
	{
		((MainWindow) Application.Current.MainWindow).GoToPlaybackFinishedPage();
	}

	public static bool AttemptUpdateParam(string inputText, int param, int nFX)
	{
		int i32 = 0;
		float f32 = 0.0f;

		switch(param)
		{
			case ATTEMPTUPDATEPARAM_FFDELAY:
			case ATTEMPTUPDATEPARAM_FFAMP:

				if(nFX < 0)
				{
					Win32Aux._MessageBox("Please select a feedforward delay channel.", "No Channel Selected", Win32Aux.MB_OK);
					return false;
				}
				break;

			case ATTEMPTUPDATEPARAM_FBDELAY:
			case ATTEMPTUPDATEPARAM_FBAMP:

				if(nFX < 0)
				{
					Win32Aux._MessageBox("Please select a feedback delay channel.", "No Channel Selected", Win32Aux.MB_OK);
					return false;
				}
				break;
		}

		switch(param)
		{
			case ATTEMPTUPDATEPARAM_DRYINAMP:
			case ATTEMPTUPDATEPARAM_OUTAMP:
			case ATTEMPTUPDATEPARAM_FFAMP:
			case ATTEMPTUPDATEPARAM_FBAMP:

				try
				{
					f32 = Single.Parse(inputText);
				}
				catch(Exception e)
				{
					Win32Aux._MessageBox("Error: invalid amplitude value entered.", "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
					return false;
				}

				break;

			case ATTEMPTUPDATEPARAM_FFDELAY:
			case ATTEMPTUPDATEPARAM_FBDELAY:

				try
				{
					i32 = Int32.Parse(inputText);
				}
				catch(Exception e)
				{
					Win32Aux._MessageBox("Error: invalid delay time value entered.", "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
					return false;
				}

				break;
		}

		switch(param)
		{
			case ATTEMPTUPDATEPARAM_DRYINAMP:
				if(CPPCore.SetDryInputAmplitude(f32)) return true;
				break;

			case ATTEMPTUPDATEPARAM_OUTAMP:
				if(CPPCore.SetOutputAmplitude(f32)) return true;
				break;

			case ATTEMPTUPDATEPARAM_FFDELAY:
				if(CPPCore.SetFFDelay((nuint) nFX, (nuint) i32)) return true;
				break;

			case ATTEMPTUPDATEPARAM_FFAMP:
				if(CPPCore.SetFFAmplitude((nuint) nFX, f32)) return true;
				break;

			case ATTEMPTUPDATEPARAM_FBDELAY:
				if(CPPCore.SetFBDelay((nuint) nFX, (nuint) i32)) return true;
				break;

			case ATTEMPTUPDATEPARAM_FBAMP:
				if(CPPCore.SetFBAmplitude((nuint) nFX, f32)) return true;
				break;

			default:
				return false;
		}

		Win32Aux._MessageBox(CPPCore.GetLastErrorMessage(), "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
		return false;
	}

	public static void audiothreadProc()
	{
		CPPCore.RunPlayback();

		/*
			audiothread may not call UI methods directly, (this will cause a crash)
			Such operation must be done using the Dispatcher object.
			There are 2 relevant methods in this context: Dispatcher.BeginInvoke and Dispatcher.Invoke

			Dispatcher.BeginInvoke() is similar to Win32 PostMessage()
			Dispatcher.Invoke() is similar to Win32 SendMessage()
		*/

		Dispatcher.FromThread(mainthread).BeginInvoke(ProcOnPlaybackFinished, null);
	}
}
