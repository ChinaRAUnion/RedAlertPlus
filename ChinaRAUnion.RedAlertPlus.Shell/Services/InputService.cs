using ChinaRAUnion.RedAlertPlus.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.UI.Core;

namespace ChinaRAUnion.RedAlertPlus.Shell.Services
{
    class InputService : IInputService
    {
        private readonly CoreWindow _coreWindow;
        private readonly CustomCursorProvider _customCursorProvider = new CustomCursorProvider();

        public InputService()
        {
            _coreWindow = CoreWindow.GetForCurrentThread();
        }

        public void SetCursor(CursorType cursor)
        {
            switch (cursor)
            {
                case CursorType.None:
                    _coreWindow.PointerCursor = null;
                    break;
                case CursorType.Normal:
                    _coreWindow.PointerCursor = new CoreCursor(CoreCursorType.Arrow, 0);
                    break;
                case CursorType.NoDrop:
                    _coreWindow.PointerCursor = _customCursorProvider.GetCursor(CustomCursors.NoDrop);
                    break;
                default:
                    break;
            }
        }
    }
}
