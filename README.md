# FluNet

Main application is in the FluNet folder. Programmed in C++ using QT Creator 5. (NOTE: The microphone array is controlled with Python, need to integrate the two so they can talk to each other).

I have created a folder for misc (mostly its the machine learning stuff in MATLAB).

NNV2.mat contains the trained neural network that needs to be integrated with the system /FluNet/scripts/CoughDetector.py or intrgration with the C++ application whioch might be more efficient (have the inferance in a thread to speed thigns up).

The temperature calibration was done after adddtional flat feild calibration meaning the camera needs to warm up a bit before you get relable temperature readings (still a little high for my liking though).

To use the Neural Compute Stick 2 I have created a script that needs to be executed cd ~ then source start_openvino.sh (initlises the requred openvino varibles and puts you into the virtual enviroment).

