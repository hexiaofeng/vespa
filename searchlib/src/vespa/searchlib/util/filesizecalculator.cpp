// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "filesizecalculator.h"
#include <vespa/vespalib/data/fileheader.h>
#include <vespa/log/log.h>
LOG_SETUP(".searchlib.util.filesizecalculator");


namespace search {

namespace {

const vespalib::string fileBitSizeTag = "fileBitSize";

bool byteAligned(uint64_t bitSize)
{
    return ((bitSize % 8) == 0);
}

}

bool
FileSizeCalculator::extractFileSize(const vespalib::GenericHeader &header,
                                    size_t headerLen,
                                    vespalib::string fileName, size_t &fileSize)
{
    if (!header.hasTag(fileBitSizeTag)) {
        return true;
    }
    uint64_t fileBitSize = header.getTag(fileBitSizeTag).asInteger();
    uint64_t fileByteSize = fileBitSize / 8;
    if (!byteAligned(fileBitSize)) {
        LOG(error,
            "Bad header file size tag for %s, fileBitSize=%zu which is not a multiple of 8",
            fileName.c_str(), fileBitSize);
        return false;
    }
    if (fileByteSize < headerLen) {
        LOG(error,
            "Bad header file size tag for %s, fileBitSize=%zu but header is %zu bits",
            fileName.c_str(), fileBitSize, headerLen * 8);
        return false;
    }
    if (fileByteSize > fileSize) {
        LOG(error,
            "Bad header file size tag for %s, fileBitSize=%zu but whole file size is %zu bits",
            fileName.c_str(), fileBitSize, fileSize * 8);
        return false;
    }
    fileSize = fileByteSize;
    return true;
}

}
