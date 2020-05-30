﻿/*
 * Copyright (C) 2018-2020 wuuhii. All rights reserved.
 *
 * The file is encoding with utf-8 (with BOM). It is a part of QtSwissArmyKnife
 * project. The project is a open source project, you can get the source from:
 *     https://github.com/qsak/QtSwissArmyKnife
 *     https://gitee.com/qsak/QtSwissArmyKnife
 *
 * For more information about the project, please join our QQ group(952218522).
 * In addition, the email address of the project author is wuuhii@outlook.com.
 */
#include <QDebug>
#include <QWidget>
#include <QHBoxLayout>

#include "SAKGlobal.hh"
#include "SAKDataStruct.hh"
#include "SAKTcpServerDevice.hh"
#include "SAKTcpServerDebugPage.hh"
#include "SAKTcpServerDeviceController.hh"

SAKTcpServerDebugPage::SAKTcpServerDebugPage(QWidget *parent)
    :SAKDebugPage (SAKDataStruct::DebugPageTypeTCPServer, parent)
    ,tcpServerDevice (Q_NULLPTR)
    ,tcpServerDeviceController (new SAKTcpServerDeviceController)
{
    initPage();
    setWindowTitle(SAKGlobal::getNameOfDebugPage(SAKDataStruct::DebugPageTypeTCPServer));
}

SAKTcpServerDebugPage::~SAKTcpServerDebugPage()
{
    tcpServerDeviceController->deleteLater();

    if (tcpServerDevice){
        tcpServerDevice->terminate();
        delete tcpServerDevice;
    }
}

void SAKTcpServerDebugPage::setUiEnable(bool enable)
{
    tcpServerDeviceController->setUiEnable(enable);
    refreshPushButton->setEnabled(enable);
}

void SAKTcpServerDebugPage::changeDeviceStatus(bool opened)
{
    /*
     * 设备打开失败，使能ui, 打开成功，禁止ui
     */
    setUiEnable(!opened);
    switchPushButton->setText(opened ? tr("关闭") : tr("打开"));
    if (!opened){
        if (tcpServerDevice){
            tcpServerDevice->terminate();
            delete tcpServerDevice;
            tcpServerDevice = Q_NULLPTR;
        }
    }
//    emit deviceStateChanged(opened);
}

void SAKTcpServerDebugPage::tryWrite(QByteArray data)
{
    emit writeBytesRequest(data, tcpServerDeviceController->currentClientHost(), tcpServerDeviceController->currentClientPort());
}

void SAKTcpServerDebugPage::afterBytesRead(QByteArray data, QString host, quint16 port)
{
    if (    (tcpServerDeviceController->currentClientHost().compare(host) == 0)
         && (tcpServerDeviceController->currentClientPort() == port)){
        emit bytesRead(data);
    }    
}

void SAKTcpServerDebugPage::afterBytesWritten(QByteArray data, QString host, quint16 port)
{
    if ((tcpServerDeviceController->currentClientHost().compare(host) == 0) && (tcpServerDeviceController->currentClientPort() == port)){
        emit bytesWritten(data);
    }
}

void SAKTcpServerDebugPage::refreshDevice()
{
    tcpServerDeviceController->refresh();
}

QWidget *SAKTcpServerDebugPage::controllerWidget()
{
    return tcpServerDeviceController;
}
