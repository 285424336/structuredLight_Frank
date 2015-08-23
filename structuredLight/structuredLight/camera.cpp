#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <direct.h>
#include <time.h>

#include "structuredlight.h"
#include "camera.h"


//#define USE_MV_UB500
#define USE_UI_2220SE

using namespace std;
using namespace cv;

#ifdef USE_MV_UB500	//use mv-ub500 sdk api function
#include <process.h>
#include "windows.h"
#pragma comment(lib,"..\\structuredLight\\MVCAMSDK.lib")
#include "CameraApi.h"

CameraSdkStatus camera_sdk_status;	//���״̬
INT  CameraNums = 1;				//����iCameraNums = 1����ʾ���ֻ��ȡ1���豸������ö�ٸ�����豸������sCameraList��С��iCameraNums��ֵ
tSdkCameraDevInfo CameraList[1];	//�豸�б�����ָ��
CameraHandle    m_hCamera;			//��������������ͬʱʹ��ʱ���������������	
tSdkCameraCapbility CameraInfo;		//������������ṹ��
tSdkFrameHead 	FrameInfo;			//ͼ���֡ͷ��Ϣָ��
BYTE*			PbyBuffer;			//ָ��ԭʼͼ�����ݵĻ�����ָ��
BYTE*           FrameBuffer;		//��ԭʼͼ������ת��ΪRGBͼ��Ļ�����
IplImage *iplImage = NULL;


#elif defined(USE_UI_2220SE)	//use UI-2220SE camera sdk api function
#include "uEye.h"
HIDS hCam = 1;		//camera handle
SENSORINFO SensorInfo;
CAMINFO CameraInfo;
INT colorMode = IS_CM_BGR8_PACKED;
UINT pixelClock = 30; //MHz
double frameRate = 52;  //fps
double exposureTime = 8.0; //ms
INT  masterGain = 0;  //0-100
INT redGain = 5, greenGain = 0, blueGain = 60; //0-100
INT gamma = 160; //multipe by 100
INT triggerMode = IS_SET_TRIGGER_SOFTWARE; // IS_SET_TRIGGER_LO_HI;
INT triggerDelay = 0;
INT flashMode = IO_FLASH_MODE_TRIGGER_LO_ACTIVE;	//pay attention to that real output is inverse
char *imageAddress = NULL;
INT memoryId = 0;
#endif


