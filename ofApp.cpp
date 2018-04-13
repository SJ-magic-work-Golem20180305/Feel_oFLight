/************************************************************
************************************************************/
#include "ofApp.h"

/************************************************************
************************************************************/
#include "ofApp.h"

/************************************************************
************************************************************/
enum{
	SIZE_DMX_UNIVERSE = 512,
};

enum LED_DEVICE_TYPE{
	LED_DEVICE_TYPE_FIXED,
	LED_DEVICE_TYPE_MOVING,
};

/************************************************************
private class
************************************************************/
class ODE{
private:
	char ch_IP[BUF_SIZE];
	
public:
	unsigned char universe[SIZE_DMX_UNIVERSE];
	
	ODE(const char* _ch_IP)
	{
		strcpy(ch_IP, _ch_IP);
	}
	const char* get_IP()
	{
		return ch_IP;
	}
};

struct LED_PARAM{
	unsigned char R;
	unsigned char G;
	unsigned char B;
	unsigned char W;
	unsigned char A;
	
	int Pan;
	int Tilt;
	
	
	
	LED_PARAM()
	: R(0), G(0), B(0), W(0), Pan(0), Tilt(0)
	{
	}
	
	void clear()
	{
		R = 0; G = 0; B = 0; W = 0;
		Pan = 0; Tilt = 0;
	}
};

struct LED_LIGHT{
	const int ODE_id;
	const int AddressFrom;
	const enum LED_DEVICE_TYPE LedDeviceType;
	
	LED_PARAM LedParam;
	
	LED_LIGHT(int _ODE_id, int _AddressFrom, enum LED_DEVICE_TYPE _LedDeviceType)
	: ODE_id(_ODE_id), AddressFrom(_AddressFrom), LedDeviceType(_LedDeviceType)
	{
	}
};

/************************************************************
param
************************************************************/
/********************
********************/
static ODE ode[] = {
	ODE("10.7.206.7"),
};
static const int NUM_ODES = sizeof(ode) / sizeof(ode[0]);

/********************
********************/
static LED_LIGHT LedLight[] = {
	LED_LIGHT(0, 0, LED_DEVICE_TYPE_MOVING),
};

static const int NUM_LEDS = sizeof(LedLight) / sizeof(LedLight[0]);





/************************************************************
************************************************************/
/******************************
******************************/
ofApp::ofApp(int _BootMode)
: Osc("127.0.0.1", 13000, 13001)
, OSC_PanelTouchedPos(-1)
, b_OscReceived(false)
, b_Simulation(false)
, State(STATE_OFF)
, t_from_StateChart(0)
, BootMode(BOOT_MODE(_BootMode))
, b_Invert_PanDirection(false)
{
}

/******************************
******************************/
ofApp::~ofApp()
{
	/********************
	何故か、exit()で以下を記述すると、dmx commandが上手く送られず、照明が光ったまま停止してしまった。
	こちらに持ってくると所望の動作となったので、デストラクタで処理する。
	********************/
	Send_AllZero_to_AllOde();
	printMessage("GoodBye");
}

/******************************
******************************/
void ofApp::exit()
{
	// printf("> Exit\n");
}

/******************************
******************************/
void ofApp::Send_AllZero_to_AllOde()
{
	for(int i = 0; i < NUM_ODES; i++){
		for(int j = 0; j < SIZE_DMX_UNIVERSE; j++){
			ode[i].universe[j] = 0;
		}
		
		artnet.sendDmx(ode[i].get_IP(), ode[i].universe, SIZE_DMX_UNIVERSE);
	}
}

//--------------------------------------------------------------
void ofApp::setup(){
	/********************
	********************/
	ofSetWindowTitle("oF Light");
	ofSetVerticalSync(true);
	ofSetFrameRate(60); // if vertical sync is off, we can go a bit fast... this caps the framerate at 60fps.
	ofSetWindowShape(WIDTH, HEIGHT);
	ofSetEscapeQuitsApp(false);
	
	ofEnableAlphaBlending();
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	// ofEnableBlendMode(OF_BLENDMODE_ADD);
	// ofEnableSmoothing();
	
	ofSetCircleResolution(50);
	
	/********************
	********************/
	gui.setup();
	{
		ofColor initColor = ofColor(255, 255, 255, 255);
		ofColor minColor = ofColor(0, 0, 0, 0);
		ofColor maxColor = ofColor(255, 255, 255, 255);
		gui.add(gui_LedColor.setup("color", initColor, minColor, maxColor));
	}
	
	/********************
	********************/
    //at first you must specify the Ip address of this machine
    artnet.setup("10.0.0.5"); //make sure the firewall is deactivated at this point
}

