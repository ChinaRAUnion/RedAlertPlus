using Caliburn.Micro;
using ChinaRAUnion.RedAlertPlus.MapEditor.Models;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tomato.Mvvm;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;

namespace ChinaRAUnion.RedAlertPlus.MapEditor.Areas.Map.ViewModels
{
    class MapDisplayViewModel : BindableBase
    {
        private readonly GameEngine _gameEngine;

        public MapDisplayViewModel()
        {
            _gameEngine = new GameEngine(IoC.Get<IGameEngineResourceResolver>());
            _gameEngine.Mode = GameMode.MapEdit;
        }

        public void SetupSwapChainPanel(object sender, RoutedEventArgs e)
        {
            var swapChainPanel = (SwapChainPanel)sender;

            _gameEngine.SetSwapChainPanel(swapChainPanel);
        }

        public async void InitializeMap(MapInfo mapInfo)
        {
            _gameEngine.GenerateMap(new GameMapGenerateOptions
            {
                Width = mapInfo.Width,
                Height = mapInfo.Height,
                TileSetName = mapInfo.TileSetName
            });
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
