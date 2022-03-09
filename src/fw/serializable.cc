#include <fw/fw.h>
#include <fw/serializable.h>
#include <circle/util.h>

// Some macros/functions to optimize for architectures

namespace Fw
{
    Serializable::Serializable()
    {
    }

    Serializable::~Serializable()
    {
    }

    SerializeBufferBase::SerializeBufferBase() :
            m_serLoc(0), m_deserLoc(0)
    {
    }

    SerializeBufferBase::~SerializeBufferBase()
    {
    }

    void SerializeBufferBase::copyFrom(const SerializeBufferBase &src)
    {
        this->m_serLoc = src.m_serLoc;
        this->m_deserLoc = src.m_deserLoc;
        FW_ASSERT(src.getBuffAddr());
        FW_ASSERT(this->getBuffAddr());
        // destination has to be same or bigger
        FW_ASSERT(src.getBuffLength() <= this->getBuffCapacity(), src.getBuffLength(), this->getBuffLength());
        (void) memcpy(this->getBuffAddr(), src.getBuffAddr(), this->m_serLoc);
    }

    // Copy constructor doesn't make sense in this virtual class as there is nothing to copy. Derived classes should
    // call the empty constructor and then call their own copy function
    SerializeBufferBase &SerializeBufferBase::operator=(const SerializeBufferBase &src)
    { // lgtm[cpp/rule-of-two]
        this->copyFrom(src);
        return *this;
    }

    // serialization routines

    SerializeStatus SerializeBufferBase::serialize(u8 val)
    {
        if (this->m_serLoc + static_cast<unsigned>(sizeof(val)) - 1 >= this->getBuffCapacity())
        {
            return FW_SERIALIZE_NO_ROOM_LEFT;
        }
        FW_ASSERT(this->getBuffAddr());
        this->getBuffAddr()[this->m_serLoc] = val;
        this->m_serLoc += sizeof(val);
        this->m_deserLoc = 0;

        return FW_SERIALIZE_OK;
    }

    SerializeStatus SerializeBufferBase::serialize(s8 val)
    {
        if (this->m_serLoc + static_cast<unsigned>(sizeof(val)) - 1 >= this->getBuffCapacity())
        {
            return FW_SERIALIZE_NO_ROOM_LEFT;
        }
        FW_ASSERT(this->getBuffAddr());
        this->getBuffAddr()[this->m_serLoc + 0] = static_cast<u8>(val);
        this->m_serLoc += sizeof(val);
        this->m_deserLoc = 0;
        return FW_SERIALIZE_OK;
    }

    SerializeStatus SerializeBufferBase::serialize(u16 val) {
        if (this->m_serLoc + static_cast<unsigned>(sizeof(val)) - 1 >= this->getBuffCapacity()) {
            return FW_SERIALIZE_NO_ROOM_LEFT;
        }
        FW_ASSERT(this->getBuffAddr());
        // MSB first
        this->getBuffAddr()[this->m_serLoc + 0] = static_cast<u8>(val >> 8);
        this->getBuffAddr()[this->m_serLoc + 1] = static_cast<u8>(val);
        this->m_serLoc += sizeof(val);
        this->m_deserLoc = 0;
        return FW_SERIALIZE_OK;
    }

    SerializeStatus SerializeBufferBase::serialize(s16 val) {
        if (this->m_serLoc + static_cast<unsigned>(sizeof(val)) - 1 >= this->getBuffCapacity()) {
            return FW_SERIALIZE_NO_ROOM_LEFT;
        }
        FW_ASSERT(this->getBuffAddr());
        // MSB first
        this->getBuffAddr()[this->m_serLoc + 0] = static_cast<u8>(val >> 8);
        this->getBuffAddr()[this->m_serLoc + 1] = static_cast<u8>(val);
        this->m_serLoc += sizeof(val);
        this->m_deserLoc = 0;
        return FW_SERIALIZE_OK;
    }