//--------------------------------------------------------------
void ofApp::update(){
	/********************
	********************/
	while(Osc.OscReceive.hasWaitingMessages()){
		ofxOscMessage m_receive;
		Osc.OscReceive.getNextMessage(&m_receive);
		
		if(m_receive.getAddress() == "/PanelToucehd_Pos"){
			OSC_PanelTouchedPos = m_receive.getArgAsFloat(0);
			
			b_OscReceived = true;
		}
	}
	
	/********************
	********************/
	StateChart();
	
	/********************
	********************/
	b_OscReceived = false;
	b_Simulation = false;
} 

/******************************
******************************/
void ofApp::StateChart()
{
	if(BootMode == BOOTMODE_SIMULATION) StateChart_simulation();
	else								StateChart_osc();
}

/******************************
******************************/
void ofApp::StateChart_simulation()
{
	/********************
	********************/
	switch(State){
		case STATE_OFF:
			if(b_Simulation){
				State = STATE_ON;
			}
			break;
			
		case STATE_ON:
			if(b_Simulation){
				State = STATE_OFF;
			}
			break;
	}
	
	/********************
	********************/
	switch(State){
		case STATE_OFF:
			Set_LedRegister_OFF();
			break;
			
		case STATE_ON:
			if(b_Invert_PanDirection)	Set_LedRegister_ON(mouseX, 0, ofGetWidth(), PAN_RIGHT, PAN_LEFT);
			else						Set_LedRegister_ON(mouseX, 0, ofGetWidth(), PAN_LEFT, PAN_RIGHT);
			break;
	}
}

/******************************
******************************/
void ofApp::StateChart_osc()
{
	/********************
	********************/
	float now = ofGetElapsedTimef();
	
	/********************
	********************/
	switch(State){
		case STATE_OFF:
			if(b_OscReceived && OSC_PanelTouchedPos != -1){
				State = STATE_ON;
				t_from_StateChart = now;
			}
			break;
			
		case STATE_ON:
			if(b_OscReceived && OSC_PanelTouchedPos != -1){
				t_from_StateChart = now;
			}else if(1.0 < now - t_from_StateChart){
				State = STATE_OFF;
			}
			break;
	}
	
	/********************
	********************/
	switch(State){
		case STATE_OFF:
			Set_LedRegister_OFF();
			break;
			
		case STATE_ON:
			if(b_OscReceived && OSC_PanelTouchedPos != -1){
				if(b_Invert_PanDirection)	Set_LedRegister_ON(OSC_PanelTouchedPos, OSC_PANELPOS_MIN, OSC_PANELPOS_MAX, PAN_RIGHT, PAN_LEFT);
				else						Set_LedRegister_ON(OSC_PanelTouchedPos, OSC_PANELPOS_MIN, OSC_PANELPOS_MAX, PAN_LEFT, PAN_RIGHT);
			}
			break;
	}
}

/******************************
******************************/
void ofApp::Set_LedRegister_ON(float val, float input_from, float input_to, float out_from, float out_to)
{
	ofColor LedColor = gui_LedColor;
	
	for(int i = 0; i < NUM_LEDS; i++){
		LedLight[i].LedParam.R = int(LedColor.r);
		LedLight[i].LedParam.G = int(LedColor.g);
		LedLight[i].LedParam.B = int(LedColor.b);
		LedLight[i].LedParam.W = 0;
		LedLight[i].LedParam.A = int(LedColor.a);
		
		LedLight[i].LedParam.Pan = int(ofMap(val, input_from, input_to, out_from, out_to, true));
		LedLight[i].LedParam.Tilt = TILT_ON;
	}
}

