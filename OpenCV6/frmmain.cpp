#include "frmmain.h"
#include "ui_frmmain.h"

#include <QtCore>
#include <QMessageBox>

frmmain::frmmain(QWidget *parent) : QMainWindow(parent), ui(new Ui::frmmain)
{
    ui->setupUi(this);

    serialport = new QSerialPort(this);
    serialport->setPortName("COM7");
    serialport->setBaudRate(QSerialPort::Baud9600);
    serialport->setDataBits(QSerialPort::Data8);
    serialport->setParity(QSerialPort::NoParity);
    serialport->setStopBits(QSerialPort::OneStop);
    serialport->setFlowControl(QSerialPort::NoFlowControl);
    serialport->open(QIODevice::ReadWrite);

    if(serialport->isWritable() != true)
    {
        QMessageBox::information(this, "error", "error: serial port");
    }

    cap.open("D:/opencv/test/test3.mov");
    if (cap.isOpened() != true)
    {
        QMessageBox::information(this, "error", "error: file doesn't find\n");
        exitProgram();
        return;
    }

    fout.open("data.txt", std::ios_base::trunc);
    if (!fout.is_open())
    {
        QMessageBox::information(this, "error", "error: file isn't open\n");
    }

    qtimer = new QTimer(this);
    qtimer1 = new QTimer(this);
    connect(qtimer, SIGNAL(timeout()), this, SLOT(processFrameAndUpdateGUI()));
    qtimer->start(20);

    connect(qtimer1, SIGNAL(timeout()), this, SLOT(processSendUART()));
    qtimer1->start(1);

    connect(serialport, SIGNAL(readyRead()), this, SLOT(UART_Receive()));

    data_pos[0] = 0xF8;
    data_pos[1] = 0x00;
    data_pos[2] = 0x00;
    data_pos[3] = 0x00;
    data_pos[4] = 0x00;
    data_pos[5] = 0x00;
    data_pos[6] = 0x00;
    data_pos[7] = 0x01;
    data_pos[8] = 0xE0;
    data_pos[9] = 0xFF;
    data_pos[10] = 0xFF;
    data_pos[11] = 0xFF;

}

frmmain::~frmmain()
{
    delete ui;
}

