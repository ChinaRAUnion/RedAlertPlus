using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tomato.Mvvm;
using Windows.UI.Xaml.Data;

namespace ChinaRAUnion.RedAlertPlus.MapEditor.Areas.Map.ViewModels
{
    class MapOperationSelectorViewModel : BindableBase
    {
        public ObservableCollection<MapOperation> GeneralOperations { get; } = new ObservableCollection<MapOperation>();

        public MapOperationSelectorViewModel()
        {
        }

        public void OnLoaded()
        {
            new MapOperation[]
            {
                new MapOperation {  Name = "无" },
                new MapOperation {  Name = "删除对象" },
                new MapOperation { Name = "地表", Children = new ObservableCollection<MapOperation>
                {
                    new MapOperation { Name = "月球表面" }
                } }
            }.Sink(GeneralOperations.Add);
        }
    }

    class MapOperation
    {
        public string Name { get; set; }

        public ObservableCollection<MapOperation> Children { get; set; }
    }
}
