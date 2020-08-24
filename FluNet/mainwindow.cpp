#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <pthread.h>
#include "seek.h"

#include <fstream>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <cmath>
#include <vector>
#include <iterator>
#include <string>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <stdlib.h>

LibSeek::SeekThermalPro thermalCamera("");
CascadeClassifier face;

using namespace std;
using namespace cv;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    feedTimer = new QTimer(this);
    coughTimer = new QTimer(this);
    alarmTimer = new QTimer(this);

    connect(feedTimer, SIGNAL(timeout()), this, SLOT(update_feed()));
    connect(coughTimer, SIGNAL(timeout()), this, SLOT(check_for_cough()));
    connect(alarmTimer, SIGNAL(timeout()), this, SLOT(play_alarm()));
    // Update the feed at apporximatly 15 FPS
    feedTimer->start(66);
    // Check to see if a cough was detected every 150ms
    coughTimer->start(150);
    alarmTimer->start(500);
    // Open thermal camera
    if (!thermalCamera.open()) cout << "Failed to open thermal camera!" << endl;
    // Load face classifier
    if(!face.load("/home/pi/FluNet/classifiers/faceClassifierV2.xml")) cout << "Failed to load face classifier!" << endl;

    additsional_flat_feild = true;
    try
    {
        calibration_frame = imread("/home/pi/IRProject/calibration-frame.png", -1);
    }
    catch (...)
    {
        additsional_flat_feild = false;
    }

    // Initilise an empty colour bar
    bar = Mat(240, 15, CV_8UC3, cv::Scalar(255,255,255));
    calibrating = false;
    circle = true;
    alarm = false;
    // Set slope and intercept  (after additional flat feild calibration)
 //   slope = 0.02300020326;
   // intercept = -69.09116877;
    slope = 0.02224832;
    intercept = -58.087727;
    // Set slope and intercept (after flat feild calibration)
   // slope = 0.01232;
    //intercept = 209.2;
    // Set slope and intercept (raw)
    //slope = 0.02414;
   // intercept = 148.3;

    alarmState = false;
   // ui->alarm->setVisible(false);
    ui->setupUi(this);
    ui->alarm->setPixmap(QPixmap("/home/pi/FluNet/images/alarm.png"));
    ui->alarm->setVisible(false);
}

MainWindow::~MainWindow()
{
    feedTimer->stop();
    coughTimer->stop();
    delete ui;
}

