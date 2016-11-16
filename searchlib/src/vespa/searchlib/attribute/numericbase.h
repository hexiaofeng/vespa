// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "attributevector.h"
#include <vespa/searchlib/common/sort.h>
#include <vespa/searchlib/attribute/enumstorebase.h>
#include "loadedenumvalue.h"

namespace search {

class NumericAttribute : public AttributeVector
{
protected:
    typedef EnumStoreBase::Index       EnumIndex;
    typedef EnumStoreBase::IndexVector EnumIndexVector;
    typedef EnumStoreBase::EnumVector  EnumVector;

    NumericAttribute(const vespalib::string & name,
                     const AttributeVector::Config & cfg)
        : AttributeVector(name, cfg)
    {
    }

    virtual void
    fillEnum0(const void *src,
              size_t srcLen,
              EnumIndexVector &eidxs);

    virtual void
    fillEnumIdx(ReaderBase &attrReader,
                const EnumIndexVector &eidxs,
                attribute::LoadedEnumAttributeVector &loaded);

    virtual void
    fillEnumIdx(ReaderBase &attrReader,
                const EnumIndexVector &eidxs,
                EnumVector &enumHist);

    virtual void
    fillPostingsFixupEnum(const attribute::LoadedEnumAttributeVector &loaded);

    virtual void
    fixupEnumRefCounts(const EnumVector &enumHist);

    template<typename T>
    class Equal
    {
    private:
        T _value;
        bool _valid;
    protected:
        Equal(QueryTermSimple &queryTerm, bool avoidUndefinedInRange = false)
            : _value(0),
              _valid(false)
        {
            (void) avoidUndefinedInRange;
            QueryTermSimple::RangeResult<T> res = queryTerm.getRange<T>();
            _valid = res.valid && res.isEqual() && !res.adjusted;
            _value = res.high;
        }
        bool isValid() const { return _valid; }
        bool match(T v) const { return v == _value; }
        Int64Range getRange() const {
            return Int64Range(static_cast<int64_t>(_value));
        }
    };

    template<typename T>
    class Range
    {
    protected:
        T _low;
        T _high;
    private:
        bool _valid;
        int _limit;
        size_t _max_per_group;
    protected:
        Range(const QueryTermSimple & queryTerm,
              bool avoidUndefinedInRange=false)
            : _low(0),
              _high(0),
              _valid(false)
        {
            QueryTermSimple::RangeResult<T> res = queryTerm.getRange<T>();
            _valid = res.isEqual() ? (res.valid && !res.adjusted) : res.valid;
            _low = res.low;
            _high = res.high;
            _limit = queryTerm.getRangeLimit();
            _max_per_group = queryTerm.getMaxPerGroup();
            if (_valid && avoidUndefinedInRange &&
                _low == std::numeric_limits<T>::min()) {
                _low += 1;
            }
        }
        Int64Range
        getRange() const
        {
            return Int64Range(static_cast<int64_t>(_low),
                              static_cast<int64_t>(_high));
        }
        bool isValid() const { return _valid; }
        bool match(T v) const { return (_low <= v) && (v <= _high); }
        int getRangeLimit() const { return _limit; }
        size_t getMaxPerGroup() const { return _max_per_group; }

        template <typename BaseType>
        search::Range<BaseType>
        cappedRange(bool isFloat, bool isUnsigned)
        {
            BaseType low = static_cast<BaseType>(_low);
            BaseType high = static_cast<BaseType>(_high);

            BaseType numMin = std::numeric_limits<BaseType>::min();
            BaseType numMax = std::numeric_limits<BaseType>::max();

            if (isFloat)
            {
                if (_low <= (-numMax)) {
                    low = -numMax;
                }
            } else {
                if (_low <= (numMin)) {
                    if (isUnsigned) {
                        low = numMin;
                    } else {
                        low = numMin + 1; // we must avoid the undefined value
                    }
                }
            }

            if (_high >= (numMax)) {
                high = numMax;
            }
            return search::Range<BaseType>(low, high);
        }

    };
public:
    DECLARE_IDENTIFIABLE_ABSTRACT(NumericAttribute);
};

} // namespace search

