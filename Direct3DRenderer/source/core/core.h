#pragma once
#include <memory>
#include <filesystem>
#include <wrl.h>

namespace wrl = Microsoft::WRL;

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}