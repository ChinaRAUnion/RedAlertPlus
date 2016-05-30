using ChinaRAUnion.RedAlertPlus;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Storage.Streams;

namespace ChinaRAUnion.RedAlertPlus.Primitives
{
    public class TileSetContent : ITileSetPackageContent
    {
        public IInputStream TileSet { get; set; }
        public IInputStream Image { get; set; }
        public IInputStream ExtraImage { get; set; }
    }
}
