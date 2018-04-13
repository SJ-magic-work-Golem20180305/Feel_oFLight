/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include "ofMain.h"
#include "ofxGui.h"
#include "ofxArtnet.h"

#include "sjCommon.h"
#include "sj_OSC.h"

/************************************************************
************************************************************/

/**************************************************
**************************************************/
class ofApp : public ofBaseApp{
private:
	/****************************************
	****************************************/
	enum{
		WIDTH = 1200,
		HEIGHT = 300,
	};
	
	enum BOOT_MODE{
		BOOTMODE_SIMULATION,
		BOOTMODE_OSC,
	};
	
	enum STATE{
		STATE_OFF,
		STATE_ON,
	};
	
	enum{
		PAN_LEFT	= 36408,
		PAN_RIGHT	= 50972,
		PAN_CENTER	= 43690,
		
		TILT_ON		= 6241,
		TILT_OFF	= 0,
		
		OSC_PANELPOS_MIN = 0,
		OSC_PANELPOS_MAX = 9,
	};
	
	/****************************************
	****************************************/
	BOOT_MODE BootMode;
	
	OSC_TARGET Osc;
	float OSC_PanelTouchedPos;
	
	ofxPanel gui;
	ofxColorSlider gui_LedColor;
	
	bool b_OscReceived;
	bool b_Simulation;
	bool b_Invert_PanDirection;
	
	ofxArtnet artnet;
	STATE State;
	float t_from_StateChart;
	
	/****************************************
	****************************************/
	void Send_AllZero_to_AllOde();
	void StateChart();
	void StateChart_simulation();
	void StateChart_osc();
	void Set_LedRegister_ON(float val, float input_from, float input_to, float out_from, float out_to);
	void Set_LedRegister_OFF();
	
	void draw_Led();
	
public:
	/****************************************
	****************************************/
	ofApp(int _BootMode);
	~ofApp();
	void exit();
	
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	
};
