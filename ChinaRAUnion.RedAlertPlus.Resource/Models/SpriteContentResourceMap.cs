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
    class SpriteContentResourceMap : ResourceMap<InputStreamWithContentTypeResource, ISpritePackageContent>
    {
        public SpriteContentResourceMap(IReadOnlyCollection<InputStreamWithContentTypeResource> resources) : base(resources)
        {
        }

        protected override async Task<ISpritePackageContent> LoadResourceCoreAsync(Uri source)
        {
            var file = await (source.IsFile ? StorageFile.GetFileFromPathAsync(source.LocalPath) : StorageFile.GetFileFromApplicationUriAsync(source));
            var archive = new ZipArchive((await file.OpenReadAsync()).AsStream(), ZipArchiveMode.Read);
            return new SpritePackageContent(archive);
        }

        private class SpritePackageContent : ISpritePackageContent
        {
            public IInputStream Image => _archive.GetEntry("ani.dds").Open().AsInputStream();

            public IInputStream Sequence => _archive.GetEntry("sequence.json").Open().AsInputStream();

            public IInputStream Coordinate => _archive.GetEntry("ani.json").Open().AsInputStream();

            private readonly ZipArchive _archive;

            public SpritePackageContent(ZipArchive archive)
            {
                _archive = archive;
            }
        }
    }
}
