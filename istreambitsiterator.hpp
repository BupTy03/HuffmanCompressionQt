#ifndef ISTREAMBITSITERATOR_HPP
#define ISTREAMBITSITERATOR_HPP

#include <istream>
#include <cassert>
#include <memory>

class IstreamBitsIterator
    : public std::iterator<
        std::input_iterator_tag,
        bool,
        std::ptrdiff_t,
        const bool*,
        const bool&>
{
public:
    static constexpr int BITS_IN_BYTE = 8;

    using value_type = bool;

    explicit IstreamBitsIterator() {}
    explicit IstreamBitsIterator(std::istream& is)
        : stream_{ &is }
        , state_{std::make_shared<current_state>()}
    {
        if (*stream_) {
            stream_->read(reinterpret_cast<char*>(&(state_->currByte)), 1);
        }
    }

    bool isLastByte() const
    {
        if(stream_ == nullptr) {
            return true;
        }

        stream_->get();
        const bool result = !bool(*stream_);
        stream_->unget();
        stream_->clear();

        return result;
    }
    std::uint8_t currentBit() const
    {
        assert(state_ != nullptr);
        return state_->currBitIndex;
    }

    IstreamBitsIterator& operator++()
    {
        ++(state_->currBitIndex);
        if (state_->currBitIndex >= BITS_IN_BYTE) {
            state_->currByte = 0;
            state_->currBitIndex = 0;

            stream_->read(reinterpret_cast<char*>(&(state_->currByte)), 1);
            if (!(*stream_)) {
                stream_ = nullptr;
                return *this;
            }
        }
        return *this;
    }
    IstreamBitsIterator operator++(int)
    {
        auto retval = *this;
        ++(*this);
        return retval;
    }

    bool operator==(IstreamBitsIterator other) const
    {
        if (stream_ == nullptr || other.stream_ == nullptr) {
            return stream_ == other.stream_;
        }
        assert(state_);
        return stream_->tellg() == other.stream_->tellg() && state_->currBitIndex == other.state_->currBitIndex;
    }

    bool operator!=(IstreamBitsIterator other) const { return !(*this == other); }

    bool operator*() const
    {
        assert(state_ != nullptr);
        return state_->currByte & (1 << (BITS_IN_BYTE - state_->currBitIndex - 1));
    }

private:
    struct current_state {
        std::uint8_t currByte = 0;
        std::uint8_t currBitIndex = 0;
    };

private:
    std::istream* stream_ = nullptr;
    std::shared_ptr<current_state> state_;
};

#endif // ISTREAMBITSITERATOR_HPP