    SerializeStatus SerializeBufferBase::serialize(u32 val) {
        if (this->m_serLoc + static_cast<unsigned>(sizeof(val)) - 1 >= this->getBuffCapacity()) {
            return FW_SERIALIZE_NO_ROOM_LEFT;
        }
        FW_ASSERT(this->getBuffAddr());
        // MSB first
        this->getBuffAddr()[this->m_serLoc + 0] = static_cast<u8>(val >> 24);
        this->getBuffAddr()[this->m_serLoc + 1] = static_cast<u8>(val >> 16);
        this->getBuffAddr()[this->m_serLoc + 2] = static_cast<u8>(val >> 8);
        this->getBuffAddr()[this->m_serLoc + 3] = static_cast<u8>(val);
        this->m_serLoc += sizeof(val);
        this->m_deserLoc = 0;
        return FW_SERIALIZE_OK;
    }

    SerializeStatus SerializeBufferBase::serialize(s32 val) {
        if (this->m_serLoc + static_cast<unsigned>(sizeof(val)) - 1 >= this->getBuffCapacity()) {
            return FW_SERIALIZE_NO_ROOM_LEFT;
        }
        FW_ASSERT(this->getBuffAddr());
        // MSB first
        this->getBuffAddr()[this->m_serLoc + 0] = static_cast<u8>(val >> 24);
        this->getBuffAddr()[this->m_serLoc + 1] = static_cast<u8>(val >> 16);
        this->getBuffAddr()[this->m_serLoc + 2] = static_cast<u8>(val >> 8);
        this->getBuffAddr()[this->m_serLoc + 3] = static_cast<u8>(val);
        this->m_serLoc += sizeof(val);
        this->m_deserLoc = 0;
        return FW_SERIALIZE_OK;
    }

    SerializeStatus SerializeBufferBase::serialize(u64 val) {
        if (this->m_serLoc + static_cast<unsigned>(sizeof(val)) - 1 >= this->getBuffCapacity()) {
            return FW_SERIALIZE_NO_ROOM_LEFT;
        }
        FW_ASSERT(this->getBuffAddr());
        // MSB first
        this->getBuffAddr()[this->m_serLoc + 0] = static_cast<u8>(val >> 56);
        this->getBuffAddr()[this->m_serLoc + 1] = static_cast<u8>(val >> 48);
        this->getBuffAddr()[this->m_serLoc + 2] = static_cast<u8>(val >> 40);
        this->getBuffAddr()[this->m_serLoc + 3] = static_cast<u8>(val >> 32);
        this->getBuffAddr()[this->m_serLoc + 4] = static_cast<u8>(val >> 24);
        this->getBuffAddr()[this->m_serLoc + 5] = static_cast<u8>(val >> 16);
        this->getBuffAddr()[this->m_serLoc + 6] = static_cast<u8>(val >> 8);
        this->getBuffAddr()[this->m_serLoc + 7] = static_cast<u8>(val);
        this->m_serLoc += sizeof(val);
        this->m_deserLoc = 0;
        return FW_SERIALIZE_OK;
    }

    SerializeStatus SerializeBufferBase::serialize(s64 val) {
        if (this->m_serLoc + static_cast<unsigned>(sizeof(val)) - 1 >= this->getBuffCapacity()) {
            return FW_SERIALIZE_NO_ROOM_LEFT;
        }
        FW_ASSERT(this->getBuffAddr());
        // MSB first
        this->getBuffAddr()[this->m_serLoc + 0] = static_cast<u8>(val >> 56);
        this->getBuffAddr()[this->m_serLoc + 1] = static_cast<u8>(val >> 48);
        this->getBuffAddr()[this->m_serLoc + 2] = static_cast<u8>(val >> 40);
        this->getBuffAddr()[this->m_serLoc + 3] = static_cast<u8>(val >> 32);
        this->getBuffAddr()[this->m_serLoc + 4] = static_cast<u8>(val >> 24);
        this->getBuffAddr()[this->m_serLoc + 5] = static_cast<u8>(val >> 16);
        this->getBuffAddr()[this->m_serLoc + 6] = static_cast<u8>(val >> 8);
        this->getBuffAddr()[this->m_serLoc + 7] = static_cast<u8>(val);
        this->m_serLoc += sizeof(val);
        this->m_deserLoc = 0;
        return FW_SERIALIZE_OK;
    }


    SerializeStatus SerializeBufferBase::serialize(f64 val) {
        // floating point values need to be byte-swapped as well, so copy to u64 and use that routine
        u64 u64Val;
        (void) memcpy(&u64Val, &val, sizeof(val));
        return this->serialize(u64Val);

    }

