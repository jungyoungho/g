#include "gtcpflowmgr.h"
#include "net/pdu/giphdr.h"
#include "net/pdu/gtcphdr.h"

// ----------------------------------------------------------------------------
// GTcpFlowMgr
// ----------------------------------------------------------------------------
void GTcpFlowMgr::deleteOldFlowMaps(GPacket* packet /* struct timeval ts */) {
  struct timeval ts = packet->ts_;
  FlowMap::iterator it = flowMap_.begin();
  while (it != flowMap_.end()) {
    GFlow::Value* value = it.value();
    long elapsed = ts.tv_sec - value->ts_.tv_sec;
    long timeout = 0;
    switch (value->state_) {
      case GFlow::Value::Half: timeout = halfTimeout_; break;
      case GFlow::Value::Full: timeout = fullTimeout_; break;
      case GFlow::Value::Rst: timeout = rstTimeout_; break;
      case GFlow::Value::Fin: timeout = finTimeout_; break;
    }
    if (elapsed >= timeout) {
      key_ = (GFlow::TcpFlowKey*)&it.key();
      value_ = value;
      emit _flowDeleted(packet);
      it = flowMap_.erase(it);
      continue;
    }
    it++;
  }
}

void GTcpFlowMgr::process(GPacket* packet) {
  long now = packet->ts_.tv_sec;
  if (checkInterval_ != 0 && now - lastCheckTick_ >= checkInterval_)
  {
    deleteOldFlowMaps(packet);
    lastCheckTick_ = now;
  }

  GIpHdr* ipHdr = packet->pdus_.findFirst<GIpHdr>();
  if (ipHdr == nullptr) return;

  GTcpHdr* tcpHdr = packet->pdus_.findFirst<GTcpHdr>();
  if (tcpHdr == nullptr) return;

  GFlow::TcpFlowKey key{ipHdr->sip(), tcpHdr->sport(), ipHdr->dip(), tcpHdr->dport()};

  FlowMap::iterator it = flowMap_.find(key);
  if (it == flowMap_.end()) {
    GFlow::Value* value = GFlow::Value::allocate(packet->ts_, GFlow::Value::Half, requestItems_.totalMemSize_);
    it = flowMap_.insert(key, value);
    key_ = (GFlow::TcpFlowKey*)&it.key();
    value_ = value;
    emit _flowCreated(packet);

    GFlow::TcpFlowKey reverseKey = GFlow::TcpFlowKey(it.key()).reverse();
    FlowMap::iterator reverseIt = flowMap_.find(reverseKey);
    if (reverseIt != flowMap_.end()) {
      it.value()->state_ = GFlow::Value::Full;
      reverseIt.value()->state_ = GFlow::Value::Full;
    }
  } else {
    GFlow::Value* value = it.value();
    value->ts_ = packet->ts_;
  }

  if ((tcpHdr->flags() & (TH_RST | TH_FIN)) != 0) {
    GFlow::Value* value = it.value();
    GFlow::Value::State state = (tcpHdr->flags() & TH_RST) ? GFlow::Value::Rst : GFlow::Value::Fin;
    value->state_ = state;
    GFlow::TcpFlowKey reverseKey = GFlow::TcpFlowKey(it.key()).reverse();
    FlowMap::iterator reverseIt = flowMap_.find(reverseKey);
    if (reverseIt != flowMap_.end()) {
      reverseIt.value()->state_ = state;
    }
  }

  this->key_ = (GFlow::TcpFlowKey*)&it.key();
  this->value_ = it.value();
  emit processed(packet);
}
