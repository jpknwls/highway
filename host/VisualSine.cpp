//-----------------------------------------------------------------------------
// name: highway.cpp
// desc: hello sine wave, real-time
//
// author: Ge Wang (ge@ccrma.stanford.edu)
//   date: fall 2014
//   uses: RtAudio by Gary Scavone
//-----------------------------------------------------------------------------
#include "RtAudio/RtAudio.h"
#include "chuck.h"
#include <math.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

#ifdef __MACOSX_CORE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

// FFT
#include "chuck_fft.h"



//-----------------------------------------------------------------------------
// function prototypes
//-----------------------------------------------------------------------------
void initGfx();
void idle();
void display();
void reshape( GLsizei width, GLsizei height );
void keyboard( unsigned char, int, int );
void mouse( int button, int state, int x, int y );
double compute_log_spacing( int fft_size, double factor );
void drawHighway();
void drawStars();
void drawMoon();

// our datetype
#define SAMPLE float
// corresponding format for RtAudio
#define MY_FORMAT RTAUDIO_FLOAT32
// sample rate
#define MY_SRATE 44100
// number of channels
#define MY_CHANNELS 2


//waterfall
#define SND_BUFFER_SIZE 1024
#define SND_FFT_SIZE ( SND_BUFFER_SIZE * 2 )

//stars
#define NUMBER_STARS SND_BUFFER_SIZE

// viewing
long g_width = 1024;
long g_height = 720;
GLint g_freq_view = 2;

// global buffer
SAMPLE * g_buffer = NULL;
long g_bufferSize;

// global variables
bool g_draw_dB = false;
ChucK * the_chuck = NULL;

//WATERFALL
struct Pt2D { float x; float y; };
Pt2D ** g_spectrums = NULL;
GLuint g_depth = 72; // for john: 64
GLfloat g_z = 0.0f;
GLboolean g_z_set = FALSE;
GLfloat g_space = .12f; // for john: .1f


GLboolean g_downsample = FALSE;
GLint g_ds = 0; // downsample amount
GLint g_fft_size = SND_FFT_SIZE; //
GLint g_wf = 0; //index associated with waterfall
GLboolean g_wutrfall = TRUE; // plot waterfall
GLboolean * g_draw = NULL; // array of booleans for waterfall
GLfloat g_log_positions[SND_FFT_SIZE/2]; // precompute positions for log spacing
GLdouble g_log_space = 0;
GLdouble g_log_factor = 1;

// for time domain waterfall
SAMPLE ** g_waveforms = NULL;
SAMPLE g_fft_buffer[SND_FFT_SIZE];
GLfloat g_wf_delay_ratio = 1.0f / 2.0f;
GLuint g_wf_delay = (GLuint)(g_depth * g_wf_delay_ratio);
GLuint g_wf_index = 0;

// gain
GLfloat g_gain = 1.0f;
GLfloat g_time_scale = 1.0f;
GLfloat g_freq_scale = 1.0f;

//COLORS
GLfloat g_color_mult_1 = 0.0f;
GLfloat g_color_mult_2 = .3f;
GLboolean g_color_slide = FALSE;
GLfloat g_color_slide_rate = .0005;


GLfloat g_bounce_bound = .5;
GLfloat g_bounce_factor = -g_bounce_bound;
GLfloat g_bounce_inc = 1.009;
long neg_bounce_inc = .991;
long pos_bounce_inc = 1.009;



//MOON
GLfloat g_rotate_angle = 1;


//-----------------------------------------------------------------------------
// Name: Star Class
// Desc: defines a single star entity
//----------------------------------------------------------------------------

class Star {
    float x, y, z, brightness, color;

public:
    Star() { };
    Star (float a, float b, float c){
        x = a;
        y = b;
        z = c;
        
    }
    void set_color(float c) { color = c; }
    void set_brightness(float b){ brightness = b; }
    void increase_z(float i) {z = z+i;}
    void draw(){
        //X
        glLineWidth(.1f);
        glColor3ub(color,color, color);

        glBegin( GL_LINE_STRIP );
        glVertex3f(x+10,y,z);
        glVertex3f(x-10,y,z);
        glEnd();
        //Y
        glBegin( GL_LINE_STRIP );
        glVertex3f(x,y+.05,z);
        glVertex3f(x,y-.05,z);
        glEnd();
        //Z
        glBegin( GL_LINE_STRIP );
        glVertex3f(x,y,z+1);
        glVertex3f(x,y,z-1);
        glEnd();
    }

    
};

Star g_stars[NUMBER_STARS];

