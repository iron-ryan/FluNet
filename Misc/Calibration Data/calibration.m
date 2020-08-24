clc; close all; clear all;

d = dir('calibration_images');

seekVals = zeros(size(d, 1)-2, 1);
tempVals = zeros(size(d, 1)-2, 1);

for i = 3:size(d, 1)
    img = load(fullfile(strcat('calibration_images\', d(i).name)));
    seekVals(i-2) = max(max(img));
    
    t = strsplit(d(i).name, '.');
    t = strsplit(t{1}, '_');
    
    tempVals(i-2) = str2num(strcat(t{1}, '.', t{2}));
end

tempVals = tempVals + 273.15;

plot(seekVals, tempVals, '.');