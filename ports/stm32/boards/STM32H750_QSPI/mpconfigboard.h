// This board is confirmed to operate using stlink and openocd.
// REPL is on UART(1) and is available through the stlink USB-UART device.
// To use openocd run "OPENOCD_CONFIG=boards/openocd_stm32f7.cfg" in
// the make command.
#define MICROPY_HW_BOARD_NAME       "H750QSPI"
#define MICROPY_HW_MCU_NAME         "STM32H750"

#define MICROPY_HW_HAS_SWITCH       (1)
#define MICROPY_HW_HAS_FLASH        (1)
#define MICROPY_HW_ENABLE_RNG       (1)
#define MICROPY_HW_ENABLE_RTC       (1)
#define MICROPY_HW_ENABLE_USB       (1)
#define MICROPY_HW_ENABLE_SERVO     (1)
#define MICROPY_HW_ENABLE_SDCARD    (1)

#define MICROPY_BOARD_EARLY_INIT    board_early_init
void board_early_init(void);

// HSE is 25MHz
// VCOClock = HSE * PLLN / PLLM = 25 MHz * 432 / 25 = 432 MHz
// SYSCLK = VCOClock / PLLP = 432 MHz / 2 = 216 MHz
// USB/SDMMC/RNG Clock = VCOClock / PLLQ = 432 MHz / 9 = 48 MHz
#define MICROPY_HW_CLK_PLLM (25)
#define MICROPY_HW_CLK_PLLN (432)
#define MICROPY_HW_CLK_PLLP (RCC_PLLP_DIV2)
#define MICROPY_HW_CLK_PLLQ (9)

#define MICROPY_HW_FLASH_LATENCY    FLASH_LATENCY_7 // 210-216 MHz needs 7 wait states

// 512MBit external QSPI flash, used for either the filesystem or XIP memory mapped
#define MICROPY_HW_QSPIFLASH_SIZE_BITS_LOG2 (26)
#define MICROPY_HW_QSPIFLASH_CS     (pin_B6)
#define MICROPY_HW_QSPIFLASH_SCK    (pin_B2)
#define MICROPY_HW_QSPIFLASH_IO0    (pin_D11)
#define MICROPY_HW_QSPIFLASH_IO1    (pin_D12)
#define MICROPY_HW_QSPIFLASH_IO2    (pin_E2)
#define MICROPY_HW_QSPIFLASH_IO3    (pin_D13)


// SPI flash, block device config (when used as the filesystem)
extern const struct _mp_spiflash_config_t spiflash_config;
extern struct _spi_bdev_t spi_bdev;
#if !USE_QSPI_XIP
#define MICROPY_HW_ENABLE_INTERNAL_FLASH_STORAGE (0)
#define MICROPY_HW_SPIFLASH_ENABLE_CACHE (1)
#define MICROPY_HW_BDEV_SPIFLASH    (&spi_bdev)
#define MICROPY_HW_BDEV_SPIFLASH_CONFIG (&spiflash_config)
#define MICROPY_HW_BDEV_SPIFLASH_SIZE_BYTES ((1 << MICROPY_HW_QSPIFLASH_SIZE_BITS_LOG2) / 8)
#define MICROPY_HW_BDEV_SPIFLASH_EXTENDED (&spi_bdev) // for extended block protocol
#endif

// UART config
#define MICROPY_HW_UART1_TX         (pin_A9)
#define MICROPY_HW_UART1_RX         (pin_A10)
#define MICROPY_HW_UART2_TX (pin_A2)
#define MICROPY_HW_UART2_RX (pin_A3)
#define MICROPY_HW_UART_REPL        PYB_UART_1
#define MICROPY_HW_UART_REPL_BAUD   115200

// I2C buses
#define MICROPY_HW_I2C1_SCL         (pin_B8)
#define MICROPY_HW_I2C1_SDA         (pin_B9)
#define MICROPY_HW_I2C2_SCL (pin_B10)
#define MICROPY_HW_I2C2_SDA (pin_B11)

// SPI buses
#define MICROPY_HW_SPI1_NSS (pin_A4)
#define MICROPY_HW_SPI1_SCK (pin_A5)
#define MICROPY_HW_SPI1_MISO (pin_A6)
#define MICROPY_HW_SPI1_MOSI (pin_A7)

#define MICROPY_HW_SPI2_NSS (pin_B12)
#define MICROPY_HW_SPI2_SCK (pin_B13)
#define MICROPY_HW_SPI2_MISO (pin_B14)
#define MICROPY_HW_SPI2_MOSI (pin_B15)

// CAN buses
#define MICROPY_HW_CAN1_TX (pin_B9)
#define MICROPY_HW_CAN1_RX (pin_B8)

// USRSW
#define MICROPY_HW_USRSW_PIN (pin_C13) // K1 on the board.
#define MICROPY_HW_USRSW_PULL (GPIO_NOPULL)
#define MICROPY_HW_USRSW_EXTI_MODE (GPIO_MODE_IT_FALLING)
#define MICROPY_HW_USRSW_PRESSED (0)

// LEDs
#define MICROPY_HW_LED1 (pin_E3) // BLUE_LED, the only controlable LED on the board.
#define MICROPY_HW_LED_ON(pin) (mp_hal_pin_low(pin))
#define MICROPY_HW_LED_OFF(pin) (mp_hal_pin_high(pin))

// SD card
#define MICROPY_HW_SDCARD_SDMMC (1)
#define MICROPY_HW_SDCARD_CK (pin_C12)
#define MICROPY_HW_SDCARD_CMD (pin_D2)
#define MICROPY_HW_SDCARD_D0 (pin_C8)
#define MICROPY_HW_SDCARD_D1 (pin_C9)
#define MICROPY_HW_SDCARD_D2 (pin_C10)
#define MICROPY_HW_SDCARD_D3 (pin_C11)

// USB config 
#define MICROPY_HW_USB_FS (1)



/******************************************************************************/
// Bootloader configuration

// Give Mboot access to the external QSPI flash
#define MBOOT_SPIFLASH_ADDR                     (0x90000000)
#define MBOOT_SPIFLASH_BYTE_SIZE                (512 * 128 * 1024)
#define MBOOT_SPIFLASH_LAYOUT                   "/0x90000000/512*128Kg"
#define MBOOT_SPIFLASH_ERASE_BLOCKS_PER_PAGE    (128 / 4) // 128k page, 4k erase block
#define MBOOT_SPIFLASH_CONFIG                   (&spiflash_config)
#define MBOOT_SPIFLASH_SPIFLASH                 (&spi_bdev.spiflash)
