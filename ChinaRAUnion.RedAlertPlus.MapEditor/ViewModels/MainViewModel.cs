using ChinaRAUnion.RedAlertPlus.Map;
using ChinaRAUnion.RedAlertPlus.MapEditor.Areas.Map.ViewModels;
using ChinaRAUnion.RedAlertPlus.MapEditor.Models;
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

        public async void OnLoaded()
        {
            var mapInfo = new MapInfo
            {
                Width = 100,
                Height = 200,
                TileSetName = "Urban"
            };
            var tileSetReader = await TileSetReader.CreateFromTileSetPackage(_resourceManager.TileSets.FindResource(mapInfo.TileSetName).Value);
            MapDisplay.InitializeMap(mapInfo);
            MapOperationSelector.InitializeMap(mapInfo, tileSetReader);
        }
    }
}
