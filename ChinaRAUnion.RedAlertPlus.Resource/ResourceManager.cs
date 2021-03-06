﻿using ChinaRAUnion.RedAlertPlus.Resource;
using ChinaRAUnion.RedAlertPlus.Resource.Models;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Dynamic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation.Collections;
using System.Collections;
using Windows.Storage;
using Newtonsoft.Json;
using ChinaRAUnion.RedAlertPlus.Primitives;
using Microsoft.Graphics.Canvas;
using Windows.Storage.Streams;

namespace ChinaRAUnion.RedAlertPlus.Resource
{
    class ResourceManager : IResourceManager
    {
        public IResourceMap<FrameAnimationImageResource, CanvasBitmap> FrameAnimationImages { get; private set; }

        public IResourceMap<InputStreamWithContentTypeResource, IRandomAccessStream> ThemeAudios { get; private set; }

        public IResourceMap<InputStreamWithContentTypeResource, IRandomAccessStream> Shaders { get; private set; }

        public IResourceMap<InputStreamWithContentTypeResource, IRandomAccessStream> Maps { get; private set; }

        public IResourceMap<InputStreamWithContentTypeResource, ITileSetPackageContent> TileSets { get; private set; }

        public IReadOnlyDictionary<string, UnitArt> UnitArts { get; private set; }

        public IResourceMap<InputStreamWithContentTypeResource, ISpritePackageContent> Sprites { get; private set; }

        private readonly Uri[] _configFiles;
        public ResourceManager(Uri[] configFiles)
        {
            _configFiles = configFiles;
        }

        public async Task InitializeAsync()
        {
            var configs = await Task.WhenAll(_configFiles.Select(LoadConfigFile));
            var frameAnimationImages = new FrameAnimationImageResourceMap(WrapMultiplySections(configs, o => o.FrameAnimationImages));
            var themeAudios = new InputStreamWithContentTypeResourceMap(WrapMultiplySections(configs, o => o.ThemeAudios));
            var shaders = new InputStreamWithContentTypeResourceMap(WrapMultiplySections(configs, o => o.Shaders));
            var maps = new InputStreamWithContentTypeResourceMap(WrapMultiplySections(configs, o => o.Maps));
            var tileSets = new TileSetContentResourceMap(WrapMultiplySections(configs, o => o.TileSets));
            var sprites = new SpriteContentResourceMap(WrapMultiplySections(configs, o => o.Sprites));

            await Task.WhenAll(frameAnimationImages.LoadAsync(), themeAudios.LoadAsync(), shaders.LoadAsync(), maps.LoadAsync(), tileSets.LoadAsync(),
                sprites.LoadAsync());

            FrameAnimationImages = frameAnimationImages;
            ThemeAudios = themeAudios;
            Shaders = shaders;
            Maps = maps;
            TileSets = tileSets;
            UnitArts = WrapMultiplySections(configs, o => o.UnitArts);
            Sprites = sprites;
        }

        private async Task<ResourceConfig> LoadConfigFile(Uri uri)
        {
            var file = await (uri.IsFile ? StorageFile.GetFileFromPathAsync(uri.LocalPath) : StorageFile.GetFileFromApplicationUriAsync(uri));
            return JsonConvert.DeserializeObject<ResourceConfig>(await FileIO.ReadTextAsync(file));
        }

        static ReadOnlyCollectionConcator<T> WrapMultiplySections<T>(ResourceConfig[] configs, Func<ResourceConfig, T[]> selector)
        {
            return new ReadOnlyCollectionConcator<T>(configs.Select(selector).ToList());
        }

        static IReadOnlyDictionary<string, T> WrapMultiplySections<T>(IEnumerable<ResourceConfig> configs, Func<ResourceConfig, IReadOnlyDictionary<string, T>> selector)
        {
            var capacity = (from c in configs
                            select selector(c)?.Count).Sum() ?? 1;
            var dict = new Dictionary<string, T>(capacity);
            foreach (var config in configs)
                selector(config)?.Sink(p => dict.Add(p.Key, p.Value));
            return dict;
        }

        class ReadOnlyCollectionConcator<T> : IReadOnlyCollection<T>
        {
            public int Count => _count;

            public IEnumerator<T> GetEnumerator()
            {
                return EnumAll().GetEnumerator();
            }

            IEnumerator IEnumerable.GetEnumerator()
            {
                return EnumAll().GetEnumerator();
            }

            private readonly IReadOnlyCollection<IReadOnlyCollection<T>> _collections;
            private readonly int _count;
            public ReadOnlyCollectionConcator(IReadOnlyCollection<IReadOnlyCollection<T>> collections)
            {
                _collections = collections;
                _count = collections.Where(o => o != null).Sum(o => o.Count);
            }

            private IEnumerable<T> EnumAll()
            {
                foreach (var col in _collections.Where(o => o != null))
                {
                    foreach (var item in col)
                    {
                        yield return item;
                    }
                }
            }
        }
    }
}