void delay(int n)
{
    QTime dieTime= QTime::currentTime().addSecs(n);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void frmmain::processFrameAndUpdateGUI()
{
    cv::Mat imageHSV;
    cv::Mat threshold;
    cv::Mat morphology;
    std::vector<std::vector<cv::Point>> contours;

    bool blnFrameReadSuccessfully = cap.read(src);

    if(src.empty())
    {
        //exitProgram();
        return;
    }
    else if(!blnFrameReadSuccessfully)
    {
        QMessageBox::information(this, "error", "unable to read from file \n\n exiting program\n");
        exitProgram();
        return;
    }

    cv::resize(src, imgOriginal, cv::Size(src.cols * 0.5, src.rows * 0.5), 0, 0, cv::INTER_LINEAR);
    cv::cvtColor(imgOriginal, imageHSV, cv::COLOR_BGR2HSV);

    cv::Scalar h_min(g_switch_value_h1, g_switch_value_s1, g_switch_value_v1);
    cv::Scalar h_max(g_switch_value_h2, g_switch_value_s2, g_switch_value_v2);
    cv::inRange(imageHSV, h_min, h_max, threshold);

    cv::Mat st1 = cv::getStructuringElement(cv::MORPH_RECT , cv::Size(80, 80));
    cv::Mat st2 = cv::getStructuringElement(cv::MORPH_RECT , cv::Size(100, 100));
    cv::morphologyEx(threshold, morphology, cv::MORPH_OPEN, st1);
    cv::morphologyEx(threshold, morphology, cv::MORPH_CLOSE, st2);

    cv::GaussianBlur(morphology, morphology, cv::Size(3, 3), 1);

#if config_findContours
    cv::findContours(morphology, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    for (size_t idx = 0; idx < contours.size(); idx++)
    {
        minRect = cv::minAreaRect(contours[idx]);
        minRect.points(vtx);

        if (cv::contourArea(contours[idx], false) > 1000)
        {
            cv::circle(imgOriginal, minRect.center, 10, cv::Scalar(0, 0, 255), -1);
            cv::putText(imgOriginal, cv::format("x: %d", static_cast<int>(minRect.center.x)), cv::Point(150, 20), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1, 1, false);
            cv::putText(imgOriginal, cv::format("y: %d", static_cast<int>(minRect.center.y)), cv::Point(300, 20), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1, 1, false);
            cv::putText(imgOriginal, cv::format("angle: %0.1f", static_cast<double>(minRect.angle)), cv::Point(10, 20), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 0), 1, 1, false);
            y = static_cast<int>(minRect.center.y);
            step = DegreeToStep(SIXTEENTH_STEP, static_cast<double>(minRect.angle));
            angle = qFloor((4.4305 + (static_cast<double>(minRect.size.width *  0.2636))) / 0.7452);

            data_pos[1] = 0x01;
            if(y > y_temp)
            {
                y_send = y - y_temp;
                data_pos[2] = 0x55;
            }
            else if(y < y_temp)
            {
                y_send = y_temp - y;
                data_pos[2] = 0x52;
            }
            data_pos[3] = 0x53;
            data_pos[4] = static_cast<uint8_t>(y_send >> 8);
            data_pos[5] = y_send & 0xFF;
            y_temp = y;

            for(int i = 0; i < 12; i++)
            {
                pop.data[i] = data_pos[i];
            }
            dfd.push_back(pop);

            data_pos[1] = 0x02;
            if(step > step_temp)
            {
                step_send = step - step_temp;
                data_pos[2] = 0x55;
            }
            else if(step < step_temp)
            {
                step_send = step_temp - step;
                data_pos[2] = 0x52;
            }
            data_pos[3] = 0x53;
            data_pos[4] = static_cast<uint8_t>(step_send >> 8);
            data_pos[5] = step_send & 0xFF;
            step_temp = step;

            for(int i = 0; i < 12; i++)
            {
                pop.data[i] = data_pos[i];
            }
            dfd.push_back(pop);
        }
    }
#endif

#if config_moments
    cv::Moments moments = cv::moments(threshold, true);
    double dm01 = moments.m01;
    double dm10 = moments.m10;
    double darea = moments.m00;

    if (darea > 100)
    {
        x = int(dm10 / darea);
        y = int(dm01 / darea);
        cv::circle(imgOriginal, cv::Point (x, y), 10, cv::Scalar(0, 0, 255), -1);
        cv::putText(imgOriginal, cv::format("%d-%d", x, y), cv::Point(x + 10, y - 10), cv::FONT_HERSHEY_SCRIPT_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1, 1, false);
        //fout << x << "\t" << y << "\t" << std::endl;

        if(y > y_last)
        {
            serialport->putChar(static_cast<char>(0xF8));
            serialport->putChar(0x01);
            serialport->putChar(0x55);
            serialport->putChar(0x53);

            serialport->putChar(static_cast<char>(y >> 8));
            serialport->putChar(static_cast<char>(y & 0xFF));

            serialport->putChar(0x00);
            serialport->putChar(0x01);
            serialport->putChar(static_cast<char>(0xE0));

//            fout << y - y_last << "\t" << "Left" << std::endl;

            y_last = y;
        }
        else if(y < y_last)
        {
            serialport->putChar(static_cast<char>(0xF8));
            serialport->putChar(0x01);
            serialport->putChar(0x52);
            serialport->putChar(0x53);

            serialport->putChar(static_cast<char>(y >> 8));
            serialport->putChar(static_cast<char>(y & 0xFF));

            serialport->putChar(0x00);
            serialport->putChar(0x01);
            serialport->putChar(static_cast<char>(0xE0));

//            fout << y_last - y << "\t" << "Right" << std::endl;

            y_last = y;
        }
    }
#endif

    QImage qimgOriginal = convertOpenCVMatToQtQImage(imgOriginal);
    ui->lblOriginal->setPixmap(QPixmap::fromImage(qimgOriginal));
}

void frmmain::processSendUART()
{
    if(!dfd.empty() && ok_receive == true)
    {
        send_packet(dfd[0].data);
        memcpy(data_last, dfd[0].data, 12);
        dfd.erase(dfd.begin());
        ok_receive = false;
        qDebug() << "OK Packet 1";

        send_packet(dfd[1].data);
        memcpy(data_last, dfd[1].data, 12);
        dfd.erase(dfd.begin());
        ok_receive = false;
        qDebug() << "OK Packet 2";
    }
    if(error_receive == true)
    {
        qDebug() << "ERROR Packet";
        send_packet(data_last);
        ok_receive = true;
        error_receive = false;
    }

    /*if(!dfd.empty() && ok_receive == true)
    {
        send_packet(dfd[1].data);
        memcpy(data_last, dfd[1].data, 12);
        dfd.erase(dfd.begin());
        ok_receive = false;
        qDebug() << "OK Packet 2";
    }
    if(error_receive == true)
    {
        qDebug() << "ERROR Packet 2";
        send_packet(data_last);
        ok_receive = true;
        error_receive = false;
    }*/

}

void frmmain::UART_Receive()
{
    dataUART = serialport->readAll();

    switch (dataUART.at(0))
    {
    /*case 0x27:
        qDebug() << "Motor 1 has finished";
        ok_motor1 = true;
        break;
    case 0x28:
        qDebug() << "Motor 2 has finished";
        ok_motor2 = true;
        break;
    case 0x31:
        qDebug() << "Motor 3 has finished";
        ok_motor3 = true;
        break;
    case 0x29:
        qDebug() << "Gripper has finished";
        ok_gripper = true;
        break;*/
    case 0x0A:
        error_receive = true;
        break;
    case 0x1D:
        ok_receive = true;
        break;
    default:
        break;
    }
}

void frmmain::exitProgram()
{
    if(qtimer->isActive()) qtimer->stop();
    fout.close();
    QApplication::quit();
}

QImage frmmain::convertOpenCVMatToQtQImage(cv::Mat mat)
{
    if(mat.channels() == 1) {                                   // if 1 channel (grayscale or black and white) image
            return QImage(static_cast<uchar*>(mat.data), mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_Indexed8);     // return QImage
        } else if(mat.channels() == 3) {                            // if 3 channel color image
            cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);                     // flip colors
            return QImage(static_cast<uchar*>(mat.data), mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_RGB888);       // return QImage
        } else {
            qDebug() << "in convertOpenCVMatToQtQImage, image was not 1 channel or 3 channel, should never get here";
        }
        return QImage();        // return a blank QImage if the above did not work
}

