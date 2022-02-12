
#define USE_A2DP
#define USE_HELIX

#include <Arduino.h>
#include <M5Atom.h>
#include <SPI.h>
#include "FS.h"
#include "SD.h"
#include "AudioTools.h"
#include "AudioCodecs/CodecMP3Helix.h"

using namespace audio_tools;

constexpr char SPEAKER_NAME[] = "Soundcore 3";
constexpr char FILE_01[]      = "/non.mp3";

A2DPStream         _source = A2DPStream::instance();
MP3DecoderHelix    _mp3;
EncodedAudioStream _decoder(&_source, &_mp3);  // Decoding stream
StreamCopy         _copy;
File               _audioFile;

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  log_i("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    log_e("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    log_e("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      log_i("  DIR : %s", file.name());

      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      log_i("  FILE: %s SIZE: %d", file.name(), file.size());
    }
    file = root.openNextFile();
  }

  file.close();
}

void setup(void) {
  // Serial.begin(115200);
  // AudioLogger::instance().begin(Serial, AudioLogger::Info);

  M5.begin(true, false, false);
  SPI.begin(23, 33, 19, -1);
  SD.end();
  delay(1000);
  if (!SD.begin(-1, SPI, 10000000)) {
    log_e("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  log_i("SD Card Size: %lluMB\n", cardSize);

  listDir(SD, "/", 0);
  _audioFile = SD.open(FILE_01);
  log_i("Open Audio File...");
  if (_audioFile.size() > 0) {
    log_i("File SIZE %d", _audioFile.size());

    _source.setVolume(1.0);
    _source.begin(TX_MODE, SPEAKER_NAME);
    log_i("A2DP is connected now...");

    // delay(500);

    _decoder.setNotifyAudioChange(_source);
    _decoder.begin();
    log_i("Begin decoder...");

    _copy.begin(_decoder, _audioFile);
    log_i("Begin copy _audioFile to decoder...");
  }
}

void loop() {
  if (!_copy.copy()) {
    auto info = _decoder.decoder().audioInfo();
    log_i("The audio rate     from the mp3 file is %d", info.sample_rate);
    log_i("The channels       from the mp3 file is %d", info.channels);
    log_i("The bit per sample from the mp3 file is %d", info.bits_per_sample);
    stop();
  }
}
