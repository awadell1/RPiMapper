% Default Settings for Mapping
pose0 = [0.5 , -0.5, pi/4];
mapfile = 'loopmap.txt';
load('debugData.mat')

%% Sonar 25x25 Grid
global testCase
filename = 'sonar_25x25';
testCase = struct('mapper', 'sonar',...
    'm', 25, 'n', 25,....
    'map', 'loopMap.mat',...
    'prior', 0,...
    'dataStore', debugData);

%Run Simulator
%robot = fastSim(pose0, @motionControl, 'map', mapfile);
[dataStore, robot] = motionControl;

% Plot Occupancy Grid at 3 timesteps
plotOccupancyGrid3(robot, dataStore, filename)

%Plot Bump locations
plotBumpTrajectory(robot, dataStore, filename)

% Plot Thresholded Data
plotOccupancyThreshold(robot, dataStore, filename)

%Plot Bump locations
plotBumpTrajectory(robot, dataStore, filename)

% Plot Thresholded Data
plotOccupancyThreshold(robot, dataStore, filename)

%% Sonar 50x50 Grid
global testCase
filename = 'sonar_50x50';
testCase = struct('mapper', 'sonar',...
   'm', 50, 'n', 50,....
   'map', 'loopMap.mat',...
   'prior', 0,...
   'dataStore', debugData);

%Run Simulator
%robot = fastSim(pose0, @motionControl, 'map', mapfile);
[dataStore, robot] = motionControl;

% Plot Occupancy Grid at 3 timesteps
plotOccupancyGrid3(robot, dataStore, filename)

%Plot Bump locations
plotBumpTrajectory(robot, dataStore, filename)

% Plot Thresholded Data
plotOccupancyThreshold(robot, dataStore, filename)