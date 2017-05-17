% Default Settings for Mapping
pose0 = [0.5 , -0.5, pi/4];
mapfile = 'loopmap.txt';


%% Hybrid 25x25 Grid
filename = 'hybrid_25x25';
global testCase
testCase = struct('mapper', 'hybrid',...
    'm', 25, 'n', 25,...
    'maxTime', 120,...
    'map', 'loopMap.mat');

%Run Simulator
robot = fastSim(pose0, @motionControl, 'map', mapfile, 'speed', 3);

% Plot Occupancy Grid at 3 timesteps
plotOccupancyGrid(robot, filename)

%Plot Bump locations
plotBumpTrajectory(robot, filename)

%% Hybrid 500x500 Grid
filename = 'hybrid_500x500';
global testCase
testCase = struct('mapper', 'hybrid',...
    'm', 500, 'n', 500,...
    'maxTime', 120,...
    'map', 'loopMap.mat');

%Run Simulator
robot = fastSim(pose0, @motionControl, 'map', mapfile, 'speed', 3);

% Plot Occupancy Grid at 3 timesteps
plotOccupancyGrid(robot, filename)

%Plot Bump locations
plotBumpTrajectory(robot, filename)