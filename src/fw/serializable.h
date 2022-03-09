#ifndef SERIALIZABLE_HPP
#define SERIALIZABLE_HPP

// Taken directly from FPrime

#include <circle/types.h>

namespace Fw
{
    enum
    {
        FW_SERIALIZE_FALSE_VALUE = 0x0,
        FW_SERIALIZE_TRUE_VALUE = 0xaa
    };

    typedef uintptr POINTER_CAST;
    typedef size_t FwBuffSizeType;

    typedef enum
    {
        FW_SERIALIZE_OK, //!< Serialization/Deserialization operation was successful
        FW_SERIALIZE_FORMAT_ERROR, //!< Data was the wrong format (e.g. wrong packet type)
        FW_SERIALIZE_NO_ROOM_LEFT,  //!< No room left in the buffer to serialize data
        FW_DESERIALIZE_BUFFER_EMPTY, //!< Deserialization buffer was empty when trying to read more data
        FW_DESERIALIZE_FORMAT_ERROR, //!< Deserialization data had incorrect values (unexpected data types)
        FW_DESERIALIZE_SIZE_MISMATCH, //!< Data was left in the buffer, but not enough to deserialize
        FW_DESERIALIZE_TYPE_MISMATCH //!< Deserialized type ID didn't match
    } SerializeStatus;

    class SerializeBufferBase; //!< forward declaration

    class Serializable
    {
    public:
        virtual SerializeStatus serialize(SerializeBufferBase &buffer) const = 0; //!< serialize contents
        virtual SerializeStatus deserialize(SerializeBufferBase &buffer) = 0; //!< deserialize to contents

    protected:
        Serializable(); //!< Default constructor
        virtual ~Serializable(); //!< destructor
    };

    class SerializeBufferBase
    {
    public:

        SerializeBufferBase &operator=(const SerializeBufferBase &src); //!< equal operator

        virtual ~SerializeBufferBase(); //!< destructor

        // Serialization for built-in types

        SerializeStatus serialize(u8 val); //!< serialize 8-bit unsigned int
        SerializeStatus serialize(s8 val); //!< serialize 8-bit signed int

        SerializeStatus serialize(u16 val); //!< serialize 16-bit unsigned int
        SerializeStatus serialize(s16 val); //!< serialize 16-bit signed int
        SerializeStatus serialize(u32 val); //!< serialize 32-bit unsigned int
        SerializeStatus serialize(s32 val); //!< serialize 32-bit signed int
        SerializeStatus serialize(u64 val); //!< serialize 64-bit unsigned int
        SerializeStatus serialize(s64 val); //!< serialize 64-bit signed int
        SerializeStatus serialize(f32 val); //!< serialize 32-bit floating point
        SerializeStatus serialize(f64 val); //!< serialize 64-bit floating point
        SerializeStatus serialize(bool val); //!< serialize boolean

        SerializeStatus
        serialize(const void* val); //!< serialize pointer (careful, only pointer value, not contents are serialized)

        SerializeStatus serialize(const u8* buff, unsigned length, bool noLength = false); //!< serialize data buffer

        SerializeStatus serialize(const SerializeBufferBase &val); //!< serialize a serialized buffer

        SerializeStatus
        serialize(const Serializable &val); //!< serialize an object derived from serializable base class

        // Deserialization for built-in types

        SerializeStatus deserialize(u8 &val); //!< deserialize 8-bit unsigned int
        SerializeStatus deserialize(s8 &val); //!< deserialize 8-bit signed int
        SerializeStatus deserialize(u16 &val); //!< deserialize 16-bit unsigned int
        SerializeStatus deserialize(s16 &val); //!< deserialize 16-bit signed int
        SerializeStatus deserialize(u32 &val); //!< deserialize 32-bit unsigned int
        SerializeStatus deserialize(s32 &val); //!< deserialize 32-bit signed int
        SerializeStatus deserialize(u64 &val); //!< deserialize 64-bit unsigned int
        SerializeStatus deserialize(s64 &val); //!< deserialize 64-bit signed int
        SerializeStatus deserialize(f32 &val); //!< deserialize 32-bit floating point
        SerializeStatus deserialize(f64 &val); //!< deserialize 64-bit floating point
        SerializeStatus deserialize(bool &val); //!< deserialize boolean

