/**
 * @file index_builder.cpp
 * @author sheep
 * @brief index factory
 * @version 0.1
 * @date 2022-05-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "storage/index/index_builder.h"
#include "storage/index/b_plus_tree_index.h"

#include <memory>

namespace TinyDB {

std::unique_ptr<Index> IndexBuilder::Build(std::unique_ptr<IndexMetadata> metadata, BufferPoolManager *bpm) {
    switch (metadata->GetIndexType()) {
    case IndexType::BPlusTreeType: {
        switch (metadata->GetKeySize()) {
        case 4: {
            auto ptr = new BPlusTreeIndex<GenericKey<4>, RID, GenericComparator<4>>(std::move(metadata), bpm);
            return std::unique_ptr<Index> (ptr);
        }
        case 8: {
            auto ptr = new BPlusTreeIndex<GenericKey<8>, RID, GenericComparator<8>>(std::move(metadata), bpm);
            return std::unique_ptr<Index> (ptr);
        }
        case 16: {
            auto ptr = new BPlusTreeIndex<GenericKey<16>, RID, GenericComparator<16>>(std::move(metadata), bpm);
            return std::unique_ptr<Index> (ptr);
        }
        case 32: {
            auto ptr = new BPlusTreeIndex<GenericKey<32>, RID, GenericComparator<32>>(std::move(metadata), bpm);
            return std::unique_ptr<Index> (ptr);
        }
        case 64: {
            auto ptr = new BPlusTreeIndex<GenericKey<64>, RID, GenericComparator<64>>(std::move(metadata), bpm);
            return std::unique_ptr<Index> (ptr);
        }
        }
        THROW_NOT_IMPLEMENTED_EXCEPTION("KeySize not supported");
    }
    default:
        THROW_NOT_IMPLEMENTED_EXCEPTION("IndexBuilder not implemented");
    }
}

}