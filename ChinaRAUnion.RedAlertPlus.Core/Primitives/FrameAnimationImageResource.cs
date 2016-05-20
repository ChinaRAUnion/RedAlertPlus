using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation;

namespace ChinaRAUnion.RedAlertPlus.Primitives
{
    public class FrameAnimationImageResource : ResourceBase
    {
        [JsonConverter(typeof(JsonSizeConverter))]
        public Size FrameSize { get; set; }
        public int FramesCount { get; set; }
        public int? ColumnsCount { get; set; }
        public bool IsLooping { get; set; }
    }
}
