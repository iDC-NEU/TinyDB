/**
 * @file tuple.h
 * @author sheep
 * @brief tuple
 * @version 0.1
 * @date 2022-05-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef TUPLE_H
#define TUPLE_H

#include "common/rid.h"
#include "common/logger.h"
#include "catalog/schema.h"
#include "type/value.h"

#include <cstring>

namespace TinyDB {

/**
 * @brief 
 * description of single tuple that stays in memory
 * Tuple format:
 * | FIXED-SIZE VALUE or VARIED-SIZE OFFSET | PAYLOAD OF VARIED-SIZE TYPE
 * i.e. for every column, either it contains the corresponding fixed-size value which can be 
 * retrieved based on column-offset in schema, or it contains the offset of varied-size type, and 
 * the corresponding payload is placed at the end of the tuple
 */
class Tuple {
public:
    // default tuple, which doesn't have any specific data nor the information
    Tuple() = default;

    // create tuple from values and corresponding schema
    Tuple(std::vector<Value> values, const Schema *schema);

    // copy constructor
    Tuple(const Tuple &other);
    Tuple &operator=(Tuple other);

    // do we need to provide this manually?
    Tuple(Tuple &&other)
        : rid_(other.rid_),
          size_(other.size_),
          data_(other.data_) {
        // move the ownership
        other.data_ = nullptr;
        other.size_ = 0;
    }

    void Swap(Tuple &rhs) {
        std::swap(rhs.data_, data_);
        std::swap(rhs.size_, size_);
        std::swap(rid_, rhs.rid_);
    }

    ~Tuple();

    // helper functions

    inline RID GetRID() const {
        return rid_;
    }
    
    // TODO: should we return const char *?
    inline char *GetData() const {
        return data_;
    }

    /**
     * @brief Get the tuple length, including varlen object
     * @return uint32_t 
     */
    inline uint32_t GetLength() const {
        return size_;
    }

    /**
     * @brief 
     * check whether the tuple contains data.
     * default tuple will be an invalid tuple.
     * @return true tuple is valid
     * @return false tuple doesn't contains any data
     */
    inline bool IsValid() const {
        return data_ != nullptr;
    }

    /**
     * @brief Get the tuple length, including varlen object
     * @return uint32_t 
     */
    inline uint32_t GetSize() const {
        return size_;
    }

    inline void SetRID(const RID &rid) {
        rid_ = rid;
    }

    // get the value of a specified column
    Value GetValue(const Schema *schema, uint32_t column_idx) const;

    /**
     * @brief 
     * generate a key tuple given schemas and attributes
     * @param schema schema of current tuple
     * @param key_schema schema of returned tuple
     * @param key_attrs indices of the columns of old schema that will constitute new schema
     * @return Tuple 
     */
    Tuple KeyFromTuple(const Schema *schema, const Schema *key_schema, const std::vector<uint32_t> &key_attrs) const;

    /**
     * @brief 
     * generate a tuple by giving base schema and target schema. And we will generate key_attrs list ourself
     * @param schema 
     * @param key_schema 
     * @return Tuple 
     */
    Tuple KeyFromTuple(const Schema *schema, const Schema *key_schema) const;

    // Is the column value null?
    inline bool IsNull(const Schema *schema, uint32_t column_idx) const {
        Value value = GetValue(schema, column_idx);
        return value.IsNull();
    }

    std::string ToString(const Schema *schema) const;

    /**
     * @brief 
     * serialize tuple data with size
     * @param storage 
     * @return size_t return the size we used
     */
    size_t SerializeToWithSize(char *storage) const;

    // deserialize tuple data with size
    static Tuple DeserializeFromWithSize(const char *storage);

    /**
     * @brief 
     * serialize tuple data without size. 
     * this serialization method is not self-contained, 
     * thus we need other metadata to store the size.
     * @param storage buffer contains tuple data
     */
    void SerializeTo(char *storage) const;

    /**
     * @brief 
     * deserialize tuple without size, same as above
     * @param storage buffer contains tuple data
     * @param size tuple size
     * @return Tuple 
     */
    static Tuple DeserializeFrom(const char *storage, uint32_t size);

    /**
     * @brief 
     * deserialize the tuple and store the new data inplace.
     * i.e. replace current data in tuple with the one in storage.
     * we will clear the previous data buffer.
     * storage contains the size.
     * @param storage data buffer
     */

    void DeserializeFromInplaceWithSize(const char *storage);
    /**
     * @brief 
     * deserialize the tuple and store the new data inplace.
     * i.e. replace current data in tuple with the one in storage.
     * we will clear the previous data buffer.
     * storage didn't contain size, so we need to pass in manually
     * @param storage data buffer
     * @param size tuple size
     */
    void DeserializeFromInplace(const char *storage, uint32_t size);

    // compare two tuple at byte level
    bool operator==(const Tuple &rhs) const {
        if (rhs.GetSize() != GetSize()) {
            return false;
        }

        return memcmp(rhs.GetData(), GetData(), GetSize()) == 0;
    }

    /**
     * @brief
     * Get the size we need to use on disk.
     * WARNING::Only used for serialization with size.
     * currently, it will return sizeof(uint32_t) + size of tuple
     * @return size_t 
     */
    size_t GetSerializationSize() const {
        return sizeof(uint32_t) + size_;
    }

private:
    // get the starting storage address of specific column
    const char *GetDataPtr(const Schema *schema, uint32_t column_idx) const;

    // default is invalid rid
    RID rid_{};

    // total size of this tuple
    uint32_t size_{0};

    // payload
    char *data_{nullptr};
};

}

#endif