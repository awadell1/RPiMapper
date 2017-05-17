function[datastore] = freedriveProgram(robot, maxTime)
% freedriveProgram: example program to manually drive iRobot Create
% Reads data from sensors, sends robot commands, and saves a datalog.
% 
%   DATASTORE = reedriveProgram(CreatePort,SonarPort,BeaconPort,tagNum,maxTime) 
% 
%   INPUTS
%       robot		A Robot Object
%       maxTime     - max time to run program (in ms)
% 
%   OUTPUTS
%       datastore   struct containing logged data
% 
%   Cornell University
%   MAE 5180: Autonomous Mobile Robots
%   Lab #3
%   Wadell, Alexius 

% declare datastore as a global variable so it can be accessed from the
% workspace even if the program is stopped
global dataStore;

% Create Maps
prior = 0;
nX = 100; nY = 100;
edgeX = linspace(-0.5, 0.5, nX);
edgeY = linspace(-0.5, 0.5, nY);
MapOccSonar = gridData(edgeX, edgeY, prior);

%Create Figures
figure
hold on
subplot(1,2,1);
mapFigSonar = plot(MapOccSonar);

subplot(1,2,2);
sonarBar = bar(robot.sonar_angle, 1:8);
ylabel('Sonar Reading [m]');

% Call up manual drive GUI
global SONAR_OFFSET
SONAR_OFFSET = [0 0 0];
h = driveArrows(robot, 0);

% initialize datalog struct (customize according to needs)
dataStore = struct('truthPose', [],...
                   'odometry', [], ...
                   'range', [], ...
				   'IMU', [], ...
                   'logOddsBump',[],...
                   'logOddsSonar',[]);
               
noRobotCount = 0;
tic
robot.maxTime = inf;
robot.runTime = tic;
dataStore.truthPose = [toc 0 0 0];
while robot.status
    % Read and Store Sensore Data
    [dataStore, noRobotCount] = readStoreSensorData(robot, noRobotCount, dataStore);
	
	% Dead Reckon
	pose0 = dataStore.truthPose(end,2:end);
	u = robot.getData(dataStore, 'odometry');
	pose = robot.intergrateOdom(pose0, u);
	robot.setData(dataStore, 'truthPose', pose);
    
    % Update Occupancy Grid
	sonar = robot.getData(dataStore, 'range');
	MapOccSonar = logOddsSonar(robot, sonar, pose, MapOccSonar, prior);
		
    % Plot Occupancy Grid and robot trajectory in real time
	dataStore = robot.setData(dataStore, 'MapSonar', MapOccSonar);

	% Update Plots of Sonar Map
	hold on
	plot(MapOccSonar, mapFigSonar);
    
	hold on
	sonarBar.YData = sonar;
	drawnow
	
    pause(0.1);
end

