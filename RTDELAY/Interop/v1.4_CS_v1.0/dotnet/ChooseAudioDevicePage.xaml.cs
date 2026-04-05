/*
	Real-Time Audio Delay 2 application for Windows
	Version 1.4 (Interop C# version 1.0)

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

using System.Runtime.InteropServices;

namespace AudioDelay;

public partial class ChooseAudioDevicePage : MyPage
{
	public ChooseAudioDevicePage()
	{
		this.InitializeComponent();
		this.init();
	}

	protected override void init()
	{
		nuint pEntry;
		nint nEntryCount;
		nint nEntry;

		base.init();

		if(CPPCore.LoadAudioDeviceList() == 0)
		{
			Win32Aux._MessageBox(CPPCore.GetLastErrorMessage(), "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
			return;
		}

		nEntryCount = CPPCore.GetAudioDeviceListEntryCount();
		if(nEntryCount < 0)
		{
			Win32Aux._MessageBox(CPPCore.GetLastErrorMessage(), "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
			return;
		}

		nEntry = 0;
		while(nEntry < nEntryCount)
		{
			pEntry = CPPCore.GetAudioDeviceListEntry((nuint) nEntry);

			if(pEntry == 0U)
			{
				Win32Aux._MessageBox(CPPCore.GetLastErrorMessage(), "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
				return;
			}

			this.ListBox1.Items.Add(Marshal.PtrToStringUni((nint) pEntry));

			nEntry++;
		}

		this.align();
	}

	protected override void align()
	{
		this.ListBox1.Width = this.Width - 2.0*(this.ListBox1.Margin.Left);
		this.ListBox1.Height = ((double) this.ListBox1.Items.Count)*20.0;
	}

	private void onButton1Clicked(object sender, RoutedEventArgs e)
	{
		CPPCore.Reset();
		this.parent.GoToChooseFilePage();
	}

	private void onButton2Clicked(object sender, RoutedEventArgs e)
	{
		int nSel = this.ListBox1.SelectedIndex;

		if(nSel < 0)
		{
			Win32Aux._MessageBox("Error: No Device Selected", "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
			return;
		}

		if(CPPCore.ChooseDevice((nuint) nSel) == 0)
		{
			Win32Aux._MessageBox(CPPCore.GetLastErrorMessage(), "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
			return;
		}

		RuntimeHandler.Proc_InitAudioObj_RunPlayback();
	}

	private void onButton3Clicked(object sender, RoutedEventArgs e)
	{
		if(CPPCore.ChooseDefaultDevice() == 0)
		{
			Win32Aux._MessageBox(CPPCore.GetLastErrorMessage(), "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
			return;
		}

		RuntimeHandler.Proc_InitAudioObj_RunPlayback();
	}
}
