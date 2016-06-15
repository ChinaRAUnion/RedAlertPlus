using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ChinaRAUnion.RedAlertPlus.Primitives
{
    public class ResourceConfig
    {
        public FrameAnimationImageResource[] FrameAnimationImages { get; set; }
        public InputStreamWithContentTypeResource[] ThemeAudios { get; set; }
        public InputStreamWithContentTypeResource[] Shaders { get; set; }
        public InputStreamWithContentTypeResource[] Maps { get; set; }
        public InputStreamWithContentTypeResource[] TileSets { get; set; }
        public InputStreamWithContentTypeResource[] Sprites { get; set; }
        public Dictionary<string, UnitArt> UnitArts { get; set; }
    }
}
