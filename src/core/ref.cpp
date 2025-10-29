#include "ref.h"
#include <unordered_set>
#include <shared_mutex>
#include <mutex>
#include "log_system.h"
namespace Mint {
    static std::shared_mutex g_live_ref_mutex;
    static std::unordered_set<void*> g_live_refs;

    namespace RefUtils {
        void AddToLiveRef(void* instance) {
            std::unique_lock lock(g_live_ref_mutex);
            // assert(instance);
            g_live_refs.insert(instance);
        }

        void RemoveFromLiveRef(void* instance) {
            std::unique_lock lock(g_live_ref_mutex);
            // assert(!instance);
            // assert(g_live_refs.find(instance) != g_live_refs.end());
            g_live_refs.erase(instance);
        }

        bool IsAlive(void* instance) {
            // assert(instance);
            std::shared_lock lock(g_live_ref_mutex);
            return g_live_refs.find(instance) != g_live_refs.end();
        }

        // FOR TEST
        void DumpLiveRefs() {
            std::shared_lock lock(g_live_ref_mutex);
            LOG_INFO(fmt::format("Dumping live refs, count: {}", g_live_refs.size()));
            for (auto &ref : g_live_refs) {
                LOG_INFO(fmt::format(" - ptr: {}, type: {}", ref, typeid(*static_cast<RefCounter *>(ref)).name()));
            }
        }
    }
}