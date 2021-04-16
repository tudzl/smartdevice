#ifndef _UDPRECIEVER_H_
#define _UDPRECIEVER_H_

#include <vector>
#include <M5Stack.h>
#include <M5TreeView.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <rom/tjpgd.h>
#include <driver/spi_master.h>
#include <esp_heap_alloc_caps.h>
#include "DMADrawer.h"

#define jpgColor(r, g, b) \
    (uint16_t)(((r) & 0xF8) | ((b) & 0xF8) << 5 | ((g) & 0xE0) >> 5 | ((g) & 0x1C) << 11)

typedef struct {
  uint16_t x;
  uint16_t y;
  uint16_t maxWidth;
  uint16_t maxHeight;
  uint16_t offX;
  uint16_t offY;
  jpeg_div_t scale;
  const void *src;
  size_t len;
  size_t index;
  M5Display *tft;
  uint16_t outWidth;
  uint16_t outHeight;
} jpg_file_decoder_t;


class UDPReceiver
{
public:
  UDPReceiver() {}

  M5TreeView::eCmd cmd;
  virtual void operator()(MenuItem* mi) {
    M5TreeView* treeView = ((M5TreeView*)(mi->topItem()));
    _softap = mi->tag != 0;
    M5.Lcd.fillScreen(0);
    M5.Lcd.setTextColor(0xFFFF, 0);
    M5.Lcd.drawString("WiFiwaiting", 0, 0);

    if (setup()) {
      do {
        cmd = treeView->checkInput();
      } while (cmd != M5TreeView::eCmd::BACK && loop());
      close();
    }
    M5.Lcd.fillScreen(MenuItem::backgroundColor);
  }
  bool setup()
  {
    for (int i = 0; i < 2; ++i) 
      _pixBuf[i] = (uint16_t*)pvPortMallocCaps(DMA_BUF_LEN * sizeof(uint16_t), MALLOC_CAP_DMA);
    udpStart();
    if (!_softap) {
      M5.Lcd.drawString(WiFi.localIP().toString(), 0, 0);
    } else {
      M5.Lcd.drawString(WiFi.softAPIP().toString(), 0, 0);
    }

    _isRunning = true;

    for (int i = 0; i < QUEUE_COUNT; ++i) {
      _flgQueue[i] = false;
    }

    Dma.setup();

    //disableCore0WDT();
    //disableCore1WDT();
    xTaskCreatePinnedToCore(taskReceive, "taskReceive", 4096, this, 1, NULL, 0);
//*/
    return true;
  }
  void close()
  {
    _udp.stop();
    _isRunning = false;
    Dma.close();
    delay(10);
  }

  uint8_t drawQueue = 0;
  bool loop()
  {
    if (cmd == M5TreeView::eCmd::ENTER || _wifi_stage == 2) {
      _udp.stop();
      udpStart();
    }

    while (_flgQueue[drawQueue]) {
      udpJpg(_udpQueue[drawQueue]);
      _flgQueue[drawQueue] = false;
      drawQueue = (1 + drawQueue) % QUEUE_COUNT;
    }
/*
    while (!_udpQueue.empty()) {
      if (M5.BtnC.isReleased()) udpJpg(_udpQueue.front());
      _udpQueue.pop();
    }
//
    if (_udp.parsePacket()) {
      _udpbuf.resize(UDP_BUF_LEN);
      _udpbuf.resize(_udp.read(&_udpbuf[0], UDP_BUF_LEN));
      if (!_udpbuf.empty()) {
        ++_count;
        udpJpg(_udpbuf);
      }
      if (_sec != millis() / 1000) {
        //Serial.printf("%d packet/s : %d drop/s\r\n", count, drop);
        Serial.printf("%d packet/s\r\n", _count);
        _sec = millis() / 1000;
        _count = 0;
      }
    }
//*/
    return true;
  }
private:
  enum
  { UDP_BUF_LEN = 1460
  , UDP_PORT = 63333
  , DMA_BUF_LEN = 10240
  , QUEUE_COUNT = 4
  };
  WiFiUDP _udp;
  uint16_t _count;
  uint32_t _sec;
  uint8_t _wifi_stage;
//*
  volatile bool _isRunning;
  volatile bool _flgQueue[QUEUE_COUNT];
  std::vector<uint8_t> _udpQueue[QUEUE_COUNT];
  uint8_t _idxQueue = 0;
  bool _softap = false;

