#include "orthoview.h"

#include <QVector3D>

#include "common.h"
#include "mainwindow.h"

OrthoView::OrthoView(QWidget *parent) :
    QCustomPlot(parent),
    mMainWindow(0),
    m_pan(false),
    m_azimuth(-PI/2),
    m_elevation(PI/2)
{
    setMouseTracking(true);
}

QSize OrthoView::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(175, 175);
}

void OrthoView::mousePressEvent(
        QMouseEvent *event)
{
    if (axisRect()->rect().contains(event->pos()))
    {
        m_beginPos = event->pos() - axisRect()->center();
        m_pan = true;
    }

    QCustomPlot::mousePressEvent(event);
}

void OrthoView::mouseReleaseEvent(
        QMouseEvent *event)
{
    m_pan = false;
    QCustomPlot::mouseReleaseEvent(event);
}

void OrthoView::mouseMoveEvent(
        QMouseEvent *event)
{
    if (m_pan)
    {
        QRect rect = axisRect()->rect();
        QPoint endPos = event->pos() - rect.center();

        double a1 = (double) (m_beginPos.x() - rect.left()) / rect.width();
        double a2 = (double) (endPos.x() - rect.left()) / rect.width();
        double a = a2 - a1;

        while (a < -PI) a += 2 * PI;
        while (a >  PI) a -= 2 * PI;

        double e1 = (double) (m_beginPos.y() - rect.bottom()) / rect.height();
        double e2 = (double) (endPos.y() - rect.bottom()) / rect.height();
        double e = e2 - e1;

        m_azimuth   -= a;
        m_elevation += e;

        m_elevation = qMax(m_elevation, -PI / 2);
        m_elevation = qMin(m_elevation,  PI / 2);

        m_beginPos = endPos;

        update();
    }

    if (QCPCurve *curve = qobject_cast<QCPCurve *>(plottable(0)))
    {
        const QCPCurveDataMap *data = curve->data();

        double resultTime;
        double resultDistance = std::numeric_limits<double>::max();

        for (QCPCurveDataMap::const_iterator it = data->constBegin();
             it != data->constEnd() && (it + 1) != data->constEnd();
             ++ it)
        {
            QPointF pt1 = QPointF(xAxis->coordToPixel(it.value().key),
                                  yAxis->coordToPixel(it.value().value));
            QPointF pt2 = QPointF(xAxis->coordToPixel((it + 1).value().key),
                                  yAxis->coordToPixel((it + 1).value().value));

            double mu;
            double dist = sqrt(distSqrToLine(pt1, pt2, event->pos(), mu));

            if (dist < resultDistance)
            {
                double t1 = it.value().t;
                double t2 = (it + 1).value().t;

                resultTime = t1 + mu * (t2 - t1);
                resultDistance = dist;
            }
        }

        if (resultDistance < selectionTolerance())
        {
            mMainWindow->setMark(resultTime);
        }
        else
        {
            mMainWindow->clearMark();
        }
    }
}

