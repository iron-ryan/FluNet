# FluNet

Main application is in the FluNet folder. Programmed in C++ using QT Creator 5. (NOTE: The microphone array is controlled with Python, need to integrate the two so they can talk to each other).

I have created a folder for misc (mostly its the machine learning stuff in MATLAB).

NNV2.mat contains the trained neural network that needs to be integrated with the system /FluNet/scripts/CoughDetector.py or intrgration with the C++ application whioch might be more efficient (have the inferance in a thread to speed thigns up).

The temperature calibration was done after adddtional flat feild calibration meaning the camera needs to warm up a bit before you get relable temperature readings (still a little high for my liking though).

To use the Neural Compute Stick 2 I have created a script that needs to be executed cd ~ then source start_openvino.sh (initlises the requred openvino varibles and puts you into the virtual enviroment).

Whats left to do is fix the bug with the neural network so it works on the pi properly, the scipt at FluNet/scripts/CoughDectector.py will need opimising when it is complete.


To recreate the issue with the neural network you can load the trained neural network in MATLAB by loading ~/Misc/Neural Network Stuff/NNV2.mat

 exportONNXNetwork(trainedNetwork_4, 'CoughNet.onnx') to convert the nerual net into a onnx file
 
 after openvino installation on a proper machine you can convert this to Intel IR by using the following command
 
 python mo_onnx.py --input_model CoughNet.onnx
 
 Which will give CoughNet.xml and CoughtNet.bin which can be used to deploy inferance on the neural compute stick, when executed you might notice its assigning the same label with the same probability to everything you give it (something might be going on with the weights).
 
 A audio alarm also  needs to added to the system, instead of just a visual one.
 
 
