/**
 * @file player-sd-a2dp.ino
 * @brief see https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-player/player-sd-a2dp/README.md
 *
 * @author Phil Schatzmann
 * @copyright GPLv3
 */

// set this in AudioConfig.h or here after installing https://github.com/pschatzmann/arduino-libhelix.git
#define USE_HELIX
#define USE_A2DP
#define SDFAT_FILE_TYPE 1

#include <AudioTools.h>
#include <AudioCodecs/CodecMP3Helix.h>
#include <SPI.h>
#include <SdFat.h>

using namespace audio_tools;

SdFat SD;
File  audioFile;

A2DPStream         _source = A2DPStream::instance();  // A2DP input - A2DPStream is a singleton!
MP3DecoderHelix    mp3decoder;
EncodedAudioStream _decoder(&_source, &mp3decoder);  // Decoding stream
StreamCopy         copier(_decoder, audioFile);

// audioFile(mp3)->mp3decoder(helix)->_source(source);

const int chipSelect = -1;

void setup() {
  // Serial.begin(115200);
  // AudioLogger::instance().begin(Serial, AudioLogger::Warning);
  SPI.begin(23, 33, 19, -1);

  // setup file
  SD.begin(SdSpiConfig(chipSelect, DEDICATED_SPI, SD_SCK_MHZ(10), &SPI));
  audioFile = SD.open("/non.mp3");

  _source.setVolume(1.0);
  _source.begin(TX_MODE, "Soundcore 3");

  _decoder.setNotifyAudioChange(_source);
  _decoder.begin();
}

void loop() {
  copier.copy();
}
