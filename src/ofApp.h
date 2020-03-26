//VERSION 1.2
#pragma once

#include "ofMain.h"
#include "PotentiometerController.h"
#include "ButtonController.h"
#include "ClickCounter.h"
#include "wiringPi.h"
#include "wiringPiSPI.h"
#include "ofxOMXPlayer.h"
#include <ofxNanoKontrolTwo.h>


typedef std::vector<std::string> stringvec;

struct controlsValues {
	float value1;
	float value2;
	float value3;
	float value4;
};
struct joystickValues {
	float js1x;
	float js1y;
	float js2x;
	float js2y;
};

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		
		void getVideos(const std::string& path,stringvec& ext, stringvec& vector);
		void getImages(const std::string& path,stringvec& ext, stringvec& vector);
	
		void drawSource();
		void nextVideo();
		void nextImage();
		void previousVideo();
		void previousImage();
		void powerOffCheckRoutine();
		void checkClicksRoutine();
		void checkJoysticksRoutine();
		void updateControlsRoutine();
		void checkVideoPlayback();

		void checkClicks();
		void checkJoysticks(joystickValues values);
		void updateControls(controlsValues values);

		// left clicks
		void centerFrameBuffer();
		void centerFrameBufferCallback(bool & v);
		void toggleSourceParamMode();
		void toggleSourceParamModeCallback(bool & v);
		void nextSource();
		void nextSourceCallback(bool & v);
		void previousSource();
		void previousSourceCallback(bool & v);

		// right clicks
		void clearFeedback();
		void clearFeedbackCallback(bool & v);
		void changeFeedbackMode();
		void changeFeedbackModeCallback(bool & v);
		void togglePaintMode();
		void togglePaintModeCallback(bool & v);
		void toggleCam();
		void toggleCamCallback(bool & v);

		controlsValues ctrlValues;
		joystickValues jsValues;
		
 		ofxNanoKontrolTwo nanoKontrol;
	
		ClickCounter clickCounter1, clickCounter2;
		PotentiometerController pot1, pot2, pot3, pot4, js1x, js1y, js2x, js2y;
		ButtonController button1, button2;
		ofFbo buffer, buffer2, buffer3;
		ofShader shader1,shader2, shader3, shader4, lumakey,shader6;
		ofVideoGrabber cam;
		ofxOMXPlayer player;
		ofImage image1,paintImage;
		ofTexture texture1;

		bool isReady;
		bool noInput;
		bool camOn;
		bool notLoading;
		bool paintMode;
		bool sourceParamMode;
		bool sourceRotateWaitRecall, sourceZoomWaitRecall, feedbackRotateWaitRecall, feedbackZoomWaitRecall;
		int selector;
		int num,imageNum;
		uint64_t loadCount;
		int nClicks1, nClicks2;
		bool turnOffStarted;
		uint64_t beginTime;
		int zoom;
		int paintZoom, oldPaintZoom;
		int sourceZoom, oldSourceZoom;
		float rotate;
		int paintRotate, oldPaintRotate;
		int sourceRotate, oldSourceRotate;
		float xAxis1, yAxis1, xAxis2, yAxis2, knob, keyTresh, dispX, dispY,paintDispX, paintDispY, sourceDispX, sourceDispY;
		


};
