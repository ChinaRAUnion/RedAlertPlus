
using Caliburn.Micro;
using ChinaRAUnion.RedAlertPlus.Resource;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tomato.Mvvm;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace ChinaRAUnion.RedAlertPlus.MapEditor.ViewModels
{
    public class SplashViewModel : BindableBase
    {
        private INavigationService _navigationService;
        private readonly IResourceManager _resourceManager;
        private readonly WinRTContainer _container;

        public SplashViewModel(WinRTContainer container, IResourceManager resourceManager)
        {
            _container = container;
            _resourceManager = resourceManager;
        }

        public async void SetupNavigationService(object sender, RoutedEventArgs e)
        {
            _navigationService = _container.RegisterNavigationService((Frame)sender);
            await LoadResources();
            NavigateToMainView();
        }

        private void NavigateToMainView()
        {
            _navigationService.For<MainViewModel>().Navigate();
        }

        private async Task LoadResources()
        {
            await _resourceManager.InitializeAsync();
        }
    }
}
