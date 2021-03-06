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

#include <QLibrary>
#include <QList>
#include "ggraph.h"

// ----------------------------------------------------------------------------
// GPluginFactory
// ----------------------------------------------------------------------------
struct GPluginFactory : GGraph::Factory {
  GPluginFactory(QObject* parent = nullptr);
  ~GPluginFactory() override;

  void load(QString folder);

protected:
  void loadFile(GGraph::Factory::ItemCategory* category, QString fileName);
  void loadFolder(GGraph::Factory::ItemCategory* category, QString folder);

protected:
  QList<QLibrary*> libraries_;
};
