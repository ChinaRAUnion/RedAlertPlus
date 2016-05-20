using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ChinaRAUnion.RedAlertPlus.Primitives
{
    public abstract class ResourceBase
    {
        public string Name { get; set; }
        public Uri Source { get; set; }
        public bool IsDefer { get; set; }
    }
}
