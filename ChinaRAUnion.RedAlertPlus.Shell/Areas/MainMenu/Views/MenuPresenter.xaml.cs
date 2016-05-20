using Caliburn.Micro;
using ChinaRAUnion.RedAlertPlus.Primitives;
using ChinaRAUnion.RedAlertPlus.Shell.Areas.MainMenu.ViewModels;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Tomato.Mvvm;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

namespace ChinaRAUnion.RedAlertPlus.Shell.Areas.MainMenu.Views
{
    public sealed partial class MenuPresenter : UserControl
    {
        public static DependencyProperty MenuProperty { get; } = DependencyProperty.Register(nameof(Menu), typeof(MenuConfig),
            typeof(MenuPresenter), new PropertyMetadata(DependencyProperty.UnsetValue, OnMenuPropertyChanged));

        public MenuConfig Menu
        {
            get { return (MenuConfig)GetValue(MenuProperty); }
            set { SetValue(MenuProperty, value); }
        }

        public BindableCollection<MenuButtonViewModel> Buttons { get; } = new BindableCollection<MenuButtonViewModel>();
        private static readonly Size ButtonSize = new Size(168.0f, 42.0f);
        private int _oldButtonsCount;

        public MenuPresenter()
        {
            this.InitializeComponent();
            buttonsEnteringDelayTimer.Tick += ButtonsEnteringDelayTimer_Tick;
            buttonsLeavingDelayTimer.Tick += ButtonsLeavingDelayTimer_Tick;
        }

        private static void OnMenuPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var presenter = ((MenuPresenter)d);
            presenter.RefreshButtons(presenter.RenderSize, force: true);
        }

        private void UserControl_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            RefreshButtons(e.NewSize);
        }

        private void RefreshButtons(Size size, bool force = false)
        {
            // 测算需要容纳的 Button 数目，向下取证
            var buttonsCount = (int)Math.Floor(size.Height / ButtonSize.Height);
            if (force || _oldButtonsCount != buttonsCount)
            {
                var buttons = new List<MenuButtonViewModel>();
                // 顶部按钮
                if (Menu?.TopButtons != null)
                    foreach (var button in Menu.TopButtons)
                        buttons.Add(new MenuButtonViewModel(this, button));
                // padding 按钮
                var paddingCount = Math.Max(0, buttonsCount - (Menu?.TopButtons?.Count ?? 0) - (Menu?.BottomButtons?.Count ?? 0));
                for (int i = 0; i < paddingCount; i++)
                    buttons.Add(new MenuButtonViewModel(this, null));
                // 底部按钮
                if (Menu?.BottomButtons != null)
                    foreach (var button in Menu.BottomButtons)
                        buttons.Add(new MenuButtonViewModel(this, button));

                Buttons.Clear();
                Buttons.AddRange(buttons);
                _loaded = _entered = _leaved = 0;
                _oldButtonsCount = buttonsCount;
            }
        }

        private DispatcherTimer buttonsEnteringDelayTimer = new DispatcherTimer()
        {
            Interval = TimeSpan.FromMilliseconds(20)
        };

        private int _loaded = 0;
        private IEnumerator buttonsEnteringEnum;
        public void ReportLoaded()
        {
            if (++_loaded == Buttons.Count)
            {
                buttonsEnteringEnum = EnteringButtons().GetEnumerator();
                buttonsEnteringDelayTimer.Start();
            }
        }

        private void ButtonsEnteringDelayTimer_Tick(object sender, object e)
        {
            buttonsEnteringEnum.MoveNext();
        }

        private IEnumerable EnteringButtons()
        {
            //_soundService.PlaySound(SoundConstants.SlideIn);
            foreach (var button in Buttons)
            {
                button.GoToEnteringState();
                yield return null;
            }
            buttonsEnteringDelayTimer.Stop();
        }

        private int _entered = 0;
        public void ReportEntered()
        {
            if (++_entered == Buttons.Count)
            {
                ShowContent();
            }
        }

        private void ShowContent()
        {
            foreach (var button in Buttons)
                button.GoToShowContentState();
        }

        private DispatcherTimer buttonsLeavingDelayTimer = new DispatcherTimer()
        {
            Interval = TimeSpan.FromMilliseconds(20)
        };

        private IEnumerator buttonsLeavingEnum;
        private void ButtonsLeavingDelayTimer_Tick(object sender, object e)
        {
            buttonsLeavingEnum.MoveNext();
        }

        private IEnumerable LeavingButtons()
        {
            //_soundService.PlaySound(SoundConstants.SlideOut);
            foreach (var button in Buttons)
            {
                button.GoToLeavingState();
                yield return null;
            }
            buttonsLeavingDelayTimer.Stop();
        }

        private int _leaved = 0;
        public void ReportLeaved()
        {
            if (++_leaved == Buttons.Count)
                DoInvoke();
        }

        private MenuButtonViewModel _invokeRequest;
        public void RequestInvoke(MenuButtonViewModel source)
        {
            _invokeRequest = source;
            buttonsLeavingEnum = LeavingButtons().GetEnumerator();
            buttonsLeavingDelayTimer.Start();
        }

        private void DoInvoke()
        {
            _invokeRequest?.Invoke();
            _invokeRequest = null;
        }
    }
}
