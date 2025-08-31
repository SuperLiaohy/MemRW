//
// Created by liaohy on 8/15/25.
//

#include "DAPReader.h"

#include <bitset>
#include <cstring>
#include <iostream>
#include <thread>

namespace {
    uint32_t DAP_INTERFACE_CLASS = 255;

    uint8_t DAP_SWJ_Clock = 0x11;
    uint8_t DAP_SWJ_Sequence = 0x12;

    uint8_t DAP_Transfer = 0x05;
    uint8_t DAP_TransferBlock = 0x06;

    uint8_t DAP_Connect = 0x02;
}

DAPReader::DAPReader() : response_buffer(255 * 5 + 3) {
    USBBulk::init();
    libusb_device **device_list = nullptr;

    int32_t device_count = libusb_get_device_list(USBBulk::Context(), &device_list);
    if (device_count < 0)
        throw std::runtime_error("Could not find any usb devices.");
    std::vector<std::shared_ptr<USBDevDesc> > devs;
    for (ssize_t i = 0; i < device_count; ++i) {
        libusb_device *device = device_list[i];
        libusb_device_descriptor desc{};
        auto item = std::make_shared<USBDevDesc>();

        // 获取设备描述符
        int result = libusb_get_device_descriptor(device, &desc);
        if (result != LIBUSB_SUCCESS)
            continue;

        // 打开设备以获取字符串描述符
        libusb_device_handle *handle = nullptr;
        result = libusb_open(device, &handle);
        if (result != LIBUSB_SUCCESS || handle == nullptr)
            continue;

        std::string product_string = "N/A";
        // 获取product string (iProduct字段)
        if (desc.iProduct) {
            char string_buffer[256];
            result = libusb_get_string_descriptor_ascii(handle, desc.iProduct,
                                                        reinterpret_cast<unsigned char *>(string_buffer),
                                                        sizeof(string_buffer));
            if (result > 0) {
                product_string = std::string(string_buffer);
            }
        }
        libusb_close(handle);

        item->vendor_id = desc.idVendor;
        item->product_id = desc.idProduct;
        item->product_string = product_string;
        if (item->is_daplink())
            devs.push_back(item);
    }
    libusb_free_device_list(device_list, device_count);

    bool found = false;
    for (auto &usb_desc: devs) {
        libusb_device_handle *handle = nullptr;
        libusb_config_descriptor *config_descriptor = nullptr;
        handle = libusb_open_device_with_vid_pid(USBBulk::Context(), usb_desc->vendor_id, usb_desc->product_id);
        if (!handle) continue;
        int res = libusb_get_active_config_descriptor(libusb_get_device(handle), &config_descriptor);
        if (res != 0) continue;
        for (int i = 0; i < config_descriptor->bNumInterfaces; ++i) {
            const libusb_interface &interface = config_descriptor->interface[i];
            // default use index 0
            if (interface.num_altsetting > 0) {
                const libusb_interface_descriptor *desc = interface.altsetting;
                if (desc->iInterface > 0) {
                    usb_desc->interface_name = "N/A";
                    char name_data[256];
                    if (libusb_get_string_descriptor_ascii(handle, desc->iInterface,
                                                           reinterpret_cast<unsigned char *>(name_data),
                                                           sizeof(name_data)))
                        usb_desc->interface_name = name_data;
                    if (desc->bInterfaceClass == DAP_INTERFACE_CLASS || is_usb_bulk(usb_desc->interface_name)) {
                        usb_desc->interface_num = desc->bInterfaceNumber;
                        for (int j = 0; j < desc->bNumEndpoints; ++j) {
                            if ((desc->endpoint[j].bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_IN)
                                usb_desc->bulk_in_endpoints = desc->endpoint[j].bEndpointAddress;
                            else usb_desc->bulk_out_endpoints = desc->endpoint[j].bEndpointAddress;
                        }
                        found = true;
                        break;
                    }
                }
            }
        }
        libusb_close(handle);
        libusb_free_config_descriptor(config_descriptor);
        if (found == true) {
            usb = std::make_unique<USBBulk>(usb_desc);
            break;
        };
    }
    if (usb == nullptr) throw std::runtime_error("no usb bulk be found.");
    std::cout << usb->desc << std::endl;
}

void DAPReader::attach_to_target() {
    this->dap_connect(DAP::SWD_Port);
    this->set_swj_clock(10'000'000);
    this->set_swj_sequence({0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
    this->set_swj_sequence({0x9E, 0xE7});
    this->set_swj_sequence({0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
    this->set_swj_sequence({0x00});
    DAP::TransferResponse response{};
    this->transfer(DPReadRequest(SW::DP::IDCODE), response);
    std::cout << "DPIDR: " << std::hex << response.data << std::dec << std::endl;
    auto idr = std::bit_cast<SW::DP::IDCODEReg>(response.data);
    std::cout << idr << std::endl;
    SW::DP::ABORTReg abort{};
    abort.DAPABORT = 0;
    abort.STKCMPCLR = 1;
    abort.STKERRCLR = 1;
    abort.WDERRCLR = 1;
    abort.ORUNERRCLR = 1;
    this->transfer(DPWriteRequest(SW::DP::ABORT, std::bit_cast<uint32_t>(abort)), response);
    SW::DP::CTRL_STATReg cs{};
    cs.CSYSPWRUPREQ = 1;
    cs.CDBGPWRUPREQ = 1;
    this->transfer(DPWriteRequest(SW::DP::CTRL_STAT, std::bit_cast<uint32_t>(cs)), response);
    response.data = 0;
    for (int i = 0; i < 10; ++i) {
        this->transfer(DPReadRequest(SW::DP::CTRL_STAT), response);
        auto cs_feedback = std::bit_cast<SW::DP::CTRL_STATReg>(response.data);
        if (cs_feedback.CDBGPWRUPACK == 1 && cs_feedback.CSYSPWRUPACK == 1) {
            std::cout << "Debug interface domain is powered up." << std::endl;
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::cout << "failed to power up debug interface domain." << std::endl;
}

uint32_t DAPReader::read_mem(uint32_t addr) {
    DAP::TransferResponse response{};
    std::vector<DAP::TransferResponse> responses(2);
    // this->transfer(APWriteRequest(SW::MEM_AP::TAR, addr), response);
    // this->transfer(APReadRequest(SW::MEM_AP::DRW), response);
    // if (sw.ap.csw->AddrInc == 0) {
    //     if (sw.ap.tar.value_or(SW::MEM_AP::TARReg(0)).data != addr) {
    //         this->transfer({APWriteRequest(SW::MEM_AP::TAR, addr), APReadRequest(SW::MEM_AP::DRW)}, responses);
    //         sw.ap.tar = std::make_optional<SW::MEM_AP::TARReg>(addr);
    //         return responses[1].data;
    //     }
    //     this->transfer(APReadRequest(SW::MEM_AP::DRW), response);
    //     return response.data;
    // }
    this->transfer({APWriteRequest(SW::MEM_AP::TAR, addr), APReadRequest(SW::MEM_AP::DRW)}, responses);
    // sw.ap.tar = std::make_optional<SW::MEM_AP::TARReg>(addr+4);
    return responses[1].data;
}

void DAPReader::auto_configure_ap() {
    sw.dp.select = std::make_optional<SW::DP::SELECTReg>();
    uint8_t APSEL = 0;
    bool found_MEM_AP = false;
    while (!found_MEM_AP) {
        DAP::TransferResponse response{};
        sw.dp.select->CTRLSEL = 0;
        sw.dp.select->APSEL = APSEL;
        sw.dp.select->APBANKSEL = 0xF;
        this->transfer(DPWriteRequest(SW::DP::SELECT, std::bit_cast<uint32_t>(sw.dp.select.value())), response);
        std::cout << "--------------APSEL:" << static_cast<uint32_t>(APSEL) << "--------------" << std::endl;
        this->transfer(APReadRequest(SW::MEM_AP::IDR), response);
        sw.ap.idr = std::bit_cast<SW::MEM_AP::IDRReg>(response.data);
        std::cout << sw.ap.idr.value() << std::endl;
        if (sw.ap.idr->CLASS == 0b1000) found_MEM_AP = true;
        sw.dp.select->APBANKSEL = 0x0;
        this->transfer(DPWriteRequest(SW::DP::SELECT, std::bit_cast<uint32_t>(sw.dp.select.value())), response);
        this->transfer(APReadRequest(SW::MEM_AP::CSW), response);
        sw.ap.csw = std::bit_cast<SW::MEM_AP::CSWReg>(response.data);
        std::cout << sw.ap.csw.value() << std::endl;
        // csw.DbgSwEnable = 1;
        // csw.AddrInc = 1;
        // this->transfer(APWriteRequest(SW::MEM_AP::CSW, std::bit_cast<uint32_t>(csw)), response);
        // this->transfer(APReadRequest(SW::MEM_AP::CSW), response);
        // csw = std::bit_cast<SW::MEM_AP::CSWReg>(response.data);
        // std::cout << csw << std::endl;
        ++APSEL;
    }
}

int DAPReader::dap_connect(uint8_t port) {
    std::vector<uint8_t> request{DAP_Connect, port};
    usb->transmit(request);
    std::vector<uint8_t> response_buffer(256);
    usb->receive(response_buffer);
    if (response_buffer[0] != DAP_Connect) return DAP::DAP_ERROR;
    return response_buffer[1];
}

int DAPReader::set_swj_clock(uint32_t clock) {
    std::vector<uint8_t> data = {DAP_SWJ_Clock, 0, 0, 0, 0};
    *reinterpret_cast<uint32_t *>(&data[1]) = clock;
    usb->transmit(data);
    std::vector<uint8_t> response(256);
    usb->receive(response);
    if (response[0] != DAP_SWJ_Clock) return DAP::DAP_ERROR;
    return response[1];
}

int DAPReader::set_swj_sequence(const uint8_t *sequence, uint8_t len) {
    std::vector<uint8_t> data(len + 2);
    data[0] = DAP_SWJ_Sequence;
    data[1] = len * 8;
    for (int i = 0; i < len; ++i) {
        data[i + 2] = sequence[i];
    }
    data.resize(len + 2);
    usb->transmit(data.data(), data.size());
    std::vector<uint8_t> response(256);
    usb->receive(response);
    if (response[0] != DAP_SWJ_Sequence) return DAP::DAP_ERROR;
    return response[1];
}

int DAPReader::transfer(const DAP::TransferRequest &requests, DAP::TransferResponse &responses) {
    std::vector<uint8_t> request_buffer(3 + 5);
    request_buffer[0] = DAP_Transfer;
    request_buffer[1] = 0;
    request_buffer[2] = 1;
    uint32_t index = 0;
    if (requests.request.RnW == 1 && requests.request.ValueMatch == 0) {
        request_buffer[3] = std::bit_cast<uint8_t>(requests.request);
        index = 1;
    } else {
        memcpy(&request_buffer[3], &requests, 5);
        index = 5;
    }
    request_buffer.resize(3 + index);
    usb->transmit(request_buffer);

    response_buffer.resize(response_buffer.capacity());
    usb->receive(response_buffer);
    if (response_buffer[0] != DAP_Transfer) return DAP::DAP_ERROR;
    uint8_t TransferCount = response_buffer[1];
    uint32_t *word = reinterpret_cast<uint32_t *>(&response_buffer[3]);
    index = 0;
    if (requests.request.TimeStamp) {
        responses.TimeStamp = word[index++];
    }
    if (requests.request.RnW == 1 && requests.request.ValueMatch == 0)
        responses.data = word[index++];
    return response_buffer[2];
}

int DAPReader::transfer(const std::vector<DAP::TransferRequest> &requests,
                        std::vector<DAP::TransferResponse> &responses) {
    std::vector<uint8_t> request_buffer(3 + requests.size() * 5);
    request_buffer[0] = DAP_Transfer;
    request_buffer[1] = 0;
    request_buffer[2] = requests.size();
    // memcpy(&request_buffer[3],requests.data(),requests.size()*5);
    uint32_t index = 0;
    for (int i = 0; i < requests.size(); ++i) {
        if (requests[i].request.RnW == 1 && requests[i].request.ValueMatch == 0) {
            request_buffer[3 + index] = std::bit_cast<uint8_t>(requests[i].request);
            ++index;
        } else {
            memcpy(&request_buffer[3 + index], &requests[i], 5);
            index += 5;
        }
    }
    request_buffer.resize(3 + index);
    usb->transmit(request_buffer);

    response_buffer.resize(response_buffer.capacity());
    usb->receive(response_buffer);
    if (response_buffer[0] != DAP_Transfer) return DAP::DAP_ERROR;
    uint8_t TransferCount = response_buffer[1];
    uint32_t *word = reinterpret_cast<uint32_t *>(&response_buffer[3]);
    index = 0;
    for (int i = 0; i < TransferCount; ++i) {
        if (requests[i].request.TimeStamp) {
            responses[i].TimeStamp = word[index++];
        }
        if (requests[i].request.RnW == 1 && requests[i].request.ValueMatch == 0)
            responses[i].data = word[index++];
    }
    return response_buffer[2];
}

DAP::TransferRequest DAPReader::Request(uint8_t APnDP, uint8_t RnW, uint8_t reg, uint8_t ValueMatch, uint8_t MatchMask,
                                        uint8_t TimeStamp, uint32_t data) {
    DAP::TransferRequest request{};
    // request.request = APnDP | (RnW<<1) | (reg&0b0000'1100) | (ValueMatch<<4) | (MatchMask<<5) | (TimeStamp<<6);
    request.request.APnDP = APnDP;
    request.request.RnW = RnW;
    request.request.reg = (reg & 0b0000'1100) >> 2;
    request.request.ValueMatch = ValueMatch;
    request.request.MatchMask = MatchMask;
    request.request.TimeStamp = TimeStamp;
    request.data = data;
    return request;
}