uint16_t frmmain::DegreeToStep(frmmain::MotorMode mode, double degree)
{
    return static_cast<uint16_t>((degree / 1.8) * mode * 2.5);
}

uint8_t *frmmain::reorder4(uint8_t *src, uint32_t len)
{
    static uint8_t dst[4];
    uint8_t appendlen, idx;

    len = (len % 4) + 4;
    appendlen = (len % 4) ? 4 - (len % 4) : 0;
    idx = 0;
    while(appendlen--)
    {
        dst[idx] = 0xFF;
        idx++;
    }
    while(len--)
    {
        dst[idx] = src[3 - idx];
        idx++;
    }
    return dst;
}

uint32_t frmmain::crc32_formula_normal_STM32(size_t len, void *data)
{
#define POLY 0x04C11DB7
    uint8_t *buffer = (uint8_t*)data;
    uint32_t crc = -1;
    uint32_t portion;
    uint8_t *reordered;

    while( len )
    {
        portion = len < 4 ? len : 4;
        reordered = reorder4(buffer, portion);
        for (uint8_t i = 0; i < 4; i++)
        {
            crc = crc ^ ((uint32_t)reordered[i] << 24);
            for( int bit = 0; bit < 8; bit++ )
            {
                if( crc & (1L << 31)) crc = (crc << 1) ^ POLY;
                else                  crc = (crc << 1);
            }
        }
        buffer += portion;
        len -= portion;
    }
    return crc;
}

void frmmain::write_serialPort(uint32_t *data)
{
    QByteArray ba;
    ba.resize(4);
    ba[0] = (static_cast<char>(*data >> 24));
    ba[1] = (static_cast<char>(*data >> 16));
    ba[2] = (static_cast<char>(*data >> 8));
    ba[3] = (static_cast<char>(*data));
    serialport->write(ba);
}

void frmmain::send_packet(uint8_t *data)
{
    if(data[0] == 0xF8 && data[8] == 0xE0)
    {
        for(uint8_t i = 0; i < 12; i++)
            serialport->putChar(static_cast<char>(data[i]));

        uint32_t crc32Result2 = crc32_formula_normal_STM32(9, data);
        write_serialPort(&crc32Result2);
    }
}

