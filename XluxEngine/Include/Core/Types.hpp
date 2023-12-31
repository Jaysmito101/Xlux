#pragma once

#include <string>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <unordered_map>
#include <memory>
#include <functional>
#include <array>
#include <cstdint>

#if defined(_WIN32) || defined(_WIN64)
	#if defined(XLUX_DLL_EXPORT)
		#define XLUX_API __declspec(dllexport)
	#elif defined(KLUX_DLL_IMPORT)
		#define XLUX_API __declspec(dllimport)
	#else
		#define XLUX_API
	#endif
#else
	#define XLUX_API 
#endif


#define XLUX_SAFE_DELETE(x) if(x) { delete x; x = nullptr; 

#ifdef _MSC_VER
	#define XLUX_FORCE_INLINE __forceinline
#else
	#define XLUX_FORCE_INLINE __attribute__((always_inline))
#endif

#define EPSILON 0.000001f

namespace xlux 
{

	using U8 = uint8_t;
	using U16 = uint16_t;
	using U32 = uint32_t;
	using U64 = uint64_t;

	using I8 = int8_t;
	using I16 = int16_t;
	using I32 = int32_t;
	using I64 = int64_t;

	using Size = size_t;

	using Bool = bool;

	using F32 = float;
	using F64 = double;

	using ValueType = F32;

	using String = std::string;
	using WString = std::wstring;
	using StringView = std::string_view;
	using WStringView = std::wstring_view;

	using CString = const char*;
	using CStringList = std::vector<CString>;

	template<typename T>
	using List = std::vector<T>;

	template <typename T>
	using Queue = std::queue<T>;

	template<typename Key, typename Value>
	using Map = std::map<Key, Value>;

	template<typename Key, typename Value>
	using UnorderedMap = std::unordered_map<Key, Value>;

	template<typename T>
	using Set = std::set<T>;

	template <typename T, int N>
	using Array = std::array<T, N>;

	template<typename A, typename B>
	struct Pair 
	{
	public:
		A x;
		B y;

		Pair() : x(0), y(0) {}
		Pair(A x, B y) : x(x), y(y) {}
	};

	template<typename A, typename B>
	inline Pair<A, B> MakePair(A x, B y)
	{
		return Pair<A, B>(x, y);
	}
	

	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T>
	using WeakRef = std::weak_ptr<T>;

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using RawPtr = T*;

	template<typename T, typename... Args>
	inline Ref<T> CreateRef(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	inline Scope<T> CreateScope(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	inline RawPtr<T> CreateRawPtr(Args&&... args)
	{
		return new T(std::forward<Args>(args)...);
	}

	template<typename Target, typename Source>
	inline Ref<Target> CastRef(const Ref<Source>& source)
	{
		return std::static_pointer_cast<Target>(source);
	}

}