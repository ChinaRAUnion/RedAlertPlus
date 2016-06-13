using ChinaRAUnion.RedAlertPlus.Rules;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ChinaRAUnion.RedAlertPlus.Api.Rules;
using ChinaRAUnion.RedAlertPlus.Primitives;
using Windows.Storage;
using Newtonsoft.Json;

namespace ChinaRAUnion.RedAlertPlus.Resource
{
    class RulesLoader : IRulesLoader
    {
        public IReadOnlyDictionary<string, Infantry> Infantry { get; private set; }

        private readonly Uri[] _configFiles;
        public RulesLoader(Uri[] configFiles)
        {
            _configFiles = configFiles;
        }

        public async Task InitializeAsync()
        {
            var configs = await Task.WhenAll(_configFiles.Select(LoadConfigFile));

            Infantry = ConcatRules(configs, o => o.Infantry);
        }

        private async Task<RulesConfig> LoadConfigFile(Uri uri)
        {
            var file = await (uri.IsFile ? StorageFile.GetFileFromPathAsync(uri.LocalPath) : StorageFile.GetFileFromApplicationUriAsync(uri));
            return JsonConvert.DeserializeObject<RulesConfig>(await FileIO.ReadTextAsync(file));
        }

        public IReadOnlyDictionary<string, T> ConcatRules<T>(IEnumerable<RulesConfig> configs, Func<RulesConfig, IReadOnlyDictionary<string, T>> selector) where T : IHasName
        {
            var capacity = (from c in configs
                            select selector(c)?.Count).Sum() ?? 1;
            var dict = new Dictionary<string, T>(capacity);
            foreach (var config in configs)
                selector(config)?.Sink(p =>
                {
                    p.Value.Name = p.Key;
                    dict.Add(p.Key, p.Value);
                });
            return dict;
        }
    }
}
