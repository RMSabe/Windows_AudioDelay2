/*
	Real-Time Audio Delay 2 application for Windows
	Version 1.3.1 (Interop C# version 1.0)

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

using Microsoft.Win32;

namespace AudioDelay;

public partial class ChooseFilePage : MyPage
{
	public ChooseFilePage()
	{
		this.InitializeComponent();
		this.init();
	}

	protected override void init()
	{
		base.init();
		this.align();
	}

	protected override void align()
	{
	}

	private void onButton1Clicked(object sender, RoutedEventArgs e)
	{
		this.procOpenFileDialog();
	}

	private void procOpenFileDialog()
	{
		OpenFileDialog ofdlg = new OpenFileDialog();
		bool? bRet;

		ofdlg.Filter = "Wave Files|*.wav|All Files|*.*";
		ofdlg.DefaultExt = ".wav";
		ofdlg.FilterIndex = 1;

		bRet = ofdlg.ShowDialog();

		if(bRet != null) if(bRet == false) return;

		CPPCore.SetFileInDir(ofdlg.FileName);
		RuntimeHandler.ProcStartAudioObject();
	}
}
