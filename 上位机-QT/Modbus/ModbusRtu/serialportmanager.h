#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>

class SerialPortManager : public QObject
{
    Q_OBJECT
public:
    explicit SerialPortManager(QObject *parent = nullptr);
    ~SerialPortManager();

    bool openPort(const QString& portName, int baudRate);
    void closePort();
    bool isOpened() const;

    void readSpeedRegs();
    void writeTargetSpeed(int speed);
    void writeMotorCtrl(int run, int dir);

signals:
    void speedDataUpdate(int target, int real);
    void portOpenSuccess();
    void portCloseSuccess();
    // 新增：收发原始数据信号
    void recvRawData(const QByteArray& data);
    void sendRawData(const QByteArray& data);

private slots:
    void onReadyRead();

private:
    QSerialPort* m_serial = nullptr;
    QByteArray m_rxBuff;

    quint16 calcCRC16(const QByteArray& data);
    QByteArray makeReadFrame(quint8 slaveAddr, quint16 regAddr, quint16 regNum);
    QByteArray makeWriteRegFrame(quint8 slaveAddr, quint16 regAddr, quint16 val);
    void parseModbus(const QByteArray& pkg);
};

#endif // SERIALPORTMANAGER_H
