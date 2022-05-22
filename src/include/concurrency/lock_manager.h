/**
 * @file lock_manager.h
 * @author sheep
 * @brief lock manager used for lock-based concurrency control protocol
 * @version 0.1
 * @date 2022-05-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "common/config.h"
#include "common/rid.h"

#include <list>
#include <condition_variable>
#include <mutex>
#include <unordered_map>

namespace TinyDB {

enum class DeadLockResolveProtocol {
    DL_DETECT,
    WAIT_DIE,
    WOUND_WAIT,
};

class TransactionManager;
class TransactionContext;

/**
 * @brief 
 * LockManager is used to manage locks for lock-based concurrency control protocol
 */
class LockManager {
    enum class LockMode {
        SHARED,
        EXECLUSIVE,
    };

    class LockRequest {
    public:
        LockRequest(txn_id_t txn_id, LockMode mode)
            : txn_id_(txn_id), lock_mode_(mode), granted_(false) {}

    private:
        txn_id_t txn_id_;
        LockMode lock_mode_;
        bool granted_;
    };

    // just a struct
    class LockRequestQueue {
    public:
        // request queue
        std::list<LockRequest> request_queue_;
        // for notifying blocked transaction on this rid
        std::condition_variable cv_;
        // whether request for upgrading
        bool upgrading_{false};
        // whether lock is held in execlusive mode
        bool writing_{false};
        // count for txn that hold shared lock
        uint32_t shared_count_{0};
    };

public:
    LockManager(DeadLockResolveProtocol resolve_protocol)
        : resolve_protocol_(resolve_protocol) {}
    
    ~LockManager() = default;

private:
    // global latch
    std::mutex latch_;
    // lock table
    std::unordered_map<RID, LockRequestQueue> lock_table_;
    // DL resolve protocol
    DeadLockResolveProtocol resolve_protocol_;

    // Deadlock resolve protocol specific

};

}