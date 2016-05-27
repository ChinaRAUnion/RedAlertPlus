using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tomato.Mvvm;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace ChinaRAUnion.RedAlertPlus.Shell.Areas.BattleControl.ViewModels
{
    public class BattleControlViewModel : BindableBase
    {
        private readonly GameEngine _gameEngine;

        public BattleControlViewModel(IGameEngineResourceResolver resourceResolver)
        {
            _gameEngine = new GameEngine(resourceResolver);
        }

        public async void SetupSwapChainPanel(object sender, RoutedEventArgs e)
        {
            _gameEngine.SetSwapChainPanel((SwapChainPanel)sender);
            await _gameEngine.InitializeAsync();
            _gameEngine.StartRenderLoop();
        }
    }
}
