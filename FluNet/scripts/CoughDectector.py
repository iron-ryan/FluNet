import matplotlib.image
import matplotlib.pyplot as plt
import pyaudio
import wave
import numpy as np
import time
from imutils import paths
import cv2
from scipy.io import wavfile

cv2.__version__

RESPEAKER_RATE = 44100
RESPEAKER_CHANNELS = 6 # change base on firmwares, 1_channel_firmware.bin as 1 or 6_channels_firmware.bin as 6
RESPEAKER_WIDTH = 2

p = pyaudio.PyAudio()
info = p.get_host_api_info_by_index(0)
numdevices = info.get('deviceCount')

# Finds the index of the microphone array
RESPEAKER_INDEX = -1
 
for i in range(0, numdevices):
        if (p.get_device_info_by_host_api_device_index(0, i).get('maxInputChannels')) > 0:
                RESPEAKER_INDEX = i
               
if(RESPEAKER_INDEX == -1):
        raise ValueError('ReSpeaker Microphone Array Not Found!')

CHUNK = 1024
RECORD_SECONDS = 1
WAVE_OUTPUT_FILENAME = "/home/pi/FluNet/CoughCheck1.wav"

stream = p.open(
            rate=RESPEAKER_RATE,
            format=p.get_format_from_width(RESPEAKER_WIDTH),
            channels=RESPEAKER_CHANNELS,
            input=True,
            input_device_index=RESPEAKER_INDEX,)

# Initialise neural network varibles
classes = ["Not Cough", "Cough"]
# Load the cough detection neural network
net = cv2.dnn.readNetFromModelOptimizer ("/home/pi/FluNet/classifiers/NN.xml", "/home/pi/FluNet/classifiers/NN.bin")
# Specify the target device as the Myriad processor on the NCS2
net.setPreferableTarget(cv2.dnn.DNN_TARGET_MYRIAD)

Fs = 8000
NFFT = int(Fs*0.03)
noverlap = int(Fs*0.01)

sent = 0;
while True:
        # Read in a seconds worth of audio
        frames = []
         
        for i in range(0, int(RESPEAKER_RATE / CHUNK * RECORD_SECONDS)):
                data = stream.read(CHUNK, exception_on_overflow = False)
                # extract channel 0 data from 6 channels
                a = np.frombuffer(data,dtype=np.int16)[5::6]
                frames.append(a.tobytes())

        
        
        # Save the array as a .wav file
        wf = wave.open(WAVE_OUTPUT_FILENAME, 'wb')
        wf.setnchannels(1)
        wf.setsampwidth(p.get_sample_size(p.get_format_from_width(RESPEAKER_WIDTH)))
        wf.setframerate(RESPEAKER_RATE)
        wf.writeframes(b''.join(frames))
        wf.close()
        
        # Read in the lastest sound wave
        sample_rate, samples = wavfile.read('/home/pi/FluNet/scripts/CoughCheck1.wav')
             
        # Convert to spectogram
       # frequencies, times, Sx = signal.spectrogram(samples, nfft=NFFT, fs=Fs, noverlap=noverlap)
        spectrogram, frequencies, times, im = plt.specgram(samples, NFFT=NFFT, Fs=Fs, noverlap=noverlap)
        matplotlib.image.imsave('/home/pi/FluNet/spectrogram.png', spectrogram)
        #plt.pcolormesh(times, frequencies, Sx, shading='gouraud')
        #plt.axis('off')
        #plt.savefig('/', bbox_inches='tight');
        #print(datetime.now() - startTime)
        # Use matplotlib to save the spectrogram with the correct colouring
        # matplotlib.image.imsave('/home/pi/IRProject/spectrogram.png', spectrogram);
        
        # Read image back in for classification
        img = cv2.imread('/home/pi/FluNet/spectrogram.png');
                
        blob = cv2.dnn.blobFromImage(img, 1.0, (220, 151), swapRB=True)

        # pass the blob through the network and obtain the detections and
        # predictions
        net.setInput(blob)
        pred = net.forward()
        
        idx = np.argsort(pred[0])[::-1][0]
        text = "{} {:.2f} {}".format(classes[idx],
                pred[0][idx] * 100, sent)
        #print(text)
        
        file_object = open(r"/home/pi/FluNet/coughing.txt","w")
        file_object.write(text)
        file_object.close()

        if sent == 0:
                sent = 1
        else:
                sent = 0
        
        
        
        # Classify
        
        #wf = wave.open(WAVE_OUTPUT_FILENAME, 'wb')
        #wf.setnchannels(1)
        #wf.setsampwidth(p.get_sample_size(p.get_format_from_width(RESPEAKER_WIDTH)))
        #wf.setframerate(RESPEAKER_RATE)
        #wf.writeframes(b''.join(frames))
        #wf.close()
        
        
        

stream.stop_stream()
stream.close()
p.terminate()
        
print("Done!")
