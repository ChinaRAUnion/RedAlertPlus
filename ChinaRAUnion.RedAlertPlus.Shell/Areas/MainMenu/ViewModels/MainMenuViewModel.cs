using Caliburn.Micro;
using ChinaRAUnion.RedAlertPlus.Messages;
using ChinaRAUnion.RedAlertPlus.Primitives;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tomato.Mvvm;

namespace ChinaRAUnion.RedAlertPlus.Shell.Areas.MainMenu.ViewModels
{
    public class MainMenuViewModel : BindableBase
    {
        private static Lazy<MenuConfig> _menuConfig = new Lazy<MenuConfig>(CreateMenuConfig);
        
        public MainMenuViewModel(IEventAggregator eventAggregator)
        {
            eventAggregator.BeginPublishOnUIThread(new ChangeMenuConfigMessage { MenuConfig = _menuConfig.Value });
        }

        static MenuConfig CreateMenuConfig()
        {
            return new MenuConfig
            {
                TitleName = "MainMenu",
                TopButtons = new List<MenuItem>
                {
                    new MenuItem(e => e.BeginPublishOnUIThread(new MainMenuNavigateMessage
                    {
                        ViewModelType = typeof(SinglePlayerViewModel)
                    }))
                    {
                        Name = "SinglePlay"
                    },
                    new MenuItem()
                    {
                        Name = "MultiplyPlay"
                    },
                    new MenuItem()
                    {
                        Name = "Options"
                    }
                },
                BottomButtons = new List<MenuItem>
                {
                    new MenuItem()
                    {
                        Name = "ExitGame"
                    }
                }
            };
        }
    }
}