/******************************
******************************/
void ofApp::Set_LedRegister_OFF()
{
	ofColor LedColor = gui_LedColor;
	
	for(int i = 0; i < NUM_LEDS; i++){
		LedLight[i].LedParam.R = 0;
		LedLight[i].LedParam.G = 0;
		LedLight[i].LedParam.B = 0;
		LedLight[i].LedParam.W = 0;
		LedLight[i].LedParam.A = 0;
		
		LedLight[i].LedParam.Pan = PAN_CENTER;
		LedLight[i].LedParam.Tilt = TILT_OFF;
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	/********************
	********************/
	ofBackground(30);
	
	draw_Led();
	
	/********************
	********************/
	gui.draw();
	
	/********************
	********************/
	ofSetColor(255, 0, 0, 255);
	
	char buf[BUF_SIZE];
	sprintf(buf, "%7.2f", OSC_PanelTouchedPos);
	ofDrawBitmapString(buf, 20, 20);
}

/******************************
******************************/
void ofApp::draw_Led()
{
	/********************
	********************/
	for(int i = 0; i < NUM_LEDS; i++){
		switch(LedLight[i].LedDeviceType){
			case LED_DEVICE_TYPE_FIXED:
				ode[ LedLight[i].ODE_id ].universe[ LedLight[i].AddressFrom + 0 ] = LedLight[i].LedParam.A;
				ode[ LedLight[i].ODE_id ].universe[ LedLight[i].AddressFrom + 1 ] = LedLight[i].LedParam.R;
				ode[ LedLight[i].ODE_id ].universe[ LedLight[i].AddressFrom + 2 ] = LedLight[i].LedParam.G;
				ode[ LedLight[i].ODE_id ].universe[ LedLight[i].AddressFrom + 3 ] = LedLight[i].LedParam.B;
				ode[ LedLight[i].ODE_id ].universe[ LedLight[i].AddressFrom + 4 ] = LedLight[i].LedParam.W;
				ode[ LedLight[i].ODE_id ].universe[ LedLight[i].AddressFrom + 5 ] = 1; // Strobe = open.
				
				break;
				
			case LED_DEVICE_TYPE_MOVING:
				ode[ LedLight[i].ODE_id ].universe[ LedLight[i].AddressFrom +  0 ] = (unsigned char)((LedLight[i].LedParam.Pan >> 8) & 0xFF);	// H
				ode[ LedLight[i].ODE_id ].universe[ LedLight[i].AddressFrom +  1 ] = (unsigned char)((LedLight[i].LedParam.Pan >> 0) & 0xFF);	// L
				ode[ LedLight[i].ODE_id ].universe[ LedLight[i].AddressFrom +  2 ] = (unsigned char)((LedLight[i].LedParam.Tilt >> 8) & 0xFF);	// H
				ode[ LedLight[i].ODE_id ].universe[ LedLight[i].AddressFrom +  3 ] = (unsigned char)((LedLight[i].LedParam.Tilt >> 0) & 0xFF);	// L
				ode[ LedLight[i].ODE_id ].universe[ LedLight[i].AddressFrom +  4 ] = LedLight[i].LedParam.R;
				ode[ LedLight[i].ODE_id ].universe[ LedLight[i].AddressFrom +  5 ] = LedLight[i].LedParam.G;
				ode[ LedLight[i].ODE_id ].universe[ LedLight[i].AddressFrom +  6 ] = LedLight[i].LedParam.B;
				ode[ LedLight[i].ODE_id ].universe[ LedLight[i].AddressFrom +  7 ] = LedLight[i].LedParam.W;
				ode[ LedLight[i].ODE_id ].universe[ LedLight[i].AddressFrom +  8 ] = 8;	// shutter open
				
				ode[ LedLight[i].ODE_id ].universe[ LedLight[i].AddressFrom +  9 ] = LedLight[i].LedParam.A;
				
				ode[ LedLight[i].ODE_id ].universe[ LedLight[i].AddressFrom + 10 ] = 0; // Pan/Tilt speed : [0 : fast] <---> [slow : 255]
				ode[ LedLight[i].ODE_id ].universe[ LedLight[i].AddressFrom + 11 ] = 0; // Blackout reset
				ode[ LedLight[i].ODE_id ].universe[ LedLight[i].AddressFrom + 12 ] = 0; // dimmer curve = standard
				
				break;
		}
	}
	
	/********************
	********************/
	for(int i = 0; i < NUM_ODES; i++){
		artnet.sendDmx(ode[i].get_IP(), ode[i].universe, SIZE_DMX_UNIVERSE);
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch(key){
		case ' ':
			b_Simulation = true;
			break;
			
		case 'i':
			b_Invert_PanDirection = !b_Invert_PanDirection;
			
			printf("b_Invert_PanDirection = %d\n", b_Invert_PanDirection);
			fflush(stdout);
			break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
