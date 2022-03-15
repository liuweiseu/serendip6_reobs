clear;
clc;
close all;

N = 32768;
TAPS = 4;
fs = 1024;
f= fs/N;

wn = f*2/fs;
ft = 'LOW';
w = 'Hanning';

lpf = fir1(TAPS*N-1,wn,ft,hanning(TAPS*N));

subplot(2,1,1);
plot(lpf);
title('impulse response ');

subplot(2,1,2);
m_lpf = 20*log(abs(fft(lpf)))/log(10);
x = 0:(fs/length(lpf)):fs/2;
plot(x,m_lpf(1:length(x)));
title('Amplitude-Frequency Characteristic');
fp = fopen('matlab_fir_weights.dat','w');
fwrite(fp,lpf,'float32');
fclose(fp);
