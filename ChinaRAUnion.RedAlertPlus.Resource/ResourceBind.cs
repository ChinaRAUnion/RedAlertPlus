using Caliburn.Micro;
using ChinaRAUnion.RedAlertPlus.Resource.Models;
using Microsoft.Graphics.Canvas;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tomato.Media.Gaming.Controls;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Data;

namespace ChinaRAUnion.RedAlertPlus.Resource
{
    public static class ResourceBind
    {
        public static DependencyProperty FrameAnimationImageProperty { get; } = DependencyProperty.RegisterAttached("FrameAnimationImage",
            typeof(string), typeof(ResourceBind), new PropertyMetadata(null, OnFrameAnimationImagePropertyChanged));

        public static void SetFrameAnimationImage(DependencyObject d, string value)
        {
            d.SetValue(FrameAnimationImageProperty, value);
        }

        public static string GetFrameAnimationImage(DependencyObject d)
        {
            return (string)d.GetValue(FrameAnimationImageProperty);
        }

        private static void OnFrameAnimationImagePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (Execute.InDesignMode) return;
            var target = d as FrameAnimationImage;
            var key = e.NewValue as string ?? string.Empty;
            var entry = IoC.Get<IResourceManager>().FrameAnimationImages.FindResource(key);
            if (target != null)
            {
                target.SetBinding(FrameAnimationImage.SourceProperty, new Binding { Path = new PropertyPath("Value"), Source = entry });
                target.SetBinding(FrameAnimationImage.FramesCountProperty, new Binding { Path = new PropertyPath("Resource.FramesCount"), Source = entry });
                target.SetBinding(FrameAnimationImage.FrameSizeProperty, new Binding { Path = new PropertyPath("Resource.FrameSize"), Source = entry });
                target.SetBinding(FrameAnimationImage.ColumnsCountProperty, new Binding { Path = new PropertyPath("Resource.ColumnsCount"), Source = entry });
                target.SetBinding(FrameAnimationImage.IsLoopingProperty, new Binding { Path = new PropertyPath("Resource.IsLooping"), Source = entry });
                target.SetBinding(FrameAnimationImage.WidthProperty, new Binding { Path = new PropertyPath("Resource.FrameSize.Width"), Source = entry });
                target.SetBinding(FrameAnimationImage.HeightProperty, new Binding { Path = new PropertyPath("Resource.FrameSize.Height"), Source = entry });
            }
        }
    }
}
