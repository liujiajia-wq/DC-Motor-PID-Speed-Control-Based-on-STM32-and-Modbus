#include "mainwindow.h"
#include <QSerialPortInfo>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QDateTime>
#include <QScrollBar>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("直流电机调速上位机");
    setMinimumSize(950, 750);
    setStyleSheet(R"(
        QMainWindow{
            background-color: #0F192D;
        }
        QPushButton{
            background-color: #1976D2;
            color: #ffffff;
            border-radius: 6px;
            padding: 8px;
            font-size: 14px;
            border: none;
        }
        QPushButton:hover{
            background-color: #2196F3;
        }
        QPushButton:pressed{
            background-color: #0D47A1;
            margin-top:2px;
            margin-bottom:-2px;
        }
        QComboBox, QLineEdit{
            background-color: #152742;
            color: #E3F2FD;
            border: 1px solid #1E88E5;
            border-radius: 6px;
            padding: 6px;
            font-size: 13px;
        }
        QLabel{
            color: #BBDEFB;
            font-size: 14px;
        }
        QTextEdit{
            background-color: #0B1220;
            color: #E3F2FD;
            border: 1px solid #1E88E5;
            border-radius: 6px;
            padding: 8px;
        }
    )");

    QFont globalFont;
    globalFont.setPointSize(10);
    setFont(globalFont);

    m_serialMgr = new SerialPortManager(this);
    m_readTimer = new QTimer(this);
    m_readTimer->setInterval(200);

    // 主布局
    QHBoxLayout *mainLayout = new QHBoxLayout;
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // 左侧面板
    QWidget *leftPanel = new QWidget;
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(15,15,15,15);
    leftLayout->setSpacing(15);
    leftPanel->setFixedWidth(260);
    leftPanel->setStyleSheet("background-color: #122038; border-radius: 8px;");

    // 串口状态指示灯
    QHBoxLayout *statusLayout = new QHBoxLayout;
    m_serialStatusLed = new QLabel;
    m_serialStatusLed->setFixedSize(16,16);
    m_serialStatusLed->setStyleSheet("background-color:#D32F2F;border-radius:8px;");
    statusLayout->addWidget(m_serialStatusLed);
    statusLayout->addWidget(new QLabel("串口未连接"));
    statusLayout->addStretch();
    leftLayout->addLayout(statusLayout);

    // 串口配置
    QGroupBox *serialGroup = new QGroupBox("串口配置");
    serialGroup->setStyleSheet("color: #4FC3F7; border: 1px solid #1E88E5; border-radius: 6px; padding: 10px;");
    QVBoxLayout *serialLayout = new QVBoxLayout(serialGroup);
    m_cmbPort = new QComboBox;
    m_cmbBaud = new QComboBox;
    m_btnOpen = new QPushButton("打开串口");
    m_btnOpen->setMinimumHeight(35);
    serialLayout->addWidget(new QLabel("串口号"));
    serialLayout->addWidget(m_cmbPort);
    serialLayout->addWidget(new QLabel("波特率"));
    serialLayout->addWidget(m_cmbBaud);
    serialLayout->addWidget(m_btnOpen);
    m_cmbBaud->addItems({"9600","19200","38400","57600","115200"});
    m_cmbBaud->setCurrentText("115200");
    refreshSerialPort();

    // 电机控制
    QGroupBox *motorGroup = new QGroupBox("电机控制");
    motorGroup->setStyleSheet("color: #4FC3F7; border: 1px solid #1E88E5; border-radius: 6px; padding: 10px;");
    QVBoxLayout *motorLayout = new QVBoxLayout(motorGroup);
    QPushButton *btnStart = new QPushButton("电机启动");
    QPushButton *btnStop  = new QPushButton("电机停止");
    QPushButton *btnFwd   = new QPushButton("正转");
    QPushButton *btnBwd   = new QPushButton("反转");
    btnStart->setMinimumHeight(30);
    btnStop->setMinimumHeight(30);
    btnFwd->setMinimumHeight(30);
    btnBwd->setMinimumHeight(30);
    m_editSpeed = new QLineEdit;
    m_editSpeed->setPlaceholderText("输入速度 0~100");
    QPushButton *btnSet = new QPushButton("设置速度");
    btnSet->setMinimumHeight(30);
    m_labTarget = new QLabel("目标速度：0");
    m_labReal   = new QLabel("当前速度：0");
    motorLayout->addWidget(btnStart);
    motorLayout->addWidget(btnStop);
    motorLayout->addWidget(btnFwd);
    motorLayout->addWidget(btnBwd);
    motorLayout->addWidget(m_editSpeed);
    motorLayout->addWidget(btnSet);
    motorLayout->addWidget(m_labTarget);
    motorLayout->addWidget(m_labReal);

    leftLayout->addWidget(serialGroup);
    leftLayout->addWidget(motorGroup);
    leftLayout->addStretch();

    // 右侧 曲线+日志
    QWidget *rightArea = new QWidget;
    QVBoxLayout *rightLayout = new QVBoxLayout(rightArea);
    rightLayout->setSpacing(10);
    rightLayout->setContentsMargins(10,10,10,10);

    m_plotWidget = new PlotWidget;
    m_plotWidget->setMinimumHeight(400);

    m_logEdit = new QTextEdit;
    m_logEdit->setReadOnly(true);
    m_logEdit->setMaximumHeight(220);
    m_logEdit->setPlaceholderText("串口收发日志（十六进制）");

    rightLayout->addWidget(m_plotWidget);
    rightLayout->addWidget(new QLabel("串口收发日志"));
    rightLayout->addWidget(m_logEdit);

    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightArea, 1);

    // 信号槽绑定
    connect(m_readTimer, &QTimer::timeout, this, &MainWindow::slotAutoReadSpeed);
    connect(m_serialMgr, &SerialPortManager::speedDataUpdate, this, &MainWindow::slotUpdateSpeed);

    // 收发数据日志
    connect(m_serialMgr, &SerialPortManager::sendRawData, this, [=](const QByteArray& d){
        appendLog("SEND", d);
    });
    connect(m_serialMgr, &SerialPortManager::recvRawData, this, [=](const QByteArray& d){
        appendLog("RECV", d);
    });

    connect(m_btnOpen, &QPushButton::clicked, this, &MainWindow::slotOpenPort);
    connect(btnStart, &QPushButton::clicked, this, &MainWindow::slotMotorStart);
    connect(btnStop,  &QPushButton::clicked, this, &MainWindow::slotMotorStop);
    connect(btnFwd,   &QPushButton::clicked, this, &MainWindow::slotDirForward);
    connect(btnBwd,   &QPushButton::clicked, this, &MainWindow::slotDirBackward);
    connect(btnSet,   &QPushButton::clicked, this, &MainWindow::slotSetTargetSpeed);

    // 串口状态灯
    connect(m_serialMgr, &SerialPortManager::portOpenSuccess, this, [=](){
        m_btnOpen->setText("关闭串口");
        m_serialStatusLed->setStyleSheet("background-color:#388E3C;border-radius:8px;");
        m_serialStatusLed->parentWidget()->findChild<QLabel*>()->setText("串口已连接");
        m_readTimer->start();
    });
    connect(m_serialMgr, &SerialPortManager::portCloseSuccess, this, [=](){
        m_btnOpen->setText("打开串口");
        m_serialStatusLed->setStyleSheet("background-color:#D32F2F;border-radius:8px;");
        m_serialStatusLed->parentWidget()->findChild<QLabel*>()->setText("串口未连接");
        m_readTimer->stop();
    });

    connect(m_serialMgr, &SerialPortManager::speedDataUpdate,
            this, &MainWindow::slotUpdateSpeed);
}

