//
// Red Alert Plus Engine
// 通用头文件
// 作者：SunnyCase
// 创建时间：2016-05-25
//
#pragma once
#include "../common.h"

#define DEFINE_NS_ENGINE namespace ChinaRAUnion { namespace RedAlertPlus { namespace Engine {
#define END_NS_ENGINE }}}
#define NS_ENGINE ChinaRAUnion::RedAlertPlus::Engine

#ifdef CRAU_ENGINE_EXPORTS
#define CRAU_ENGINE_API __declspec(dllexport)
#else
#define CRAU_ENGINE_API __declspec(dllimport)
#endif