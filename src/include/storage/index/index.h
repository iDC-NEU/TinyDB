/**
 * @file index.h
 * @author sheep
 * @brief index
 * @version 0.1
 * @date 2022-05-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef INDEX_H
#define INDEX_H

#include "catalog/schema.h"
#include "type/value.h"
#include "storage/table/tuple.h"

#include <string>
#include <sstream>
#include <memory>
#include <deque>
#include <unordered_set>

namespace TinyDB {

class Page;

enum IndexType {
    BPlusTree = 0,
    HashTable = 1,
};

/**
 * @brief 
 * Metadata of index.
 * Metadata maintains the key schema since index it the mapping from key to RID.
 */
class IndexMetadata {
public:
    IndexMetadata() = delete;
    IndexMetadata(std::string index_name, std::string table_name, const Schema *tuple_schema, std::vector<uint32_t> key_attrs, IndexType type)
        : index_name_(index_name), table_name_(table_name), key_attrs_(key_attrs), type_(type) {
        // generate key schema based on tuple schema and key attrs
        key_schema_ = Schema::CopySchema(tuple_schema, key_attrs);
    }

    ~IndexMetadata() {
        delete key_schema_;
    }

    inline const std::string &GetIndexName() const {
        return index_name_;
    }
    
    inline const std::string &GetTableName() const {
        table_name_;
    }

    // TODO: shall we add const here?
    inline Schema *GetKeySchema() const {
        return key_schema_;
    }

    /**
     * @brief 
     * return the number of columns inside index key
     * @return uint32_t 
     */
    inline uint32_t GetIndexColumnCount() const {
        return key_schema_->GetColumnCount();
    }

    inline const std::vector<uint32_t> &GetKeyAttrs() const {
        return key_attrs_;
    }

    inline IndexType GetIndexType() const {
        return type_;
    }

    // for debugging
    std::string ToString() const {
        std::stringstream os;

        std::string type = type_ == BPlusTree ? "BPlusTree" : "HashTable";

        os << "IndexMetadata["
           << "Name = " << index_name_ << ", "
           << "Type = " << type << ", "
           << "TableName = " << table_name_ << "] :: "
           << key_schema_->ToString();

        return os.str();
    }

private:
    std::string index_name_;
    std::string table_name_;
    Schema *key_schema_;
    // this is not necessary since we can generate key_attrs from key_schema and base schema
    const std::vector<uint32_t> key_attrs_;
    IndexType type_;
};

/**
 * @brief 
 * concurrent index execution context.
 * bustub was using transaction to recording these informations.
 * but i really want to make these modules isloated, so i decoupled ConcurrentIndexExecutionContext
 * from transaction
 */
struct IndexExecutionContext {
    // the pages that we latched during index operation
    std::unique_ptr<std::deque<Page *>> page_set_;
    // the page IDs that were deleted during index operation
    std::unique_ptr<std::unordered_set<page_id_t>> deleted_page_set_;
};

/**
 * @brief 
 * base class for different indices. provide the basic interfaces that every index should implement,
 * which are InsertKey, DeleteKey and ScanKey.
 */
class Index {
public:
    explicit Index(std::unique_ptr<IndexMetadata> metadata)
        : metadata_(std::move(metadata)) {}
    
    virtual ~Index() {}

    inline IndexMetadata *GetMetadata() const {
        return metadata_.get();
    }

    inline int GetIndexColumnCount() const {
        return metadata_->GetIndexColumnCount();
    }
    
    inline const std::string &GetIndexName() const {
        return metadata_->GetIndexName();
    }

    inline Schema *GetKeySchema() const {
        return metadata_->GetKeySchema();
    }

    const std::vector<uint32_t> &GetKeyAttrs() const {
        return metadata_->GetKeyAttrs();
    }

    std::string ToString() const {
        return metadata_->ToString();
    }

    // real operations

    /**
     * @brief 
     * insert an entry into index
     * @param key key to inserted
     * @param rid value to inserted
     */
    virtual void InsertEntry(const Tuple &key, RID rid) = 0;

    /**
     * @brief 
     * delete an entry
     * @param key target key
     * @param rid target rid, for supporting duplicated keys
     */
    virtual void DeleteEntry(const Tuple &key, RID rid) = 0;

    /**
     * @brief 
     * get the rid corresponding to given key
     * @param key target key
     * @param result result array, for supporting duplicated keys
     */
    virtual void ScanKey(const Tuple &key, std::vector<RID> *result) = 0;

private:
    std::unique_ptr<IndexMetadata> metadata_;
};

}

#endif