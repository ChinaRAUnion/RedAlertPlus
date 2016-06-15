using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ChinaRAUnion.RedAlertPlus.Primitives
{
    public class UnitArt : IUnitArt
    {
        public bool Remapable { get; set; }

        public string Sprite { get; set; }

        public IReadOnlyList<WeaponFireOffset> WeaponFireOffsets { get; set; } = new WeaponFireOffset[] { };

        public string Cameo { get; set; }
    }
}
