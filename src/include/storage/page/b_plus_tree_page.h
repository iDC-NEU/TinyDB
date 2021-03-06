/**
 * @file b_plus_tree_page.h
 * @author sheep
 * @brief base class for different type of B+Tree pages
 * @version 0.1
 * @date 2022-05-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef B_PLUS_TREE_PAGE_H
#define B_PLUS_TREE_PAGE_H

#include "buffer/buffer_pool_manager.h"
#include "storage/index/generic_key.h"
#include "storage/page/page_header.h"

namespace TinyDB {

#define INDEX_TEMPLATE_ARGUMENTS \
    template <typename KeyType, typename ValueType, typename KeyComparator>

// key-value pair in b+tree
#define MappingType std::pair<KeyType, ValueType>

enum class IndexPageType {
    INVALID_INDEX_PAGE = 0,
    LEAF_PAGE,
    INTERNAL_PAGE,
};

/**
 * @brief 
 * Header part for B+Tree page. We inherit the basic header format from page.
 * We didn't inherit Page class directly since we want to manipulate the data just like
 * in memory data structure by reinterpreting data pointer to the BPlusTreePage.
 * Header format:
 * ----------------------------------------------------
 * | PageId(4) | LSN(4) | CurrentSize(4) | MaxSize(4) |
 * ----------------------------------------------------
 * | ParentPageId(4) | PageType(4) |
 * ---------------------------------
 */
class BPlusTreePage: public PageHeader {
public:
    /**
     * @brief 
     * return whether current page is leaf page
     * @return
     */
    inline bool IsLeafPage() const {
        return page_type_ == IndexPageType::LEAF_PAGE;
    }

    /**
     * @brief 
     * return whether current page is root page
     * @return true 
     * @return false 
     */
    bool IsRootPage() const {
        return parent_page_id_ == INVALID_PAGE_ID;
    }

    /**
     * @brief
     * set the type of current page
     * @param page_type 
     */
    void SetPageType(IndexPageType page_type) {
        page_type_ = page_type;
    }

    /**
     * @brief
     * get size of current page
     * @return int 
     */
    int GetSize() const {
        return size_;
    }

    /**
     * @brief
     * set size of current page
     * @param size 
     */
    void SetSize(int size) {
        size_ = size;
    }

    /**
     * @brief 
     * increase the size of current page by amount
     * @param amount 
     */
    void IncreaseSize(int amount) {
        size_ += amount;
    }

    /**
     * @brief
     * get max page size.
     * when page size is greater than max size, we will trigger
     * a split on that page
     * @return int 
     */
    int GetMaxSize() const {
        return max_size_;
    }

    void SetMaxSize(int size) {
        max_size_ = size;
    }

    /**
     * @brief
     * get min page size. 
     * Generally, min page size == max page size / 2
     * @return uint32_t 
     */
    int GetMinSize() const {
        if (IsLeafPage()) {
            return max_size_ / 2;
        }
        return (max_size_ + 1) / 2;
    }

    page_id_t GetParentPageId() const {
        return parent_page_id_;
    }

    void SetParentPageId(page_id_t parent_page_id) {
        parent_page_id_ = parent_page_id;
    }

protected:
    static_assert(sizeof(IndexPageType) == 4);
    static constexpr uint32_t BPLUSTREE_HEADER_SIZE = 24;
    // member varibles that both internal page and leaf page
    // will share

    // count of pairs that stored in current page
    int size_;
    // max count of pairs that could be stored in current page
    int max_size_;
    // pointer to parent page
    page_id_t parent_page_id_;
    // page type
    IndexPageType page_type_;
};

}

#endif