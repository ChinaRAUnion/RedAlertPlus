using ChinaRAUnion.RedAlertPlus.Api.Rules;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ChinaRAUnion.RedAlertPlus.Rules
{
    public interface IRulesLoader
    {
        IReadOnlyDictionary<string, Infantry> Infantry { get; }

        Task InitializeAsync();
    }
}
