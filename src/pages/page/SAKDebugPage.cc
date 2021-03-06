﻿/*
 * Copyright 2018-2020 Qter(qsaker@qq.com). All rights reserved.
 *
 * The file is encoded using "utf8 with bom", it is a part
 * of QtSwissArmyKnife project.
 *
 * QtSwissArmyKnife is licensed according to the terms in
 * the file LICENCE in the root of the source code directory.
 */
#include <QPixmap>
#include <QDateTime>
#include <QSettings>
#include <QKeyEvent>
#include <QMetaEnum>
#include <QScrollBar>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QTextStream>
#include <QIntValidator>
#include <QLoggingCategory>

#include "SAKGlobal.hh"
#include "SAKSettings.hh"
#include "SAKDebugPage.hh"
#include "SAKDataStruct.hh"
#include "SAKSqlDatabase.hh"
#include "SAKCRCInterface.hh"
#include "SAKDebugPageDevice.hh"
#include "SAKDebugPageController.hh"
#include "SAKOtherAnalyzerThread.hh"
#include "SAKOtherHighlighterManager.hh"
#include "SAKDebugPageOtherController.hh"
#include "SAKDebugPageInputController.hh"
#include "SAKDebugPageOutputController.hh"
#ifdef SAK_IMPORT_CHARTS_MODULE
#include "SAKDebugPageChartsController.hh"
#endif
#include "SAKOtherAnalyzerThreadManager.hh"
#include "SAKOtherAutoResponseItemManager.hh"
#include "SAKDebugPageStatisticsController.hh"

#include "ui_SAKDebugPage.h"

SAKDebugPage::SAKDebugPage(int type, QWidget *parent)
    :QWidget(parent)
    ,mDevice(Q_NULLPTR)
    ,mDeviceController(Q_NULLPTR)
    ,mIsInitializing(true)
    ,mDebugPageType(type)
    ,mUi(new Ui::SAKDebugPage)
{
    mUi->setupUi(this);
    initializingVariables();

    mOutputController = new SAKDebugPageOutputController(this, this);
    mOtherController = new SAKDebugPageOtherController(this, this);
    mStatisticsController = new SAKDebugPageStatisticsController(this, this);
    mInputController = new SAKDebugPageInputController(this, this);
#ifdef SAK_IMPORT_CHARTS_MODULE
    mChartsController= new SAKDebugPageChartsController(this);
#endif

    // Initializing the timer.
    mClearInfoTimer.setInterval(SAK_CLEAR_MESSAGE_INTERVAL);
    connect(&mClearInfoTimer, &QTimer::timeout, this, &SAKDebugPage::cleanInfo);
    mIsInitializing = false;
}

SAKDebugPage::~SAKDebugPage()
{
    delete mDevice;
#ifdef SAK_IMPORT_CHARTS_MODULE
    delete mChartsController;
#endif
    delete mUi;
}

void SAKDebugPage::write(QByteArray data)
{
    emit requestWriteData(data);
}

void SAKDebugPage::writeRawData(QString rawData, int textFormat)
{
    emit requestWriteRawData(rawData, textFormat);
}

void SAKDebugPage::outputMessage(QString msg, bool isInfo)
{
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss ");
    QString temp;
    temp.append(time);
    temp.append(msg);
    time = QString("<font color=silver>%1</font>").arg(time);

    if (isInfo){
        mInfoLabel->setStyleSheet(QString("QLabel{color:black}"));
    }else{
        mInfoLabel->setStyleSheet(QString("QLabel{color:red}"));
        QApplication::beep();
    }

    msg.prepend(time);
    mInfoLabel->setText(msg);
    mClearInfoTimer.start();
}

quint32 SAKDebugPage::pageType()
{
    return mDebugPageType;
}

QSettings *SAKDebugPage::settings()
{
    return SAKSettings::instance();
}

QSqlDatabase *SAKDebugPage::sqlDatabase()
{
    return SAKSqlDatabase::instance()->sqlDatabase();
}