    SerializeStatus SerializeBufferBase::serialize(f32 val)
    {

        // floating point values need to be byte-swapped as well, so copy to u32 and use that routine
        u32 u32Val;
        (void) memcpy(&u32Val, &val, sizeof(val));
        return this->serialize(u32Val);

    }

    SerializeStatus SerializeBufferBase::serialize(bool val)
    {
        if (this->m_serLoc + static_cast<unsigned>(sizeof(u8)) - 1 >= this->getBuffCapacity())
        {
            return FW_SERIALIZE_NO_ROOM_LEFT;
        }

        FW_ASSERT(this->getBuffAddr());
        if (val)
        {
            this->getBuffAddr()[this->m_serLoc + 0] = FW_SERIALIZE_TRUE_VALUE;
        }
        else
        {
            this->getBuffAddr()[this->m_serLoc + 0] = FW_SERIALIZE_FALSE_VALUE;
        }

        this->m_serLoc += sizeof(u8);
        this->m_deserLoc = 0;
        return FW_SERIALIZE_OK;
    }

    SerializeStatus SerializeBufferBase::serialize(const void* val)
    {
        if (this->m_serLoc + static_cast<unsigned>(sizeof(void*)) - 1
            >= this->getBuffCapacity())
        {
            return FW_SERIALIZE_NO_ROOM_LEFT;
        }

        return this->serialize(reinterpret_cast<POINTER_CAST>(val));

    }

    SerializeStatus SerializeBufferBase::serialize(const u8* buff, unsigned length, bool noLength)
    {
        // First serialize length
        SerializeStatus stat;
        if (not noLength)
        {
            stat = this->serialize(static_cast<FwBuffSizeType>(length));
            if (stat != FW_SERIALIZE_OK)
            {
                return stat;
            }
        }

        // make sure we have enough space
        if (this->m_serLoc + length > this->getBuffCapacity())
        {
            return FW_SERIALIZE_NO_ROOM_LEFT;
        }

        // copy buffer to our buffer
        (void) memcpy(&this->getBuffAddr()[this->m_serLoc], buff, length);
        this->m_serLoc += length;
        this->m_deserLoc = 0;

        return FW_SERIALIZE_OK;
    }

    SerializeStatus SerializeBufferBase::serialize(const Serializable &val)
    {
        return val.serialize(*this);
    }

    SerializeStatus SerializeBufferBase::serialize(
            const SerializeBufferBase &val)
    {
        unsigned size = val.getBuffLength();
        if (this->m_serLoc + size + static_cast<unsigned>(sizeof(FwBuffSizeType))
            > this->getBuffCapacity())
        {
            return FW_SERIALIZE_NO_ROOM_LEFT;
        }

        // First, serialize size
        SerializeStatus stat = this->serialize(static_cast<FwBuffSizeType>(size));
        if (stat != FW_SERIALIZE_OK)
        {
            return stat;
        }

        FW_ASSERT(this->getBuffAddr());
        FW_ASSERT(val.getBuffAddr());
        // serialize buffer
        (void) memcpy(&this->getBuffAddr()[this->m_serLoc], val.getBuffAddr(), size);
        this->m_serLoc += size;
        this->m_deserLoc = 0;

        return FW_SERIALIZE_OK;
    }

    // deserialization routines

    SerializeStatus SerializeBufferBase::deserialize(u8 &val)
    {
        // check for room
        if (this->getBuffLength() == this->m_deserLoc)
        {
            return FW_DESERIALIZE_BUFFER_EMPTY;
        }
        else if (this->getBuffLength() - this->m_deserLoc < static_cast<unsigned>(sizeof(val)))
        {
            return FW_DESERIALIZE_SIZE_MISMATCH;
        }
        // read from current location
        FW_ASSERT(this->getBuffAddr());
        val = this->getBuffAddr()[this->m_deserLoc + 0];
        this->m_deserLoc += sizeof(val);
        return FW_SERIALIZE_OK;
    }

