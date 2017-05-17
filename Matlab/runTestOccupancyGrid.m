% Load dataStore
load('debugData.mat')

%Set Parameters
lo = 0;
NumCellX = 25;
NumCellY = 25;
boundaryX = [-2.5, 2.5];
boundaryY = [-2.5, 2.5];

% Run TestOccupancyGrid
TestOccupancyGrid(debugData, lo, NumCellX, NumCellY, boundaryX, boundaryY)