/* Update the feed at approximatly 15 FPS*/
void MainWindow::update_feed()
{
    if(!calibrating)
    {
        if (!thermalCamera.read(raw_frame, raw_temp_frame)) cout << "Lost connection to thermal camera!" << endl;

        // need to detect faces before additional flat feild calibration
        detect_faces();

        if(additsional_flat_feild) raw_frame += 0x4000 - calibration_frame;
        last_temp = raw_frame;

        process_thermal_frame();
        process_colour_bar();
        process_temperature();
        process_faces();

        thermalImage = QImage(processed_frame.data, processed_frame.cols, processed_frame.rows, QImage::Format_RGB888);
        ui->thermalFeed->setPixmap(QPixmap::fromImage(thermalImage));
        ui->thermalFeed->resize(ui->thermalFeed->pixmap()->size());
    }
}
/* Processes the current thermal frame (visually) */
void MainWindow::process_thermal_frame()
{
    temperature_frame = raw_frame;
    // Reduce noise
    medianBlur(raw_frame, raw_frame, 3);
    normalize(raw_frame, processed_frame, 0, 65535, cv::NORM_MINMAX);
    processed_frame.convertTo(processed_frame, CV_8UC1, 1.0/255.0);

    // Determine the selected colourmap
    QString cb = ui->heatmapChoice->currentText();
    if(QString::compare(cb, "Grey", Qt::CaseInsensitive) == 0)
    {
        // If default colour map do nothing
    } else if(QString::compare(cb, "Hot", Qt::CaseInsensitive) == 0)
        applyColorMap(processed_frame, processed_frame, COLORMAP_HOT);
    else if(QString::compare(cb, "Autumn", Qt::CaseInsensitive) == 0)
        applyColorMap(processed_frame, processed_frame, COLORMAP_AUTUMN);
    else if(QString::compare(cb, "Bone", Qt::CaseInsensitive) == 0)
        applyColorMap(processed_frame, processed_frame, COLORMAP_BONE);
    else if(QString::compare(cb, "Jet", Qt::CaseInsensitive) == 0)
        applyColorMap(processed_frame, processed_frame, COLORMAP_JET);
    else if(QString::compare(cb, "Winter", Qt::CaseInsensitive) == 0)
        applyColorMap(processed_frame, processed_frame, COLORMAP_WINTER);
    else if(QString::compare(cb, "Rainbow", Qt::CaseInsensitive) == 0)
        applyColorMap(processed_frame, processed_frame, COLORMAP_RAINBOW);
    else if(QString::compare(cb, "Ocean", Qt::CaseInsensitive) == 0)
        applyColorMap(processed_frame, processed_frame, COLORMAP_OCEAN);
    else if(QString::compare(cb, "Summer", Qt::CaseInsensitive) == 0)
        applyColorMap(processed_frame, processed_frame, COLORMAP_SUMMER);
    else if(QString::compare(cb, "Spring", Qt::CaseInsensitive) == 0)
        applyColorMap(processed_frame, processed_frame, COLORMAP_SPRING);
    else if(QString::compare(cb, "Cool", Qt::CaseInsensitive) == 0)
        applyColorMap(processed_frame, processed_frame, COLORMAP_COOL);
    else if(QString::compare(cb, "HSV", Qt::CaseInsensitive) == 0)
        applyColorMap(processed_frame, processed_frame, COLORMAP_HSV);
    else if(QString::compare(cb, "Pink", Qt::CaseInsensitive) == 0)
        applyColorMap(processed_frame, processed_frame, COLORMAP_PINK);
    else
         cout << "Undefined Colour Map!" << endl;

    // OpenCV uses BGR Format by default, swap to RGB so image displays correctly on screen
    cvtColor(processed_frame, processed_frame, COLOR_BGR2RGB);
    //Update the gui to show the new processed thermal image

}

