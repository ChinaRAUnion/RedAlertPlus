#include "stdafx.h"
#include "Shp.h"

namespace
{
#pragma pack(push, 1)
	struct SHPHeader
	{
		WORD a;
		WORD width, height;
		WORD frameCount;
	};

	struct ShpFrameHeader
	{
		WORD OffsetX;
		WORD OffsetY;
		WORD CroppedWidth;
		WORD CroppedHeight;
		uint32_t CompressionType;
		argb RadarColor;
		uint32_t Zeros;
		uint32_t DataOffset;
	};
#pragma pack(pop)

	void DecodeSrcIntoDest(const byte* src, size_t length, byte* dst)
	{
		const auto end = src + length;
		while (src != end)
		{
			auto value = *src++;
			if (value == 0)
			{
				auto count = *src++;
				while (count--)
					*dst++ = 0;
			}
			else
				*dst++ = value;
		}
	}

	ShpFrame LoadFrame(std::istream& stream, const ShpFrameHeader& header)
	{
		ShpFrame frame;
		frame.OffsetX = header.OffsetX;
		frame.OffsetY = header.OffsetY;
		frame.CroppedWidth = header.CroppedWidth;
		frame.CroppedHeight = header.CroppedHeight;
		frame.RadarColor = header.RadarColor;
		frame.RawData.resize(frame.CroppedWidth * frame.CroppedHeight);

		stream.seekg(header.DataOffset, std::ios::beg);
		if (header.CompressionType == 3)
		{
			// RLE-zero compressed scanlines
			for (size_t y = 0; y < frame.CroppedHeight; y++)
			{
				WORD len;
				stream.read(reinterpret_cast<char*>(&len), sizeof(len));
				len -= 2;
				auto data = std::make_unique<byte[]>(len);
				stream.read(reinterpret_cast<char*>(data.get()), len);
				DecodeSrcIntoDest(data.get(), len, frame.RawData.data() + frame.CroppedWidth * y);
			}
		}
		else
		{
			WORD len;
			if (header.CompressionType == 2)
			{
				stream.read(reinterpret_cast<char*>(&len), sizeof(len));
				len -= 2;
			}
			else
				len = frame.CroppedWidth;
			for (size_t y = 0; y < frame.CroppedHeight; y++)
				stream.read(reinterpret_cast<char*>(frame.RawData.data()) + frame.CroppedWidth * y, len);
		}
		return frame;
	}
}

void Shp::Load(std::istream& stream)
{
	SHPHeader header;
	stream.read(reinterpret_cast<char*>(&header), sizeof(header));
	_frameWidth = header.width;
	_frameHeight = header.height;
	_frameCount = header.frameCount;

	std::vector<ShpFrameHeader> frameHeaders(_frameCount);
	for (auto& frameHeader : frameHeaders)
		stream.read(reinterpret_cast<char*>(&frameHeader), sizeof(frameHeader));

	_frames.reserve(_frameCount);
	for (size_t i = 0; i < _frameCount; i++)
		_frames.emplace_back(LoadFrame(stream, frameHeaders[i]));
}