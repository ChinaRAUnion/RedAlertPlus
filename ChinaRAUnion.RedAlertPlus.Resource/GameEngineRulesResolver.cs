using ChinaRAUnion.RedAlertPlus.Rules;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections;
using ChinaRAUnion.RedAlertPlus.Api.Rules;

namespace ChinaRAUnion.RedAlertPlus.Resource
{
    class GameEngineRulesResolver : IGameEngineRulesResolver
    {
        public IReadOnlyDictionary<string, object> Infantry { get; }

        private readonly IRulesLoader _rulesLoader;
        public GameEngineRulesResolver(IRulesLoader rulesLoader)
        {
            _rulesLoader = rulesLoader;

            Infantry = new ReadOnlyDictionaryBoxer<string, Infantry>(rulesLoader.Infantry);
        }

        class ReadOnlyDictionaryBoxer<TKey, TValue> : IReadOnlyDictionary<TKey, object> where TValue : class
        {
            private readonly IReadOnlyDictionary<TKey, TValue> _dict;
            public ReadOnlyDictionaryBoxer(IReadOnlyDictionary<TKey, TValue> dict)
            {
                _dict = dict;
            }

            public object this[TKey key] => _dict[key];

            public int Count => _dict.Count;

            public IEnumerable<TKey> Keys => _dict.Keys;

            public IEnumerable<object> Values => _dict.Values;

            public bool ContainsKey(TKey key)
            {
                return _dict.ContainsKey(key);
            }

            public IEnumerator<KeyValuePair<TKey, object>> GetEnumerator()
            {
                foreach (var item in _dict)
                    yield return new KeyValuePair<TKey, object>(item.Key, item.Value);
            }

            public bool TryGetValue(TKey key, out object value)
            {
                TValue innerValue;
                var result = _dict.TryGetValue(key, out innerValue);
                value = innerValue;
                return result;
            }

            IEnumerator IEnumerable.GetEnumerator()
            {
                return _dict.GetEnumerator();
            }
        }
    }
}
