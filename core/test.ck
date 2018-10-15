SinOsc c => dac;

900 => int n;
800 => int m;
700 => int l;
750 => int k;


while(true) {
    n => c.freq;
    200::ms => now;
    0 => c.gain;
    100::ms => now;
    .5 => c.gain;
    m => c.freq;
    200::ms => now;
    0 => c.gain;
    50::ms => now;
    .5 => c.gain;
     l => c.freq;
    200::ms => now;
    0 => c.gain;
    100::ms => now;
    .5 => c.gain;
    k => c.freq;
    50::ms => now;
    0 => c.gain;
    50::ms => now;
    .5 => c.gain;
}