void OrthoView::updateView()
{
    // Calculate camera vectors
    QVector3D up(-sin(m_elevation) * cos(m_azimuth),
                 -sin(m_elevation) * sin(m_azimuth),
                  cos(m_elevation));
    QVector3D bk(cos(m_elevation) * cos(m_azimuth),
                 cos(m_elevation) * sin(m_azimuth),
                 sin(m_elevation));
    QVector3D rt = QVector3D::crossProduct(up, bk);

    double lower = mMainWindow->rangeLower();
    double upper = mMainWindow->rangeUpper();

    QVector< double > t, x, y, z;

    double xMin, xMax;
    double yMin, yMax;
    double zMin, zMax;

    bool first = true;

    for (int i = 0; i < mMainWindow->dataSize(); ++i)
    {
        const DataPoint &dp = mMainWindow->dataPoint(i);

        if (lower <= dp.t && dp.t <= upper)
        {
            t.append(dp.t);

            QVector3D cur;
            if (mMainWindow->units() == PlotValue::Metric)
            {
                cur = QVector3D(dp.x, dp.y, dp.z);
            }
            else
            {
                cur = QVector3D(dp.x, dp.y, dp.z) * METERS_TO_FEET;
            }

            x.append(QVector3D::dotProduct(cur, rt));
            y.append(QVector3D::dotProduct(cur, up));
            z.append(QVector3D::dotProduct(cur, bk));

            if (first)
            {
                xMin = xMax = x.back();
                yMin = yMax = y.back();
                zMin = zMax = z.back();

                first = false;
            }
            else
            {
                if (x.back() < xMin) xMin = x.back();
                if (x.back() > xMax) xMax = x.back();

                if (y.back() < yMin) yMin = y.back();
                if (y.back() > yMax) yMax = y.back();

                if (z.back() < zMin) zMin = z.back();
                if (z.back() > zMax) zMax = z.back();
            }
        }
    }

    clearPlottables();

    QCPCurve *curve = new QCPCurve(xAxis, yAxis);
    curve->setData(t, x, y);
    curve->setPen(QPen(Qt::black));
    addPlottable(curve);

    setViewRange(xMin, xMax, yMin, yMax);

    if (mMainWindow->markActive())
    {
        const DataPoint &dpEnd = mMainWindow->interpolateDataT(mMainWindow->markEnd());

        QVector< double > xMark, yMark, zMark;

        QVector3D cur;
        if (mMainWindow->units() == PlotValue::Metric)
        {
            cur = QVector3D(dpEnd.x, dpEnd.y, dpEnd.z);
        }
        else
        {
            cur = QVector3D(dpEnd.x, dpEnd.y, dpEnd.z) * METERS_TO_FEET;
        }

        xMark.append(QVector3D::dotProduct(cur, rt));
        yMark.append(QVector3D::dotProduct(cur, up));
        zMark.append(QVector3D::dotProduct(cur, bk));

        QCPGraph *graph = addGraph();
        graph->setData(xMark, yMark);
        graph->setPen(QPen(Qt::black));
        graph->setLineStyle(QCPGraph::lsNone);
        graph->setScatterStyle(QCPScatterStyle::ssDisc);
    }

    if (mMainWindow->dataSize() > 0)
    {
        QVector< double > xMark, yMark, zMark;

        for (int i = 0; i < mMainWindow->waypointSize(); ++i)
        {
            const DataPoint &dp0 = mMainWindow->dataPoint(
                        mMainWindow->dataSize() - 1);
            DataPoint dp = mMainWindow->waypoint(i);

            double distance = mMainWindow->getDistance(dp0, dp);
            double bearing = mMainWindow->getBearing(dp0, dp);

            dp.x = distance * sin(bearing);
            dp.y = distance * cos(bearing);

            QVector3D cur;
            if (mMainWindow->units() == PlotValue::Metric)
            {
                cur = QVector3D(dp.x, dp.y, dp.z);
            }
            else
            {
                cur = QVector3D(dp.x, dp.y, dp.z) * METERS_TO_FEET;
            }

            xMark.append(QVector3D::dotProduct(cur, rt));
            yMark.append(QVector3D::dotProduct(cur, up));
            zMark.append(QVector3D::dotProduct(cur, bk));
        }

        QCPGraph *graph = addGraph();
        graph->setData(xMark, yMark);
        graph->setPen(QPen(Qt::black));
        graph->setLineStyle(QCPGraph::lsNone);
        graph->setScatterStyle(QCPScatterStyle::ssPlus);
    }

    replot();
}

void OrthoView::setViewRange(
        double xMin,
        double xMax,
        double yMin,
        double yMax)
{
    QPainter painter(this);

    double xMMperPix = (double) painter.device()->widthMM() / painter.device()->width();
    double yMMperPix = (double) painter.device()->heightMM() / painter.device()->height();

    QRect rect = axisRect()->rect();

    double xSpan = (xMax - xMin) * 1.2;
    double ySpan = (yMax - yMin) * 1.2;

    double xScale = xSpan / rect.width() / xMMperPix;
    double yScale = ySpan / rect.height() / yMMperPix;

    double scale = qMax(xScale, yScale);

    double xMid = (xMin + xMax) / 2;
    double yMid = (yMin + yMax) / 2;

    xMin = xMid - rect.width() * xMMperPix * scale / 2;
    xMax = xMid + rect.width() * xMMperPix * scale / 2;

    yMin = yMid - rect.height() * yMMperPix * scale / 2;
    yMax = yMid + rect.height() * yMMperPix * scale / 2;

    xAxis->setRange(xMin, xMax);
    yAxis->setRange(yMin, yMax);
}
