function MapOcc = logOddsBump(robot, bump, pose, MapOcc, prior)
% logOddsBump: Computes the log odds of each cell in the grid using the
% bump sensor
%   INPUT
%       robot       A robot object: Used to get sonar readings
%       bump        The sonar range measurements [m]
%       pose        The current pose of the robot [x, y, theta]
%       occMap      A gridData object: Represents the log odds of the map
%       prior       The prior log odds for the cell being occupied l_0
%   OUTPUT
%       occMap      A gridData object: Represents the log odds of the map

%% Set Proabilities
P_free = -1;
P_occ = 10;


%% For each valid sonarReading
for s = 1:length(bump)
    % Find the bump sensor pose
    bumpPose = robot.bumpPose(pose, s, 50);
    
    % Find cell containg the bump sensor
   [i, j] = MapOcc.hasPoint(bumpPose(:,1), bumpPose(:,2));
        
    % Update Occupancy Map Belief
    if bump(s)
        MapOcc.data(i,j) = MapOcc.data(i,j) + P_occ - prior;
    else
        MapOcc.data(i,j) = MapOcc.data(i,j) + P_free - prior;
    end
        
end
