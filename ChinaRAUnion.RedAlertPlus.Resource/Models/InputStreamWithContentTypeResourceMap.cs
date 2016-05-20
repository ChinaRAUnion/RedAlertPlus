using ChinaRAUnion.RedAlertPlus.Primitives;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Storage;
using Windows.Storage.Streams;

namespace ChinaRAUnion.RedAlertPlus.Resource.Models
{
    class InputStreamWithContentTypeResourceMap : ResourceMap<InputStreamWithContentTypeResource, IRandomAccessStream>
    {
        public InputStreamWithContentTypeResourceMap(IReadOnlyCollection<InputStreamWithContentTypeResource> resources) : base(resources)
        {
        }

        protected override async Task<IRandomAccessStream> LoadResourceCoreAsync(Uri source)
        {
            var file = await(source.IsFile ? StorageFile.GetFileFromPathAsync(source.LocalPath) : StorageFile.GetFileFromApplicationUriAsync(source));
            return await file.OpenReadAsync();
        }
    }
}