void frmmain::on_btnStart_clicked()
{
    int16_t current_value = 0;
    uint16_t unsigned_value = 0;
    uint16_t pos_motor1 = 2000;

    current_value = static_cast<int16_t>(step);

    if(serialport->isWritable() == true)
    {
        for(int i = 0; i < 3; i++)
        {
            if(i == 0)
            {
                serialport->putChar(static_cast<char>(0xF8));                               //START BIT
                serialport->putChar(0x01);                                                  //Driver 1
                serialport->putChar(0x55);                                                  //Up
                serialport->putChar(0x53);                                                  //SIXTEENTH STEP

                serialport->putChar(static_cast<char>(pos_motor1 >> 8));                    //STEP
                serialport->putChar(static_cast<char>(pos_motor1 & 0xFF));                  //STEP

                serialport->putChar(0x00);
                serialport->putChar(0x01);
                serialport->putChar(static_cast<char>(0xE0));                               //STOP BIT
            }
            else if(i == 1)
            {
                if (current_value > 0)
                {
                    serialport->putChar(static_cast<char>(0xF8));                           //START BIT
                    serialport->putChar(0x02);                                              //Driver 2
                    serialport->putChar(0x55);                                              //Left
                    serialport->putChar(0x53);                                              //SIXTEENTH STEP

                    serialport->putChar(static_cast<char>(current_value >> 8));             //STEP
                    serialport->putChar(static_cast<char>(current_value & 0xFF));           //STEP

                    serialport->putChar(0x00);
                    serialport->putChar(0x01);
                    serialport->putChar(static_cast<char>(0xE0));                           //STOP BIT
                }
                else if (current_value < 0)
                {
                    unsigned_value = 0xFFFF - (static_cast<uint16_t>(current_value) - 1);

                    if(serialport->isWritable() == true)
                    {
                        serialport->putChar(static_cast<char>(0xF8));                       //START BIT
                        serialport->putChar(0x02);                                          //Driver 2
                        serialport->putChar(0x52);                                          //Right
                        serialport->putChar(0x53);                                          //SIXTEENTH STEP
                        serialport->putChar(static_cast<char>(unsigned_value >> 8));        //STEP
                        serialport->putChar(static_cast<char>(unsigned_value & 0xFF));      //STEP
                        serialport->putChar(0x00);
                        serialport->putChar(0x01);
                        serialport->putChar(static_cast<char>(0xE0));                       //STOP BIT
                    }
                }
            }
            else if(i == 2)
            {
                serialport->putChar(static_cast<char>(0xF9));                               //START BIT
                serialport->putChar(static_cast<char>(angle));                              //ANGLE
                serialport->putChar(0x00);
                serialport->putChar(0x00);
                serialport->putChar(0x00);
                serialport->putChar(0x00);
                serialport->putChar(0x00);
                serialport->putChar(0x00);
                serialport->putChar(static_cast<char>(0xE0));                               //STOP BIT
            }

            //delay(1);
        }
    }

    /*if (current_value > 0)
    {
        if(serialport->isWritable() == true)
        {
            //fout << current_value << "\t" << "direct CW\t";

            serialport->putChar(static_cast<char>(0xF8));                   //START BIT
            serialport->putChar(0x02);                                      //Driver 2
            serialport->putChar(0x52);                                      //Down
            serialport->putChar(0x53);                                      //SIXTEENTH STEP

            serialport->putChar(static_cast<char>(current_value >> 8));     //STEP
            serialport->putChar(static_cast<char>(current_value & 0xFF));   //STEP

            serialport->putChar(0x00);
            serialport->putChar(0x01);
            serialport->putChar(static_cast<char>(0xE0));                   //STOP BIT

            serialport->putChar(static_cast<char>(0xF8));                   //START BIT
            serialport->putChar(0x03);                                      //Driver 3
            serialport->putChar(0x52);                                      //Down
            serialport->putChar(0x53);                                      //SIXTEENTH STEP

            serialport->putChar(static_cast<char>(current_value >> 8));     //STEP
            serialport->putChar(static_cast<char>(current_value & 0xFF));   //STEP

            serialport->putChar(0x00);
            serialport->putChar(0x01);
            serialport->putChar(static_cast<char>(0xE0));                   //STOP BIT
        }
    }
    else
    {
        unsigned_value = 0xFFFF - (static_cast<uint16_t>(current_value) - 1);

        //fout << unsigned_value << "\t" << "direct CCW\t";

        if(serialport->isWritable() == true)
        {
            serialport->putChar(static_cast<char>(0xF8));                   //START BIT
            serialport->putChar(0x02);                                      //Driver 2
            serialport->putChar(0x55);                                      //Down
            serialport->putChar(0x53);                                      //SIXTEENTH STEP
            serialport->putChar(static_cast<char>(unsigned_value >> 8));    //STEP
            serialport->putChar(static_cast<char>(unsigned_value & 0xFF));  //STEP
            serialport->putChar(0x00);
            serialport->putChar(0x01);
            serialport->putChar(static_cast<char>(0xE0));                   //STOP BIT
        }
    }*/

    //fout << angle << std::endl;

    /*serialport->putChar(static_cast<char>(0xF9));                               //START BIT
    serialport->putChar(static_cast<char>(angle >> 8));                         //ANGLE
    serialport->putChar(0x00);
    serialport->putChar(0x00);
    serialport->putChar(0x00);
    serialport->putChar(0x00);
    serialport->putChar(0x00);
    serialport->putChar(0x00);
    serialport->putChar(static_cast<char>(0xE0));                               //STOP BIT
    */
}
