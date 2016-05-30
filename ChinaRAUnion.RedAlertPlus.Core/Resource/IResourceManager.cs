using ChinaRAUnion.RedAlertPlus.Primitives;
using Microsoft.Graphics.Canvas;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Storage.Streams;

namespace ChinaRAUnion.RedAlertPlus.Resource
{
    public interface IResourceManager
    {
        IResourceMap<FrameAnimationImageResource, CanvasBitmap> FrameAnimationImages { get; }
        IResourceMap<InputStreamWithContentTypeResource, IRandomAccessStream> ThemeAudios { get; }
        IResourceMap<InputStreamWithContentTypeResource, IRandomAccessStream> Shaders { get; }
        IResourceMap<InputStreamWithContentTypeResource, IRandomAccessStream> Maps { get; }
        IResourceMap<InputStreamWithContentTypeResource, TileSetContent> TileSets { get; }
        Task InitializeAsync();
    }

    public interface IResourceMap<TDefination, TResult> where TDefination : ResourceBase
    {
        ResourceEntry<TDefination, TResult> FindResource(string key);
    }
}
