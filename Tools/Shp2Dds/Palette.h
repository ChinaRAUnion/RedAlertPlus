#pragma once
#include <iostream>
#include <array>

union rgb
{
	uint32_t value = 0;
	struct
	{
		unsigned char r, g, b;
	};
};

class Palette
{
public:
	Palette()
	{

	}

	void Load(std::istream& stream)
	{
		for (auto& color : _colors)
		{
			stream.read(reinterpret_cast<char*>(&color), 3);
			color.r *= 4;
			color.g *= 4;
			color.b *= 4;
		}
	}

	const std::array<rgb, 256>& GetColors() const noexcept { return _colors; }
private:
	std::array<rgb, 256> _colors;
};