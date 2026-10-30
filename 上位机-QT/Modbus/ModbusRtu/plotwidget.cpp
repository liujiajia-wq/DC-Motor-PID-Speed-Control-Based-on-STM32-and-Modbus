#include "plotwidget.h"
#include <QPainter>
#include <QPen>
#include <QFont>

PlotWidget::PlotWidget(QWidget *parent)
    : QWidget(parent), m_maxPoints(120)
{
    setStyleSheet("background-color:#0B1220;");
}

void PlotWidget::addData(int target, int real)
{
    m_targetData.append(target);
    m_realData.append(real);

    if(m_targetData.size() > m_maxPoints) {
        m_targetData.removeFirst();
        m_realData.removeFirst();
    }

    update();
}

void PlotWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect rect = this->rect();
    rect.adjust(60, 30, -20, -50);

    int w = rect.width();
    int h = rect.height();
    int left = rect.left();
    int right = rect.right();
    int top = rect.top();
    int bottom = rect.bottom();

    // 网格
    painter.setPen(QPen(QColor(40, 40, 70), 1));
    for (int i = 0; i <= 10; i++) {
        int x = left + w * i / 10;
        painter.drawLine(x, top, x, bottom);
    }
    for (int i = 0; i <= 5; i++) {
        int y = top + h * i / 5;
        painter.drawLine(left, y, right, y);
    }

    // 坐标轴
    painter.setPen(QPen(Qt::white, 2));
    painter.drawLine(left, top, left, bottom);
    painter.drawLine(left, bottom, right, bottom);

    int minVal = 0;
    int maxVal = 100;

    // 字体
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    painter.setPen(Qt::white);

    // ---------------------- 纵轴：脉冲数 ----------------------
    for (int i = 0; i <= 5; i++) {
        int y = bottom - h * i / 5;
        int val = minVal + (maxVal - minVal) * i / 5;
        painter.drawLine(left - 5, y, left, y);
        painter.drawText(left - 45, y + 4, QString::number(val));
    }
    painter.drawText(left - 55, top - 10, "速度(脉冲)");

    // ---------------------- 横轴：时间(ms) ----------------------
    for (int i = 0; i <= 10; i++) {
        int x = left + w * i / 10;
        painter.drawLine(x, bottom, x, bottom + 5);
        painter.drawText(x - 10, bottom + 20, QString::number(i * 200));
    }
    painter.drawText(right - 40, bottom + 40, "时间(ms)");

    if (m_targetData.size() < 2) return;

    // 目标速度（蓝色）
    painter.setPen(QPen(QColor(66, 165, 245), 2));
    for (int i = 0; i < m_targetData.size()-1; i++) {
        int x1 = left + w * i / (m_maxPoints - 1);
        int y1 = bottom - (m_targetData[i] - minVal) * h / (maxVal - minVal);
        int x2 = left + w * (i+1) / (m_maxPoints - 1);
        int y2 = bottom - (m_targetData[i+1] - minVal) * h / (maxVal - minVal);
        painter.drawLine(x1, y1, x2, y2);
    }

    // 当前速度（绿色）
    painter.setPen(QPen(QColor(102, 187, 106), 2));
    for (int i = 0; i < m_realData.size()-1; i++) {
        int x1 = left + w * i / (m_maxPoints - 1);
        int y1 = bottom - (m_realData[i] - minVal) * h / (maxVal - minVal);
        int x2 = left + w * (i+1) / (m_maxPoints - 1);
        int y2 = bottom - (m_realData[i+1] - minVal) * h / (maxVal - minVal);
        painter.drawLine(x1, y1, x2, y2);
    }

    // 图例
    painter.setPen(QColor(66, 165, 245));
    painter.drawText(right - 100, 25, "目标速度");

    painter.setPen(QColor(102, 187, 106));
    painter.drawText(right - 100, 45, "当前速度");
}
