%% segmentation.m

clear; clc;

imageDir = fullfile(pwd, 'daffodilSeg', 'ImagesRsz256');
labelDir = fullfile(pwd, 'daffodilSeg', 'LabelsRsz256');

imds = imageDatastore(imageDir);
classNames = ["background","flower"];
labelIDs   = [0 1];
pxds = pixelLabelDatastore(labelDir, classNames, labelIDs);

% 2. Split into Train (60%), Val (20%), Test (20%)
numFiles = numel(imds.Files);
idx      = randperm(numFiles);

numTrain = round(0.6 * numFiles);
numVal   = round(0.2 * numFiles);

trainIdx = idx(1:numTrain);
valIdx   = idx(numTrain+1 : numTrain+numVal);
testIdx  = idx(numTrain+numVal+1 : end);

imdsTrain = subset(imds,   trainIdx);
pxdsTrain = subset(pxds,   trainIdx);
imdsVal   = subset(imds,   valIdx);
pxdsVal   = subset(pxds,   valIdx);
imdsTest  = subset(imds,   testIdx);
pxdsTest  = subset(pxds,   testIdx);


trainingDS   = combine(imdsTrain, pxdsTrain);
validationDS = combine(imdsVal,   pxdsVal);


inputTileSize = [256 256 3];
numClasses    = 2;
encoderDepth  = 3;

lgraph = unetLayers(inputTileSize, numClasses, ...
                    'EncoderDepth', encoderDepth);


options = trainingOptions('adam', ...
    'InitialLearnRate',    1e-3, ...
    'MaxEpochs',           20, ...
    'MiniBatchSize',       8, ...
    'Shuffle',             'every-epoch', ...
    'ValidationData',      validationDS, ...
    'ValidationFrequency', floor(numel(imdsTrain.Files)/8), ...
    'Verbose',             true, ...
    'Plots',               'training-progress');


net = trainNetwork(trainingDS, lgraph, options);


pxdsValResult = semanticseg(imdsVal, net, ...
                            'MiniBatchSize', 8, ...
                            'Verbose', false);
metricsVal = evaluateSemanticSegmentation(pxdsValResult, pxdsVal, ...
                                          'Verbose', false);
disp('=== Validation Metrics ===')
disp(metricsVal.DataSetMetrics)


pxdsTestResult = semanticseg(imdsTest, net, ...
                             'MiniBatchSize', 8, ...
                             'Verbose', false);
metricsTest = evaluateSemanticSegmentation(pxdsTestResult, pxdsTest, ...
                                           'Verbose', false);
disp('=== Held-Out Test Metrics ===')
disp(metricsTest.DataSetMetrics)


I = readimage(imdsTest,1);
C = semanticseg(I, net);
figure;
subplot(1,2,1); imshow(I);     title('Test Image');
subplot(1,2,2); imshow(labeloverlay(I,C)); title('Predicted Mask');


segmentnet = net; 
save('segmentnet.mat', 'segmentnet');
