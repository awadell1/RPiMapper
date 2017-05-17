classdef Robot < handle
	% Robot: Class for interacting with the Create Robot and dataStore
	%
	%   Cornell University
	%   MAE 4180/5180 CS 3758: Autonomous Mobile Robots
	%   Homework 4
	%   Wadell, Alexius
	
	% Properties that do not change
	% All units are kg/m/s/rad unless otherwise noted
	properties(Constant)

		%% Physical Parameters of the Create Robot
		radius = 0.16;				% radius of the Create Robot
		wheel2center = 0.05;		% Distance between center of robot and wheels
		maxVelocity = 0.20;			% Sets the max speed of the robot [m/s]
		drive_epsilon = 0.15;		% Tuning Parameter for feedback linearization
		fwdVel_std = 0.01;			% Std. dev of Fwd Velocity Command
		angVel_std = 0.1;			% Std. dev of Angular Velocity Command

		%% Navigation Parameters
		nav_radius = 0.20;			% Effective Radius of Robot when navigating
		closeEnough = 0.15;			% Close enough to target to be considered at the target [m]
		
		%% Sonar Parameters
		sonar_range = 3;						% Max Linear range of all sonar sensors [m]
		sonar_radius = 0.14;					% Mounting Radius of the sonar sensor [m]
		sonar_min_range= 0.02;					% Minimum linear range of sonar sensor [m]

		% Angular position of the sonar sensors
		sonar_angle = [...
					pi,...
					-0.75*pi,...
					-0.5*pi,...
					-0.25*pi,...
					0,...
					0.25*pi,...
					0.5*pi,...
					0.75*pi];
		sonar_std = 0.05;						% Measurement Noise of the sonar [m]
		sonar_random = 0.04;					% Probability of a random measurement
		sonar_beamWidth = deg2rad(45);			% Sonar Beam Angles [rad]

		% Values greater than 1, widen sensor stdev given to particle filter
		% Values less than 1, shrink the stdev given to particle filter
		% std_par = std_real ^ (1/belief_trust)
		belief_trust = 1;
	end
	
	% Properties that should be set once and only once
	properties(SetAccess=immutable)
		% Ports Used to communicate with the Create
		robotPort = [];    % Port used to communicate with the Roomba
		tagNum = [];
	end
	
	% Properties for replaying previously logged data
	properties(SetAccess = private)
		debugData = [];	% Stores a previously logged dataStore
		debugIndex = 1;	% Current row in debugData from which to read
		debugMode = false;
	end

	properties	
		% Default Max run time for the robot [s] Note: this value is overridden
		% when running in DebugMode so that robot can control the start/stop of
		% the control program. When in debugMode, the maxTime is set to inf
		% while there is still data to be read from debugData. Once all data
		% has been read from debugData, maxTime is set to zero, thus ending the
		% control loop. 
		% if robot.status is recommended over if toc > robot.maxTime
		maxTime = 0;
		
		% Start time of the Robot Instance
		runTime = tic;

		% Map
		map = []    % RobotMap Object to describe the current map
	end

	% Properties that are calculated as needed
	properties(Dependent)
		status	% Flag to check if the robot can keep running
	end
	
	methods
		function obj = Robot(robotAddress)
			% Creates a Robot Object with the default properties and parse
			% the supplied inputs
			%
			%   INPUT:
			%		robotAddress  ipaddress of the RPi3
			%
			%   OUTPUT
			%       obj     A Robot Class object
			
			% Check for empty
			if nargin == 0
				return
			end
			
			% Connect to Pi
			obj.robotPort = tcpclient(robotAddress, 1618);
			
			% Check connection
			resp = obj.sendPacket('CSS');
			if strncmp(resp, 'CSS', 3)
				fprintf('Connected!\n');
			end
			
		end
		
		function pose = sonarPose(obj, poseRobot, s)
			%sonarPose: Returns the pose of the given sonar index
			%   INPUT
			%       poseRobot   The pose of the robot [x, y, t]
			%       s           The index of the sonar
			%   OUTPUT
			%       pose        The pose of the sonar
			
			pose = poseRobot;
			theta = poseRobot(3);
			pose(1:2) = pose(1:2) + obj.sonar_radius*[cos(theta), sin(theta)];
			pose(3) = pose(3) + obj.sonar_angle(s);
		end
		
		function pose = bumpPose(obj, poseRobot, s, n)
			%sonarPose: Returns the pose of the given bump sensor index
			%	INPUT
			%		poseRobot	The pose of the robot [x, y, t]
			%		s				The index of the sonar
			%		n				The number of point to interpolate
			%	OUTPUT
			%		pose 			The pose of the bump sensor
			
			pose = repmat(poseRobot(1:2), [n,1]);
			
			% Sample points along the bump sensor
			theta = linspace(obj.bump_angle(s,1), obj.bump_angle(s,2), n)';
			theta = theta + poseRobot(3);
			
			% Compute location of each point along the bump sensor
			pose = pose + obj.bump_radius*[cos(theta), sin(theta)];
		end

		function indicateWaypoint(obj, waypoint)
			% indicateWaypoint: Flash and beep to indicate arrival

			% Visually Indicate
			BeepRoomba(obj.createPort);

			% Update Map
			obj.map = obj.map.visted(waypoint);
		end

		function status = get.status(obj)
			% get.status: Returns the status of the robot. Used to check if the
			% maxTime has elapsed or if there is no more data to be read from
			% debugData
			%	INPUT
			%		obj		A Robot class object
			%	OUTPUT
			%		status	The status of the robot

			if toc(obj.runTime) < obj.maxTime
				status = true;
			else
				status = false;
			end
		end
		
		function [response, time] = sendPacket(obj, varargin)
			
			% Create Packet
			packet = sprintf(varargin{:});
			
			% Check for stale messages
			if obj.robotPort.BytesAvailable
				obj.robotPort.read;
			end
				
			
			% Send Message
			obj.robotPort.write(uint8(packet));
			
			% Read Response
			msg = char(obj.robotPort.read(1026, 'uint8'));
						
			% Extract Response and Time Stamp
			[time, ~,~,nI] = sscanf(msg, '%f', 1);
			response = msg(nI:end);
		end
		
		%% Define External Functions
		[sonarRange] = sonarPredict(robot, pose)
		
		[belief] = sonarBelief(robot, pose, reading)
		
		[cmdV,cmdW] = limitCmds(robot, fwdVel,angVel)
		
		[dataStore, noRobotCount] = readStoreSensorData(robot, noRobotCount, dataStore, BeaconMode)

		[V, w] = feedbacklin(robot, Vx, Vy, pose)

		[belief] = sensorBelief(robot, pose, dataStore)

		[beacon] = predictBeacon(robot, pose)
		
		SetFwdVelAngVel(robot, FwdVel, AngVel)
		
	end
	
	methods(Static)
		function data = getData(dataStore, dataField)
			% getData: Returns the last entry in dataStore for the given dataField
			%	INPUT
			%		dataStore		The dataStore structure used to store robot data
			%		dataField		The Field in the dataStore (ie. truthPose) of interest
			%	OUTPUT
			%		data			The last dataStore entry for the given dataField
			
			% Check for data
			if isfield(dataStore, dataField) && ~isempty(dataStore.(dataField))
				%Check for mTime
				if isfield(dataStore, 'mTime') && ~iscell(dataStore.(dataField))
					validTime = dataStore.mTime(end,2:end);
					time = dataStore.(dataField)(:, 1);

					% Return Data from the last mTime Frame
					valid = time >= validTime(:, 1) & time <= validTime(:, 2);
					data = dataStore.(dataField)(valid, 2:end);
				else
					data = dataStore.(dataField)(end, 2:end);
				end
			else
				data = [];
			end
		end

		function dataStore = setData(dataStore, dataField, data)
			% setData: Appendes the given entry to dataStore for the supplied field
			%	INPUT
			%		dataStore		The dataStore structure used to store robot data
			%		dataField		The Field in the dataStore (ie. truthPose) to store the data
			%		data				The data to be stored
			%	OUTPUT
			%		dataStore		Updated dataStore structure

			% Check if the dataField doen't exist or is empty
			if ~isfield(dataStore, dataField) || isempty(dataStore.(dataField))
				dataStore.(dataField) = [];
			end

			% Check data Type
			if ~iscell(dataStore.(dataField)) && (isvector(data) && ~isobject(data))
				% Store vectors as a new row
				dataStore.(dataField)(end+1,:) = [toc data];
			else
				% Otherwise store as cell array
				dataStore.(dataField){end+1} = data;
			end
		end
		function plotPose(pose)
			% plotPose: Plots the pose of the robot on the current figure
			%	INPUT
			%		pose		pose of the robot [x, y, t]

			% Check hold status
			priorHold = ishold;
			hold on

			% Default Robot Radius [m]
			radius = 0.2;
			LineWidth = 2;

			% Draw robot body
			n = 25;
			t = linspace(0, 2*pi, n);
			r = radius*ones(1, n);
			[x,y] = pol2cart(t,r);
			plot(x + pose(1), y + pose(2), 'b-', 'LineWidth', 1);

			% Draw Orientation
			x = [0, radius * cos(pose(3))] + pose(1);
			y = [0, radius * sin(pose(3))] + pose(2);	
			plot(x, y, 'b-', 'LineWidth', 1);

			% Restore Hold status
			if priorHold
				hold on
			else
				hold off
			end
		end

		% External Static Methods
		[pose, J] = intergrateOdom(pose0, u, dt)
	end
	
	methods(Access=private)
		function delete(robot)
			% delete: Delete method for the robot class. Used to ensure a safe
			% exit from the control program
			%	INPUT
			%		robot		A Robot Class Object

			try
				%Check if in debug mode
				if ~robot.debugMode
					% Stop the Robot
					robot.SetFwdVelAngVel(0, 0);
				end
				
				% Disconnect from Robot
				robot.sendPacket('SDC');
			catch
				warning('Unable to stop the robot')
			end
		end
	end
	
end

