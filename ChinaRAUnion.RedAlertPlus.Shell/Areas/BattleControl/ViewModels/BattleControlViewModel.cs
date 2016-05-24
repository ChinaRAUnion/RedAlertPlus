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
        private readonly SwapChainProvider _swapChainProvider;

        public BattleControlViewModel()
        {
            _swapChainProvider = new SwapChainProvider();
        }

        public void SetupSwapChainPanel(object sender, RoutedEventArgs e)
        {
            _swapChainProvider.SetSwapChainPanel((SwapChainPanel)sender);
        }
    }
}
