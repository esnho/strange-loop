//VERSION 1.2
#include "ofApp.h"
#include <math.h> 
#define BOOST_FILESYSTEM_VERSION 3
#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <fstream>

namespace fs = std::filesystem;
typedef std::vector<std::string> stringvec;
stringvec videos;
stringvec images;
stringvec extVideos;
stringvec extImages;
std::string path("/media/pi/USBKey");

bool foundMidiController = false;

///////////////////////////////////////
//--------------------------------------------------------------
void ofApp::setup(){
	ctrlValues.value1 = 0.0f;
	ctrlValues.value2 = 0.0f;
	ctrlValues.value3 = 0.0f;
	ctrlValues.value4 = 0.0f;
	jsValues.js1x = 0.0f;
	jsValues.js1y = 0.0f;
	jsValues.js2x = 0.0f;
	jsValues.js2y = 0.0f;
	
	ofSetVerticalSync(false);
	shader1.load("shadersES2/shader1");
	shader2.load("shadersES2/shader2");
	shader3.load("shadersES2/shader3");
	shader4.load("shadersES2/shader4");
	lumakey.load("shadersES2/lumakey");
	shader6.load("shadersES2/hueShift");

	//setup midi controller
	nanoKontrol.setPortID(1);
	foundMidiController = nanoKontrol.setup();
	if (foundMidiController) {
		ofAddListener(nanoKontrol.callbacks.cycle, this, &ofApp::togglePaintModeCallback);
		ofAddListener(nanoKontrol.callbacks.player.rewind, this, &ofApp::previousSourceCallback);
		ofAddListener(nanoKontrol.callbacks.player.forward, this, &ofApp::nextSourceCallback);
		ofAddListener(nanoKontrol.callbacks.track.left, this, &ofApp::clearFeedbackCallback);
		ofAddListener(nanoKontrol.callbacks.track.right, this, &ofApp::centerFrameBufferCallback);
		ofAddListener(nanoKontrol.callbacks.marker.set, this, &ofApp::toggleSourceParamModeCallback);
		ofAddListener(nanoKontrol.callbacks.marker.right, this, &ofApp::changeFeedbackModeCallback);
		ofAddListener(nanoKontrol.callbacks.player.rec, this, &ofApp::toggleCamCallback);
	}

	//assigning the ADC ic's (MCP3008) pin number to potControllers
	isReady = pot1.setup(5);
	pot2.setup(2);
	pot3.setup(4);
	pot4.setup(3);
	js1x.setup(7);
	js1y.setup(6);
	js2x.setup(1);
	js2y.setup(0);

	button1.setup(2);
	button2.setup(0);
	clickCounter1.setup(0);
	clickCounter2.setup(2);

	image1.load("images/noInput.png");

	//CONTROL VARIABLES
	yAxis1 = 0.0;
	xAxis1 = 0.0;
	xAxis2 = 0.5;
	yAxis2 = 0.5;
	nClicks1 = 0;
	nClicks2 = 0;
	selector = 1;
	num = 0;
	imageNum = 0;
	camOn = false;
	paintMode = false;
	sourceParamMode = false;
	paintMode = false;
	noInput = false;
	turnOffStarted = false;
	notLoading = false;
	feedbackRotateWaitRecall = false;
	feedbackZoomWaitRecall = false;
	sourceRotateWaitRecall = false;
	sourceZoomWaitRecall = false;
	loadCount = 0;

	
	ofHideCursor();
	ofBackground(0, 0, 0);
	ofSetFrameRate(30);
	texture1.allocate(ofGetWidth(), ofGetHeight(),GL_RGBA);
	buffer.allocate(ofGetWidth(), ofGetHeight(),GL_RGBA);
	buffer2.allocate(ofGetWidth(), ofGetHeight(),GL_RGBA);
	buffer3.allocate(ofGetWidth(), ofGetHeight(),GL_RGBA);
	
	buffer.begin();
	ofClear(255,255,255, 0);
	buffer.end();	
	buffer2.begin();
	ofClear(255,255,255, 0);
	buffer2.end();	
	buffer3.begin();
	ofClear(255,255,255, 0);
	buffer3.end();	

	//The following has to be UNCOMMENTED for NTSC operation ->
	/* system("sudo v4l2-ctl -d /dev/video0 --set-standard=0");
	cam.setDeviceID(0);
	cam.setDesiredFrameRate(30);
    	cam.initGrabber(720, 480); */
	//The following has to be UNCOMMENTED for PAL operation ->
	system("sudo v4l2-ctl -d /dev/video0 --set-standard=6");
	cam.setDeviceID(0);
	cam.setDesiredFrameRate(25);
  cam.initGrabber(720, 576);
	
	if(cam.isInitialized()){
		camOn = true;
	}
	extVideos.push_back(".mp4");
	extVideos.push_back(".avi");
	extVideos.push_back(".flv");
	extVideos.push_back(".mov");
	extVideos.push_back(".mkv");
	extVideos.push_back(".mpg");
	extImages.push_back(".bmp");
	extImages.push_back(".png");
	extImages.push_back(".gif");
	extImages.push_back(".jpg");
	
	
	system("sudo mount /dev/sda1 /media/pi/USBKey -o uid=pi,gid=pi");
	getVideos(path, extVideos, videos);
	getImages(path, extImages, images);
	if (images.size() == 0 && videos.size() == 0 && !cam.isInitialized()){
		noInput = true;
	}
	else if(videos.size() > 0){	
		ofxOMXPlayerSettings settings;
		settings.enableAudio = false;
		player.loadMovie(videos[num]);
		
	}
	if(images.size() > 0){
		paintImage.load(images[0]);
		if (videos.size() == 0 && !cam.isInitialized()){
			paintMode = true;
		}
		
	}

} 

