using ChinaRAUnion.RedAlertPlus.Primitives;
using ChinaRAUnion.RedAlertPlus.Resource.Models;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Dynamic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Tomato.Mvvm;

namespace ChinaRAUnion.RedAlertPlus.Resource
{
    abstract class ResourceMap<TDefination, TResult> : IResourceMap<TDefination, TResult>, INotifyPropertyChanged where TDefination : ResourceBase
    {
        public event PropertyChangedEventHandler PropertyChanged;
        private readonly IReadOnlyDictionary<string, ResourceEntry<TDefination, TResult>> _entries;

        public ResourceMap(IReadOnlyCollection<TDefination> resources)
        {
            var entries = new Dictionary<string, ResourceEntry<TDefination, TResult>>(resources.Count);
            foreach (var res in resources)
            {
                entries.Add(res.Name, new ResourceEntry<TDefination, TResult>
                {
                    Resource = res
                });
            }
            _entries = entries;
        }

        public Task LoadAsync()
        {
            var tasks = (from r in _entries
                         where !r.Value.Resource.IsDefer
                         select LoadResourceAsync(r.Value, notify: false));
            return Task.WhenAll(tasks);
        }

        private async Task LoadResourceAsync(ResourceEntry<TDefination, TResult> r, bool notify)
        {
            r.Value = await LoadResourceCoreAsync(r.Resource.Source);
            if (notify)
                PropertyChanged(this, new PropertyChangedEventArgs(r.Resource.Name));
        }

        protected abstract Task<TResult> LoadResourceCoreAsync(Uri source);

        public ResourceEntry<TDefination, TResult> FindResource(string key)
        {
            ResourceEntry<TDefination, TResult> entry;
            if (_entries.TryGetValue(key, out entry))
            {
                if (entry.Resource.IsDefer)
                {
                    if (Interlocked.CompareExchange(ref entry.IsLoading, 1, 0) == 0)
                        LoadResourceAsync(entry, true);
                }
                return entry;
            }
            return null;
        }
    }
}