    SerializeStatus SerializeBufferBase::deserialize(s8 &val)
    {
        // check for room
        if (this->getBuffLength() == this->m_deserLoc)
        {
            return FW_DESERIALIZE_BUFFER_EMPTY;
        }
        else if (this->getBuffLength() - this->m_deserLoc < static_cast<unsigned>(sizeof(val)))
        {
            return FW_DESERIALIZE_SIZE_MISMATCH;
        }
        // read from current location
        FW_ASSERT(this->getBuffAddr());
        val = static_cast<s8>(this->getBuffAddr()[this->m_deserLoc + 0]);
        this->m_deserLoc += sizeof(val);
        return FW_SERIALIZE_OK;
    }

    SerializeStatus SerializeBufferBase::deserialize(u16 &val) {
        // check for room
        if (this->getBuffLength() == this->m_deserLoc) {
            return FW_DESERIALIZE_BUFFER_EMPTY;
        } else if (this->getBuffLength() - this->m_deserLoc < static_cast<unsigned>(sizeof(val))) {
            return FW_DESERIALIZE_SIZE_MISMATCH;
        }
        // read from current location
        FW_ASSERT(this->getBuffAddr());
        // MSB first
        val = static_cast<u16>(
            ((this->getBuffAddr()[this->m_deserLoc + 1]) << 0) |
            ((this->getBuffAddr()[this->m_deserLoc + 0]) << 8)
        );
        this->m_deserLoc += sizeof(val);
        return FW_SERIALIZE_OK;
    }

    SerializeStatus SerializeBufferBase::deserialize(s16 &val) {
        // check for room
        if (this->getBuffLength() == this->m_deserLoc) {
            return FW_DESERIALIZE_BUFFER_EMPTY;
        } else if (this->getBuffLength() - this->m_deserLoc < static_cast<unsigned>(sizeof(val))) {
            return FW_DESERIALIZE_SIZE_MISMATCH;
        }
        // read from current location
        FW_ASSERT(this->getBuffAddr());
        // MSB first
        val = static_cast<s16>(
            ((this->getBuffAddr()[this->m_deserLoc + 1]) << 0) |
            ((this->getBuffAddr()[this->m_deserLoc + 0]) << 8)
        );
        this->m_deserLoc += sizeof(val);
        return FW_SERIALIZE_OK;
    }

    SerializeStatus SerializeBufferBase::deserialize(u32 &val) {
        // check for room
        if (this->getBuffLength() == this->m_deserLoc) {
            return FW_DESERIALIZE_BUFFER_EMPTY;
        } else if (this->getBuffLength() - this->m_deserLoc < static_cast<unsigned>(sizeof(val))) {
            return FW_DESERIALIZE_SIZE_MISMATCH;
        }
        // read from current location
        FW_ASSERT(this->getBuffAddr());
        // MSB first
        val = (static_cast<u32>(this->getBuffAddr()[this->m_deserLoc + 3]) << 0)
                | (static_cast<u32>(this->getBuffAddr()[this->m_deserLoc + 2]) << 8)
                | (static_cast<u32>(this->getBuffAddr()[this->m_deserLoc + 1]) << 16)
                | (static_cast<u32>(this->getBuffAddr()[this->m_deserLoc + 0]) << 24);
        this->m_deserLoc += sizeof(val);
        return FW_SERIALIZE_OK;
    }

    SerializeStatus SerializeBufferBase::deserialize(s32 &val) {
        // check for room
        if (this->getBuffLength() == this->m_deserLoc) {
            return FW_DESERIALIZE_BUFFER_EMPTY;
        } else if (this->getBuffLength() - this->m_deserLoc < static_cast<unsigned>(sizeof(val))) {
            return FW_DESERIALIZE_SIZE_MISMATCH;
        }
        // read from current location
        FW_ASSERT(this->getBuffAddr());
        // MSB first
        val = (static_cast<s32>(this->getBuffAddr()[this->m_deserLoc + 3]) << 0)
                | (static_cast<s32>(this->getBuffAddr()[this->m_deserLoc + 2]) << 8)
                | (static_cast<s32>(this->getBuffAddr()[this->m_deserLoc + 1]) << 16)
                | (static_cast<s32>(this->getBuffAddr()[this->m_deserLoc + 0]) << 24);
        this->m_deserLoc += sizeof(val);
        return FW_SERIALIZE_OK;
    }

