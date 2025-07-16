#pragma once

#define MADT_APIC_CPU 0x00
#define MADT_APIC_IO 0x01
#define MADT_APIC_INT 0x02
#define MADT_APIC_NMI 0x03

#define LAPIC_REG_ID 0x20
#define LAPIC_REG_TIMER_CURCNT 0x390
#define LAPIC_REG_TIMER_INITCNT 0x380
#define LAPIC_REG_TIMER 0x320
#define LAPIC_REG_TIMER 0x320
#define LAPIC_REG_SPURIOUS 0xf0
#define LAPIC_REG_TIMER_DIV 0x3e0

#define APIC_ICR_LOW 0x300
#define APIC_ICR_HIGH 0x310

#include <libs/klibc.hpp>

namespace acpi
{

    typedef struct
    {
        std::uint8_t addressid;
        std::uint8_t register_bitwidth;
        std::uint8_t register_bitoffset;
        std::uint8_t access_size;
        std::uint64_t address;
    } acpi_address_t;

    struct ACPISDTheader
    {
        char signature[4];
        std::uint32_t length;
        std::uint8_t revision;
        std::uint8_t checksum;
        char OEMID[6];
        char OEMTableID[8];
        std::uint32_t OEMRevision;
        std::uint32_t CreatorID;
        std::uint32_t CreatorRevision;
    };

    typedef struct
    {
        char signature[8];              // 签名
        std::uint8_t checksum;          // 校验和
        char oem_id[6];                 // OEM ID
        std::uint8_t revision;          // 版本
        std::uint32_t rsdt_address;     // V1: RSDT 地址 (32-bit)
        std::uint32_t length;           // 结构体长度
        std::uint64_t xsdt_address;     // V2: XSDT 地址 (64-bit)
        std::uint8_t extended_checksum; // 扩展校验和
        std::uint8_t reserved[3];       // 保留字段
    } __attribute__((packed)) rsdp;

    typedef struct
    {
        struct ACPISDTheader h;
        std::uint64_t pointer_to_other_sdt;
    } __attribute__((packed)) xsdt;

    typedef struct
    {
        struct ACPISDTheader h;
        std::uint32_t local_apic_address;
        std::uint32_t flags;
        void *entries;
    } __attribute__((packed)) madt;

    struct madt_header
    {
        std::uint8_t entry_type;
        std::uint8_t length;
    } __attribute__((packed));

    struct madt_io_apic
    {
        struct madt_header h;
        std::uint8_t apic_id;
        std::uint8_t reserved;
        std::uint32_t address;
        std::uint32_t gsib;
    } __attribute__((packed));

    struct madt_local_apic
    {
        struct madt_header h;
        std::uint8_t ACPI_Processor_UID;
        std::uint8_t local_apic_id;
        std::uint32_t flags;
    };

    struct generic_address
    {
        std::uint8_t address_space;
        std::uint8_t bit_width;
        std::uint8_t bit_offset;
        std::uint8_t access_size;
        std::uint64_t address;
    } __attribute__((packed));

    struct hpet
    {
        struct ACPISDTheader h;
        std::uint32_t event_block_id;
        struct generic_address base_address;
        std::uint16_t clock_tick_unit;
        std::uint8_t page_oem_flags;
    } __attribute__((packed));

    typedef struct
    {
        std::uint64_t configurationAndCapability;
        std::uint64_t comparatorValue;
        std::uint64_t fsbInterruptRoute;
        std::uint64_t unused;
    } __attribute__((packed)) hpet_timer;

    typedef struct
    {
        std::uint64_t generalCapabilities;
        std::uint64_t reserved0;
        std::uint64_t generalConfiguration;
        std::uint64_t reserved1;
        std::uint64_t generalIntrruptStatus;
        std::uint8_t reserved3[0xc8];
        std::uint64_t mainCounterValue;
        std::uint64_t reserved4;
        hpet_timer timers[];
    } __attribute__((packed)) hpet_info;

    typedef struct dsdt_table
    {
        std::uint8_t signature[4];
        std::uint32_t length;
        std::uint8_t revision;
        std::uint8_t checksum;
        std::uint8_t oem_id[6];
        std::uint8_t oem_tableid[8];
        std::uint32_t oem_revision;
        std::uint32_t creator_id;
        std::uint8_t definition_block;
    } __attribute__((packed)) dsdt;

    typedef struct facp_table
    {
        struct ACPISDTheader h;
        std::uint32_t firmware_ctrl;
        std::uint32_t dsdt;
        std::uint8_t reserved;
        std::uint8_t preferred_pm_profile;
        std::uint16_t sci_int;
        std::uint32_t smi_cmd;
        std::uint8_t acpi_enable;
        std::uint8_t acpi_disable;
        std::uint8_t s4bios_req;
        std::uint8_t pstate_cnt;
        std::uint32_t pm1a_evt_blk;
        std::uint32_t pm1b_evt_blk;
        std::uint32_t pm1a_cnt_blk;
        std::uint32_t pm1b_cnt_blk;
        std::uint32_t pm2_cnt_blk;
        std::uint32_t pm_tmr_blk;
        std::uint32_t gpe0_blk;
        std::uint32_t gpe1_blk;
        std::uint8_t pm1_evt_len;
        std::uint8_t pm1_cnt_len;
        std::uint8_t pm2_cnt_len;
        std::uint8_t pm_tmr_len;
        std::uint8_t gpe0_blk_len;
        std::uint8_t gpe1_blk_len;
        std::uint8_t gpe1_base;
        std::uint8_t cst_cnt;
        std::uint16_t p_lvl2_lat;
        std::uint16_t p_lvl3_lat;
        std::uint16_t flush_size;
        std::uint16_t flush_stride;
        std::uint8_t duty_offset;
        std::uint8_t duty_width;
        std::uint8_t day_alrm;
        std::uint8_t mon_alrm;
        std::uint8_t century;
        std::uint16_t iapc_boot_arch;
        std::uint8_t reserved2;
        std::uint32_t flags;
        struct generic_address reset_reg;
        std::uint8_t reset_value;
        std::uint8_t reserved3[3];
        std::uint64_t x_firmware_ctrl;
        std::uint64_t x_dsdt;
        struct generic_address x_pm1a_evt_blk;
        struct generic_address x_pm1b_evt_blk;
        struct generic_address x_pm1a_cnt_blk;
        struct generic_address x_pm1b_cnt_blk;
        struct generic_address x_pm2_cnt_blk;
        struct generic_address x_pm_tmr_blk;
        struct generic_address x_gpe0_blk;
        struct generic_address x_gpe1_blk;
    } __attribute__((packed)) facp;

    typedef struct
    {
        struct ACPISDTheader Header;
        std::uint64_t Reserved;
    } __attribute__((packed)) mcfg;

    typedef struct
    {
        std::uint64_t base_address;
        std::uint16_t pci_segment_group;
        std::uint8_t start_bus;
        std::uint8_t end_bus;
        std::uint32_t reserved;
    } __attribute__((packed)) mcfg_entry;

    typedef struct generic_address generic_address;
    typedef struct hpet hpet;
    typedef struct madt_header madt_header;
    typedef struct madt_io_apic madt_io_apic;
    typedef struct madt_local_apic madt_local_apic;
    typedef struct facp_table acpi_facp_t;

    void init();

}
