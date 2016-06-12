using Caliburn.Micro;
using ChinaRAUnion.RedAlertPlus.FileFormat;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Text;
using System.Threading.Tasks;
using Windows.Graphics.Imaging;
using Windows.Storage.Streams;
using Windows.UI.Xaml.Media;

namespace ChinaRAUnion.RedAlertPlus.Map
{
    public class TileSetReader
    {
        private readonly TileSet _rawTileSet;
        private readonly ImageSource[] _pickAnyUnits;
        private TileSetGraphicsProvider _tsgProvider;

        public TileSet TileSet => _rawTileSet;

        public TileSetReader(TileSet tileSet)
        {
            _rawTileSet = tileSet;
            _pickAnyUnits = new ImageSource[tileSet.PickAnyTileUnits.Count];
        }

        private async Task LoadAsync()
        {
            _tsgProvider = await TileSetGraphicsProvider.CreateFromTileSet(IoC.Get<IGameEngineResourceResolver>(), _rawTileSet.Name);
            for (int i = 0; i < _pickAnyUnits.Length; i++)
                _pickAnyUnits[i] = await _tsgProvider.CreatePickAnyUnit(i);
        }

        public Tuple<string, ImageSource> GetPickAnyTileUnit(int id)
        {
            return Tuple.Create(_rawTileSet.PickAnyTileUnits[id].Category, _pickAnyUnits[id]);
        }

        public static async Task<TileSetReader> CreateFromTileSetPackage(ITileSetPackageContent package)
        {
            var reader = new TileSetReader(await LoadTileSetAsync(package.TileSet));
            await reader.LoadAsync();
            return reader;
        }

        private static async Task<TileSet> LoadTileSetAsync(IInputStream stream)
        {
            using (var reader = new StreamReader(stream.AsStreamForRead()))
            {
                return JsonConvert.DeserializeObject<TileSet>(await reader.ReadToEndAsync());
            }
        }
    }
}