QString SAKDebugPage::settingsGroup()
{
    QString group;
    int pageType = mDebugPageType;
    QMetaEnum metaEnum = QMetaEnum::fromType<SAKDataStruct::SAKEnumDebugPageType>();
    for (int i = 0; i < metaEnum.keyCount(); i++){
        if (metaEnum.value(i) == pageType){
            group = QString(metaEnum.key(i));
        }
    }

    return group;
}

SAKDebugPageOtherController *SAKDebugPage::otherController()
{
    return mOtherController;
}

SAKDebugPageInputController *SAKDebugPage::inputController()
{
    return mInputController;
}

#ifdef SAK_IMPORT_CHARTS_MODULE
SAKDebugPageChartsController *SAKDebugPage::chartsController()
{
   return mChartsController;
}
#endif

SAKDebugPageOutputController *SAKDebugPage::outputController()
{
    return mOutputController;
}

SAKDebugPageStatisticsController *SAKDebugPage::statisticsController()
{
    return mStatisticsController;
}

void SAKDebugPage::initializingPage()
{
    setupController();
    setupDevice();
}

void SAKDebugPage::changedDeviceState(bool opened)
{
    mSendPushButton->setEnabled(opened);
    mSendPresetPushButton->setEnabled(opened);
    mCycleEnableCheckBox->setEnabled(opened);
    mRefreshPushButton->setEnabled(!opened);
    mDeviceController->setUiEnable(opened);

    mSwitchPushButton->setEnabled(true);
}

void SAKDebugPage::cleanInfo()
{
    mClearInfoTimer.stop();
    mInfoLabel->clear();
}

void SAKDebugPage::openOrColoseDevice()
{
    if (!mDevice){
        setupDevice();
    }

    if (mDevice){
        if (mDevice->isRunning()){
            closeDevice();
        }else{
            openDevice();
        }
    }
}

void SAKDebugPage::openDevice()
{
    if (mDevice){
        mDevice->start();
        mSwitchPushButton->setText(tr("Close"));
    }
}

void SAKDebugPage::closeDevice()
{
    if (mDevice){
        mDevice->requestInterruption();
        mDevice->requestWakeup();
        mDevice->exit();
        mDevice->wait();
        mDevice->deleteLater();
        mDevice = Q_NULLPTR;
        mSwitchPushButton->setText(tr("Open"));
    }
}

void SAKDebugPage::setupDevice()
{
    mDevice = createDevice();
    if (mDevice){
        connect(this, &SAKDebugPage::requestWriteData, mDevice, &SAKDebugPageDevice::writeBytes);
        connect(mDevice, &SAKDebugPageDevice::bytesWritten, this, &SAKDebugPage::bytesWritten);
#if 0
        connect(device, &SAKDevice::bytesRead, this, &SAKDebugPage::bytesRead);
#else
        // The bytes read will be input to analyzer, after analyzing, the bytes will be input to debug page
        SAKOtherAnalyzerThreadManager *analyzerManager = mOtherController->analyzerThreadManager();
        connect(mDevice, &SAKDebugPageDevice::bytesRead, analyzerManager, &SAKOtherAnalyzerThreadManager::inputBytes);

        // The function may be called multiple times, so do something to ensure that the signal named bytesAnalysed
        // and the slot named bytesRead are connected once.
        connect(analyzerManager, &SAKOtherAnalyzerThreadManager::bytesAnalysed, this, &SAKDebugPage::bytesRead, static_cast<Qt::ConnectionType>(Qt::AutoConnection|Qt::UniqueConnection));
#endif
        connect(mDevice, &SAKDebugPageDevice::messageChanged, this, &SAKDebugPage::outputMessage);
        connect(mDevice, &SAKDebugPageDevice::deviceStateChanged, this, &SAKDebugPage::changedDeviceState);
        connect(mDevice, &SAKDebugPageDevice::finished, this, &SAKDebugPage::closeDevice);
    }
}

void SAKDebugPage::setupController()
{
    SAKDebugPageController *controller = deviceController();
    if (controller){
        QHBoxLayout *layout = new QHBoxLayout(mDeviceSettingFrame);
        mDeviceSettingFrame->setLayout(layout);
        layout->addWidget(controller);
        layout->setContentsMargins(0, 0, 0, 0);
        mDeviceController = controller;
    }
}

