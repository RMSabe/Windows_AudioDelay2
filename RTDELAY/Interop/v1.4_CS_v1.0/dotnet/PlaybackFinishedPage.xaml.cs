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

namespace AudioDelay;

public partial class PlaybackFinishedPage : MyPage
{
	public PlaybackFinishedPage()
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
		this.parent.GoToChooseFilePage();
	}
}