  static uint16_t* _pixBuf[2];
  static DMADrawer Dma;

  static void taskReceive(void* arg) {
    UDPReceiver* me = (UDPReceiver*)arg;
    uint16_t count = 0;
    uint16_t drop = 0;
    uint32_t sec = 0;
    uint8_t idx = 0;

    while (me->_isRunning) {
      if (me->_udp.parsePacket()) {
        if (me->_flgQueue[idx]) {
          ++drop;
          me->_udp.flush();
        } else {
          ++count;
          me->_udpQueue[idx].resize(UDP_BUF_LEN);
          me->_udpQueue[idx].resize(me->_udp.read(&me->_udpQueue[idx][0], UDP_BUF_LEN));
          if (!me->_udpQueue[idx].empty()) {
            me->_flgQueue[idx] = true;
            idx = (1 + idx) % QUEUE_COUNT;
          }
        }
      }
      if (sec != millis() / 1000) {
        Serial.printf("draw %d packet/s : drop %d packet/s\r\n", count, drop);
        sec = millis() / 1000;
        if (count == 0 && drop == 0) {
          if (me->_wifi_stage == 1) me->_wifi_stage = 2;
        } else {
          me->_wifi_stage = 1;
        }
        count = 0;
        drop = 0;
      }
      delay(1);
    }
    vTaskDelete(NULL);
  } 

  void udpStart(void) {
    _wifi_stage = 0;
    Serial.println("wifi init");
    if (!_softap) {
      WiFi.mode(WIFI_MODE_STA);
      for (int i = 0; i < 10; ++i) {
        WiFi.disconnect(true);
        delay(100);
        WiFi.begin();
        for (int j = 0; j < 100; ++j) {
          if (WiFi.status() != WL_CONNECTED) {
            delay(50);
          }
        }
        if (WiFi.status() == WL_CONNECTED) break;
      }
      _udp.begin(WiFi.localIP(), UDP_PORT);
      Serial.println(WiFi.localIP());
    } else {
      WiFi.disconnect(true);
      WiFi.mode(WIFI_MODE_AP);
      WiFi.begin();
      _udp.begin(WiFi.softAPIP(), UDP_PORT);
      Serial.println(WiFi.softAPIP());
    }
  }

  static uint32_t jpgRead(JDEC *decoder, uint8_t *buf, uint32_t len) {
    jpg_file_decoder_t *jpeg = (jpg_file_decoder_t *)decoder->device;
    if (buf) {
      memcpy(buf, (const uint8_t *)jpeg->src + jpeg->index, len);
    }
    jpeg->index += len;
    return len;
  }

