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

namespace AudioDelay;

public partial class PlaybackRunningPage : MyPage
{
	private RadioButton[]? ffselRadioButtonArray = null;
	private RadioButton[]? fbselRadioButtonArray = null;

	public PlaybackRunningPage()
	{
		this.InitializeComponent();
		this.init();
	}

	protected override void init()
	{
		uint nRadioButton;

		base.init();

		if(CPPCore.RTDELAY_N_FFCH != 0U)
		{
			this.ffselRadioButtonArray = new RadioButton[CPPCore.RTDELAY_N_FFCH];

			for(nRadioButton = 0U; nRadioButton < CPPCore.RTDELAY_N_FFCH; nRadioButton++)
			{
				this.ffselRadioButtonArray[nRadioButton] = new RadioButton();
				this.ffselRadioButtonArray[nRadioButton].Content = "FF Channel " + (nRadioButton + 1U).ToString();
				this.Container3_Subcontainer1.Children.Add(this.ffselRadioButtonArray[nRadioButton]);
			}
		}

		if(CPPCore.RTDELAY_N_FBCH != 0U)
		{
			this.fbselRadioButtonArray = new RadioButton[CPPCore.RTDELAY_N_FBCH];

			for(nRadioButton = 0U; nRadioButton < CPPCore.RTDELAY_N_FBCH; nRadioButton++)
			{
				this.fbselRadioButtonArray[nRadioButton] = new RadioButton();
				this.fbselRadioButtonArray[nRadioButton].Content = "FB Channel " + (nRadioButton + 1U).ToString();
				this.Container4_Subcontainer1.Children.Add(this.fbselRadioButtonArray[nRadioButton]);
			}
		}
	}

