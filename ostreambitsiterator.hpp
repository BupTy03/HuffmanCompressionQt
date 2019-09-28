#ifndef OSTREAMBITSITERATOR_HPP
#define OSTREAMBITSITERATOR_HPP

#include "utils.hpp"

#include <ostream>
#include <memory>
#include <cassert>

class OstreamBitsIterator;

namespace OstreamBitsIteratorImpl {
    class reference_type {
        friend OstreamBitsIterator;
    public:
        reference_type& operator=(bool value);

    private:
        explicit reference_type(OstreamBitsIterator* context) : context_{ context } { assert(context_ != nullptr); }
        OstreamBitsIterator* context_ = nullptr;
    };
}

class OstreamBitsIterator : public std::iterator<std::output_iterator_tag, bool, std::ptrdiff_t, bool*, OstreamBitsIteratorImpl::reference_type>
{
    friend OstreamBitsIteratorImpl::reference_type;
public:
    explicit OstreamBitsIterator(std::ostream& os)
        : stream_{ &os }
        , state_{std::make_shared<current_state>()}
    { }

    void flush()
    {
        assert(state_ != nullptr);
        if (stream_ != nullptr && (*stream_) && state_->currBitIndex > 0) {
            write(*stream_, state_->currByte);
        }
    }
    std::uint8_t currentBit() const
    {
        assert(state_ != nullptr);
        return state_->currBitIndex;
    }

    OstreamBitsIterator& operator++()
    {
        assert(state_ != nullptr);

        ++(state_->currBitIndex);
        if (state_->currBitIndex >= BITS_IN_BYTE) {
            if (stream_ == nullptr) {
                return *this;
            }

            write(*stream_, state_->currByte);
            state_->currByte = 0;
            state_->currBitIndex = 0;
            if (!(*stream_)) {
                stream_ = nullptr;
                return *this;
            }
        }
        return *this;
    }
    OstreamBitsIterator operator++(int)
    {
        auto retval = *this;
        ++(*this);
        return retval;
    }

    bool operator==(OstreamBitsIterator other) const
    {
        if (stream_ == nullptr || other.stream_ == nullptr) {
            return stream_ == other.stream_;
        }

        assert(state_ != nullptr);
        return stream_->tellp() == other.stream_->tellp() && state_->currBitIndex == other.state_->currBitIndex;
    }
    bool operator!=(OstreamBitsIterator other) const { return !(*this == other); }

    reference operator*() { return reference(this); }

private:
    struct current_state {
        std::uint8_t currByte = 0;
        std::uint8_t currBitIndex = 0;
    };

private:
    std::ostream* stream_ = nullptr;
    std::shared_ptr<current_state> state_;
};

namespace OstreamBitsIteratorImpl {
    reference_type& reference_type::operator=(bool value)
    {
        context_->state_->currByte |= (value << (BITS_IN_BYTE - context_->state_->currBitIndex - 1));
        return *this;
    }
}

#endif // OSTREAMBITSITERATOR_HPP
