using ChinaRAUnion.RedAlertPlus.FileFormat;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ChinaRAUnion.RedAlertPlus.Map
{
    public class TileSetReader
    {
        private TileSet _rawTileSet;


        protected TileSetReader()
        {

        }

        private async Task LoadAsync(Stream stream)
        {
            var archive = new ZipArchive(stream, ZipArchiveMode.Read);
            await Task.WhenAll(LoadTileSetAsync(archive.GetEntry(TileSetJsonFileName)),
                LoadImageAsync(archive.GetEntry(ImageFileName)), LoadExtraImageAsync(archive.GetEntry(ExtraImageFileName)));
        }

        private Task LoadImageAsync(ZipArchiveEntry entry)
        {
            throw new NotImplementedException();
        }

        private Task LoadExtraImageAsync(ZipArchiveEntry entry)
        {
            throw new NotImplementedException();
        }

        private async Task LoadTileSetAsync(ZipArchiveEntry entry)
        {
            using (var reader = new StreamReader(entry.Open()))
            {
                _rawTileSet = JsonConvert.DeserializeObject<TileSet>(await reader.ReadToEndAsync());
            }
        }

        public static async Task<TileSetReader> CreateFromStream(Stream stream)
        {
            var reader = new TileSetReader();
            await reader.LoadAsync(stream);
            return reader;
        }


        private const string TileSetJsonFileName = "tileSet.json";
        private const string ImageFileName = "image.dds";
        private const string ExtraImageFileName = "extraImage.dds";
    }
}
