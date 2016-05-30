using Caliburn.Micro;
using ChinaRAUnion.RedAlertPlus;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Tomato.Media.Codec;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Activation;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace ChinaRAUnion.RedAlertPlus.Shell
{
    /// <summary>
    /// 提供特定于应用程序的行为，以补充默认的应用程序类。
    /// </summary>
    sealed partial class App : CaliburnApplication
    {
        private WinRTContainer _container;
        private IEventAggregator _eventAggregator;
        private ILog _logger;
        private CodecManager _codecManager = new CodecManager();

        /// <summary>
        /// 初始化单一实例应用程序对象。这是执行的创作代码的第一行，
        /// 已执行，逻辑上等同于 main() 或 WinMain()。
        /// </summary>
        public App()
        {
            LogManager.GetLog = type => new DebugLog(type);
            _logger = LogManager.GetLog(typeof(App));

            //Microsoft.ApplicationInsights.WindowsAppInitializer.InitializeAsync(
            //    Microsoft.ApplicationInsights.WindowsCollectors.Metadata |
            //    Microsoft.ApplicationInsights.WindowsCollectors.Session |
            //    Microsoft.ApplicationInsights.WindowsCollectors.PageView |
            //    Microsoft.ApplicationInsights.WindowsCollectors.UnhandledException);
            //TaskScheduler.UnobservedTaskException += TaskScheduler_UnobservedTaskException;
            //this.UnhandledException += App_UnhandledException;
            _codecManager.RegisterDefaultCodecs();
            this.InitializeComponent();
        }

        private void App_UnhandledException(object sender, Windows.UI.Xaml.UnhandledExceptionEventArgs e)
        {
            //var client = new Microsoft.ApplicationInsights.TelemetryClient();
            //client.TrackException(e.Exception);
            _logger.Error(e.Exception);
            e.Handled = true;
        }

        private void TaskScheduler_UnobservedTaskException(object sender, UnobservedTaskExceptionEventArgs e)
        {
            //var client = new Microsoft.ApplicationInsights.TelemetryClient();
            //client.TrackException(e.Exception);
            _logger.Error(e.Exception);
            e.SetObserved();
        }

        protected override void Configure()
        {
            //var config = LoadConfig();
            _container = new WinRTContainer();
            _container.RegisterWinRTServices();

            _container.UseShell();
            _container.UseResource()
                .UseResourceManager(new Uri("ms-appx:///Assets/Config/UIArt.json"),
                                    new Uri("ms-appx:///Assets/Config/UIAudio.json"),
                                    new Uri("ms-appx:///Assets/Config/Shaders.json"),
                                    new Uri("ms-appx:///Assets/Config/TileSets.json"),
                                    new Uri("ms-appx:///Assets/Config/Maps.json"));
            _container.UseAudio();
            _eventAggregator = _container.GetInstance<IEventAggregator>();
            ViewModelBinder.ApplyConventionsByDefault = false;
        }

        protected override object GetInstance(Type service, string key)
        {
            return _container.GetInstance(service, key);
        }

        protected override IEnumerable<object> GetAllInstances(Type service)
        {
            return _container.GetAllInstances(service);
        }

        protected override void BuildUp(object instance)
        {
            _container.BuildUp(instance);
        }

        protected override void OnLaunched(LaunchActivatedEventArgs args)
        {
#if DEBUG
            if (System.Diagnostics.Debugger.IsAttached)
            {
                this.DebugSettings.EnableFrameRateCounter = true;
            }
#endif
            DisplayRootViewFor<ViewModels.MainViewModel>();
            if (args.PreviousExecutionState == ApplicationExecutionState.Terminated)
            {
                //_eventAggregator.PublishOnUIThread(new ResumeStateMessage());
            }
        }
    }
}
