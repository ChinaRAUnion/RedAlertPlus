using ChinaRAUnion.RedAlertPlus.FileFormat;
using ChinaRAUnion.RedAlertPlus.Primitives;
using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Storage;

namespace ChinaRAUnion.RedAlertPlus.Resource.Models
{
    class TileSetContentResourceMap : ResourceMap<InputStreamWithContentTypeResource, TileSetContent>
    {
        public TileSetContentResourceMap(IReadOnlyCollection<InputStreamWithContentTypeResource> resources) : base(resources)
        {
        }

        protected override async Task<TileSetContent> LoadResourceCoreAsync(Uri source)
        {
            var file = await (source.IsFile ? StorageFile.GetFileFromPathAsync(source.LocalPath) : StorageFile.GetFileFromApplicationUriAsync(source));
            var archive = new ZipArchive((await file.OpenReadAsync()).AsStream(), ZipArchiveMode.Read);
            return new TileSetContent
            {
                TileSet = archive.GetEntry(TileSet.TileSetJsonFileName).Open().AsInputStream(),
                Image = archive.GetEntry(TileSet.ImageFileName).Open().AsInputStream(),
                ExtraImage = archive.GetEntry(TileSet.ExtraImageFileName).Open().AsInputStream()
            };
        }
    }
}
