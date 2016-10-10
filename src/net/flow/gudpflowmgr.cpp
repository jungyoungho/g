#include "gudpflowmgr.h"
#include "net/pdu/giphdr.h"
#include "net/pdu/gudphdr.h"

// ----------------------------------------------------------------------------
// GUdpFlowMgr
// ----------------------------------------------------------------------------
void GUdpFlowMgr::deleteOldFlowMaps(struct timeval ts) {
  FlowMap::iterator it = flowMap_.begin();
  while (it != flowMap_.end()) {
    GFlow::Value* value = it.value();
    long elapsed = ts.tv_sec - value->ts_.tv_sec;
    long timeout;
    switch (value->state_) {
      case GFlow::Value::Half: timeout = halfTimeout_; break;
      case GFlow::Value::Full: timeout = fullTimeout_; break;
      case GFlow::Value::Rst: qCritical() << "unrecheable Rst"; timeout = 0; break;
      case GFlow::Value::Fin: qCritical() << "unrecheable Fin"; timeout = 0; break;
    }
    if (elapsed >= timeout) {
      emit _flowDeleted(&it.key(), value);
      it = flowMap_.erase(it);
      continue;
    }
    it++;
  }
}

void GUdpFlowMgr::process(GPacket* packet) {
  long now = packet->ts_.tv_sec;
  if (checkInterval_ != 0 && now - lastCheckTick_ >= checkInterval_)
  {
    deleteOldFlowMaps(packet->ts_);
    lastCheckTick_ = now;
  }

  GIpHdr* ipHdr = packet->pdus_.findFirst<GIpHdr>();
  if (ipHdr == nullptr) return;

  GUdpHdr* udpHdr = packet->pdus_.findFirst<GUdpHdr>();
  if (udpHdr == nullptr) return;

  GFlow::UdpFlowKey key;
  key.sip = ipHdr->sip();
  key.dip = ipHdr->dip();
  key.sport = udpHdr->sport();
  key.dport = udpHdr->dport();

  FlowMap::iterator it = flowMap_.find(key);
  if (it == flowMap_.end()) {
    GFlow::Value* value = GFlow::Value::allocate(packet->ts_, GFlow::Value::Half, requestItems_.totalMemSize_);
    it = flowMap_.insert(key, value);
    emit _flowCreated(&it.key(), value);

    GFlow::UdpFlowKey reverseKey = GFlow::UdpFlowKey(it.key()).reverse();
    FlowMap::iterator reverseIt = flowMap_.find(reverseKey);
    if (reverseIt != flowMap_.end()) {
      it.value()->state_ = GFlow::Value::Full;
      reverseIt.value()->state_ = GFlow::Value::Full;
    }
  } else {
    GFlow::Value* value = it.value();
    value->ts_ = packet->ts_;
  }

  this->key_ = (GFlow::UdpFlowKey*)&it.key();
  this->value_ = it.value();
  emit processed(packet);
}