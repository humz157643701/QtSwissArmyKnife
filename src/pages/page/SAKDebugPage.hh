﻿/*
 * Copyright 2018-2020 Qter(qsaker@qq.com). All rights reserved.
 *
 * The file is encoded using "utf8 with bom", it is a part
 * of QtSwissArmyKnife project.
 *
 * QtSwissArmyKnife is licensed according to the terms in
 * the file LICENCE in the root of the source code directory.
 */
#ifndef SAKDEBUGPAGE_HH
#define SAKDEBUGPAGE_HH

#include <QSize>
#include <QTimer>
#include <QLabel>
#include <QMutex>
#include <QWidget>
#include <QCheckBox>
#include <QGroupBox>
#include <QComboBox>
#include <QDateTime>
#include <QMetaEnum>
#include <QTextEdit>
#include <QSettings>
#include <QListWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QApplication>
#include <QTextBrowser>
#include <QSqlDatabase>

class SAKDataFactory;
class SAKCRCInterface;
class SAKDebugPageDevice;
class SAKDebugPageController;
class SAKOtherHighlighterManager;
class SAKDebugPageOtherController;
class SAKDebugPageInputController;
class SAKDebugPageOutputController;
#ifdef SAK_IMPORT_CHARTS_MODULE
class SAKDebugPageChartsController;
#endif
class SAKOtherTransmissionPageViewer;
class SAKDebugPageStatisticsController;

namespace Ui {
    class SAKDebugPage;
}

/// @brief Debugging page
class SAKDebugPage : public QWidget
{
    Q_OBJECT
public:
    SAKDebugPage(int type, QWidget *parent = Q_NULLPTR);
    ~SAKDebugPage();

    friend class SAKDebugPageOtherController;
    friend class SAKDebugPageInputController;
    friend class SAKDebugPageChartsController;
    friend class SAKDebugPageOutputController;
    friend class SAKDebugPageStatisticsController;

    /**
     * @brief write: Write data to device
     * @param data: Data need to be written
     */
    void write(QByteArray data);

    /**
     * @brief writeRawData: Write raw data
     * @param rawData: Input text
     * @param textFormat: input text fromat
     */
    void writeRawData(QString rawData, int textFormat);

    /**
     * @brief outputMessage: Output message to ui
     * @param msg: The message that need to be show
     * @param isInfo: true-text color is green, false-text color is red
     */
    void outputMessage(QString msg, bool isInfo = true);

    /**
     * @brief pageType: The type of debugging page.(SAKDataStruct::SAKEnumDebugPageType)
     * @return Type of debugging page
     */
    quint32 pageType();

    /**
     * @brief settingsInstance: Get the settings instance
     * @return Settings instance
     */
    QSettings *settings();

    /**
     * @brief sqlDatabase: Get the database instance
     * @return Database instance
     */
    QSqlDatabase *sqlDatabase();

    /**
     * @brief settingsGroup: Get the settings group
     * @param pageType: The type of debug page
     * @return Settings group
     */
    QString settingsGroup();

    /**
     * @brief otherController: Get SAKDebugPageOtherController instance pointer
     * @return SAKDebugPageOtherController instance pointer
     */
    SAKDebugPageOtherController *otherController();

    /**
     * @brief inputController: Get SAKDebugPageInputController instance pointer
     * @return SAKDebugPageInputController instance pointer
     */
    SAKDebugPageInputController *inputController();

#ifdef SAK_IMPORT_CHARTS_MODULE
    /**
     * @brief chartsController: Get SAKDebugPageChartsController instance pointer
     * @return SAKDebugPageChartsController instance pointer
     */
    SAKDebugPageChartsController *chartsController();
#endif

    /**
     * @brief outputController: Get SAKDebugPageOutputController instance pointer
     * @return SAKDebugPageOutputController instance pointer
     */
    SAKDebugPageOutputController *outputController();

    /**
     * @brief statisticsController: Get SAKDebugPageStatisticsController instance pointer
     * @return SAKDebugPageStatisticsController instance pointer
     */
    SAKDebugPageStatisticsController *statisticsController();
public:
    /**
     * @brief deviceController: Get the device controller instance, the controller will be destroy when the page is closed.
     * @return Device controller instance pointer
     */
    virtual SAKDebugPageController *deviceController() = 0;
private:
    /**
     * @brief createDevice: Get the device instance, the device will be destroy when it is closed.
     * @return Device instance pointer
     */
    virtual SAKDebugPageDevice* createDevice() = 0;
protected:
    /**
     * @brief initializingPage: Initializing, the function must be called in the constructor of subclass.
     */
    void initializingPage();
private:
    SAKDebugPageDevice *mDevice;
    SAKDebugPageController *mDeviceController;
    bool mIsInitializing;
    int mDebugPageType;
    QString mSettingKey;
    QTimer mClearInfoTimer;
    QMutex mReadWriteParametersMutex;

