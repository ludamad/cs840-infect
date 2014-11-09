#ifndef DATAREADWRITE_H_
#define DATAREADWRITE_H_

#include <cstdio>
#include <iostream>
#include <string>

#include <map>

#include "SerializeBuffer.h"
#include "smartptr.h"

#include "customassert.h"

/* Passed to objects visit() method during writing: */
struct DataWriter {
	DataWriter(const std::string& fname) {
		FILE* file = fopen(fname.c_str(), "wb"); // write-binary
		buffer.set( new SerializeBuffer(file, SerializeBuffer::OUTPUT, /* Close file: */ true));
	}

    template <typename T>
    void visit(const T& obj) {
        buffer->write(obj);
    }

    template <typename T>
    void visit(std::vector<T>& obj) {
        buffer->write_container(obj);
    }

    template <typename T>
    void visit(const std::vector<T>& obj) {
        buffer->write_container(obj);
    }

    /**
     * 'check_visit' saves a value, to be checked up reload.
     * This should be some derived quantity, to be used as a sanity check.
     */
    template <typename T>
    void check_visit(const T& obj) {
        visit(obj);
    }

    template <typename T>
    void visit_smartptr(const smartptr<T>& obj) {
        if (obj.empty()) {
            *this << (void*)NULL << false;
            return;
        }
        void* raw_ptr = (void*)obj.get();
        bool added = false;
        if (ptr_map[raw_ptr].empty()) {
            added = true;
        }
        *this << raw_ptr << added;
//        printf("Placing %p add=%d\n", raw_ptr, added);
        if (added) {
            ptr_map[raw_ptr] = smartptr<void*>(obj);
            obj->visit(*this);
        }
    }

    template <typename T>
    DataWriter& operator<<(const T& obj) {
        visit(obj);
        return *this;
    }

    template <typename T>
    void write_container(T& obj) {
        buffer->write_container(obj);
    }

    template <typename T>
    void visit_container(T& obj) {
        buffer->write_container(obj);
    }

    template <typename T>
    void visit_objs(std::vector<T>& objs) {
        *this << objs.size();
        for (T& obj : objs) {
            obj.visit(*this);
        }
    }

    template <typename T>
    void visit_size(T& obj) {
        size_t size = obj.size();
        (*this) << size;
    }

    bool is_reading() {
        return false;
    }

    bool is_writing() {
        return true;
    }

    void visit_raw(const char* data, size_t n) {
        buffer->write_raw(data, n);
    }

private:
    std::map<void*, smartptr<void*>> ptr_map;
    smartptr<SerializeBuffer> buffer;
};

/* Passed to objects visit() method during reading: */
struct DataReader {
	DataReader(const std::string& fname) {
        FILE* file = fopen(fname.c_str(), "rb"); // read-binary
        buffer.set(new SerializeBuffer(file, SerializeBuffer::INPUT, /* Close file: */true));
    }

    template <typename T>
    DataReader& operator<<(T& obj) {
        visit(obj);
        return *this;
    }

    template <typename T>
    void visit(T& obj) {
        buffer->read(obj);
    }

    /**
     * 'check_visit' saves a value, to be checked up reload.
     * This should be some derived quantity, to be used as a sanity check.
     */
    template <typename T>
    void check_visit(const T& obj) {
        T val;
        visit(val);
        ASSERT(val == obj, "Values do not match!");
    }

    template <typename T>
    void visit_smartptr(smartptr<T>& obj) {
        void* old_ptr = NULL;
        bool added = false;
        (*this) << old_ptr << added;
//        printf("Retrieving %p add=%d\n", old_ptr);
        if (added) {
//            printf("Recreating %p\n", old_ptr);
            T val;
            val.visit(*this);
            obj.set(new T(val));
            ptr_map[old_ptr] = smartptr<void*>(obj);
        } else if (old_ptr == NULL) {
            obj.clear();
        } else {
            obj = ptr_map[old_ptr];
        }
    }

    template <typename T>
    void visit(std::vector<T>& obj) {
        buffer->read_container(obj);
    }

    template <typename T>
    void visit_objs(std::vector<T>& objs) {
        size_t size;
        *this << size;
        objs.resize(size);
        for (T& obj : objs) {
            obj.visit(*this);
        }
    }

    template <typename T>
    void visit_size(T& obj) {
        size_t size = -1;
        (*this) << size;
        obj.resize(size);
    }

    template <typename T>
    void write_container(T& obj) {
        throw "Not intended for writing!";
    }

    template <typename T>
    void visit_container(T& obj) {
        buffer->read_container(obj);
    }

    template <typename T>
    void operator<<(const T& obj) {
        visit(obj);
    }

    bool is_reading() {
        return true;
    }

    bool is_writing() {
        return false;
    }

private:
    std::map<void*, smartptr<void*>> ptr_map;
    smartptr<SerializeBuffer> buffer;
    void visit_raw(char* data, size_t n) {
        buffer->read_raw(data, n);
    }
};

// Read-write macro with 1 variable:
#define READ_WRITE1(rw, context) \
    template <typename Visitor, typename Context> \
    void visit(Visitor& rw, Context& context)

#define READ_WRITE(rw) \
    template <typename Visitor> \
    void visit(Visitor& rw)

#define PREREAD_WRITE(rw) \
    template <typename Visitor> \
    void previsit(Visitor& rw)


template <typename Visitor, typename Set>
inline void visit_set(Visitor& rw, Set& set) {
    if (rw.is_reading()) {
        size_t size = -1;
        rw << size;
        for (int i = 0 ; i < size; i++) {
            int used;
            rw << used;
            set.insert(used);
        }
    } else {
        // Writing:
        rw << set.size();
        for (int used : set) {
            rw << used;
        }
    }
}

#endif
