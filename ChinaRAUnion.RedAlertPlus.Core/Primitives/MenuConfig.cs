using Caliburn.Micro;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tomato.Mvvm;
using Windows.ApplicationModel.Resources;

namespace ChinaRAUnion.RedAlertPlus.Primitives
{
    public sealed class MenuConfig
    {
        public string TitleName { get; set; }
        public string Title => IoC.Get<ResourceLoader>().GetString($"{TitleName}/Text");
        public List<MenuItem> TopButtons { get; set; }
        public List<MenuItem> BottomButtons { get; set; }
    }

    public sealed class MenuItem : BindableBase
    {
        public string Name { get; set; }

        private bool _isEnabled = true;
        public bool IsEnabled
        {
            get { return _isEnabled; }
            set { SetProperty(ref _isEnabled, value); }
        }

        public string Text => IoC.Get<ResourceLoader>().GetString($"{Name}/Text");

        private Action _invoker;
        public MenuItem()
        {

        }

        public MenuItem(Action invoker)
        {
            _invoker = invoker;
        }

        public MenuItem(Action<IEventAggregator> invoker)
        {
            _invoker = () =>
            {
                var aggregator = IoC.Get<IEventAggregator>();
                invoker(aggregator);
            };
        }

        public void Invoke()
        {
            _invoker?.Invoke();
        }
    }
}
