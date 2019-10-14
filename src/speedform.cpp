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

#include "speedform.h"
#include "ui_speedform.h"

#include "common.h"
#include "datapoint.h"
#include "mainwindow.h"
#include "plotvalue.h"
#include "speedscoring.h"
#include "ppcupload.h"

SpeedForm::SpeedForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SpeedForm),
    mMainWindow(0)
{
    ui->setupUi(this);

    connect(ui->faiButton, SIGNAL(clicked()), this, SLOT(onFAIButtonClicked()));
    connect(ui->upButton, SIGNAL(clicked()), this, SLOT(onUpButtonClicked()));
    connect(ui->downButton, SIGNAL(clicked()), this, SLOT(onDownButtonClicked()));

    connect(ui->bottomEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));

    // Connect optimization buttons
    connect(ui->actualButton, SIGNAL(clicked()), this, SLOT(onActualButtonClicked()));
    connect(ui->optimalButton, SIGNAL(clicked()), this, SLOT(onOptimalButtonClicked()));
    connect(ui->optimizeButton, SIGNAL(clicked()), this, SLOT(onOptimizeButtonClicked()));

    // Connect PPC button
    connect(ui->ppcButton, SIGNAL(clicked()), this, SLOT(onPpcButtonClicked()));
}

SpeedForm::~SpeedForm()
{
    delete ui;
}

QSize SpeedForm::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(175, 175);
}

void SpeedForm::setMainWindow(
        MainWindow *mainWindow)
{
    mMainWindow = mainWindow;
}

void SpeedForm::updateView()
{
    // Update mode selection
    ui->actualButton->setChecked(mMainWindow->windowMode() == MainWindow::Actual);
    ui->optimalButton->setChecked(mMainWindow->windowMode() == MainWindow::Optimal);

    SpeedScoring *method = (SpeedScoring *) mMainWindow->scoringMethod(MainWindow::Speed);

    const double bottom = method->windowBottom();

    // Update window bounds
    ui->bottomEdit->setText(QString("%1").arg(bottom));

    DataPoint dpBottom, dpTop;
    bool success;

    switch (mMainWindow->windowMode())
    {
    case MainWindow::Actual:
        success = method->getWindowBounds(mMainWindow->data(), dpBottom, dpTop);
        break;
    case MainWindow::Optimal:
        success = method->getWindowBounds(mMainWindow->optimal(), dpBottom, dpTop);
        break;
    }

    if (success)
    {
        // Calculate results
        const double time = dpBottom.t - dpTop.t;
        const double verticalSpeed = (dpTop.z - dpBottom.z) / time;

        // Update display
        if (mMainWindow->units() == PlotValue::Metric)
        {
            ui->verticalSpeedEdit->setText(QString("%1").arg(verticalSpeed * MPS_TO_KMH));
            ui->verticalSpeedUnits->setText(tr("km/h"));
        }
        else
        {
            ui->verticalSpeedEdit->setText(QString("%1").arg(verticalSpeed * MPS_TO_MPH));
            ui->verticalSpeedUnits->setText(tr("mph"));
        }
        ui->actualButton->setEnabled(true);
        ui->optimizeButton->setEnabled(true);
        ui->ppcButton->setEnabled(true);
    }
    else
    {
        // Update display
        if (mMainWindow->units() == PlotValue::Metric)
        {
            ui->verticalSpeedUnits->setText(tr("km/h"));
        }
        else
        {
            ui->verticalSpeedUnits->setText(tr("mph"));
        }

        ui->verticalSpeedEdit->setText(tr("n/a"));

        ui->actualButton->setEnabled(false);
        ui->optimalButton->setEnabled(false);
        ui->optimizeButton->setEnabled(false);
        ui->ppcButton->setEnabled(false);
    }
}

void SpeedForm::onFAIButtonClicked()
{
    SpeedScoring *method = (SpeedScoring *) mMainWindow->scoringMethod(MainWindow::Speed);
    method->setWindow(1700);
}

void SpeedForm::onApplyButtonClicked()
{
    double bottom = ui->bottomEdit->text().toDouble();

    SpeedScoring *method = (SpeedScoring *) mMainWindow->scoringMethod(MainWindow::Speed);
    method->setWindow(bottom);

    mMainWindow->setFocus();
}

void SpeedForm::onUpButtonClicked()
{
    SpeedScoring *method = (SpeedScoring *) mMainWindow->scoringMethod(MainWindow::Speed);
    method->setWindow(method->windowBottom() + 10);
}

void SpeedForm::onDownButtonClicked()
{
    SpeedScoring *method = (SpeedScoring *) mMainWindow->scoringMethod(MainWindow::Speed);
    method->setWindow(method->windowBottom() - 10);
}

void SpeedForm::keyPressEvent(QKeyEvent *event)
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

void SpeedForm::onActualButtonClicked()
{
    mMainWindow->setWindowMode(MainWindow::Actual);
}

void SpeedForm::onOptimalButtonClicked()
{
    mMainWindow->setWindowMode(MainWindow::Optimal);
}

void SpeedForm::onOptimizeButtonClicked()
{
    SpeedScoring *method = (SpeedScoring *) mMainWindow->scoringMethod(MainWindow::Speed);

    // Perform optimization
    method->optimize();

    ui->optimalButton->setEnabled(true);

    // Switch to optimal view
    mMainWindow->setWindowMode(MainWindow::Optimal);
}

void SpeedForm::onPpcButtonClicked() {

    // Return if plot empty
    if (mMainWindow->dataSize() == 0) return;

    PPCUpload *uploader = new PPCUpload(mMainWindow);
    SpeedScoring *method = (SpeedScoring *) mMainWindow->scoringMethod(MainWindow::Speed);
    DataPoint dpBottom, dpTop;

    ui->faiButton->click();
    ui->actualButton->click();

    if (method->getWindowBounds(mMainWindow->data(), dpBottom, dpTop)) {

        const double time = dpBottom.t - dpTop.t;
        const double distance = mMainWindow->getDistance(dpTop, dpBottom);
        const double windowTop = dpTop.z;
        const double windowBottom = dpBottom.z;

        uploader->upload("SS", windowTop, windowBottom, time, distance);
    }
}
