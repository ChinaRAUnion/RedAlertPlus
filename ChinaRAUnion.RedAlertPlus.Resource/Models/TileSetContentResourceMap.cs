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
using Windows.Storage.Streams;

namespace ChinaRAUnion.RedAlertPlus.Resource.Models
{
    class TileSetContentResourceMap : ResourceMap<InputStreamWithContentTypeResource, ITileSetPackageContent>
    {
        public TileSetContentResourceMap(IReadOnlyCollection<InputStreamWithContentTypeResource> resources) : base(resources)
        {
        }

        protected override async Task<ITileSetPackageContent> LoadResourceCoreAsync(Uri source)
        {
            var file = await (source.IsFile ? StorageFile.GetFileFromPathAsync(source.LocalPath) : StorageFile.GetFileFromApplicationUriAsync(source));
            var archive = new ZipArchive((await file.OpenReadAsync()).AsStream(), ZipArchiveMode.Read);
            return new TileSetContent(archive);
        }

        private class TileSetContent : ITileSetPackageContent
        {
            public IInputStream ExtraImage => _archive.GetEntry(FileFormat.TileSet.ExtraImageFileName).Open().AsInputStream();
            public IInputStream Image => _archive.GetEntry(FileFormat.TileSet.ImageFileName).Open().AsInputStream();
            public IInputStream TileSet => _archive.GetEntry(FileFormat.TileSet.TileSetJsonFileName).Open().AsInputStream();

            private readonly ZipArchive _archive;

            public TileSetContent(ZipArchive archive)
            {
                _archive = archive;
            }
        }
    }
}
