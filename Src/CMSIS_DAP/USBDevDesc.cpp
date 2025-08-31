//
// Created by liaohy on 8/12/25.
//

#include "USBDevDesc.h"
#include <algorithm>
#include <iostream>

bool USBDevDesc::is_daplink() {
    std::string trans_name = this->product_string;
    std::ranges::transform(trans_name, trans_name.begin(), tolower);
    return (trans_name.find("cmsis") != std::string::npos ||
            trans_name.find("dap") != std::string::npos ||
            trans_name.find("debug") != std::string::npos ||
            trans_name.find("link") != std::string::npos);
};


bool is_usb_bulk(const std::string &str) {
    std::string trans_name = str;
    std::ranges::transform(trans_name,trans_name.begin(), tolower);
    return (trans_name.find('2') != std::string::npos ||
            trans_name.find("bulk") != std::string::npos);
}

