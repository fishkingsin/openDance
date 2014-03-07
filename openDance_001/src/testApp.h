#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxLibwebsockets.h"

#define NUM_MESSAGES 30 // how many past messages we want to keep
#ifdef LED
#include "ofxLPD8806.h"
#include "ofxLEDsLPD8806.h"
#endif
#define row 1
#define col 160
#include "ofxDuration.h"

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void exit();
		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        void printClients();
        ofxLibwebsockets::Server server;
    
        bool bConnected;
    
        //queue of rec'd messages
        vector<string> messages;
    
        //string to send to clients
        string toSend;
    
        // websocket methods
        void onConnect( ofxLibwebsockets::Event& args );
        void onOpen( ofxLibwebsockets::Event& args );
        void onClose( ofxLibwebsockets::Event& args );
        void onIdle( ofxLibwebsockets::Event& args );
        void onMessage( ofxLibwebsockets::Event& args );
        void onBroadcast( ofxLibwebsockets::Event& args );
#ifdef LED
	ofxLEDsLPD8806 *led;
	
	ofxLPD8806 spi;
#endif
	int numLED;
//	vector<ofColor> colors;
    ofImage spectum;
    ofRectangle spectumRect;
    ofColor myofColour;
    ofColor myColor;
    float myBrightness;
    float mySaturation;

    
    //----------------------------------------------------
	ofxDuration duration;
    
	void trackUpdated(ofxDurationEventArgs& args);
	string ip;
	int port;
    //----------------------------------------------------
    

};
