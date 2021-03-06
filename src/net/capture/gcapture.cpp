#include "gcapture.h"
#include "net/parser/gparserfactory.h"

// ----------------------------------------------------------------------------
// GCaptureThread
// ----------------------------------------------------------------------------
void GCaptureThread::run() {
  GCapture* capture = dynamic_cast<GCapture*>(parent());
  Q_ASSERT(capture != nullptr);
  capture->run();
}

// ----------------------------------------------------------------------------
// GCapture
// ----------------------------------------------------------------------------
GCapture::~GCapture() {
  close();
}

bool GCapture::captureThreadOpen() {
  if (autoRead_)
  {
    // ----- by gilgil 2009.08.31 -----
    //
    // There can be a case that even if thread starts,
    // state remains not not Opened(still Opening) state.
    // So set state_ into Opened before open thread.
    //
    state_ = Opened;
    thread_.start();
    // --------------------------------
  }
  return true;
}

bool GCapture::captureThreadClose() {
  if (autoRead_) {
    thread_.quit();
    thread_.wait();
  }
  return true;
}

GPacket::Result GCapture::read(GPacket* packet) {
  (void)packet;
  SET_ERR(GErr::VIRTUAL_FUNCTION_CALL, "virtual function call");
  return GPacket::Fail;
}

GPacket::Result GCapture::write(GPacket* packet) {
  (void)packet;
  SET_ERR(GErr::VIRTUAL_FUNCTION_CALL, "virtual function call");
  return GPacket::Fail;
}

GPacket::Result GCapture::write(u_char* buf, size_t len) {
  (void)buf;
  (void)len;
  SET_ERR(GErr::VIRTUAL_FUNCTION_CALL, "virtual function call");
  return GPacket::Fail;
}

GPacket::Result GCapture::relay(GPacket* packet) {
  (void)packet;
  SET_ERR(GErr::VIRTUAL_FUNCTION_CALL, "virtual function call");
  return GPacket::Fail;
}

void GCapture::run() {
  qDebug() << "beg"; // gilgil temp 2015.10.28
  GParser* parser = GParserFactory::getDefaultParser(dataLinkType());
  Q_ASSERT(parser != nullptr);

  GPacket packet;
  packet.capture_ = this;
  while (active()) {
    packet.clear();
    GPacket::Result res = read(&packet);
    if (res == GPacket::TimeOut) continue;
    if (res == GPacket::Eof || res == GPacket::Fail) break;
    if (autoParse_) parser->parse(&packet);
    emit captured(&packet);
    if (this->pathType() == InPath && !packet.control.block_) {
      res = relay(&packet);
      if (res != GPacket::Ok) {
        qWarning() << "relay return " << (int)res; // gilgil temp 2015.10.29
      }
    }
  }
  qDebug() << "end"; // gilgil temp 2015.10.28
  emit closed();
}