//--------------------------------------------------------------

void ofApp::update(){
	powerOffCheckRoutine();
	if (!isReady) {
		return;
	}

	nanoKontrol.update();
	
	if(camOn){
		cam.update();
	}	
	//Here are calle the methods that read the device controls and check that
	//the loaded video is playing back correctly. Check them out, they're after the draw method.
	checkClicksRoutine();
	checkJoysticksRoutine();
	updateControlsRoutine();
	checkVideoPlayback();
}

//--------------------------------------------------------------
void ofApp::draw() {
// ofLog(OF_LOG_NOTICE,"fps: " + ofToString(ofGetFrameRate())+ " xAxis1: "  + ofToString(xAxis1) + " yAxis1:" + ofToString(yAxis1) + " xAxis2:" + ofToString(xAxis2) + " yAxis2:" + ofToString(yAxis2)) ;
	if(!player.isTextureEnabled())
	{
		return;
	}
	if (noInput == true){
		image1.draw(0,0, ofGetWidth(), ofGetHeight());
		
	}
	else{

	buffer.begin();

	switch(selector){
  		case 1://CONTRAST
			shader1.begin();
			shader1.setUniform1f("in1", knob);
			shader1.setUniform1f("dispX", dispX*0.04);
			shader1.setUniform1f("dispY", dispY*0.04);
			shader1.setUniform1f("in2", yAxis2);
			ofPushMatrix();
			ofTranslate(ofGetWidth()/2,ofGetHeight()/2,0);
			ofRotateZDeg((int)rotate);
			ofTranslate(0, 0, zoom);
			
			texture1.draw(-ofGetWidth()/2,-ofGetHeight()/2);
			
			ofPopMatrix();
			shader1.end();
			break;


		case 2: //CHANGE HUE
			shader2.begin();
			shader2.setUniform1f("in1", yAxis2);
			shader2.setUniform1f("in2", knob);
			shader2.setUniform1f("dispX", dispX*0.04);
			shader2.setUniform1f("dispY", dispY*0.04);

			ofPushMatrix();
			
			ofTranslate(ofGetWidth()/2,ofGetHeight()/2,0);
			ofRotateZDeg((int)rotate);
			ofTranslate(0, 0, zoom);

			texture1.draw(-ofGetWidth()/2,-ofGetHeight()/2);		
			
			ofPopMatrix();
			shader2.end();
			break;

		case 3://NEGATIVE
			shader3.begin();
			shader3.setUniform1f("dispX", dispX*0.04);
			shader3.setUniform1f("dispY", dispY*0.04);
			shader3.setUniform1f("in1", yAxis2);
			shader3.setUniform1f("in2", knob);

			ofPushMatrix();

			ofTranslate(ofGetWidth()/2,ofGetHeight()/2,0);
			ofTranslate(0, 0, zoom);
			ofRotateZDeg((int)rotate);

			texture1.draw(-ofGetWidth()/2,-ofGetHeight()/2);
			
			ofPopMatrix();
			shader3.end();
			break;
		case 4: //PIXELATE
			shader4.begin();
			shader4.setUniform1f("in1", knob);
			shader4.setUniform1f("in2", yAxis2);
			shader4.setUniform1f("dispX", dispX*0.04);
			shader4.setUniform1f("dispY", dispY*0.04);
			shader4.setUniform1f("textureWidth", (float) ofGetWidth());
			shader4.setUniform1f("textureHeight", (float) ofGetHeight());
			ofPushMatrix();
			ofTranslate(ofGetWidth()/2,ofGetHeight()/2,0);
			ofTranslate(0, 0, zoom);
	
			ofRotateZDeg((int)rotate);
			texture1.draw(-ofGetWidth()/2,-ofGetHeight()/2);
		
			ofPopMatrix();
			shader4.end();
			break;
	case 5://pure feedback with speed control
			
			ofPushMatrix();
			ofTranslate(ofGetWidth()/2,ofGetHeight()/2,0);
			ofRotateZDeg((int)rotate);
			ofTranslate(-dispX*10,-dispY*10, zoom);
			if(ofGetFrameNum() % (uint64_t)(1.0+(knob*3)) == 0 ){
				texture1.draw(-ofGetWidth()/2,-ofGetHeight()/2);
			}
			ofPopMatrix();
			break;

	}
	
	buffer.end();
	buffer2.begin();
	ofClear(255,255,255, 0);

    //LUMA KEY
	lumakey.begin();
	lumakey.setUniform1f("th1", keyTresh);
	lumakey.setUniform1f("op1", 0.0); 

	ofPushMatrix();
	ofTranslate(ofGetWidth()/2, ofGetHeight()/2, 0);
	ofTranslate(-sourceDispX * 600, -sourceDispY * 600, sourceZoom);
	ofRotateZDeg((int)sourceRotate);
	ofTranslate(-ofGetWidth()/2, -ofGetHeight()/2, 0);

	drawSource();

	ofPopMatrix();

	lumakey.end();
	buffer2.end();
	buffer3.begin();
	buffer.draw(0, 0);
	buffer2.draw(0, 0);
	buffer3.end();
	//Load here the feedback
	texture1 = buffer3.getTexture();
	//FINAL HUE+SAT ADJUST
	shader6.begin();
	

	if(selector == 5){ //if in no effect mode you can adjust saturation also.
		shader6.setUniform1f("in2",yAxis2);
	}
	else if(selector == 2){
		shader6.setUniform1f("in2", 0.65);
	}
	else{
		shader6.setUniform1f("in2", 1.3);
	}
	shader6.setUniform1f("hue", xAxis2);

	buffer.draw(0, 0);
	shader6.end();
	
	buffer2.draw(0, 0);
	}

	for (int i = 0; i < 8; i++) {
    ofDrawBitmapStringHighlight(ofToString(nanoKontrol.values.ctrl[i].knob, 3), (100 * i) + 20, 100);
  }

	for (int i = 0; i < 8; i++) {
    ofDrawBitmapStringHighlight(ofToString(nanoKontrol.values.ctrl[i].slider, 3), (100 * i) + 20, 150);
  }

	ofDrawBitmapStringHighlight(ofToString(num), 20, 200);
	ofDrawBitmapStringHighlight(ofToString(videos[num]), 20, 240);
}

