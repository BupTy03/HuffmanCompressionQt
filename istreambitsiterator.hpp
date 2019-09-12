#ifndef ISTREAMBITSITERATOR_HPP
#define ISTREAMBITSITERATOR_HPP

#include <istream>
#include <cassert>
#include <memory>

class IstreamBitsIterator;
//static bool is_last_byte(const IstreamBitsIterator& it);
//static std::uint8_t current_bit(const IstreamBitsIterator& it);

class IstreamBitsIterator
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
        return stream_ == nullptr || bool(*stream_);
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

            if (!(*stream_)) {
                stream_ = nullptr;
                return *this;
            }
            stream_->read(reinterpret_cast<char*>(&(state_->currByte)), 1);
            stream_->get();
            if (!(*stream_)) {
                stream_ = nullptr;
                return *this;
            }
            stream_->unget();
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

//bool is_last_byte(const IstreamBitsIterator& it)  { return it.isLastByte(); }
//std::uint8_t current_bit(const IstreamBitsIterator& it) { return it.currentBit(); }

#endif // ISTREAMBITSITERATOR_HPP
