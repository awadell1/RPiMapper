function[dataStore, robot] = motionControl(Ports, tagNum, maxTime)
% motionControl: Drive around react to walls
%
%   INPUTS
%       CreatePort  Create port object (get from running RoombaInit)
%       SonarPort   Sonar port object (get from running RoombaInit)
%       BeaconPort  Camera port object (get from running RoombaInit)
%       tagNum      robot number for overhead localization
%       maxTime     max time to run program (in seconds)
%
%   OUTPUTS
%       dataStore   struct containing logged data
%
%   NOTE: Assume differential-drive robot whose wheels turn at a constant
%         rate between sensor readings.
%
%   Cornell University
%   MAE 4180/5180 CS 3758: Autonomous Mobile Robots
%   Homework 4
%   Wadell, Alexius


%% Initalize map to logOdds = 0
% Grow map by 5 cm
MapOcc = robot.map.getGrid(10, 10, 0.05);

% Use testCase maxTime if set
if isfield(testCase, 'maxTime')
	robot.maxTime = testCase.maxTime;
end

% Global datastore for compatiability
global dataStore
dataStore = struct('truthPose', [],...
    'odometry', [], ...
    'lidar', [], ...
    'sonar', [], ...
    'bump', [], ...
    'beacon', []);

%% Plot Occupancy Grid as we go
figure
hold on
mapFig = plot(MapOcc);
plot(robot.map)
hold off

%Set State
state = 'drive';

% Start Control loop
noRobotCount = 0; tic
while toc < robot.maxTime
	%& READ & STORE SENSOR DATA
	[dataStore, noRobotCount] = readStoreSensorData(robot, noRobotCount, dataStore);
	
	%% CONTROL FUNCTION
	

	switch state
		case 'drive'
			%Keep Driving
			cmdV = 0.3; cmdW = 0.0;
		case 'backup'
			%Find last bump
			wasBumped = any(dataStore.bump(:,2:end)');
			lastBump = find(wasBumped, 1, 'last');
			
			%Distance Backed up
			deltaD = sum(dataStore.odometry(lastBump+1:end, 2));
			
			%Check for backup completion
			if deltaD < -0.25 %Backup 0.25 m
				%Start Turn
				state = 'turn';
				cmdV = 0.0; cmdW = 0.4*sign(randn+0.5);
			end
		case 'turn'
			%Find last bump
			wasBumped = any(dataStore.bump(:,2:end)');
			lastBump = find(wasBumped, 1, 'last');
			
			%Distance Turned
			deltaA = sum(dataStore.odometry(lastBump+1:end, 3));
			
			%Check for turn completion
			if abs(deltaA) > deg2rad(45)
				%Resume Drive
				state = 'drive';
				cmdV = 0.3; cmdW = -0.0;
			end
	end
	
	% if overhead localization loses the robot for too long, stop it
	if noRobotCount >= 3
		robot.SetFwdVelAngVel(0, 0);
	else
		robot.SetFwdVelAngVel(cmdV, cmdW );
	end
	
	%% Mapping
	
	% Update the map
	truthPose = robot.getData(dataStore, 'truthPose');
	switch testCase.mapper
		case 'sonar'
			sonarReading = robot.getData(dataStore, 'sonar');
			MapOcc = logOddsSonar(robot, sonarReading, truthPose, MapOcc, testCase.prior);
		case 'bump'
			bumpReading = robot.getData(dataStore, 'bump');
			MapOcc = logOddsBump(robot, bumpReading, truthPose, MapOcc, testCase.prior);
		case 'hybrid'
			bumpReading = robot.getData(dataStore, 'bump');
			MapOcc = logOddsSonar(robot, bumpReading, truthPose, MapOcc, testCase.prior);
			
			sonarReading = robot.getData(dataStore, 'sonar');
			MapOcc = logOddsBump(robot, sonarReading, truthPose, MapOcc, testCase.prior);
	end
	
	% Update Plot of Map
	plot(MapOcc, mapFig);
	
	%Add Map to dataStore
	dataStore = robot.setData(dataStore, 'MapOcc', MapOcc);
end

% set forward and angular velocity to zero (stop robot) before exiting the function
robot.SetFwdVelAngVel(0, 0);

