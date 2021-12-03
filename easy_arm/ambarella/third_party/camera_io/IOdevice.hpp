#pragma once

//opencv
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

// standard
#include <memory>


/**
  Camera Communication protocol
*/
class CameraDevicePImpl;
class CameraDevice{
public:
    enum CameraID{
        NONE = 0,
        I05C,
        I05M,
        I08C,
        R01TH
    };

    enum CameraImSize{

        autoSize = 0,

        I05C_1920x1080 = 100,
        I05C_2432x2048,

        I05M_1920x1080 = 200,
        I05M_2432x2048,

        I08C_1280x720 = 300,
        I08C_1920x1080,
        I08C_2048x1536,
        I08C_2560x1440,
        I08C_2592x1944,
        I08C_2688x1520,
        I08C_3072x2048,
        I08C_3840x2160,

        R01TH_1280x800 = 400
    };



    CameraDevice();
    ~CameraDevice();
    /**
     * @brief setImageSize
     * Select the image size (must be done before loadCamera)
     */
    void setImageSize(CameraImSize);
    /**
     * @brief setGainExposure
     * Set gain and exposure value (must be done before loadCamera);
      If gain value is negative, the gain exposure is set
      automatically.
     * @param gain gain value
     * @param exposure exposure value
     */
    void setGainExposure(int gain, int exposure = -1);
    /**
     * @brief setToFConfidence
     * Set the ToF confidence value (must be done before loadCamera)
     * @param confidence
     */
    void setToFConfidence(int confidence);
    /**
     * @brief setToFFlyingPixelThreshold
     * Set the Flying pixel threshold for ToF camera (must be done before loadCamera)
     * @param flying_pixel
     */
    void setToFFlyingPixelThreshold(double flying_pixel);
    /**
     * @brief setTemporalFilter
     * Enable/disable the temporal filter for the ToF (must be done before loadCamera)
     */
    void setTemporalFilter(bool);
    /**
     * @brief loadCamera
     * Apply all the previous settings and load the camera
     * @param camID ID of the camera
     * @param ImSize ID of the image size format
     * @return
     */
    int loadCamera(CameraID camID = CameraID::NONE,CameraImSize ImSize = CameraImSize::autoSize);
    /**
     * @brief grabImage
     * Grab a 3channels 8bits images matrix for color camera and a
     * Single channels 8bits image matrix for monochrome.
      If the camera is monochrome, then each channel is equal
     * @return the image grabbed
     */
    cv::Mat grabImage();
    /**
     * @brief grabTof
     * Grab a depth matrix from tof sensor
     * @return the image grabbed
     */
    cv::Mat grabTof();
private:
    std::unique_ptr<CameraDevicePImpl> pimpl;
};


/**
  TOF Process
*/
class TOFProcessPImpl;
class TOFProcess{
public:
    TOFProcess();
    ~TOFProcess();
    /**
     * @brief loadCalib
     * load the calibration file
     * @param file calibration file full path
     * @return negative value if failed
     */
    int loadCalib(std::string file);
    /**
     * @brief setCam
     * set the camera
     * @param imSize image size
     * @param fx focal along x
     * @param fy focal along y
     * @param cx principal point x
     * @param cy principal point y
     * @param dist distortion (array of 5 double)
     * @return negative value if failed
     */
    int setCam(cv::Size imSize,
               double fx, double fy,
               double cx, double cy,
               double *dist);
    /**
     * @brief getCamMatrix
     * @return the current cam matrix loaded [3x3] double
     */
    cv::Mat getCamMatrix();
    /**
     * @brief getDist
     * @return the distorsion param [1x5] double
     */
    cv::Mat getDist();
    /**
     * @brief TOFToImage
     * convert deptm matrix to 3channel colored RGB image
     * @param im depth matrix
     * @param backToBlack all background point is set to black color
     * @return the image matrix. if fail, the image is empty.
     */
    cv::Mat TOFToImage(cv::Mat &im, bool backToBlack);
    /**
     * @brief TOFTo3DCloud
     * convert depth image matrix to 3Dpoint cloud
     * @param im depth image matrix
     * @param zeroCancel do not convert point if depth is 0
     * @return 3XN matrix of float
     */
    cv::Mat TOFTo3DCloud(cv::Mat &im,bool zeroCancel);
    /**
     * @brief TOFTo3DCloud
     * convert specific point coordinate to 3Dpoint cloud using depth image
     * @param im depth image matrix
     * @param pts [2xN] matrix of image coordinates(int) to convert
     * @return  3XN matrix of float
     */
    cv::Mat TOFTo3DCloud(cv::Mat &im,cv::Mat &pts);

public:
    std::unique_ptr<TOFProcessPImpl> pimpl;
};


/**
  GPIO Communication protocol
*/
class GPIODevicePImpl;
class GPIODevice{
public:
    GPIODevice();
    ~GPIODevice();
    /**
     * @brief gpioInit
     *
     * @return negative value if failed
     */
    int gpioInit();
    /**
     * @brief gpioInput
     * @return negative value if failed,
     *         0 for low level
     *         positive value for high level
     */
    int gpioInput();
    /**
     * @brief gpioOutput
     * @param GPIO_port: port to write (PORT_0 or PORT_1)
     * @param value: 0 is low level and 1 is high level
     * @param delay: delay before setting high level
     * @param duration: duration of the high level
     * @return negative value if failed
     */
    enum GPIOOutputPort{PORT_0=0, PORT_1};
    int gpioOutput(GPIOOutputPort GPIO_port, int value, int delay,int duration);
private:
    std::unique_ptr<GPIODevicePImpl> pimpl;
};


/**
  RS232 Communication protocol
*/
class RS232DevicePImpl;
class RS232Device{
public:
    RS232Device();
    ~RS232Device();
    /**
     * @brief rs232Init
     * Initialized RS232 communication protocol
     * @param bdrate Baud rate value
     * @return negative value if failed
     */
    int rs232Init(int bdrate);
    /**
     * @brief rs232Send
     * Send a string on the RS232 bus
     * @param str string to send
     * @return negative value if failed
     *         positive value: number of byte sent
     */
    int rs232Send(std::string str);
    /**
     * @brief rs232Receive
     * Receive a string from the RS232 bus
     * @param str the output string to filled
     * @return negative value if failed
     *         positive value: number of byte received
     *         Notice that the internal buffer size is 512 bytes
     */
    int rs232Receive(std::string &str);
private:
    std::unique_ptr<RS232DevicePImpl> pimpl;
};

/**
  HDMI Display Communication protocol
*/
class HDMIDevicePImpl;
class HDMIDevice{
public:
    enum DisplayOption{
        NO_SCALED=0,
        AUTO_SCALE,
        FULLSCREEN
    };
    HDMIDevice();
    ~HDMIDevice();
    /**
     * @brief HDMIInit
     * Initialize the HDMI display
     * @return negative value if failed
     */
    int HDMIInit();
    /**
     * @brief HDMIShow
     * display the image on the HDMI port
     * @param im: image to display (CV8UC1 or CV_8UC3)
     * @param option:
     *      NO_SCALED: (the image is crop if necessary to the hdmi display buffer (1280x720))
     *      AUTO_SCALE: the ratio is concerne and the image is resized to fit with the hdmi display buffer
     *      FULLSCREEN: the image is resize at the hdmi display size
     * @return negative value if failed
     */
    int HDMIShow(cv::Mat &im, DisplayOption option = AUTO_SCALE);
private:
    std::unique_ptr<HDMIDevicePImpl> pimpl;
};
