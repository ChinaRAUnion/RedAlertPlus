using System;
using Newtonsoft.Json;
using Windows.Foundation;

namespace ChinaRAUnion.RedAlertPlus.Primitives
{
    internal class JsonSizeConverter : JsonConverter
    {
        public override bool CanConvert(Type objectType)
        {
            return objectType == typeof(Size);
        }

        public override object ReadJson(JsonReader reader, Type objectType, object existingValue, JsonSerializer serializer)
        {
            var text = ((string)reader.Value).Split(',');
            return new Size(double.Parse(text[0].Trim()), double.Parse(text[1].Trim()));
        }

        public override void WriteJson(JsonWriter writer, object value, JsonSerializer serializer)
        {
            var size = (Size)value;
            writer.WriteValue($"{size.Width}, {size.Height}");
        }
    }
}