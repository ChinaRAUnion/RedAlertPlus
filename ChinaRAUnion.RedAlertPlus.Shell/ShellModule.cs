using Caliburn.Micro;
using ChinaRAUnion.RedAlertPlus.Services;
using ChinaRAUnion.RedAlertPlus.Shell.Services;
using ChinaRAUnion.RedAlertPlus.Shell.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.ApplicationModel.Resources;

namespace ChinaRAUnion.RedAlertPlus
{
    static class ShellModule
    {
        public static void UseShell(this SimpleContainer container)
        {
            container.Singleton<IInputService, InputService>();
            container.Singleton<MainViewModel>();
            container.Singleton<SplashViewModel>();
            container.Singleton<Shell.Areas.MainMenu.ViewModels.MainMenuLayoutViewModel>();
            container.PerRequest<Shell.Areas.MainMenu.ViewModels.MainMenuViewModel>();
            container.PerRequest<Shell.Areas.MainMenu.ViewModels.SinglePlayerViewModel>();
            container.PerRequest<Shell.Areas.BattleControl.ViewModels.BattleControlViewModel>();
            //container.PerRequest<PlaylistViewModel>();
            //container.Singleton<PlayingViewModel>();
            //container.Singleton<LyricsViewModel>();
            //container.Singleton<EffectsViewModel>();
            //container.Singleton<SettingsViewModel>();
            //container.Singleton<AboutViewModel>();
            //container.Singleton<IConfigurationService, ConfigurationService>();
            container.Instance(ResourceLoader.GetForCurrentView());
        }

        private static void ModuleReferences()
        {
            var type = new[]
            {
                typeof(Caliburn.Micro.ViewModelBinder)
            };
        }
    }
}
