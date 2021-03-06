/**
 * @file buffer_pool_manager.cpp
 * @author sheep
 * @brief implementation of buffer pool manager
 * @version 0.1
 * @date 2022-04-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef BUFFER_POOL_MANAGER_CPP
#define BUFFER_POOL_MANAGER_CPP

#include "buffer/buffer_pool_manager.h"
#include "buffer/lru_replacer.h"
#include "common/logger.h"
#include "recovery/log_manager.h"
#include "storage/page/page_header.h"

namespace TinyDB {

BufferPoolManager::BufferPoolManager(size_t pool_size, DiskManager *disk_manager, LogManager *log_manager)
    : pool_size_(pool_size), disk_manager_(disk_manager), log_manager_(log_manager) {
    // allocate the in-memory page array
    pages_ = new Page[pool_size_];
    // pool size has no meaning for lru replacer, because we(buffer pool manager)
    // are controlling the page num
    replacer_ = new LRUReplacer(pool_size_);

    // initially, every page is in the free list
    for (size_t i = 0; i < pool_size_; i++) {
        free_list_.emplace_back(static_cast<frame_id_t>(i));
    }
}

BufferPoolManager::~BufferPoolManager() {
    delete[] pages_;
    delete replacer_;
}

Page *BufferPoolManager::FetchPage(page_id_t page_id, bool outbound_is_error) {
    std::lock_guard<std::mutex> guard(latch_);

    // if the page is cached
    auto it = page_table_.find(page_id);
    if (it != page_table_.end()) {
        auto page = &pages_[it->second];
        // pin this frame
        replacer_->Pin(it->second);
        // increment the pin count
        page->pin_count_ += 1;
        return page;
    }

    // allocate a new slot
    frame_id_t frame_id = -1;
    if (!free_list_.empty()) {
        // we will use free slot first
        frame_id = free_list_.back();
        free_list_.pop_back();
    } else {
        // otherwise, let's evict a page and reuse it's slot
        if (!replacer_->Evict(&frame_id)) {
            // maybe we should throw runtime error?
            // because this means there is no more slot.
            // or sleep on conditional variable waiting for a slot
            return nullptr;
        }

        auto page = &pages_[frame_id];
        if (page->is_dirty_) {
            FlushPageHelper(frame_id);
        }
        // evict this page
        page_table_.erase(page->GetPageId());
    }

    page_table_[page_id] = frame_id;

    auto page = &pages_[frame_id];
    // initialize the in-memory page representation
    page->page_id_ = page_id;
    page->is_dirty_ = false;
    page->pin_count_ = 1;
    disk_manager_->ReadPage(page_id, page->data_, outbound_is_error);

    return page;
}

bool BufferPoolManager::UnpinPage(page_id_t page_id, bool is_dirty) {
    std::lock_guard<std::mutex> guard(latch_);
    // failed to find this page
    auto it = page_table_.find(page_id);
    if (it == page_table_.end()) {
        return false;
    }
    frame_id_t frame_id = it->second;

    // update is_dirty flag
    pages_[frame_id].is_dirty_ = pages_[frame_id].is_dirty_ || is_dirty;

    // failed to unpin this page
    if (pages_[frame_id].pin_count_ == 0) {
        return false;
    }

    pages_[frame_id].pin_count_ -= 1;
    if (pages_[frame_id].pin_count_ == 0) {
        // put it into replacer
        replacer_->Unpin(frame_id);
    }

    return true;
}

bool BufferPoolManager::FlushPage(page_id_t page_id) {
    std::lock_guard<std::mutex> guard(latch_);
    // this page is not cached
    auto it = page_table_.find(page_id);
    if (it == page_table_.end()) {
        return false;
    }

    // flush to disk
    frame_id_t frame_id = it->second;
    FlushPageHelper(frame_id);

    return true;
}

void BufferPoolManager::FlushPageHelper(frame_id_t frame_id) {
    // write ahead log protocol: 
    // before writting a page into disk, all related logs has to be flush into disk first.
    auto page = &pages_[frame_id];
    if (log_manager_ != nullptr) {
        // get lsn of current page
        auto header = reinterpret_cast<PageHeader *>(page->GetData());
        auto lsn = header->GetLSN();
        // flush the log and record the time
        auto t1 = std::chrono::steady_clock::now();
        // force the log
        log_manager_->Flush(lsn, true);
        auto t2 = std::chrono::steady_clock::now();
        flush_wait_time_ += std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    }
    disk_manager_->WritePage(page->GetPageId(), page->GetData());
    page->is_dirty_ = false;
}

Page *BufferPoolManager::NewPage(page_id_t *page_id) {
    std::lock_guard<std::mutex> guard(latch_);

    // no more space
    if (free_list_.empty() && replacer_->Size() == 0) {
        return nullptr;
    }

    // allocate new page from disk
    *page_id = disk_manager_->AllocatePage();

    frame_id_t frame_id = -1;
    if (!free_list_.empty()) {
        frame_id = free_list_.back();
        free_list_.pop_back();
    } else {
        if (!replacer_->Evict(&frame_id)) {
            return nullptr;
        }

        auto page = &pages_[frame_id];
        if (page->is_dirty_) {
            FlushPageHelper(frame_id);
        }

        page_table_.erase(page->GetPageId());
    }
    page_table_[*page_id] = frame_id;

    auto page = &pages_[frame_id];
    // initialize the in-memory page representation
    page->page_id_ = *page_id;
    page->is_dirty_ = false;
    page->pin_count_ = 1;
    page->ZeroData();

    return page;
}

bool BufferPoolManager::DeletePage(page_id_t page_id) {
    std::lock_guard<std::mutex> guard(latch_);
    // deallocate this page, return it to disk manager
    disk_manager_->DeallocatePage(page_id);
    auto it = page_table_.find(page_id);
    if (it == page_table_.end()) {
        // already removed
        return true;
    }

    frame_id_t frame_id = page_table_[page_id];
    // check whether other one is still using
    if (pages_[frame_id].pin_count_ > 0) {
        return false;
    }
    // reset page id, because this might interfere "FlushAllPages"
    pages_[frame_id].page_id_ = -1;

    page_table_.erase(page_id);
    // put this slot to free list
    free_list_.push_front(frame_id);
    // remove it from replacer
    replacer_->Pin(frame_id);
    return true;
}

void BufferPoolManager::FlushAllPages() {
    std::lock_guard<std::mutex> guard(latch_);

    // maybe we should iterate hash table?
    for (size_t i = 0; i < pool_size_; i++) {
        if (page_table_.count(pages_[i].GetPageId()) == 0) {
            continue;
        }
        FlushPageHelper(i);
    }
}

bool BufferPoolManager::CheckPinCount() {
    std::lock_guard<std::mutex> guard(latch_);
    bool flag = true;
    for (size_t i = 0; i < pool_size_; i++) {
        if (page_table_.count(pages_[i].GetPageId()) == 0) {
            continue;
        }
        if (pages_[i].GetPinCount() != 0) {
            LOG_ERROR("page %d has pin count %d", pages_[i].GetPageId(), pages_[i].GetPinCount());
            flag = false;
        }
    }
    return flag;
}

}

#endif