	protected override void align()
	{
		const double INTERCOMPONENT_MARGIN = 10.0;

		const double TEXTBLOCK1_FONTSIZE = 35.0;
		const double TEXTBLOCK1_FONTSIZEMARGIN = 10.0;

		const double TEXTBLOCK2_FONTSIZE = 20.0;
		const double TEXTBLOCK2_FONTSIZEMARGIN = 6.0;
		const double TEXTBLOCK2_NLINES = 2.0;

		const double BUTTON_WIDTH = 160.0;
		const double BUTTON_HEIGHT = 20.0;

		const double PAGE_MARGIN_TOP = 20.0;
		const double PAGE_MARGIN_BOTTOM = 60.0;
		const double PAGE_MARGIN_LEFTRIGHT = 50.0;

		const double CONTAINER1_HEIGHT = 140.0;
		const double CONTAINER2_HEIGHT = 100.0;
		const double CONTAINER3_HEIGHT = 180.0;
		const double CONTAINER4_HEIGHT = 180.0;

		const double CONTAINERTEXT_FONTSIZE = 12.0;
		const double CONTAINERTEXT_FONTSIZEMARGIN = 4.0;

		const double TEXTBLOCK_PARAMSIN_HEIGHT = (CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN)*(((double) CPPCore.RTDELAY_N_FFCH) + 2.0);
		const double TEXTBLOCK_PARAMSOUT_HEIGHT = (CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN)*(((double) CPPCore.RTDELAY_N_FBCH) + 2.0);

		const double RADIOBUTTON_SUBCONTAINER_WIDTH = 150.0;

		double marginLeft;
		double marginTop;
		double width;
		double height;
		double parentCenterX;
		double parentCenterY;

		int nRadioButton;

		parentCenterX = this.Width/2.0;
		parentCenterY = this.Height/2.0;

		marginLeft = PAGE_MARGIN_LEFTRIGHT;
		marginTop = PAGE_MARGIN_TOP;
		width = this.Width - 2.0*marginLeft;
		height = TEXTBLOCK1_FONTSIZE + TEXTBLOCK1_FONTSIZEMARGIN;

		this.TextBlock1.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.TextBlock1.Width = width;
		this.TextBlock1.Height = height;
		this.TextBlock1.FontSize = TEXTBLOCK1_FONTSIZE;

		marginLeft = PAGE_MARGIN_LEFTRIGHT;
		marginTop = this.TextBlock1.Margin.Top + this.TextBlock1.Height + INTERCOMPONENT_MARGIN;
		width = this.TextBlock1.Width;
		height = (TEXTBLOCK2_FONTSIZE + TEXTBLOCK2_FONTSIZEMARGIN)*TEXTBLOCK2_NLINES;

		this.TextBlock2.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.TextBlock2.Width = width;
		this.TextBlock2.Height = height;
		this.TextBlock2.FontSize = TEXTBLOCK2_FONTSIZE;

		width = BUTTON_WIDTH;
		height = BUTTON_HEIGHT;
		marginLeft = parentCenterX - width/2.0;
		marginTop = this.Height - BUTTON_HEIGHT - PAGE_MARGIN_BOTTOM;

		this.Button_StopPlayback.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.Button_StopPlayback.Width = width;
		this.Button_StopPlayback.Height = height;

		marginLeft = PAGE_MARGIN_LEFTRIGHT;
		marginTop = this.TextBlock2.Margin.Top + this.TextBlock2.Height + INTERCOMPONENT_MARGIN;
		width = this.TextBlock2.Width;
		height = CONTAINER1_HEIGHT;

		this.Container1.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.Container1.Width = width;
		this.Container1.Height = height;

		marginLeft = PAGE_MARGIN_LEFTRIGHT;
		marginTop = this.Container1.Margin.Top + this.Container1.Height + INTERCOMPONENT_MARGIN;
		width = this.Container1.Width;
		height = CONTAINER2_HEIGHT;

		this.Container2.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.Container2.Width = width;
		this.Container2.Height = height;

		marginLeft = PAGE_MARGIN_LEFTRIGHT;
		marginTop = this.Container2.Margin.Top + this.Container2.Height + INTERCOMPONENT_MARGIN;
		width = this.Container2.Width;
		height = CONTAINER3_HEIGHT;

		this.Container3.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.Container3.Width = width;
		this.Container3.Height = height;

		marginLeft = PAGE_MARGIN_LEFTRIGHT;
		marginTop = this.Container3.Margin.Top + this.Container3.Height + INTERCOMPONENT_MARGIN;
		width = this.Container3.Width;
		height = CONTAINER4_HEIGHT;

		this.Container4.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.Container4.Width = width;
		this.Container4.Height = height;

		width = BUTTON_WIDTH;
		height = BUTTON_HEIGHT;
		marginLeft = parentCenterX - width/2.0;
		marginTop = this.Container4.Margin.Top + this.Container4.Height + INTERCOMPONENT_MARGIN;

		this.Button_ResetAllParams.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.Button_ResetAllParams.Width = width;
		this.Button_ResetAllParams.Height = height;

		/*SETUP CONTAINER 1*/

		parentCenterX = this.Container1.Width/2.0;
		parentCenterY = this.Container1.Height/2.0;

		marginLeft = INTERCOMPONENT_MARGIN;
		marginTop = INTERCOMPONENT_MARGIN;
		width = this.Container1.Width - 2.0*marginLeft;
		height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		this.TextBlock_Params.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.TextBlock_Params.Width = width;
		this.TextBlock_Params.Height = height;
		this.TextBlock_Params.FontSize = CONTAINERTEXT_FONTSIZE;

		width = parentCenterX - 2.0*INTERCOMPONENT_MARGIN;
		height = TEXTBLOCK_PARAMSIN_HEIGHT;
		marginLeft = INTERCOMPONENT_MARGIN;
		marginTop = this.Container1.Height - height - INTERCOMPONENT_MARGIN;

		this.TextBlock_ParamsIn.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.TextBlock_ParamsIn.Width = width;
		this.TextBlock_ParamsIn.Height = height;
		this.TextBlock_ParamsIn.FontSize = CONTAINERTEXT_FONTSIZE;

		width = this.TextBlock_ParamsIn.Width;
		height = TEXTBLOCK_PARAMSOUT_HEIGHT;
		marginLeft = parentCenterX + INTERCOMPONENT_MARGIN;
		marginTop = this.Container1.Height - height - INTERCOMPONENT_MARGIN;

		this.TextBlock_ParamsOut.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.TextBlock_ParamsOut.Width = width;
		this.TextBlock_ParamsOut.Height = height;
		this.TextBlock_ParamsOut.FontSize = CONTAINERTEXT_FONTSIZE;

		/*SETUP CONTAINER 2*/

		parentCenterX = this.Container2.Width/2.0;
		parentCenterY = this.Container2.Height/2.0;

		marginLeft = 0.0;
		marginTop = 0.0;
		width = parentCenterX;
		height = this.Container2.Height;

		this.Container2_Subcontainer1.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.Container2_Subcontainer1.Width = width;
		this.Container2_Subcontainer1.Height = height;

		marginLeft = parentCenterX;
		marginTop = 0.0;
		width = parentCenterX;
		height = this.Container2.Height;

		this.Container2_Subcontainer2.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.Container2_Subcontainer2.Width = width;
		this.Container2_Subcontainer2.Height = height;

		/*SETUP CONTAINER 2 SUBCONTAINER 1*/

		marginLeft = INTERCOMPONENT_MARGIN;
		marginTop = INTERCOMPONENT_MARGIN;
		width = this.Container2_Subcontainer1.Width - 2.0*marginLeft;
		height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;
		
		this.TextBlock_UpdateDryInAmp.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.TextBlock_UpdateDryInAmp.Width = width;
		this.TextBlock_UpdateDryInAmp.Height = height;
		this.TextBlock_UpdateDryInAmp.FontSize = CONTAINERTEXT_FONTSIZE;

		marginTop = this.TextBlock_UpdateDryInAmp.Margin.Top + this.TextBlock_UpdateDryInAmp.Height + INTERCOMPONENT_MARGIN;

		this.TextBox_UpdateDryInAmp.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.TextBox_UpdateDryInAmp.Width = width;
		this.TextBox_UpdateDryInAmp.Height = height;
		this.TextBox_UpdateDryInAmp.FontSize = CONTAINERTEXT_FONTSIZE;

		marginTop = this.TextBox_UpdateDryInAmp.Margin.Top + this.TextBox_UpdateDryInAmp.Height + INTERCOMPONENT_MARGIN;
		height = BUTTON_HEIGHT;

		this.Button_UpdateDryInAmp.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.Button_UpdateDryInAmp.Width = width;
		this.Button_UpdateDryInAmp.Height = height;

		/*SETUP CONTAINER 2 SUBCONTAINER 2*/

		marginLeft = INTERCOMPONENT_MARGIN;
		marginTop = INTERCOMPONENT_MARGIN;
		width = this.Container2_Subcontainer2.Width - 2.0*marginLeft;
		height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		this.TextBlock_UpdateOutAmp.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.TextBlock_UpdateOutAmp.Width = width;
		this.TextBlock_UpdateOutAmp.Height = height;
		this.TextBlock_UpdateOutAmp.FontSize = CONTAINERTEXT_FONTSIZE;

		marginTop = this.TextBlock_UpdateOutAmp.Margin.Top + this.TextBlock_UpdateOutAmp.Height + INTERCOMPONENT_MARGIN;

		this.TextBox_UpdateOutAmp.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.TextBox_UpdateOutAmp.Width = width;
		this.TextBox_UpdateOutAmp.Height = height;
		this.TextBox_UpdateOutAmp.FontSize = CONTAINERTEXT_FONTSIZE;

		marginTop = this.TextBox_UpdateOutAmp.Margin.Top + this.TextBox_UpdateOutAmp.Height + INTERCOMPONENT_MARGIN;
		height = BUTTON_HEIGHT;

		this.Button_UpdateOutAmp.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.Button_UpdateOutAmp.Width = width;
		this.Button_UpdateOutAmp.Height = height;

		/*SETUP CONTAINER 3*/

		parentCenterX = this.Container3.Width/2.0;
		parentCenterY = this.Container3.Height/2.0;

		marginLeft = INTERCOMPONENT_MARGIN;
		marginTop = INTERCOMPONENT_MARGIN;
		width = this.Container3.Width - 2.0*marginLeft;
		height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		this.TextBlock_FF.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.TextBlock_FF.Width = width;
		this.TextBlock_FF.Height = height;
		this.TextBlock_FF.FontSize = CONTAINERTEXT_FONTSIZE;

		width = BUTTON_WIDTH;
		height = BUTTON_HEIGHT;
		marginLeft = parentCenterX - BUTTON_WIDTH/2.0;
		marginTop = this.Container3.Height - height - INTERCOMPONENT_MARGIN;

		this.Button_FFReset.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.Button_FFReset.Width = width;
		this.Button_FFReset.Height = height;

		marginLeft = 0.0;
		marginTop = this.TextBlock_FF.Margin.Top + this.TextBlock_FF.Height;
		width = RADIOBUTTON_SUBCONTAINER_WIDTH;
		height = this.Button_FFReset.Margin.Top - marginTop;

		this.Container3_Subcontainer1.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.Container3_Subcontainer1.Width = width;
		this.Container3_Subcontainer1.Height = height;

		width = (this.Container3.Width - this.Container3_Subcontainer1.Width)/2.0;
		marginLeft = this.Container3.Width - width;

		this.Container3_Subcontainer3.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.Container3_Subcontainer3.Width = width;
		this.Container3_Subcontainer3.Height = height;

		marginLeft = this.Container3_Subcontainer3.Margin.Left - width;

		this.Container3_Subcontainer2.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.Container3_Subcontainer2.Width = width;
		this.Container3_Subcontainer2.Height = height;

		/*SETUP CONTAINER 3 SUBCONTAINER 1*/

		parentCenterX = this.Container3_Subcontainer1.Width/2.0;
		parentCenterY = this.Container3_Subcontainer1.Height/2.0;

		marginLeft = INTERCOMPONENT_MARGIN;
		marginTop = INTERCOMPONENT_MARGIN;
		width = this.Container3_Subcontainer1.Width - 2.0*marginLeft;
		height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		this.TextBlock_FFChannelSel.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.TextBlock_FFChannelSel.Width = width;
		this.TextBlock_FFChannelSel.Height = height;
		this.TextBlock_FFChannelSel.FontSize = CONTAINERTEXT_FONTSIZE;

		if(CPPCore.RTDELAY_N_FFCH != 0U)
		{
			nRadioButton = ((int) CPPCore.RTDELAY_N_FFCH) - 1;

			height = BUTTON_HEIGHT;
			marginTop = this.Container3_Subcontainer1.Height - height - INTERCOMPONENT_MARGIN;

			this.ffselRadioButtonArray[nRadioButton].Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
			this.ffselRadioButtonArray[nRadioButton].Width = width;
			this.ffselRadioButtonArray[nRadioButton].Height = height;

			nRadioButton--;

			while(nRadioButton >= 0)
			{
				marginTop = this.ffselRadioButtonArray[nRadioButton + 1].Margin.Top - height;

				this.ffselRadioButtonArray[nRadioButton].Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
				this.ffselRadioButtonArray[nRadioButton].Width = width;
				this.ffselRadioButtonArray[nRadioButton].Height = height;

				nRadioButton--;
			}
		}

		/*SETUP CONTAINER 3 SUBCONTAINER 2*/

		marginLeft = INTERCOMPONENT_MARGIN;
		marginTop = INTERCOMPONENT_MARGIN;
		width = this.Container3_Subcontainer2.Width - 2.0*marginLeft;
		height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		this.TextBlock_FFUpdateDelay.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.TextBlock_FFUpdateDelay.Width = width;
		this.TextBlock_FFUpdateDelay.Height = height;
		this.TextBlock_FFUpdateDelay.FontSize = CONTAINERTEXT_FONTSIZE;

		marginTop = this.TextBlock_FFUpdateDelay.Margin.Top + this.TextBlock_FFUpdateDelay.Height + INTERCOMPONENT_MARGIN;

		this.TextBox_FFUpdateDelay.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.TextBox_FFUpdateDelay.Width = width;
		this.TextBox_FFUpdateDelay.Height = height;
		this.TextBox_FFUpdateDelay.FontSize = CONTAINERTEXT_FONTSIZE;

		marginTop = this.TextBox_FFUpdateDelay.Margin.Top + this.TextBox_FFUpdateDelay.Height + INTERCOMPONENT_MARGIN;
		height = BUTTON_HEIGHT;

		this.Button_FFUpdateDelay.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.Button_FFUpdateDelay.Width = width;
		this.Button_FFUpdateDelay.Height = height;

		/*SETUP CONTAINER 3 SUBCONTAINER 3*/

		marginLeft = INTERCOMPONENT_MARGIN;
		marginTop = INTERCOMPONENT_MARGIN;
		width = this.Container3_Subcontainer3.Width - 2.0*marginLeft;
		height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		this.TextBlock_FFUpdateAmp.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.TextBlock_FFUpdateAmp.Width = width;
		this.TextBlock_FFUpdateAmp.Height = height;
		this.TextBlock_FFUpdateAmp.FontSize = CONTAINERTEXT_FONTSIZE;

		marginTop = this.TextBlock_FFUpdateAmp.Margin.Top + this.TextBlock_FFUpdateAmp.Height + INTERCOMPONENT_MARGIN;

		this.TextBox_FFUpdateAmp.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.TextBox_FFUpdateAmp.Width = width;
		this.TextBox_FFUpdateAmp.Height = height;
		this.TextBox_FFUpdateAmp.FontSize = CONTAINERTEXT_FONTSIZE;

		marginTop = this.TextBox_FFUpdateAmp.Margin.Top + this.TextBox_FFUpdateAmp.Height + INTERCOMPONENT_MARGIN;
		height = BUTTON_HEIGHT;

		this.Button_FFUpdateAmp.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.Button_FFUpdateAmp.Width = width;
		this.Button_FFUpdateAmp.Height = height;

		/*SETUP CONTAINER 4*/

		parentCenterX = this.Container4.Width/2.0;
		parentCenterY = this.Container4.Height/2.0;

		marginLeft = INTERCOMPONENT_MARGIN;
		marginTop = INTERCOMPONENT_MARGIN;
		width = this.Container4.Width - 2.0*marginLeft;
		height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		this.TextBlock_FB.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.TextBlock_FB.Width = width;
		this.TextBlock_FB.Height = height;
		this.TextBlock_FB.FontSize = CONTAINERTEXT_FONTSIZE;

		width = BUTTON_WIDTH;
		height = BUTTON_HEIGHT;
		marginLeft = parentCenterX - BUTTON_WIDTH/2.0;
		marginTop = this.Container4.Height - height - INTERCOMPONENT_MARGIN;

		this.Button_FBReset.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.Button_FBReset.Width = width;
		this.Button_FBReset.Height = height;

		marginLeft = 0.0;
		marginTop = this.TextBlock_FB.Margin.Top + this.TextBlock_FB.Height;
		width = RADIOBUTTON_SUBCONTAINER_WIDTH;
		height = this.Button_FBReset.Margin.Top - marginTop;

		this.Container4_Subcontainer1.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.Container4_Subcontainer1.Width = width;
		this.Container4_Subcontainer1.Height = height;

		width = (this.Container4.Width - this.Container4_Subcontainer1.Width)/2.0;
		marginLeft = this.Container4.Width - width;

		this.Container4_Subcontainer3.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.Container4_Subcontainer3.Width = width;
		this.Container4_Subcontainer3.Height = height;

		marginLeft = this.Container4_Subcontainer3.Margin.Left - width;

		this.Container4_Subcontainer2.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.Container4_Subcontainer2.Width = width;
		this.Container4_Subcontainer2.Height = height;

		/*SETUP CONTAINER 4 SUBCONTAINER 1*/

		parentCenterX = this.Container4_Subcontainer1.Width/2.0;
		parentCenterY = this.Container4_Subcontainer1.Height/2.0;

		marginLeft = INTERCOMPONENT_MARGIN;
		marginTop = INTERCOMPONENT_MARGIN;
		width = this.Container4_Subcontainer1.Width - 2.0*marginLeft;
		height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		this.TextBlock_FBChannelSel.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.TextBlock_FBChannelSel.Width = width;
		this.TextBlock_FBChannelSel.Height = height;
		this.TextBlock_FBChannelSel.FontSize = CONTAINERTEXT_FONTSIZE;

		if(CPPCore.RTDELAY_N_FBCH != 0U)
		{
			nRadioButton = ((int) CPPCore.RTDELAY_N_FBCH) - 1;

			height = BUTTON_HEIGHT;
			marginTop = this.Container4_Subcontainer1.Height - height - INTERCOMPONENT_MARGIN;

			this.fbselRadioButtonArray[nRadioButton].Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
			this.fbselRadioButtonArray[nRadioButton].Width = width;
			this.fbselRadioButtonArray[nRadioButton].Height = height;

			nRadioButton--;

			while(nRadioButton >= 0)
			{
				marginTop = this.fbselRadioButtonArray[nRadioButton + 1].Margin.Top - height;

				this.fbselRadioButtonArray[nRadioButton].Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
				this.fbselRadioButtonArray[nRadioButton].Width = width;
				this.fbselRadioButtonArray[nRadioButton].Height = height;

				nRadioButton--;
			}
		}

		/*SETUP CONTAINER 4 SUBCONTAINER 2*/

		marginLeft = INTERCOMPONENT_MARGIN;
		marginTop = INTERCOMPONENT_MARGIN;
		width = this.Container4_Subcontainer2.Width - 2.0*marginLeft;
		height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		this.TextBlock_FBUpdateDelay.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.TextBlock_FBUpdateDelay.Width = width;
		this.TextBlock_FBUpdateDelay.Height = height;
		this.TextBlock_FBUpdateDelay.FontSize = CONTAINERTEXT_FONTSIZE;

		marginTop = this.TextBlock_FBUpdateDelay.Margin.Top + this.TextBlock_FBUpdateDelay.Height + INTERCOMPONENT_MARGIN;

		this.TextBox_FBUpdateDelay.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.TextBox_FBUpdateDelay.Width = width;
		this.TextBox_FBUpdateDelay.Height = height;
		this.TextBox_FBUpdateDelay.FontSize = CONTAINERTEXT_FONTSIZE;

		marginTop = this.TextBox_FBUpdateDelay.Margin.Top + this.TextBox_FBUpdateDelay.Height + INTERCOMPONENT_MARGIN;
		height = BUTTON_HEIGHT;

		this.Button_FBUpdateDelay.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.Button_FBUpdateDelay.Width = width;
		this.Button_FBUpdateDelay.Height = height;

		/*SETUP CONTAINER 4 SUBCONTAINER 3*/

		marginLeft = INTERCOMPONENT_MARGIN;
		marginTop = INTERCOMPONENT_MARGIN;
		width = this.Container4_Subcontainer3.Width - 2.0*marginLeft;
		height = CONTAINERTEXT_FONTSIZE + CONTAINERTEXT_FONTSIZEMARGIN;

		this.TextBlock_FBUpdateAmp.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.TextBlock_FBUpdateAmp.Width = width;
		this.TextBlock_FBUpdateAmp.Height = height;
		this.TextBlock_FBUpdateAmp.FontSize = CONTAINERTEXT_FONTSIZE;

		marginTop = this.TextBlock_FBUpdateAmp.Margin.Top + this.TextBlock_FBUpdateAmp.Height + INTERCOMPONENT_MARGIN;

		this.TextBox_FBUpdateAmp.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.TextBox_FBUpdateAmp.Width = width;
		this.TextBox_FBUpdateAmp.Height = height;
		this.TextBox_FBUpdateAmp.FontSize = CONTAINERTEXT_FONTSIZE;

		marginTop = this.TextBox_FBUpdateAmp.Margin.Top + this.TextBox_FBUpdateAmp.Height + INTERCOMPONENT_MARGIN;
		height = BUTTON_HEIGHT;

		this.Button_FBUpdateAmp.Margin = new Thickness(marginLeft, marginTop, 0.0, 0.0);
		this.Button_FBUpdateAmp.Width = width;
		this.Button_FBUpdateAmp.Height = height;
	}

