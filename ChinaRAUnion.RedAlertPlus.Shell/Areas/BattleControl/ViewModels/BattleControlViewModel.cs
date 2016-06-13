using Caliburn.Micro;
using ChinaRAUnion.RedAlertPlus.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tomato.Mvvm;
using Windows.System;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;
using Windows.UI.Input;

namespace ChinaRAUnion.RedAlertPlus.Shell.Areas.BattleControl.ViewModels
{
    public class BattleControlViewModel : BindableBase
    {
        private readonly GameEngine _gameEngine;

        public BattleControlViewModel(IGameEngineResourceResolver resourceResolver, IGameEngineRulesResolver rulesResolver)
        {
            _gameEngine = new GameEngine(resourceResolver, rulesResolver);
            _gameEngine.UseMap("map1");
        }

        public void OnLoaded()
        {
            IoC.Get<IInputService>().SetCursor(CursorType.NoDrop);
        }

        public async void SetupSwapChainPanel(object sender, RoutedEventArgs e)
        {
            var swapChainPanel = (SwapChainPanel)sender;

            _gameEngine.SetSwapChainPanel(swapChainPanel);
            await _gameEngine.InitializeAsync();
            _gameEngine.StartRenderLoop();
        }

        public void OnPointerMoved(object sender, PointerRoutedEventArgs e)
        {
            var ele = (UIElement)sender;
            _gameEngine.OnPointerMoved(ele.RenderSize, e.GetCurrentPoint(ele));
        }

        public void OnPointerPressed(object sender, PointerRoutedEventArgs e)
        {
            var ele = (UIElement)sender;
            _gameEngine.OnPointerPressed(ele.RenderSize, e.GetCurrentPoint(ele));
        }

        public void OnPointerReleased(object sender, PointerRoutedEventArgs e)
        {
            var ele = (UIElement)sender;
            _gameEngine.OnPointerReleased(ele.RenderSize, e.GetCurrentPoint(ele));
        }
    }
}