    SerializeStatus SerializeBufferBase::deserialize(u64 &val) {
        // check for room
        if (this->getBuffLength() == this->m_deserLoc) {
            return FW_DESERIALIZE_BUFFER_EMPTY;
        } else if (this->getBuffLength() - this->m_deserLoc < static_cast<unsigned>(sizeof(val))) {
            return FW_DESERIALIZE_SIZE_MISMATCH;
        }
        // read from current location
        FW_ASSERT(this->getBuffAddr());
        // MSB first
        val = (static_cast<u64>(this->getBuffAddr()[this->m_deserLoc + 7]) << 0)
                | (static_cast<u64>(this->getBuffAddr()[this->m_deserLoc + 6]) << 8)
                | (static_cast<u64>(this->getBuffAddr()[this->m_deserLoc + 5]) << 16)
                | (static_cast<u64>(this->getBuffAddr()[this->m_deserLoc + 4]) << 24)
                | (static_cast<u64>(this->getBuffAddr()[this->m_deserLoc + 3]) << 32)
                | (static_cast<u64>(this->getBuffAddr()[this->m_deserLoc + 2]) << 40)
                | (static_cast<u64>(this->getBuffAddr()[this->m_deserLoc + 1]) << 48)
                | (static_cast<u64>(this->getBuffAddr()[this->m_deserLoc + 0]) << 56);

        this->m_deserLoc += sizeof(val);
        return FW_SERIALIZE_OK;
    }

    SerializeStatus SerializeBufferBase::deserialize(s64 &val) {
        // check for room
        if (this->getBuffLength() == this->m_deserLoc) {
            return FW_DESERIALIZE_BUFFER_EMPTY;
        } else if (this->getBuffLength() - this->m_deserLoc < static_cast<unsigned>(sizeof(val))) {
            return FW_DESERIALIZE_SIZE_MISMATCH;
        }
        // read from current location
        FW_ASSERT(this->getBuffAddr());
        // MSB first
        val = (static_cast<s64>(this->getBuffAddr()[this->m_deserLoc + 7]) << 0)
                | (static_cast<s64>(this->getBuffAddr()[this->m_deserLoc + 6]) << 8)
                | (static_cast<s64>(this->getBuffAddr()[this->m_deserLoc + 5]) << 16)
                | (static_cast<s64>(this->getBuffAddr()[this->m_deserLoc + 4]) << 24)
                | (static_cast<s64>(this->getBuffAddr()[this->m_deserLoc + 3]) << 32)
                | (static_cast<s64>(this->getBuffAddr()[this->m_deserLoc + 2]) << 40)
                | (static_cast<s64>(this->getBuffAddr()[this->m_deserLoc + 1]) << 48)
                | (static_cast<s64>(this->getBuffAddr()[this->m_deserLoc + 0]) << 56);
        this->m_deserLoc += sizeof(val);
        return FW_SERIALIZE_OK;
    }

    SerializeStatus SerializeBufferBase::deserialize(f64 &val) {

        // deserialize as 64-bit int to handle endianness
        u64 tempVal;
        SerializeStatus stat = this->deserialize(tempVal);
        if (stat != FW_SERIALIZE_OK) {
            return stat;
        }
        // copy to argument
        (void) memcpy(&val, &tempVal, sizeof(val));

        return FW_SERIALIZE_OK;
    }

    SerializeStatus SerializeBufferBase::deserialize(bool &val)
    {
        // check for room
        if (this->getBuffLength() == this->m_deserLoc)
        {
            return FW_DESERIALIZE_BUFFER_EMPTY;
        }
        else if (this->getBuffLength() - this->m_deserLoc < static_cast<unsigned>(sizeof(u8)))
        {
            return FW_DESERIALIZE_SIZE_MISMATCH;
        }
        // read from current location
        FW_ASSERT(this->getBuffAddr());
        if (FW_SERIALIZE_TRUE_VALUE == this->getBuffAddr()[this->m_deserLoc + 0])
        {
            val = true;
        }
        else if (FW_SERIALIZE_FALSE_VALUE == this->getBuffAddr()[this->m_deserLoc + 0])
        {
            val = false;
        }
        else
        {
            return FW_DESERIALIZE_FORMAT_ERROR;
        }

        this->m_deserLoc += sizeof(u8);
        return FW_SERIALIZE_OK;
    }

