close all
% Create Box Map
l = 2;
map = [0 0 l 0;...
    l 0 l l;...
    l l 0 l;...
    0 l 0 0];

%Create Robot
robot = Robot([], 'map', map);
dataStore = struct;

% Place robot in middle of box
pose = [l/2, l/2, 0];
robot.setData(dataStore, 'truthPose', pose);

%Sample Sonars
robot.setData(dataStore, 'sonar', robot.sonarPredict(pose));
sonarRange = robot.getData(dataStore, 'sonar');
fprintf('Sonar: [%f %f %f]\n', sonarRange);

% Get Grid
occMap = robot.map.getGrid(50,50,0.1);

% Find cells in sonar beam
sonarPose = robot.sonarPose(pose, 2);
[inBeam, cellRange] = occMap.hasBeam(sonarPose, deg2rad(45));

%% Run logOddSonar Calcs
% Set Proabilities
P_free = 10;
P_occ = 10;
beta = 0.1;

% For inBeam cells less than the sonar reading -> Assign free
freeCells = P_free * inBeam.data .* (cellRange.data < sonarRange(2));

% For cells at the sonar range
maxRange = cellRange.data <= (beta + sonarRange(2));
minRange = cellRange.data >= sonarRange(2);
occupiedCells = P_occ * inBeam.data .* (maxRange & minRange);

% Plot inBeam
figure
subplot(1,3,1)
plot(inBeam)
plot(robot.map)
title('In Beam')
colorbar off

% Plot Free Cells
fc = cellRange;
fc.data = freeCells;
subplot(1,3,2)
plot(fc)
plot(robot.map)
title('Free')
colorbar off

% Plot Occupied Cells
Oc = cellRange;
Oc.data = occupiedCells;
subplot(1,3,3)
plot(Oc)
plot(robot.map)
title('Occupied')
colorbar off

saveas(gcf, fullfile('img', 'SonarModel'), 'png')
