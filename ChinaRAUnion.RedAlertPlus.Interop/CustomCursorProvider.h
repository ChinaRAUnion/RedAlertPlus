//
// Red Alert Plus
// CustomCursorProvider
// ���ߣ�SunnyCase
// ����ʱ�䣺2016-06-01
//
#pragma once
#include "../include/common.h"

DEFINE_NS_RAP

public enum class CustomCursors
{
	NoDrop
};

public ref class CustomCursorProvider sealed
{
public:
	Windows::UI::Core::CoreCursor^ GetCursor(CustomCursors cursor);
};

END_NS_RAP