/* Displays the correct colour bar */
void MainWindow::process_colour_bar()
{
    // Creates the colour bar
    for(int i = 0; i < 240; ++i)
        for(int j = 0; j < 15; ++j)
            bar.at<Vec3b>(239-i,j) = Vec3b(i,i,i);

    normalize(bar, bar, 0, 240, NORM_MINMAX);

    // Determine the selected colourmap
    QString cb = ui->heatmapChoice->currentText();
    if(QString::compare(cb, "Grey", Qt::CaseInsensitive) == 0)
    {
        // If default colour map do nothing
    } else if(QString::compare(cb, "Hot", Qt::CaseInsensitive) == 0)
        applyColorMap(bar, bar, COLORMAP_HOT);
    else if(QString::compare(cb, "Autumn", Qt::CaseInsensitive) == 0)
        applyColorMap(bar, bar, COLORMAP_AUTUMN);
    else if(QString::compare(cb, "Bone", Qt::CaseInsensitive) == 0)
        applyColorMap(bar, bar, COLORMAP_BONE);
    else if(QString::compare(cb, "Jet", Qt::CaseInsensitive) == 0)
        applyColorMap(bar, bar, COLORMAP_JET);
    else if(QString::compare(cb, "Winter", Qt::CaseInsensitive) == 0)
        applyColorMap(bar, bar, COLORMAP_WINTER);
    else if(QString::compare(cb, "Rainbow", Qt::CaseInsensitive) == 0)
        applyColorMap(bar, bar, COLORMAP_RAINBOW);
    else if(QString::compare(cb, "Ocean", Qt::CaseInsensitive) == 0)
        applyColorMap(bar, bar, COLORMAP_OCEAN);
    else if(QString::compare(cb, "Summer", Qt::CaseInsensitive) == 0)
        applyColorMap(bar, bar, COLORMAP_SUMMER);
    else if(QString::compare(cb, "Spring", Qt::CaseInsensitive) == 0)
        applyColorMap(bar, bar, COLORMAP_SPRING);
    else if(QString::compare(cb, "Cool", Qt::CaseInsensitive) == 0)
        applyColorMap(bar, bar, COLORMAP_COOL);
    else if(QString::compare(cb, "HSV", Qt::CaseInsensitive) == 0)
        applyColorMap(bar, bar, COLORMAP_HSV);
    else if(QString::compare(cb, "Pink", Qt::CaseInsensitive) == 0)
        applyColorMap(bar, bar, COLORMAP_PINK);
    else
        cout << "Undefined Colour Map!" << endl;

    cvtColor(bar, bar, COLOR_BGR2RGB);
    // OpenCV uses BGR Format by default, swap to RGB so image displays correctly on screen
    // Need to get rid of the weird black blob at the end (Displays correctly in the OpenCV window)
    QImage colourBar = QImage(bar.data, bar.cols, bar.rows, QImage::Format_RGB888);
    ui->colourbarFeed->setPixmap(QPixmap::fromImage(colourBar));
    ui->colourbarFeed->resize(ui->colourbarFeed->pixmap()->size());
}
/* Old reprposed button function (ignore me) */
void MainWindow::on_calibrate_clicked()
{
    auto end = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    string s(ctime(&end_time));

    writeCSV("/home/pi/Desktop/images/" + s.substr(0, s.size()-1) + ".csv", last_temp);
}