    SerializeStatus SerializeBufferBase::deserialize(void*&val)
    {
        return this->deserialize(reinterpret_cast<POINTER_CAST &>(val));
    }

    SerializeStatus SerializeBufferBase::deserialize(f32 &val)
    {
        // deserialize as 64-bit int to handle endianness
        u32 tempVal;
        SerializeStatus stat = this->deserialize(tempVal);
        if (stat != FW_SERIALIZE_OK)
        {
            return stat;
        }
        (void) memcpy(&val, &tempVal, sizeof(val));

        return FW_SERIALIZE_OK;
    }

    SerializeStatus SerializeBufferBase::deserialize(u8* buff, unsigned &length, bool noLength)
    {

        FW_ASSERT(this->getBuffAddr());

        if (not noLength)
        {
            FwBuffSizeType storedLength;

            SerializeStatus stat = this->deserialize(storedLength);

            if (stat != FW_SERIALIZE_OK)
            {
                return stat;
            }

            // make sure it fits
            if ((storedLength > this->getBuffLeft()) or (storedLength > length))
            {
                return FW_DESERIALIZE_SIZE_MISMATCH;
            }

            (void) memcpy(buff, &this->getBuffAddr()[this->m_deserLoc], storedLength);

            length = static_cast<unsigned>(storedLength);

        }
        else
        {
            // make sure enough is left
            if (length > this->getBuffLeft())
            {
                return FW_DESERIALIZE_SIZE_MISMATCH;
            }

            (void) memcpy(buff, &this->getBuffAddr()[this->m_deserLoc], length);
        }

        this->m_deserLoc += length;
        return FW_SERIALIZE_OK;
    }

    SerializeStatus SerializeBufferBase::deserialize(Serializable &val)
    {
        return val.deserialize(*this);
    }

    SerializeStatus SerializeBufferBase::deserialize(SerializeBufferBase &val)
    {

        FW_ASSERT(val.getBuffAddr());
        SerializeStatus stat = FW_SERIALIZE_OK;

        FwBuffSizeType storedLength;

        stat = this->deserialize(storedLength);

        if (stat != FW_SERIALIZE_OK)
        {
            return stat;
        }

        // make sure destination has enough room

        if ((storedLength > val.getBuffCapacity()) or (storedLength > this->getBuffLeft()))
        {
            return FW_DESERIALIZE_SIZE_MISMATCH;
        }

        FW_ASSERT(this->getBuffAddr());
        (void) memcpy(val.getBuffAddr(), &this->getBuffAddr()[this->m_deserLoc],
                      storedLength);

        stat = val.setBuffLen(storedLength);

        if (stat != FW_SERIALIZE_OK)
        {
            return stat;
        }

        this->m_deserLoc += storedLength;

        return FW_SERIALIZE_OK;
    }

    void SerializeBufferBase::resetSer()
    {
        this->m_deserLoc = 0;
        this->m_serLoc = 0;
    }

    void SerializeBufferBase::resetDeser()
    {
        this->m_deserLoc = 0;
    }

    SerializeStatus SerializeBufferBase::deserializeSkip(unsigned numBytesToSkip)
    {
        // check for room
        if (this->getBuffLength() == this->m_deserLoc)
        {
            return FW_DESERIALIZE_BUFFER_EMPTY;
        }
        else if (this->getBuffLength() - this->m_deserLoc < numBytesToSkip)
        {
            return FW_DESERIALIZE_SIZE_MISMATCH;
        }
        // update location in buffer to skip the value
        this->m_deserLoc += numBytesToSkip;
        return FW_SERIALIZE_OK;
    }

    unsigned SerializeBufferBase::getBuffLength() const
    {
        return this->m_serLoc;
    }

    SerializeStatus SerializeBufferBase::setBuff(const u8* src, unsigned length)
    {
        if (this->getBuffCapacity() < length)
        {
            return FW_SERIALIZE_NO_ROOM_LEFT;
        }
        else
        {
            FW_ASSERT(src);
            FW_ASSERT(this->getBuffAddr());
            memcpy(this->getBuffAddr(), src, length);
            this->m_serLoc = length;
            this->m_deserLoc = 0;
            return FW_SERIALIZE_OK;
        }
    }

