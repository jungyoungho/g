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


#include <QDialog>
#include <QListWidget>
#include <QModelIndex>
#include <QPushButton>

// ----------------------------------------------------------------------------
// SignalSlotForm
// ----------------------------------------------------------------------------
class SignalSlotForm : public QDialog
{
	Q_OBJECT
	
public:
  explicit SignalSlotForm(QWidget *parent = nullptr);
  ~SignalSlotForm() override;

public:
  QListWidget* lwSignalList_;
  QListWidget* lwSlotList_;
  QPushButton* btnOK_;
  QPushButton* btnCancel_;

public:
	void setControl();
	
private slots:
  void on_btnOK_clicked(bool);
  void on_btnCancel_clicked(bool);
  void on_lwSignalList_currentRowChanged(int);
  void on_lwSlotList_currentRowChanged(int);
  void on_lwSignalList_clicked(const QModelIndex&);
  void on_lwSlotList_clicked(const QModelIndex&);
};