//Initialize camera
int CameraInitialize(SlParameter &sl_parameter)
{
//	cv::Mat frame_grab;
	//use usb webcam
//	VideoCapture videocapture(sl_parameter.camera_id);
//	if (!videocapture.isOpened())
//	{
//		cerr << "Failed to open camera" << endl;
//		return -1;
//	}
//	cout << "Initialize camera------------------------------------------" << endl;
//	//set properties of the camera
//	videocapture.set(CV_CAP_PROP_FRAME_WIDTH, sl_parameter.camera_width);
//	videocapture.set(CV_CAP_PROP_FRAME_HEIGHT, sl_parameter.camera_height);
//
//	namedWindow("Video in real-time", WINDOW_NORMAL);
//	while (1)
//	{
//		videocapture >> frame_grab;
//		imshow("Video in real-time", frame_grab);
//		waitKey(50);
//#ifdef DEBUG_PROJECT
//		cout << "Camera Properties:" << endl;
//		cout << "camera id: " << sl_parameter.camera_id << endl;
//		cout << "frame rate: " << videocapture.get(CV_CAP_PROP_FPS) << endl;
//		cout << "width: " << videocapture.get(CV_CAP_PROP_FRAME_WIDTH) << endl;
//		cout << "height: " << videocapture.get(CV_CAP_PROP_FRAME_HEIGHT) << endl;
//		cout << "brightness: " << videocapture.get(CV_CAP_PROP_BRIGHTNESS) << endl;
//		cout << "contrast: " << videocapture.get(CV_CAP_PROP_CONTRAST) << endl;
//		cout << "saturation: " << videocapture.get(CV_CAP_PROP_SATURATION) << endl;
//		cout << "hue: " << videocapture.get(CV_CAP_PROP_HUE) << endl;
//		cout << "gain: " << videocapture.get(CV_CAP_PROP_GAIN) << endl;
//		cout << "exposure: " << videocapture.get(CV_CAP_PROP_EXPOSURE) << endl;
//#endif
//		cout << "-------------------------------------------------------" << endl << endl;
//	}

#ifdef USE_MV_UB500		//use mv-ub500 sdk api function
	//use industry camera mv ub500
	Mat frame_grab;
	//���SDK��ʼ��
	if ((camera_sdk_status= CameraSdkInit(1)) != CAMERA_STATUS_SUCCESS)
	{
		cout << "Camera sdk init failed: " << camera_sdk_status<<endl;
		return -1;
	}
	//ö���豸������豸�б�
	if ((camera_sdk_status=CameraEnumerateDevice(CameraList, &CameraNums)) != CAMERA_STATUS_SUCCESS || CameraNums == 0)
	{
		cout << "No camera was found: " << camera_sdk_status << endl;
		return -1;
	}
	//��ʼ���豸
	if ((camera_sdk_status  = CameraInit(&CameraList[0], -1, -1, &m_hCamera)) != CAMERA_STATUS_SUCCESS)
	{
		cout << "Camera  init failed: " << camera_sdk_status << endl;
		return -1;
	}
	//��ʼ���������������
	CameraGetCapability(m_hCamera, &CameraInfo);
	FrameBuffer = (BYTE *)malloc(CameraInfo.sResolutionRange.iWidthMax*CameraInfo.sResolutionRange.iWidthMax * 3);
	//����ͼ��ɼ�ģʽ
	CameraPlay(m_hCamera);
	waitKey(2000);			//wait for camera start
	//for (int i = 0; i < 5; i++)
	//{
	//	GetImage(frame_grab);
	//	if (!frame_grab.empty())
	//		imshow("camera initialize", frame_grab);
	//	waitKey(200);
	//}
	GetImage(frame_grab);
	if (!frame_grab.empty())
		cout << "Camera initialize successful......" <<endl<<endl;
	else
		cout << "Camera initializa failed......" << endl<<endl;
//	destroyWindow("camera initialize");
	return 0;

#elif defined(USE_UI_2220SE)	//use UI-2220SE camera sdk api function
	
	INT nRet = is_InitCamera(&hCam, NULL);
	if (nRet == IS_SUCCESS){
		cout << "camera UI_2220SE init success!" << endl;

		//query and display information about sensor and camera
		is_GetSensorInfo(hCam, &SensorInfo);
		is_GetCameraInfo(hCam, &CameraInfo);
		
		nRet = is_SetColorMode(hCam, colorMode);
		if (nRet != IS_SUCCESS){
			cout << "set color mode at" << colorMode << "failed;error code:" << nRet << endl;
		}

		//setting parameters,closing all automatic function firstly
		double auto_parameter1 = 0, auto_parameter2 = 0;

		nRet=is_PixelClock(hCam, IS_PIXELCLOCK_CMD_SET, (void*)&pixelClock, sizeof(pixelClock));
		if (nRet != IS_SUCCESS){
			cout << "set pixelclock at " << pixelClock << "MHz failed;error code:" << nRet << endl;
			return 0;
		}

		is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_SENSOR_FRAMERATE, &auto_parameter1, &auto_parameter2);
		nRet=is_SetFrameRate(hCam, frameRate, &frameRate);
		if (nRet != IS_SUCCESS){
			cout << "set framerate at " << frameRate << "fps failed;error code:" << nRet << endl;
			return 0;
		}

		is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_SENSOR_GAIN_SHUTTER, &auto_parameter1, &auto_parameter2);
		nRet = is_Exposure(hCam, IS_EXPOSURE_CMD_SET_EXPOSURE, &exposureTime, 8);
		if (nRet != IS_SUCCESS){
			cout << "set exposuretime at " << exposureTime << "ms failed;error code:" << nRet << endl;
			return 0;
		}

		is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_SENSOR_GAIN, &auto_parameter1, &auto_parameter2);
		is_SetGainBoost(hCam, IS_SET_GAINBOOST_ON);
		nRet=is_SetHardwareGain(hCam, masterGain, redGain, greenGain, blueGain);
		if (nRet != IS_SUCCESS){
			cout << "set hardware gain failed;error code:" << nRet << endl;
			return 0;
		}

		is_SetHardwareGamma(hCam, IS_SET_HW_GAMMA_ON);
		nRet = is_Gamma(hCam, IS_GAMMA_CMD_SET, &gamma, sizeof(gamma));
		if (nRet != IS_SUCCESS){
			cout << "set hardware gamma failed;error code:" << nRet << endl;
			return 0;
		}

