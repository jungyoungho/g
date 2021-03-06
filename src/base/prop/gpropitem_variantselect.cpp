#include "gpropitem_variantselect.h"

#ifdef QT_GUI_LIB

// ----------------------------------------------------------------------------
// GPropItemVariantSelect
// ----------------------------------------------------------------------------
GPropItemVariantSelect::GPropItemVariantSelect(GPropItemParam param) : GPropItemComboBox(param) {
  // comboBox_->addItems(items); // gilgil temp 2015.12.04
  QObject::connect(comboBox_, SIGNAL(currentIndexChanged(QString)), this, SLOT(myCurrentIndexChanged(QString)));
}

void GPropItemVariantSelect::update() {
  QVariant value = object_->property(mpro_.name());
  comboBox_->setCurrentText(value.toString());
  int i = comboBox_->findText(value.toString());
  if (i != -1)
    comboBox_->setCurrentIndex(i);
}

void GPropItemVariantSelect::myCurrentIndexChanged(const QString &arg1) {
  bool res = object_->setProperty(mpro_.name(), QVariant::fromValue<QString>(arg1));
  if (!res) {
    qWarning() << QString("object->setProperty(%1, %2) return false").arg(mpro_.name(), arg1);
  }
  update();
}

#endif // QT_GUI_LIB