//-----------------------------------------------------------------------------
// name: callme()
// desc: audio callback
//-----------------------------------------------------------------------------
int callme( void * outputBuffer, void * inputBuffer, unsigned int numFrames,
            double streamTime, RtAudioStreamStatus status, void * data )
{
    // cast!
    SAMPLE * input = (SAMPLE *)inputBuffer;
    SAMPLE * output = (SAMPLE *)outputBuffer;
    
    // compute chuck!
    // (TODO: to fill in)
   the_chuck->run(input, output, numFrames);
    
    // fill
    for( int i = 0; i < numFrames; i++ )
    {
        // copy the input to visualize only the left-most channel
        if (the_chuck->running()) {
            g_buffer[i] = output[i*MY_CHANNELS];
        } else {
            g_buffer[i] = input[i*MY_CHANNELS];
        }
        
        // also copy in the output from chuck to our visualizer

        // (TODO: to fill in)
        
        // mute output -- TODO will need to disable this once ChucK produces output, in order for you to hear it!
        //for( int j = 0; j < MY_CHANNELS; j++ ) { output[i*MY_CHANNELS + j] = 0; }
    }
    
    
    
    return 0;
}




//-----------------------------------------------------------------------------
// name: main()
// desc: entry point
//-----------------------------------------------------------------------------
int main( int argc, char ** argv )
{
    // instantiate RtAudio object
    RtAudio audio;
    // variables
    unsigned int bufferBytes = 0;
    // frame size
    unsigned int bufferFrames = 1024;
    
    // check for audio devices
    if( audio.getDeviceCount() < 1 )
    {
        // nopes
        cout << "no audio devices found!" << endl;
        exit( 1 );
    }
    
    // initialize GLUT
    glutInit( &argc, argv );
    // init gfx
    initGfx();
    
    // let RtAudio print messages to stderr.
    audio.showWarnings( true );
    
    // set input and output parameters
    RtAudio::StreamParameters iParams, oParams;
    iParams.deviceId = audio.getDefaultInputDevice();
    iParams.nChannels = MY_CHANNELS;
    iParams.firstChannel = 0;
    oParams.deviceId = audio.getDefaultOutputDevice();
    oParams.nChannels = MY_CHANNELS;
    oParams.firstChannel = 0;
    
    // create stream options
    RtAudio::StreamOptions options;
    
    // go for it
    try {
        // open a stream
        audio.openStream( &oParams, &iParams, MY_FORMAT, MY_SRATE, &bufferFrames, &callme, (void *)&bufferBytes, &options );
    }
    catch( RtError& e )
    {
        // error!
        cout << e.getMessage() << endl;
        exit( 1 );
    }
    
    // compute
    bufferBytes = bufferFrames * MY_CHANNELS * sizeof(SAMPLE);
    // allocate global buffer
    g_bufferSize = bufferFrames;
    g_buffer = new SAMPLE[g_bufferSize];
    memset( g_buffer, 0, sizeof(SAMPLE)*g_bufferSize );
    
    // set up chuck
    the_chuck = new ChucK();
    the_chuck->setParam("SAMPLE_RATE", MY_SRATE);
    the_chuck->setParam("INPUT_CHANNELS",MY_CHANNELS );
    the_chuck->setParam("OUTPUT_CHANNELS",MY_CHANNELS );
    // TODO: set sample rate and number of in/out channels on our chuck

    // TODO: initialize our chuck
    the_chuck->init();
    //TODO: run a chuck program
    the_chuck->compileFile("core/test2.ck","", 1);
   
    // go for it
    try {
        // start stream
        audio.startStream();
        
        // let GLUT handle the current thread from here
        glutMainLoop();
        
        // stop the stream.
        audio.stopStream();
    }
    catch( RtError& e )
    {
        // print error message
        cout << e.getMessage() << endl;
        goto cleanup;
    }
    
cleanup:
    // close if open
    if( audio.isStreamOpen() )
        audio.closeStream();
    
    // done
    return 0;
}




