//
// Created by liaohy on 8/15/25.
//

#pragma once
#include <cstdint>
#include <bitset>

namespace SW {
    namespace DP {
        constexpr uint8_t IDCODE = 0x00; // read
        constexpr uint8_t ABORT = 0x00; // write
        constexpr uint8_t CTRL_STAT = 0x04; //sel == 0
        constexpr uint8_t WCR = 0x04; // sel == 1
        constexpr uint8_t RESEND = 0x08; // read
        constexpr uint8_t SELECT = 0x08; // write
        constexpr uint8_t RDBUFF = 0x0C; // read
        constexpr uint8_t ROUTESEL = 0x0C; // write optional

#pragma pack(push, 1)
        struct IDCODEReg {
            uint32_t RAO: 1;
            uint32_t DESIGNER: 11;
            uint32_t VERSION: 4;
            uint32_t MIN: 1;
            uint32_t RES0: 3;
            uint32_t PARTNO: 8;
            uint32_t REVISION: 4;
        };
#pragma pack(pop)
#pragma pack(push, 1)
        struct ABORTReg {
            uint32_t DAPABORT: 1;
            uint32_t STKCMPCLR: 1;
            uint32_t STKERRCLR: 1;
            uint32_t WDERRCLR: 1;
            uint32_t ORUNERRCLR: 1;
            uint32_t RESERVED: 27;
        };
#pragma pack(pop)
#pragma pack(push, 1)
        struct CTRL_STATReg {
            uint32_t ORUNDETECT: 1;
            uint32_t STICKYORUN: 1;
            uint32_t TRNMODE: 2;
            uint32_t STICKYCMP: 1;
            uint32_t STICKYERR: 1;
            uint32_t READOK: 1;
            uint32_t WDATAERR: 1;
            uint32_t MASKLANE: 4;
            uint32_t TRNCNT: 12;
            uint32_t RAZ_SBZP: 2;
            uint32_t CDBGRSTREQ: 1;
            uint32_t CDBGRSTACK: 1;
            uint32_t CDBGPWRUPREQ: 1;
            uint32_t CDBGPWRUPACK: 1;
            uint32_t CSYSPWRUPREQ: 1;
            uint32_t CSYSPWRUPACK: 1;
        };
#pragma pack(pop)
#pragma pack(push, 1)
        struct WCRReg {
            uint32_t PRESCALER: 3;
            uint32_t SBZ: 3;
            uint32_t WIREMODE: 2;
            uint32_t TURNROUND: 2;
            uint32_t RESERVED: 22;
        };
#pragma pack(pop)
        struct RESENDReg {
            uint32_t data;
        };
#pragma pack(push, 1)
        struct SELECTReg {
            uint32_t CTRLSEL: 1;
            uint32_t SBZ: 3;
            uint32_t APBANKSEL: 4;
            uint32_t RESERVED: 16;
            uint32_t APSEL: 8;
        };
#pragma pack(pop)
        struct RDBUFFReg {
            uint32_t data;
        };
    }

    namespace MEM_AP {
        constexpr uint8_t CSW = 0x00; // read
        constexpr uint8_t TAR = 0x04; // write
        constexpr uint8_t DRW = 0x0C; //sel == 0
        constexpr uint8_t BD0 = 0x10; // sel == 0
        constexpr uint8_t BD1 = 0x14; // sel == 0
        constexpr uint8_t BD2 = 0x18; // sel == 0
        constexpr uint8_t BD3 = 0x1C; // sel == 0
        constexpr uint8_t MBT = 0x20; // read
        constexpr uint8_t T0TR = 0x30; // write
        constexpr uint8_t CFG1 = 0xE0; // read
        constexpr uint8_t BASE_LARGE = 0xF0; // write optional
        constexpr uint8_t CFG = 0xF4; // write optional
        constexpr uint8_t BASE = 0xF8; // write optional
        constexpr uint8_t IDR = 0xFC; // write optional

#pragma pack(push, 1)
        struct CSWReg {
            uint32_t Size: 3;
            uint32_t RES0: 1;
            uint32_t AddrInc: 2;
            uint32_t DeviceEn: 1;
            uint32_t TrInProg: 1;
            uint32_t Mode: 4;
            uint32_t Type: 3;
            uint32_t MTE: 1;
            uint32_t RES1: 7;
            uint32_t SPIDEN: 1;
            uint32_t Port: 7;
            uint32_t DbgSwEnable: 1;
        };
#pragma pack(pop)
        struct TARReg {
            uint32_t data;
        };

