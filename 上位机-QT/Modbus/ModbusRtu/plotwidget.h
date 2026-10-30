#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include <QWidget>
#include <QVector>

class PlotWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PlotWidget(QWidget *parent = nullptr);
    void addData(int target, int real);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<int> m_targetData;
    QVector<int> m_realData;
    int m_maxPoints;
};

#endif // PLOTWIDGET_H
