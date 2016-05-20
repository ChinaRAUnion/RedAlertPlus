using ChinaRAUnion.RedAlertPlus.Resource;
using ChinaRAUnion.RedAlertPlus.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;

namespace ChinaRAUnion.RedAlertPlus.Audio.Services
{
    public class ThemeService : IThemeService
    {
        private readonly MediaElement _mediaElement = new MediaElement();
        private const string IntroResourceName = "Intro";
        private readonly IResourceManager _resourceManager;
        private string _currentThemeName;

        public ThemeService(IResourceManager resourceManager)
        {
            _resourceManager = resourceManager;

            _mediaElement.AudioCategory = AudioCategory.GameMedia;
            _mediaElement.AudioDeviceType = AudioDeviceType.Multimedia;
            _mediaElement.IsLooping = true;
            _mediaElement.AutoPlay = true;
        }

        public void PlayIntro()
        {
            Play(IntroResourceName);
        }

        private void Play(string themeName)
        {
            if(_currentThemeName != themeName)
            {
                var res = _resourceManager.ThemeAudios.FindResource(themeName);
                _mediaElement.SetSource(res.Value, res.Resource.ContentType);
                _currentThemeName = themeName;
            }
            _mediaElement.Play();
        }
    }
}
