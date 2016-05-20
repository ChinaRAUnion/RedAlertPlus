using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tomato.Mvvm;

namespace ChinaRAUnion.RedAlertPlus.Primitives
{
    public class ResourceEntry<TDefination, TResult> : BindableBase where TDefination : ResourceBase
    {
        public int IsLoading;

        public TDefination Resource { get; set; }

        private TResult _value;
        public TResult Value
        {
            get { return _value; }
            set { SetProperty(ref _value, value); }
        }
    }
}
