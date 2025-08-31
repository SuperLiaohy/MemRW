//
// Created by liaohy on 8/15/25.
//

#pragma once

#include "SWReg.h"
#include "USBBulk.h"


struct DPRegs {
    std::optional<SW::DP::IDCODEReg> idcode;
    std::optional<SW::DP::ABORTReg> abort;
    std::optional<SW::DP::CTRL_STATReg> cs;
    std::optional<SW::DP::WCRReg> wcr;
    std::optional<SW::DP::RESENDReg> resend;
    std::optional<SW::DP::SELECTReg> select;
    std::optional<SW::DP::RDBUFFReg> rdbuff;
};

struct APRegs {
    std::optional<SW::MEM_AP::CSWReg> csw;
    std::optional<SW::MEM_AP::TARReg> tar;
    std::optional<SW::MEM_AP::DRWReg> drw;
    std::optional<SW::MEM_AP::BDReg> bd0;
    std::optional<SW::MEM_AP::BDReg> bd1;
    std::optional<SW::MEM_AP::BDReg> bd2;
    std::optional<SW::MEM_AP::BDReg> bd3;
    std::optional<SW::MEM_AP::MBTReg> mbt;
    std::optional<SW::MEM_AP::T0TRReg> t0tr;
    std::optional<SW::MEM_AP::CFG1Reg> cfg1;
    std::optional<SW::MEM_AP::BASE_LARGEReg> base_large;
    std::optional<SW::MEM_AP::CFGReg> cfg;
    std::optional<SW::MEM_AP::BASEReg> base;
    std::optional<SW::MEM_AP::IDRReg> idr;
};

struct SerialDebugInterface {
    DPRegs dp;
    APRegs ap;
};

namespace DAP {
    enum Port {
        Default_Port = 0,
        SWD_Port = 1,
        JTAG_Port = 2,
    };
    enum ResponseStatus {
        DAP_OK = 0x00,
        DAP_ERROR = 0xFF
    };
#pragma pack(push,1)
    struct TransferRequest {
        struct {
            uint8_t APnDP : 1;
            uint8_t RnW : 1;
            uint8_t reg : 2;
            uint8_t ValueMatch : 1;
            uint8_t MatchMask : 1;
            uint8_t RES : 1;
            uint8_t TimeStamp : 1;
        } request;
        uint32_t data;
    };
#pragma pack(pop)
#pragma pack(push,1)
    struct TransferResponse {
        uint32_t TimeStamp;
        uint32_t data;
    };
#pragma pack(pop)
}


class DAPReader {
public:
    DAPReader();

    SerialDebugInterface sw;
    std::unique_ptr<USBBulk> usb;

    void attach_to_target();
    uint32_t read_mem(uint32_t addr);
    void auto_configure_ap();

    int dap_connect(uint8_t port);
    int set_swj_clock(uint32_t clock);
    int set_swj_sequence(const uint8_t* sequence, uint8_t len);
    int set_swj_sequence(const std::vector<uint8_t>& sequence) {return set_swj_sequence(sequence.data(),sequence.size());}
    int transfer(const DAP::TransferRequest& requests, DAP::TransferResponse& responses);
    int transfer(const std::vector<DAP::TransferRequest>& requests, std::vector<DAP::TransferResponse>& responses);

    static DAP::TransferRequest Request(uint8_t APnDP, uint8_t RnW, uint8_t reg, uint8_t ValueMatch, uint8_t MatchMask, uint8_t TimeStamp, uint32_t data);
    static DAP::TransferRequest APWriteRequest(uint8_t reg, uint32_t data, uint8_t TimeStamp = 0, uint8_t MatchMask = 0)
    {return Request(1,0,reg,0,MatchMask,TimeStamp,data);}
    static DAP::TransferRequest APReadRequest(uint8_t reg, uint32_t data = 0, uint8_t TimeStamp = 0, uint8_t ValueMatch = 0)
    {return Request(1,1,reg,ValueMatch,0,TimeStamp,data);}
    static DAP::TransferRequest DPWriteRequest(uint8_t reg, uint32_t data, uint8_t TimeStamp = 0, uint8_t MatchMask = 0)
    {return Request(0,0,reg,0,MatchMask,TimeStamp,data);}
    static DAP::TransferRequest DPReadRequest(uint8_t reg, uint32_t data = 0, uint8_t TimeStamp = 0, uint8_t ValueMatch = 0)
    {return Request(0,1,reg,ValueMatch,0,TimeStamp,data);}

private:
    std::vector<uint8_t> response_buffer;
};
