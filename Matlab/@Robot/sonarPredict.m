function[sonarRange] = sonarPredict(robot, pose)
% SONARPREDICT: predict the sonar range measurements for a robot operating
% in a known map.
%
%   INPUTS
%       robot       A Robot Object
%       robotPose   n-by-3 pose vector in global coordinates (x,y,theta)
%
%   OUTPUTS
%       range       K-by-1 vector of sonar ranges (meters)
%
%   Cornell University
%   MAE 4180/5180 CS 3758: Autonomous Mobile Robots
%   Wadell, Alexius

% Extract x, y, t to be sampled
x = repmat(pose(:,1), 1, 3);
y = repmat(pose(:,2), 1, 3);
t = bsxfun(@plus, pose(:,3), robot.sonar_angle);
x = x(:); y = y(:); t = t(:);

%Predict the range of each sonar
sonarRange = robot.map.intersectRay(x, y, t);

%Correct for Robot Radius
sonarRange = sonarRange - robot.radius;

% Reshape for final list
sonarRange = reshape(sonarRange, [], length(robot.sonar_angle));
end