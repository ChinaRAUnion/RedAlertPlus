//
// Red Alert Plus Engine
// IRenderable
// 作者：SunnyCase
// 创建时间：2016-05-27
//
#pragma once
#include "../include/engine/engine.h"

DEFINE_NS_ENGINE

interface IRenderable
{
	virtual void Update() = 0;
	virtual void Render() = 0;
};

END_NS_ENGINE