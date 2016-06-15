using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ChinaRAUnion.RedAlertPlus.Primitives;
using Windows.Storage.Streams;

namespace ChinaRAUnion.RedAlertPlus.Resource
{
    class GameEngineResourceResolver : IGameEngineResourceResolver
    {
        private readonly IResourceManager _resourceManager;

        public GameEngineResourceResolver(IResourceManager resourceManager)
        {
            _resourceManager = resourceManager;
        }

        public IRandomAccessStream ResolveMap(string name)
        {
            return _resourceManager.Maps.FindResource(name)?.Value;
        }

        public IRandomAccessStream ResolveShader(string name)
        {
            return _resourceManager.Shaders.FindResource(name)?.Value;
        }

        public ISpritePackageContent ResolveSpritePackageFile(string name)
        {
            return _resourceManager.Sprites.FindResource(name)?.Value;
        }

        public IRandomAccessStreamWithContentType ResolveTexture(string name)
        {
            throw new NotImplementedException();
        }

        public ITileSetPackageContent ResolveTileSetPackageFile(string name)
        {
            return _resourceManager.TileSets.FindResource(name)?.Value;
        }

        public IUnitArt ResolveUnitArt(string name)
        {
            return _resourceManager.UnitArts[name];
        }
    }
}
