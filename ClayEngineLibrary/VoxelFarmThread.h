#pragma once

#include "ClayEngine.h"

#include <future>
#include <thread>

_CE_BEGIN
struct voxelfarm_thread_func
{
	void operator()(std::future<void> future);
private:
};

class VoxelFarmThread
{
	std::thread t;
	std::promise<void> p{};
public:
	VoxelFarmThread();
	~VoxelFarmThread();
};
using VoxelFarmThreadPtr = std::unique_ptr<ClayEngine::VoxelFarmThread>;
_CE_END
