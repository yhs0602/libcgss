#pragma once

#include <type_traits>
#include "cgss_env.h"
#include "cgss_cenum.h"

CGSS_NS_BEGIN

    enum class StreamSeekOrigin : uint32_t {
        Begin = CGSS_ORIGIN_BEGIN,
        Current = CGSS_ORIGIN_CURRENT,
        End = CGSS_ORIGIN_END
    };

    enum class HcaCipherType : uint32_t {
        NoCipher = CGSS_HCA_CIPH_NO_CIPHER,
        Static = CGSS_HCA_CIPH_STATIC,
        WithKey = CGSS_HCA_CIPH_WITH_KEY
    };

    enum class OpResult : int32_t {
        OK = CGSS_OP_OK,
        GenericFault = CGSS_OP_GENERIC_FAULT,
        BufferTooSmall = CGSS_OP_BUFFER_TOO_SMALL,
        NotImplemented = CGSS_OP_NOT_IMPLEMENTED,
        InvalidOperation = CGSS_OP_INVALID_OPERATION,
        InvalidArgument = CGSS_OP_INVALID_ARGUMENT,
        FormatError = CGSS_OP_FORMAT_ERROR,
        ChecksumError = CGSS_OP_CHECKSUM_ERROR,
        DecodeFailed = CGSS_OP_DECODE_FAILED
    };

    enum class FileMode : uint32_t {
        None = CGSS_FILE_MODE_NONE,
        Append = CGSS_FILE_MODE_APPEND,
        Create = CGSS_FILE_MODE_CREATE,
        CreateNew = CGSS_FILE_MODE_CREATE_NEW,
        OpenExisting = CGSS_FILE_MODE_OPEN_EXISTING,
        OpenOrCreate = CGSS_FILE_MODE_OPEN_OR_CREATE
    };

    enum class FileAccess : uint32_t {
        None = CGSS_FILE_ACCESS_NONE,
        Read = CGSS_FILE_ACCESS_READ,
        Write = CGSS_FILE_ACCESS_WRITE,
        ReadWrite = CGSS_FILE_ACCESS_READ_WRITE,
    };

#define UNDER(x) std::underlying_type_t<x>
#define DECL_ENUM_CLS_OP(cls, op)  inline cls operator op(cls l, cls r) { \
                                        return (cls)(static_cast<UNDER(cls)>(l) op static_cast<UNDER(cls)>(r)); \
                                    }

    DECL_ENUM_CLS_OP(FileMode, |)

    DECL_ENUM_CLS_OP(FileMode, &)

    DECL_ENUM_CLS_OP(FileAccess, |)

    DECL_ENUM_CLS_OP(FileAccess, &)

#undef DECL_ENUM_CLS_OP
#undef UNDER

CGSS_NS_END