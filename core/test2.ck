500::ms => dur step;
250::ms => dur half_step;
125::ms => dur quarter_step;

// the patch
adc => dac;


// sound file
"core/kick.wav" => string kick;
"core/bass.wav" => string bass;
"core/snare.wav" => string snare;
"core/clap.wav" => string clap;
"core/hat.wav" => string hat;



SndBuf kck => Gain g_kick => dac;
SndBuf bss => Gain g_bass => dac;
SndBuf snr => Gain g_snare => dac;
SndBuf clp => Gain g_clap => dac;
SndBuf ht => Gain g_hat => dac;


kick => kck.read;
bass => bss.read;
snare => snr.read;
clap => clp.read;
hat => ht.read;

kck.samples() => kck.pos;
bss.samples() => bss.pos;
snr.samples() => snr.pos;
clp.samples() => clp.pos;
ht.samples()  => ht.pos;

.6 => g_kick.gain;
.4 => g_snare.gain;
.4 => g_clap.gain;
.1 => g_hat.gain;
.7 => g_bass.gain;





// Textural Elements





// Phasor
PercFlut flute => NRev r_phas => Gain g_flute => dac;

fun void flute1(float start, float end, float gain, float d) {
    start => float curr_freq;
    d => float dec;
    
    while( true )
    { 
        if (curr_freq < end) {
            start => curr_freq;
        }
        gain => g_flute.gain;
        curr_freq => flute.freq;
        .9 => flute.noteOn;
        step => now;
        4 => flute.noteOff;
        step => now;
        curr_freq - (dec * curr_freq) => curr_freq;
    }
    
}


// Moog

Moog moog => Gain g_moog => dac;

440 => moog.vibratoFreq;
.2 => moog.vibratoGain;
.6 => moog.afterTouch;
.8 => moog.filterQ;


fun void moogie( float base){
 
 while(true) {
     base => moog.freq;
     .75 => moog.noteOn;
     half_step => now;
     base * 1.5 => moog.freq;
     step => now;
     4 => moog.noteOff;
     half_step=> now;
     base => moog.freq;
     .75 => moog.noteOn;
     step => now;
     4 => moog.noteOff;
     step => now;
     base * .5 => moog.freq;
     .3 => moog.noteOn;
     half_step => now;
     base * .5 => moog.freq;
     .1 => moog.noteOn;
     half_step => now;
     step => now;
     base * 2 => moog.freq;
     .75 => moog.noteOn;
     half_step => now;
     base* 1.1 => moog.freq;
     half_step => now;
     step => now;
     4 => moog.noteOff;
 }
    
}

fun void moogie_part2( float base){
    
    while(true) {
        base => moog.freq;
        .75 => moog.noteOn;
        half_step => now;
        2*base => moog.freq;
        step => now;
        base => moog.freq;
        .75 => moog.noteOn;
        step => now;
        2*base => moog.freq;
        .75 => moog.noteOn;
        base => moog.freq;
        .75 => moog.noteOn;
        half_step => now;
        2*base => moog.freq;
        step => now;
        base => moog.freq;
        .75 => moog.noteOn;
        half_step => now;
        2*base => moog.freq;
        half_step => now;
        .85 => moog.noteOn;
         half_step => now;
        .85 => moog.noteOn;
    
    }
    
}

fun void moogie_part3( float base){
    
    while(true) {
        base => moog.freq;
        .75 => moog.noteOn;
        half_step => now;
        2*base => moog.freq;
        step => now;
        base => moog.freq;
        .75 => moog.noteOn;
        step => now;
        
    }
    
}

// HevyMetl

HevyMetl metal => Gain g_metal => dac;
4 => metal.lfoSpeed;
.2 => metal.lfoDepth;

fun void metallic() {
    while (true) {
       .3 => g_metal.gain;
       400 => metal.freq;
       .5 => metal.noteOn;
       4* step => now;
       800 => metal.freq;
       .5 => metal.noteOn;
       4* step => now;
       10 => metal.noteOff;
        8* step => now;
    }
    
}

fun void metallic2() {
    while (true) {
        .3 => g_metal.gain;
        400 => metal.freq;
        .5 => metal.noteOn;
        2* step => now;
        380 => metal.freq;
        .4 => metal.noteOn;
        2* step => now;
        300 => metal.freq;
        .5 => metal.noteOn;
        2* step => now;
        
        10 => metal.noteOff;
        2* step => now;
    }
    
}



// Saw Wave

//SawOsc saw => NRev r_saw => Gain g_saw => dac;

//.5 => r_saw.mix;




// Drums


fun void loop1() 
{
    
    0 => kck.pos;
    step => now;
    step => now;
    0 => clp.pos;
    half_step => now;
    0 => snr.pos;
    half_step => now;
    0 => kck.pos;  
    0 => bss.pos;
    0 => ht.pos;
    step => now;
    0 => kck.pos;
    0 => bss.pos;
    step => now;
    step => now;
    0 => kck.pos;
    step => now;
    0 => snr.pos;
    step => now;
}


fun void loop2() 
{
    
    0 => kck.pos;
    step => now;
    step => now;
    0 => snr.pos;
    half_step => now;
    0 => snr.pos;
    half_step => now;
    0 => bss.pos;
    step => now;
    0 => kck.pos;
    half_step => now;
    0 => clp.pos;
    half_step => now;
    0 => kck.pos;
    0 => bss.pos;
    step => now;
    step => now;
    0 => snr.pos;
    step => now;
}

fun void hatloop1() 
{
    1.5 => ht.rate;
    while(true) {
        0 => ht.pos;
        half_step => now;
}
    
}

fun void hatloop2() 
{
    1.5 => ht.rate;
    while(true) {
        0 => ht.pos;
        half_step => now;
        0 => ht.pos;
        half_step => now;
        0 => ht.pos;
        half_step => now;
        0 => ht.pos;
        quarter_step => now;
         0 => ht.pos;
        quarter_step => now;
        0 => ht.pos;
        half_step => now;
        0 => ht.pos;
        half_step => now;
    }
    
}

fun void drums()
{
// time loop
    while( true )
    {
    //pitch_shifted_noise(100::ms, 0.3, .99);
    //noise(100::ms, 0.3);
    loop1();
    loop2();
    loop1();
    loop2();
    loop1();
    loop1();
    //hatloop1();
    //1::second => now;
    }
}



//SONG STRUCTURE


spork ~ moogie(400) @=> Shred @ s_moog;
spork ~ metallic2() @=> Shred @ s_metallic1;

8::step => now;
spork ~ hatloop2() @=> Shred @ s_hat1;

8::step => now;

spork ~ drums() @=> Shred @ s_drums1;


16::step => now;


spork ~ flute1(800, 400, .25, .2);
spork ~ hatloop1() @=> Shred @ s_hat2;
spork ~ metallic() @=> Shred @ s_metallic2;
s_metallic1.exit();

s_moog.exit();
s_hat1.exit();

16::step => now;

spork ~ metallic2();
s_metallic2.exit();


16::step => now;

spork ~ moogie_part2(400)  @=> Shred @ s_moog2;
s_drums1.exit();
s_hat2.exit();


8::step => now;
spork ~ metallic2();

8::step => now;


16::step => now;
spork ~ drums();
s_moog2.exit();
spork ~ moogie_part3(600);

16::step => now;
spork ~ hatloop1();


16::step => now;
