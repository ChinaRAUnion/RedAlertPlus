using Caliburn.Micro;
using ChinaRAUnion.RedAlertPlus.Resource;
using ChinaRAUnion.RedAlertPlus.Rules;
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

        public ResourceModuleConfig UseResourceManager(params Uri[] resourceConfigFiles)
        {
            _container.Instance<IResourceManager>(new ResourceManager(resourceConfigFiles));
            _container.Singleton<IGameEngineResourceResolver, GameEngineResourceResolver>();
            return this;
        }

        public ResourceModuleConfig UseRulesLoader(params Uri[] resourceConfigFiles)
        {
            _container.Instance<IRulesLoader>(new RulesLoader(resourceConfigFiles));
            _container.Singleton<IGameEngineRulesResolver, GameEngineRulesResolver>();
            return this;
        }
    }
}