  static uint32_t jpgWrite(JDEC *decoder, void *bitmap, JRECT *rect) {
    jpg_file_decoder_t *jpeg = (jpg_file_decoder_t *)decoder->device;
    if ((rect->right <  jpeg->offX)
     || (rect->left >= (jpeg->offX + jpeg->outWidth))
     || (rect->bottom < jpeg->offY)
     || (rect->top >=  (jpeg->offY + jpeg->outHeight))) {
      return 1;
    }

    uint16_t x = rect->left;
    uint16_t y = rect->top;
    uint16_t w = rect->right + 1 - x;
    uint16_t h = rect->bottom + 1 - y;
    uint8_t *data = (uint8_t *)bitmap;

    if (rect->top < jpeg->offY) {
      uint16_t linesToSkip = jpeg->offY - rect->top;
      data += linesToSkip * w * 3;
      h -= linesToSkip;
      y += linesToSkip;
    }
    if (rect->bottom >= (jpeg->offY + jpeg->outHeight)) {
      uint16_t linesToSkip = (rect->bottom + 1) - (jpeg->offY + jpeg->outHeight);
      h -= linesToSkip;
    }
    uint16_t oL = (rect->left < jpeg->offX) ? jpeg->offX - rect->left : 0;
    uint16_t oR = (rect->right >= (jpeg->offX + jpeg->outWidth)) ? (rect->right + 1) - (jpeg->offX + jpeg->outWidth) : 0;

    static uint8_t pixFlip = 0;
    uint16_t pixIndex;
    uint16_t line;
    uint16_t hh = h;
    uint16_t ww = w - (oL + oR);
    uint8_t r, g, b;
    uint16_t* p = _pixBuf[pixFlip];
    uint16_t yy = 0;
    while (h--) {
      data += 3 * oL;
      line = ww;
      pixIndex = x * 2 + (++yy * 2 - 1) * jpeg->outWidth * 2;
      //pixIndex += jpeg->outWidth * 2;
      while (line--) {
        r = data[0];
        g = data[1];
        b = data[2];
        p[pixIndex- jpeg->outWidth * 2] = jpgColor( r                     ,  g                     ,  b                     );
        p[pixIndex++]                   = jpgColor((r < 251) ? r + 4 : 255, (g < 253) ? g + 2 : 255, (b < 251) ? b + 4 : 255);
        p[pixIndex- jpeg->outWidth * 2] = jpgColor((r < 249) ? r + 6 : 255, (g < 252) ? g + 3 : 255, (b < 249) ? b + 6 : 255);
        p[pixIndex++]                   = jpgColor((r < 253) ? r + 2 : 255, (g < 254) ? g + 1 : 255, (b < 253) ? b + 2 : 255);
        data += 3;
      }
      data += 3 * oR;
    }

    if (x + ww >= jpeg->outWidth) {
      Dma.draw((- jpeg->offX + jpeg->x + oL * 2)
              , (y * 2 - jpeg->offY + jpeg->y)
              , jpeg->outWidth * 2  // ww * 2
              , hh * 2
              , p);
      pixFlip = 1 - pixFlip;
    }
    return 1;
  }

  static bool jpgDecode(jpg_file_decoder_t *jpeg, uint32_t (*reader)(JDEC *, uint8_t *, uint32_t)) {
    static uint8_t work[3100];
    JDEC decoder;

    JRESULT jres = jd_prepare(&decoder, reader, work, 3100, jpeg);
    if (jres != JDR_OK) {
      log_e("jd_prepare failed! %s", jd_errors[jres]);
      return false;
    }

    uint16_t jpgWidth = decoder.width / (1 << (uint8_t)(jpeg->scale));
    uint16_t jpgHeight = decoder.height / (1 << (uint8_t)(jpeg->scale));

    if (jpeg->offX >= jpgWidth || jpeg->offY >= jpgHeight) {
      log_e("Offset Outside of JPEG size");
      return false;
    }

    size_t jpgMaxWidth = jpgWidth - jpeg->offX;
    size_t jpgMaxHeight = jpgHeight - jpeg->offY;

    jpeg->outWidth = (jpgMaxWidth > jpeg->maxWidth) ? jpeg->maxWidth : jpgMaxWidth;
    jpeg->outHeight = (jpgMaxHeight > jpeg->maxHeight) ? jpeg->maxHeight : jpgMaxHeight;

    jres = jd_decomp(&decoder, jpgWrite, (uint8_t)jpeg->scale);
    if (jres != JDR_OK) {
      log_e("jd_decomp failed! %s", jd_errors[jres]);
      return false;
    }

    return true;
  }

  void udpJpg(const std::vector<uint8_t> &udpbuf) {
    int w = (uint16_t)udpbuf[2];// + ((uint16_t)udpbuf[3] << 8);
    int y = udpbuf[0];
    int h = udpbuf[1];

    jpg_file_decoder_t jpeg;

    jpeg.src = &udpbuf[4];
    jpeg.len = udpbuf.size() - 4;
    jpeg.index = 0;
    jpeg.x = 160 - w;
    jpeg.y = 120 - h + y * 2;
    jpeg.maxWidth = w;
    jpeg.maxHeight = 120;
    jpeg.offX = 0;
    jpeg.offY = 0;
    jpeg.scale = JPEG_DIV_NONE;
    jpeg.tft = &M5.Lcd;

    jpgDecode(&jpeg, jpgRead);
  }
};
DMADrawer UDPReceiver::Dma;
uint16_t* UDPReceiver::_pixBuf[2];

#endif
