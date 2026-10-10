#include <safetyhook.hpp>
#include <safetyhook/easy.hpp>

namespace util
{
    class MidhookManager
    {
        using Self = MidhookManager;

    private:
        std::vector<SafetyHookMid> m_midhooks;
        MidhookManager() = default;

    public:
        MidhookManager(Self &&) = delete;

        static Self *get()
        {
            static Self instance;
            return &instance;
        }

        inline void save(SafetyHookMid &&hook)
        {
            m_midhooks.push_back(std::move(hook));
        };
    };

    void midhook(uintptr_t address, std::string_view label, void (*callback)(safetyhook::Context &));
}