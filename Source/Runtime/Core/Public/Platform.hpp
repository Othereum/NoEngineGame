#pragma once
#include "Core.hpp"
#include "Templates/Pointer.hpp"
#include <bitset>
#include <filesystem>
#include <memory>
#include <string>

namespace oeng
{
inline namespace core
{

namespace fs = std::filesystem;
using namespace std::literals;

namespace detail
{
#ifndef NDEBUG
[[nodiscard]] CORE_API bool IsDebuggingImpl() noexcept;
#endif
} // namespace detail

/**
 * Get user data path.
 * - On debug/development/test build: "../Saved"
 * - On Windows shipping build: "C:/Users/{user}/Documents/{game}" by default
 * - On Unix shipping build: "/home/{user}/.{game}"
 */
[[nodiscard]] CORE_API fs::path GetUserDataPath();

/**
 * Check if the debugger is attached.
 * If NDEBUG is defined, always returns false.
 * @return True if debugger is attached.
 */
[[nodiscard]] inline bool IsDebugging() noexcept
{
#ifdef NDEBUG
    return false;
#else
    return detail::IsDebuggingImpl();
#endif
}

class CORE_API Dll
{
  public:
    explicit Dll(std::u8string filepath);

    [[nodiscard]] void* GetSymbol(std::u8string_view name) const;
    [[nodiscard]] void* FindSymbol(std::u8string_view name) const noexcept;

    template <class T>[[nodiscard]] T& GetSymbol(std::u8string_view name) const
    {
        return *(T*)GetSymbol(name);
    }

    template <class Fn, class... Args> decltype(auto) Call(std::u8string_view fn_name, Args&&... args) const
    {
        return GetSymbol<Fn>(fn_name)(std::forward<Args>(args)...);
    }

  private:
    SharedPtr<void> dll_;
    std::u8string filepath_;
};

class CORE_API CpuInfo
{
  public:
    enum class Vendor
    {
        kOther,
        kIntel,
        kAmd
    };

    static const CpuInfo& Get();

    [[nodiscard]] auto& GetVendor() const noexcept
    {
        return vendor_;
    }

    [[nodiscard]] auto& GetBrand() const noexcept
    {
        return brand_;
    }

    [[nodiscard]] Vendor GetVendorEnum() const noexcept
    {
        return vendor_enum_;
    }

    [[nodiscard]] bool IsIntel() const noexcept
    {
        return vendor_enum_ == Vendor::kIntel;
    }

    [[nodiscard]] bool IsAmd() const noexcept
    {
        return vendor_enum_ == Vendor::kAmd;
    }

    [[nodiscard]] bool SSE3() const noexcept
    {
        return f_1_ecx_.test(0);
    }

    [[nodiscard]] bool PCLMULQDQ() const noexcept
    {
        return f_1_ecx_.test(1);
    }

    [[nodiscard]] bool MONITOR() const noexcept
    {
        return f_1_ecx_.test(3);
    }

    [[nodiscard]] bool SSSE3() const noexcept
    {
        return f_1_ecx_.test(9);
    }

    [[nodiscard]] bool FMA() const noexcept
    {
        return f_1_ecx_.test(12);
    }

    [[nodiscard]] bool CMPXCHG16B() const noexcept
    {
        return f_1_ecx_.test(13);
    }

    [[nodiscard]] bool SSE41() const noexcept
    {
        return f_1_ecx_.test(19);
    }

    [[nodiscard]] bool SSE42() const noexcept
    {
        return f_1_ecx_.test(20);
    }

    [[nodiscard]] bool MOVBE() const noexcept
    {
        return f_1_ecx_.test(22);
    }

    [[nodiscard]] bool POPCNT() const noexcept
    {
        return f_1_ecx_.test(23);
    }

    [[nodiscard]] bool AES() const noexcept
    {
        return f_1_ecx_.test(25);
    }

    [[nodiscard]] bool XSAVE() const noexcept
    {
        return f_1_ecx_.test(26);
    }

    [[nodiscard]] bool OSXSAVE() const noexcept
    {
        return f_1_ecx_.test(27);
    }

    [[nodiscard]] bool AVX() const noexcept
    {
        return f_1_ecx_.test(28);
    }

    [[nodiscard]] bool F16C() const noexcept
    {
        return f_1_ecx_.test(29);
    }

    [[nodiscard]] bool RDRAND() const noexcept
    {
        return f_1_ecx_.test(30);
    }

    [[nodiscard]] bool MSR() const noexcept
    {
        return f_1_edx_.test(5);
    }

    [[nodiscard]] bool CX8() const noexcept
    {
        return f_1_edx_.test(8);
    }

    [[nodiscard]] bool SEP() const noexcept
    {
        return f_1_edx_.test(11);
    }

    [[nodiscard]] bool CMOV() const noexcept
    {
        return f_1_edx_.test(15);
    }

    [[nodiscard]] bool CLFSH() const noexcept
    {
        return f_1_edx_.test(19);
    }

    [[nodiscard]] bool MMX() const noexcept
    {
        return f_1_edx_.test(23);
    }

    [[nodiscard]] bool FXSR() const noexcept
    {
        return f_1_edx_.test(24);
    }

    [[nodiscard]] bool SSE() const noexcept
    {
        return f_1_edx_.test(25);
    }

    [[nodiscard]] bool SSE2() const noexcept
    {
        return f_1_edx_.test(26);
    }

    [[nodiscard]] bool FSGSBASE() const noexcept
    {
        return f_7_ebx_.test(0);
    }

    [[nodiscard]] bool BMI1() const noexcept
    {
        return f_7_ebx_.test(3);
    }

    [[nodiscard]] bool HLE() const noexcept
    {
        return IsIntel() & f_7_ebx_.test(4);
    }

    [[nodiscard]] bool AVX2() const noexcept
    {
        return f_7_ebx_.test(5);
    }

    [[nodiscard]] bool BMI2() const noexcept
    {
        return f_7_ebx_.test(8);
    }

    [[nodiscard]] bool ERMS() const noexcept
    {
        return f_7_ebx_.test(9);
    }

    [[nodiscard]] bool INVPCID() const noexcept
    {
        return f_7_ebx_.test(10);
    }

    [[nodiscard]] bool RTM() const noexcept
    {
        return IsIntel() & f_7_ebx_.test(11);
    }

    [[nodiscard]] bool AVX512F() const noexcept
    {
        return f_7_ebx_.test(16);
    }

    [[nodiscard]] bool RDSEED() const noexcept
    {
        return f_7_ebx_.test(18);
    }

    [[nodiscard]] bool ADX() const noexcept
    {
        return f_7_ebx_.test(19);
    }

    [[nodiscard]] bool AVX512PF() const noexcept
    {
        return f_7_ebx_.test(26);
    }

    [[nodiscard]] bool AVX512ER() const noexcept
    {
        return f_7_ebx_.test(27);
    }

    [[nodiscard]] bool AVX512CD() const noexcept
    {
        return f_7_ebx_.test(28);
    }

    [[nodiscard]] bool SHA() const noexcept
    {
        return f_7_ebx_.test(29);
    }

    [[nodiscard]] bool PREFETCHWT1() const noexcept
    {
        return f_7_ecx_.test(0);
    }

    [[nodiscard]] bool LAHF() const noexcept
    {
        return f_81_ecx_.test(0);
    }

    [[nodiscard]] bool LZCNT() const noexcept
    {
        return IsIntel() & f_81_ecx_.test(5);
    }

    [[nodiscard]] bool ABM() const noexcept
    {
        return IsAmd() & f_81_ecx_.test(5);
    }

    [[nodiscard]] bool SSE4a() const noexcept
    {
        return IsAmd() & f_81_ecx_.test(6);
    }

    [[nodiscard]] bool XOP() const noexcept
    {
        return IsAmd() & f_81_ecx_.test(11);
    }

    [[nodiscard]] bool TBM() const noexcept
    {
        return IsAmd() & f_81_ecx_.test(21);
    }

    [[nodiscard]] bool SYSCALL() const noexcept
    {
        return IsIntel() & f_81_edx_.test(11);
    }

    [[nodiscard]] bool MMXEXT() const noexcept
    {
        return IsAmd() & f_81_edx_.test(22);
    }

    [[nodiscard]] bool RDTSCP() const noexcept
    {
        return IsIntel() & f_81_edx_.test(27);
    }

    [[nodiscard]] bool _3DNOWEXT() const noexcept
    {
        return IsAmd() & f_81_edx_.test(30);
    }

    [[nodiscard]] bool _3DNOW() const noexcept
    {
        return IsAmd() & f_81_edx_.test(31);
    }

  private:
    CpuInfo();

    char8_t vendor_[13]{};
    char8_t brand_[49]{};

    Vendor vendor_enum_ = Vendor::kOther;

    std::bitset<32> f_1_ecx_;
    std::bitset<32> f_1_edx_;
    std::bitset<32> f_7_ebx_;
    std::bitset<32> f_7_ecx_;
    std::bitset<32> f_81_ecx_;
    std::bitset<32> f_81_edx_;
};

CORE_API void CpuId(int cpu_info[4], int func_id) noexcept;
CORE_API void CpuIdCnt(int cpu_info[4], int func_id, int sub_func_id) noexcept;

} // namespace core
} // namespace oeng
