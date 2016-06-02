using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ChinaRAUnion.RedAlertPlus.Services
{
    public enum CursorType
    {
        None,
        Normal,
        
        NoDrop
    }

    public interface IInputService
    {
        void SetCursor(CursorType cursor);
    }
}
