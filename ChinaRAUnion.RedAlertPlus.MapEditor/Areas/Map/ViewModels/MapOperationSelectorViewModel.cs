using Caliburn.Micro;
using ChinaRAUnion.RedAlertPlus.Map;
using ChinaRAUnion.RedAlertPlus.MapEditor.Models;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tomato.Mvvm;
using Windows.ApplicationModel.Resources;
using Windows.UI.Xaml.Data;
using ChinaRAUnion.RedAlertPlus.FileFormat;
using ChinaRAUnion.RedAlertPlus.Resource;

namespace ChinaRAUnion.RedAlertPlus.MapEditor.Areas.Map.ViewModels
{
    class MapOperationSelectorViewModel : BindableBase
    {
        public ObservableCollection<MapOperation> GeneralOperations { get; } = new ObservableCollection<MapOperation>();
        public ObservableCollection<MapOperation> UnitOperations { get; }

        private MapOperation _terrainMapOperation;

        public MapOperationSelectorViewModel()
        {
            _terrainMapOperation = new MapOperation
            {
                Name = "Editor/Terrain"
            };
            UnitOperations = new ObservableCollection<MapOperation>(PopulateUnitOperations());
        }

        public void OnLoaded()
        {
            new MapOperation[]
            {
                new MapOperation { Name = "Editor/None" },
                new MapOperation { Name = "Editor/DeleteObject" },
                _terrainMapOperation,
            }.Sink(GeneralOperations.Add);
        }

        public void InitializeMap(MapInfo mapInfo, TileSetReader tileSetReader)
        {
            PopulateTerrainMapOperation(tileSetReader.TileSet);
        }

        private void PopulateTerrainMapOperation(TileSet tileSet)
        {
            _terrainMapOperation.Children.Clear();
            foreach (var item in from u in tileSet.PickAnyTileUnits
                                 where u.Category != "Ramps"
                                 select u)
                _terrainMapOperation.Children.Add(new MapOperation
                {
                    Name = $"/{tileSet.Name}/{item.Category}"
                });
        }

        private IEnumerable<MapOperation> PopulateUnitOperations()
        {
            var resManager = IoC.Get<IResourceManager>();
            var infantryOperation = new MapOperation { Name = "Editor/Infantry" };
            //PopulateInfantryOperation(infantryOperation);

            return new[] { infantryOperation };
        }
    }

    class MapOperation
    {
        public string Name { get; set; }

        public string DisplayName => IoC.Get<ResourceLoader>().GetString(Name);

        public ObservableCollection<MapOperation> Children { get; set; } = new ObservableCollection<MapOperation>();
    }
}
