#include "midhook.hpp"

#include <Geode/Geode.hpp>

namespace util
{

    void midhook(uintptr_t address, std::string_view label, void (*callback)(safetyhook::Context &))
    {
        MidhookManager::get()->save(safetyhook::create_mid(address, callback));
        geode::log::info("[MIDHOOK] enabled {} at 0x{:x}", label, address);
    }

}

// all this borrowed from silicate sob