#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "seek.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <iostream>

using namespace std;
using namespace cv;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void update_feed();
    void process_thermal_frame();
    void process_colour_bar();
    void on_calibrate_clicked();
    void process_temperature();
    void detect_faces();
    void process_faces();
    void check_for_cough();
    void play_alarm();
    void writeCSV(string filename, Mat m);


    double apply_emissivity(double temperture, double background, double emissivity);
    double kelvin_to_degrees_celcius(double kelvin);
    double kelvin_to_farenheit(double kelvin);

    void on_checkBox_clicked(bool checked);

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    QTimer *feedTimer, *coughTimer, *alarmTimer;
    Mat raw_frame, processed_frame, temperature_frame, bar, calibration_frame, last_temp, raw_temp_frame;
    QImage thermalImage, alarmImage;
    Point topLeft, bottomRight;
    vector<Rect> faces;
    string classification;
    bool calibrating, additsional_flat_feild, circle, sent, alarm, alarmState;
    double slope, intercept, probaility;
    //QMediaPlayer audioPLayer;
};

#endif // MAINWINDOW_H
