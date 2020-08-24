import pyaudio
import wave
import numpy as np
import time
 
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

print(i)
if(RESPEAKER_INDEX == -1):
        raise ValueError('ReSpeaker Microphone Array Not Found!')

CHUNK = 1024
RECORD_SECONDS = 500
WAVE_OUTPUT_FILENAME = "BackgroundMusic4.wav"
WAVE_OUTPUT_FILENAME_2 = "CoughCheck2.wav"

stream = p.open(
            rate=RESPEAKER_RATE,
            format=p.get_format_from_width(RESPEAKER_WIDTH),
            channels=RESPEAKER_CHANNELS,
            input=True,
            input_device_index=RESPEAKER_INDEX,)

while True:
        
        print("* Recording A")

        frames = []
         
        for i in range(0, int(RESPEAKER_RATE / CHUNK * RECORD_SECONDS)):
                data = stream.read(CHUNK, exception_on_overflow = False)
                # extract channel 0 data from 6 channels
                a = np.frombuffer(data,dtype=np.int16)[1::6]
                frames.append(a.tostring())

        print("* done recording A")

      #  stream.stop_stream()
     #   stream.close()
      #  p.terminate()

        wf = wave.open(WAVE_OUTPUT_FILENAME, 'wb')
        wf.setnchannels(1)
        wf.setsampwidth(p.get_sample_size(p.get_format_from_width(RESPEAKER_WIDTH)))
        wf.setframerate(RESPEAKER_RATE)
        wf.writeframes(b''.join(frames))
        wf.close()
        break
        
        
        

stream.stop_stream()
stream.close()
p.terminate()
        
print("Done!")