//--------------------------------------------------------------

void ofApp::checkClicksRoutine(){
	if (foundMidiController) {
		
	} else {
		checkClicks();
	}
}

void ofApp::checkClicks(){
	nClicks1 = clickCounter1.update();
	nClicks2 = clickCounter2.update();
	switch(nClicks1){
		case 1:
			centerFrameBuffer();
			nClicks1 = 0;
			break;
		
		case 2:
			toggleSourceParamMode();
			nClicks1 = 0;
			break;
		
		case 3:
			nextSource();
			nClicks1 = 0;
			break;
			
		default:
			nClicks1 = 0;
			break;
	}

	
	switch(nClicks2){
		case 1:
			clearFeedback();
			nClicks2 = 0;
			break;
		
		case 2:
			changeFeedbackMode();
			nClicks2 = 0;
			break;
		case 3:
			togglePaintMode();
			nClicks2 = 0;
			break;
		case 4:
			toggleCam();
			nClicks2 = 0;
			break;
		default:
			nClicks2 = 0;
			break;
	}
}

void ofApp::centerFrameBufferCallback(bool & v){
	centerFrameBuffer();
}
void ofApp::centerFrameBuffer(){
	//Center the framebuffer position
	xAxis1 = 0;
	yAxis1 = 0;
}