/* Grabs the temperture from the frame and updates the GUI */
void MainWindow::process_temperature()
{
    double min, max, foreground, background, mBackground;
    Point minLoc, maxLoc;
    cv::minMaxLoc(raw_frame, &min, &max, &minLoc, &maxLoc);
    // Use mean of image as the background
    Scalar m = cv::mean(raw_frame);
    mBackground = (slope * m[0]) + intercept;

    foreground = (slope * max) + intercept;
    background = (slope * min) + intercept;

    // Determine emissivity level
    double emissivity = 0;
    QString cb = ui->emmisivityChoice->currentText();

    if(QString::compare(cb, "Skin (0.98)", Qt::CaseInsensitive) == 0)
        emissivity = 0.98;
    else if(QString::compare(cb, "Matte (0.97)", Qt::CaseInsensitive) == 0)
        emissivity = 0.97;
    else if(QString::compare(cb, "Black Body (1.0)", Qt::CaseInsensitive) == 0)
        emissivity = 1.0;
    else if(QString::compare(cb, "Semi-matte (0.80)", Qt::CaseInsensitive) == 0)
        emissivity = 0.80;
    else if(QString::compare(cb, "Semi-glossy (0.60)", Qt::CaseInsensitive) == 0)
        emissivity = 0.60;
    else if(QString::compare(cb, "Glossy (0.30)", Qt::CaseInsensitive) == 0)
        emissivity = 0.30;
    else if(QString::compare(cb, "Black Body (1.0)", Qt::CaseInsensitive) == 0)
        emissivity = 1.0;
    else
        cout << "Undefined Emissivity!" << endl;

    cb = ui->temperatureChoice->currentText();
    // Determine correct temperture and add 1.5 (celcius / kelvin) to be conservative about the cameras error rate.
    double tempertureMax, tempertureMin;
    if(QString::compare(cb, "Kelvin", Qt::CaseInsensitive) == 0)
    {
        tempertureMax = apply_emissivity(foreground, mBackground, emissivity);
        tempertureMin = apply_emissivity(background, mBackground, emissivity);
    }
    else if(QString::compare(cb, "Fahenheit", Qt::CaseInsensitive) == 0)
    {
        tempertureMax = kelvin_to_farenheit(apply_emissivity(foreground, mBackground, emissivity));
        tempertureMin = kelvin_to_farenheit(apply_emissivity(background, mBackground, emissivity));
    }
    else if(QString::compare(cb, "Celsius", Qt::CaseInsensitive) == 0)
    {
        tempertureMax = kelvin_to_degrees_celcius(apply_emissivity(foreground, mBackground, emissivity));
        tempertureMin = kelvin_to_degrees_celcius(apply_emissivity(background, mBackground, emissivity));
    }
    else {
        cout << "Undefined Temperture Unit!" << endl;
    }
    // Update Temperture units on GUI
    ui->highT->setText(QString::number(ceil(tempertureMax)));
    ui->lowT->setText(QString::number(floor(tempertureMin)));

    // Determine which part of image to highlight
    cb = ui->highlight->currentText();

    if(QString::compare(cb, "Hottest Point", Qt::CaseInsensitive) == 0)
    {
        topLeft = Point(maxLoc.x - 10, maxLoc.y + 10);
        bottomRight = Point(maxLoc.x + 10, maxLoc.y - 10);
    } else {
        topLeft = Point(minLoc.x - 10, minLoc.y + 10);
        bottomRight = Point(minLoc.x + 10, minLoc.y - 10);
    }
}
/* Applys the emissivity correction to a temperature value in kelvin */
double MainWindow::apply_emissivity(double temperture, double background, double emissivity)
{
    return pow(((pow(temperture, 4) - (1.0f - emissivity) * pow(background, 4)) / emissivity), 0.25f);
}
/* Converts kelvin to degrees */
double MainWindow::kelvin_to_degrees_celcius(double kelvin)
{
    return kelvin - 273.15f;
}
/* Converts kelvin to farenheit */
double MainWindow::kelvin_to_farenheit(double kelvin)
{
    return (kelvin - 273.15f) * 1.8f + 32.0f;
}
/* Locates the faces in the frame */
void MainWindow::detect_faces()
{
    Mat temp;
    normalize(raw_frame, temp, 0, 65535, NORM_MINMAX);
    temp.convertTo(temp, CV_8UC1, 1.0/256.0);
    face.detectMultiScale(temp, faces, 1.3, 10);
}
/* Loops over all the detected faces and checks temperature */
void MainWindow::process_faces()
{
    QString tempIfFace = "No Faces Detected";
    double highTemperature = -INFINITY, minT, maxT, mBackground, highestLoc;

    Scalar m = cv::mean(temperature_frame);
    mBackground = (slope * m[0]) + intercept;

    // Draw face & sound alarm in needed
    for(size_t i = 0; i < faces.size(); ++i)
    {
        // Draw a circle around the "faces".
        if(circle)
        {
            Point center(faces[i].x + faces[i].width/2, faces[i].y + faces[i].height/2);
            ellipse(processed_frame, center, Size(faces[i].width/2, faces[i].height/2), 0, 0, 360, Scalar(120, 81, 169), 4);
        }
        // Get temperture readings from the face only (Only take the temperture of what we know is a human to minimise false positives).
        Mat faceROIs = temperature_frame(faces[i]);
        cv::minMaxLoc(faceROIs, &minT, &maxT);

        if(maxT > highTemperature)
        {
            highTemperature = maxT;
            //highestLoc = maxLoc;
        }
    }
    // If theres faces update GUI and check temperature
    if(highTemperature != -INFINITY)
    {
        // Determine emissivity level
        double emissivity = 0.98;
        QString cb = ui->emmisivityChoice->currentText();

        if(QString::compare(cb, "Skin (0.98)", Qt::CaseInsensitive) == 0)
            emissivity = 0.98;
        else if(QString::compare(cb, "Matte (0.97)", Qt::CaseInsensitive) == 0)
            emissivity = 0.97;
        else if(QString::compare(cb, "Black Body (1.0)", Qt::CaseInsensitive) == 0)
            emissivity = 1.0;
        else if(QString::compare(cb, "Semi-matte (0.80)", Qt::CaseInsensitive) == 0)
            emissivity = 0.80;
        else if(QString::compare(cb, "Semi-glossy (0.60)", Qt::CaseInsensitive) == 0)
            emissivity = 0.60;
        else if(QString::compare(cb, "Glossy (0.30)", Qt::CaseInsensitive) == 0)
            emissivity = 0.30;
        else if(QString::compare(cb, "Black Body (1.0)", Qt::CaseInsensitive) == 0)
            emissivity = 1.0;
        else
            cout << "Undefined Emissivity!" << endl;

        cb = ui->temperatureChoice->currentText();

        // Determine temperature
        highTemperature = (slope * highTemperature) + intercept;
        double tempertureMax;

        if(QString::compare(cb, "Kelvin", Qt::CaseInsensitive) == 0)
        {
            tempertureMax = apply_emissivity(highTemperature, mBackground, emissivity);
            tempIfFace = QString::number(tempertureMax);
        }
        else if(QString::compare(cb, "Fahenheit", Qt::CaseInsensitive) == 0)
        {
            tempertureMax = kelvin_to_farenheit(apply_emissivity(highTemperature, mBackground, emissivity));
            tempIfFace = QString::number(tempertureMax);
        }
        else if(QString::compare(cb, "Celsius", Qt::CaseInsensitive) == 0)
        {
            tempertureMax = kelvin_to_degrees_celcius(apply_emissivity(highTemperature, mBackground, emissivity));
            tempIfFace = QString::number(tempertureMax);
        }
        else {
            cout << "Undefined Temperture Unit!" << endl;
        }

        tempIfFace = QString::number(tempertureMax);
        // Sound alarm if needed
        if(tempertureMax > 38.5)//   or/and coughing)
        {
            alarm = true;
        }

    }
    // Draw rectangle around the hot of cold point
    QString cb = ui->highlight->currentText();

    if(QString::compare(cb, "None", Qt::CaseInsensitive) == 0)
    {

    } else {
        rectangle(processed_frame, topLeft, bottomRight, cv::Scalar(0, 255, 102), 2, 8);
    }

    ui->faceTemp->setText(tempIfFace);
}

