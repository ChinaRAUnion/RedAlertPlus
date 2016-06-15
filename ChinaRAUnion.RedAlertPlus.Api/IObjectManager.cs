using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace ChinaRAUnion.RedAlertPlus.Api
{
    [ComVisible(true)]
    [Guid("79F19D7E-169A-4FA2-AECD-C4A9C0D8A826")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IObjectManager
    {
        bool PlaceInfantry([MarshalAs(UnmanagedType.BStr)]string name, [MarshalAs(UnmanagedType.U4)]uint x, [MarshalAs(UnmanagedType.U4)]uint y);
    }
}
