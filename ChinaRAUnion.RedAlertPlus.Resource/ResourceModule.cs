using Caliburn.Micro;
using ChinaRAUnion.RedAlertPlus.Resource;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.ApplicationModel.Resources;

namespace ChinaRAUnion.RedAlertPlus
{
    public static class ResourceModule
    {
        public static ResourceModuleConfig UseResource(this SimpleContainer container)
        {
            return new ResourceModuleConfig(container);
        }
    }

    public class ResourceModuleConfig
    {
        private readonly SimpleContainer _container;

        internal ResourceModuleConfig(SimpleContainer container)
        {
            _container = container;
        }

        public void UseResourceManager(params Uri[] resourceConfigFiles)
        {
            _container.Instance<IResourceManager>(new ResourceManager(resourceConfigFiles));
        }
    }
}
