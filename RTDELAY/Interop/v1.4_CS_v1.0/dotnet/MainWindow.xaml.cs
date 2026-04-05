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

using System.Threading;
using System.Windows.Threading;

namespace AudioDelay;

public partial class MainWindow : Window
{
	public MainWindow()
	{
		if(!RuntimeHandler.AppInit()) Win32Aux.ExitProcess(Win32Aux.EXITCODE_ERROR_GENERIC);

		this.InitializeComponent();
		RuntimeHandler.mainthread = Thread.CurrentThread;

		this.GoToChooseFilePage();
	}

	public void GoToChooseFilePage()
	{
		this.Content = new ChooseFilePage();
	}

	public void GoToChooseAudioDevicePage()
	{
		this.Content = new ChooseAudioDevicePage();
	}

	public void GoToPlaybackRunningPage()
	{
		this.Content = new PlaybackRunningPage();
	}

	public void GoToPlaybackFinishedPage()
	{
		this.Content = new PlaybackFinishedPage();
	}

	private void onWindowClosed(object sender, EventArgs e)
	{
		RuntimeHandler.AppDeinit();
	}

	private void onWindowSizeChanged(object sender, SizeChangedEventArgs e)
	{
		if(this.Content != null) ((MyPage) this.Content).onWindowSizeChanged(sender, e);
	}
}
