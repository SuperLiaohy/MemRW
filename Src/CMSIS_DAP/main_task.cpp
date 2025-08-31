
#include "DAPReader.h"
#include <chrono>
#include <iostream>
#include <bits/this_thread_sleep.h>

int read_data_task() {
    try {
        DAPReader daplink;
        daplink.attach_to_target();
        daplink.auto_configure_ap();
        uint32_t target_addr = 0x20003b90;
        while (true) {
            auto start_time = std::chrono::high_resolution_clock::now();
            uint32_t data = daplink.read_mem(target_addr);
            auto end_time = std::chrono::high_resolution_clock::now();
            auto time_spent = std::chrono::duration_cast<std::chrono::microseconds>((end_time - start_time));
            std::cout << "\n--- Read successful! ---" << std::endl;
            std::cout << "microseconds: " << time_spent << std::endl;
            std::cout << "word: " << data << std::endl;
        }
    }  catch (const std::runtime_error&e) {
        std::cout << "catch error: " << e.what() << std::endl;
    }

    USBBulk::exit();
    return 0;
}
