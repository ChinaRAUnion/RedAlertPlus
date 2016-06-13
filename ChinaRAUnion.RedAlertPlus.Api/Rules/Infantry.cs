using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace ChinaRAUnion.RedAlertPlus.Api.Rules
{
    [ComVisible(true)]
    [Guid("5AA28C1D-6066-45DC-A01D-7F773B9CB91F")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IHasName
    {
        string Name { get; set; }
    }

    [ComVisible(true)]
    [Guid("C9A331C3-4F37-424B-A3A7-20993FD92AC8")]
    public interface IInfantry : IHasName
    {
        float MaxStrength { get; set; }
    }

    [ComVisible(true)]
    [Guid("7C8B0CD4-0F22-4398-9B82-937BBAB12055")]
    [ComDefaultInterface(typeof(IInfantry))]
    [ClassInterface(ClassInterfaceType.None)]
    public class Infantry : IInfantry
    {
        public string Name { get; set; }
        public float MaxStrength { get; set; }
    }
}
