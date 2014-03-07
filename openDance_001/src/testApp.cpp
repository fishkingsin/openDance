#include "testApp.h"

#include <locale>
float myHue = 0;

bool isNumber(const string& s){
	locale loc;
	std::string::const_iterator it = s.begin();
    while (it != s.end() && (std::isdigit(*it, loc))) ++it;
	return !s.empty() && it == s.end();
}
//--------------------------------------------------------------
void testApp::setup(){
    spectumRect.set(0, 0, ofGetWidth()*0.5, ofGetHeight());
    spectum.loadImage("images.jpg");
	ofSetLogLevel(OF_LOG_VERBOSE);
    // setup a server with default options on port 9092
    // - pass in true after port to set up with SSL
    ofxXmlSettings xmlSettings("settings.xml");
    if(xmlSettings.pushTag("XML"))
    {
        string xmlString;
        xmlSettings.copyXmlToString(xmlString);
        ofLogVerbose()<<xmlString;
    }
    
    
    ofxLibwebsockets::ServerOptions options = ofxLibwebsockets::defaultServerOptions();
    options.port = 2014;
    options.documentRoot = ofToDataPath("web/");
    options.bUseSSL = false; //ssl not working on Win right now!
    bConnected = server.setup(options);
    
    // Uncomment this to set up a server with a protocol
    // Right now, clients created via libwebsockets that are connecting to servers
    // made via libwebsockets seem to want a protocol. Hopefully this gets fixed,
    // but until now you have to do something like this:
	
    /*
	 ofxLibwebsockets::ServerOptions options = ofxLibwebsockets::defaultServerOptions();
	 options.port = 9092;
	 options.protocol = "of-protocol";
	 bConnected = server.setup( options );
	 */
    
    // this adds your app as a listener for the server
    server.addListener(this);
    
    ofBackground(0);
    ofSetFrameRate(60);
	numLED = row*col;
#ifdef LED
	led = new ofxLEDsLPD8806(numLED);
	
	if(	spi.connect())
	{
		ofLogNotice()<<"connected to SPI";
	}
#endif
    
    //duration===============================================================
    ofBuffer settings = ofBufferFromFile("SimpleReceiverPort.txt");
	string portString = settings.getText();
	if(isNumber(portString)){
		port = ofToInt(portString);
	}
	else {
		port = 12345;
		ofSystemAlertDialog("SimpleReceiverPort.txt in data/ needs to contain a valid listening port number. Defaulting to 12345");
	}
	
	duration.setup(port);
	//ofxDuration is an OSC receiver, with special functions to listen for Duration specific messages
	//optionally set up a font for debugging
	duration.setupFont("GUI/NewMedia Fett.ttf", 12);
	ofAddListener(duration.events.trackUpdated, this, &testApp::trackUpdated);
    //duration===============================================================
	
}
void testApp::trackUpdated(ofxDurationEventArgs& args){
	ofLogVerbose("Duration Event") << "track type " << args.track->type << " updated with name " << args.track->name << " and value " << args.track->value << endl;
    {
        if(args.track->name.find("brightness")!=string::npos)
        {
            myBrightness = args.track->value;

        }
        if(args.track->name.find("saturate")!=string::npos)
        {
            mySaturation= args.track->value;


        }
        if(args.track->name.find("color")!=string::npos)
        {
            myHue = args.track->color.getHue();
            //            ofLogVerbose()<<myColor;
            
        }
        
        

        
    }
}
void testApp::exit()
{
#ifdef LED
	led->clear(0);
	
	spi.send(led->txBuffer);
#endif
}
//--------------------------------------------------------------
void testApp::update(){

    myColor.setHue(myHue);
    myColor.setSaturation(mySaturation);
    myColor.setBrightness(myBrightness);
    toSend = ofToHex(myColor.getHex());
    
	server.send(toSend.substr(2,string::npos-2) );

}

