#include <QDebug>
#include <QProcess>
#include <QTextStream>
#include "grtm.h"

// ----------------------------------------------------------------------------
// GRtm
// ----------------------------------------------------------------------------
GRtm::GRtm() {
  loadFromSystem();
}

GRtm::~GRtm() {
  clear();
}

bool GRtm::loadFromSystem() {
  clear();

  QString command = "cat /proc/net/route";
  QProcess p;
  p.start(command);
  if (!p.waitForFinished()) {
    qWarning() << QString("waitForFinished(%1) return false").arg(command);
    return false;
  }

  QList<QByteArray> baList = p.readAll().split('\n');
  bool firstLine = true;
  QList<QString> fields;

  foreach (QByteArray ba, baList) {
    QTextStream ts(ba);
    if (firstLine) {
      firstLine = false;
      while (true) {
        QString field;
        ts >> field;
        if (field == "") break;
        fields.append(field);
      }
    } else {
      GRtmEntry entry;
      for (int i = 0; i < fields.count(); i++) {
        QString field = fields.at(i);
        QString value;
        ts >> value;
        if (value == "") break;
        if (field == "Iface")
          entry.intf_ = value;
        else if (field == "Destination")
          entry.dst_ = ntohl(value.toInt(nullptr, 16));
        else if (field == "Gateway")
          entry.gateway_ = ntohl(value.toInt(nullptr, 16));
        else if (field == "Mask")
          entry.mask_ = ntohl(value.toInt(nullptr, 16));
        else if (field == "Metric")
          entry.metric_ = value.toInt(nullptr, 16);
      }
      if (entry.intf_ != "") // if not empty
        append(entry);
    }
  }

  return true;
}

GRtmEntry* GRtm::getBestEntry(GIp ip) {
  GRtmEntry* res = nullptr;

  int _count = count();
  for (int i = 0; i < _count; i++) {
    GRtmEntry& entry = (GRtmEntry&)at(i);

    if ((entry.dst_ & entry.mask_) != (ip & entry.mask_)) continue;
    if (res == nullptr) {
      res = &entry;
      continue;
    }
    if (entry.mask_ > res->mask_) {
      res = &entry;
      continue;
    } else
    if (entry.mask_ == res->mask_) {
      if (entry.metric_ < res->metric_) {
        res = &entry;
        continue;
      }
    }
  }

  return res;
}

GIp GRtm::getGateway(QString intf) {
  for (GRtm::iterator it = begin(); it != end(); it++) {
    GRtmEntry& entry = *it;
    if (entry.intf_ == intf && entry.gateway_ != 0)
      return entry.gateway_;
  }
  return GIp(uint32_t(0));
}

GRtm& GRtm::instance() {
  static GRtm rtm;
  return rtm;
}

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

TEST(GRtm, loadTest) {
  GRtm& rtm = GRtm::instance();
  qDebug() << "Routing Table Manager : count =" << rtm.count();
  for (GRtm::iterator it = rtm.begin(); it != rtm.end(); it++) {
    GRtmEntry& entry = *it;
    qDebug() << QString("dst=%1 mask=%2 gateway=%3 intf=%4 metric=%5").arg(
      QString(entry.dst_),
      QString(entry.mask_),
      QString(entry.gateway_),
      QString(entry.intf_),
      QString(entry.metric_)
    );
  }
}

TEST(GRtm, bestTest) {
  GRtm& rtm = GRtm::instance();
  GRtmEntry* entry = rtm.getBestEntry("8.8.8.8");
  EXPECT_NE(entry, nullptr);
  qDebug() << "best entry for 8.8.8.8 is" << entry->intf_;
}

#endif // GTEST
