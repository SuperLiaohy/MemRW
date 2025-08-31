//
// Created by liaohy on 8/23/25.
//

#include "USBBulk.h"

libusb_context* USBBulk::ctx = nullptr;

int USBBulk::transmit(uint8_t *data, uint32_t len, uint32_t timeout) {
    int transferred = 0;
    return libusb_bulk_transfer(dev, desc.bulk_out_endpoints, data, len, &transferred, timeout);
}

int USBBulk::receive(std::vector<uint8_t> &data, uint32_t timeout) {
    int transferred = 0;
    auto res = libusb_bulk_transfer(dev, desc.bulk_in_endpoints, data.data(), data.capacity(), &transferred, timeout);
    data.resize(transferred);
    return res;
}