%% classification.m

clear; clc;

rootDir = fullfile(pwd, '17flowers');
imds    = imageDatastore(rootDir, ...
            'IncludeSubfolders', true, ...
            'LabelSource', 'foldernames');

% 2. Split into Train+Val (80%) and Test (20%)
[imdsTrainVal, imdsTest] = splitEachLabel(imds, 0.8, 'randomized');

% 3. From Train+Val, carve out Train (75%) and Val (25%)
[imdsTrain, imdsVal] = splitEachLabel(imdsTrainVal, 0.75, 'randomized');


inputSize  = [256 256 3];
numClasses = numel(categories(imdsTrain.Labels));

layers = [
    imageInputLayer(inputSize,'Name','in')

    convolution2dLayer(5,32,'Padding','same','Name','conv1')
    batchNormalizationLayer('Name','bn1')
    reluLayer('Name','relu1')
    maxPooling2dLayer(2,'Stride',2,'Name','pool1')

    convolution2dLayer(3,64,'Padding','same','Name','conv2')
    batchNormalizationLayer('Name','bn2')
    reluLayer('Name','relu2')
    maxPooling2dLayer(2,'Stride',2,'Name','pool2')

    convolution2dLayer(3,128,'Padding','same','Name','conv3')
    batchNormalizationLayer('Name','bn3')
    reluLayer('Name','relu3')
    maxPooling2dLayer(2,'Stride',2,'Name','pool3')

    fullyConnectedLayer(512,'Name','fc1')
    reluLayer('Name','relu4')
    dropoutLayer(0.5,'Name','drop1')

    fullyConnectedLayer(numClasses,'Name','fc2')
    softmaxLayer('Name','soft')
    classificationLayer('Name','classoutput')
];

lgraph = layerGraph(layers);


augImdsTrain = augmentedImageDatastore(inputSize, imdsTrain);
augImdsVal   = augmentedImageDatastore(inputSize, imdsVal);
augImdsTest  = augmentedImageDatastore(inputSize, imdsTest);


options = trainingOptions('sgdm', ...
    'InitialLearnRate',1e-3, ...
    'MaxEpochs',15, ...
    'MiniBatchSize',32, ...
    'Shuffle','every-epoch', ...
    'ValidationData',augImdsVal, ...
    'ValidationFrequency',floor(numel(imdsTrain.Files)/32), ...
    'Verbose',true, ...
    'Plots','training-progress');


net = trainNetwork(augImdsTrain, lgraph, options);


YPredVal = classify(net, augImdsVal);
YVal     = imdsVal.Labels;
valAcc   = mean(YPredVal == YVal)*100;
fprintf('Validation accuracy: %.2f%%\n', valAcc);


YPredTest = classify(net, augImdsTest);
YTest     = imdsTest.Labels;
testAcc   = mean(YPredTest == YTest)*100;
fprintf('Test (unseen) accuracy: %.2f%%\n', testAcc);


figure;
confusionchart(YTest, YPredTest);
title('Test-set Confusion Matrix');


classnet = net; 
save('classnet.mat','classnet');
