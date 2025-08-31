//
// Created by liaohy on 8/12/25.
//

#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <format>
#include <algorithm>

struct USBDevDesc {
    uint16_t vendor_id;
    uint16_t product_id;
    std::string product_string;

    uint16_t interface_num;
    std::string interface_name;

    uint8_t bulk_in_endpoints;
    uint8_t bulk_out_endpoints;

    bool is_daplink();

    template<typename T>
    friend T& operator<<(T& ostream, const USBDevDesc& dev);
};

template<typename T>
T& operator<<(T& ostream, const USBDevDesc &dev) {
    ostream << "product_string: " << dev.product_string << "\n";
    ostream << "vendor_id: " << std::format("{:x}", dev.vendor_id) << "\n";
    ostream << "product_id: " << std::format("{:x}", dev.product_id) << "\n";
    ostream << "interface_num: " << dev.interface_num << "\n";
    ostream << "interface_name: " << dev.interface_name << "\n";
    ostream << "bulk_in_endpoints: " << static_cast<uint32_t>(dev.bulk_in_endpoints) << "\n";
    ostream << "bulk_out_endpoints: " << static_cast<uint32_t>(dev.bulk_out_endpoints) << "\n";
    return ostream;
}

bool is_usb_bulk(const std::string &str);
