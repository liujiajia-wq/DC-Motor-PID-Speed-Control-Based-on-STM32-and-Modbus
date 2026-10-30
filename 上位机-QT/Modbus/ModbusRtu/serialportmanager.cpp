#include "serialportmanager.h"
#include <QDebug>

#define SLAVE_ADDR 0x01

SerialPortManager::SerialPortManager(QObject *parent)
    : QObject(parent)
{
    m_serial = new QSerialPort(this);
    connect(m_serial, &QSerialPort::readyRead, this, &SerialPortManager::onReadyRead);
}

SerialPortManager::~SerialPortManager()
{
    closePort();
}

bool SerialPortManager::openPort(const QString &portName, int baudRate)
{
    if(m_serial->isOpen())
        closePort();

    m_serial->setPortName(portName);
    m_serial->setBaudRate(baudRate);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    bool ok = m_serial->open(QIODevice::ReadWrite);
    if(ok) {
        emit portOpenSuccess();
        qDebug() << "串口打开成功！";
    } else {
        qDebug() << "打开失败：" << m_serial->errorString();
    }
    return ok;
}

void SerialPortManager::closePort()
{
    if(m_serial->isOpen()) {
        m_serial->close();
        emit portCloseSuccess();
    }
}

bool SerialPortManager::isOpened() const
{
    return m_serial->isOpen();
}

// 【关键】接收数据并强制解析
void SerialPortManager::onReadyRead()
{
    QByteArray data = m_serial->readAll();
    emit recvRawData(data);

    qDebug() << "收到数据：" << data.toHex(' ');

    // 直接解析！
    parseModbus(data);
}

quint16 SerialPortManager::calcCRC16(const QByteArray &data)
{
    quint16 crc = 0xFFFF;
    for(uchar ch : data) {
        crc ^= ch;
        for(int i=0; i<8; i++) {
            if(crc & 1) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

QByteArray SerialPortManager::makeReadFrame(quint8 slaveAddr, quint16 regAddr, quint16 regNum)
{
    QByteArray frame;
    frame.append(slaveAddr);
    frame.append(0x03);
    frame.append((regAddr >> 8) & 0xFF);
    frame.append(regAddr & 0xFF);
    frame.append((regNum >> 8) & 0xFF);
    frame.append(regNum & 0xFF);
    quint16 crc = calcCRC16(frame);
    frame.append(crc & 0xFF);
    frame.append((crc >> 8) & 0xFF);
    return frame;
}

QByteArray SerialPortManager::makeWriteRegFrame(quint8 slaveAddr, quint16 regAddr, quint16 val)
{
    QByteArray frame;
    frame.append(slaveAddr);
    frame.append(0x06);
    frame.append((regAddr >> 8) & 0xFF);
    frame.append(regAddr & 0xFF);
    frame.append((val >> 8) & 0xFF);
    frame.append(val & 0xFF);
    quint16 crc = calcCRC16(frame);
    frame.append(crc & 0xFF);
    frame.append((crc >> 8) & 0xFF);
    return frame;
}

void SerialPortManager::readSpeedRegs()
{
    if(!isOpened()) return;
    QByteArray f = makeReadFrame(SLAVE_ADDR, 0, 2);
    m_serial->write(f);
    emit sendRawData(f);
}

void SerialPortManager::writeTargetSpeed(int speed)
{
    if(!isOpened()) return;
    if(speed < 0) speed = 0;
    if(speed > 100) speed = 100;
    QByteArray f = makeWriteRegFrame(SLAVE_ADDR, 0, speed);
    m_serial->write(f);
    emit sendRawData(f);
}

void SerialPortManager::writeMotorCtrl(int run, int dir)
{
    if(!isOpened()) return;
    QByteArray f = makeWriteRegFrame(SLAVE_ADDR, 1, run);
    m_serial->write(f);
    emit sendRawData(f);
}

// ==============================================
// 【最终正确解析】收到什么解析什么！
// ==============================================
void SerialPortManager::parseModbus(const QByteArray &pkg)
{
    qDebug() << "进入解析：" << pkg.toHex(' ');

    if (pkg.size() != 9) {
        qDebug() << "长度不对，不是速度帧";
        return;
    }

    // 标准回复：01 03 04 目标H 目标L 当前H 当前L CRC
    uint8_t addr   = pkg[0];
    uint8_t func   = pkg[1];
    uint8_t len    = pkg[2];

    if (addr != 0x01 || func != 0x03 || len != 0x04) {
        qDebug() << "格式不匹配";
        return;
    }

    int target = (pkg[3] << 8) | pkg[4];
    int real   = (pkg[5] << 8) | pkg[6];

    qDebug() << "解析成功！目标速度：" << target << " 当前速度：" << real;

    // 【关键】发信号给界面！
    emit speedDataUpdate(target, real);
}
