using Caliburn.Micro;
using ChinaRAUnion.RedAlertPlus.MapEditor.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.ApplicationModel.Resources;

namespace ChinaRAUnion.RedAlertPlus
{
    static class MapEditorModule
    {
        public static void UseMapEditor(this SimpleContainer container)
        {
            container.Singleton<MainViewModel>();
            container.Singleton<SplashViewModel>();
            //container.Instance(ResourceLoader.GetForCurrentView());
        }

        private static void ModuleReferences()
        {
            var type = new[]
            {
                typeof(Caliburn.Micro.ViewModelBinder)
            };
        }
    }
}
