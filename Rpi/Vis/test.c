#include <iostream>
#include "bcm_host.h"
#include "EGL/egl.h"
#include "VG/openvg.h"

using namespace std;

/*
	BCM STUFF
*/

EGL_DISPMANX_WINDOW_T nativewindow;
uint32_t display_width;
uint32_t display_height;

int startBCM() {

    //get the bcm setup
    bcm_host_init();

    //the variables
    DISPMANX_ELEMENT_HANDLE_T dispman_element;
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
    DISPMANX_UPDATE_HANDLE_T dispman_update;
    DISPMANX_TRANSFORM_T dispman_trans;
    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;

    // create an EGL window surface, passing context width/height
    int success = graphics_get_display_size(0 /* LCD */,
                                            &display_width, &display_height);
    if ( success < 0 )
    {
        return 1;
    }

    // You can hardcode the resolution here:
    //display_width = 640;
    //display_height = 480;

    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.width = display_width;
    dst_rect.height = display_height;

    src_rect.x = 0;
    src_rect.y = 0;
    src_rect.width = display_width << 16;
    src_rect.height = display_height << 16;

    dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
    dispman_update = vc_dispmanx_update_start( 0 );

    dispman_element = vc_dispmanx_element_add ( dispman_update,
                                                dispman_display, 0/*layer*/, &dst_rect, 0/*src*/,
                                                &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/,
                                                0/*clamp*/, dispman_trans/*transform*/);

    nativewindow.element = dispman_element;
    nativewindow.width = display_width;
    nativewindow.height = display_height;
    vc_dispmanx_update_submit_sync( dispman_update );

    //DEBUG PRINT SCREEN DIMENSIONS
    cout << "Width  : " << display_width << endl;
    cout << "Height : " << display_height << endl;

    return 0;
}

/*
	EGL stuff
*/

EGLDisplay eglDisplay;
EGLSurface eglSurface;
EGLContext eglContext;

int startEGL() {

    //lets get EGL up and running using the native window
    //we use egl to create a context for OpenVG to use

    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(eglDisplay == EGL_NO_DISPLAY) {
        cout << "Error Creating EGL display!" << endl;
        return 1;
    }

    //initialise EGL
    if(eglInitialize(eglDisplay, NULL, NULL) != EGL_TRUE) {
        cout << "Error Initialising EGL display!" << endl;
        return 2;
    }

    //Create an EGL config for what we want
    EGLConfig config;
    EGLint numConfigs;
    EGLint configs[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_RENDERABLE_TYPE, EGL_OPENVG_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
    };

    //Choose the config to use
    if(eglChooseConfig(eglDisplay, configs, &config, 1, &numConfigs) == EGL_FALSE) {
        cout << "Error With EGL config!" << endl;
        return 3;
    }

    //tell EGL we are using OpenVG
    //IMPORTANT THIS MUST BE DONE BEFORE CREATING THE CONTEXT!
    if(eglBindAPI(EGL_OPENVG_API) != EGL_TRUE) {
        cout << "Error setting api to openvg!" << endl;
        return 7;
    }

    //Now lets create the rendering context
    eglContext = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT, NULL);
    if(eglContext == EGL_NO_CONTEXT) {
        cout << "Error creating context!" << endl;
        return 4;
    }

    //Create the window surface
    eglSurface = eglCreateWindowSurface(eglDisplay, config, &nativewindow, NULL);
    if(eglSurface == EGL_NO_CONTEXT) {
        cout << "Error creating surface!" << endl;
        return 5;
    }

    //make context current
    if(eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext) != EGL_TRUE) {
        cout << "Error making context current!" << endl;
        return 6;
    }

    return 0;
}

/*
	Open VG Setup
*/

int startVG() {
    //set the screen clear colour
    VGfloat color[4] = {0.2f,0.2f,0.2f,1.0f};
    vgSetfv(VG_CLEAR_COLOR, 4, color);

    return 0;
}

/*
	Open VG Drawing
*/

int drawVG() {
    //clear the screen
    vgClear(0, 0, (VGint)display_width, (VGint)display_height);

    /*
        DO ALL THE DRAWING FROM HERE
     */

    return 0;
}

/*
	Main Program Start
*/

int main(int argc, char *argv[]) {

    //start up bcm and check for error
    if(startBCM() != 0) {
        cout << "Error starting BCM!" << endl;
        return 1;
    }

    //setup EGL for our context
    if(startEGL() != 0) {
        cout << "Error starting EGL!" << endl;
        return 2;
    }

    //Start OpenVG
    startVG();

    //Program loop
    int count = 0;

    while(count < 600) {
        //Drawing
        drawVG();

        //swap the buffers
        eglSwapBuffers(eglDisplay, eglSurface);

        //Increment timer thinking 60hz refresh here
        count++;
    }

    //cleanup
    eglDestroySurface(eglDisplay, eglSurface);
    eglDestroyContext(eglDisplay, eglContext);
    eglTerminate(eglDisplay);

    return 0;
}