void MainWindow::appendLog(const QString &dir, const QByteArray &data)
{
    QString time = QDateTime::currentDateTime().toString("[hh:mm:ss.zzz] ");
    QString hexStr = data.toHex(' ').toUpper();

    QString color = (dir == "SEND") ? "#42A5F5" : "#66BB6A";
    QString html = QString("<span style='color:%1;'>%2 %3: %4</span>")
            .arg(color).arg(time).arg(dir).arg(hexStr);

    m_logEdit->append(html);
    m_logEdit->verticalScrollBar()->setValue(m_logEdit->verticalScrollBar()->maximum());
}

MainWindow::~MainWindow()
{
}

void MainWindow::refreshSerialPort()
{
    m_cmbPort->clear();
    foreach(QSerialPortInfo info, QSerialPortInfo::availablePorts())
    {
        m_cmbPort->addItem(info.portName());
    }
}

void MainWindow::slotOpenPort()
{
    if(m_serialMgr->isOpened())
    {
        m_serialMgr->closePort();
        return;
    }
    QString port = m_cmbPort->currentText();
    int baud = m_cmbBaud->currentText().toInt();
    m_serialMgr->openPort(port, baud);
}

// 启动
void MainWindow::slotMotorStart()
{
    m_currDir = 1;       // 启动命令
    m_serialMgr->writeMotorCtrl(1, 0);
}

// 停止
void MainWindow::slotMotorStop()
{
    m_currDir = 0;
    m_serialMgr->writeMotorCtrl(0, 0);
}

// 正转
void MainWindow::slotDirForward()
{
    m_currDir = 2;
    m_serialMgr->writeMotorCtrl(2, 0);
}

// 反转
void MainWindow::slotDirBackward()
{
    m_currDir = 3;
    m_serialMgr->writeMotorCtrl(3, 0);
}

void MainWindow::slotSetTargetSpeed()
{
    int speed = m_editSpeed->text().toInt();
    m_serialMgr->writeTargetSpeed(speed);
}

void MainWindow::slotAutoReadSpeed()
{
    m_serialMgr->readSpeedRegs();
}

void MainWindow::slotUpdateSpeed(int target, int real)
{
    // 更新界面文字
    m_labTarget->setText(QString("目标速度：%1").arg(target));
    m_labReal->setText(QString("当前速度：%1").arg(real));

    // 把数据推给折线图
    m_plotWidget->addData(target, real);
}
