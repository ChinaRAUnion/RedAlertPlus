using Caliburn.Micro;
using ChinaRAUnion.RedAlertPlus.Resource;
using ChinaRAUnion.RedAlertPlus.Rules;
using ChinaRAUnion.RedAlertPlus.Services;
using ChinaRAUnion.RedAlertPlus.Shell.Areas.MainMenu.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tomato.Mvvm;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace ChinaRAUnion.RedAlertPlus.Shell.ViewModels
{
    public class SplashViewModel : BindableBase
    {
        private readonly INavigationService _navigationService;
        private readonly IResourceManager _resourceManager;

        public SplashViewModel(INavigationService navigationService, IResourceManager resourceManager)
        {
            _navigationService = navigationService;
            _resourceManager = resourceManager;
        }

        public async void OnLoaded()
        {
            IoC.Get<IInputService>().SetCursor(CursorType.None);

            await LoadResources();
            NavigateToMainMenu();
        }

        private void NavigateToMainMenu()
        {
            //_navigationService.For<MainMenuLayoutViewModel>().Navigate();
            _navigationService.For<Areas.BattleControl.ViewModels.BattleControlViewModel>().Navigate();
        }

        private async Task LoadResources()
        {
            await _resourceManager.InitializeAsync();
            await IoC.Get<IRulesLoader>().InitializeAsync();
        }
    }
}