//		is_SetTriggerDelay(hCam, triggerDelay);
		nRet = is_SetExternalTrigger(hCam, triggerMode);
		if (nRet != IS_SUCCESS){
			cout << "set external trigger at " << triggerMode << "failed;error code:" << nRet << endl;
			return 0;
		}

		//must set trigger firstly
		IO_FLASH_PARAMS flashParams;
		flashParams.s32Delay = 0;	   //delay after explosure start(us)
		flashParams.u32Duration = 0;   //flash voltage level duration(us),0=explosure time
		is_IO(hCam, IS_IO_CMD_FLASH_SET_MODE, (void*)&flashMode, sizeof(flashMode));
		nRet = is_IO(hCam, IS_IO_CMD_FLASH_SET_PARAMS, (void*)&flashParams, sizeof(flashParams));
		if (nRet != IS_SUCCESS){
			cout << "set flash mode failed;error code:" << nRet << endl;
			return 0;
		}

		nRet=is_AllocImageMem(hCam, sl_parameter.camera_width, sl_parameter.camera_height, 24, &imageAddress, &memoryId);
		if (nRet == IS_SUCCESS){
			nRet = is_SetImageMem(hCam, imageAddress, memoryId);
			if (nRet != IS_SUCCESS){
				cout << "allocate memory failed;error code:" << nRet << endl;
				return 0;
			}
		}

		return -1;
	}
	else{
		cout << "Init camera UI-2220SE failed;error code:" << nRet<< endl;
		return 0;
	}
#endif
	
}

int GetImage(Mat &frame_grab)
{
#ifdef USE_MV_UB500		//use mv-ub500 sdk api function
	clock_t clock_begin;
	clock_begin = clock();
	//ע��������������һ�δ�������ָ����ͼ��
	if (CameraGetImageBuffer(m_hCamera, &FrameInfo, &PbyBuffer, 200) == CAMERA_STATUS_SUCCESS)
	{
		////����õ�ԭʼ����ת����RGB��ʽ�����ݣ�ͬʱ����ISPģ�飬��ͼ����н��룬������������ɫУ���ȴ���
		camera_sdk_status = CameraImageProcess(m_hCamera, PbyBuffer, FrameBuffer, &FrameInfo);//����ģʽ
		if (camera_sdk_status == CAMERA_STATUS_SUCCESS)
		{
			//ת�����ݲ���ʾ
			iplImage = cvCreateImageHeader(cvSize(FrameInfo.iWidth, FrameInfo.iHeight), IPL_DEPTH_8U, 3);
			cvSetData(iplImage, FrameBuffer, FrameInfo.iWidth * 3);
			//cvShowImage("camera", iplImage);
			Mat frame_temp(iplImage, true);
			frame_grab = frame_temp.clone();
		}
		//�ڳɹ�����CameraGetImageBuffer�󣬱������CameraReleaseImageBuffer���ͷŻ�õ�buffer��
		CameraReleaseImageBuffer(m_hCamera, PbyBuffer);
		cvReleaseImageHeader(&iplImage);
		cout << clock() - clock_begin << " ";
		return -1;
	}
	else
		return 0;
#elif defined(USE_UI_2220SE) 	//use UI-2220SE camera sdk api function
	clock_t clock_begin;
	clock_begin = clock();
	is_FreezeVideo(hCam, IS_WAIT);
	cout << clock() - clock_begin << " ";

	frame_grab.data = (unsigned char *)imageAddress;
	return 0;
#endif

}

void CameraClear(void)
{
#ifdef USE_MV_UB500		//use mv-ub500 sdk api function
	CameraUnInit(m_hCamera);

#elif defined(USE_UI_2220SE)	//use UI-2220SE camera sdk api function
	is_ExitCamera (hCam);
#endif
}