/* Toggles the outlining of faces */
void MainWindow::on_checkBox_clicked(bool checked)
{
    if(checked) circle = true;
    else circle = false;
    alarm = true;
 //   play_alarm();
}

/* Sound recording and classification is done in Python stores values in text file. */
void MainWindow::check_for_cough()
{
    ifstream file("/home/pi/FluNet/coughing.txt");

    if(file.is_open())
        for(int i = 0; i < 3; ++i)
            if(i == 0)
                file >> classification;
            else if(i == 1)
                file >> probaility;
            else
                file >> sent;
}

/* Need to do a better alarm */
void MainWindow::play_alarm()
{
    if(alarm)
    {
        if(alarmState) ui->alarm->setVisible(false);
        else ui->alarm->setVisible(true);

        alarmState = !alarmState;
    } else {
      //  ui->alarm->setVisible(false);
        alarmState = false;
    }
    //ui->alarm->setVisable(false);

    //if(alarm) play sound for 1 second
    // if(alarm) system("mpg321 -g 100 ~/FluNet/alarm.mp3");
}

/* On stop alarm button in GUI */

void MainWindow::on_pushButton_clicked()
{
    alarm = false;
}

/* Writes a OpenCV matrix to a CSV file. */
void MainWindow::writeCSV(string filename, Mat m)
{
     ofstream myfile;
     myfile.open(filename.c_str());
     myfile << cv::format(m, cv::Formatter::FMT_CSV) << std::endl;
     myfile.close();
}