        struct DRWReg {
            uint32_t data;
        };

        struct BDReg {
            uint32_t data;
        };

        struct MBTReg {
            uint32_t res;
        };
#pragma pack(push, 1)
        struct T0TRReg {
            uint32_t T0: 4;
            uint32_t T1: 4;
            uint32_t T2: 4;
            uint32_t T3: 4;
            uint32_t T4: 4;
            uint32_t T5: 4;
            uint32_t T6: 4;
            uint32_t T7: 4;
        };
#pragma pack(pop)
#pragma once
#pragma pack(push, 1)
        struct CFG1Reg {
            uint32_t TAG0SIZE: 4;
            uint32_t TAG0GRAN: 5;
            uint32_t RES: 23;
        };
#pragma pack(pop)
        struct BASE_LARGEReg {
            uint32_t data;
        };
#pragma pack(push, 1)
        struct CFGReg {
            uint32_t BE: 1;
            uint32_t LA: 1;
            uint32_t LD: 1;
            uint32_t RES: 29;
        };
#pragma pack(pop)
#pragma pack(push, 1)
        struct BASEReg {
            uint32_t P: 1;
            uint32_t Format: 1;
            uint32_t RES: 10;
            uint32_t BASEADDR: 20;
        };
#pragma pack(pop)
#pragma pack(push, 1)
        struct IDRReg {
            uint32_t TYPE: 4;
            uint32_t VARIANT: 4;
            uint32_t RES: 5;
            uint32_t CLASS: 4;
            uint32_t DESIGNER: 11;
            uint32_t REVISION: 4;
        };
#pragma pack(pop)
    }
}

