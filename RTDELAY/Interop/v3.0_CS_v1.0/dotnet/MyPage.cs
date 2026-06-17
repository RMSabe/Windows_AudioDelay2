/*
	Real-Time Audio Delay 2 application for Windows
	Version 3.0 (Interop C# version 1.0)

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

public abstract class MyPage : Page
{
	protected MainWindow parent = (MainWindow) Application.Current.MainWindow;

	public virtual void init()
	{
		this.Width = this.parent.Width;
		this.Height = this.parent.Height;
	}

	public virtual void deinit()
	{
	}

	protected abstract void align();

	public void onWindowSizeChanged(object sender, SizeChangedEventArgs e)
	{
		this.Width = this.parent.Width;
		this.Height = this.parent.Height;
		this.align();
	}
}
