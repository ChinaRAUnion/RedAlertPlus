using ChinaRAUnion.RedAlertPlus.Primitives;
using Microsoft.Graphics.Canvas;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation;

namespace ChinaRAUnion.RedAlertPlus.Resource.Models
{
    class FrameAnimationImageResourceMap : ResourceMap<FrameAnimationImageResource, CanvasBitmap>
    {
        public FrameAnimationImageResourceMap(IReadOnlyCollection<FrameAnimationImageResource> resources) : base(resources)
        {
        }

        protected override async Task<CanvasBitmap> LoadResourceCoreAsync(Uri source)
        {
            return await CanvasBitmap.LoadAsync(CanvasDevice.GetSharedDevice(), source);
        }
    }
}