	private void updateTextParams()
	{
		uint nFX = 0U;
		string paramsIn = "";
		string paramsOut = "";

		paramsIn = "Dry Input Amplitude: ";
		paramsIn += CPPCore.GetDryInputAmplitude().ToString();
		paramsIn += "\r\n";

		for(nFX = 0U; nFX < CPPCore.RTDELAY_N_FFCH; nFX++)
		{
			paramsIn += "\r\nFF Channel ";
			paramsIn += (nFX + 1U).ToString();
			paramsIn += ":\tDelay Time: ";
			paramsIn += CPPCore.GetFFDelay((nuint) nFX).ToString();
			paramsIn += " samples\tDelay Amplitude: ";
			paramsIn += CPPCore.GetFFAmplitude((nuint) nFX).ToString();
		}

		paramsOut = "Output Amplitude: ";
		paramsOut += CPPCore.GetOutputAmplitude().ToString();
		paramsOut += "\r\n";

		for(nFX = 0U; nFX < CPPCore.RTDELAY_N_FBCH; nFX++)
		{
			paramsOut += "\r\nFB Channel ";
			paramsOut += (nFX + 1U).ToString();
			paramsOut += ":\tDelay Time: ";
			paramsOut += CPPCore.GetFBDelay((nuint) nFX).ToString();
			paramsOut += " samples\tDelay Amplitude: ";
			paramsOut += CPPCore.GetFBAmplitude((nuint) nFX).ToString();
		}

		this.TextBlock_ParamsIn.Text = paramsIn;
		this.TextBlock_ParamsOut.Text = paramsOut;
	}