//--------------------------------------------------------------
void testApp::draw(){
    
//    spectum.draw(spectumRect);
    
//    ofPushStyle();
//    ofSetColor(myColor);
//    ofRect(0,0,ofGetWidth(),ofGetHeight());
//    ofPopStyle();
    
    ofPushStyle();
    ofSetColor(255,255, 255, 125);
    duration.draw(ofGetWidth()*0.5,0, ofGetWidth()*0.5, ofGetHeight());
    ofPopStyle();
    
    if ( bConnected ){
        ofDrawBitmapString("WebSocket server setup at "+ofToString( server.getPort() ) + ( server.usingSSL() ? " with SSL" : " without SSL"), 20, 20);
        
        ofSetColor(150);
        ofDrawBitmapString("Click anywhere to open up client example", 20, 40);
    } else {
        ofDrawBitmapString("WebSocket setup failed :(", 20,20);
    }
    
    int x = 20;
    int y = 100;
    
    ofSetColor(0,150,0);
    ofDrawBitmapString("Console", x, 80);
    
    ofSetColor(255);
    for (int i = messages.size() -1; i >= 0; i-- ){
        ofDrawBitmapString( messages[i], x, y );
        y += 20;
    }
    if (messages.size() > NUM_MESSAGES) messages.erase( messages.begin() );
    
    ofSetColor(150,0,0);
    ofDrawBitmapString("Type a message, hit [RETURN] to send:", x, ofGetHeight()-60);
    ofSetColor(255);
    ofDrawBitmapString(toSend, x, ofGetHeight() - 40);
	
	ofPushMatrix();
	ofScale(5, 5);
	ofPopMatrix();
}

//--------------------------------------------------------------
void testApp::onConnect( ofxLibwebsockets::Event& args ){
    cout<<"on connected"<<endl;
}

//--------------------------------------------------------------
void testApp::onOpen( ofxLibwebsockets::Event& args ){
    cout<<"new connection open"<<endl;
    messages.push_back("New connection from " + args.conn.getClientIP() + ", " + args.conn.getClientName() );
}

//--------------------------------------------------------------
void testApp::onClose( ofxLibwebsockets::Event& args ){
    cout<<"on close"<<endl;
    messages.push_back("Connection closed");
}

//--------------------------------------------------------------
void testApp::onIdle( ofxLibwebsockets::Event& args ){
    cout<<"on idle"<<endl;
}

//--------------------------------------------------------------
void testApp::onMessage( ofxLibwebsockets::Event& args ){
    cout<<"got message "<<args.message<<endl;
}

//--------------------------------------------------------------
void testApp::onBroadcast( ofxLibwebsockets::Event& args ){
    cout<<"got broadcast "<<args.message<<endl;
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    // do some typing!
    if ( key != OF_KEY_RETURN ){
        if ( key == OF_KEY_BACKSPACE ){
            if ( toSend.length() > 0 ){
                toSend.erase(toSend.end()-1);
            }
        } else {
            toSend += key;
        }
    } else {
        
        // send to all clients
        server.send( toSend );
        messages.push_back("Sent: '" + toSend + "' to "+ ofToString(server.getConnections().size())+" websockets" );
        toSend = "";
    }
    if(key==OF_KEY_RETURN)
    {
        ;
        
        toSend = ofToHex(myofColour.getHex());
        server.send( toSend.substr(2,string::npos-2) );
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
    //    int _x = x*((float)spectum.getWidth()/(float)spectumRect.width);
    //    int _y = y*((float)spectum.getHeight()/(float)spectumRect.height);
    //    myofColour = spectum.getPixelsRef().getColor(_x,_y);
    //    toSend = ofToHex(myofColour.getHex());
    //    server.send( toSend.substr(2,string::npos-2) );
	
    
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    string url = "http";
    if ( server.usingSSL() ){
        url += "s";
    }
    url += "://localhost:" + ofToString( server.getPort() );
    ofLaunchBrowser(url);
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
	
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
	
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){
	
}