void SAKDebugPage::on_refreshPushButton_clicked()
{
    if (mDeviceController){
        mDeviceController->refreshDevice();
    }else{
        Q_ASSERT_X(false, __FUNCTION__, "A null pointer!");
    }
}

void SAKDebugPage::on_switchPushButton_clicked()
{
    mSwitchPushButton->setEnabled(false);
    openOrColoseDevice();
}

void SAKDebugPage::initializingVariables()
{
    // Devce control
    mSwitchPushButton = mUi->switchPushButton;
    mRefreshPushButton = mUi->refreshPushButton;
    mDeviceSettingFrame = mUi->deviceSettingFrame;

    // Message output
    mInfoLabel = mUi->infoLabel;

    // Input settings
    mInputModelComboBox = mUi->inputModelComboBox;
    mCycleEnableCheckBox = mUi->cycleEnableCheckBox;
    mCycleTimeLineEdit = mUi->cycleTimeLineEdit;
    mSaveInputDataPushButton = mUi->saveInputDataPushButton;
    mReadinFilePushButton = mUi->readinFilePushButton;
    mAddCRCCheckBox = mUi->addCRCCheckBox;
    mCrcSettingsPushButton = mUi->crcSettingsPushButton;
    mClearInputPushButton = mUi->clearInputPushButton;
    mSendPushButton = mUi->sendPushButton;
    mInputTextEdit = mUi->inputTextEdit;
    mCrcParameterModelsComboBox = mUi->crcParameterModelsComboBox;
    mCrcLabel = mUi->crcLabel;
    mPresetPushButton = mUi->presetPushButton;
    mSendPresetPushButton = mUi->sendPresetPushButton;

    // Output settings
    mRxLabel = mUi->rxLabel;
    mTxLabel = mUi->txLabel;
    mOutputTextFormatComboBox= mUi->outputTextFormatComboBox;
    mAutoWrapCheckBox = mUi->autoWrapCheckBox;
    mShowDateCheckBox = mUi->showDateCheckBox;
    mShowTimeCheckBox = mUi->showTimeCheckBox;
    mShowMsCheckBox = mUi->showMsCheckBox;
    mShowRxDataCheckBox = mUi->showRxDataCheckBox;
    mShowTxDataCheckBox = mUi->showTxDataCheckBox;
    mSaveOutputToFileCheckBox = mUi->saveOutputToFileCheckBox;
    mOutputFilePathPushButton = mUi->outputFilePathPushButton;
    mClearOutputPushButton = mUi->clearOutputPushButton;
    mSaveOutputPushButton = mUi->saveOutputPushButton;
    mOutputTextBroswer = mUi->outputTextBroswer;

    // Statistics
    mRxSpeedLabel = mUi->rxSpeedLabel;
    mTxSpeedLabel = mUi->txSpeedLabel;
    mRxFramesLabel = mUi->rxFramesLabel;
    mTxFramesLabel = mUi->txFramesLabel;
    mRxBytesLabel = mUi->rxBytesLabel;
    mTxBytesLabel = mUi->txBytesLabel;
    mResetTxCountPushButton = mUi->resetTxCountPushButton;
    mResetRxCountPushButton = mUi->resetRxCountPushButton;

    // Other settings
    mTransmissionSettingPushButton = mUi->transmissionSettingPushButton;
    mAnalyzerPushButton = mUi->analyzerPushButton;
    mAutoResponseSettingPushButton = mUi->autoResponseSettingPushButton;
    mTimingSendingPushButton = mUi->timingSendingPushButton;
    mHighlightSettingPushButton = mUi->highlightSettingPushButton;
    mMoreSettingsPushButton = mUi->moreSettingsPushButton;

    // Charts
    mDataVisualizationPushButton = mUi->dataVisualizationPushButton;
}

void SAKDebugPage::on_dataVisualizationPushButton_clicked()
{
#ifdef SAK_IMPORT_CHARTS_MODULE
    if (mChartsController->isHidden()){
        mChartsController->show();
    }else{
        mChartsController->activateWindow();
    }
#else
    QMessageBox::warning(this, tr("Unsupported function"), tr("The function has been disable, beause of developer's Qt version is not supported!"));
#endif
}
