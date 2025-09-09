//
// Created by liaohy on 8/31/25.
//

#pragma once

#include <cstdint>
#include <atomic>
#include <cstring>
#include <vector>

template<uint32_t N, typename T = uint8_t, typename Container = std::vector<T>>
class RingBuffer {
public:
    RingBuffer() : read_handle(0), write_handle(0), buffer(up_power_of_2(N)) {};

    const Container& get_container() const {return buffer;}

    Container get_valid_container() const {
        uint32_t len = size();
        Container ret(len);
        const uint32_t first_chunk = std::min(capacity() - (read_handle & mask()), len);
        const uint32_t second_chunk = len - first_chunk;

        // 复制第一段数据（可能是全部）
        std::memcpy(static_cast<T *>(ret.data()), &buffer[read_handle & mask()], first_chunk * sizeof(T));

        // 如果有第二段数据（环绕情况），从缓冲区开头复制
        if (second_chunk > 0) {
            std::memcpy(static_cast<T *>(ret.data()) + first_chunk, &buffer[0], second_chunk * sizeof(T));
        }

        return ret;
    }

    bool is_empty() const { return write_handle == read_handle; }
    bool is_full() const { return (write_handle + 1) & mask() == read_handle; }

    // 返回缓冲区掩码（总是 2 的幂次方减 1）
    static consteval uint32_t mask() { return capacity() - 1; }

    // 返回缓冲区实际容量（向上取整为 2 的幂次方）
    static consteval uint32_t capacity() { return up_power_of_2(N); }

    // 返回当前缓冲区中的元素数量
    uint32_t size() const {
        return (capacity() + write_handle - read_handle) & mask();
    }

    // 返回当前可用空间数量
    uint32_t available() const {
        return mask() - size();
    }

    uint32_t get_read_index() const { return read_handle; }
    uint32_t get_write_index() const { return write_handle; }

    // 安全地访问元素，确保索引在范围内
    T operator[](const uint32_t index) const {
        if (index >= size()) {
            // 可以选择抛出异常或返回默认值
            return T{};
        }
        return buffer[(read_handle + index) & mask()];
    }

    T get() const { return buffer[read_handle]; }
    T* get_read_point() { return &buffer[read_handle]; }

    bool write_data(const T* data, uint32_t len) {
        if (available() < len)
            return false;

        const uint32_t first_chunk = std::min(capacity() - (write_handle & mask()), len);
        const uint32_t second_chunk = len - first_chunk;

        // 复制第一段数据（可能是全部）
        std::memcpy(&buffer[write_handle & mask()], data, first_chunk * sizeof(T));

        // 如果有第二段数据（环绕情况），复制到缓冲区开头
        if (second_chunk > 0) {
            std::memcpy(&buffer[0], data + first_chunk, second_chunk * sizeof(T));
        }

        // 更新写指针
        write_handle = (write_handle + len) & mask();
        return true;
    }

    bool write_data_force(const T* data, uint32_t len) {
        if (available() < len)
            read_handle = (read_handle + len - available()) & mask();

        const uint32_t first_chunk = std::min(capacity() - (write_handle & mask()), len);
        const uint32_t second_chunk = len - first_chunk;

        // 复制第一段数据（可能是全部）
        std::memcpy(&buffer[write_handle & mask()], data, first_chunk * sizeof(T));

        // 如果有第二段数据（环绕情况），复制到缓冲区开头
        if (second_chunk > 0) {
            std::memcpy(&buffer[0], data + first_chunk, second_chunk * sizeof(T));
        }

        // 更新写指针
        write_handle = (write_handle + len) & mask();
        return true;
    }

    bool get_data(T* data, uint32_t len) {
        if (size() < len)
            return false;

        const uint32_t first_chunk = std::min(capacity() - (read_handle & mask()), len);
        const uint32_t second_chunk = len - first_chunk;

        // 复制第一段数据（可能是全部）
        std::memcpy(data, &buffer[read_handle & mask()], first_chunk * sizeof(T));

        // 如果有第二段数据（环绕情况），从缓冲区开头复制
        if (second_chunk > 0) {
            std::memcpy(data + first_chunk, &buffer[0], second_chunk * sizeof(T));
        }

        // 更新读指针
        read_handle = (read_handle + len) & mask();
        return true;
    }

    void add_write(uint32_t len) { write_handle = (write_handle + len) & mask(); }
    void add_read(uint32_t len = 1) { read_handle = (read_handle + len) & mask(); }

    void reset() {
        read_handle = 0;
        write_handle = 0;
    }

private:
    static consteval uint32_t up_power_of_2(uint32_t number) {
        if (number == 0)
            return 2;
        number--;
        for (uint32_t i = 1; i < sizeof(uint32_t) * 8; i <<= 1)
            number |= number >> i;
        return number + 1;
    }

    Container buffer;

    // 考虑使用原子变量以支持多线程访问
    // 对于多线程环境，取消下面的注释并注释掉普通变量
    // std::atomic<uint32_t> write_handle;
    // std::atomic<uint32_t> read_handle;

    uint32_t write_handle;
    uint32_t read_handle;
};

