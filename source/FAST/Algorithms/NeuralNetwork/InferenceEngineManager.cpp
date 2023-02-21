#include <FAST/Config.hpp>
#include <FAST/Utility.hpp>
#include <FAST/DeviceManager.hpp>
#ifdef WIN32
// For TensorFlow AVX2 check - Taken from https://docs.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex?view=msvc-160
#include <vector>
#include <bitset>
#include <array>
#include <string>
#include <intrin.h>
namespace fast {
class InstructionSet
{
    // forward declarations
    class InstructionSet_Internal;

public:
    // getters
    static std::string Vendor(void) { return CPU_Rep.vendor_; }
    static std::string Brand(void) { return CPU_Rep.brand_; }

    static bool SSE3(void) { return CPU_Rep.f_1_ECX_[0]; }
    static bool PCLMULQDQ(void) { return CPU_Rep.f_1_ECX_[1]; }
    static bool MONITOR(void) { return CPU_Rep.f_1_ECX_[3]; }
    static bool SSSE3(void) { return CPU_Rep.f_1_ECX_[9]; }
    static bool FMA(void) { return CPU_Rep.f_1_ECX_[12]; }
    static bool CMPXCHG16B(void) { return CPU_Rep.f_1_ECX_[13]; }
    static bool SSE41(void) { return CPU_Rep.f_1_ECX_[19]; }
    static bool SSE42(void) { return CPU_Rep.f_1_ECX_[20]; }
    static bool MOVBE(void) { return CPU_Rep.f_1_ECX_[22]; }
    static bool POPCNT(void) { return CPU_Rep.f_1_ECX_[23]; }
    static bool AES(void) { return CPU_Rep.f_1_ECX_[25]; }
    static bool XSAVE(void) { return CPU_Rep.f_1_ECX_[26]; }
    static bool OSXSAVE(void) { return CPU_Rep.f_1_ECX_[27]; }
    static bool AVX(void) { return CPU_Rep.f_1_ECX_[28]; }
    static bool F16C(void) { return CPU_Rep.f_1_ECX_[29]; }
    static bool RDRAND(void) { return CPU_Rep.f_1_ECX_[30]; }

    static bool MSR(void) { return CPU_Rep.f_1_EDX_[5]; }
    static bool CX8(void) { return CPU_Rep.f_1_EDX_[8]; }
    static bool SEP(void) { return CPU_Rep.f_1_EDX_[11]; }
    static bool CMOV(void) { return CPU_Rep.f_1_EDX_[15]; }
    static bool CLFSH(void) { return CPU_Rep.f_1_EDX_[19]; }
    static bool MMX(void) { return CPU_Rep.f_1_EDX_[23]; }
    static bool FXSR(void) { return CPU_Rep.f_1_EDX_[24]; }
    static bool SSE(void) { return CPU_Rep.f_1_EDX_[25]; }
    static bool SSE2(void) { return CPU_Rep.f_1_EDX_[26]; }

    static bool FSGSBASE(void) { return CPU_Rep.f_7_EBX_[0]; }
    static bool BMI1(void) { return CPU_Rep.f_7_EBX_[3]; }
    static bool HLE(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_7_EBX_[4]; }
    static bool AVX2(void) { return CPU_Rep.f_7_EBX_[5]; }
    static bool BMI2(void) { return CPU_Rep.f_7_EBX_[8]; }
    static bool ERMS(void) { return CPU_Rep.f_7_EBX_[9]; }
    static bool INVPCID(void) { return CPU_Rep.f_7_EBX_[10]; }
    static bool RTM(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_7_EBX_[11]; }
    static bool AVX512F(void) { return CPU_Rep.f_7_EBX_[16]; }
    static bool RDSEED(void) { return CPU_Rep.f_7_EBX_[18]; }
    static bool ADX(void) { return CPU_Rep.f_7_EBX_[19]; }
    static bool AVX512PF(void) { return CPU_Rep.f_7_EBX_[26]; }
    static bool AVX512ER(void) { return CPU_Rep.f_7_EBX_[27]; }
    static bool AVX512CD(void) { return CPU_Rep.f_7_EBX_[28]; }
    static bool SHA(void) { return CPU_Rep.f_7_EBX_[29]; }

    static bool PREFETCHWT1(void) { return CPU_Rep.f_7_ECX_[0]; }

    static bool LAHF(void) { return CPU_Rep.f_81_ECX_[0]; }
    static bool LZCNT(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_81_ECX_[5]; }
    static bool ABM(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[5]; }
    static bool SSE4a(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[6]; }
    static bool XOP(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[11]; }
    static bool TBM(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[21]; }

    static bool SYSCALL(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_81_EDX_[11]; }
    static bool MMXEXT(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_EDX_[22]; }
    static bool RDTSCP(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_81_EDX_[27]; }
    static bool _3DNOWEXT(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_EDX_[30]; }
    static bool _3DNOW(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_EDX_[31]; }

private:
    static const InstructionSet_Internal CPU_Rep;

