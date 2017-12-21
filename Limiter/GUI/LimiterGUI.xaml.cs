using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;
using BuzzGUI.Interfaces;
using BuzzGUI.Common;

namespace Limiter.GUI
{
	public class MachineGUIFactory : IMachineGUIFactory
	{
		public IMachineGUI CreateGUI(IMachineGUIHost host) { return new LimiterGUI(); }
	}

	public partial class LimiterGUI : UserControl, IMachineGUI
	{
		DispatcherTimer timer;

		public LimiterGUI()
		{
			InitializeComponent();

			this.IsVisibleChanged += (sender, e) =>
			{
				if (IsVisible && timer == null)
				{
					SetTimer();
				}
				else if (!IsVisible && timer != null)
				{
					timer.Stop();
					timer = null;
				}
			};

			this.SizeChanged += (sender, e) => { UpdateMeter(); };
		}

		void SetTimer()
		{
			timer = new DispatcherTimer();
			timer.Interval = TimeSpan.FromMilliseconds(20);
			timer.Tick += (sender, e) => { UpdateMeter(); };
			timer.Start();
		}

		void UpdateMeter()
		{
			var db = Math.Min(0, Math.Max(-30, Decibel.FromAmplitude(Attenuation)));
			var w = (db + 30.0) / 30.0 * meter.ActualWidth;
			mask.Width = w;
		}

		double Attenuation
		{
			get
			{
				if (machine != null)
				{
					var request = new MemoryStream();
					var bw = new BinaryWriter(request);
					bw.Write((int)0);	// request attenuation value
					var response = machine.SendGUIMessage(request.ToArray());
					if (response != null)
					{
						var br = new BinaryReader(new MemoryStream(response));
						return br.ReadDouble();
					}
				}

				return 1.0;
			}
		}

		IMachine machine;
		public IMachine Machine
		{
			get { return machine; }
			set
			{
				machine = value;
			}
		}

	}
}