        SerializeStatus deserialize(void*&val); //!< deserialize point value (careful, pointer value only, not contents)

        // length should be set to max, returned value is actual size stored. If noLength
        // is true, use the length variable as the actual number of bytes to deserialize
        SerializeStatus deserialize(u8* buff, unsigned &length, bool noLength = false); //!< deserialize data buffer
        // serialize/deserialize Serializable


        SerializeStatus deserialize(Serializable &val);  //!< deserialize an object derived from serializable base class

        SerializeStatus deserialize(SerializeBufferBase &val);  //!< serialize a serialized buffer

        void resetSer(); //!< reset to beginning of buffer to reuse for serialization
        void resetDeser(); //!< reset deserialization to beginning

        SerializeStatus
        deserializeSkip(unsigned numBytesToSkip); //!< Skips the number of specified bytes for deserialization
        virtual unsigned getBuffCapacity() const = 0; //!< returns capacity, not current size, of buffer
        unsigned getBuffLength() const; //!< returns current buffer size
        unsigned getBuffLeft() const; //!< returns how much deserialization buffer is left
        virtual u8* getBuffAddr() = 0; //!< gets buffer address for data filling
        virtual const u8* getBuffAddr() const = 0; //!< gets buffer address for data reading, const version
        const u8* getBuffAddrLeft() const; //!< gets address of remaining non-deserialized data.
        u8* getBuffAddrSer(); //!< gets address of end of serialization. DANGEROUS! Need to know max buffer size and adjust when done
        SerializeStatus setBuff(const u8* src, unsigned length); //!< sets buffer contents and size
        SerializeStatus setBuffLen(unsigned length); //!< sets buffer length manually after filling with data
        SerializeStatus copyRaw(SerializeBufferBase &dest,
                                unsigned size); //!< directly copies buffer without looking for a size in the stream.
        // Will increment deserialization pointer
        SerializeStatus copyRawOffset(SerializeBufferBase &dest,
                                      unsigned size); //!< directly copies buffer without looking for a size in the stream.
        // Will increment deserialization pointer


#ifdef BUILD_UT
        bool operator==(const SerializeBufferBase& other) const;
            friend std::ostream& operator<<(std::ostream& os, const SerializeBufferBase& buff);
#endif
    protected:

        SerializeBufferBase(); //!< default constructor

    private:
        // A no-implementation copy constructor here will prevent the default copy constructor from being called
        // accidentally, and without an implementation it will create an error for the developer instead.
        SerializeBufferBase(const SerializeBufferBase &src); //!< constructor with buffer as source

        void copyFrom(const SerializeBufferBase &src); //!< copy data from source buffer
        unsigned m_serLoc; //!< current offset in buffer of serialized data
        unsigned m_deserLoc; //!< current offset for deserialization
    };

    // Helper class for building buffers with external storage

    class ExternalSerializeBuffer : public SerializeBufferBase
    {
    public:
        ExternalSerializeBuffer(u8* buffPtr, unsigned size); //!< construct with external buffer
        ExternalSerializeBuffer(); //!< default constructor
        void setExtBuffer(u8* buffPtr, unsigned size); //!< Set the external buffer
        void clear(); //!< clear external buffer

        // pure virtual functions
        unsigned getBuffCapacity() const override;

        u8* getBuffAddr() override;

        const u8* getBuffAddr() const override;

        // no copying
        ExternalSerializeBuffer(ExternalSerializeBuffer &other) = delete;
        ExternalSerializeBuffer(ExternalSerializeBuffer* other) = delete;

    private:

        // private data
        u8* m_buff; //!< pointer to external buffer
        unsigned m_buffSize; //!< size of external buffer
    };

}
#endif
