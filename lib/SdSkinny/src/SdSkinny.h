// SdSkinny.h
//
// *** WARNING *** 
// This slimmed-down version of SdFat makes the following assumptions to reduce code size and 
// RAM usage. Do not use if any of these conditions are false for your application:
// 1. Using a SDHC card

#ifndef SDSKINNY_H
#define SDSKINNY_H

#include <SPI.h>

/** status for card in the ready state */
const uint8_t R1_READY_STATE = 0X00;
/** status for card in the idle state */
const uint8_t R1_IDLE_STATE = 0X01;
/** status bit for illegal command */
const uint8_t R1_ILLEGAL_COMMAND = 0X04;
/** start data token for read or write single block*/
const uint8_t DATA_START_BLOCK = 0XFE;
/** stop token for write multiple blocks*/
const uint8_t STOP_TRAN_TOKEN = 0XFD;
/** start data token for write multiple blocks*/
const uint8_t WRITE_MULTIPLE_TOKEN = 0XFC;
/** mask for data response tokens after a write block operation */
const uint8_t DATA_RES_MASK = 0X1F;
/** write data accepted token */
const uint8_t DATA_RES_ACCEPTED = 0X05;

typedef enum {
  SD_CARD_ERROR_NONE = 0,

  // Basic commands and switch command.
  SD_CARD_ERROR_CMD0 = 0X20,
  SD_CARD_ERROR_CMD2,
  SD_CARD_ERROR_CMD3,
  SD_CARD_ERROR_CMD6,
  SD_CARD_ERROR_CMD7,
  SD_CARD_ERROR_CMD8,
  SD_CARD_ERROR_CMD9,
  SD_CARD_ERROR_CMD10,
  SD_CARD_ERROR_CMD12,
  SD_CARD_ERROR_CMD13,

  // Read, write, erase, and extension commands.
  SD_CARD_ERROR_CMD17 = 0X30,
  SD_CARD_ERROR_CMD18,
  SD_CARD_ERROR_CMD24,
  SD_CARD_ERROR_CMD25,
  SD_CARD_ERROR_CMD32,
  SD_CARD_ERROR_CMD33,
  SD_CARD_ERROR_CMD38,
  SD_CARD_ERROR_CMD58,
  SD_CARD_ERROR_CMD59,

  // Application specific commands.
  SD_CARD_ERROR_ACMD6 = 0X40,
  SD_CARD_ERROR_ACMD13,
  SD_CARD_ERROR_ACMD23,
  SD_CARD_ERROR_ACMD41,

  // Read/write errors
  SD_CARD_ERROR_READ = 0X50,
  SD_CARD_ERROR_READ_CRC,
  SD_CARD_ERROR_READ_FIFO,
  SD_CARD_ERROR_READ_REG,
  SD_CARD_ERROR_READ_START,
  SD_CARD_ERROR_READ_TIMEOUT,
  SD_CARD_ERROR_STOP_TRAN,
  SD_CARD_ERROR_WRITE,
  SD_CARD_ERROR_WRITE_FIFO,
  SD_CARD_ERROR_WRITE_START,
  SD_CARD_ERROR_FLASH_PROGRAMMING,
  SD_CARD_ERROR_WRITE_TIMEOUT,

    // Misc errors.
  SD_CARD_ERROR_DMA = 0X60,
  SD_CARD_ERROR_ERASE,
  SD_CARD_ERROR_ERASE_SINGLE_BLOCK,
  SD_CARD_ERROR_ERASE_TIMEOUT,
  SD_CARD_ERROR_INIT_NOT_CALLED,
  SD_CARD_ERROR_FUNCTION_NOT_SUPPORTED
} sd_error_code_t;

// SD card commands
/** GO_IDLE_STATE - init card in spi mode if CS low */
const uint8_t CMD0 = 0X00;
/** ALL_SEND_CID - Asks any card to send the CID. */
const uint8_t CMD2 = 0X02;
/** SEND_RELATIVE_ADDR - Ask the card to publish a new RCA. */
const uint8_t CMD3 = 0X03;
/** SWITCH_FUNC - Switch Function Command */
const uint8_t CMD6 = 0X06;
/** SELECT/DESELECT_CARD - toggles between the stand-by and transfer states. */
const uint8_t CMD7 = 0X07;
/** SEND_IF_COND - verify SD Memory Card interface operating condition.*/
const uint8_t CMD8 = 0X08;
/** SEND_CSD - read the Card Specific Data (CSD register) */
const uint8_t CMD9 = 0X09;
/** SEND_CID - read the card identification information (CID register) */
const uint8_t CMD10 = 0X0A;
/** STOP_TRANSMISSION - end multiple block read sequence */
const uint8_t CMD12 = 0X0C;
/** SEND_STATUS - read the card status register */
const uint8_t CMD13 = 0X0D;
/** READ_SINGLE_BLOCK - read a single data block from the card */
const uint8_t CMD17 = 0X11;
/** READ_MULTIPLE_BLOCK - read a multiple data blocks from the card */
const uint8_t CMD18 = 0X12;
/** WRITE_BLOCK - write a single data block to the card */
const uint8_t CMD24 = 0X18;
/** WRITE_MULTIPLE_BLOCK - write blocks of data until a STOP_TRANSMISSION */
const uint8_t CMD25 = 0X19;
/** ERASE_WR_BLK_START - sets the address of the first block to be erased */
const uint8_t CMD32 = 0X20;
/** ERASE_WR_BLK_END - sets the address of the last block of the continuous
    range to be erased*/
const uint8_t CMD33 = 0X21;
/** ERASE - erase all previously selected blocks */
const uint8_t CMD38 = 0X26;
/** APP_CMD - escape for application specific command */
const uint8_t CMD55 = 0X37;
/** READ_OCR - read the OCR register of a card */
const uint8_t CMD58 = 0X3A;
/** CRC_ON_OFF - enable or disable CRC checking */
const uint8_t CMD59 = 0X3B;
/** SET_BUS_WIDTH - Defines the data bus width for data transfer. */
const uint8_t ACMD6 = 0X06;
/** SD_STATUS - Send the SD Status. */
const uint8_t ACMD13 = 0X0D;
/** SET_WR_BLK_ERASE_COUNT - Set the number of write blocks to be
     pre-erased before writing */
const uint8_t ACMD23 = 0X17;
/** SD_SEND_OP_COMD - Sends host capacity support information and
    activates the card's initialization process */
const uint8_t ACMD41 = 0X29;

#define SD_SCK_HZ(maxSpeed) SPISettings(maxSpeed, MSBFIRST, SPI_MODE0)
#define SD_SCK_MHZ(maxMhz) SPISettings(1000000UL*maxMhz, MSBFIRST, SPI_MODE0)
// SPI divisor constants
/** Set SCK to max rate of F_CPU/2. */
#define SPI_FULL_SPEED SD_SCK_MHZ(50)

class SdSkinny {
    private:
        uint8_t m_csPin;
        void spiSend(uint8_t data);
        uint8_t spiReceive();
        void spiStop();
        uint8_t cardCommand(uint8_t cmd, uint32_t arg);
        uint8_t cardAcmd(uint8_t cmd, uint32_t arg);
        void error(sd_error_code_t);
    public:
        SdSkinny() {};
        bool begin(uint8_t csPin = SS, SPISettings spiSettings = SPI_FULL_SPEED);
        sd_error_code_t errorCode;
};

#endif