void ofApp::toggleSourceParamModeCallback(bool & v){
	toggleSourceParamMode();
}
void ofApp::toggleSourceParamMode(){
	//Change source position, rotation and zoom, or go back
	//to changing parameters in the feedback
	if(!paintMode){
		sourceParamMode = !sourceParamMode;
		if(sourceParamMode){
			sourceZoomWaitRecall = true;
			sourceRotateWaitRecall = true;
			xAxis1 = sourceDispX;
			yAxis1 = sourceDispY;
		}
		else{
			feedbackZoomWaitRecall = true;
			feedbackRotateWaitRecall = true;
			xAxis1 = dispX;
			yAxis1 = dispY;
		}
	}
}

void ofApp::previousSourceCallback(bool & v){
	previousSource();
}
void ofApp::previousSource(){
	//Jump to next video
	if(!paintMode){
		previousVideo();
	} else {
		previousImage();
	}
}

void ofApp::nextSourceCallback(bool & v){
	nextSource();
}
void ofApp::nextSource(){
	//Jump to next video
	if(!paintMode){
		nextVideo();
	}
	else{
		nextImage();
	}
}

void ofApp::clearFeedbackCallback(bool & v){
	clearFeedback();
}
void ofApp::clearFeedback(){
	//Zero out the feedback parameters and clear the
	//frame buffer
	xAxis2 = 0.0;
	yAxis2 = 0.5;
	buffer.begin();
	ofClear(255,255,255, 0);
	buffer.end();	
	buffer2.begin();
	ofClear(255,255,255, 0);
	buffer2.end();	
	buffer3.begin();
	ofClear(255,255,255, 0);
	buffer3.end();
}

void ofApp::changeFeedbackModeCallback(bool & v){
	changeFeedbackMode();
}
void ofApp::changeFeedbackMode(){
	//Change feedback mode
	if (selector < 5) {
		selector++;
	}	else {
		selector = 1;
	}
}

void ofApp::togglePaintModeCallback(bool & v){
	togglePaintMode();
}
void ofApp::togglePaintMode(){
	//Go to paint mode (if images are present) and back
	if(images.size() > 0){
		paintMode = !paintMode;
		if(paintMode){
			xAxis1 = 0;
			yAxis1 = 0;
		}
		else{
			feedbackRotateWaitRecall = true;
			feedbackZoomWaitRecall = true;
			xAxis1 = dispX;
			yAxis1 = dispX;
		}
	}
}

void ofApp::toggleCamCallback(bool & v){
	toggleCam();
}
void ofApp::toggleCam(){
	//switch from capture to stored videos mode if possible
	if(cam.isInitialized()&& videos.size()>0){
		camOn = !camOn;
	}
}


void ofApp::checkJoysticksRoutine(){
	if (foundMidiController) {
		jsValues.js1x = nanoKontrol.values.ctrl[0].slider * 1023.0;
		jsValues.js1y = nanoKontrol.values.ctrl[1].slider * 1023.0;
		jsValues.js2x = nanoKontrol.values.ctrl[2].slider * 1023.0;
		jsValues.js2y = nanoKontrol.values.ctrl[3].slider * 1023.0;
	} else {
		jsValues.js1x = js1x.potValue;
		jsValues.js1y = js1y.potValue;
		jsValues.js2x = js2x.potValue;
		jsValues.js2y = js2y.potValue;
	}
	checkJoysticks(jsValues);
}

