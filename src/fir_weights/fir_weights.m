clear;
clc;
close all;

N = 32768;
TAPS = 8;
fs = 1000;
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
filename = ['fir_w_',int2str(N),'_',int2str(TAPS),'.dat']
title('Amplitude-Frequency Characteristic');
fp = fopen(filename,'w');
fwrite(fp,lpf,'float32');
fclose(fp);

% weights = load('weights.txt');
% figure;
% subplot(2,1,1);
% stem(weights);
% subplot(2,1,2);
% w_lpf = 20*log(abs(fft(weights)))/log(10);
% w_lpf = w_lpf - max(w_lpf);
% plot(x,w_lpf(1:length(x)));