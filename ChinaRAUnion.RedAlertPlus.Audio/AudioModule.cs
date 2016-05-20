using Caliburn.Micro;
using ChinaRAUnion.RedAlertPlus.Audio.Services;
using ChinaRAUnion.RedAlertPlus.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.ApplicationModel.Resources;

namespace ChinaRAUnion.RedAlertPlus
{
    public static class AudioModule
    {
        public static void UseAudio(this SimpleContainer container)
        {
            container.Singleton<IThemeService, ThemeService>();
        }
    }
}
