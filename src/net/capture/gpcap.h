// ----------------------------------------------------------------------------
//
// G Library
//
// http://www.gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#pragma once

#include <pcap.h>
#include "gcapture.h"

// ----------------------------------------------------------------------------
// GPcap
// ----------------------------------------------------------------------------
struct GPcap : GCapture {
  Q_OBJECT
  Q_PROPERTY(QString filter MEMBER filter_)

public:
  QString filter_{""};

public:
  enum {
    ERROR_IN_PCAP_NEXT_EX = GErrCategory::PCAP,
  };

public:
  GPcap(QObject* parent = nullptr) : GCapture(parent) {}
  ~GPcap() override;

protected:
  bool doOpen() override;
  bool doClose() override;

public:
  GPacket::Result read(GPacket* packet) override;
  GPacket::Result write(GPacket* packet) override;
  GPacket::Result write(u_char* buf, size_t len) override;
  GPacket::Result relay(GPacket* packet) override;

  GPacket::DataLinkType dataLinkType() override { return dataLinkType_; }
  PathType pathType() override { return OutOfPath; }

protected:
  bool pcapProcessFilter(pcap_if_t* dev);

  pcap_t* pcap_{nullptr};
  GPacket::DataLinkType dataLinkType_{GPacket::Null};
};
