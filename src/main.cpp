
// Add this in your sketch or change the setting in AudioConfig.h
#define USE_A2DP
#define USE_HELIX

#include <SPIFFS.h>
#include "AudioTools.h"
#include "AudioCodecs/CodecMP3Helix.h"

using namespace audio_tools;

constexpr char SPEAKER_NAME[] = "Soundcore 3";
constexpr char FILE_01[]      = "/non_BGM.mp3";

A2DPStream _source = A2DPStream::instance();
EncodedAudioStream _decoder(&_source, new MP3DecoderHelix());  // Decoding stream
StreamCopy _copy;
File _audioFile;

// Arduino Setup
void setup(void) {
  //Serial.begin(115200);
  //AudioLogger::instance().begin(Serial, AudioLogger::Info);

  _source.setVolume(1.0);
  _source.begin(TX_MODE, SPEAKER_NAME);
  log_i("A2DP is connected now...");

  if (!SPIFFS.begin()) {
    log_e("SPIFFS Mount error.");
    return;
  } else {
    log_i("SPIFFS Mounted...");
  }

  _audioFile = SPIFFS.open(FILE_01);
  log_i("Open Audio File...");

  _decoder.setNotifyAudioChange(_source);
  _decoder.begin();
  log_i("Begin decoder...");

  _copy.begin(_decoder, _audioFile);
  log_i("Begin copy _audioFile to decoder...");
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