    class InstructionSet_Internal
    {
    public:
        InstructionSet_Internal()
            : nIds_{ 0 },
            nExIds_{ 0 },
            isIntel_{ false },
            isAMD_{ false },
            f_1_ECX_{ 0 },
            f_1_EDX_{ 0 },
            f_7_EBX_{ 0 },
            f_7_ECX_{ 0 },
            f_81_ECX_{ 0 },
            f_81_EDX_{ 0 },
            data_{},
            extdata_{}
        {
            //int cpuInfo[4] = {-1};
            std::array<int, 4> cpui;

            // Calling __cpuid with 0x0 as the function_id argument
            // gets the number of the highest valid function ID.
            __cpuid(cpui.data(), 0);
            nIds_ = cpui[0];

            for (int i = 0; i <= nIds_; ++i)
            {
                __cpuidex(cpui.data(), i, 0);
                data_.push_back(cpui);
            }

            // Capture vendor string
            char vendor[0x20];
            memset(vendor, 0, sizeof(vendor));
            *reinterpret_cast<int*>(vendor) = data_[0][1];
            *reinterpret_cast<int*>(vendor + 4) = data_[0][3];
            *reinterpret_cast<int*>(vendor + 8) = data_[0][2];
            vendor_ = vendor;
            if (vendor_ == "GenuineIntel")
            {
                isIntel_ = true;
            }
            else if (vendor_ == "AuthenticAMD")
            {
                isAMD_ = true;
            }

            // load bitset with flags for function 0x00000001
            if (nIds_ >= 1)
            {
                f_1_ECX_ = data_[1][2];
                f_1_EDX_ = data_[1][3];
            }

            // load bitset with flags for function 0x00000007
            if (nIds_ >= 7)
            {
                f_7_EBX_ = data_[7][1];
                f_7_ECX_ = data_[7][2];
            }

            // Calling __cpuid with 0x80000000 as the function_id argument
            // gets the number of the highest valid extended ID.
            __cpuid(cpui.data(), 0x80000000);
            nExIds_ = cpui[0];

            char brand[0x40];
            memset(brand, 0, sizeof(brand));

            for (int i = 0x80000000; i <= nExIds_; ++i)
            {
                __cpuidex(cpui.data(), i, 0);
                extdata_.push_back(cpui);
            }

            // load bitset with flags for function 0x80000001
            if (nExIds_ >= 0x80000001)
            {
                f_81_ECX_ = extdata_[1][2];
                f_81_EDX_ = extdata_[1][3];
            }

            // Interpret CPU brand string if reported
            if (nExIds_ >= 0x80000004)
            {
                memcpy(brand, extdata_[2].data(), sizeof(cpui));
                memcpy(brand + 16, extdata_[3].data(), sizeof(cpui));
                memcpy(brand + 32, extdata_[4].data(), sizeof(cpui));
                brand_ = brand;
            }
        };

        int nIds_;
        int nExIds_;
        std::string vendor_;
        std::string brand_;
        bool isIntel_;
        bool isAMD_;
        std::bitset<32> f_1_ECX_;
        std::bitset<32> f_1_EDX_;
        std::bitset<32> f_7_EBX_;
        std::bitset<32> f_7_ECX_;
        std::bitset<32> f_81_ECX_;
        std::bitset<32> f_81_EDX_;
        std::vector<std::array<int, 4>> data_;
        std::vector<std::array<int, 4>> extdata_;
    };
};
// Initialize static member data
const InstructionSet::InstructionSet_Internal InstructionSet::CPU_Rep;
}
#else
#include <dlfcn.h>
#endif
#include "InferenceEngineManager.hpp"

namespace fast {

bool InferenceEngineManager::m_loaded = false;
std::unordered_map<std::string, std::function<InferenceEngine*()>> InferenceEngineManager::m_engines;

#ifdef WIN32
//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsString()
{
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0)
        return std::string(); //No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    //Free the buffer.
    LocalFree(messageBuffer);

    return message;
}
#endif

std::vector<std::string> InferenceEngineManager::getEngineList() {
    loadAll();
    std::vector<std::string> list;
    for(auto&& engine : m_engines)
        list.push_back(engine.first);
    return list;
}

