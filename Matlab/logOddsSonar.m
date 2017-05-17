function occMap = logOddsSonar(robot, sonarRange, pose, occMap, prior)
% logOddsSonar: Computes the log odds of each cell in the grid using sonar
% 
%   INPUT
%       robot       A robot object: Used to get sonar readings
%       sonarRange  The sonar range measurements [m]
%       pose        The current pose of the robot [x, y, theta]
%       occMap      A gridData object: Represents the log odds of the map
%       prior       The prior log odds for the cell being occupied l_0
%   OUTPUT
%       occMap      A gridData object: Represents the log odds of the map


% Check for sonar nan readings
valid = ~isnan(sonarRange);

%% Set Proabilities
P_free = -10;
P_occ = 50;

% Define Sensor Thickness
beta = 0.1;

if ~numel(valid)
	return
end

%% For each valid sonarReading
for s = find(valid)
    % Find cells in sonar beam
    sonarPose = robot.sonarPose(pose, s);
    [inBeam, cellRange] = occMap.hasBeam(sonarPose, robot.sonar_beamWidth);
    
    % For inBeam cells less than the sonar reading -> Assign free
    freeCells = (P_free - prior) * inBeam.data .* (cellRange.data < sonarRange(s));
    
    % For cells at the sonar range
    maxRange = cellRange.data <= (beta + sonarRange(s));
    minRange = cellRange.data >= sonarRange(s);
    occupiedCells = (P_occ - prior) * inBeam.data .* (maxRange & minRange);
    
    % Update Occupancy Map Belief
    occMap.data = freeCells + occupiedCells + occMap.data;
end