	private int ffselGetSelection()
	{
		bool? bRet;
		int nRadioButton;

		nRadioButton = 0;
		while(nRadioButton < ((int) CPPCore.RTDELAY_N_FFCH))
		{
			bRet = this.ffselRadioButtonArray[nRadioButton].IsChecked;
			if(bRet == true) return nRadioButton;

			nRadioButton++;
		}

		return -1;
	}

	private int fbselGetSelection()
	{
		bool? bRet;
		int nRadioButton;

		nRadioButton = 0;
		while(nRadioButton < ((int) CPPCore.RTDELAY_N_FBCH))
		{
			bRet = this.fbselRadioButtonArray[nRadioButton].IsChecked;
			if(bRet == true) return nRadioButton;

			nRadioButton++;
		}

		return -1;
	}

	private void onPageLoaded(object sender, RoutedEventArgs e)
	{
		this.align();
		this.updateTextParams();
	}

	private void onButtonStopPlaybackClicked(object sender, RoutedEventArgs e)
	{
		CPPCore.StopPlayback();
	}

	private void onButtonResetAllParamsClicked(object sender, RoutedEventArgs e)
	{
		if(!CPPCore.ResetFFParams()) Win32Aux._MessageBox(CPPCore.GetLastErrorMessage(), "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
		if(!CPPCore.ResetFBParams()) Win32Aux._MessageBox(CPPCore.GetLastErrorMessage(), "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));

		this.updateTextParams();
	}

	private void onButtonUpdateDryInAmpClicked(object sender, RoutedEventArgs e)
	{
		if(RuntimeHandler.AttemptUpdateParam(this.TextBox_UpdateDryInAmp.Text, RuntimeHandler.ATTEMPTUPDATEPARAM_DRYINAMP, -1))
		{
			this.updateTextParams();
			this.TextBox_UpdateDryInAmp.Text = "";
		}
	}

	private void onButtonUpdateOutAmpClicked(object sender, RoutedEventArgs e)
	{
		if(RuntimeHandler.AttemptUpdateParam(this.TextBox_UpdateOutAmp.Text, RuntimeHandler.ATTEMPTUPDATEPARAM_OUTAMP, -1))
		{
			this.updateTextParams();
			this.TextBox_UpdateOutAmp.Text = "";
		}
	}

	private void onButtonFFUpdateDelayClicked(object sender, RoutedEventArgs e)
	{
		if(RuntimeHandler.AttemptUpdateParam(this.TextBox_FFUpdateDelay.Text, RuntimeHandler.ATTEMPTUPDATEPARAM_FFDELAY, this.ffselGetSelection()))
		{
			this.updateTextParams();
			this.TextBox_FFUpdateDelay.Text = "";
		}
	}

	private void onButtonFFUpdateAmpClicked(object sender, RoutedEventArgs e)
	{
		if(RuntimeHandler.AttemptUpdateParam(this.TextBox_FFUpdateAmp.Text, RuntimeHandler.ATTEMPTUPDATEPARAM_FFAMP, this.ffselGetSelection()))
		{
			this.updateTextParams();
			this.TextBox_FFUpdateAmp.Text = "";
		}
	}

	private void onButtonFFResetClicked(object sender, RoutedEventArgs e)
	{
		if(!CPPCore.ResetFFParams())
		{
			Win32Aux._MessageBox(CPPCore.GetLastErrorMessage(), "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
			return;
		}

		this.updateTextParams();
	}

	private void onButtonFBUpdateDelayClicked(object sender, RoutedEventArgs e)
	{
		if(RuntimeHandler.AttemptUpdateParam(this.TextBox_FBUpdateDelay.Text, RuntimeHandler.ATTEMPTUPDATEPARAM_FBDELAY, this.fbselGetSelection()))
		{
			this.updateTextParams();
			this.TextBox_FBUpdateDelay.Text = "";
		}
	}

	private void onButtonFBUpdateAmpClicked(object sender, RoutedEventArgs e)
	{
		if(RuntimeHandler.AttemptUpdateParam(this.TextBox_FBUpdateAmp.Text, RuntimeHandler.ATTEMPTUPDATEPARAM_FBAMP, this.fbselGetSelection()))
		{
			this.updateTextParams();
			this.TextBox_FBUpdateAmp.Text = "";
		}
	}

	private void onButtonFBResetClicked(object sender, RoutedEventArgs e)
	{
		if(!CPPCore.ResetFBParams())
		{
			Win32Aux._MessageBox(CPPCore.GetLastErrorMessage(), "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
			return;
		}

		this.updateTextParams();
	}
}
