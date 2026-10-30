#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QTextEdit>

#include "serialportmanager.h"
#include "plotwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void refreshSerialPort();
    void slotOpenPort();
    void slotMotorStart();
    void slotMotorStop();
    void slotDirForward();
    void slotDirBackward();
    void slotSetTargetSpeed();
    void slotUpdateSpeed(int target, int real);
    void slotAutoReadSpeed();

private:
    QTabWidget* m_tabWidget;
    SerialPortManager* m_serialMgr;
    PlotWidget* m_plotWidget;
    QTimer* m_readTimer;

    // 串口页
    QComboBox* m_cmbPort;
    QComboBox* m_cmbBaud;
    QPushButton* m_btnOpen;

    // 电机控制页
    QLineEdit* m_editSpeed;
    QLabel* m_labTarget;
    QLabel* m_labReal;

    int m_currDir = 1;

  //  QTextEdit *m_logEdit;  // 加这一行

 //   void appendLog(const QString &dir, const QByteArray &data); // 加这一行

    QLabel *m_serialStatusLed;
    QTextEdit *m_logEdit;
    void appendLog(const QString &dir, const QByteArray &data);

private:
    int m_targetSpeed;
    int m_realSpeed;
};

#endif // MAINWINDOW_H