void InferenceEngineManager::loadAll() {
    if(m_loaded) {
        return;
    }
    std::string prefix = "InferenceEngine";
#ifndef WIN32
    prefix = "lib" + prefix;

    if(Reporter::getGlobalReportMethod(Reporter::INFO) != Reporter::COUT) {
        // Disable tensorflow error messages
        setenv("TF_CPP_MIN_LOG_LEVEL", "2", 1);
        setenv("TF_CPP_MIN_VLOG_LEVEL", "2", 1);
    }
#else
	if(Reporter::getGlobalReportMethod(Reporter::INFO) != Reporter::COUT) {
        // Disable tensorflow error messages
		_putenv_s("TF_CPP_MIN_LOG_LEVEL", "2");
		_putenv_s("TF_CPP_MIN_VLOG_LEVEL", "2");
    }
#endif
    Reporter::info() << "Loading inference engines in folder " << Config::getLibraryPath() << Reporter::end();
    for(auto&& item : getDirectoryList(Config::getLibraryPath(), true, false)) {
        auto path = join(Config::getLibraryPath(), item);
        if(item.substr(0, prefix.size()) == prefix) {
            std::string name = item.substr(prefix.size(), item.rfind('.') - prefix.size());
            Reporter::info() << "Loading inference engine " << name << " from shared library " << path << Reporter::end();
#ifdef WIN32
            if(name == "TensorFlow") {
                if(!InstructionSet::AVX2()) {
                    Reporter::warning() << "You CPU does not support AVX2, unable to load TensorFlow inference engine." << Reporter::end();
                    continue;
                }
            }
            SetErrorMode(SEM_FAILCRITICALERRORS); // TODO To avoid diaglog box, when not able to load a DLL
            SetDllDirectory(Config::getLibraryPath().c_str());
            auto handle = LoadLibrary(path.c_str());
            SetDllDirectory("");
            if(!handle) {
                Reporter::warning() << "Failed to load plugin because " << GetLastErrorAsString() << Reporter::end();
                continue;
            }
            auto load = (InferenceEngine* (*)())GetProcAddress(handle, "load");
            if(!load) {
                FreeLibrary(handle);
                Reporter::warning() << "Failed to get adress to load function because " << GetLastErrorAsString() << Reporter::end();
                continue;
            }
#else
            if(name == "TensorFlow") {
#ifdef __arm64__
#else
                if(!__builtin_cpu_supports("avx2")) {
                    Reporter::warning() << "You CPU does not support AVX2, unable to load TensorFlow inference engine." << Reporter::end();
                    continue;
                }
#endif
            }
            auto handle = dlopen(path.c_str(), RTLD_LAZY);
            if(!handle) {
                Reporter::warning() << "Failed to load plugin because " << dlerror() << Reporter::end();
                continue;
            }
            auto load = (InferenceEngine* (*)())dlsym(handle, "load");
            if(!load) {
                dlclose(handle);
                Reporter::warning() << "Failed to get address of load function because " << Reporter::end();// dlerror() << Reporter::end();
                continue;
            }
#endif
            m_engines[name] = load;
        }
    }
    m_loaded = true;
}


std::shared_ptr<InferenceEngine> InferenceEngineManager::loadBestAvailableEngine() {
    loadAll();
    if(m_engines.empty())
        throw Exception("No inference engines available on the system");

    if(isEngineAvailable("TensorFlow")) {
        // Default is tensorflow if GPU support is enabled
        auto engine = loadEngine("TensorFlow");
        auto devices = engine->getDeviceList();
        for (auto &&device : devices) {
            if (device.type == InferenceDeviceType::GPU)
                return engine;
        }
    }

    if(isEngineAvailable("TensorRT"))
        return loadEngine("TensorRT");

#ifdef WIN32
    // If on windows, and main device is not an intel device, use ONNXRuntime.
    if(std::dynamic_pointer_cast<OpenCLDevice>(DeviceManager::getInstance()->getDefaultDevice())->getPlatformVendor() != PLATFORM_VENDOR_INTEL) {
        if(isEngineAvailable("ONNXRuntime"))
            return loadEngine("ONNXRuntime");
    }
#endif

    return loadEngine(getEngineList().front());
}

std::shared_ptr<InferenceEngine> InferenceEngineManager::loadBestAvailableEngine(ModelFormat modelFormat) {
    loadAll();
    if(m_engines.empty())
        throw Exception("No inference engines available on the system");

    if(isEngineAvailable("TensorFlow") && loadEngine("TensorFlow")->isModelFormatSupported(modelFormat)) {
        // Default is tensorflow if GPU support is enabled
        auto engine = loadEngine("TensorFlow");
        auto devices = engine->getDeviceList();
        for (auto &&device : devices) {
            if (device.type == InferenceDeviceType::GPU)
                return engine;
        }
    }

    if(isEngineAvailable("TensorRT") && loadEngine("TensorRT")->isModelFormatSupported(modelFormat))
        return loadEngine("TensorRT");

#ifdef WIN32
    // If on windows, and main device is not an intel device, use ONNXRuntime.
    if(std::dynamic_pointer_cast<OpenCLDevice>(DeviceManager::getInstance()->getDefaultDevice())->getPlatformVendor() != PLATFORM_VENDOR_INTEL) {
        if(isEngineAvailable("ONNXRuntime") && loadEngine("ONNXRuntime")->isModelFormatSupported(modelFormat))
            return loadEngine("ONNXRuntime");
    }
#endif

    for(auto name : getEngineList()) {
        auto engine = loadEngine(name);
        if(engine->isModelFormatSupported(modelFormat))
            return engine;
    }
    throw Exception("No engine for model format found");
}



bool InferenceEngineManager::isEngineAvailable(std::string name) {
    loadAll();
    return m_engines.count(name) > 0;
}

std::shared_ptr<InferenceEngine> InferenceEngineManager::loadEngine(std::string name) {
    loadAll();
    if(m_engines.count(name) == 0)
        throw Exception("Inference engine with name " + name + " is not available");
    // Call the load function which the map stores a handle to
    return std::shared_ptr<InferenceEngine>(m_engines.at(name)());
}

}
