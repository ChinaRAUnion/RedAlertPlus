using Caliburn.Micro;
using ChinaRAUnion.RedAlertPlus.Messages;
using ChinaRAUnion.RedAlertPlus.Primitives;
using ChinaRAUnion.RedAlertPlus.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tomato.Media.Gaming.Controls;
using Tomato.Mvvm;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace ChinaRAUnion.RedAlertPlus.Shell.Areas.MainMenu.ViewModels
{
    public class MainMenuLayoutViewModel : BindableBase, IHandle<ChangeMenuConfigMessage>, IHandle<MainMenuNavigateMessage>
    {
        private INavigationService _contentNavigator;
        private FrameAnimationImage _alertMeterPointerAnimator;

        private MenuConfig _menuConfig;
        public MenuConfig MenuConfig
        {
            get { return _menuConfig; }
            set { SetProperty(ref _menuConfig, value); }
        }

        public string MenuTitle => MenuConfig?.Title;
        private readonly IThemeService _themeService;

        public MainMenuLayoutViewModel(IEventAggregator eventAggreator, IThemeService themeService)
        {
            eventAggreator.Subscribe(this);

            _themeService = themeService;
        }

        public void OnLoaded()
        {
            IoC.Get<IInputService>().SetCursor(CursorType.Normal);
        }

        public void SetupContentFrame(object sender, RoutedEventArgs e)
        {
            _contentNavigator = new FrameAdapter((Frame)sender);
            NavigateToMainMenu();
            PlayIntro();
        }

        private void PlayIntro()
        {
            _themeService.PlayIntro();
        }

        private void NavigateToMainMenu()
        {
            _contentNavigator.For<MainMenuViewModel>().Navigate();
        }

        public void OnAlterMeterEnteringAnimationEnded()
        {
            _alertMeterPointerAnimator.Visibility = Visibility.Visible;
            _alertMeterPointerAnimator.Play();
        }

        public void SetupAlertMeterPointerAnimator(object sender, RoutedEventArgs e)
        {
            _alertMeterPointerAnimator = (FrameAnimationImage)sender;
        }

        public void Handle(ChangeMenuConfigMessage message)
        {
            MenuConfig = message.MenuConfig;
            OnPropertyChanged(nameof(MenuTitle));
        }

        public void Handle(MainMenuNavigateMessage message)
        {
            _contentNavigator.NavigateToViewModel(message.ViewModelType);
        }
    }
}
