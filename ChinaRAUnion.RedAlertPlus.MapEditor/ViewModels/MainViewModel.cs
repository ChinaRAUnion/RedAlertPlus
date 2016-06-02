using ChinaRAUnion.RedAlertPlus.MapEditor.Areas.Map.ViewModels;
using ChinaRAUnion.RedAlertPlus.Resource;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tomato.Mvvm;

namespace ChinaRAUnion.RedAlertPlus.MapEditor.ViewModels
{
    public class MainViewModel : BindableBase
    {
        private readonly IResourceManager _resourceManager;
        internal MapDisplayViewModel MapDisplay { get; } = new MapDisplayViewModel();
        internal MapOperationSelectorViewModel MapOperationSelector { get; } = new MapOperationSelectorViewModel();

        public MainViewModel(IResourceManager resourceManager)
        {
            _resourceManager = resourceManager;
        }

        public void OnLoaded()
        {
            MapDisplay.InitializeMap(100, 200);
        }
    }
}