//-----------------------------------------------------------------------------
// Name: reshapeFunc( )
// Desc: called when window size changes
//-----------------------------------------------------------------------------
void initGfx()
{
    // double buffer, use rgb color, enable depth buffer
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
    // initialize the window size
    glutInitWindowSize( g_width, g_height );
    // set the window postion
    glutInitWindowPosition( 100, 100 );
    // create the window
    glutCreateWindow( "highway" );
    
    // set the idle function - called when idle
    glutIdleFunc( idle );
    // set the display function - called when redrawing
    glutDisplayFunc( display );
    // set the reshape function - called when client area changes
    glutReshapeFunc( reshape );
    // set the keyboard function - called on keyboard events
    glutKeyboardFunc( keyboard );
    // set the mouse function - called on mouse stuff
    glutMouseFunc( mouse );
    
    // set clear color
    glClearColor( 0, 0, 0, 1 );
    // enable color material
    glEnable( GL_COLOR_MATERIAL );
    // enable depth test
    glEnable( GL_DEPTH_TEST );

    // initialize
    g_spectrums = new Pt2D *[g_depth];
    for( int i = 0; i < g_depth; i++ )
    {
        g_spectrums[i] = new Pt2D[SND_FFT_SIZE];
        memset( g_spectrums[i], 0, sizeof(Pt2D)*SND_FFT_SIZE );
    }
    g_draw = new GLboolean[g_depth];
    memset( g_draw, 0, sizeof(GLboolean)*g_depth );

    g_log_space = compute_log_spacing( g_fft_size / 2, g_log_factor );


    for( int i = 0; i < NUMBER_STARS; i++ ) {
        float x = -500 + ((float) rand()/RAND_MAX) * (g_width*3  );
        float y = 3 * ((float) rand()/RAND_MAX);
        float z = 50 + ((float) rand()/RAND_MAX) * 50;
        Star star = Star(x,y,z);
        g_stars[i] = star;
    }

}




//-----------------------------------------------------------------------------
// Name: reshape( )
// Desc: called when window size changes
//-----------------------------------------------------------------------------
void reshape( GLsizei w, GLsizei h )
{
    // save the new window size
    g_width = w; g_height = h;
    // map the view port to the client area
    glViewport( 0, 0, w, h );
    // set the matrix mode to project
    glMatrixMode( GL_PROJECTION );
    // load the identity matrix
    glLoadIdentity( );
    // create the viewing frustum
    gluPerspective( 20.5, (GLfloat) w / (GLfloat) h, 1.0, 250.0 );
    // set the matrix mode to modelview
    glMatrixMode( GL_MODELVIEW );
    // load the identity matrix
    glLoadIdentity( );
    // position the view point
    gluLookAt( 0.0f, 1.50f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f );
     // compute log spacing
    g_log_space = compute_log_spacing( g_fft_size / 2, g_log_factor );
}




//-----------------------------------------------------------------------------
// Name: keyboard( )
// Desc: key event
//-----------------------------------------------------------------------------
void keyboard( unsigned char key, int x, int y )
{
    switch( key )
    {
        case '1':
            g_color_slide = FALSE;
            g_color_mult_1 = .45f;
            g_color_mult_2 = .45f;
            break;

        case '2':
            g_color_slide = FALSE;
            g_color_mult_1 = .08f;
            g_color_mult_2 = .55f;
            break;

        case '3':
            g_color_slide = FALSE;
            g_color_mult_1 = .97f;
            g_color_mult_2 = .000001f;
            break;
        case '4':
            g_color_slide = FALSE;
            g_color_mult_1 = .79f;
            g_color_mult_2 = .94f;
            break;
        case '5':
            g_color_slide = TRUE;
            g_color_mult_1 = .7f;
            g_color_mult_2 = .3f;
            break;
        case 'q':
            exit(1);
            break;
            
        case 'd':
            g_draw_dB = !g_draw_dB;
            break;
    }
    
    glutPostRedisplay( );
}




//-----------------------------------------------------------------------------
// Name: mouse( )
// Desc: handles mouse stuff
//-----------------------------------------------------------------------------
void mouse( int button, int state, int x, int y )
{
    if( button == GLUT_LEFT_BUTTON )
    {
        // when left mouse button is down
        if( state == GLUT_DOWN )
        {
        }
        else
        {
        }
    }
    else if ( button == GLUT_RIGHT_BUTTON )
    {
        // when right mouse button down
        if( state == GLUT_DOWN )
        {
        }
        else
        {
        }
    }
    else
    {
    }
    
    glutPostRedisplay( );
}




//-----------------------------------------------------------------------------
// Name: idle( )
// Desc: callback from GLUT
//-----------------------------------------------------------------------------
void idle( )
{
    // render the scene
    glutPostRedisplay( );
}

void bounce (float bounce_factor )
{
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );
    gluLookAt( 0.0f, 1.50f, 10.0f, 0.0f, 0.0f, 0.0f + bounce_factor, 0.0f+ bounce_factor/8.0, 1.0f, 0.0f );
}

