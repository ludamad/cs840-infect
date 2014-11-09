/*
 * SerializeBuffer.h:
 *  Buffer for serialization, with a buffer-full callback
 */

#ifndef SERIALIZEBUFFER_H_
#define SERIALIZEBUFFER_H_

#include <stdexcept>
#include <string>
#include <cstring>
#include <vector>
#include <cstdio>
#include "int_types.h"
#include "strformat.h"

#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#ifdef __APPLE__
   static inline unsigned long long be64toh(unsigned long long value) {
        int num = 42;
        if (*(char *)&num == 42) {
            uint32_t high_part = htonl((uint32_t)(value >> 32));
            uint32_t low_part = htonl((uint32_t)(value & 0xFFFFFFFFLL));
            return (((uint64_t)low_part) << 32) | high_part;
        } else {
            return value;
        }
    }
#  define be16toh(x) htons(x)
#  define be32toh(x) htonl(x)
#else
# if __BYTE_ORDER == __LITTLE_ENDIAN
#  define htobe16(x) __bswap_16 (x)
#  define htole16(x) (x)
#  define be16toh(x) __bswap_16 (x)
#  define le16toh(x) (x)

#  define htobe32(x) __bswap_32 (x)
#  define htole32(x) (x)
#  define be32toh(x) __bswap_32 (x)
#  define le32toh(x) (x)

#   define htobe64(x) __bswap_64 (x)
#   define htole64(x) (x)
#   define be64toh(x) __bswap_64 (x)
#   define le64toh(x) (x)
#else
#  define htobe16(x) (x)
#  define htole16(x) __bswap_16 (x)
#  define be16toh(x) (x)
#  define le16toh(x) __bswap_16 (x)

#  define htobe32(x) (x)
#  define htole32(x) __bswap_32 (x)
#  define be32toh(x) (x)
#  define le32toh(x) __bswap_32 (x)

#   define htobe64(x) (x)
#   define htole64(x) __bswap_64 (x)
#   define be64toh(x) (x)
#   define le64toh(x) __bswap_64 (x)
#endif
#endif
const int MAX_ALLOC_SIZE = 1024 * 1024; //1MB
const int MAX_BUFFER_SIZE = 128 * 1024; //128kb

typedef void (*buffer_flushf)(void* context, const char* data, size_t size);
typedef void (*buffer_fillf)(void* context, std::vector<char>& buffer,
		size_t maxsize);
typedef void (*buffer_closef)(void* context);

class SerializeBufferError : public std::runtime_error {
public:
	SerializeBufferError(const std::string& msg) : runtime_error(msg) {
	}
};

#ifndef LCOMMON_NO_EXCEPTIONS

inline void serialize_buffer_error(const std::string& msg) {
	throw SerializeBufferError(msg);
}

#else

inline void serialize_buffer_error(const std::string& msg) {
	fprintf(stderr, "Serialization error has occurred: %s\n", msg.c_str());
	abort();
}

#endif

