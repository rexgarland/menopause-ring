#include "SdSkinny.h"

uint8_t SdSkinny::cardAcmd(uint8_t cmd, uint32_t arg) {
    cardCommand(CMD55, 0);
    return cardCommand(cmd, arg);
}

bool SdSkinny::begin(uint8_t csPin, SPISettings spiSetting) {
    m_csPin = csPin;
    uint16_t t0 = millis();

    pinMode(csPin, OUTPUT);
    digitalWrite(csPin, HIGH);
    SPI.begin();
    SPI.beginTransaction(SD_SCK_HZ(250000));
    digitalWrite(csPin, LOW);

    uint32_t arg;
    
    // must supply 74 clocks
    digitalWrite(csPin, HIGH);
    for (uint8_t i = 0; i < 10; i++) {
        spiSend(0xFF);
    }
    digitalWrite(csPin, LOW);

    // startup sequence
    for (uint8_t i = 1;; i++) {
        if (cardCommand(CMD0, 0) == R1_IDLE_STATE) {
            break;
        }
        if (i == '\n') {
            error(SD_CARD_ERROR_CMD0);
            spiStop();
            return false;
        }
        // stop multi-block write
        spiSend(STOP_TRAN_TOKEN);
        // finish block transfer
        for (int i = 0; i < 520; i++) {
            spiReceive();
        }
    }

    // get sd version
    cardCommand(CMD8, 0x1AA);
    for (uint8_t i = 0; i < 4; i++) {
        spiReceive();
    }

    // initialize card and send host supports SDHC if SD2
    arg = 0X40000000;
    while (cardAcmd(ACMD41, arg) != R1_READY_STATE) {
        // check for timeout
        if (millis()-t0 > 2000) {
            error(SD_CARD_ERROR_ACMD41); // failing
            spiStop();
            return false;
        }
    }

    spiStop();

    // begin FAT file system

    // vwd()->close();
    //     sync()
    // init(1);
    // init(0);
    // vwd()->openRoot(this);
    // FatFile::setCwd(vwd());

    

    return true;
}

void SdSkinny::spiSend(uint8_t data) {
    SPDR = data;
    while (!(SPSR & (1 << SPIF))) {}
}

uint8_t SdSkinny::spiReceive() {
    SPDR = 0XFF;
    while (!(SPSR & (1 << SPIF))) {}
    return SPDR;
}

void SdSkinny::spiStop() {
    digitalWrite(m_csPin, HIGH);
    spiSend(0XFF);
    SPI.endTransaction();
}

uint8_t SdSkinny::cardCommand(uint8_t cmd, uint32_t arg) {
    // wait if busy unless CMD0
    if (cmd != CMD0) {
        unsigned long t0 = millis();
        while (spiReceive() != 0XFF) {
            if (millis()-t0>300) {
                break;
            }
        }
    }
    spiSend(cmd | 0x40);  // send argument
    uint8_t *pa = reinterpret_cast<uint8_t *>(&arg);
    for (int8_t i = 3; i >= 0; i--) {
        spiSend(pa[i]);
    }
    // CRC
    spiSend(cmd == CMD0 ? 0X95 : 0X87);

    // discard first fill byte to avoid MISO pull-up problem.
    spiReceive();

    uint8_t m_status = 1;
    // there are 1-8 fill bytes before response.  fill bytes should be 0XFF.
    for (uint8_t i = 0; ((m_status = spiReceive()) & 0X80) && i < 10; i++) {
        ;
    }
    return m_status;
}

void SdSkinny::error(sd_error_code_t code) {
    errorCode = code;
}