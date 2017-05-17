function TestOccupancyGrid(dataStore, lo, NumCellX, NumCellY, boundaryX, boundaryY)
% TestOccupancyGrid: Takes sensor reading stored in dataStore and plots the
% occupancy grid using the bump sensors, and another using the sonar
% sensors.
%	INPUT
%		dataStore
%		lo
%		NumCellX
%		NumCellY
%		boundaryX
%		boundaryY
%	Output (Via Figures)
%		Final Occupancy Grid generated using bump sensors
%		Final Occupancy Grid generated using sonar sensors

% Create a Robot Object
robot = Robot([], 'dataStore', dataStore);

% Initalize Occupancy Maps
edgeX = linspace(boundaryX(1), boundaryX(2), NumCellX);
edgeY = linspace(boundaryY(1), boundaryY(2), NumCellY);
MapOccBump = gridData(edgeX, edgeY, lo);
MapOccSonar = MapOccBump;

% Step over all measurements
while robot.status
	% Update local dataStore
	dataStore = readStoreSensorData(robot, 0, dataStore);

	% Readings
	truthPose = robot.getData(dataStore, 'truthPose');
	sonar =robot.getData(dataStore, 'sonar');
	bump = robot.getData(dataStore, 'bump');

	% Update Maps
	MapOccSonar = logOddsSonar(robot, sonar, truthPose, MapOccSonar, lo);
	MapOccBump = logOddsBump(robot, bump, truthPose, MapOccBump, lo);
end

% Plot Bump Map
plotOccupancyGrid(MapOccBump)
title('Final Occupancy Map: Bump Sensors')

% Plot Sonar Map
plotOccupancyGrid(MapOccSonar)
title('Final Occupancy Map using Sonar Sensors')