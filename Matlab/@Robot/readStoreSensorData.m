function [dataStore, noRobotCount] = readStoreSensorData(robot, noRobotCount, dataStore, BeaconMode)
% This function tries to read all the sensor information from the Create
% and store it in a data structure
%
%   [noRobotCount,dataStore]=readStoreSensorData(robot, noRobotCount, dataStore)
%
%   INPUTS
%				robot         A Robot Class object -> Contains Ports and tagNum of the CreateRobot
%				noRobotCount	Number of consecutive times the robot was "lost" by the overhead localization
%				dataStore			struct containing logged data
%				BeaconMode		1:(DEFAULT) AR Tag Mode [time, tag number, X_camera, Y_camera, Z_camera, rot_camera]
%											0: Blob Detection Mode [time, [3 element color vector], bearing, range]
%
%   OUTPUTS
%				noRobotCount	Updated number of consecutive times the robot was "lost" by the overhead localization
%				dataStore			Updated struct containing logged data
%
%   Cornell University
%   MAE 5180: Autonomous Mobile Robots

%Extract Ports from robot object
CreatePort = robot.createPort;
SonarPort = robot.sonarPort;
BeaconPort = robot.beaconPort;
tagNum = robot.tagNum;

% Set Beacon Mode
BeaconMode = 1;

%Note Start Time
try
	startTime = toc;
catch
	tic;
	startTime = toc;
end

% Check if the Robot is in debugMode
if ~robot.debugMode
	%% read truth pose (from overhead localization system)
	try
		[px, py, pt] = OverheadLocalizationCreate(tagNum);
		if (px == 0 && py == 0 && pt == 0 && ~isa(tagNum,'CreateRobot'))
			disp('Overhead localization lost the robot!')
			noRobotCount = noRobotCount + 1;
		else
			poseX = px; poseY = py; poseTheta = pt;
			dataStore.truthPose = [dataStore.truthPose ; ...
				toc poseX poseY poseTheta];
			noRobotCount = 0;
		end
	catch
		disp('Error retrieving or saving overhead localization data.');
	end

	cmd = {'GOM', 'GRM', 'GIM'};
	name = {'odometry', 'range', 'IMU'};
	for i = 1:length(cmd)
		% read sensor data from robot
		try
			[resp, time] = robot.sendPacket(cmd{i});
			
			% Parse resp
			data = sscanf(resp, '%f');
			
			% Append to dataStore
			dataStore.(name{i}) = [dataStore.(name{i}) ; ...
				time data'];
		catch
			disp('Error retrieving or saving odometry data.');
		end
	end
else
	%% Debug Mode on -> Read from supplied datastore
	
	sensors = {'truthPose', 'odometry', 'lidar', 'sonar', 'bump', 'beacon'};
	
	% Find Cycle Time Bounds
	mTime = robot.debugData.mTime(robot.debugIndex, 2:end);
	if robot.debugIndex < size(robot.debugData.mTime)
		mTime(2) = robot.debugData.mTime(robot.debugIndex+1, 2);
	end
	
	for s = 1:length(sensors)
		if isfield(robot.debugData, sensors{s})
			% Try to read each sensor
			dataLength = size(robot.debugData.(sensors{s}),2)-1;
			
			try
				% Check Valid Time Range
				time = robot.debugData.(sensors{s})(:,1);
				
				% Pull debugData
				data = robot.debugData.(sensors{s})(time <= mTime(2) & time > mTime(1), :);
				
				
				% Drop time stamp
				data(1) = [];
				
				if isempty(data)
					data = nan(1, dataLength);
				end
			catch
				data = nan(1, dataLength);
			end
		else
			data = nan(1, dataLength);
		end
		
		% Log sonar reading to dataStore
		dataStore = robot.setData(dataStore, sensors{s}, data);
	end
	
	% Iterate debugIndex
	robot.debugIndex = robot.debugIndex +1;
	
	% Check for end of data
	if robot.debugIndex > size(robot.debugData.mTime,1)
		% End control loop
		robot.maxTime = 0;
	end
end

% Note End Time
endTime = toc;
dataStore = robot.setData(dataStore, 'mTime', [startTime, endTime]);