//-----------------------------------------------------------------------------
// Name: display( )
// Desc: callback function invoked to draw the client area
//-----------------------------------------------------------------------------
void display( )
{
    // local state
    static GLfloat zrot = 0.0f, c = 0.0f;

    
    g_bounce_factor *= g_bounce_inc;
    if (g_bounce_factor > g_bounce_bound) {

        g_bounce_inc = neg_bounce_inc;
    }
    if (g_bounce_factor < -g_bounce_bound) {
        g_bounce_inc = pos_bounce_inc;
    }
    //bounce(g_bounce_factor);  
    // clear the color and depth buffers
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glPushMatrix();
    drawMoon();
    
    glPopMatrix();


    

    glPushMatrix();
    // color
        glColor3f( .5, 1, .5 );
        // save current matrix state
        glPushMatrix();
            drawHighway();
            glPushMatrix();
                //drawStars();
                glPushMatrix();
                    

                glPopMatrix();
            glPopMatrix();
        glPopMatrix();
    glPopMatrix();  

    
    // flush!
                    
  

    glFlush( );
    // swap the double buffer
    glutSwapBuffers( );
}

void drawMoon() {

    
    //loop over buffer
    GLfloat x = -5;
    GLfloat angle_inc = 180.0 / g_bufferSize;
    GLfloat rot = 0.0;
    GLfloat xinc = ::fabs(4*x/(g_bufferSize));
    glLineWidth(5.0f);




    for( int i = 0; i < g_bufferSize; i++ )
    {
        if (g_buffer[i] < .005) { continue; }

        
        glColor4f( 0.5, 0.5, 0.5, .4 ); 

        if (g_buffer[i] > .035) { glColor4f( 1,1,1, 1); }

        GLfloat z_pos =  -10 - 10 * ((float) rand()/RAND_MAX);


        //DRAW EACH BAR
        glPushMatrix();

            glBegin(GL_POLYGON);

            GLfloat curr = pow(g_buffer[i], .9 );

            glVertex3f( x, 2*curr, z_pos );
            glVertex3f( x, curr, z_pos );
            glVertex3f( x*1.01, curr, z_pos );
            glVertex3f( x*1.01, 2*curr,z_pos );  

            glEnd();

            //ROTATE
        
        glPopMatrix();

        GLfloat x_pos =  2 *  ((float) rand()/RAND_MAX);
        GLfloat y_pos =  2 * ((float) rand()/RAND_MAX);



        glRotatef(87.5, 0.0f, 0.0f, 1.0f);
        
  

        //UPDATE PARAMETERS
        rot += angle_inc;
        x += xinc;
    }

        glColor3f( 0.0, 0.0, 0.0);
        glBegin(GL_POLYGON);

        glVertex3f( 0, 1, 15 );
        glVertex3f( 0, -10, 15 );
        glVertex3f( g_width*2, -10, 15);
        glVertex3f( g_width*2, 1,15 );  

        glEnd();
    //glTranslatef( 0.0f, 0.0f, -20.0f);
  

}

void drawStars()
{
    for( GLint i = 0; i < g_depth; i++ )
            { 
                for (int j = 0; j < g_bufferSize; j++) {

                    Star curr = g_stars[j];
                    curr.increase_z(float(i)/2.0);
                    if (fabs(g_buffer[j]) > .02) {
                        curr.set_color(fabs(g_buffer[j]) * 3000);
                    } else { 
                        if (fabs(g_buffer[j]) < .005) {
                           curr.set_color(0); 
                        }
                         
                    }
                    curr.draw();

                }
                
            }
       
}

