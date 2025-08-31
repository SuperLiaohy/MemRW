//
// Created by liaohy on 8/23/25.
//

#pragma once

#include <array>
#include "USBDevDesc.h"
#include <libusb.h>
#include <memory>

class USBBulk {
public:
    USBDevDesc desc;

    static int init() {
        if (ctx == nullptr) return libusb_init(&ctx);
        return true;
    }

    static libusb_context *Context() { return ctx; }

    static void exit() {
        if (ctx != nullptr) {
            libusb_exit(ctx);
            ctx = nullptr;
        }
    }

    explicit USBBulk(const USBDevDesc &desc) : desc(desc) {
        dev = libusb_open_device_with_vid_pid(Context(), desc.vendor_id, desc.product_id);
        if (!dev) throw std::runtime_error("Could not open usb bulk.");
        if (libusb_claim_interface(dev, desc.interface_num)!=0) throw std::runtime_error("Could not claim Bulk interface.");
    }

    explicit USBBulk(const std::shared_ptr<USBDevDesc> &Pdesc) : desc(*Pdesc) {
        dev = libusb_open_device_with_vid_pid(Context(), desc.vendor_id, desc.product_id);
        if (!dev) throw std::runtime_error("Could not open usb bulk.");
        if (libusb_claim_interface(dev, desc.interface_num)!=0) throw std::runtime_error("Could not claim Bulk interface.");
    }

    ~USBBulk() {
        if (dev != nullptr) libusb_close(dev);
    }

    int transmit(uint8_t *data, uint32_t len, uint32_t timeout = 1000);
    int transmit(std::vector<uint8_t>& data, uint32_t timeout = 1000) {return transmit(data.data(), data.size(), timeout);}
    template<uint32_t len>
    int transmit(const std::array<uint8_t, len> &data, uint32_t timeout = 1000) {
        return transmit(data.data(), len, timeout);
    }


    int receive(std::vector<uint8_t>& data, uint32_t timeout = 1000);

private:
    static libusb_context *ctx;
    libusb_device_handle *dev = nullptr;
};
