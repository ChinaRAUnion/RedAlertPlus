using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ChinaRAUnion.RedAlertPlus.FileFormat
{
    public class Property
    {
        [JsonProperty("name")]
        public string Name { get; set; }

        [JsonProperty("value")]
        public string Value { get; set; }

        public override string ToString()
        {
            return $"{Name}: {Value}";
        }
    }

    public struct Offset
    {
        [JsonProperty("x")]
        public int X { get; set; }

        [JsonProperty("y")]
        public int Y { get; set; }

        public override string ToString()
        {
            return $"{X}, {Y}";
        }
    }

    public struct TileUnitSize
    {
        [JsonProperty("xlength")]
        [Range(1, int.MaxValue)]
        public int XLength { get; set; }

        [JsonProperty("zlength")]
        [Range(1, int.MaxValue)]
        public int ZLength { get; set; }

        [JsonIgnore]
        public int Count => XLength * ZLength;

        public override string ToString()
        {
            return $"{XLength}x{ZLength}";
        }
    }

    public class Terrain
    {
        [JsonProperty("name")]
        public string Name { get; set; }

        [JsonProperty("tile")]
        public int Tile { get; set; }

        public override string ToString()
        {
            return $"{Name}: {Tile}";
        }
    }

    [JsonConverter(typeof(TerrainCornerJsonConverter))]
    public struct TerrainCorner
    {
        public int TopLeft { get; set; }
        public int TopRight { get; set; }
        public int BottomLeft { get; set; }
        public int BottomRight { get; set; }

        public override string ToString()
        {
            return JsonConvert.SerializeObject(this).ToString();
        }
    }

    public struct ExtraImageRef
    {
        [JsonProperty("extraimage")]
        public int ExtraImage { get; set; }

        [JsonProperty("offset")]
        public Offset Offset { get; set; }
    }

    public class Tile
    {
        [JsonProperty("terrain")]
        public TerrainCorner? Terrain { get; set; }

        [JsonProperty("extraimage")]
        public ExtraImageRef? ExtraImage { get; set; }
    }

    public class ExtraImage
    {
        [JsonProperty("x")]
        public int X { get; set; }

        [JsonProperty("y")]
        public int Y { get; set; }

        [JsonProperty("width")]
        public int Width { get; set; }

        [JsonProperty("height")]
        public int Height { get; set; }
    }

    class TerrainCornerJsonConverter : JsonConverter
    {
        public override bool CanConvert(Type objectType)
        {
            return objectType == typeof(TerrainCorner);
        }

        public override object ReadJson(JsonReader reader, Type objectType, object existingValue, JsonSerializer serializer)
        {
            var corner = new TerrainCorner();
            if (reader.TokenType != JsonToken.StartArray) return new TerrainCorner?();
            corner.TopLeft = reader.ReadAsInt32() ?? 0;
            corner.TopRight = reader.ReadAsInt32() ?? 0;
            corner.BottomLeft = reader.ReadAsInt32() ?? 0;
            corner.BottomRight = reader.ReadAsInt32() ?? 0;
            reader.Read();
            if (reader.TokenType != JsonToken.EndArray) return new TerrainCorner?();
            return corner;
        }

        public override void WriteJson(JsonWriter writer, object value, JsonSerializer serializer)
        {
            var corner = (TerrainCorner)value;
            writer.WriteStartArray();
            writer.WriteValue(corner.TopLeft);
            writer.WriteValue(corner.TopRight);
            writer.WriteValue(corner.BottomLeft);
            writer.WriteValue(corner.BottomRight);
            writer.WriteEndArray();
        }
    }

    public class TileUnitElement
    {
        [JsonProperty("tile")]
        public int Tile { get; set; }

        [JsonProperty("height")]
        public int Height { get; set; }
    }

    public class TileUnit
    {
        [JsonProperty("size")]
        public TileUnitSize Size { get; set; }

        [JsonProperty("tiles")]
        public TileUnitElement[] Tiles { get; set; }

        [JsonProperty("category")]
        [Required]
        public string Category { get; set; }

        public TileUnit()
        {
            Tiles = new TileUnitElement[0];
        }

        public TileUnit(TileUnitSize size)
        {
            Size = size;
            Tiles = new TileUnitElement[size.Count];
        }
    }

    public class PickAnyTileUnit
    {
        [JsonProperty("tiles")]
        public List<int> Tiles { get; set; } = new List<int>();

        [JsonProperty("category")]
        [Required]
        public string Category { get; set; }

        public PickAnyTileUnit()
        {

        }
    }

    /// <summary>
    /// Tile 图块
    /// </summary>
    public class TileSet
    {
        [JsonProperty("name")]
        public string Name { get; set; }

        [JsonProperty("tilewidth")]
        public int TileWidth { get; set; }

        [JsonProperty("tileheight")]
        public int TileHeight { get; set; }

        [JsonProperty("terrains")]
        public List<Terrain> Terrains { get; set; } = new List<Terrain>();

        [JsonProperty("tiles")]
        public List<Tile> Tiles { get; set; }

        [JsonProperty("extraimages")]
        public List<ExtraImage> ExtraImages { get; set; }

        [JsonProperty("tileunits")]
        public List<TileUnit> TileUnits { get; set; } = new List<TileUnit>();

        [JsonProperty("pickanytileunits")]
        public List<PickAnyTileUnit> PickAnyTileUnits { get; set; } = new List<PickAnyTileUnit>();

        public TileSet()
        {
        }

        
        public const string TileSetJsonFileName = "tileSet.json";
        public const string ImageFileName = "image.dds";
        public const string ExtraImageFileName = "extraImage.dds";
    }
}
