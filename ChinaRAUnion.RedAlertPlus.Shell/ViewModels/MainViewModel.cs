using Caliburn.Micro;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace ChinaRAUnion.RedAlertPlus.Shell.ViewModels
{
    public class MainViewModel
    {
        private readonly WinRTContainer _container;
        private INavigationService _navigationService;

        public MainViewModel(WinRTContainer container)
        {
            _container = container;
        }

        public void SetupNavigationService(object sender, RoutedEventArgs e)
        {
            _navigationService = _container.RegisterNavigationService((Frame)sender);
            NavigateToSplash();
        }

        private void NavigateToSplash()
        {
            _navigationService.For<SplashViewModel>().Navigate();
        }
    }
}
