close all
% Default Settings for Mapping
pose0 = [0.5 , -0.5, pi/4];


%% Bump 25x25 Grid
filename = 'bump_25x25';
global testCase
testCase = struct('mapper', 'bump',...
    'm', 12, 'n', 12,...
		'x', [-2.5, 2.5],...
		'y', [-1.5, 1.5],...
    'maxTime', 120,...
    'map', 'labmat.mat',...
		'prior', 0,...
    'dataStore', debugData);

%Run Simulator
[dataStore, robot] = fastSim(pose0, @motionControl, 'speed', 3);

% Plot Occupancy Grid at 3 timesteps
plotOccupancyGrid3(robot, dataStore, filename)

%Plot Bump locations
plotBumpTrajectory(robot, dataStore, filename)

% Plot Thresholded Data
plotOccupancyThreshold(robot, dataStore, filename)

%% Bump 50x50 Grid
filename = 'bump_50x50';
global testCase
testCase = struct('mapper', 'bump',...
    'm', 50, 'n', 120,....
    'map', 'loopMap.mat',...
    'prior', 0,...
    'dataStore', debugData);

%Run Simulator
%robot = fastSim(pose0, @motionControl, 'map', mapfile, 'speed', 3);
[dataStore, robot] = motionControl;

% Plot Occupancy Grid at 3 timesteps
plotOccupancyGrid3(robot, dataStore, filename)

%Plot Bump locations
plotBumpTrajectory(robot, dataStore, filename)

% Plot Thresholded Data
plotOccupancyThreshold(robot, dataStore, filename)