void drawHighway() {
        GLfloat ytemp, fval;
        SAMPLE * buffer = g_fft_buffer;

        // get the latest (possibly preview) window
        memset( buffer, 0, SND_FFT_SIZE * sizeof(SAMPLE) );

        // copy currently playing audio into buffer
        memcpy( buffer, g_buffer, SND_BUFFER_SIZE * sizeof(SAMPLE) );

        // take forward FFT; result in buffer as FFT_SIZE/2 complex values
        rfft( (float *)buffer, g_fft_size/2, FFT_FORWARD );
        // cast to complex
        complex * cbuf = (complex *)buffer;
       
        // reset drawing offsets
        GLfloat x = -3.8f;
        GLfloat y = -1.0f;
        GLfloat inc = 3.6f / g_fft_size;

        // color the spectrum
        glColor3f( 0.4f, 1.0f, 0.4f );
        // set vertex normals
        glNormal3f( 0.0f, 1.0f, 0.0f );

        // copy current magnitude spectrum into waterfall memory
        for( GLint i = 0; i < g_fft_size/2; i++ )
        {
            // copy x coordinate
            g_spectrums[g_wf][i].x = x;
            g_spectrums[g_wf][i].y = g_gain * g_freq_scale * 
                    ( 20.0f * log10( cmp_abs(cbuf[i])/8.0 ) + 80.0f ) / 80.0f + y + .5f;
            // increment x
            x += inc * g_freq_view;
        }

        // draw the right things
        g_draw[g_wf] = g_wutrfall;

        // reset drawing variables
        x = -3.8f;
        
        if (g_color_slide) {
            g_color_mult_1 = g_color_mult_1 + (g_color_slide_rate * g_color_mult_1);
            g_color_mult_2 = g_color_mult_2 - (g_color_slide_rate * g_color_mult_2);
            if (g_color_mult_1 > 1.3 || g_color_mult_1 < -.3 || g_color_mult_2 > 1.3 || g_color_mult_2 < -.3) {
                g_color_slide_rate = -g_color_slide_rate;
            }
        }
    
        // translate in world coordinate
        glTranslatef( x, -1.0, g_z );
        // scale it
        glScalef( inc*g_freq_view , 1.0 , -g_space );
        // loop through each layer of waterfall
        for( GLint i = 0; i < g_depth; i++ )
        {
            if( i == g_wf_delay || g_wutrfall )
            {
                // if layer is flagged for draw
                if( g_draw[(g_wf+i)%g_depth] )
                {
                    // get the magnitude spectrum of layer
                    Pt2D * pt = g_spectrums[(g_wf+i)%g_depth];
                    // future
                    // brightness based on depth
                    //fval = (g_depth - i + g_wf_delay) / (float)(g_depth);
                    fval = (g_depth - i + g_wf_delay) / (float)(g_depth);
                    if( i < g_wf_delay )
                    {
                        float cval = 1 - i / (float)(g_wf_delay);
                        cval = g_color_mult_1  + cval * (g_color_mult_2 - g_color_mult_1);
                        glColor3f( g_color_mult_1 * fval , cval * fval, g_color_mult_2 * fval);
                    } else {
                        float cval = 1 - ((i) + g_wf_delay) / (float)(g_depth - g_wf_delay);
                        cval = g_color_mult_1 + cval * (g_color_mult_2 - g_color_mult_1);
                        glColor3f(g_color_mult_1 * fval,  cval * fval, g_color_mult_2 * fval);  
                    }

                    // float fval = (g_depth - i ) / (float)(g_depth);
                    // float slew = .3;                
                    // glColor3f( 0.0f + slew*fval, 1.0f - slew*fval, 0.0 + fval);    

                    // render the actual spectrum layer
                    glBegin( GL_LINE_STRIP );
                    float end_loc;
                    for( GLint j = 0; j < g_fft_size/g_freq_view; j++, pt++ )
                    {
                        // // draw the vertex
                        float d =  g_depth - (float) i;
                       // if (i > g_wf_delay) {d*=.95;} 
                        end_loc = g_log_positions[j];
                        float y = pt->y;

                        if (i < 10) {
                            y = 1.1 * y + .25;
                        }
                        glVertex3f( g_log_positions[j],   y,  d );
                    }

                    pt--;
                    for( GLint j = 0; j < g_fft_size/g_freq_view; j++, pt--  )
                    {
                        // draw the vertex

                        float d =  g_depth - (float) i;
                        //if (i > g_wf_delay) {d*=.95;}
                        float y = pt->y;

                        if (i < 10) {
                            y = 1.1 * y + .25;
                        }                    
                        glVertex3f(end_loc +  g_log_positions[j],  y,  d );
                    }

                    glEnd();

                    // back to default line width
                    glLineWidth(1.0f);
                }
            }
        }
     
        // wtrfll
      
        // advance index
        g_wf--;
        // mod
        g_wf = (g_wf + g_depth) % g_depth; 

}

//-----------------------------------------------------------------------------
// Name: map_log_spacing( )
// Desc: ...
//-----------------------------------------------------------------------------
inline double map_log_spacing( double ratio, double power )
{
    // compute location
    return ::pow(ratio, power) * g_fft_size/g_freq_view; 
}

//-----------------------------------------------------------------------------
// Name: compute_log_spacing( )
// Desc: ...
//-----------------------------------------------------------------------------
double compute_log_spacing( int fft_size, double power )
{
    int maxbin = fft_size; // for future in case we want to draw smaller range
    int minbin = 0; // what about adding this one?

    for(int i = 0; i < fft_size; i++)
    {
        // compute location
        g_log_positions[i] = map_log_spacing( (double)i/fft_size, power ); 
        // normalize, 1 if maxbin == fft_size
        g_log_positions[i] /= pow((double)maxbin/fft_size, power);
    }

    return 1/::log(fft_size);
}