void ofApp::checkJoysticks(joystickValues values){
	if(!paintMode && !sourceParamMode){
		dispX = xAxis1;
		dispY = yAxis1;
	}
	else if(paintMode){
		paintDispX = xAxis1*(ofGetWidth() + (paintImage.getWidth()/5));
		paintDispY = yAxis1*(ofGetHeight() + (paintImage.getHeight()/5));
	}
	else if(sourceParamMode){
		sourceDispX = xAxis1;
		sourceDispY = yAxis1;
	}
	if(jsValues.js1y > 535 || jsValues.js1y < 470){
		//The last two values in the following method determine the sensibility of the joystick
		xAxis1 += ofMap((float)jsValues.js1y, 0.0, 1023.0, 0.05, -0.05);
		//These are the bounds of the frame movement
		if(xAxis1 > 1.00)
				xAxis1 = 1.00;
		if(xAxis1 < -1.00)
				xAxis1 = -1.00; 
	}
	if(jsValues.js1x > 535 || jsValues.js1x < 470){
		yAxis1 += ofMap((float)jsValues.js1x, 0.0, 1023.0, -0.05, 0.05);
		if(yAxis1 > 1.0)
				yAxis1 = 1.00;
		if(yAxis1 < -1.0)
				yAxis1 = -1.00; 
	
	}

	if(jsValues.js2x > 535 || jsValues.js2x < 470){
		xAxis2 += ofMap((float)jsValues.js2x, 0.0, 1023.0, -0.015, +0.015);
		//*DON'T*adjust these to alter the bounds of the parameter
		//keep this 1.0 and 0.0 and alter the shader parameters
		if(xAxis2 > 1.0)
			xAxis2 = 0.0;
		if(xAxis2 < 0.0)
			xAxis2 = 1.0;
	}
	if(jsValues.js2y > 535 || jsValues.js2y < 470){
		yAxis2 += ofMap((float)jsValues.js2y, 0.0, 1023.0, -0.017, +0.017);
		if(yAxis2 > 1.0)
			yAxis2 = 1.0;
		if(yAxis2 < 0.0)
			yAxis2 = 0.0;		
	}
}

void ofApp::updateControlsRoutine(){
	if (foundMidiController) {
		ctrlValues.value1 = nanoKontrol.values.ctrl[0].knob;
		ctrlValues.value2 = nanoKontrol.values.ctrl[1].knob;
		ctrlValues.value3 = nanoKontrol.values.ctrl[2].knob;
		ctrlValues.value4 = nanoKontrol.values.ctrl[3].knob;
	} else {
		ctrlValues.value1 = ofMap((float)pot1.potValue, 0.0, 1023.0, 0.0f, 1.0);
		ctrlValues.value2 = ofMap((float)pot2.potValue, 0.0, 1023.0, 0.0f, 1.0);
		ctrlValues.value3 = ofMap((float)pot3.potValue, 0.0, 1023.0, 0.0f, 1.0);
		ctrlValues.value4 = ofMap((float)pot4.potValue, 0.0, 1023.0, 0.0f, 1.0);
	}
	updateControls(ctrlValues);
}
void ofApp::updateControls(controlsValues values){
	keyTresh = values.value2;
	if(keyTresh > -0.03 && keyTresh < 0.03){
		keyTresh = 0.0;
	}
	
	knob = ctrlValues.value4;
	if(!paintMode && !sourceParamMode){
		if(abs(zoom - ((ctrlValues.value1 * 100.0f) -50.0f))<10){
			feedbackZoomWaitRecall = false;
		}
		if(!feedbackZoomWaitRecall){
			zoom = ((ctrlValues.value1 * 100.0f) -50.0f);
		}
		if(abs(rotate - ((ctrlValues.value3 * 20.0f) - 10.0f))<2.5){
			feedbackRotateWaitRecall = false;
		}
		if(!feedbackRotateWaitRecall){
			rotate = ((ctrlValues.value3 * 20.0f) - 10.0f);
		}
	}
	if(sourceParamMode){
		//Don't change the parameters value until the potentiometer position
		//almost matches the stored value
		if(abs(oldSourceRotate - ((ctrlValues.value3 * 360.0f) - 180.0f))<5){
			sourceRotateWaitRecall = false;
		}
		if(!sourceRotateWaitRecall){
			//the following code avoids jump in the parameters 
			//removing input noise
			sourceRotate = ((ctrlValues.value3 * 360.0f) - 180.0f);
			if(abs(sourceRotate) < 1){
				sourceRotate = 0;
			} 
			if(abs(sourceRotate-oldSourceRotate) < 3){
				sourceRotate = oldSourceRotate;
			}
			oldSourceRotate = sourceRotate;
		}
		if(abs(oldSourceZoom - ((ctrlValues.value1 * 1200.0f) -600.0f))< 30){
			sourceZoomWaitRecall = false;
		}
		if(!sourceZoomWaitRecall){
			sourceZoom = ((ctrlValues.value1 * 1200.0f) -600.0f);
			if(abs(sourceZoom-oldSourceZoom) < 30){
				sourceZoom = oldSourceZoom;
			}
			oldSourceZoom = sourceZoom;
		}
	}
	//If in paintmode map the controls to paintparams
    // and then set dispX and dispY to 0 so that
 	//the frame buffer is centered
	if(paintMode){
		paintRotate = ((ctrlValues.value3 * 360.0f) - 180.0f);
		if(abs(paintRotate) < 1){
			paintRotate = 0;
		} 
		if(abs(paintRotate-oldPaintRotate) < 2){
			paintRotate = oldPaintRotate;
		}
		oldPaintRotate = paintRotate;
		paintZoom = ((ctrlValues.value1 * 1200.0f) -600.0f);
		if(abs(paintZoom-oldPaintZoom) < 20){
			paintZoom = oldPaintZoom;
		}
		oldPaintZoom = paintZoom;
		dispX = 0;
		dispY = 0;
	}
}