#define LSERIALIZE_CHECK(check) \
	if (!(check)) { serialize_buffer_error( format( "SerializeBuffer line %d: %s", __LINE__, #check) ); }

class SerializeBuffer {
public:
	enum IOType {
		INPUT,
		OUTPUT
	};

	SerializeBuffer(void* context, buffer_flushf flushf, buffer_fillf fillf, buffer_closef closef);
	SerializeBuffer();
	SerializeBuffer(FILE* file, IOType type, bool close_file = false);

	// Low level fetch, guaranteed to be in buffer:
	char* fetch_raw(size_t n) {
		if (_buffer.size() - _read_position < n) {
			fill();
		}
		LSERIALIZE_CHECK(_buffer.size() >= n + _read_position);
		char* ref = &_buffer[_read_position];
		_read_position += n;
		return ref;
	}

	// Low level read/writes:
	void read_raw(char* bytes, size_t n) {
		memcpy(bytes, fetch_raw(n), n);
	}

	void write_raw(const char* bytes, size_t n) {
		if (_buffer.size() + n > MAX_BUFFER_SIZE) {
			flush();
		}
		_buffer.insert(_buffer.end(), bytes, bytes + n);
	}

	// Specialized write/reads:
	void write_int(int32_t i) {
		write(i);
	}
	int32_t read_int() {
		int32_t integer;
		read(integer);
		return integer;
	}
	template<class T>
	void read_int(T& t) {
		t = (T)read_int();
	}
	double read_double() {
		double d;
		read(d);
		return d;
	}
	void write_double(double d) {
		write(d);
	}
	void write_byte(int i) {
		write((char)i);
	}

	char read_byte() {
		char byte;
		read(byte);
		return byte;
	}

	void write(const std::string& str) {
		write((int)str.size());
		write_raw(str.c_str(), str.size());
	}
	void write_str(const char* str) {
		int len = (int)strlen(str);
		write(len);
		write_raw(str, len);
	}
	void read(std::vector<bool>::reference ref) {
		// For vector<bool>! Blah.
		bool bit;
		read(bit);
		ref = bit;
	}
	void read(std::string& str) {
		int size;
		read(size);
		LSERIALIZE_CHECK(size_t(size) < MAX_ALLOC_SIZE);
		str.resize(size);
		if (size > 0) {
			read_raw(&str[0], size);
		}
	}

	std::string read_str() {
		std::string str;
		read(str);
		return str;
	}

	// Will endian-swap any values of 2, 4, 8 byte-length.
	// Should only be passed atomic elements ever!
	// TODO: Error on different passed lengths
	template<class T>
	void read(T& t) {
		read_raw((char*)&t, sizeof(T));
		if (sizeof(T) == 2) {
			*(uint16_t*)&t = be16toh(*(uint16_t*)&t);
		} else if (sizeof(T) == 4) {
			*(uint32_t*)&t = be32toh(*(uint32_t*)&t);
		} else if (sizeof(T) == 8) {
			*(uint64_t*)&t = be64toh(*(uint64_t*)&t);
		}
	}

	// Will endian-swap any values of 2, 4, 8 byte-length.
	// Should only be passed atomic elements ever!
	// TODO: Error on different passed lengths
	template<class T>
	void write(const T& t) {
		if (sizeof(T) == 2) {
			uint16_t val = be16toh(*(uint16_t*)&t);
			write_raw((const char*)&val, sizeof(T));
		} else if (sizeof(T) == 4) {
			uint32_t val = be32toh(*(uint32_t*)&t);
			write_raw((const char*)&val, sizeof(T));
		} else if (sizeof(T) == 8) {
			uint64_t val = be64toh(*(uint64_t*)&t);
			write_raw((const char*)&val, sizeof(T));
		} else {
			write_raw((const char*)&t, sizeof(T));
		}
	}
	// High-level read/writes:
	template<class T>
	void write_container(const T& t) {
		write((int)t.size());
        write_raw((const char*)&t[0], t.size() * sizeof(typename T::value_type));
	}
    template<class T>
    void write_container(T& t) {
        write((int)t.size());
        write_raw((const char*)&t[0], t.size() * sizeof(typename T::value_type));
    }
	template<class T>
	void read_container(T& t) {
		int size;
		read(size);
		LSERIALIZE_CHECK(size_t(size * sizeof(typename T::value_type)) < MAX_ALLOC_SIZE);
		t.resize(size);
        read_raw((char*)&t[0], size * sizeof(typename T::value_type));
	}


	~SerializeBuffer();

	const char* data() const {
		return &_buffer[0];
	}
	size_t size() const {
		return _buffer.size();
	}
	int read_position() const {
		return _read_position;
	}

	void close();
	void clear();
	void flush();
	void fill();
	void move_read_position(int len) {
		_read_position += len;
		LSERIALIZE_CHECK(_read_position >= 0);
	}
	bool empty() const {
		return _buffer.empty();
	}

private:
	std::vector<char> _buffer;
	int _read_position;
	void* _context;
	buffer_flushf _flushf;
	buffer_fillf _fillf;
	buffer_closef _closef;
};

// The following functions are hacks for quick and dirty serialization of known POD regions
// Writes plain-old-data
#define SERIALIZE_POD_REGION(serializer, obj, member1, member2) \
	serializer.write_raw( (char*) (& obj-> member1 ),\
		(char*)(1 + & obj-> member2) - (char*)(& obj-> member1))

// Reads plain-old-data
#define DESERIALIZE_POD_REGION(serializer, obj, member1, member2) \
	serializer.read_raw( (char*) (& obj-> member1 ),\
		(char*)(1 + & obj-> member2) - (char*)(& obj-> member1))

template<typename T>
inline bool serializer_equals_read(SerializeBuffer& sb, const T& value) {
	T t;
	sb.read(t);
	return t == value;
}

#endif /* SERIALIZEBUFFER_H_ */