    SerializeStatus SerializeBufferBase::setBuffLen(unsigned length)
    {
        if (this->getBuffCapacity() < length)
        {
            return FW_SERIALIZE_NO_ROOM_LEFT;
        }
        else
        {
            this->m_serLoc = length;
            this->m_deserLoc = 0;
            return FW_SERIALIZE_OK;
        }
    }

    unsigned SerializeBufferBase::getBuffLeft() const
    {
        return this->m_serLoc - this->m_deserLoc;
    }

    SerializeStatus SerializeBufferBase::copyRaw(SerializeBufferBase &dest, unsigned size)
    {
        // make sure there is sufficient size in destination
        if (dest.getBuffCapacity() < size)
        {
            return FW_SERIALIZE_NO_ROOM_LEFT;
        }
        // otherwise, set destination buffer to data from deserialization pointer plus size
        SerializeStatus stat = dest.setBuff(&this->getBuffAddr()[this->m_deserLoc], size);
        if (stat == FW_SERIALIZE_OK)
        {
            this->m_deserLoc += size;
        }
        return stat;

    }

    SerializeStatus SerializeBufferBase::copyRawOffset(SerializeBufferBase &dest, unsigned size)
    {
        // make sure there is sufficient size in destination
        if (dest.getBuffCapacity() < size + dest.getBuffLength())
        {
            return FW_SERIALIZE_NO_ROOM_LEFT;
        }
        // make sure there is sufficient buffer in source
        if (this->getBuffLeft() < size)
        {
            return FW_DESERIALIZE_SIZE_MISMATCH;
        }

        // otherwise, serialize bytes to destination without writing length
        SerializeStatus stat = dest.serialize(&this->getBuffAddr()[this->m_deserLoc], size, true);
        if (stat == FW_SERIALIZE_OK)
        {
            this->m_deserLoc += size;
        }
        return stat;

    }

    // return address of buffer not yet deserialized. This is used
    // to copy the remainder of a buffer.
    const u8* SerializeBufferBase::getBuffAddrLeft() const
    {
        return &this->getBuffAddr()[this->m_deserLoc];
    }

    //!< gets address of end of serialization. Used to manually place data at the end
    u8* SerializeBufferBase::getBuffAddrSer()
    {
        return &this->getBuffAddr()[this->m_serLoc];
    }

#ifdef BUILD_UT
    bool SerializeBufferBase::operator==(const SerializeBufferBase& other) const {
        if (this->getBuffLength() != other.getBuffLength()) {
            return false;
        }

        const u8* us = this->getBuffAddr();
        const u8* them = other.getBuffAddr();

        FW_ASSERT(us);
        FW_ASSERT(them);

        for (unsigned byte = 0; byte < this->getBuffLength(); byte++) {
            if (us[byte] != them[byte]) {
                return false;
            }
        }

        return true;
    }

    std::ostream& operator<<(std::ostream& os, const SerializeBufferBase& buff) {

        const u8* us = buff.getBuffAddr();

        FW_ASSERT(us);

        for (unsigned byte = 0; byte < buff.getBuffLength(); byte++) {
            os << "[" << std::setw(2) << std::hex << std::setfill('0') << us[byte] << "]" << std::dec;
        }

        return os;
    }
#endif

    ExternalSerializeBuffer::ExternalSerializeBuffer(u8* buffPtr, unsigned size)
    {
        this->setExtBuffer(buffPtr, size);
    }

    ExternalSerializeBuffer::ExternalSerializeBuffer()
    {
        this->clear();
    }

    void ExternalSerializeBuffer::setExtBuffer(u8* buffPtr, unsigned size)
    {
        FW_ASSERT(buffPtr);
        this->m_buff = buffPtr;
        this->m_buffSize = size;
    }

    void ExternalSerializeBuffer::clear()
    {
        this->m_buff = nullptr;
        this->m_buffSize = 0;
    }

    unsigned ExternalSerializeBuffer::getBuffCapacity() const
    {
        return this->m_buffSize;
    }

    u8* ExternalSerializeBuffer::getBuffAddr()
    {
        return this->m_buff;
    }

    const u8* ExternalSerializeBuffer::getBuffAddr() const
    {
        return this->m_buff;
    }

}


