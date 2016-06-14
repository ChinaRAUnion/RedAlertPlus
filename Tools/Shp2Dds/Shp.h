#pragma once
#include <iostream>
#include <vector>
#include <memory>

union argb
{
	uint32_t value;
	struct
	{
		byte a, r, g, b;
	};
};

struct ShpFrame
{
	uint32_t OffsetX;
	uint32_t OffsetY;
	uint32_t CroppedWidth;
	uint32_t CroppedHeight;
	argb RadarColor;

	std::vector<byte> RawData;
};

class Shp
{
public:
	Shp(){}

	void Load(std::istream& stream);

	uint32_t GetFrameWidth() const noexcept { return _frameWidth; }
	uint32_t GetFrameHeight() const noexcept { return _frameHeight; }
	uint32_t GetFrameCount() const noexcept { return _frameCount; }
	const std::vector<ShpFrame>& GetFrames() const noexcept { return _frames; }
private:
	uint32_t _frameWidth;
	uint32_t _frameHeight;
	uint32_t _frameCount;
	std::vector<ShpFrame> _frames;
};