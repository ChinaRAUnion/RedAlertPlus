﻿using ChinaRAUnion.RedAlertPlus.Primitives;
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
    class ResourceConfig
    {
        public FrameAnimationImageResource[] FrameAnimationImages { get; set; }
        public InputStreamWithContentTypeResource[] ThemeAudios { get; set; }
        public InputStreamWithContentTypeResource[] Shaders { get; set; }
    }

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
