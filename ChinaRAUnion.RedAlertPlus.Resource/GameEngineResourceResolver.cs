using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
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

        public IRandomAccessStream ResolveShader(string name)
        {
            throw new NotImplementedException();
        }

        public IRandomAccessStreamWithContentType ResolveTexture(string name)
        {
            throw new NotImplementedException();
        }

        public ITileSetPackageContent ResolveTileSetPackageFile(string name)
        {
            throw new NotImplementedException();
        }

        class TileSetPackageContent : ITileSetPackageContent
        {
            public IRandomAccessStream ExtraImage { get; set; }

            public IRandomAccessStream Image { get; set; }

            public IRandomAccessStream TileSet { get; set; }
        }
    }
}