void ofApp::previousVideo(){
	if(videos.size() > 0){
		if (num-1 > 0){
			num -= 1;
		} else {
			num = videos.size() - 1;
		}
		player.loadMovie(videos[num]);
	}
}

void ofApp::nextVideo(){
	if(videos.size() > 0){
		if (num+1 < videos.size()){
			num += 1;}
		else{
			num = 0;}
		player.loadMovie(videos[num]);
	}
}

void ofApp::drawSource(){
	if(camOn){
		cam.draw(0,0, ofGetWidth(), ofGetHeight());
	}
	else if (!paintMode){
		player.draw(0,0, ofGetWidth(), ofGetHeight());
	}
	else{
		ofPushMatrix();
		ofTranslate(paintImage.getWidth()/2, paintImage.getHeight()/2);
		ofTranslate(0, 0, paintZoom);
		ofTranslate(-paintDispX, -paintDispY);
		ofRotateZDeg(paintRotate);
		paintImage.draw(-paintImage.getWidth()/2, -paintImage.getHeight()/2);
		ofPopMatrix();
	}
}
void ofApp::previousImage(){
	if(images.size() > 0){
		if (imageNum-1 > 0){
			imageNum -= 1;
		}	else {
			imageNum = images.size() - 1;
		}
		paintImage.load(images[imageNum]);
	}
}
void ofApp::nextImage(){
	if(images.size() > 0){
		if (imageNum+1 < images.size()){
			imageNum += 1;}
		else{
			imageNum = 0;}
		paintImage.load(images[imageNum]);
	}
}
void ofApp::powerOffCheckRoutine(){
	//If both button remain pressed for 3 seconds the device turns off
	if(button1.currentValue && button2.currentValue){

		if(!turnOffStarted){
			beginTime = ofGetSystemTimeMillis();
			turnOffStarted = true;
		}
		else{
			if(ofGetSystemTimeMillis() - beginTime > 2000){
				system("sudo killall -9 feedback; sudo poweroff");
			}
		}
	}
	else{
		turnOffStarted = false;
	}
}

void ofApp::checkVideoPlayback(){
	if(videos.size() > 0){
		if(player.isFrameNew()){
			notLoading = false;
		}
		else if(!player.isFrameNew() && !notLoading && !camOn){
			notLoading = true;
			loadCount = ofGetSystemTimeMillis();
		}
		else if(!player.isFrameNew() && notLoading && !camOn){
			if(ofGetSystemTimeMillis()-loadCount >= 1000 && loadCount > 0){
				ofLogNotice() << "***VIDEO WASN'T LOADING, SKIPPED IT!*** " << videos[num];
				notLoading = false;
				loadCount = 0;
				videos.erase(videos.begin()+num);
				if (num >= videos.size()) {
					num = videos.size() - 1;
				}
				player.loadMovie(videos[num]);
			}
		}
		
	}
}

//These two methods look through the directories in the specifed folder and selects video/image
//files with the specified extensions.
void ofApp::getVideos(const std::string& path, stringvec& ext,  stringvec& vector){
	for(auto& p: fs::recursive_directory_iterator(path)){
		for(int i = 0; i < ext.size(); i++){
			if(boost::iequals(p.path().extension().string(), ext[i])){
				vector.push_back(p.path().string());
				break;
			}
		}
		std::sort(vector.begin(),vector.end());
	}
}
void ofApp::getImages(const std::string& path, stringvec& ext,  stringvec& vector){
	for(auto& p: fs::recursive_directory_iterator(path))
    	{
		for(int i = 0; i < ext.size(); i++){
			if(boost::iequals(p.path().extension().string(), ext[i])){
			   vector.push_back(p.path().string());
			   break;
			}
		}
		std::sort(vector.begin(),vector.end());
        }
}

