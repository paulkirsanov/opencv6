#ifndef FRMMAIN_H
#define FRMMAIN_H

#define config_moments 0
#define config_findContours 1
#define config_send2mcu     1

#include <QMainWindow>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QtSerialPort/QSerialPort>
#include <fstream>

#include <QtMath>
#include <QTimer>

#include <vector>

namespace Ui {
class frmmain;
}

class frmmain : public QMainWindow
{
    Q_OBJECT

public:
    explicit frmmain(QWidget *parent = nullptr);
    ~frmmain();

    typedef enum
    {
        FULL_STEP = 1,
        HALF_STEP = 2,
        QUARTER_STEP = 4,
        EIGHTH_STEP = 8,
        SIXTEENTH_STEP = 16
    } MotorMode;

    bool ok_motor1 = true;
    bool ok_motor2 = true;
    bool ok_motor3 = false;
    bool ok_gripper = false;
    bool ok_receive = true;
    bool error_receive = false;

public slots:
    void processFrameAndUpdateGUI();
    void processSendUART();
    void UART_Receive();

private slots:
    void on_btnStart_clicked();

private:
    Ui::frmmain *ui;
    QSerialPort* serialport;
    QTimer* qtimer;
    QTimer* qtimer1;

    bool value = false;
    uint16_t old_step = 0;
    int old_angle = 0;
    bool flag = false;
    cv::RotatedRect minRect;
    int step = 0;
    int step_send = 0, step_temp = 0;
    int angle = 0;
    cv::Point2f vtx[4];
    int x = 0, y = 0;
    int y_send = 0, y_temp = 0;
    int current_value = 1;

    cv::VideoCapture cap;
    cv::Mat imgOriginal, src;

    int g_switch_value_h1 = 165;
    int g_switch_value_s1 = 120;
    int g_switch_value_v1 = 140;
    int g_switch_value_h2 = 170;
    int g_switch_value_s2 = 207;
    int g_switch_value_v2 = 220;

    std::ofstream fout;

    QImage convertOpenCVMatToQtQImage(cv::Mat mat);
    void exitProgram();
    uint16_t DegreeToStep(MotorMode mode, double degree);

    uint8_t data_pos[12] = {0};
    uint8_t data_last[12] = {0};

    uint8_t * reorder4(uint8_t *src, uint32_t len);
    uint32_t crc32_formula_normal_STM32(size_t len, void *data);
    void write_serialPort(uint32_t *data);
    void send_packet(uint8_t *data);

    QByteArray dataUART;

    struct f_data
    {
        uint8_t data[12];
    }pop;

    std::vector<f_data> dfd;

};

#endif // FRMMAIN_H
