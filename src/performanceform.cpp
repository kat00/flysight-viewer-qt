/***************************************************************************
**                                                                        **
**  FlySight Viewer                                                       **
**  Copyright 2018 Michael Cooper, Klaus Rheinwald                        **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>. **
**                                                                        **
****************************************************************************
**  Contact: Michael Cooper                                               **
**  Website: http://flysight.ca/                                          **
****************************************************************************/

#include "performanceform.h"
#include "ui_performanceform.h"

#include "common.h"
#include "dataplot.h"
#include "datapoint.h"
#include "mainwindow.h"
#include "performancescoring.h"
#include "plotvalue.h"
#include "ppcupload.h"

PerformanceForm::PerformanceForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PerformanceForm),
    mMainWindow(0)
{
    ui->setupUi(this);

    connect(ui->startEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));
    connect(ui->endEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));

    // Connect PPC button
    connect(ui->ppcButton, SIGNAL(clicked()), this, SLOT(onPpcButtonClicked()));
}

PerformanceForm::~PerformanceForm()
{
    delete ui;
}

QSize PerformanceForm::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(175, 175);
}

void PerformanceForm::setMainWindow(
        MainWindow *mainWindow)
{
    mMainWindow = mainWindow;
}

void PerformanceForm::updateView()
{
    // Return if plot empty
    if (mMainWindow->dataSize() == 0) return;

    PerformanceScoring *method = (PerformanceScoring *) mMainWindow->scoringMethod(MainWindow::Performance);

    const double start = method->startTime();
    const double end = method->endTime();

    // Update window bounds
    ui->startEdit->setText(QString("%1").arg(start));
    ui->endEdit->setText(QString("%1").arg(end));

    DataPoint dpStart = mMainWindow->interpolateDataT(start);
    DataPoint dpEnd = mMainWindow->interpolateDataT(end);

    // Calculate results
    const double time = dpEnd.t - dpStart.t;
    const double vDistance = dpStart.z - dpEnd.z;
    const double hDistance = mMainWindow->getDistance(dpStart, dpEnd);

    // Update display
    ui->timeEdit->setText(QString("%1").arg(time, 0, 'f', 3));
    ui->vDistanceEdit->setText(QString("%1").arg(vDistance, 0, 'f', 3));
    ui->hDistanceEdit->setText(QString("%1").arg(hDistance, 0, 'f', 3));

    // Auxiliary values
    ui->startLatEdit->setText(QString("%1").arg(dpStart.lat, 0, 'f', 7));
    ui->startLonEdit->setText(QString("%1").arg(dpStart.lon, 0, 'f', 7));
    ui->startElevEdit->setText(QString("%1").arg(dpStart.z, 0, 'f', 3));

    ui->endLatEdit->setText(QString("%1").arg(dpEnd.lat, 0, 'f', 7));
    ui->endLonEdit->setText(QString("%1").arg(dpEnd.lon, 0, 'f', 7));
    ui->endElevEdit->setText(QString("%1").arg(dpEnd.z, 0, 'f', 3));
}

void PerformanceForm::onApplyButtonClicked()
{
    double start = ui->startEdit->text().toDouble();
    double end = ui->endEdit->text().toDouble();

    PerformanceScoring *method = (PerformanceScoring *) mMainWindow->scoringMethod(MainWindow::Performance);
    method->setRange(start, end);

    ui->ppcButton->setEnabled(start < end);

    mMainWindow->setFocus();
}

void PerformanceForm::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        // Reset window bounds
        updateView();

        // Release focus
        mMainWindow->setFocus();
    }

    QWidget::keyPressEvent(event);
}

void PerformanceForm::onPpcButtonClicked() {

    // Return if plot empty
    if (mMainWindow->dataSize() == 0) return;

    PPCUpload *uploader = new PPCUpload(mMainWindow);
    PerformanceScoring *method = (PerformanceScoring *) mMainWindow->scoringMethod(MainWindow::Performance);
    DataPoint dpTop = mMainWindow->interpolateDataT(method->startTime());
    DataPoint dpBottom = mMainWindow->interpolateDataT(method->endTime());

    const double time = dpBottom.t - dpTop.t;
    const double distance = mMainWindow->getDistance(dpTop, dpBottom);
    const double windowTop = dpTop.z;
    const double windowBottom = dpBottom.z;

    uploader->upload("WS", windowTop, windowBottom, time, distance);
}