    // Debug page modules
    SAKDebugPageOtherController *mOtherController;
    SAKDebugPageInputController *mInputController;
#ifdef SAK_IMPORT_CHARTS_MODULE
    SAKDebugPageChartsController *mChartsController;
#endif
    SAKDebugPageOutputController *mOutputController;
    SAKDebugPageStatisticsController *mStatisticsController;
private:
    void cleanInfo();

    void openOrColoseDevice();
    void closeDevice();
    void openDevice();

    void setupDevice();
    void setupController();
    void changedDeviceState(bool opened);
signals:
    // Emit the read data
    void bytesRead(QByteArray data);
    // Emit the written data
    void bytesWritten(QByteArray data);
    void requestWriteData(QByteArray data);
    void requestWriteRawData(QString data, int textFormat);

    // ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
    // ui component
private:
    Ui::SAKDebugPage *mUi;
private:
    // All variable about ui will be initialize in the function
    void initializingVariables();

    /*************************************************************************/
    // Device control module
protected:
    QPushButton *mRefreshPushButton;
    QPushButton *mSwitchPushButton;
    QFrame *mDeviceSettingFrame;
private slots:
    void on_refreshPushButton_clicked();
    void on_switchPushButton_clicked();

    /*************************************************************************/
    // Data input settings module
protected:
    QComboBox *mInputModelComboBox;
    QCheckBox *mCycleEnableCheckBox;
    QLineEdit *mCycleTimeLineEdit;
    QPushButton *mReadinFilePushButton;
    QPushButton *mSaveInputDataPushButton;
    QCheckBox *mAddCRCCheckBox;
    QPushButton *mCrcSettingsPushButton;
    QPushButton *mClearInputPushButton;
    QPushButton *mSendPushButton;
    QTextEdit *mInputTextEdit;
    QComboBox *mCrcParameterModelsComboBox;
    QLabel *mCrcLabel;
    QPushButton *mAddInputItemPushButton;
    QPushButton *mDeleteInputItemPushButton;
    QListWidget *mInputDataItemListWidget;
    QPushButton *mPresetPushButton;
    QPushButton *mSendPresetPushButton;

    /*************************************************************************/
    // Message output module
protected:
    QLabel *mInfoLabel;

    /*************************************************************************/
    // Data output module
protected:
    QLabel *mRxLabel;
    QLabel *mTxLabel;
    QComboBox *mOutputTextFormatComboBox;
    QCheckBox *mShowDateCheckBox;
    QCheckBox *mAutoWrapCheckBox;
    QCheckBox *mShowTimeCheckBox;
    QCheckBox *mShowMsCheckBox;
    QCheckBox *mShowRxDataCheckBox;
    QCheckBox *mShowTxDataCheckBox;
    QCheckBox *mSaveOutputToFileCheckBox;
    QPushButton *mOutputFilePathPushButton;
    QPushButton *mClearOutputPushButton;
    QPushButton *mSaveOutputPushButton;
    QTextBrowser *mOutputTextBroswer;

    /*************************************************************************/
    // Data statistics module
protected:
    bool mReceivedFlag;
    bool mSendFlag;

    QLabel *mRxSpeedLabel;
    QLabel *mTxSpeedLabel;
    QLabel *mRxFramesLabel;
    QLabel *mTxFramesLabel;
    QLabel *mRxBytesLabel;
    QLabel *mTxBytesLabel;
    QPushButton *mResetTxCountPushButton;
    QPushButton *mResetRxCountPushButton;

    /*************************************************************************/
    // Other settings module
protected:
    QPushButton *mTransmissionSettingPushButton;
    QPushButton *mAnalyzerPushButton;
    QPushButton *mAutoResponseSettingPushButton;
    QPushButton *mTimingSendingPushButton;
    QPushButton *mHighlightSettingPushButton;
    QPushButton *mMoreSettingsPushButton;

    /*************************************************************************/
    // Charts module
protected:
    QPushButton *mDataVisualizationPushButton;
private slots:
    void on_dataVisualizationPushButton_clicked();
};

#endif  // SAKTABPAGE_HH