template<typename T>
T &operator<<(T &ostream, const SW::DP::IDCODEReg &reg) {
    ostream << "IDCODE.RAO: " << std::bitset<1>(reg.RAO) << "\n";
    ostream << "IDCODE.DESIGNER: " << std::bitset<1>(reg.DESIGNER) << "\n";
    ostream << "IDCODE.VERSION: " << std::bitset<4>(reg.VERSION) << "\n";
    ostream << "IDCODE.MIN: " << std::bitset<1>(reg.MIN) << "\n";
    ostream << "IDCODE.RES0: " << std::bitset<3>(reg.RES0) << "\n";
    ostream << "IDCODE.PARTNO: " << std::bitset<8>(reg.PARTNO) << "\n";
    ostream << "IDCODE.REVISION: " << std::bitset<4>(reg.REVISION) << "\n";
    return ostream;
}
template<typename T>
T &operator<<(T &ostream, const SW::DP::ABORTReg &reg) {
    ostream << "ABORT.DAPABORT: " << std::bitset<1>(reg.DAPABORT) << "\n";
    ostream << "ABORT.STKCMPCLR: " << std::bitset<1>(reg.STKCMPCLR) << "\n";
    ostream << "ABORT.STKERRCLR: " << std::bitset<1>(reg.STKERRCLR) << "\n";
    ostream << "ABORT.WDERRCLR: " << std::bitset<1>(reg.WDERRCLR) << "\n";
    ostream << "ABORT.ORUNERRCLR: " << std::bitset<1>(reg.ORUNERRCLR) << "\n";
    ostream << "ABORT.RESERVED: " << std::bitset<27>(reg.RESERVED) << "\n";
    return ostream;
}
template<typename T>
T &operator<<(T &ostream, const SW::DP::CTRL_STATReg &reg) {
    ostream << "CTRL_STAT.ORUNDETECT: " << std::bitset<1>(reg.ORUNDETECT) << "\n";
    ostream << "CTRL_STAT.STICKYORUN: " << std::bitset<1>(reg.STICKYORUN) << "\n";
    ostream << "CTRL_STAT.TRNMODE: " << std::bitset<2>(reg.TRNMODE) << "\n";
    ostream << "CTRL_STAT.STICKYCMP: " << std::bitset<1>(reg.STICKYCMP) << "\n";
    ostream << "CTRL_STAT.STICKYERR: " << std::bitset<1>(reg.STICKYERR) << "\n";
    ostream << "CTRL_STAT.READOK: " << std::bitset<1>(reg.READOK) << "\n";
    ostream << "CTRL_STAT.WDATAERR: " << std::bitset<1>(reg.WDATAERR) << "\n";
    ostream << "CTRL_STAT.MASKLANE: " << std::bitset<4>(reg.MASKLANE) << "\n";
    ostream << "CTRL_STAT.TRNCNT: " << std::bitset<12>(reg.TRNCNT) << "\n";
    ostream << "CTRL_STAT.RAZ_SBZP: " << std::bitset<2>(reg.RAZ_SBZP) << "\n";
    ostream << "CTRL_STAT.CDBGRSTREQ: " << std::bitset<1>(reg.CDBGRSTREQ) << "\n";
    ostream << "CTRL_STAT.CDBGRSTACK: " << std::bitset<1>(reg.CDBGRSTACK) << "\n";
    ostream << "CTRL_STAT.CDBGPWRUPREQ: " << std::bitset<1>(reg.CDBGPWRUPREQ) << "\n";
    ostream << "CTRL_STAT.CDBGPWRUPACK: " << std::bitset<1>(reg.CDBGPWRUPACK) << "\n";
    ostream << "CTRL_STAT.CSYSPWRUPREQ: " << std::bitset<1>(reg.CSYSPWRUPREQ) << "\n";
    ostream << "CTRL_STAT.CSYSPWRUPACK: " << std::bitset<1>(reg.CSYSPWRUPACK) << "\n";
    return ostream;
}
template<typename T>
T &operator<<(T &ostream, const SW::DP::WCRReg &reg) {
    ostream << "WCR.PRESCALER: " << std::bitset<3>(reg.PRESCALER) << "\n";
    ostream << "WCR.SBZ: " << std::bitset<3>(reg.SBZ) << "\n";
    ostream << "WCR.WIREMODE: " << std::bitset<2>(reg.WIREMODE) << "\n";
    ostream << "WCR.TURNROUND: " << std::bitset<2>(reg.TURNROUND) << "\n";
    ostream << "WCR.RESERVED: " << std::bitset<22>(reg.RESERVED) << "\n";
    return ostream;
}
template<typename T>
T &operator<<(T &ostream, const SW::DP::RESENDReg &reg) {
    ostream << "RESEND.data: " << reg.data << "\n";
    return ostream;
}
template<typename T>
T &operator<<(T &ostream, const SW::DP::SELECTReg &reg) {
    ostream << "SELECT.CTRLSEL: " << std::bitset<1>(reg.CTRLSEL) << "\n";
    ostream << "SELECT.SBZ: " << std::bitset<3>(reg.SBZ) << "\n";
    ostream << "SELECT.APBANKSEL: " << std::bitset<4>(reg.APBANKSEL) << "\n";
    ostream << "SELECT.RESERVED: " << std::bitset<16>(reg.RESERVED) << "\n";
    ostream << "SELECT.APSEL: " << std::bitset<8>(reg.APSEL) << "\n";
    return ostream;
}
template<typename T>
T &operator<<(T &ostream, const SW::DP::RDBUFFReg &reg) {
    ostream << "RDBUFF.data: " << reg.data << "\n";
    return ostream;
}
template<typename T>
T &operator<<(T &ostream, const SW::MEM_AP::CSWReg &reg) {
    ostream << "CSW.Size: " << std::bitset<3>(reg.Size) << "\n";
    ostream << "CSW.RES0: " << std::bitset<1>(reg.RES0) << "\n";
    ostream << "CSW.AddrInc: " << std::bitset<2>(reg.AddrInc) << "\n";
    ostream << "CSW.DeviceEn: " << std::bitset<1>(reg.DeviceEn) << "\n";
    ostream << "CSW.TrInProg: " << std::bitset<1>(reg.TrInProg) << "\n";
    ostream << "CSW.Mode: " << std::bitset<4>(reg.Mode) << "\n";
    ostream << "CSW.Type: " << std::bitset<3>(reg.Type) << "\n";
    ostream << "CSW.MTE: " << std::bitset<1>(reg.MTE) << "\n";
    ostream << "CSW.RES1: " << std::bitset<7>(reg.RES1) << "\n";
    ostream << "CSW.SPIDEN: " << std::bitset<1>(reg.SPIDEN) << "\n";
    ostream << "CSW.Port: " << std::bitset<7>(reg.Port) << "\n";
    ostream << "CSW.DbgSwEnable: " << std::bitset<1>(reg.DbgSwEnable) << "\n";
    return ostream;
}
template<typename T>
T &operator<<(T &ostream, const SW::MEM_AP::TARReg &reg) {
    ostream << "TAR.data: " << reg.data << "\n";
    return ostream;
}
template<typename T>
T &operator<<(T &ostream, const SW::MEM_AP::DRWReg &reg) {
    ostream << "DRW.data: " << reg.data << "\n";
    return ostream;
}
template<typename T>
T &operator<<(T &ostream, const SW::MEM_AP::BDReg &reg) {
    ostream << "BD.data: " << reg.data << "\n";
    return ostream;
}
template<typename T>
T &operator<<(T &ostream, const SW::MEM_AP::MBTReg &reg) {
    ostream << "MBT.res: " << reg.res << "\n";
    return ostream;
}
template<typename T>
T &operator<<(T &ostream, const SW::MEM_AP::T0TRReg &reg) {
    ostream << "T0TR.T0: " << std::bitset<4>(reg.T0) << "\n";
    ostream << "T0TR.T1: " << std::bitset<4>(reg.T1) << "\n";
    ostream << "T0TR.T2: " << std::bitset<4>(reg.T2) << "\n";
    ostream << "T0TR.T3: " << std::bitset<4>(reg.T3) << "\n";
    ostream << "T0TR.T4: " << std::bitset<4>(reg.T4) << "\n";
    ostream << "T0TR.T5: " << std::bitset<4>(reg.T5) << "\n";
    ostream << "T0TR.T6: " << std::bitset<4>(reg.T6) << "\n";
    ostream << "T0TR.T7: " << std::bitset<4>(reg.T7) << "\n";
    return ostream;
}
template<typename T>
T &operator<<(T &ostream, const SW::MEM_AP::CFG1Reg &reg) {
    ostream << "CFG1.TAG0SIZE: " << std::bitset<4>(reg.TAG0SIZE) << "\n";
    ostream << "CFG1.TAG0GRAN: " << std::bitset<5>(reg.TAG0GRAN) << "\n";
    ostream << "CFG1.RES: " << std::bitset<23>(reg.RES) << "\n";
    return ostream;
}
template<typename T>
T &operator<<(T &ostream, const SW::MEM_AP::BASE_LARGEReg &reg) {
    ostream << "BASE_LARGE.data: " << reg.data << "\n";
    return ostream;
}
template<typename T>
T &operator<<(T &ostream, const SW::MEM_AP::CFGReg &reg) {
    ostream << "CFG.BE: " << std::bitset<1>(reg.BE) << "\n";
    ostream << "CFG.LA: " << std::bitset<1>(reg.LA) << "\n";
    ostream << "CFG.LD: " << std::bitset<1>(reg.LD) << "\n";
    ostream << "CFG.RES: " << std::bitset<29>(reg.RES) << "\n";
    return ostream;
}
template<typename T>
T &operator<<(T &ostream, const SW::MEM_AP::BASEReg &reg) {
    ostream << "BASE.P: " << std::bitset<1>(reg.P) << "\n";
    ostream << "BASE.Format: " << std::bitset<1>(reg.Format) << "\n";
    ostream << "BASE.RES: " << std::bitset<10>(reg.RES) << "\n";
    ostream << "BASE.BASEADDR: " << std::bitset<20>(reg.BASEADDR) << "\n";
    return ostream;
}
template<typename T>
T &operator<<(T &ostream, const SW::MEM_AP::IDRReg &reg) {
    ostream << "IDR.TYPE: " << std::bitset<4>(reg.TYPE) << "\n";
    ostream << "IDR.VARIANT: " << std::bitset<4>(reg.VARIANT) << "\n";
    ostream << "IDR.RES: " << std::bitset<5>(reg.RES) << "\n";
    ostream << "IDR.CLASS: " << std::bitset<4>(reg.CLASS) << "\n";
    ostream << "IDR.DESIGNER: " << std::bitset<11>(reg.DESIGNER) << "\n";
    ostream << "IDR.REVISION: " << std::bitset<4>(reg.REVISION) << "\n";
    return ostream;
}
