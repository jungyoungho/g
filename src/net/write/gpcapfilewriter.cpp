#include "gpcapfilewriter.h"
#include <QDateTime>
#include <QDir>
#include <QFileInfo>

// ----------------------------------------------------------------------------
// GPcapFileWriter
// ----------------------------------------------------------------------------
GPcapFileWriter::GPcapFileWriter(QObject* parent) : GStateObj(parent) {
}

GPcapFileWriter::~GPcapFileWriter() {
  close();
}

bool GPcapFileWriter::doOpen() {
  if (fileName_ == "") {
    SET_ERR(GErr::FILE_NAME_NOT_SPECIFIED, "file name is not specified");
    return false;
  }

  int dataLink = GPacket::dataLinkTypeToInt(dataLinkType_);
  pcap_ = pcap_open_dead(dataLink, snapLen_);
  if (pcap_ == nullptr) {
    SET_ERR(GErr::RETURN_NULL, QString("pcap_open_dead(%1, %2)) return null").arg(dataLink, snapLen_));
    return false;
  }

  QString path = QFileInfo(fileName_).path();
  QString fileName = QFileInfo(fileName_).fileName();
  QDateTime now = QDateTime::currentDateTime();
  QString newFileName = now.toString(fileName);
  if (path != "") {
    QDir dir;
    dir.mkpath(path);
    newFileName = path + QDir::separator() + newFileName;
  }

  pcap_dumper_ = pcap_dump_open(pcap_, qPrintable(newFileName));
  if (pcap_dumper_ == nullptr) {
    SET_ERR(GErr::RETURN_NULL, QString("pcap_dump_open(, %1)) return null").arg(newFileName));
    pcap_close(pcap_);
    pcap_ = nullptr;
    return false;
  }
  return true;
}

bool GPcapFileWriter::doClose() {
  if (pcap_dumper_ != nullptr) {
    pcap_dump_close(pcap_dumper_);
    pcap_dumper_ = nullptr;
  }
  if (pcap_ != nullptr) {
    pcap_close(pcap_);
    pcap_ = nullptr;
  }
  return true;
}

void GPcapFileWriter::write(GPacket* packet) {
  struct pcap_pkthdr pkthdr;
  pkthdr.ts = packet->ts_;
  pkthdr.caplen = pkthdr.len = (bpf_u_int32)packet->buf_.size_;
  pcap_dump((u_char*)pcap_dumper_, &pkthdr, packet->buf_.data_);
  emit written(packet);
}