template<typename T = uint8_t, typename Container = std::vector<T>>
class RingBufferDynamic {
public:
    explicit RingBufferDynamic(uint32_t N) : read_handle(0), write_handle(0), buffer(up_power_of_2(N)), all_length(up_power_of_2(N)) {};

    const Container& get_container() const {return buffer;}

    Container get_valid_container() const {
        uint32_t len = size();
        Container ret(len);
        const uint32_t first_chunk = std::min(capacity() - (read_handle & mask()), len);
        const uint32_t second_chunk = len - first_chunk;

        // 复制第一段数据（可能是全部）
        std::memcpy(static_cast<T *>(ret.data()), &buffer[read_handle & mask()], first_chunk * sizeof(T));

        // 如果有第二段数据（环绕情况），从缓冲区开头复制
        if (second_chunk > 0) {
            std::memcpy(static_cast<T *>(ret.data()) + first_chunk, &buffer[0], second_chunk * sizeof(T));
        }

        return ret;
    }

    void change_capacity(uint32_t N) {
        auto l = up_power_of_2(N);
        buffer.resize(l);
        all_length = l;
    }

    bool is_empty() const { return write_handle == read_handle; }
    bool is_full() const { return ((write_handle + 1) & mask()) == read_handle; }

    // 返回缓冲区掩码（总是 2 的幂次方减 1）
    uint32_t mask() const { return capacity() - 1; }

    // 返回缓冲区实际容量（向上取整为 2 的幂次方）
    uint32_t capacity() const { return all_length; }

    // 返回当前缓冲区中的元素数量
    uint32_t size() const {
        return (capacity() + write_handle - read_handle) & mask();
    }

    // 返回当前可用空间数量
    uint32_t available() const {
        return mask() - size();
    }

    uint32_t get_read_index() const { return read_handle; }
    uint32_t get_write_index() const { return write_handle; }

    // 安全地访问元素，确保索引在范围内
    T operator[](const uint32_t index) const {
        if (index >= size()) {
            // 可以选择抛出异常或返回默认值
            return T{};
        }
        return buffer[(read_handle + index) & mask()];
    }

    T get() const { return buffer[read_handle]; }
    T* get_read_point() { return &buffer[read_handle]; }

    bool write_data(const T* data, uint32_t len) {
        if (available() < len)
            return false;

        const uint32_t first_chunk = std::min(capacity() - (write_handle & mask()), len);
        const uint32_t second_chunk = len - first_chunk;

        // 复制第一段数据（可能是全部）
        std::memcpy(&buffer[write_handle & mask()], data, first_chunk * sizeof(T));

        // 如果有第二段数据（环绕情况），复制到缓冲区开头
        if (second_chunk > 0) {
            std::memcpy(&buffer[0], data + first_chunk, second_chunk * sizeof(T));
        }

        // 更新写指针
        write_handle = (write_handle + len) & mask();
        return true;
    }

    bool write_data_force(const T* data, uint32_t len) {
        if (available() < len)
            read_handle = (read_handle + len - available()) & mask();

        const uint32_t first_chunk = std::min(capacity() - (write_handle & mask()), len);
        const uint32_t second_chunk = len - first_chunk;

        // 复制第一段数据（可能是全部）
        std::memcpy(&buffer[write_handle & mask()], data, first_chunk * sizeof(T));

        // 如果有第二段数据（环绕情况），复制到缓冲区开头
        if (second_chunk > 0) {
            std::memcpy(&buffer[0], data + first_chunk, second_chunk * sizeof(T));
        }

        // 更新写指针
        write_handle = (write_handle + len) & mask();
        return true;
    }

    bool get_data(T* data, uint32_t len) {
        if (size() < len)
            return false;

        const uint32_t first_chunk = std::min(capacity() - (read_handle & mask()), len);
        const uint32_t second_chunk = len - first_chunk;

        // 复制第一段数据（可能是全部）
        std::memcpy(data, &buffer[read_handle & mask()], first_chunk * sizeof(T));

        // 如果有第二段数据（环绕情况），从缓冲区开头复制
        if (second_chunk > 0) {
            std::memcpy(data + first_chunk, &buffer[0], second_chunk * sizeof(T));
        }

        // 更新读指针
        read_handle = (read_handle + len) & mask();
        return true;
    }

    void add_write(uint32_t len) { write_handle = (write_handle + len) & mask(); }
    void add_read(uint32_t len = 1) { read_handle = (read_handle + len) & mask(); }

    void reset() {
        read_handle = 0;
        write_handle = 0;
    }

private:
    static constexpr uint32_t up_power_of_2(uint32_t number) {
        if (number == 0)
            return 2;
        number--;
        for (uint32_t i = 1; i < sizeof(uint32_t) * 8; i <<= 1)
            number |= number >> i;
        return number + 1;
    }

    uint32_t all_length;
    Container buffer;

    // 考虑使用原子变量以支持多线程访问
    // 对于多线程环境，取消下面的注释并注释掉普通变量
    // std::atomic<uint32_t> write_handle;
    // std::atomic<uint32_t> read_handle;

    uint32_t write_handle;
    uint32_t read_handle;
};