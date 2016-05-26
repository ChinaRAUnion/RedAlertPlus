//
// Red Alert Plus Engine
// 设备依赖资源容器
// 作者：SunnyCase
// 创建时间：2016-05-26
//
#pragma once
#include "../include/engine/engine.h"
#include <ppltasks.h>
#include <d3d12.h>

DEFINE_NS_ENGINE

interface IDeviceDependentResourcesContainer
{
	virtual concurrency::task<void> CreateDeviceDependentResources(IResourceResovler* resourceResolver) = 0;
	virtual void UploadGpuResource(std::vector<WRL::ComPtr<IUnknown>>& resourcesWaitForUpload) = 0;
};


END_NS_ENGINE